#ifndef REST_HANDLER_H_
#define REST_HANDLER_H_

#include <QObject>
#include <QMap>
#include <QMutex>
#include <QDateTime>
#include "config.h"

class QNetworkAccessManager;
class QNetworkReply;

class RestHandler : public QObject {
    Q_OBJECT
public:
    RestHandler(QObject *parent);
    ~RestHandler();
    /*!
     * \brief Send REST API request to server. The request is asynchronous,
     * one must connect to finished() signal in order to get the result.
     * \param instrument
     * \param from
     * \param granularity
     * \return true is the operation is successful, false otherwise
     */
    bool sendRequest(const QString &instrument,
                     const QDateTime &from,
                     const QString &granularity = CANDLE_GRANULARITY);
    /*! Abort all pending requests.
     */
    void abortRequests();
signals:
    /*! Emitted when a request has been completed successfully
     * \param error non empty string if an error occured
     */
    void finished(const QVariant &content);
private slots:
    void requestFinished(QNetworkReply* reply);
private:
    QNetworkAccessManager* _http;
    struct ConnectionCache {
        QString url;
    };
    QMap<QNetworkReply*, ConnectionCache> _loadingMap;
    QMutex _loadingMapMutex;
};

#endif
