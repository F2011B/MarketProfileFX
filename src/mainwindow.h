#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProgressDialog>
#include <QThread>
#include "qcustomplot.h"
#include "marketprofile.h"
#include "candlestickchart.h"

class QPushButton;
class QComboBox;
class RestHandler;
class DataManager;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
    MarketProfile* marketProfile() {
        return _profile;
    }
protected:
    void resizeEvent(QResizeEvent *event);
private slots:
    void showDialog(const QString &msg,
                           QMessageBox::Icon icon);
    void onUpdate();
    void onSwitch();
    void computeFrom(const QDateTime &latest);
    void onRestRequestFinished(const QVariant &content);
    void onCurrentIndexChanged(int index);
    void onLoadRequestFinished(const MarketProfile::DataMap &inputData);
private:
    void displayData(const MarketProfile::DataMap &inputData);
    bool parseCandle(QDateTime &dateTime, MarketProfile::Data &profileData,
                     bool &complete, const QJsonObject &item);
    void sendRestRequest();
    QSharedPointer<QCPFinancialDataContainer> ConvertToQCPFinancialData(const MarketProfile::DataMap &inputData);

    CandlestickChart *_candle;
    MarketProfile *_profile;
    QPushButton *_updateButton;
    QPushButton *_switchButton;
    QComboBox *_symbolCombo;
    QSpinBox *_symbolSpinBox;
    RestHandler *_restHandler;
    DataManager *_dataManager;
    QDateTime _from;
    QProgressDialog _progress;
    bool _loadOldData;
    QThread _dataManagerThread;    
    MarketProfile::DataMap parseCandles(QJsonArray candles);
};

#endif // MAINWINDOW_H
