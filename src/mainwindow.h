#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "qcustomplot.h"
#include "marketprofile.h"

class QPushButton;
class QComboBox;
class RestHandler;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow() {}
    MarketProfile* marketProfile() {
        return _profile;
    }
protected:
    void resizeEvent(QResizeEvent *event);
private slots:
    void onUpdate();
    void onRestRequestFinished(const QVariant &content);
private:
    void showDialog(const QString &msg, QMessageBox::Icon icon = QMessageBox::Critical);
    bool parseCandle(QDateTime &dateTime, MarketProfile::Data &profileData,
                     bool &complete, const QJsonObject &item);
    MarketProfile *_profile;
    QPushButton *_updateButton;
    QComboBox *_symbolCombo;
    RestHandler *_restHandler;
};

#endif // MAINWINDOW_H
