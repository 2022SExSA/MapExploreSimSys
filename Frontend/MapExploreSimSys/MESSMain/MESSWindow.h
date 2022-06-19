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

private slots:
    void on_pushButton_simulate_clicked();

    void on_pushButton_statistics_clicked();

    void on_pushButton_playback_clicked();

    void on_pushButton_mapdesigner_clicked();
    void on_tabWidget_tabCloseRequested(int index);



    void on_tabWidget_tabBarDoubleClicked(int index);

private:
    Ui::MESSWindow *ui;

};
#endif // MESSWINDOW_H
