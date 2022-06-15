#include "ExperimentConfigWidget.h"

#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    ExperimentConfigWidget w;
    w.show();
    return a.exec();
}
