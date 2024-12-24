#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "./includes/AudioAPI.h"
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
public slots:
    void on_btn_enum_devices_input_clicked();
    void on_btn_enum_devices_output_clicked();
    void on_btn_init_clicked();
	void on_btn_start_record_clicked();
    void on_btn_stop_record_clicked();
private:
    Ui::MainWindow *ui;
    WASRecorder* record;
};
#endif // MAINWINDOW_H
