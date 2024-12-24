#include "AudioRecorder.h"

AudioIODevice::AudioIODevice(QObject* parent) : QIODevice(parent), recordedData(new QByteArray)
{
}

bool AudioIODevice::Initialize()
{
    blnInit = true;
    return true;
}
template <typename T>
const T& clamp(const T& v, const T& lo, const T& hi) {
    return std::min(std::max(v, lo), hi);
}
qreal normalizeAudioLevel(qreal dbLevel) {
    constexpr qreal minDb = -60;  // 最小 dB 值
    constexpr qreal maxDb = 0;    // 最大 dB 值

    // 将 dB 值线性映射到 0-100 范围
    qreal normalizedLevel = 100 * (dbLevel - minDb) / (maxDb - minDb);

    // 将值限制在 0-100 范围内
    normalizedLevel = clamp(normalizedLevel, 0.0, 100.0);

    return normalizedLevel;
}
qint64 AudioIODevice::writeData(const char* data, qint64 len)
{
    QMutexLocker locker(&mutex);

    recordedData->append(data, len);

    // 使用 QAudioFormat 来处理音频数据
    int sampleSize = format.sampleSize();
    QAudioFormat::SampleType sampleType = format.sampleType();
    int channelCount = format.channelCount();
    int bytesPerSample = sampleSize / 8;

    double rmsValue = 0;
    int sampleCount = len / bytesPerSample;

    if (sampleType == QAudioFormat::SignedInt) {
        // 假设是16位有符号整数（可能需要根据实际情况调整）
        const qint16* samples = reinterpret_cast<const qint16*>(data);
        qint64 sum = 0;
        for (int i = 0; i < sampleCount; ++i) {
            qint16 sampleValue = qFromLittleEndian<qint16>(samples[i]);
            sum += sampleValue * sampleValue;
        }
        rmsValue = std::sqrt(static_cast<double>(sum) / sampleCount) / 32768.0;
    }
    else if (sampleType == QAudioFormat::UnSignedInt) {
        // 假设是16位无符号整数
        const quint16* samples = reinterpret_cast<const quint16*>(data);
        qint64 sum = 0;
        for (int i = 0; i < sampleCount; ++i) {
            quint16 sampleValue = qFromLittleEndian<quint16>(samples[i]);
            // 将样本值偏移，使其中心在0
            int value = static_cast<int>(sampleValue) - 32768;
            sum += value * value;
        }
        rmsValue = std::sqrt(static_cast<double>(sum) / sampleCount) / 32768.0;
    }
    else if (sampleType == QAudioFormat::Float) {
        // 假设是32位浮点数
        const float* samples = reinterpret_cast<const float*>(data);
        double sum = 0;
        for (int i = 0; i < sampleCount; ++i) {
            float sampleValue = samples[i];
            sum += sampleValue * sampleValue;
        }
        rmsValue = std::sqrt(sum / sampleCount);
    }
    else {
        // 处理其他类型的数据
        return len;
    }

    // 防止对数计算出现负无穷
    if (rmsValue < 1e-10) {
        rmsValue = 1e-10;
    }

    // 将 RMS 值转换为分贝
    double dbValue = 20 * std::log10(rmsValue);

    // 定义最小和最大分贝值，用于归一化
    constexpr double minDb = -60.0;
    constexpr double maxDb = 0.0;

    // 将分贝值归一化到 0 - 1 范围
    double normalizedVolume = (dbValue - minDb) / (maxDb - minDb);
    // 限制在 0 - 1 范围内
    normalizedVolume = clamp(normalizedVolume, 0.0, 1.0);

    // 转换为 0 - 100 范围
    double volume = normalizedVolume * 100;

    // 发出音量信号
    emit sig_volumeChanged(volume);

    return len;
}

void AudioIODevice::SetFormat(QAudioFormat format)
{
    this->format = format;
}

bool AudioIODevice::IsInit() const
{
    return this->blnInit;
}

bool AudioIODevice::open(OpenMode mode)
{
    return QIODevice::open(mode | QIODevice::ReadWrite); // 使用ReadWrite模式打开
}

qint64 AudioIODevice::readData(char* data, qint64 maxlen)
{
    Q_UNUSED(data);
    Q_UNUSED(maxlen);
    return 0;
}

void AudioIODevice::FinishRecord()
{
    // 在内存中的录音数据上添加WAV文件头并保存到文件
    //QFile outputFile("output.wav");
    //addWavHeader(*recordedData, outputFile);
    recordedData->clear();
}

qint64 AudioIODevice::addWavHeader(const QByteArray& pcmData, QFile& wavFile)
{
    WAVFILEHEADER WavFileHeader;
    memcpy(WavFileHeader.RiffName, "RIFF", 4);
    memcpy(WavFileHeader.WavName, "WAVE", 4);
    memcpy(WavFileHeader.FmtName, "fmt ", 4);

    WavFileHeader.nFmtLength = 16;
    WavFileHeader.nAudioFormat = 1;
    memcpy(WavFileHeader.DATANAME, "data", 4);

    WavFileHeader.nBitsPerSample = 16;
    WavFileHeader.nBytesPerSample = 2;
    WavFileHeader.nSampleRate = 48000;
    WavFileHeader.nBytesPerSecond = 16000;
    WavFileHeader.nChannleNumber = 1;

    if (!wavFile.open(QIODevice::WriteOnly))
    {
        return -1;
    }

    qint64 nFileLen = pcmData.size();

    WavFileHeader.nRiffLength = nFileLen + sizeof(WavFileHeader) - 8;
    WavFileHeader.nDataLength = nFileLen;

    wavFile.write(reinterpret_cast<char*>(&WavFileHeader), sizeof(WavFileHeader));
    wavFile.write(pcmData);

    wavFile.close();

    return nFileLen;
}

bool AudioIODevice::SaveToFile(const QString& filePath)
{
    QFile outputFile(filePath);
    //尝试打开文件
    /*if (!outputFile.open(QIODevice::WriteOnly))
    {
        qDebug() << __FUNCTION__ << " Cant Open File:" << filePath;
        return false;
   }*/
    if (addWavHeader(*recordedData, outputFile) != -1) {
        return true;
    }
    return false;
}

bool AudioIODevice::CanPlayRecordedData()
{
    return this->recordedData->size() > 0;
}

AudioRecorderInterface::AudioRecorderInterface(QObject* parent) : QObject(parent)
{


}

bool AudioRecorderInterface::Initialize(const QAudioFormat& format, const QAudioDeviceInfo& info)
{
    if (blninit) return true;
    this->format = format;
    this->info = info;
    this->audioIODevice = new AudioIODevice(this);
    connect(this->audioIODevice, &AudioIODevice::sig_volumeChanged, this, &AudioRecorderInterface::Sig_Volumechanged);
    this->audioInput = new QAudioInput(this->info, this->format, this);
    this->blninit = true;
    this->audioIODevice->SetFormat(this->format);
    return true;
}

void AudioRecorderInterface::SetAudioFormat(QAudioFormat format)
{
    this->format = format;
    bool blnRunning = this->audioInput->state() == QAudio::ActiveState;
    this->audioInput->stop();
    delete this->audioInput;
    this->audioInput = new QAudioInput(this->info, this->format, this);
    if (blnRunning) {
        this->audioInput->start(this->audioIODevice);
    }
}

void AudioRecorderInterface::SetAudioDeviceInfo(QAudioDeviceInfo info)
{
    this->info = info;
    bool blnRunning = this->audioInput->state() == QAudio::ActiveState;
    this->audioInput->stop();
    delete this->audioInput;
    this->audioInput = new QAudioInput(this->info, this->format, this);
    if (blnRunning) {
        this->audioInput->start(this->audioIODevice);
    }
}

bool AudioRecorderInterface::StartRecord()
{
    this->audioIODevice->Initialize();
    this->audioIODevice->open(QIODevice::WriteOnly);
    this->audioInput->start(this->audioIODevice);
    return true;
}

void AudioRecorderInterface::StopRecord()
{
    this->audioInput->stop();
    this->audioIODevice->FinishRecord();
}

void AudioRecorderInterface::PauseRecord()
{
    if (this->checkRecording()) {
        this->audioInput->suspend();
    }
}

void AudioRecorderInterface::ResumeRecord()
{
    if (this->audioInput->state() == QAudio::SuspendedState) {
        this->audioInput->resume();
    }
}
qreal calculateAudioLevel(const QByteArray& audioData) {
    qreal sumSquared = 0;
    const qint16* samples = reinterpret_cast<const qint16*>(audioData.constData());
    int sampleCount = audioData.size() / sizeof(qint16);

    for (int i = 0; i < sampleCount; ++i) {
        sumSquared += qPow(samples[i], 2);
    }

    qreal rms = qSqrt(sumSquared / sampleCount);
    // 将 RMS 值转换为 dB 值
    qreal dbLevel = 20 * qLn(rms / std::numeric_limits<qint16>::max()) / qLn(10);
    return dbLevel;
}

void AudioRecorderInterface::PlayRecordedData()
{
    if (audioOutput) {
        delete audioOutput;
    }

    if (this->audioIODevice->getRecordedData().data()->size() == 0)
    {
        qDebug() << __FUNCTION__ << "No Data to play";
        return;
    }

    // 获取默认音频输出设备
    QAudioDeviceInfo outputDeviceInfo = QAudioDeviceInfo::defaultOutputDevice();

    // 初始化 QAudioOutput 使用默认输出设备和当前格式
    audioOutput = new QAudioOutput(outputDeviceInfo, format, this);
    connect(audioOutput, &QAudioOutput::stateChanged, this, [this](QAudio::State newState) {
        if (newState == QAudio::IdleState) {
            // 播放结束后的处理逻辑
            emit this->Sig_PlaybackFinished();
        }
        });
    // 设置 notifyInterval，例如每 100 毫秒发送一次通知
    audioOutput->setNotifyInterval(100);
    // 将录制的数据写入 QBuffer 并播放
    QIODevice* memoryDevice = new QBuffer(this->audioIODevice->getRecordedData().data(), this);
    connect(audioOutput, &QAudioOutput::notify, this, [this, memoryDevice]() {
        // 获取已经处理的音频数据时长
        qint64 processedUSecs = audioOutput->processedUSecs();

        // 计算当前播放的音频数据在 memoryDevice 中的位置
        qint64 currentPosition = (processedUSecs * format.sampleRate() * format.channelCount() * format.sampleSize()) / (1000000 * 8);

        // 读取相应位置的音频数据
        memoryDevice->seek(currentPosition);
        QByteArray audioData = memoryDevice->read(1024); // 读取 1024 字节的音频数据

        // 计算音频电平
        qreal level = calculateAudioLevel(audioData);
        // 将 dB 值归一化到 0-100 范围
        qreal normalizedLevel = normalizeAudioLevel(level);
        // 发射信号或更新 UI 显示电平值
        emit Sig_VolumePlayed(normalizedLevel);
        });
    memoryDevice->open(QIODevice::ReadOnly);

    audioOutput->start(memoryDevice);

}

void AudioRecorderInterface::StopPlayBack()
{
    if (audioOutput) {
        audioOutput->stop(); // 停止音频播放
        audioOutput->disconnect(); // 断开音频输出的所有连接
        delete audioOutput; // 删除音频输出对象
        audioOutput = nullptr;
    }
}

bool AudioRecorderInterface::SaveRecordedData(const QString& filePath)
{
    return this->audioIODevice->SaveToFile(filePath);
}

bool AudioRecorderInterface::CanRecordDataPlay()
{
    if (this->audioIODevice) {
        return this->audioIODevice->CanPlayRecordedData();
    }
    else {
        return false;
    }
}

bool AudioRecorderInterface::checkRecording()
{
    return this->audioInput->state() == QAudio::ActiveState;
}