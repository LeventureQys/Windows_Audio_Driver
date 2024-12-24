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

void MainWindow::on_btn_init_clicked()
{
    //获取所有设备的id 
    InitParams param;
	//我需要两个wchar_t*指针，分别指向输入设备和输出设备的id

    //获取整个，我需要DM40
    auto device_list_input = this->record->get_audio_devices_ids(true);
    auto device_list_output = this->record->get_audio_devices_ids(false);
    wchar_t* input_device_id = nullptr;
    wchar_t* output_device_id = nullptr;

    for (auto item : device_list_input) {
        if (item.first.find(L"DM40") != std::wstring::npos) {
            size_t len = item.second.length() + 1;      
            input_device_id = new wchar_t[len];
			wcscpy_s(input_device_id, len, item.second.c_str());
        }
    }

    for (auto item : device_list_output) {
        if (item.first.find(L"QCY") != std::wstring::npos) {
            size_t len = item.second.length() + 1;
			output_device_id = new wchar_t[len];
            wcscpy_s(output_device_id, len, item.second.c_str());
        }
    }

    param.input_device_id = input_device_id;
    param.output_device_id = output_device_id;

    //初始化record
    this->record->init(param);

}

void MainWindow::on_btn_start_record_clicked()
{
    this->record->start_record();
}

void MainWindow::on_btn_stop_record_clicked()
{
    this->record->stop_record();
}

void MainWindow::on_btn_enum_devices_input_clicked()
{
    auto item = WASRecorder::get_audio_devices_ids(true);
    QString single_line;
    single_line.append("Input Device List: \r\n");
    for (auto item : item) {  
        single_line.append(QString::fromStdWString(item.first));
        single_line.append("|");
        single_line.append(QString::fromStdWString(item.second));
        single_line.append("\r\n");
    }
   
	this->ui->plainTextEdit->appendPlainText(single_line);   
}

void MainWindow::on_btn_enum_devices_output_clicked()
{
    auto item = WASRecorder::get_audio_devices_ids(false);
    QString single_line;
    single_line.append("Output Device List: \r\n");
    for (auto item : item) {
        single_line.append(QString::fromStdWString(item.first));
        single_line.append("|");
        single_line.append(QString::fromStdWString(item.second));
        single_line.append("\r\n");
    }

    this->ui->plainTextEdit->appendPlainText(single_line);
}
