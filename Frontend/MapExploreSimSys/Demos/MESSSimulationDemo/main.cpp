#include <QApplication>

#include "MESSSimulationWidget.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    MESSSimulationWidget w("http://127.0.0.1:8080");
    w.show();
    return a.exec();
}
