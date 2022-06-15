#include "StatisticsWidget.h"
#include "ui_StatisticsWidget.h"

StatisticsWidget::StatisticsWidget(const QString &ws_url,QWidget *parent) :
    QWidget(parent),
    ws_server_url_(ws_url),
    ui(new Ui::StatisticsWidget){
      ui->setupUi(this);
      ws_socket_ = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);
      ws_socket_->open(ws_server_url_);

      connect(ws_socket_, &QWebSocket::connected, [this]() {
          qDebug() << "Connnected";
          connect(ws_socket_, &QWebSocket::textMessageReceived, this, &StatisticsWidget::wsOnMessage);
      });
}


StatisticsWidget::~StatisticsWidget()
{

    delete ui;

}

void StatisticsWidget::initial_data(std::vector<QJsonValue> json_list)
{
    line_list = std::vector<QLineSeries> (json_list.size());
    ui->tableWidget->setRowCount(json_list.size()+1);
    ui->tableWidget->setColumnCount(5);

    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "算法" << "地图尺寸" << "小车数量" << "执行时间" << "点亮区域");
    for(int i = 0; i < json_list.size(); i++){
        QJsonObject json = json_list[i].toObject();
        line_list[i].setName(json["plugin_id"].toString());
        line_name_node.insert(json["plugin_id"].toString(),i);
        ui->tableWidget->setItem(i+1,0,new QTableWidgetItem(json["plugin_id"].toString()));
        ui->tableWidget->setItem(i+1,1,new QTableWidgetItem(json["car_components_comfig"].toArray().size()));
        QJsonObject map_size = json["map_config"].toObject();
        QString sized = map_size["width"].toString() + '*' + map_size["height"].toString();
        ui->tableWidget->setItem(i+1,2,new QTableWidgetItem(sized));
    }

}

void StatisticsWidget::paint_chart()
{
    QChartView *chartView = new QChartView(this);
    QChart *mChart = new QChart();
    for(auto &temp: line_list){
        mChart->addSeries(&temp);
    }
    mChart->createDefaultAxes();
    mChart->setTheme(QChart::ChartThemeDark);
    mChart->setTitle("统计图表");
    chartView->setChart(mChart);
    chartView->setRenderHints(QPainter::Antialiasing);
    ui->horizontalLayout->addWidget(chartView);
}

void StatisticsWidget::wsOnMessage(const QString &msg)
{
    QJsonDocument jsonDocument = QJsonDocument::fromJson(msg.toLocal8Bit().data());
    QJsonObject json = jsonDocument.object();
    int temp  = line_name_node[json["plugin_id"].toString()];
    line_list[temp].append(json["time"].toDouble(),json["light_grid"].toInt());
    ui->tableWidget->setItem(temp+1, 3, new QTableWidgetItem(json["time"].toDouble()));
    ui->tableWidget->setItem(temp+1, 4, new QTableWidgetItem(json["light_grid"].toInt()));
    update();
}

void StatisticsWidget::on_begin_button_clicked()
{
    QJsonObject json;
    json.insert("statistics_begin",true);
    ws_socket_->sendTextMessage(QString(QJsonDocument(json).toJson()));
}

void StatisticsWidget::on_stop_button_clicked()
{
    QJsonObject json;
    json.insert("statistics_begin",false);
    ws_socket_->sendTextMessage(QString(QJsonDocument(json).toJson()));
}
