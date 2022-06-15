#include "StatisticsWidget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    StatisticsWidget x("123");
    //x.show();

    return a.exec();
}
