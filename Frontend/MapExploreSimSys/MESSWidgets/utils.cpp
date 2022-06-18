#include "utils.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QMessageBox>
#include <QNetworkAccessManager>

void asyncHttpGET(
        QNAM * mgr, const QString & url,
        std::function<void (const QString &)> then,
        std::function<void(const QString &)> err) {
    QNetworkRequest resq(url);
    QNetworkReply *reply = mgr->get(resq);
    QObject::connect(reply, &QNetworkReply::finished, [then, err, reply]() {
        if (reply->error() != QNetworkReply::NoError) {
            err(reply->errorString());
        }
        then(reply->readAll());
    });
}

void asyncHttpPOST(
        QNAM *mgr, const QString & url, const QByteArray &body,
        std::function<void (const QString &)> then,
        std::function<void(const QString &)> err) {
    QNetworkRequest resq(url);
    qDebug() << __FILE__ << __LINE__;
    resq.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    qDebug() << __FILE__ << __LINE__;
    QNetworkReply *reply = mgr->post(resq, body);
    qDebug() << __FILE__ << __LINE__;
    QObject::connect(reply, &QNetworkReply::finished, [then, err, reply]() {
        qDebug() << __FILE__ << __LINE__;
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << __FILE__ << __LINE__;
            err(reply->errorString());
        qDebug() << __FILE__ << __LINE__;
            return;
        }
        qDebug() << __FILE__ << __LINE__;
        then(reply->readAll());
        qDebug() << __FILE__ << __LINE__;
    });
    qDebug() << __FILE__ << __LINE__;
}

RespData json2ResponseData(const std::string & str, bool *ok) {
    try {
        RespData data;
        xpack::json::decode(str, data);
        return data;
    } catch (std::exception &) {
       if (ok) *ok = false;
    }

    if (ok) *ok = true;
    return {};
}
