#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "qcustomplot.h"
#include "marketprofile.h"

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
private:
    MarketProfile *_profile;
};

#endif // MAINWINDOW_H
