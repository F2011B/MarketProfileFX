#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    resize(548, 420);
    QWidget *centralWidget= new QWidget(this);
    QVBoxLayout *verticalLayout= new QVBoxLayout(centralWidget);

    _profile = new MarketProfile(centralWidget);
    verticalLayout->addWidget(_profile);
    setCentralWidget(centralWidget);
}

MainWindow::~MainWindow()
{
}

bool MainWindow::loadTimeSeries(const QMap<QDateTime, MarketProfile::Data> &timeSeries)
{
    if (NULL != _profile) {
        _profile->display(timeSeries);
        return true;
    }
    return false;
}
