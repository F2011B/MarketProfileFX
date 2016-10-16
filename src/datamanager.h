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
    bool save(const QMap<QDateTime, MarketProfile::Data> &data);
private:
    static QString databasePath();
    QSqlDatabase _db;
};

#endif
