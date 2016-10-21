#include <QSqlError>
#include <QSqlQuery>
#include "datamanager.h"
#include "config.h"
#include "mainwindow.h"

#define TABLE_NAME "candles"

DataManager::DataManager()
{
    //open database
    _db = QSqlDatabase::addDatabase("QSQLITE", TABLE_NAME);
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

    //create required table if needed
    if (!createTable()) {
        MainWindow::showDialog("Cannot create table into database", QMessageBox::Warning);
        _db.close();
    }
    qDebug() << "Using db from" << dbPath;
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

bool DataManager::createTable()
{
    static const QString createTableSql = "create table " TABLE_NAME "(symb TEXT, dateTime INTEGER, open REAL, high REAL, low REAL, close REAL, volume INTEGER)";

    //check for table existence
    QSqlQuery query(_db);
    if (query.exec("select name,sql from sqlite_master where type='table' and name='" TABLE_NAME "'")) {
        if (query.next()) {
            bool exists = query.value(0).toBool();
            QString sql = query.value(1).toString();
            bool hasLayout = sql.contains(createTableSql, Qt::CaseInsensitive);
            qDebug() << TABLE_NAME << "has layout" << hasLayout;
            if (exists && hasLayout) {
                //table exists and has the expected layout
                qDebug() << "table" << TABLE_NAME << "already exists";
                return true;
            }
            if (!hasLayout) {
                if (!query.exec("drop table " TABLE_NAME )) {
                    qDebug() << "Cannot drop table " TABLE_NAME << query.lastError().text();
                    return false;
                }
            }
        }
    }
    //create table for storing requests if table does not exist
    if (!query.exec(createTableSql)) {
        qCritical() << "Cannot create table" TABLE_NAME << query.lastError().text();
        return false;
    }
    return true;
}

bool DataManager::save(const QString &symb,
                       const QMap<QDateTime, MarketProfile::Data> &data)
{
    if (!_db.isOpen()) {
        qCritical() << "Db is closed";
        return false;
    }
    qDebug() << "Saving" << data.size() << "rows for symb" << symb;

    //symb, dateTime, data
    QSqlQuery query(_db);
    if (!query.prepare("INSERT INTO " TABLE_NAME " (symb, dateTime, open, high, low, close, volume) VALUES (:symb, :dateTime, :open, :high, :low, :close, :volume)")) {
        qCritical() << "Cannot prepare query for inserting row into database" << query.lastError().text();
        return false;
    }

    QMapIterator<QDateTime, MarketProfile::Data> it(data);
    query.bindValue(":symb", symb);
    while (it.hasNext()) {
        it.next();
        MarketProfile::Data value = it.value();
        query.bindValue(":dateTime", it.key().toTime_t());
        query.bindValue(":open", value.open);
        query.bindValue(":high", value.high);
        query.bindValue(":low", value.low);
        query.bindValue(":close", value.close);
        query.bindValue(":volume", value.volume);
        if (!query.exec()) {
            qCritical() << "Cannot exec insert query" << query.lastError().text();
            return false;
        }
    }
    return update();//remove old entries
}

bool DataManager::load(const QString &symb, QMap<QDateTime, MarketProfile::Data> &data)
{
    if (!_db.isOpen()) {
        qCritical() << "Db is closed";
        return false;
    }

    QSqlQuery query(_db);
    if (!query.exec("select dateTime, open, high, low, close, volume from " TABLE_NAME " where symb = '"+symb+"' order by dateTime ASC;")) {
        qCritical() << "Cannot exec select query" << query.lastError().text();
        return false;
    }
    QDateTime dateTime;
    MarketProfile::Data value;
    data.clear();
    while (query.next()) {
        dateTime.setTime_t(query.value(0).toUInt());
        value.open = query.value(1).toDouble();
        value.high = query.value(2).toDouble();
        value.low = query.value(3).toDouble();
        value.close = query.value(4).toDouble();
        value.volume = query.value(5).toInt();
        data[dateTime] = value;
    }
    qDebug() << "Loaded" << data.size() << "rows for symb" << symb;
    return true;
}

int DataManager::requestsToDeleteCount(uint thresholdSec)
{
    int count = 0;
    QSqlQuery query(_db);
    if (!query.exec("select count(*) from " TABLE_NAME " where dateTime<="+QString::number(thresholdSec)+";")) {
        qCritical() << "Cannot exec count query" << query.lastError().text();
        return count;
    }
    if (query.next()) {
        count = query.value(0).toInt();
    }
    return count;
}

bool DataManager::update()
{
    const QDateTime now = QDateTime::currentDateTime();
    const QDateTime threshold = now.addDays(-OBSOLETE_DATA_THRESHOLD_DAYS);
    const uint thresholdSec = threshold.toTime_t();
    qDebug() << "Update db, rows to remove" << requestsToDeleteCount(thresholdSec);
    QSqlQuery query(_db);
    if (!query.exec("delete from " TABLE_NAME " where dateTime<="+QString::number(thresholdSec)+";")) {
        qCritical() << "Cannot execute delete query" << query.lastError().text();
        return false;
    }
    return true;
}
