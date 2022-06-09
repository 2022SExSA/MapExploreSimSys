#ifndef MAPDESIGNER_H
#define MAPDESIGNER_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <vector>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <string>
#include <QString>
#include <QFileDialog>
QT_BEGIN_NAMESPACE
namespace Ui { class MapDesigner; }
QT_END_NAMESPACE

class MapDesigner : public QWidget
{
    Q_OBJECT

public:
    MapDesigner(QWidget *parent = nullptr);
    ~MapDesigner();
    void paintEvent(QPaintEvent *Event);
    void mousePressEvent(QMouseEvent *Event);
    void mouseMoveEvent(QMouseEvent *Event);
    void mouseReleaseEvent(QMouseEvent *Event);
private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();

private:
    Ui::MapDesigner *ui;
    int num_width = 0;
    int num_height = 0;
    int single_grid_width = 0;
    int single_grid_height = 0;
    int grid_width = 0;
    int grid_height = 0;
    std::vector<std::vector<int>> flag;
    bool push_flag = false;
    int initial_x = 0;
    int initial_y = 0;
};
#endif // MAPDESIGNER_H
