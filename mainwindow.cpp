#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    resize(548, 420);
    QWidget *centralWidget= new QWidget(this);
    QVBoxLayout *verticalLayout= new QVBoxLayout(centralWidget);

    _profile = new MarketProfile(centralWidget);
    _profile->setBackgroudColor(255, 255, 255);
    _profile->setLiteralColor(0, 0, 255);
    _profile->setXLabel("Date of the trading");
    _profile->setYLabel("Price");
    _profile->setLabelColor(255, 0, 255);

    verticalLayout->addWidget(_profile);
    setCentralWidget(centralWidget);
}
