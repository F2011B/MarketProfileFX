#include <QSqlError>
#include <QSqlQuery>
#include "datamanager.h"
#include "config.h"
#include "mainwindow.h"

#define TABLE_NAME "candles"

DataManager::DataManager()
{
    _now = new QDateTime();
    _threshold = new QDateTime();

    //open database
    _db = QSqlDatabase::addDatabase("QSQLITE", TABLE_NAME);
    if (!_db.isValid()) {
        const QString msg = "Sqlite driver not supported";
        emit showDialog(msg, QMessageBox::Warning);
        qDebug() << msg;
        return;
    }
    const QString dbPath = databasePath();
    _db.setDatabaseName(dbPath);
    if (!_db.open()) {
        const QString msg = "Cannot open database: " + _db.lastError().text();
        emit showDialog(msg, QMessageBox::Warning);
        qCritical() << msg;
        return;
    }

    //create required table if needed
    if (!createTable()) {
        emit showDialog("Cannot create table into database", QMessageBox::Warning);
        _db.close();
    }
    qDebug() << "Using db from" << dbPath;

    //connect available slots
    connect(this, &DataManager::requestSave, this, &DataManager::save);
    connect(this, &DataManager::requestLoad, this, &DataManager::load);
    //connect(this, &DataManager::updateSymbol, this, &DataManager::updateSymbolByRest);
}

QString DataManager::databasePath()
{
    const QString dbFolder = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QFileInfo databaseFileInfo(QString("%1/%2").arg(dbFolder).arg(APP_NAME ".db"));
    const QString databasePath = databaseFileInfo.absoluteFilePath();
    if (!databaseFileInfo.exists()) {
        qWarning() << "Database file does not exist";
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
            if (exists && hasLayout) {
                //table exists and has the expected layout
                qDebug() << "Table" << TABLE_NAME << "already exists";
                return true;
            }
            qDebug() << "Table" << TABLE_NAME << "does not exist";
            if (!hasLayout) {
                if (!query.exec("drop table " TABLE_NAME )) {
                    qCritical() << "Cannot drop table " TABLE_NAME << query.lastError().text();
                    return false;
                }
            }
        }
    } else {
        qCritical() << "Cannot execute query" << query.lastError().text();
    }
    //create table for storing requests if table does not exist
    if (!query.exec(createTableSql)) {
        qCritical() << "Cannot create table" TABLE_NAME << query.lastError().text();
        return false;
    }
    return true;
}

bool DataManager::save(const QString &symb,
                       const MarketProfile::DataMap &data)
{
    if (!_db.isOpen()) {
        qCritical() << "Db is closed";
        return false;
    }
    if (data.isEmpty()) {
        qDebug() << "No data to save";
        return true;
    }

    QMutexLocker lock(&_dbMutex);

    qDebug() << "Saving into db" << data.size() << "rows for symb" << symb;

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

bool DataManager::QueryCandles(QSqlQuery query, const QString &symb)
{
    if (!query.exec("select dateTime, open, high, low, close, volume from " TABLE_NAME " where symb = '"+symb+"' order by dateTime ASC;")) {
        qCritical() << "Cannot exec select query" << query.lastError().text();
        emit finishedLoad(_loadedData);
        return false;
    }
    return true;
}

QSqlQuery DataManager::ReadSymbolFromDB(const QString &symb)
{
    QMutexLocker lock(&_dbMutex);
    QSqlQuery query(_db);
    QueryCandles(query, symb);

    return query;
}

void DataManager::convertToMarketProfileData(QSqlQuery query)
{
    QDateTime dateTime;
    MarketProfile::Data value;
    while (query.next()) {
        dateTime.setTime_t(query.value(0).toUInt());
        value.open = query.value(1).toDouble();
        value.high = query.value(2).toDouble();
        value.low = query.value(3).toDouble();
        value.close = query.value(4).toDouble();
        value.volume = query.value(5).toInt();
        _loadedData[dateTime] = value;
    }
}

bool DataManager::load(const QString &symb)
{
    _loadedData.clear();

    if (!_db.isOpen()) {
        qCritical() << "Db is closed";
        emit finishedLoad(_loadedData);
        return false;
    }

    QSqlQuery query = ReadSymbolFromDB(symb);

    convertToMarketProfileData(query);

    QDateTime startDate = getStartDateFromData();
    QDateTime endDate = getEndDateFromData();

    updateSymbolByRest(symb,startDate, endDate);


    qDebug() << "Loaded from db" << _loadedData.size() << "rows for symb" << symb;
    emit finishedLoad(_loadedData);
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

QDateTime DataManager::getStartDateFromData()
{
//_loadedData
    return *_now;
}

QDateTime DataManager::getEndDateFromData()
{
    return *_now;
}

void DataManager::updateDateBorders()
{
    _now->setDate(QDateTime::currentDateTime().date());
    _now->setTime(QDateTime::currentDateTime().time());
    _threshold->setDate(_now->addDays(-OBSOLETE_DATA_THRESHOLD_DAYS).date());
    _threshold->setTime(_now->addDays(-OBSOLETE_DATA_THRESHOLD_DAYS).time());
}

bool DataManager::update()
{
    updateDateBorders();
    const uint thresholdSec = _threshold->toTime_t();
    const int count = requestsToDeleteCount(thresholdSec);
    if (0 < count) {
        qDebug() << "Updating db, rows to remove" << count;
        QSqlQuery query(_db);
        if (!query.exec("delete from " TABLE_NAME " where dateTime<="+QString::number(thresholdSec)+";")) {
            qCritical() << "Cannot execute delete query" << query.lastError().text();
            return false;
        }
    }
    return true;
}

void DataManager::updateSymbolByRest(const QString &symbol,const QDateTime &startDateTime, const QDateTime &endDateTime)
{



}
