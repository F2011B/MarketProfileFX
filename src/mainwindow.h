#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProgressDialog>
#include <QThread>
#include "qcustomplot.h"
#include "marketprofile.h"

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
    void computeFrom(const QDateTime &latest);
    void onRestRequestFinished(const QVariant &content);
    void onCurrentIndexChanged(int index);
    void onLoadRequestFinished(const MarketProfile::DataMap &inputData);
private:
    void displayData(const MarketProfile::DataMap &inputData);
    bool parseCandle(QDateTime &dateTime, MarketProfile::Data &profileData,
                     bool &complete, const QJsonObject &item);
    void sendRestRequest();
    MarketProfile *_profile;
    QPushButton *_updateButton;
    QComboBox *_symbolCombo;
    RestHandler *_restHandler;
    DataManager *_dataManager;
    QDateTime _from;
    QProgressDialog _progress;
    bool _loadOldData;
    QThread _dataManagerThread;
};

#endif // MAINWINDOW_H
