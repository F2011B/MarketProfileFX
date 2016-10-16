#include <QSqlError>
#include <QSqlQuery>
#include "datamanager.h"
#include "config.h"
#include "mainwindow.h"

DataManager::DataManager()
{
    //open database
    _db = QSqlDatabase::addDatabase("QSQLITE", "candles");
    if (!_db.isValid()) {
        const QString msg = "Sqlite driver not supported";
        MainWindow::showDialog(msg, QMessageBox::Warning);
        qDebug() << msg;
        return;
    }
    const QString dbPath = databasePath();
    _db.setDatabaseName(dbPath);
    if (!_db.open()) {
        const QString msg = "Cannot open database: " + _db.lastError().text();
        MainWindow::showDialog(msg, QMessageBox::Warning);
        qDebug() << msg;
        return;
    }
}

QString DataManager::databasePath()
{
    const QString dbFolder = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QFileInfo databaseFileInfo(QString("%1/%2").arg(dbFolder).arg(APP_NAME ".db"));
    const QString databasePath = databaseFileInfo.absoluteFilePath();
    if (!databaseFileInfo.exists()) {
        qDebug() << "Database does not exist";
    }
    return databasePath;
}

bool DataManager::save(const QString &symb,
                       const QMap<QDateTime, MarketProfile::Data> &data)
{
    //symb, dateTime, data
}
