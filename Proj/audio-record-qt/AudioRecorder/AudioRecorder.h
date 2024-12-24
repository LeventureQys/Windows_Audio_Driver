#pragma once

#include <QAudioInput>
#include <QAudioDeviceInfo>
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
#include <algorithm> // for std::clamp
#include <QtEndian>
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

class AUDIORECORDER_EXPORT AudioIODevice : public QIODevice {
    Q_OBJECT
public:
    AudioIODevice(QObject* parent = nullptr);
    bool Initialize();

    qint64 writeData(const char* data, qint64 len) override;
	void SetFormat(QAudioFormat format);
    bool IsInit() const;
    bool open(OpenMode mode) override;

    qint64 readData(char* data, qint64 maxlen) override;

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
};

class AUDIORECORDER_EXPORT AudioRecorderInterface : public QObject {
    Q_OBJECT
public:
    AudioRecorderInterface(QObject* parent = nullptr);
    bool Initialize(const QAudioFormat& format, const QAudioDeviceInfo& info);

    void SetAudioFormat(QAudioFormat format);
    void SetAudioDeviceInfo(QAudioDeviceInfo info);

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
    QAudioFormat format;
    QAudioDeviceInfo info;
    QAudioInput* audioInput = nullptr;
    AudioIODevice* audioIODevice = nullptr;
    QAudioOutput* audioOutput = nullptr;
    bool checkRecording();
signals:
    void Sig_Volumechanged(double volume);
    void Sig_VolumePlayed(double volume);
    void Sig_PlaybackFinished();
};
