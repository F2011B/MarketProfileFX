#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    resize(548, 420);
    QWidget *centralWidget= new QWidget(this);
    QVBoxLayout *verticalLayout= new QVBoxLayout(centralWidget);
    _customPlot = new QCustomPlot(centralWidget);
    _customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
                                 QCP::iSelectLegend | QCP::iSelectPlottables);
    verticalLayout->addWidget(_customPlot);
    setCentralWidget(centralWidget);
    _profile = new MarketProfile(_customPlot, font());
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
