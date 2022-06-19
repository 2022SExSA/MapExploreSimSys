#ifndef USERMGRWIDGET_H
#define USERMGRWIDGET_H

#include <QWidget>

#include "dll_export.h"

QT_BEGIN_NAMESPACE
namespace Ui { class UserMgrWidget; }
QT_END_NAMESPACE

class MESSWIDGETS_EXPORT UserMgrWidget : public QWidget {
    Q_OBJECT

public:
    UserMgrWidget(QWidget *parent = nullptr);
    ~UserMgrWidget();
    void showUserInfo();

private slots:
    void on_SearchPushButton_3_clicked();
    void on_pushButton_3_clicked();
    void on_deletePushButton_3_clicked();
    void on_updatePushButton_3_clicked();
    void on_returnedPushButton_3_clicked();

private:
    Ui::UserMgrWidget *ui;
    bool Adding = true;
    bool Updating = true;
    int length;
    std::string token;
};
#endif // !USERMGRWIDGET_H
