#include "MESSWindow.h"

#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    MESSWindow w;
    w.show();
    return a.exec();
}