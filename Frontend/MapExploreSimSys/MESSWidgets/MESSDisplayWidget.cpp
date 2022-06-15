#include "MESSDisplayWidget.h"

#include <QMessageBox>
#include <QWebSocket>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QAbstractSocket>

MESSDisplayWidget::MESSDisplayWidget(
        QWidget *parent)
    : BaseDisplayWidget(parent) {
}

void MESSDisplayWidget::start(const QString & http_url, const QString & ws_url) {
    http_server_url_ = http_url;
    ws_server_url_ = ws_url;
    ws_socket_ = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);
    http_mgr = new QNetworkAccessManager(this);
    ws_socket_->open(ws_server_url_);

    connect(ws_socket_, &QWebSocket::connected, [this]() {
        qDebug() << "Connnected";
        connect(ws_socket_, &QWebSocket::textMessageReceived, this, &MESSDisplayWidget::wsOnMessage);
    });
}

void MESSDisplayWidget::paintEvent(QPaintEvent * event) {
    BaseDisplayWidget::paintEvent(event);
}

void MESSDisplayWidget::wsOnMessage(const QString & msg) {
    qDebug() << msg;
    auto jsonDoc = QJsonDocument::fromJson(msg.toUtf8());
    auto respData = jsonDoc.object();

    auto iter = respData.find("code");
    if (iter == respData.end()) {
        QMessageBox::warning(this, "错误", "请求错误");
        return;
    }

    if (iter.value().toInt() != 0) {
        auto iterMsg = respData.find("msg");
        QString content = "请求错误";
        if (iterMsg != respData.end()) {
            content = iterMsg.value().toString();
        }
        QMessageBox::warning(this, "", content);
        return;
    }

    if (respData.contains("data")) {
        auto data = respData["data"].toObject();
        if (data.contains("resources")) {
            BaseDisplayWidget::ImageMap img_cache;
            Q_ASSERT(data["resources"].isArray());
            auto resources = data["resources"].toArray();
            for (const auto &res : resources) {
                Q_ASSERT(res.isObject());
                auto resObj = res.toObject();
                if (resObj.contains("id") && resObj.contains("path")) {
                    auto id = resObj["id"].toInt();
                    auto path = resObj["path"].toString();
                    QNetworkRequest resq(http_server_url_.toString() + "/" + path);
                    QNetworkReply *reply = http_mgr->get(resq);
                    connect(reply, &QNetworkReply::finished, [this, id, reply]() {
                       if (reply->error() != QNetworkReply::NoError) {
                           QMessageBox::warning(this, "错误", reply->errorString());
                       }
                       QImage img;
                       img.loadFromData(reply->readAll());
                       this->addResource(id, img);
                       qInfo() << "Get resource: " << id;
                    });
                }
            }
        }

        if (data.contains("orders")) {
            std::vector<RenderOrder> orders;
            auto arr = data["orders"].toArray();
            for (const auto &e : arr) {
                RenderOrder order;
                order.fromJson(e);
                orders.push_back(std::move(order));
            }
            this->display(orders);
        }
    }
}
