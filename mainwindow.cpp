#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    resize(548, 420);
    QWidget *centralWidget= new QWidget(this);
    QVBoxLayout *verticalLayout= new QVBoxLayout(centralWidget);
    _customPlot = new QCustomPlot(centralWidget);
    verticalLayout->addWidget(_customPlot);
    setCentralWidget(centralWidget);
}

MainWindow::~MainWindow()
{

}
