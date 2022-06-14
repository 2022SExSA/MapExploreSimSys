#include "MESSSimulationWidget.h"
#include "ui_MESSSimulationWidget.h"

MESSSimulationWidget::MESSSimulationWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MESSSimulationWidget) {
    ui->setupUi(this);
}

MESSSimulationWidget::~MESSSimulationWidget() {
    delete ui;
}
