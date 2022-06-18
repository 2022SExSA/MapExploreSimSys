#include "LoginWidget.h"
#include "ui_LoginWidget.h"
#include "RegisterWidget.h"
#include "da_utils.h"

#include <QMessageBox>

LoginWidget::LoginWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginWidget) {
    ui->setupUi(this);
}

LoginWidget::~LoginWidget() {
    delete ui;
}

void LoginWidget::setAfterLogin(const std::function<void ()> & func) {
    after_login = func;
}

void LoginWidget::on_LoginPushButton_clicked() {
    QString id = ui->IDLineEdit->text();
    QString password = ui->PasswordLineEdit->text();
    bool userType = ui->UserRadioButton->isChecked();
    bool adminType = ui->UserRadioButton->isChecked(); Q_UNUSED(adminType);
    LoginState state = AuthUser(id, password, userType ? User : Admin);
    // FIXME: 判断state提示用户什么错误
    // 1. "用户名不存在"
    // 2. "密码错误"
    // 3. "用户类型不匹配"
    // 4. "登录成功"
    if (state == UserNameNotFound) {
        QMessageBox::warning(this, "登录失败", "用户不存在");
        return;
    } else if (state == UserNamePasswordNotMatched) {
        QMessageBox::warning(this, "登录失败", "密码错误");
        return;
    } else if (state == UserTypeNotMatched) {
        QMessageBox::warning(this, "登录失败", "用户类型不匹配");
        return;
    }
    if (state == LoginSuccess) {
        if (after_login) after_login();
    }
}

void LoginWidget::on_RegisterPushButton_clicked() {
    RegisterWidget *w = new RegisterWidget();
    w->show();
    this->close();
}
