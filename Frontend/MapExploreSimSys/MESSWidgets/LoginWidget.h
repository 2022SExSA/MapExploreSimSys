#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class LoginWidget; }
QT_END_NAMESPACE

class LoginWidget : public QWidget {
    Q_OBJECT

public:
    LoginWidget(QWidget *parent = nullptr);
    ~LoginWidget();

    void setAfterLogin(const std::function<void()> &func);

private slots:
    void on_LoginPushButton_clicked();
    void on_RegisterPushButton_clicked();

private:
    Ui::LoginWidget *ui;
    std::function<void()> after_login{nullptr};
};
#endif // WIDGET_H
