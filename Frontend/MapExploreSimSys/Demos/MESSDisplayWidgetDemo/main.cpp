#include <QApplication>

#include "MESSDisplayWidget.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    MESSDisplayWidget w("http://127.0.0.1:9876", "ws://127.0.0.1:9876");
    w.setWindowTitle("Demo:MESSDisplayWidget");
    w.show();

    return a.exec();
}
