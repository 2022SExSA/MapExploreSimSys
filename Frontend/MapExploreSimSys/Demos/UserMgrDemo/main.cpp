#include <QApplication>
#include <QDebug>

#include "da_utils.h"
#include "UserMgrWidget.h"
#include "LoginWidget.h"

int main(int argc, char *argv[]) {
    InitDAUtils("http://127.0.0.1:9999");
    QApplication a(argc, argv);
    LoginWidget lw;
    lw.show();
    lw.setAfterLogin([&lw](UserType type, const std::string &token) {
        if (type == Admin) {
            SetAuthToken(token);
            UserMgrWidget *w = new UserMgrWidget;
            w->show();
            lw.close();
        } else if (type == User) {
            // Do something
        }
    });
    return a.exec();
}
