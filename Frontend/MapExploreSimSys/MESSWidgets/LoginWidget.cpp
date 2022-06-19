#include "LoginWidget.h"
#include "ui_LoginWidget.h"
#include "RegisterWidget.h"
#include "da_utils.h"

#include <QMessageBox>

LoginWidget::LoginWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginWidget) {
    ui->setupUi(this);
    ui->UserRadioButton->setChecked(true);
}

LoginWidget::~LoginWidget() {
    delete ui;
}

void LoginWidget::setAfterLogin(const std::function<void (UserType, const std::string &)> & func) {
    after_login = func;
}

void LoginWidget::on_LoginPushButton_clicked() {
    QString id = ui->IDLineEdit->text();
    QString password = ui->PasswordLineEdit->text();
    bool userType = ui->UserRadioButton->isChecked();
    bool adminType = ui->UserRadioButton->isChecked(); Q_UNUSED(adminType);
    RD rd;
    AuthUser(id, password, userType ? User : Admin, rd);
    if (rd.code == 0) {
        if (after_login) after_login((userType ? User : Admin), rd.data["token"].GetString());
    } else {
        QMessageBox::warning(this, "登录失败", rd.msg.c_str());
    }
}

void LoginWidget::on_RegisterPushButton_clicked() {
    RegisterWidget *w = new RegisterWidget();
    w->show();
    this->close();
}
