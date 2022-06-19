#include <QApplication>
#include <QDebug>

#include "da_utils.h"
#include "LoginWidget.h"

int main(int argc, char *argv[]) {
    InitDAUtils("http://127.0.0.1:9999");
    QApplication a(argc, argv);
    LoginWidget w;
    w.show();
    return a.exec();
}
