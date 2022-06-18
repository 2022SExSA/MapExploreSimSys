#include "RegisterWidget.h"
#include "ui_RegisterWidget.h"
#include "LoginWidget.h"
#include "da_utils.h"
#include <QMessageBox>
#include <QLineEdit>

RegisterWidget::RegisterWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RegisterForm) {
    ui->setupUi(this);
}

RegisterWidget::~RegisterWidget() {
    delete ui;
}

void RegisterWidget::on_ConfirmPushbutton_clicked() {
    QString id = ui->UserLineEdit->text();
    QString password = ui->PasswordLineEdit->text();
    QString password_con = ui->ConfirmLineEdit->text();

    if(id.length() < 8) {
        QMessageBox::warning(this,"提示","用户名长度小于8位");

    } else if(id.length() > 16) {
        QMessageBox::warning(this,"提示","用户名长度大于16位");
    } else {
        if(password.length() < 8) {
            QMessageBox::warning(this,"提示","密码长度小于8位");
        }
        else if(password.length() > 64) {
            QMessageBox::warning(this,"提示","密码长度大于64位");
        }
        else {
            if (password != password_con) {
                QMessageBox::warning(this,"提示","密码不一致");
            } else {
                //判断id是否重复
                LoginWidget *w = new LoginWidget();
                QMessageBox::information(this,"提示",QString("注册成功"));
                this->close();
                w->show();
            }
        }
    }
}
