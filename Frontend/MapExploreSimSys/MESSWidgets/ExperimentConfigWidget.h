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

    bool checkData();
    bool setDataFromJSON(const QJsonValue &json);
    void setData(const ExperimentConfig &data);
    std::string getData();
    void showData();
    void clearShow();
    const ExperimentConfig &getInteralData();
    void setAuthToken(const std::string &token);
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
