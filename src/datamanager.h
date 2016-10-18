#ifndef DATA_MANAGER_H_
#define DATA_MANAGER_H_

#include <QSqlDatabase>
#include <QMap>
#include <QDateTime>
#include <QString>
#include "marketprofile.h"

class DataManager {
public:
    DataManager();
    bool save(const QString &symb, const QMap<QDateTime, MarketProfile::Data> &data);
    bool load(const QString &symb, QMap<QDateTime, MarketProfile::Data> &data);
private:
    bool update();
    static QString databasePath();
    bool createTable();
    QSqlDatabase _db;
};

#endif
