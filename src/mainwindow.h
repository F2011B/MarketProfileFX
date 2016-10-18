#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
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
    static void showDialog(const QString &msg,
                           QMessageBox::Icon icon = QMessageBox::Critical);
protected:
    void resizeEvent(QResizeEvent *event);
private slots:
    void onUpdate();
    void computeFrom(const QDateTime &latest);
    void onRestRequestFinished(const QVariant &content);
private:
    bool parseCandle(QDateTime &dateTime, MarketProfile::Data &profileData,
                     bool &complete, const QJsonObject &item);
    void displayData(QMap<QDateTime, MarketProfile::Data> &inputData);
    MarketProfile *_profile;
    QPushButton *_updateButton;
    QComboBox *_symbolCombo;
    RestHandler *_restHandler;
    DataManager *_dataManager;
    QDateTime _from;
};

#endif // MAINWINDOW_H
