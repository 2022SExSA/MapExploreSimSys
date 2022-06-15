#ifndef EXPERIMENTCONFIGWIDGET_H
#define EXPERIMENTCONFIGWIDGET_H

#include <QWidget>

#include "dll_export.h"
#include "config.h"

namespace Ui { class ExperimentConfigWidget; }

class MESSWIDGETS_EXPORT ExperimentConfigWidget : public QWidget {
    Q_OBJECT

public:
    explicit ExperimentConfigWidget(QWidget *parent = nullptr);
    ~ExperimentConfigWidget();

    QJsonValue getData();
private:
   void showData();
   const ExperimentConfig &getInteralData();
private slots:
    void on_pushButtonImportMap_clicked();
    void on_pushButton_clicked();
    void on_pushButtonExport_clicked();
    void on_pushButtonImport_clicked();

private:
    Ui::ExperimentConfigWidget *ui;
    ExperimentConfig data_;
    int current_editing_navi_index_{-1};
    int current_editing_car_index_{-1};
};

#endif // EXPERIMENTCONFIGWIDGET_H
