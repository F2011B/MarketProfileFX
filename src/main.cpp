#include <QApplication>
#include <QFile>
#include <QDebug>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    qSetMessagePattern("%{time process} [%{type}] - %{message} (%{file}:%{line})");

    MainWindow w;
    w.show();

    return a.exec();
}
