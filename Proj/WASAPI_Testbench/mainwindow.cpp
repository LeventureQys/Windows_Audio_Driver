#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->record = new WASRecorder();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btn_enum_devices_clicked()
{
    auto item = this->record->get_audio_devices_ids();
    QString single_line;
    for (auto item : item) {  
        single_line.append(QString::fromStdWString(item.first));
        single_line.append("|");
        single_line.append(QString::fromStdWString(item.second));
        single_line.append("\r\n");
    }
   
	this->ui->plainTextEdit->appendPlainText(single_line);   
}
