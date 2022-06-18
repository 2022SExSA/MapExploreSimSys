#include "StatisticsConfigWidget.h"
#include "ui_StatisticsConfigWidget.h"
#include <QMessageBox>
#include <QDebug>
#include <QDateTime>


StatisticsConfigWidget::StatisticsConfigWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StatisticsConfigWidget)
{
    ui->setupUi(this);
    ui->widget->show();
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    //ui->tableWidget->setItem(0,1,new QTableWidgetItem("配置数据1"));
    connect(ui->tableWidget,&QTableWidget::doubleClicked,this,&StatisticsConfigWidget::doublecliched_row);
}

StatisticsConfigWidget::~StatisticsConfigWidget()
{
    delete ui;
}

void StatisticsConfigWidget::show_table(int num, QJsonObject json)
{
    ui->tableWidget->setItem(num,0,new QTableWidgetItem(QString("配置数据" + QString::number(num))));
    ui->tableWidget->setItem(num,2,new QTableWidgetItem(QString::number(json["car_components_config"].toArray().size())));
    int length = json["navigator_components_config"].toArray().size();
    QJsonArray navigator = json["navigator_components_config"].toArray();
    QString plugin = "";
    for(int i = 0; i < length; i++){
        QJsonObject x = navigator[i].toObject();
        plugin += x["plugin_id"].toString() + " ";
    }
     ui->tableWidget->setItem(num,1,new QTableWidgetItem(plugin));
     ui->tableWidget->update();
}

std::vector<QJsonObject> StatisticsConfigWidget::get_config_data()
{
    return config_data;
}

QMap<int, int> StatisticsConfigWidget::get_config_data_flag()
{
    return config_data_flag;
}

void StatisticsConfigWidget::on_pushButtonadd_clicked()
{

    if(ui->widget->checkData()){
        QDateTime time = QDateTime::currentDateTime();   //获取当前时间
        int timeT = time.toTime_t();
        int row = ui->tableWidget->rowCount();
        ui->tableWidget->setRowCount(row+1);
        QString config = ui->widget->getData().c_str();
        QJsonDocument jsonDocument = QJsonDocument::fromJson(config.toLocal8Bit().data());
        QJsonObject json = jsonDocument.object();
        json["auth_token"] = timeT;
        config_data_flag[timeT] = row;
//        qDebug() << row;
        config_data.push_back(json);
        show_table(row,json);
//        QJsonArray car = json["car_components_config"];

    }
    else{
        QMessageBox::warning(this,tr("警告"),tr("配置数据不全或错误"),QMessageBox::Ok,QMessageBox::Ok);
    }
}

void StatisticsConfigWidget::on_pushButtondelete_clicked()
{
    int rowIndex = ui->tableWidget->currentRow();

    if (rowIndex != -1){
        ui->tableWidget->removeRow(rowIndex);
        int count = 0;
        for(auto temp = config_data.begin(); temp != config_data.end(); temp++){
            if(count == rowIndex){
                config_data.erase(temp);
            }
            count++;
        }
        int row = ui->tableWidget->rowCount();
        for(int i = 0; i < row; i++){
            show_table(i,config_data[i]);
        }
    }
    else{
        QMessageBox::warning(this,tr("警告"),tr("请先选中需删除的行"),QMessageBox::Ok,QMessageBox::Ok);
    }
}

void StatisticsConfigWidget::on_pushButtonsave_clicked()
{
    int rowIndex = ui->tableWidget->currentRow();
    if (rowIndex != -1){
        if(ui->widget->checkData()){
            QString config = ui->widget->getData().c_str();
            QJsonDocument jsonDocument = QJsonDocument::fromJson(config.toLocal8Bit().data());
            QJsonObject json = jsonDocument.object();
            config_data[rowIndex] = json;
            show_table(rowIndex,json);
        }
        else{
            QMessageBox::warning(this,tr("警告"),tr("配置数据不全或错误"),QMessageBox::Ok,QMessageBox::Ok);
        }

    }
    else{
        QMessageBox::warning(this,tr("警告"),tr("请双击所需修改的配置信息"),QMessageBox::Ok,QMessageBox::Ok);
    }
}

void StatisticsConfigWidget::doublecliched_row()
{
    qDebug() << "@@";
    int rowIndex = ui->tableWidget->currentRow();
    if(rowIndex != -1){
        QJsonValue json = config_data[rowIndex];
        if(ui->widget->setDataFromJSON(json)){
            ui->widget->showData();
        }
        else{
            QMessageBox::warning(this,tr("警告"),tr("请双击所需修改的配置信息"),QMessageBox::Ok,QMessageBox::Ok);
        }
    }
}
