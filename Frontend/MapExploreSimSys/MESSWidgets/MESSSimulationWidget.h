#ifndef MESSSIMULATIONWIDGET_H
#define MESSSIMULATIONWIDGET_H

#include <QUrl>
#include <QWidget>

#include "dll_export.h"

namespace Ui { class MESSSimulationWidget; }

class QNetworkAccessManager;

class MESSWIDGETS_EXPORT MESSSimulationWidget : public QWidget {
    Q_OBJECT

public:
    explicit MESSSimulationWidget(const QString &server_url, QWidget *parent = nullptr);
    ~MESSSimulationWidget();

private slots:
    void on_pushButtonStart_clicked();
    void on_pushButtonPause_clicked();
    void on_pushButtonStop_clicked();

private:
    Ui::MESSSimulationWidget *ui;
    QUrl http_server_url_;
    QNetworkAccessManager *http_mgr_{nullptr};
    enum ES { NOTHING, RUNNNING, PAUSING } exper_state_{NOTHING};
};

#endif // MESSSIMULATIONWIDGET_H
