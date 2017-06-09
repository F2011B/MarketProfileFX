#ifndef DATA_MANAGER_H_
#define DATA_MANAGER_H_

#include <QSqlDatabase>
#include <QMap>
#include <QDateTime>
#include <QString>
#include <QMutex>
#include "marketprofile.h" // Get rid of that dependency DataManager has nothing to do with marketprofile

class DataManager : public QObject {
    Q_OBJECT
public:
    DataManager();
signals:
    void requestSave(const QString &symb, const MarketProfile::DataMap &data);
    void requestLoad(const QString &symb);
    void updateSymbol(const QString &symb);
    //should block until the slot finishes
    void finishedLoad(const MarketProfile::DataMap &data);
    void showDialog(const QString &msg, QMessageBox::Icon icon);
private slots:
    bool save(const QString &symb, const MarketProfile::DataMap &data);
    bool load(const QString &symb);
private:
    // Methods
    int requestsToDeleteCount(uint thresholdSec);
    bool update();
    static QString databasePath();
    bool createTable();
    MarketProfile::DataMap _loadedData;
    QMutex _dbMutex;
    QSqlDatabase _db;

    QDateTime getStartDateFromData();
    QDateTime getEndDateFromData();
    void updateDateBorders();    
    void updateSymbolByRest(const QString &symbol,const QDateTime &startDateTime, const QDateTime &endDateTime);

    // Variables
    QDateTime* _now;
    QDateTime* _threshold;
    bool QueryCandles(QSqlQuery query, const QString &symb);
    QSqlQuery ReadSymbolFromDB(const QString &symb);
    void convertToMarketProfileData(QSqlQuery query);
};

#endif
