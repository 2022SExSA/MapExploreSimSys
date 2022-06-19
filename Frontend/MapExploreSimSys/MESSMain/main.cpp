#include "da_utils.h"
#include "server_config.h"
#include "LoginWidget.h"
#include "UserMgrWidget.h"
#include "MESSWindow.h"

#include <QApplication>

int main(int argc, char *argv[]) {
    InitDAUtils(USER_SERVER_HTTP_ROOT_URL);
    QApplication a(argc, argv);
    LoginWidget lw;
    lw.setAfterLogin([&lw](UserType type, const std::string &token) {
        if (type == User) {
            MESSWindow *main_window = new MESSWindow;
            main_window->show();
            lw.close();
        } else if (type == Admin) {
            SetAuthToken(token);
            UserMgrWidget *umw = new UserMgrWidget;
            umw->show();
            lw.close();
        }
    });
    lw.show();
    return a.exec();
}
