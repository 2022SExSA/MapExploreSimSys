#include <QApplication>

#include "ExperimentDemandWidget.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    ExperimentDemandWidget w("http://127.0.0.1:8080");
    w.show();
    return a.exec();
}
