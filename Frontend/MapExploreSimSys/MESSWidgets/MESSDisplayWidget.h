#ifndef MESSDISPLAYWIDGET_H
#define MESSDISPLAYWIDGET_H

#include <QUrl>

#include "dll_export.h"
#include "BaseDisplayWidget.h"

class QWebSocket;
class QNetworkAccessManager;

class MESSWIDGETS_EXPORT MESSDisplayWidget : public BaseDisplayWidget {
    Q_OBJECT
public:
    explicit MESSDisplayWidget(
            const QString &http_url,
            const QString &ws_url,
            QWidget *parent = nullptr);

    void paintEvent(QPaintEvent *event) override;
public slots:
    void wsOnMessage(const QString &msg);

private:
    QUrl http_server_url_;
    QUrl ws_server_url_;
    QWebSocket *ws_socket_{nullptr};
    QNetworkAccessManager *http_mgr{nullptr};
};

#endif // MESSDISPLAYWIDGET_H
