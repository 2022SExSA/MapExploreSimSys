#include "ExperimentDemandWidget.h"
#include "ui_ExperimentDemandWidget.h"

#include "utils.h"
#include "da_utils.h"
#include "server_config.h"
#include "backend_config.h"
#include "RenderOrder.h"

#include <QTimer>
#include <QNetworkAccessManager>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QMessageBox>
#include <QDateTime>
#include <QNetworkReply>

static QModelIndex getRootModelIndex(const QModelIndex &index) {
    if (!index.parent().isValid()) return index;
    return getRootModelIndex(index.parent());
}

ExperimentDemandWidget::ExperimentDemandWidget(const QString &server_url, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ExperimentDemandWidget),
    http_server_url_(server_url) {
    ui->setupUi(this);

    http_mgr_ = new QNetworkAccessManager(this);
    ui->treeViewRunningExperiments->setModel(&running_experiments_data_json_model);
    ui->treeViewHistoryExperiment->setModel(&history_experiments_data_json_model);
    ui->treeViewRunningExperiments->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->treeViewHistoryExperiment->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->lineEditHisExperToken->setEnabled(false);
    ui->lineEditRunExperToken->setEnabled(false);
    ui->tabWidgetData->setTabText(0, "正在运行的实验");
    ui->tabWidgetData->setTabText(1, "实验历史");
    ui->tabWidgetDisplay->removeTab(0);
    ui->tabWidgetDisplay->removeTab(0);
    ui->tabWidgetDisplay->setUsesScrollButtons(true);
    ui->tabWidgetDisplay->setTabsClosable(true);
    ui->tabWidgetDisplay->setMovable(true);
    flushAndShow();

    QObject::connect(ui->treeViewHistoryExperiment, &QTreeView::clicked, [this] (const QModelIndex &index) {
        auto i = getRootModelIndex(index).data().toString().toInt();
        auto token = current_auth_tokens_of_his_expr[i];
        if (!playback_.count(token)) {
            ui->progressBarHistory->setValue(0);
            ui->pushButtonPlaybackExper->setText("回放");
        } else {
            const auto &play = playback_[token];
            ui->progressBarHistory->setValue((play.curr_index * ui->progressBarHistory->maximum()) / play.render_orders.size());
            if (play.running) {
                ui->pushButtonPlaybackExper->setText("暂停");
            } else {
                ui->pushButtonPlaybackExper->setText("开始");
            }
        }
        ui->lineEditHisExperToken->setText(QString::number(i) + "#" + token);
    });

    QObject::connect(ui->treeViewRunningExperiments, &QTreeView::clicked, [this] (const QModelIndex &index) {
        auto token = getRootModelIndex(index).data().toString();
        ui->lineEditRunExperToken->setText(token);
    });
}

ExperimentDemandWidget::~ExperimentDemandWidget() {
    delete ui;
}

void ExperimentDemandWidget::flushAndShow() {
    auto syncGetFile = [this](const QString &path) -> QByteArray {
        QNetworkRequest resq(http_server_url_.toString() + "/" + path);
        QNetworkReply *reply = http_mgr_->get(resq);
        QEventLoop eventLoop;
        connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
        eventLoop.exec();
        if (reply->error() != QNetworkReply::NoError) {
            QMessageBox::warning(this, QString("数据下载错误(%1)").arg(path), reply->errorString());
            return {};
        }
        return reply->readAll();
    };

    asyncHttpPOST(
        http_mgr_,
        http_server_url_.toString() + "/get_running_simulations",
        {},
        [this](const QString &resp) {
            const auto json_bytes = resp.toUtf8();
            RD rd;
            backend_def::RunningExperimentsData backend_configs;
            try {
                xpack::json::decode(json_bytes.toStdString(), rd);
                rd.data.decode(backend_configs);
            }  catch (const std::exception &e) {
                qDebug() << e.what();
            }
            for (const auto &kv : backend_configs.experiments) {
                const auto &e = kv.second.init_config;
                qDebug() << e.auth_token.c_str() << e.view_config.ws_url.port;
                ports_of_running_[e.auth_token.c_str()] = e.view_config.ws_url.port;
            }
            auto json_doc = QJsonDocument::fromJson(json_bytes);
            running_experiments_data_json_model.loadJson(QJsonDocument(json_doc.object()["data"].toObject()["experiments"].toObject()).toJson());
        },
        [this](const QString &err) {
            QMessageBox::warning(this, "", err);
        });
    asyncHttpPOST(
        http_mgr_,
        http_server_url_.toString() + "/get_history_simulations",
        {},
        [this, syncGetFile](const QString &resp) {
            current_auth_tokens_of_his_expr.clear();
            auto json_doc = QJsonDocument::fromJson(resp.toUtf8());
            auto arr = json_doc.object()["data"].toArray();
            QJsonArray experiments;
            for (const auto &e : arr) {
                auto obj = e.toObject();
                // auto rendering_data = QString(syncGetFile(obj["rendering_orders_path"].toString()));
                // auto lines = rendering_data.split("\n");
                // QJsonArray rendering_orders;
                // for (const auto &line : lines) {
                //    if (line.isEmpty()) continue;
                //    RenderOrder order;
                //    order.fromJson(QJsonDocument::fromJson(line.toUtf8()).object());
                //    rendering_orders.push_back(order.stringify().c_str());
                //}
                QJsonObject hufri_json;
                hufri_json["ID"] = obj["user_id"].toString();
                hufri_json["开始时间"] = QDateTime::fromTime_t(obj["started_at"].toInt()).toString();
                hufri_json["结束时间"] = QDateTime::fromTime_t(obj["stoped_at"].toInt()).toString();
                hufri_json["配置"] = QJsonDocument::fromJson(syncGetFile(obj["config_path"].toString())).object();
                // hufri_json["录制"] = rendering_orders;
                current_auth_tokens_of_his_expr.push_back(hufri_json["ID"].toString());
                experiments.push_back(hufri_json);
            }

            history_experiments_data_json_model.loadJson(QJsonDocument(experiments).toJson());
        },
        [this](const QString &err) {
            QMessageBox::warning(this, "", err);
        });
}

void ExperimentDemandWidget::on_pushButtonPlaybackExper_clicked() {
    auto syncGetFile = [this](const QString &path) -> QByteArray {
        QNetworkRequest resq(http_server_url_.toString() + "/" + path);
        QNetworkReply *reply = http_mgr_->get(resq);
        QEventLoop eventLoop;
        connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
        eventLoop.exec();
        if (reply->error() != QNetworkReply::NoError) {
            QMessageBox::warning(this, QString("数据下载错误(%1)").arg(path), reply->errorString());
            return {};
        }
        return reply->readAll();
    };

    bool ok;
    int i = ui->lineEditHisExperToken->text().split('#')[0].toInt(&ok);
    if (!ok) return;
    auto token = current_auth_tokens_of_his_expr[i];
    if (playback_.count(token)) {
        auto &p = playback_[token];
        if (p.running) {
            p.flusher->start(p.delta_ms);
            p.running = false;
            ui->pushButtonPlaybackExper->setText("开始");
        } else {
            p.flusher->start(p.delta_ms);
            p.running = true;
            ui->pushButtonPlaybackExper->setText("暂停");
        }
        return;
    }

    // Launch new
    auto lines = syncGetFile(token + ".mess_show").split('\n');
    std::vector<std::vector<RenderOrder>> orders_by_freame;
    for (const auto &line : lines) {
        if (line.isEmpty()) continue;
        RenderOrder order;
        order.fromJson(QJsonDocument::fromJson(line).object());
        Q_ASSERT(order.args.size() >= 7);
        std::size_t fr = order.args[6];
        if (orders_by_freame.size() < fr + 1) orders_by_freame.resize(fr + 1);
        auto &curr_orders = orders_by_freame[fr];
        curr_orders.push_back(order);
    }

    auto &play = playback_[token];
    auto *tm = play.flusher = new QTimer(this);
    auto *w = (play.display_widget = new BaseDisplayWidget(this));
    play.token = token;
    play.render_orders = std::move(orders_by_freame);

    static const char * ress_name[] = {
        "background.png",
        "covered_grid.png",
        "nonblock_grid.jpg",
        "block_grid.gif",
        "car.gif",
        "future_route_grid.png"
    };

    int cnt = 0;
    for (auto f : ress_name) {
        QNetworkRequest resq(http_server_url_.toString() + "/" + f);
        QNetworkReply *reply = http_mgr_->get(resq);
        connect(reply, &QNetworkReply::finished, [this, w, reply, cnt, f]() {
           if (reply->error() != QNetworkReply::NoError) {
               QMessageBox::warning(this, "错误", reply->errorString());
           }
           QImage img;
           img.loadFromData(reply->readAll());
           w->addResource(cnt, img);
           qDebug() << "Get" << f;
        });
        ++cnt;
    }

    w->setProperty("type", "H"); // History
    w->setProperty("token", token);
    ui->tabWidgetDisplay->addTab(w, QString("回放(ID:%1)").arg(token));
    ui->tabWidgetDisplay->setCurrentWidget(w);
    ui->pushButtonPlaybackExper->setText("开始");
    play.running = false;
    connect(tm, &QTimer::timeout, [this, w, &play]() {
        const auto &o = play.render_orders;
        if (play.curr_index >= (int)o.size()) {
            w->update();
            return;
        }
        w->display(o[play.curr_index]);
        if (play.running) ++play.curr_index;
        bool ok;
        int i = ui->lineEditHisExperToken->text().split('#')[0].toInt(&ok); Q_ASSERT(ok);
        auto current_token = current_auth_tokens_of_his_expr[i];
        if (current_token == play.token) {
            ui->progressBarHistory->setValue((play.curr_index * ui->progressBarHistory->maximum()) / o.size());
        }
    });
}

void ExperimentDemandWidget::on_pushButtonPlaybackExperFlush_clicked() {
    flushAndShow();
}

void ExperimentDemandWidget::on_spinBox_valueChanged(int new_val) {
    bool ok;
    int i = ui->lineEditHisExperToken->text().split('#')[0].toInt(&ok);
    if (!ok) return;
    auto token = current_auth_tokens_of_his_expr[i];
    if (!playback_.count(token)) return;
    auto &p = playback_[token];
    p.delta_ms = 1000.f / new_val;
    if (p.running) p.flusher->start(p.delta_ms);
}

void ExperimentDemandWidget::on_tabWidgetDisplay_tabCloseRequested(int index) {
    QWidget *item_widget = ui->tabWidgetDisplay->widget(index);
    if (item_widget) {
        Q_ASSERT(item_widget->property("type").type() == QVariant::String);
        Q_ASSERT(item_widget->property("token").type() == QVariant::String);
        auto type = item_widget->property("type").toString();
        auto token = item_widget->property("token").toString();
        if (type == "R") {
            running_live_[token].clear();
            running_live_.erase(token);
        } else if (type == "H") {
            playback_[token].clear();
            playback_.erase(token);
        }
        item_widget->deleteLater();
    }
}

void ExperimentDemandWidget::on_pushButtonLiveExper_clicked() {
    auto token = ui->lineEditRunExperToken->text();
    if (!running_live_.count(token)) { // not exists
        auto &r = running_live_[token];
        r.display_widget = new MESSDisplayWidget;
        r.display_widget->start(
            QString("http://%1:%2")
                .arg(VIEW_COMPONENT_SERVER_IP)
                .arg(ports_of_running_[token]),
            QString("ws://%1:%2")
                .arg(VIEW_COMPONENT_SERVER_IP)
                .arg(ports_of_running_[token])
        );
        r.display_widget->setProperty("type", "R"); // Running: shit code
        r.display_widget->setProperty("token", token); // Token
        ui->tabWidgetDisplay->addTab(r.display_widget, QString("直播(ID:%1)").arg(token));
        ui->tabWidgetDisplay->setCurrentWidget(r.display_widget);
    } else {
        auto &r = running_live_[token];
        Q_ASSERT(r.display_widget);
        ui->tabWidgetDisplay->setCurrentWidget(r.display_widget);
    }
}

void ExperimentDemandWidget::on_pushButtonLiveExperFlush_clicked() {
    flushAndShow();
}

// Runtime Context
void ExperimentDemandWidget::PlaybackCtx::clear() {
    this->flusher->stop();
    this->flusher->deleteLater();
    this->display_widget->close();
}

void ExperimentDemandWidget::RuningLiveCtx::clear() {
    this->display_widget->close();
}
