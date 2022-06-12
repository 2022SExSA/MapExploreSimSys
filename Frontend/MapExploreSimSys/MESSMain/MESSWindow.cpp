#include "MESSWindow.h"
#include "ui_MESSWindow.h"

MESSWindow::MESSWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MESSWindow) {
    ui->setupUi(this);
}

MESSWindow::~MESSWindow() {
    delete ui;
}

