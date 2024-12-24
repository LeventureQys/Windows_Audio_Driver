#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(&this->interface, &AudioRecorderInterface::Sig_Volumechanged, this, [=](double volume) {
        qDebug() << "volume changed : " << volume;
        });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btn_record_clicked()
{
    this->interface.StartRecord();
}

void MainWindow::on_btn_stop_clicked()
{
    this->interface.StopRecord();
}

void MainWindow::on_btn_init_clicked()
{
    QAudioFormat format;
    format.setSampleRate(48000);
    format.setChannelCount(1);  // 单声道
    format.setSampleFormat(QAudioFormat::SampleFormat::Int16);
    format.setChannelConfig(QAudioFormat::ChannelConfigMono);
   // format.setCodec("audio/pcm");
    //format.setByteOrder(QAudioFormat::LittleEndian);
    //format.setSampleType(QAudioFormat::SignedInt);
    QAudioDevice info = QMediaDevices::defaultAudioInput();
    if (!info.isFormatSupported(format)) {
        qDebug() << __FUNCTION__ << "Not supported";
        return;
    }
    interface.Initialize(format, info);
}

void MainWindow::on_btn_pause_clicked()
{
    interface.PauseRecord();
}

void MainWindow::on_btn_resume_clicked()
{
    interface.ResumeRecord();
}

void MainWindow::on_btn_play_clicked()
{
    interface.PlayRecordedData();
}

