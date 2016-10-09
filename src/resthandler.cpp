#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include "resthandler.h"
#include "config.h"

RestHandler::RestHandler(QObject *parent) : QObject(parent),
    _http(new QNetworkAccessManager(this))
{
    connect(_http, SIGNAL(finished(QNetworkReply*)), this, SLOT(requestFinished(QNetworkReply*)));
}

RestHandler::~RestHandler()
{
    _http->deleteLater();
}

bool RestHandler::sendRequest(const QString &instrument, int count,
                              const QString &granularity)
{
    qDebug() << "Sending request" << instrument << count << granularity;

    QString url = QString(OANDA_URL).arg(instrument).arg(count).arg(granularity);
    QNetworkRequest request(url);
    QSslConfiguration config = request.sslConfiguration();
    config.setProtocol(QSsl::TlsV1_0);
    request.setSslConfiguration(config);
    QNetworkReply *reply = _http->get(request);
    QMutexLocker lock(&_loadingMapMutex);
    _loadingMap[reply].url = url;
    return true;
}

void RestHandler::requestFinished(QNetworkReply* reply)
{
    {
        QMutexLocker lock(&_loadingMapMutex);
        if(!_loadingMap.contains(reply)) {
            qCritical() << "Received unknown reply";
            emit finished("Unknown reply");
            return;
        }
    }
    int rc = reply->error();
    switch (rc) {
    case QNetworkReply::NoError:
    {
        //get JSON reply
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        if (doc.isArray()) {
            emit finished(doc.array());
        } else if (doc.isObject()) {
            emit finished(doc.object());
        } else {
            emit finished(QJsonObject());
        }
    }
        break;
    default:
    {
        int httpCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QString errString = QString(reply->readAll());
        if (errString.isEmpty()) {
            errString = reply->errorString();
            if (errString.isEmpty()) {
                errString = QString("Unknown error - HTTP error code %1").arg(httpCode);
            }
        }
        qDebug() << "HTTP error code" << httpCode << ":" << errString;
        emit finished(errString);
    }
    }
    {
        //remove request
        QMutexLocker lock(&_loadingMapMutex);
        _loadingMap.remove(reply);
    }
}

void RestHandler::abortRequests()
{
    QMutexLocker lock(&_loadingMapMutex);
    _loadingMap.clear();
}
