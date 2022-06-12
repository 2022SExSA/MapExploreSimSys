#ifndef MESSWINDOW_H
#define MESSWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MESSWindow; }
QT_END_NAMESPACE

class MESSWindow : public QMainWindow {
    Q_OBJECT

public:
    MESSWindow(QWidget *parent = nullptr);
    ~MESSWindow();

private:
    Ui::MESSWindow *ui;
};
#endif // MESSWINDOW_H
