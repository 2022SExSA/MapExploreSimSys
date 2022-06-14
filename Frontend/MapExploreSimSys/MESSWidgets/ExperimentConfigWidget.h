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

private slots:
    void on_pushButtonImportMap_clicked();

private:
    Ui::ExperimentConfigWidget *ui;
    ExperimentConfig data_;
};

#endif // EXPERIMENTCONFIGWIDGET_H
