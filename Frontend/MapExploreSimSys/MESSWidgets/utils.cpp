#include "utils.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QMessageBox>
#include <QNetworkAccessManager>

void asyncHttpGET(
        QNAM * mgr, const QString & url,
        const std::function<void (const QString &)> & then,
        const std::function<void(const QString &)> &err) {
    QNetworkRequest resq(url);
    QNetworkReply *reply = mgr->get(resq);
    QObject::connect(reply, &QNetworkReply::finished, [&then, &err, reply]() {
        if (reply->error() != QNetworkReply::NoError) {
            err(reply->errorString());
        }
        then(reply->readAll());
    });
}

void asyncHttpPOST(
        QNAM *mgr, const QString & url, const QByteArray &body,
        const std::function<void (const QString &)> & then,
        const std::function<void(const QString &)> &err) {
    QNetworkRequest resq(url);
    QNetworkReply *reply = mgr->post(resq, body);
    QObject::connect(reply, &QNetworkReply::finished, [&then, &err, reply]() {
        if (reply->error() != QNetworkReply::NoError) {
            err(reply->errorString());
        }
        then(reply->readAll());
    });
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
