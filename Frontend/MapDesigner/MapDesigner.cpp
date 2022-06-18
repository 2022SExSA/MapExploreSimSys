#include "MapDesigner.h"
#include "ui_MapDesigner.h"
#include <fstream>
#include <QDebug>

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
            if(map_flag[i][j] == 0){
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
    if(ui->width->value() != 0 && ui->height->value() != 0 && Event->x()>0 && Event->y()>0){
        if(Event->button() == Qt::LeftButton){
            push_flag = true;
            push_status = false;
        }
        else if(Event->button() == Qt::RightButton){
            push_flag = true;
            push_status = true;
        }
        save_flag = false;
        initial_x = Event->x();
        initial_y = Event->y();
        int y_num = (Event->y() - ui->map->y())/single_grid_height;
        int x_num = (Event->x() - ui->map->x())/single_grid_width;
        if(x_num < num_width && y_num < num_height){
            if(map_flag[y_num][x_num] == 1){
                map_flag[y_num][x_num] = 0;
            }
            else{
                map_flag[y_num][x_num] = 1;
            }
        }
        update();
    }
}

void MapDesigner::mouseMoveEvent(QMouseEvent *e)
{
    if(push_flag && e->x()>0 && e->y()>0){
        int x_num_begin;
        int y_num_begin;
        int x_num_end;
        int y_num_end;
        if(e->y() >= initial_y && e->x() >= initial_x){
            x_num_begin = (initial_y - ui->map->y())/single_grid_height;
            y_num_begin = (initial_x - ui->map->x())/single_grid_width;
            x_num_end = (e->y() - ui->map->y())/single_grid_height;
            y_num_end = (e->x() - ui->map->x())/single_grid_width;
        }
        else if(e->y() <= initial_y && e->x() >= initial_x){
             x_num_begin = (e->y() - ui->map->y())/single_grid_height;
             y_num_begin = (initial_x - ui->map->x())/single_grid_width;
             x_num_end = (initial_y - ui->map->y())/single_grid_height;
             y_num_end = (e->x() - ui->map->x())/single_grid_width;
        }
        else if(e->y() >= initial_y && e->x() <= initial_x){
            x_num_begin = (initial_y - ui->map->y())/single_grid_height;
            y_num_begin = (e->x() - ui->map->x())/single_grid_width;
            x_num_end = (e->y() - ui->map->y())/single_grid_height;
            y_num_end = (initial_x - ui->map->x())/single_grid_width;

        }
        else {
            x_num_begin = (e->y() - ui->map->y())/single_grid_height;
            y_num_begin = (e->x() - ui->map->x())/single_grid_width;
            x_num_end = (initial_y - ui->map->y())/single_grid_height;
            y_num_end = (initial_x - ui->map->x())/single_grid_width;

        }
        for(int i = x_num_begin; i <= x_num_end; i++){
            for(int j = y_num_begin; j <= y_num_end; j++){
                if(i < num_height && j < num_width){
                    if(!push_status){
                        if(map_flag[i][j] == 0){
                            map_flag[i][j] = 1;
                        }
                    }
                    else {
                        if(map_flag[i][j] == 1){
                            map_flag[i][j] = 0;
                        }
                    }
                }
            }
        }
       ui->map->update();
    }
}

void MapDesigner::mouseReleaseEvent(QMouseEvent *Event)
{
    if(Event->button() == Qt::LeftButton){
        push_flag = false;
    }
}

void MapDesigner::closeEvent(QCloseEvent *event)
{
    if(!save_flag){
        QMessageBox::StandardButton result = QMessageBox::warning(this, "保存", "是否保存更改？",
                              QMessageBox::Yes|QMessageBox::No |QMessageBox::Cancel,
                              QMessageBox::Yes);
        if (result == QMessageBox::Yes){
            if(save()){
                event->accept();
                return;
            }
            event->ignore();
        }
        else if(result == QMessageBox::No){
            event->accept();
        }
        else if(result == QMessageBox::Cancel){
            event->ignore();
        }
    }
}

bool MapDesigner::save()
{
    QJsonObject size;
    size.insert("width", ui->width->value());
    size.insert("height", ui->height->value());
    std::string map_code;
    for(int i = 0; i < num_height; i++){
        for(int j = 0; j < num_width; j++){
            map_code.append(std::to_string(map_flag[i][j]));
        }
    }
    QJsonObject json;
    json.insert("size", size);
    json.insert("map_code", QJsonValue(map_code.c_str()));
    QJsonDocument doc(json);
    if(!be_saved){
        QString fileName = QFileDialog::getSaveFileName(this, "导出地图", "", tr("Curve TagName Files (*.json)"));
        file_path = fileName;
        if(!fileName.isNull()){
            std::ofstream ofs;
            ofs.open(fileName.toStdString(),std::ios::out);
            ofs<<doc.toJson().data();
            return true;
        }
    }
    else{
        std::ofstream ofs;
        ofs.open(file_path.toStdString(),std::ios::out);
        ofs<<doc.toJson().data();
        return true;
    }
    return false;
}

bool MapDesigner::open_file()
{
    QString fileName = QFileDialog::getOpenFileName(
            this,
            tr("open a file."),
            "",
            tr("json_files(*.json)"));
    if(fileName.isEmpty()){
        QMessageBox::warning(this,tr("waring"),tr("打开文件失败"),QMessageBox::Ok,QMessageBox::Ok);
        return false;
    }
    else{
        file_path = fileName;
        std::ifstream ifs;
        ifs.open(fileName.toStdString(),std::ios::in);
        std::string in;
        while(true) {
            std::string tmp;
            std::getline(ifs, tmp);
            //qDebug() << tmp.c_str();
            if (tmp.empty()) break;
            in.append(tmp);
        }

        QJsonDocument jsonDocument = QJsonDocument::fromJson(QString::fromStdString(in).toLocal8Bit().data());
        QJsonObject json = jsonDocument.object();
        QJsonObject size = json["size"].toObject();
        ui->width->setValue(size["width"].toInt());
        ui->height->setValue(size["height"].toInt());
        num_width = ui->width->value();
        num_height = ui->height->value();
        map_flag = std::vector<std::vector<int>> (num_height,std::vector<int>(num_width));
        QString map_code = json["map_code"].toString();
        for(int i = 0; i < num_height; i++){
            for(int j = 0; j < num_width; j++){
                map_flag[i][j] = map_code[i*num_width+j].toLatin1() - '0';
            }
        }
        return true;
    }
    update();
    return false;
}


void MapDesigner::on_pushButton_clicked()
{
    if(ui->width->value() != 0 && ui->height->value() != 0){
        num_width = ui->width->value();
        num_height = ui->height->value();
        map_flag = std::vector<std::vector<int>> (ui->height->value(),std::vector<int>(ui->width->value(),0));
    }
    else if(ui->width->value() == 0 && ui->height->value() == 0){
        QMessageBox::warning(this,tr("warning"),tr("输入数据不可为0"),QMessageBox::Ok,QMessageBox::Ok);
    }
    else if(ui->height->value() == 0){
        QMessageBox::warning(this,tr("warning"),tr("长格数不可为0"),QMessageBox::Ok,QMessageBox::Ok);
    }
    else if(ui->width->value() == 0){
        QMessageBox::warning(this,tr("warning"),tr("宽格数不可为0"),QMessageBox::Ok,QMessageBox::Ok);
    }
    save_flag = false;
    ui->map->update();
}

void MapDesigner::on_pushButton_2_clicked()
{
    if(save()){
        save_flag = true;
        be_saved = true;
    }
}

void MapDesigner::on_pushButton_3_clicked()
{
    be_saved = false;
    if(save()){
        save_flag = true;
        be_saved = true;
    }
    else{
        be_saved = true;
    }
}

void MapDesigner::on_pushButton_4_clicked()
{
    if(open_file()){
        save_flag = true;
        be_saved = true;
    }

}
