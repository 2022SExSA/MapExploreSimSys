#include "MapDesigner.h"
#include "ui_MapDesigner.h"
#include <fstream>

MapDesigner::MapDesigner(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MapDesigner) {
    ui->setupUi(this);

}

MapDesigner::~MapDesigner() {
    delete ui;
}

void MapDesigner::paintEvent(QPaintEvent *Event) {
    QPainter painter(this);

    if(num_width != 0 && num_height != 0){
        single_grid_width = ui->map->width()/num_width;
        single_grid_height = ui->map->height()/num_height;
        grid_width = single_grid_width*num_width;
        grid_height = single_grid_height*num_height;
    }

    QBrush brush(Qt::SolidPattern);
    for(int i = 0; i < num_height; i++){
        for(int j = 0; j < num_width; j++){
            if(flag[i][j] == 0){
//                painter.setBrush(Qt::white);
                brush.setColor(Qt::white);
                painter.fillRect(ui->map->x() + single_grid_width*j,ui->map->y()+single_grid_height*i,
                                 single_grid_width, single_grid_height,
                                 brush);
            }
            else{
//                painter.setBrush(Qt::black);
                brush.setColor(Qt::black);
                painter.fillRect(ui->map->x()+single_grid_width*j,ui->map->y()+single_grid_height*i,
                                 single_grid_width, single_grid_height,
                                 brush);
            }
        }
    }
    painter.setBrush(Qt::NoBrush);

    for(int i = 0; i <= ui->width->value(); i++){
        painter.drawLine(ui->map->x() + single_grid_width * i, ui->map->y(),
                         ui->map->x() + single_grid_width * i, ui->map->y() + grid_height);
    }
    for(int i = 0; i <= ui->height->value(); i++){
        painter.drawLine(ui->map->x(), ui->map->y() + single_grid_height * i, ui->map->x() + grid_width,
                         ui->map->y() + single_grid_height * i);
    }
}

void MapDesigner::mousePressEvent(QMouseEvent *Event)
{
    if(Event->button() == Qt::LeftButton && ui->width->value() != 0 && ui->height->value() != 0){
        push_flag = true;
        initial_x = Event->x();
        initial_y = Event->y();
        int y_num = (Event->y() - ui->map->y())/single_grid_height;
        int x_num = (Event->x() - ui->map->x())/single_grid_width;
        if(x_num < num_width && y_num < num_height){
            if(flag[y_num][x_num] == 1){
                flag[y_num][x_num] = 0;
            }
            else{
                flag[y_num][x_num] = 1;
            }
        }
        update();
    }
}

void MapDesigner::mouseMoveEvent(QMouseEvent *e)
{
    if(push_flag){
        int x_num_begin = (initial_y-ui->map->y())/single_grid_height;
        int y_num_begin = (initial_x - ui->map->x())/single_grid_width;
        int x_num_end = (e->y() - ui->map->y())/single_grid_height;
        int y_num_end = (e->x() - ui->map->x())/single_grid_width;
        for(int i = x_num_begin; i <= x_num_end; i++){
            for(int j = y_num_begin; j <= y_num_end; j++){
                if(i < num_height && j < num_width){
                    if(flag[i][j] == 0){
                        flag[i][j] = 1;
                    }
                }
            }
        }
       update();
    }
}

void MapDesigner::mouseReleaseEvent(QMouseEvent *Event)
{
    if(Event->button() == Qt::LeftButton){
        push_flag = false;
    }
}


void MapDesigner::on_pushButton_clicked()
{
    if(ui->width->value() != 0 && ui->height->value() != 0){
    num_width = ui->width->value();
    num_height = ui->height->value();
    flag = std::vector<std::vector<int>> (ui->height->value(),std::vector<int>(ui->width->value(),0));
    }
    ui->map->update();
}

void MapDesigner::on_pushButton_2_clicked()
{
    QJsonObject size;
    size.insert("width", ui->width->value());
    size.insert("height", ui->height->value());
    std::string map_code;
    for(int i = 0; i < num_height; i++){
        for(int j = 0; j < num_width; j++){
            map_code.append(std::to_string(flag[i][j]));
        }
    }
    QJsonObject json;
    json.insert("size", size);
    json.insert("map_code", QJsonValue(map_code.c_str()));
    QJsonDocument doc(json);

    QString fileName = QFileDialog::getSaveFileName(this, "导出地图", "", tr("Curve TagName Files (*.json)"));
    std::ofstream ofs;
    ofs.open(fileName.toStdString(),std::ios::out);
    ofs<<doc.toJson().data();
}
