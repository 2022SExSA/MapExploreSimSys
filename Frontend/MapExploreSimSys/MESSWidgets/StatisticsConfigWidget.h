#ifndef STATISTICSCONFIGWIDGET_H
#define STATISTICSCONFIGWIDGET_H

#include <QWidget>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <vector>
#include <QMap>

#include "dll_export.h"

namespace Ui {
class StatisticsConfigWidget;
}

class MESSWIDGETS_EXPORT StatisticsConfigWidget : public QWidget {
    Q_OBJECT

public:
    explicit StatisticsConfigWidget(QWidget *parent = nullptr);
    ~StatisticsConfigWidget();
    void show_table(int num, QJsonObject json);
    std::vector<QJsonObject> get_config_data();
    QMap<QString,int> get_config_data_flag();

private slots:
    void on_pushButtonadd_clicked();

    void on_pushButtondelete_clicked();

    void on_pushButtonsave_clicked();
    void doublecliched_row();

private:
    Ui::StatisticsConfigWidget *ui;
    std::vector<QJsonObject> config_data;
    QMap<QString,int> config_data_flag;
    int count = 0;
};

#endif // STATISTICSCONFIGWIDGET_H
