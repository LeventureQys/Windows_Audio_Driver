#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <algorithm> // for std::clamp
#include <QMainWindow>
#include "./AudioRecorder/AudioRecorder.h"
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    
    void on_btn_record_clicked();
    void on_btn_stop_clicked();
    void on_btn_init_clicked();
    void on_btn_pause_clicked();
    void on_btn_resume_clicked();
    void on_btn_play_clicked();
private:
    AudioRecorderInterface interface;
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
