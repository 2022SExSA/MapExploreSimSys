#include "StatisticsWidget.h"
#include "ui_StatisticsWidget.h"
#include "StatisticsConfigWidget.h"

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
    StatisticsConfigWidget *config_ui;
    ui->horizontalLayout->addWidget(config_ui);

}


StatisticsWidget::~StatisticsWidget()
{

    delete ui;

}

void StatisticsWidget::initial_data(std::vector<QJsonObject> json_list)
{
    line_list = std::vector<QLineSeries> (json_list.size());
    ui->tableWidget->setRowCount(json_list.size()+1);
    ui->tableWidget->setColumnCount(5);

    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "实验" << "地图尺寸" << "小车数量" << "执行时间" << "点亮区域");
    for(int i = 0; i < json_list.size(); i++){
        QJsonObject json = json_list[i];
        int current_data = line_name_node[json["auth_token"].toInt()];
        line_list[i].setName("实验"+QString::number(current_data));
//        line_name_node.insert(json["plugin_id"].toString(),i);
        ui->tableWidget->setItem(current_data,0,new QTableWidgetItem("实验"+QString::number(current_data)));
        ui->tableWidget->setItem(current_data,1,new QTableWidgetItem(json["car_components_comfig"].toArray().size()));
        QJsonObject map_size = json["map_config"].toObject();
        QString sized = map_size["width"].toString() + '*' + map_size["height"].toString();
        ui->tableWidget->setItem(current_data,2,new QTableWidgetItem(sized));
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
    int temp  = line_name_node[json["auth_token"].toInt()];
    line_list[temp].append(json["frame_cnt"].toDouble(),json["light_grid"].toInt());
    ui->tableWidget->setItem(temp+1, 3, new QTableWidgetItem(json["frame_cnt"].toDouble()));
    ui->tableWidget->setItem(temp+1, 4, new QTableWidgetItem(json["light_grid"].toInt()));
    paint_chart();
    update();
}

void StatisticsWidget::on_begin_button_clicked()
{
//    QJsonObject json;
//    json.insert("statistics_begin",true);
    ui->horizontalLayout->removeWidget(config_ui);
    paint_chart();
    ws_socket_->sendTextMessage("S");
}

void StatisticsWidget::on_stop_button_clicked()
{
//    QJsonObject json;
//    json.insert("statistics_begin",false);
    ws_socket_->sendTextMessage("P");
}

void StatisticsWidget::on_begin_button_2_clicked()
{
    config_data = config_ui->get_config_data();
    line_name_node = config_ui->get_config_data_flag();
    initial_data(config_data);
    QJsonArray config_send;
    for(auto &temp:config_data){
        config_send.append(temp);
    }
    QJsonDocument doc =  QJsonDocument(config_send);
    QString send_msg = "C" + doc.toJson(QJsonDocument::Compact);
    ws_socket_->sendTextMessage(send_msg);
}
