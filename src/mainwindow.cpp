#include <QPushButton>
#include <QComboBox>
#include <QDebug>
#include <QMessageBox>
#include "mainwindow.h"
#include "resthandler.h"
#include "datamanager.h"
#include "settingsmanager.h"
#include "config.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), _progress(this)
{
    QWidget *centralWidget= new QWidget(this);
    QGridLayout *gridLayout= new QGridLayout(centralWidget);

    //update button
    _updateButton = new QPushButton(tr("Update"), centralWidget);
    connect(_updateButton, &QPushButton::clicked, this, &MainWindow::onUpdate);
    gridLayout->addWidget(_updateButton, 0, 0, 1, 1, Qt::AlignHCenter);

    //symbol combo
    QLabel *symbolLabel = new QLabel(tr("Symbol"), centralWidget);
    gridLayout->addWidget(symbolLabel, 0, 1, 1, 1, Qt::AlignRight);
    _symbolCombo = new QComboBox(centralWidget);
    connect(_symbolCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &MainWindow::onCurrentIndexChanged);
    QStringList symbols;
    symbols << "EUR_USD" << "WTICO_USD" << "XAU_USD" << "DE30_EUR" << "SPX500_USD";
    _symbolCombo->addItems(symbols);
    int currentIndex = 0;
    SettingsManager::readCurrentSymbolIndex(currentIndex);
    _symbolCombo->setCurrentIndex(currentIndex);
    gridLayout->addWidget(_symbolCombo, 0, 2, 1, 1, Qt::AlignLeft);

    //main plot
    _profile = new MarketProfile(centralWidget);
    _profile->setBackgroudColor(255, 255, 255);
    _profile->setLiteralColor(0, 0, 255);
    _profile->setXLabel("Date of the trading");
    _profile->setYLabel("Price");
    _profile->setLabelColor(255, 0, 255);
    gridLayout->addWidget(_profile, 1, 0, 1, 3);

    //progress dialog
    _progress.setLabelText(tr("Please wait ..."));
    _progress.setCancelButton(NULL);
    _progress.setRange(0, 0);
    _progress.setMinimumDuration(500);
    _progress.setAutoClose(true);
    _progress.setAutoReset(true);
    _progress.setWindowModality(Qt::WindowModal);
    _progress.reset();//show progress only when needed

    //REST handler
    _restHandler = new RestHandler(this);
    connect(_restHandler, &RestHandler::finished, this,
            &MainWindow::onRestRequestFinished);

    //Data manager
    _dataManager = new DataManager();
    _dataManager->moveToThread(&_dataManagerThread);
    connect(&_dataManagerThread, &QThread::finished, _dataManager, &QObject::deleteLater);
    connect(_dataManager, &DataManager::finishedLoad, this, &MainWindow::onLoadRequestFinished,
            Qt::BlockingQueuedConnection);
    connect(_dataManager, &DataManager::showDialog, this, &MainWindow::showDialog);
    _dataManagerThread.start();
    _loadOldData = true;//make sure that old data is loaded first
    onUpdate();//send request for new data

    setCentralWidget(centralWidget);
    setGeometry(QApplication::desktop()->availableGeometry());
}

MainWindow::~MainWindow()
{
    _dataManagerThread.quit();
    _dataManagerThread.wait();
    if (NULL != _symbolCombo) {
        const int currentIndex = _symbolCombo->currentIndex();
        SettingsManager::writeCurrentSymbolIndex(currentIndex);
    }
}

void MainWindow::resizeEvent(QResizeEvent */*event*/)
{
    setGeometry(QApplication::desktop()->availableGeometry());
    _profile->replot();//recompute letter heights
}

void MainWindow::onUpdate()
{
    if (_loadOldData) {
        _loadOldData = false;
        qDebug() << "Loading old data from db if any";
        _profile->clearPlot();
        emit _dataManager->requestLoad(_symbolCombo->currentText());
        //http request is send once this request is finished
    } else {
        sendRestRequest();
    }
}

void MainWindow::onRestRequestFinished(const QVariant &content)
{
    qDebug() << "onRestRequestFinished";
    QJsonObject data;
    QString errorString;
    int type = content.type();
    switch (type) {
    case QMetaType::QJsonObject:
        data = content.toJsonObject();
        break;
    case QMetaType::QString:
        errorString = content.toString();
        qCritical() << "Error while retrieving data: " << errorString;
        showDialog(errorString, QMessageBox::Critical);
        _progress.reset();
        return;
    default:
        qCritical() << "Unknown variant type";
        showDialog(tr("Unknown variant type"), QMessageBox::Critical);
        _progress.reset();
        return;
    }

    //parse candles array
    QJsonArray candles = data.value(CANDLES_NAME).toArray();
    if (candles.isEmpty()) {
        qDebug() << "No data received";
        _progress.reset();
        return;
    }
    qDebug() << "Got" << candles.size() << "candles";
    MarketProfile::DataMap inputData;
    for (int i = 0; i < candles.size(); ++i) {
        QJsonObject item = candles.at(i).toObject();
        QDateTime dateTime;
        MarketProfile::Data profileData;
        bool complete = false;
        bool rc = parseCandle(dateTime, profileData, complete, item);
        if (!rc) {
            qWarning() << "Cannot parse candle" << i;
            continue;
        }
        if (!complete) {
            qDebug() << "Found incomplete candle at index" << i;
            continue;//TODO
        }
        inputData[dateTime] = profileData;
    }
    qDebug() << "Number of complete candles" << inputData.size();
    displayData(inputData);

    _progress.reset();

    //save data
    emit _dataManager->requestSave(_symbolCombo->currentText(), inputData);
}

void MainWindow::onLoadRequestFinished(const MarketProfile::DataMap &inputData)
{
    displayData(inputData);
    sendRestRequest();
}

void MainWindow::displayData(const MarketProfile::DataMap &inputData)
{
    if (!inputData.isEmpty()) {
        bool rc = _profile->updateTimeSeries(inputData);
        if (!rc) {
            qCritical() << "Cannot load data";
            showDialog(tr("Cannot load data"), QMessageBox::Critical);
        }
        computeFrom(inputData.lastKey());
    } else {
        computeFrom(QDateTime());
    }
}

void MainWindow::computeFrom(const QDateTime &latest)
{
    if (latest.isValid()) {
        _from = latest;
        qDebug() << "Start date for HTTP request is computed from old data" << _from;
    } else {
        _from = QDateTime::currentDateTime().addDays(-OBSOLETE_DATA_THRESHOLD_DAYS);
        qDebug() << "Start date for HTTP request uses default value" << _from;
    }
}

void MainWindow::showDialog(const QString &msg, QMessageBox::Icon icon)
{
    QMessageBox dlg(icon, APP_NAME, msg, QMessageBox::Ok, this);
    dlg.exec();
}

bool MainWindow::parseCandle(QDateTime &dateTime, MarketProfile::Data &profileData,
                 bool &complete, const QJsonObject &item)
{
    const QString dateTimeStr = item.value(TIME_NAME).toString();
    if (dateTimeStr.isEmpty()) {
        qCritical() << "Cannot find" << TIME_NAME;
        return false;
    }
    dateTime = QDateTime::fromString(dateTimeStr, Qt::ISODate);
    if (!dateTime.isValid()) {
        qCritical() << "Invalid date and time";
        return false;
    }
    profileData.open = item.value(OPEN_NAME).toDouble(-1);
    if (0 > profileData.open) {
        qCritical() << "Cannot find" << OPEN_NAME;
        return false;
    }
    profileData.high = item.value(HIGH_NAME).toDouble(-1);
    if (0 > profileData.high) {
        qCritical() << "Cannot find" << HIGH_NAME;
        return false;
    }
    profileData.low = item.value(LOW_NAME).toDouble(-1);
    if (0 > profileData.low) {
        qCritical() << "Cannot find" << LOW_NAME;
        return false;
    }
    profileData.close = item.value(CLOSE_NAME).toDouble(-1);
    if (0 > profileData.close) {
        qCritical() << "Cannot find" << CLOSE_NAME;
        return false;
    }
    profileData.volume = item.value(VOLUME_NAME).toInt(-1);
    if (0 > profileData.volume) {
        qCritical() << "Cannot find" << VOLUME_NAME;
        return false;
    }
    QJsonValue val = item.value(COMPLETE_NAME);
    if (!val.isBool()) {
        qCritical() << "Cannot find" << COMPLETE_NAME;
        return false;
    }
    complete = val.toBool();
    return true;
}

void MainWindow::onCurrentIndexChanged(int index)
{
    qDebug() << "Symbol changed" << index;
    _loadOldData = (0 <= index);
}

void MainWindow::sendRestRequest()
{
    bool rc = _restHandler->sendRequest(_symbolCombo->currentText(), _from);
    if (rc) {
        _progress.setVisible(true);
    } else {
        qCritical() << "Cannot send request";
        showDialog(tr("Cannot send update request"), QMessageBox::Critical);
    }
}
