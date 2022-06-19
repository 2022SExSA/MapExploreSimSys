#ifndef REGISTERFORM_H
#define REGISTERFORM_H

#include <QWidget>

namespace Ui { class RegisterWidget; }

class RegisterWidget : public QWidget {
    Q_OBJECT

public:
    explicit RegisterWidget(QWidget *parent = nullptr);
    ~RegisterWidget();

private slots:
    void on_ConfirmPushbutton_clicked();

    void on_ExitPushbutton_clicked();

private:
    Ui::RegisterWidget *ui;
};

#endif // REGISTERFORM_H
