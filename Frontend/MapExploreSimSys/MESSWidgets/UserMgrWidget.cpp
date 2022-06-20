#include "UserMgrWidget.h"
#include "ui_UserMgrWidget.h"
#include "da_utils.h"
#include <QMessageBox>
#include <QVector>
#include <QStringList>
#include <QDebug>

UserMgrWidget::UserMgrWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::UserMgrWidget) {
    ui->setupUi(this);
    QStringList text;
    text.append("用户id");
    text.append("用户姓名");
    text.append("类别");
    ui->TypeComboBox->addItems(text); qDebug() << __FILE__ << __LINE__;
    showUserInfo(); qDebug() << __FILE__ << __LINE__;
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows); //设置选择行为，以行为单位
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection); //设置选择模式，选择单行
}

UserMgrWidget::~UserMgrWidget() {
    delete ui;
}

void UserMgrWidget::showUserInfo() {
    QVector<UserInfo> userInfos;
    RD rd;
    userInfos = GetAllUserInfo(rd);
    if (rd.code != 0) {
        QMessageBox::warning(this, "错误", "获取用户信息失败");
        return;
    }
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setColumnCount(4);
    ui->tableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem("id"));
    ui->tableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem("密码"));
    ui->tableWidget->setHorizontalHeaderItem(2, new QTableWidgetItem("姓名"));
    ui->tableWidget->setHorizontalHeaderItem(3, new QTableWidgetItem("类型"));
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    for(int i = 0;i<userInfos.size();i++) {
        ui->tableWidget->insertRow(i);
        ui->tableWidget->setItem(i, 0, new QTableWidgetItem(userInfos[i].id.c_str()));
        ui->tableWidget->setItem(i, 1, new QTableWidgetItem(userInfos[i].password.c_str()));
        ui->tableWidget->setItem(i, 2, new QTableWidgetItem(userInfos[i].name.c_str()));
        ui->tableWidget->setItem(i, 3, new QTableWidgetItem(QString::number(userInfos[i].type)));
        ui->tableWidget->item(i,0)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        ui->tableWidget->item(i,1)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        ui->tableWidget->item(i,2)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        ui->tableWidget->item(i,3)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    }
}

void UserMgrWidget::on_SearchPushButton_3_clicked() {
    QString text = ui->TextLineEdit_3->text();
    QString Column = ui->TypeComboBox->currentText();

    RD rd;
    std::map<QString, QString> TABLE = {
        {"用户id", "id"},
        {"用户姓名", "name"},
        {"类别", "type"}
    };
    QVector<UserInfo> userInfos = GetUserInfoWithFilter(text.trimmed(), TABLE[Column].trimmed(), rd);
    if (rd.code != 0) {
        QMessageBox::warning(this, "错误", QString("获取用户信息失败: ") + rd.msg.c_str());
        return;
    }
    ui->tableWidget->clear();
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setColumnCount(4);
    ui->tableWidget->setHorizontalHeaderItem(0,new QTableWidgetItem("id"));
    ui->tableWidget->setHorizontalHeaderItem(1,new QTableWidgetItem("密码"));
    ui->tableWidget->setHorizontalHeaderItem(2,new QTableWidgetItem("姓名"));
    ui->tableWidget->setHorizontalHeaderItem(3,new QTableWidgetItem("类型"));

    for(int i = 0;i<userInfos.size();i++) {
        ui->tableWidget->insertRow(i);
        ui->tableWidget->setItem(i, 0, new QTableWidgetItem(userInfos[i].id.c_str()));
        ui->tableWidget->setItem(i, 1, new QTableWidgetItem(userInfos[i].password.c_str()));
        ui->tableWidget->setItem(i, 2, new QTableWidgetItem(userInfos[i].name.c_str()));
        ui->tableWidget->setItem(i, 3, new QTableWidgetItem(QString::number(userInfos[i].type)));
    }
}


void UserMgrWidget::on_pushButton_3_clicked()
{
    ui->deletePushButton_3->hide();
    ui->updatePushButton_3->hide();
    if(Adding == true)
    {
        ui->SearchPushButton_3->setDisabled(true);
        Adding = false;
        ui->pushButton_3->setText("保存");
        ui->tableWidget->insertRow(ui->tableWidget->rowCount());

        ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 0, new QTableWidgetItem(""));
        ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 1, new QTableWidgetItem(""));
        ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 2, new QTableWidgetItem(""));
        ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 3, new QTableWidgetItem(""));

    } else {
        QMessageBox::StandardButton standardButton = QMessageBox::question(this,"提示","是否保存？");
        if(standardButton == QMessageBox::Yes) {
            UserInfo userInfo;
            userInfo.id = ui->tableWidget->item(ui->tableWidget->rowCount()-1,0)->text().toStdString();
            userInfo.password = ui->tableWidget->item(ui->tableWidget->rowCount()-1,1)->text().toStdString();
            userInfo.name = ui->tableWidget->item(ui->tableWidget->rowCount()-1,2)->text().toStdString();
            userInfo.type = ui->tableWidget->item(ui->tableWidget->rowCount()-1,3)->text().toInt();

            RD rd;
            InsertUser(userInfo, rd);
            if(rd.code == 0) {
                ui->SearchPushButton_3->setDisabled(!true);
                ui->tableWidget->item(ui->tableWidget->rowCount()-1,0)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
                ui->tableWidget->item(ui->tableWidget->rowCount()-1,1)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
                ui->tableWidget->item(ui->tableWidget->rowCount()-1,2)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
                ui->tableWidget->item(ui->tableWidget->rowCount()-1,3)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

                QMessageBox::information(this,"提示","保存成功");
                ui->pushButton_3->setText("增加");
                Adding = true;
                ui->deletePushButton_3->show();
                ui->updatePushButton_3->show();
            }
            else {
                QMessageBox::information(this, "提示", QString("保存失败: ") + rd.msg.c_str());
            }
        }
    }
}


void UserMgrWidget::on_deletePushButton_3_clicked(){
    QMessageBox::StandardButton standardButton = QMessageBox::question(this,"提示","是否删除？");
    if(standardButton == QMessageBox::Yes)
    {
        int length = ui->tableWidget->currentRow();
        if(length >= 0&&length < ui->tableWidget->rowCount() )
        {
            RD rd;
            DeleteUser(ui->tableWidget->item(ui->tableWidget->currentRow(),0)->text(), rd);
            if(rd.code == 0)
            {
                ui->tableWidget->removeRow(ui->tableWidget->currentRow());
                QMessageBox::information(this,"提示","删除成功");
            }
            else
            {
                QMessageBox::warning(this,"提示",QString("删除失败: ") + rd.msg.c_str());
            }
        }
        else
        {
            QMessageBox::information(this,"提示","请选择删除信息");
        }
    }
}

void UserMgrWidget::on_updatePushButton_3_clicked()
{
    if(Updating == true) {

        int length = ui->tableWidget->currentRow();
        this->length = length;
        if(length >= 0 && length < ui->tableWidget->rowCount())
        {
            Updating = false;
            ui->deletePushButton_3->hide();
            ui->pushButton_3->hide();
            ui->updatePushButton_3->setText("保存");
            ui->tableWidget->item(length,0)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
            ui->tableWidget->item(length,1)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
            ui->tableWidget->item(length,2)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
            ui->tableWidget->item(length,3)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
        }
        else
        {
            QMessageBox::warning(this,"提示","您还没有选择修改信息");
        }
    }
    else
    {
        RD rd;
        UserInfo userInfo;
        userInfo.id = ui->tableWidget->item(this->length,0)->text().toStdString();
        userInfo.password = ui->tableWidget->item(this->length,1)->text().toStdString();
        userInfo.name = ui->tableWidget->item(this->length,2)->text().toStdString();
        userInfo.type = ui->tableWidget->item(this->length,3)->text().toInt();

        UpdatingUser(userInfo, rd);
        if(rd.code == 0)
        {
            QMessageBox::information(this,"提示","保存修改成功");
            ui->updatePushButton_3->setText("修改");
            Updating = true;
            ui->deletePushButton_3->show();
            ui->pushButton_3->show();
            ui->tableWidget->item(length,0)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            ui->tableWidget->item(length,1)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            ui->tableWidget->item(length,2)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            ui->tableWidget->item(length,3)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

        }
        else
        {
            QMessageBox::warning(this,"提示","保存失败");
        }

    }
}

void UserMgrWidget::on_returnedPushButton_3_clicked() {
    this->close();//整合时需更改
}

