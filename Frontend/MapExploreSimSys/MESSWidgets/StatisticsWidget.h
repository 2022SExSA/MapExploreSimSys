#ifndef STATISTICSWIDGE_H
#define STATISTICSWIDGE_H

#include <QWidget>
#include "StatisticsConfigWidget.h"
#include <vector>
#include "dll_export.h"
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>
#include <QWebSocket>
#include <QtCharts>
#include <QChartView>
#include <QSplineSeries>
#include <QUrl>
#include <QMap>

namespace Ui {
class StatisticsWidget;
}

class MESSWIDGETS_EXPORT StatisticsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StatisticsWidget(const QString &ws_url,QWidget *parent = nullptr);
    ~StatisticsWidget();
    void initial_data(const std::vector<QJsonObject> json_list);
    void paint_chart();
public slots:
    void wsOnMessage(const QString &msg);
private slots:
    void on_begin_button_clicked();

    void on_stop_button_clicked();

    void on_begin_button_2_clicked();

private:
    Ui::StatisticsWidget *ui;
    QUrl ws_server_url_;
    QWebSocket *ws_socket_{nullptr};
    std::vector<QLineSeries> line_list;
    std::vector<QJsonObject> config_data;
    QMap<int,int> line_name_node;
    StatisticsConfigWidget *config_ui;
};

#endif // STATISTICSWIDGE_H
