#include <QApplication>
#include <QFile>
#include <QDebug>

#include "mainwindow.h"

//to simplify the data is already generated in slices of 30 minutes
static
int generateData(QMap<QDateTime, MarketProfile::Data> &data, int maxLineNo = -1)
{
    QString filename = "/Users/bogdan/projects/draw_financial_data/DUK 30 Minutes.txt";
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << file.errorString();
        return EXIT_FAILURE;
    }
    data.clear();
    int lineNo = 0;
    while (!file.atEnd()) {
        QByteArray line = file.readLine().trimmed();
        if (0 == lineNo++) {
            continue;
        }
        QList<QByteArray> tok = line.split(',');
        if (7 > tok.length()) {
            qWarning() << "Invalid line" << line;
            continue;
        }
        QDateTime dateTime = QDateTime::fromString(tok[0]+" "+tok[1], "dd.MM.yyyy hh:mm:ss");
        MarketProfile::Data item;
        item.open = tok[2].toDouble();
        item.high = tok[3].toDouble();
        item.low = tok[4].toDouble();
        item.close = tok[5].toDouble();
        item.volume = tok[6].toInt();
        data[dateTime] = item;
        if ((0 <= maxLineNo) && ((maxLineNo-1) < lineNo)) {
            break;
        }
    }
    return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
     w.show();//do this in order to get correct information about widget size

    QMap<QDateTime, MarketProfile::Data> data;
    MarketProfile *profile = NULL;
    if (EXIT_SUCCESS == generateData(data)) {
        profile = w.marketProfile();
        bool rc = profile->loadTimeSeries(data);
        if (!rc) {
            qCritical() << "Cannot load time series";
            return EXIT_FAILURE;
        }
    }

    /*data.clear();
    if (EXIT_SUCCESS == generateData(data, 14)) {
        bool rc = profile->loadTimeSeries(data);
        if (!rc) {
            qCritical() << "Cannot load time series";
        }
    }*/

    /*data.clear();
    if (EXIT_SUCCESS == generateData(data, 14)) {
        bool rc = profile->updateTimeSeries(data);
        if (!rc) {
            qCritical() << "Cannot load time series";
        }
    }*/

    if (NULL != profile) {
        QDateTime dateTime(QDate(2016, 6, 6));
        QMap<QDateTime, double> position;
        position[dateTime] = 80.5;
        QString indicatorName = "test indicator";
        bool rc = profile->addIndicator(indicatorName, position);
        if (!rc) {
            qCritical() << "Cannot add indicator";
        }
        position[dateTime] = 80.7;
        indicatorName = "test indicator";
        rc = profile->updateIndicator(indicatorName, position);
        if (!rc) {
            qCritical() << "Cannot update indicator";
        }
        rc = profile->hideIndicator(indicatorName);
        if (!rc) {
            qCritical() << "Cannot hide indicator";
        }
        rc = profile->showIndicator(indicatorName);
        if (!rc) {
            qCritical() << "Cannot show indicator";
        }
        rc = profile->removeIndicator(indicatorName);
        if (!rc) {
            qCritical() << "Cannot remove indicator";
        }
        //add the indicator for testing purposes
        rc = profile->addIndicator(indicatorName, position);
        if (!rc) {
            qCritical() << "Cannot add indicator";
        }
    }

    return a.exec();
}
