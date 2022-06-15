#include "MESSSimulationWidget.h"
#include "ui_MESSSimulationWidget.h"

#include "utils.h"

#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMessageBox>
#include <QNetworkAccessManager>

MESSSimulationWidget::MESSSimulationWidget(const QString &server_url, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MESSSimulationWidget),
    http_server_url_(server_url) {
    ui->setupUi(this);
    http_mgr_ = new QNetworkAccessManager(this);
}

MESSSimulationWidget::~MESSSimulationWidget() {
    delete ui;
}

void MESSSimulationWidget::on_pushButtonStart_clicked() {
    auto config = ui->widgetConfig->getData();

    asyncHttpPOST(
        http_mgr_,
        http_server_url_.toString() + "/simulation",
        QJsonDocument(config.toObject()).toJson(),
        [this](const QString &resp) {
            // {
            //    "code":
            //    "msg":
            //    "data": { "view_http_url" : "...",
            //               "view_ws_url" : "..."
            //    }
            // }
            auto data = json2ResponseData(resp.toStdString());
            if (data.code != 0) {
                QMessageBox::warning(this, "", data.msg.c_str());
                return;
            }
            ui->widgetDisplay->start(
                    data.data.view_http_url.c_str(),
                    data.data.view_ws_url.c_str());
        },
        [this](const QString &err) {
            QMessageBox::warning(this, "", err);
        });
}
