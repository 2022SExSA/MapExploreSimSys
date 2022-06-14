#ifndef MESSSIMULATIONWIDGET_H
#define MESSSIMULATIONWIDGET_H

#include <QWidget>

#include "dll_export.h"

namespace Ui { class MESSSimulationWidget; }

class MESSWIDGETS_EXPORT MESSSimulationWidget : public QWidget {
    Q_OBJECT

public:
    explicit MESSSimulationWidget(QWidget *parent = nullptr);
    ~MESSSimulationWidget();

private:
    Ui::MESSSimulationWidget *ui;
};

#endif // MESSSIMULATIONWIDGET_H
