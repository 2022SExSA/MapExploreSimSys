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

// Create
void MESSSimulationWidget::on_pushButtonStart_clicked() {
    if (!ui->widgetConfig->checkData()) return;
    ui->widgetConfig->setAuthToken("MESSAuthToken_" + std::to_string(std::time(nullptr)));
    auto config = ui->widgetConfig->getData();
    qDebug() << config.c_str();
    asyncHttpPOST(
        http_mgr_,
        http_server_url_.toString() + "/simulation",
        QByteArray(config.c_str(), config.size()),
        [this](const QString &resp) {
            // {
            //    "code":
            //    "msg":
            //    "data": { "view_http_url" : "...",
            //               "view_ws_url" : "..."
            //    }
            // }
            qDebug() << resp;
            auto data = json2ResponseData(resp.toStdString());
            if (data.code != 0) {
                QMessageBox::warning(this, "", data.msg.c_str());
                return;
            }
            exper_state_ = PAUSING;
            ui->pushButtonPause->setText("开始");
            ui->widgetDisplay->start(
                    data.data.view_http_url.c_str(),
                    data.data.view_ws_url.c_str());
        },
        [this](const QString &err) {
            QMessageBox::warning(this, "", err);
        });
}

// Start | Pasuse
void MESSSimulationWidget::on_pushButtonPause_clicked() {
    auto config = ui->widgetConfig->getInteralData();
    ES next_state = NOTHING;
    QString url;
    QString next_button_text;
    if (exper_state_ == RUNNNING) {
        next_state = PAUSING;
        url = "/pause_simulation";
        next_button_text = "开始";
    } else if (exper_state_ == PAUSING) {
        next_state = RUNNNING;
        url = "/start_simulation";
        next_button_text = "暂停";
    } else {
        QMessageBox::information(this, "提示", "请先创建一个实验");
        return;
    }
    asyncHttpPOST(
        http_mgr_,
        http_server_url_.toString() + url,
        QString("{\"auth_token\":\"%1\"}").arg(config.auth_token.c_str()).toUtf8(),
        [this, next_state, next_button_text](const QString &resp) {
            // {
            //    "code":
            //    "msg":
            //    "data": { "view_http_url" : "...",
            //               "view_ws_url" : "..."
            //    }
            // }
            qDebug() << resp;
            auto data = json2ResponseData(resp.toStdString());
            if (data.code != 0) {
                QMessageBox::warning(this, "", data.msg.c_str());
                return;
            }
            exper_state_ = next_state;
            ui->pushButtonPause->setText(next_button_text);
        },
        [this](const QString &err) {
            QMessageBox::warning(this, "", err);
        });
}

void MESSSimulationWidget::on_pushButtonStop_clicked() {
    if (exper_state_ != RUNNNING && exper_state_ != PAUSING) {
        QMessageBox::information(this, "提示", "沒有运行实验");
        return;
    }

    auto config = ui->widgetConfig->getInteralData();

    auto auth_token  = config.auth_token;
    auto save = [this, auth_token]() {
        asyncHttpPOST(
            http_mgr_,
            http_server_url_.toString() + "/save_simulation",
            QString("{\"auth_token\":\"%1\"}").arg(auth_token.c_str()).toUtf8(),
            [this](const QString &resp) {
                qDebug() << resp;
                auto data = json2ResponseData(resp.toStdString());
                if (data.code != 0) {
                    QMessageBox::warning(this, "", data.msg.c_str());
                    return;
                }
                QMessageBox::information(this, "提示", "已自动保存自服务器");
            },
            [this](const QString &err) {
                QMessageBox::warning(this, "", err);
            });
    };

    asyncHttpPOST(
        http_mgr_,
        http_server_url_.toString() + "/stop_simulation",
        QString("{\"auth_token\":\"%1\"}").arg(config.auth_token.c_str()).toUtf8(),
        [this, save](const QString &resp) {
            // {
            //    "code":
            //    "msg":
            //    "data": { "view_http_url" : "...",
            //               "view_ws_url" : "..."
            //    }
            // }
            qDebug() << resp;
            auto data = json2ResponseData(resp.toStdString());
            if (data.code != 0) {
                QMessageBox::warning(this, "", data.msg.c_str());
                return;
            }
            exper_state_ = NOTHING;
            ui->widgetDisplay->try_stop();
            ui->widgetDisplay->update();
            save();
        },
        [this](const QString &err) {
            QMessageBox::warning(this, "", err);
        });
}
