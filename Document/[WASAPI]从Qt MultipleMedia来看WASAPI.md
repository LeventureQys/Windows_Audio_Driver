# [WASAPI] 从Qt MultipleMedia 来看WASAPI

最近在学习有关Windows上的音频驱动相关的知识，在正式开始说WASAPI之前，我想先说一说Qt的Multiple Media，为什么呢？因为Qt的MultipleMedia实际上是WASAPI的一层封装，它在是线上替我做了很多事，就好像在Microsoft的文档上会推荐你先学习Windows.Media.Capture，然后再看low level的WASAPI。

我这篇文章中，一方面是我Qt MultipleMedia用的比较多,另一方面，Qt MultiMedia也比较简单，为音频相关的API做了很多封装，这样就不需要你自己一个个HRESULT的去调试和测试了。

## Qt MultiMedia Audio Recorder

由于Qt在5进6之后对Qt MultiMedia进行了大范围重构，所以这里Qt的项目我做了两个版本，分别为
[audio-record-qt](https://github.com/LeventureQys/Windows_Audio_Driver/tree/main/Proj/audio-record-qt)

[audio-record-qt6](https://github.com/LeventureQys/Windows_Audio_Driver/tree/main/Proj/audio-record-qt-6)

在调用上，Qt6和Qt5没有本质区别，所以这里我将着重聊一聊qt5上的录音机

在Qt5中，录音机的数据流如图所示：

![](https://raw.githubusercontent.com/LeventureQys/Picturebed/main/image/Qt5MultiMedia%E9%9F%B3%E9%A2%91%E6%95%B0%E6%8D%AE%E6%B5%81.drawio.png)

流程大概如下：

1. 获取所有设备的信息
2. 根据名称匹配，获取我们需要的那个设备的QAudioDeviceInfo
3. 使用QAudioDeviceInfo，获取到QAudioInput（输入）和QAudioOutput（输出）设备
4. 重写一个QIODevice类，修改其writeData方法，并在其中完成你想要做的事情，包括但不限于：保存为文件，获得耳返数据，进行算法的处理等等。
5. 将你继承了QIODevice的类的成员变量，放进QAudioInput和QAudioOutput的start中，这样一个完整的流就完成了。

其实WASAPI实际上也就是沿着这个Qt的MultiMedia的思路进行开发就可以了，但是在WASAPI中，没有Qt的封装，接口上会更加复杂一点而已。但是总的流程并没有本质区别。

还有需要注意的一点，就是QIODevice和QByteArray对数据流的封装做的很好，在纯C++中只能自己手动管理，所以这个地方可能会出现内存泄漏的风险，在开发的时候需要多多注意内存泄漏的问题。

## WASAPI Audio Recorder

工程地址：
    [LeventureQys/Windows_Audio_Driver/WASAPI_Testbench](https://github.com/LeventureQys/Windows_Audio_Driver/tree/main/Proj/WASAPI_Testbench)

在WASAPI中，和Qt的MultiMedia中大的流程是一样的，但是在接口上来说往往更加复杂，简单的来说，流程大致如下：

![](https://raw.githubusercontent.com/LeventureQys/Picturebed/main/image/WASAPI%E9%9F%B3%E9%A2%91%E6%95%B0%E6%8D%AE%E6%B5%81.drawio.png)

其中和QtMultiMedia中最重要的区别就是没有一个专门的QIODevice去帮我处理线程和数据的关系，而是需要自己单开一个线程，然后从Capture/Render实例中去GetBuffer，然后从中获取数据或者往里面写入数据，再手动释放。

这个过程非常自由，同样也非常容易出现意外，所以在操作WASAPI的过程中需要谨慎谨慎再谨慎。

具体的代码详情见Github链接    [LeventureQys/Windows_Audio_Driver/WASAPI_Testbench](https://github.com/LeventureQys/Windows_Audio_Driver/tree/main/Proj/WASAPI_Testbench) 我这里只简单说说我在工程中遇到的几个小问题。

 1. 输入设备的IAudioClient Initialize方法失败

我的调用函数如下：
```c++
hr = this->ptr_audio_client->Initialize(
    AUDCLNT_STREAMFLAGS_LOOPBACK | AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
    AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
    hnsDefaultDevicePeriod,
    hnsDefaultDevicePeriod,
    format_wav,
    NULL);
```
在这个函数中，第二个参数我设置的是AUDCLNT_STREAMFLAGS_LOOPBACK | AUDCLNT_STREAMFLAGS_EVENTCALLBACK 这个地方具体要取决于设备是否允许进行回环录制和是否允许回调，并不是所有麦克风都支持这俩。

2. 录制后的声音播放出来有很强的噪音，但我能确定声音是从麦克风传来的。

这种情况大概率是两边的声音没有对齐，这个根据wav的编码方式来的。简单地说，就是两边的channel和bitrate不匹配，导致声音无法对齐。具体你需要比对这两个format,然后再根据实际情况在音频处理处做应对和调整

```c++
WAVEFORMATEX* format_wav = NULL;
hr = ptr_audio_client->GetMixFormat(&format_wav);
if (FAILED(hr)) throw std::exception("Cant Get Mix Format!");

WAVEFORMATEX* format_wav_output = NULL;
hr = ptr_output_audio_client->GetMixFormat(&format_wav_output);
if (FAILED(hr)) throw std::exception("Cant Get Mix Format Output!");
```

具体怎么调整详情可以看

[[音视频学习笔记]二、什么是PCM音频？一些常见的PCM处理](https://blog.csdn.net/Andius/article/details/136739875)

比如我这里，我的麦克风的channels是1，但是耳机的channels是2，所以这里在播放的时候需要调整一下，将每一个bit都复制一份，放到输出的音频流中，如代码所示：

```c++
BYTE* pRenderData;
hr = ptr_output_audio_client_render->GetBuffer(numFramesAvailable, &pRenderData);
if (FAILED(hr)) {
    std::cerr << "GetBuffer (render) failed: " << hr << std::endl;
    return hr;
}
float* inputData = reinterpret_cast<float*>(pData);
float* outputData = reinterpret_cast<float*>(pRenderData);

for (UINT32 i = 0; i < numFramesAvailable; i++) {
    // 将单声道复制到立体声的两个通道
    outputData[i * 2] = inputData[i];
    outputData[i * 2 + 1] = inputData[i];
}
```