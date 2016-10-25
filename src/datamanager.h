#ifndef DATA_MANAGER_H_
#define DATA_MANAGER_H_

#include <QSqlDatabase>
#include <QMap>
#include <QDateTime>
#include <QString>
#include <QMutex>
#include "marketprofile.h"

class DataManager : public QObject {
    Q_OBJECT
public:
    DataManager();
signals:
    void requestSave(const QString &symb, const MarketProfile::DataMap &data);
    void requestLoad(const QString &symb);
    //should block until the slot finishes
    void finishedLoad(const MarketProfile::DataMap &data);
    void showDialog(const QString &msg, QMessageBox::Icon icon);
private slots:
    bool save(const QString &symb, const MarketProfile::DataMap &data);
    bool load(const QString &symb);
private:
    int requestsToDeleteCount(uint thresholdSec);
    bool update();
    static QString databasePath();
    bool createTable();
    MarketProfile::DataMap _loadedData;
    QMutex _dbMutex;
    QSqlDatabase _db;
};

#endif
