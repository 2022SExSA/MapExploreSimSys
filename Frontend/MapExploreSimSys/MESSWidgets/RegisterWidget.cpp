#include "RegisterWidget.h"
#include "ui_RegisterWidget.h"
#include "LoginWidget.h"
#include "da_utils.h"
#include <QString>
#include <QByteArray>
#include <QMessageBox>
#include <QLineEdit>
#include <QCryptographicHash>

RegisterWidget::RegisterWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RegisterWidget) {
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
                LoginWidget *w = new LoginWidget();
                RD rd;
                UserInfo u;
                u.id = ui->UserLineEdit->text().toStdString();
                u.name = ui->NameLineEdit->text().toStdString();
                u.password =
                        QCryptographicHash::hash( // 利用Sha1加密，一定程度上提高安全性
                            ui->PasswordLineEdit->text().toLatin1(),
                            QCryptographicHash::Sha1).toHex().toStdString();
                u.type = User;
                AddUser(u, rd);
                if (rd.code == 0) {
                    QMessageBox::information(this, "提示", QString("注册成功"));
                    this->close();
                    w->show();
                } else {
                    QMessageBox::warning(this, "注册失败", rd.msg.c_str());
                }
            }
        }
    }
}

void RegisterWidget::on_ExitPushbutton_clicked() {
    LoginWidget *w = new LoginWidget();
    w->show();
    this->close();
}
