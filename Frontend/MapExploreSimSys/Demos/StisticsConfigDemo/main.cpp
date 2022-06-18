#include "StatisticsConfigWidget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    StatisticsConfigWidget w;
    w.show();

    return a.exec();
}
