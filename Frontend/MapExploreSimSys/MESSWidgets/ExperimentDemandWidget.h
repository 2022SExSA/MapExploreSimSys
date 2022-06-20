#ifndef EXPERIMENTDEMANDWIDGET_H
#define EXPERIMENTDEMANDWIDGET_H

#include <QUrl>
#include <QWidget>

#include "BaseDisplayWidget.h"
#include "MESSDisplayWidget.h"
#include "dll_export.h"
#include "JsonModel.h"

class QNetworkAccessManager;

namespace Ui { class ExperimentDemandWidget; }

class MESSWIDGETS_EXPORT ExperimentDemandWidget : public QWidget {
    Q_OBJECT

public:
    explicit ExperimentDemandWidget(const QString &server_url, QWidget *parent = nullptr);
    ~ExperimentDemandWidget();

    void flushAndShow();
private slots:
    void on_pushButtonPlaybackExper_clicked();
    void on_pushButtonPlaybackExperFlush_clicked();
    void on_spinBox_valueChanged(int arg1);
    void on_tabWidgetDisplay_tabCloseRequested(int index);
    void on_pushButtonLiveExper_clicked();
    void on_pushButtonLiveExperFlush_clicked();

private:
    struct PlaybackCtx {
        QTimer* flusher{nullptr};
        BaseDisplayWidget *display_widget{nullptr};
        int curr_index = 0;
        bool running = true;
        int delta_ms = 1000;
        QString token;
        std::vector<std::vector<RenderOrder>> render_orders;
    };

    struct RuningLiveCtx {
        QString token;
        MESSDisplayWidget *display_widget{nullptr};
    };

    Ui::ExperimentDemandWidget *ui;
    QUrl http_server_url_;
    QNetworkAccessManager *http_mgr_{nullptr};
    JsonModel running_experiments_data_json_model;
    JsonModel history_experiments_data_json_model;
    std::vector<QString> current_auth_tokens_of_his_expr;
    std::map<QString, PlaybackCtx> playback_; // runtime
    std::map<QString, RuningLiveCtx> running_live_; // runtime
    std::map<QString, int> ports_of_running_;
};

#endif // EXPERIMENTDEMANDWIDGET_H
