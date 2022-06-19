#include "StatisticsWidget.h"
#include "ui_StatisticsWidget.h"
#include "StatisticsConfigWidget.h"
QT_CHARTS_USE_NAMESPACE
StatisticsWidget::StatisticsWidget(const QString &ws_url,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StatisticsWidget),
    ws_server_url_(ws_url) {
    ui->setupUi(this);
    ws_socket_ = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);
    ws_socket_->open(ws_server_url_);

    connect(ws_socket_, &QWebSocket::connected, [this]() {
        qDebug() << "Connnected";
        connect(ws_socket_, &QWebSocket::textMessageReceived, this, &StatisticsWidget::wsOnMessage);
    });
    ui->horizontalLayout->addWidget(config_ui = new StatisticsConfigWidget());
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    chartView = new QChartView(this);
    mChart = new QChart();
}

StatisticsWidget::~StatisticsWidget() {
    delete ui;

}

void StatisticsWidget::initial_data(std::vector<QJsonObject> json_list) {
    line_list = std::vector<QLineSeries> (json_list.size());
    //qDebug()<<json_list.size();
    ui->tableWidget->setRowCount(json_list.size());
    ui->tableWidget->setColumnCount(5);
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "实验" << "地图尺寸" << "小车数量" << "执行时间" << "点亮区域");
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
//    ui->tableWidget->horizontalHeader()->setSectionResizeMode()
    for(std::size_t i = 0; i < json_list.size(); i++){
        QJsonObject json = json_list[i];
        int current_data = line_name_node[json["auth_token"].toString()];
//        qDebug()<<current_data;
        line_list[i].setName("实验"+QString::number(current_data));
//        line_name_node.insert(json["plugin_id"].toString(),i);
        ui->tableWidget->setItem(current_data,0,new QTableWidgetItem("实验"+QString::number(current_data)));
        ui->tableWidget->setItem(current_data,2,new QTableWidgetItem(QString::number(json["car_components_comfig"].toArray().size())));
        QJsonObject map_size = json["map_config"].toObject();
        QString sized = map_size["width"].toString() + "*" + map_size["height"].toString();
        ui->tableWidget->setItem(current_data,1,new QTableWidgetItem(sized));
    }
}

void StatisticsWidget::paint_chart()
{

    //mChart->removeAllSeries();
    QValueAxis *m_axisX = new QValueAxis();
    QValueAxis *m_axisY = new QValueAxis();
    m_axisX->setMin(0);
    m_axisY->setMin(0);
    m_axisX->setMax(200);
    m_axisY->setMax(200);
    mChart->addAxis(m_axisX,Qt::AlignLeft);
    mChart->addAxis(m_axisY,Qt::AlignBottom);
    for(auto &temp: line_list){
        temp.setPointsVisible();
        mChart->addSeries(&temp);
        temp.attachAxis(m_axisX);
        temp.attachAxis(m_axisY);
    }
    mChart->createDefaultAxes();
    mChart->setAnimationOptions(QChart::SeriesAnimations);
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
//    qDebug()<<msg;
    int temp  = line_name_node[json["auth_token"].toString()];
    line_list[temp].append(QPointF(json["frame_cnt"].toInt(),json["light_grid"].toInt()));
    ui->tableWidget->setItem(temp, 3, new QTableWidgetItem(QString::number(json["frame_cnt"].toInt())));
    ui->tableWidget->setItem(temp, 4, new QTableWidgetItem(QString::number(json["light_grid"].toInt())));
//    ui->horizontalLayout->removeWidget(chartView);
//    chartView->repaint();
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
    //paint_chart();
}
