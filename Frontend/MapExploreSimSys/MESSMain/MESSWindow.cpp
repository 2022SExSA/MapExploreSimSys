#include "MESSWindow.h"
#include "ui_MESSWindow.h"
#include "MESSSimulationWidget.h"
#include "server_config.h"
#include "StatisticsWidget.h"
#include "MapDesigner.h"
#include "ExperimentDemandWidget.h"
#include <QDebug>


MESSWindow::MESSWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MESSWindow) {
    ui->setupUi(this);
    ui->tabWidget->removeTab(0);
    ui->tabWidget->removeTab(0);
    ui->tabWidget->setUsesScrollButtons(true);
    ui->tabWidget->setTabsClosable(true);//可以关闭tab
    ui->tabWidget->setMovable(true);//可以移动
}

MESSWindow::~MESSWindow() {
    delete ui;
}


void MESSWindow::on_pushButton_simulate_clicked()
{
     MESSSimulationWidget *w = new MESSSimulationWidget(HTTP_ROOT_URL);
     ui->tabWidget->addTab(w,"实验仿真");
     ui->tabWidget->setCurrentWidget(w);
}

void MESSWindow::on_pushButton_statistics_clicked()
{
    StatisticsWidget *w = new StatisticsWidget(WEBSOCKET_URL);
    ui->tabWidget->addTab(w,"实验数据统计");
    ui->tabWidget->setCurrentWidget(w);
}

void MESSWindow::on_pushButton_playback_clicked()
{
    ExperimentDemandWidget *w = new ExperimentDemandWidget(HTTP_ROOT_URL);
    ui->tabWidget->addTab(w,"实验回放");
    ui->tabWidget->setCurrentWidget(w);
}

void MESSWindow::on_pushButton_mapdesigner_clicked()
{
    MapDesigner *w = new MapDesigner();
    ui->tabWidget->addTab(w,"地图绘制");
    ui->tabWidget->setCurrentWidget(w);
    qDebug()<<"##";
}

void MESSWindow::on_tabWidget_tabCloseRequested(int index) {
    QWidget *pItemWidget = ui->tabWidget->widget(index);
    if (pItemWidget) {
        pItemWidget->close();
        pItemWidget->deleteLater();
    }
}

void MESSWindow::on_tabWidget_tabBarDoubleClicked(int index) {
    QWidget *itemWidget = ui->tabWidget->widget(index);
    if (itemWidget) {
        ui->tabWidget->removeTab(index);
        itemWidget->setParent(nullptr);
        itemWidget->show();
    }
}
