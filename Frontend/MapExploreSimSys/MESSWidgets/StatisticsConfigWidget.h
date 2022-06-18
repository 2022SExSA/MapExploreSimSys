#ifndef STATISTICSCONFIGWIDGET_H
#define STATISTICSCONFIGWIDGET_H

#include <QWidget>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <vector>
#include <QMap>



namespace Ui {
class StatisticsConfigWidget;
}

class StatisticsConfigWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StatisticsConfigWidget(QWidget *parent = nullptr);
    ~StatisticsConfigWidget();
    void show_table(int num, QJsonObject json);

private slots:
    void on_pushButtonadd_clicked();

    void on_pushButtondelete_clicked();

    void on_pushButtonsave_clicked();
    void doublecliched_row();

private:
    Ui::StatisticsConfigWidget *ui;
    std::vector<QJsonObject> config_data;
//    QMap<QString,int> config_data_num;
};

#endif // STATISTICSCONFIGWIDGET_H
