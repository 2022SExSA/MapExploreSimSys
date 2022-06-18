#include "StatisticsWidget.h"

#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    StatisticsWidget x("ws://127.0.0.1:8080");
    x.show();

    return a.exec();
}
