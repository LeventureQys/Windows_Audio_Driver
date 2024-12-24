#pragma once

#include <QAudioInput>
#include <QMediaDevices>
#include <QAudioDevice>
#include <QObject>
#include <QIODevice>
#include <QFile>
#include <QMutex>
#include <QDateTime>
#include <QDir>
#include <QSharedPointer>
#include <QVector>
#include <QAudioOutput>
#include "qbuffer.h"
#include <QtGlobal>
#include <cmath> 
#include "qdebug.h"
#include "qtimer.h"
#include <QtMath>
#include "QAudioFormat"
#include <algorithm> // for std::clamp
#include <QtEndian>
#include "qaudiosource.h"
#include "qaudiosink.h"
#include "QMediaCaptureSession"
#ifdef AUDIORECORDER
#define AUDIORECORDER_EXPORT Q_DECL_EXPORT
#else
#define AUDIORECORDER_EXPORT Q_DECL_IMPORT
#endif

struct AUDIORECORDER_EXPORT WAVFILEHEADER
{
    char RiffName[4];
    unsigned long nRiffLength;

    char WavName[4];
    char FmtName[4];
    unsigned long nFmtLength;

    unsigned short nAudioFormat;
    unsigned short nChannleNumber;
    unsigned long nSampleRate;
    unsigned long nBytesPerSecond;
    unsigned short nBytesPerSample;
    unsigned short nBitsPerSample;

    char DATANAME[4];
    unsigned long nDataLength;
};

class AUDIORECORDER_EXPORT AudioIODevice : public QBuffer {
    Q_OBJECT
public:
    AudioIODevice(QObject* parent = nullptr);
    bool Initialize();

    qint64 writeData(const char* data, qint64 len) override;
    qint64 readData(char* data, qint64 maxlen) override;
	void SetFormat(QAudioFormat format);
    bool IsInit() const;
    bool open(OpenMode mode) override;



    void FinishRecord();
    QSharedPointer<QByteArray> getRecordedData() const { return recordedData; }
    bool SaveToFile(const QString& filePath);
    bool CanPlayRecordedData();
private:
    qint64 addWavHeader(const QByteArray& pcmData, QFile& wavFile);
    QMutex mutex;
    QSharedPointer<QByteArray> recordedData;
    bool blnInit = false;
    QAudioFormat format;
signals:
    void sig_volumeChanged(double volume);
    //void sig_volumeChanged_output(double volume);
};

class AUDIORECORDER_EXPORT AudioRecorderInterface : public QObject {
    Q_OBJECT
public:
    AudioRecorderInterface(QObject* parent = nullptr);
    bool Initialize(const QAudioFormat& format, const QAudioDevice& info);

    void SetAudioFormat(QAudioFormat format);
    void SetAudioDeviceInfo(QAudioDevice info);

    bool StartRecord();
    void StopRecord();
    void PauseRecord();
    void ResumeRecord();
    void PlayRecordedData();
    void StopPlayBack();
    bool SaveRecordedData(const QString& filePath);
    bool CanRecordDataPlay();
private:
    bool blninit = false;
    qsizetype int_buffer_size = 360;
    QAudioFormat format;
    //QAudioDeviceInfo info;
    QAudioDevice info;
    QAudioSource* audioInput = nullptr;
    AudioIODevice* audioIODevice = nullptr;
    //AudioIODevice* audioOutputStream = nullptr;
    QAudioSink* audioOutput = nullptr;
    QTimer timer_output_seeker;
    //QMediaCaptureSession session_output;
    bool checkRecording();
    /// <summary>
    /// key : time value : volume
    /// </summary>
    QMap<size_t, QPair<size_t, double>> map_time_volume;
   // QMap<size_t, double> map_time_volume;
    void RecordVolume(double volume);
    void LoopFromMapTimeVolume(int index);
signals:
    void Sig_Volumechanged(double volume);
    void Sig_VolumePlayed(double volume);
    void Sig_PlaybackFinished();
};
