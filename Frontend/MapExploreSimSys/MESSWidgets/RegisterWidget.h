#ifndef REGISTERFORM_H
#define REGISTERFORM_H

#include <QWidget>

namespace Ui { class RegisterForm; }

class RegisterWidget : public QWidget {
    Q_OBJECT

public:
    explicit RegisterWidget(QWidget *parent = nullptr);
    ~RegisterWidget();

private slots:
    void on_ConfirmPushbutton_clicked();

private:
    Ui::RegisterForm *ui;
};

#endif // REGISTERFORM_H
