#include "LoginWidget.h"
#include "ui_LoginWidget.h"
#include "RegisterWidget.h"
#include "da_utils.h"

#include <QMessageBox>
#include <QCryptographicHash>

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
    QString primal_password = ui->PasswordLineEdit->text();
    // 对密码进行加密处理(使用Sha1, 一定程度上提高安全性)
    QByteArray bytes_password = QCryptographicHash::hash(primal_password.toLatin1(), QCryptographicHash::Sha1);
    QString password = bytes_password.toHex();
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
