#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <functiondiscoverykeys_devpkey.h>
#include "vector"
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "mmdevapi.lib")

// 定义录制音频的接口
class IAudioRecorder {
public:
    virtual HRESULT StartRecording(const wchar_t* deviceId, REFERENCE_TIME hnsBufferDuration) = 0;
    virtual HRESULT StopRecording() = 0;
    virtual HRESULT PlayRecordedAudio() = 0;
    virtual ~IAudioRecorder() {}
};

// WASAPI 录制音频的实现类
class WasapiAudioRecorder : public IAudioRecorder {
public:
    WasapiAudioRecorder() :
        m_pAudioClient(NULL),
        m_pCaptureClient(NULL),
        m_hEvent(NULL),
        m_bRecording(false) {}

    ~WasapiAudioRecorder() {
        StopRecording();
        if (m_pCaptureClient) {
            m_pCaptureClient->Release();
            m_pCaptureClient = NULL;
        }
        if (m_pAudioClient) {
            m_pAudioClient->Release();
            m_pAudioClient = NULL;
        }
        if (m_hEvent) {
            CloseHandle(m_hEvent);
            m_hEvent = NULL;
        }
    }

    HRESULT StartRecording(const wchar_t* deviceId, REFERENCE_TIME hnsBufferDuration) override {
        HRESULT hr;

        // 初始化 COM
        hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
        if (FAILED(hr)) {
            return hr;
        }

        // 创建 IMMDeviceEnumerator
        IMMDeviceEnumerator* pEnumerator = NULL;
        hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
        if (FAILED(hr)) {
            return hr;
        }

        // 获取指定的音频设备
        IMMDevice* pDevice = NULL;
        if (deviceId) {
            hr = pEnumerator->GetDevice(deviceId, &pDevice);
        }
        else {
            hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
        }
        pEnumerator->Release();
        if (FAILED(hr)) {
            return hr;
        }

        // 激活 IAudioClient 接口
        hr = pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&m_pAudioClient);
        pDevice->Release();
        if (FAILED(hr)) {
            return hr;
        }

        // 获取音频格式
        WAVEFORMATEX* pwfx = NULL;
        hr = m_pAudioClient->GetMixFormat(&pwfx);
        if (FAILED(hr)) {
            return hr;
        }

        // 初始化 IAudioClient
        hr = m_pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,
            AUDCLNT_STREAMFLAGS_LOOPBACK | AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
            hnsBufferDuration, 0, pwfx, NULL);
        CoTaskMemFree(pwfx);
        if (FAILED(hr)) {
            return hr;
        }

        // 获取 IAudioCaptureClient 接口
        hr = m_pAudioClient->GetService(__uuidof(IAudioCaptureClient), (void**)&m_pCaptureClient);
        if (FAILED(hr)) {
            return hr;
        }

        // 创建事件
        m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (m_hEvent == NULL) {
            return E_FAIL;
        }

        // 设置事件回调
        hr = m_pAudioClient->SetEventHandle(m_hEvent);
        if (FAILED(hr)) {
            return hr;
        }

        // 开始录制
        hr = m_pAudioClient->Start();
        if (FAILED(hr)) {
            return hr;
        }

        m_bRecording = true;

        // 启动音频捕获线程
        m_hThread = CreateThread(NULL, 0, CaptureThreadProc, this, 0, NULL);
        if (m_hThread == NULL) {
            return E_FAIL;
        }

        return S_OK;
    }

    HRESULT StopRecording() override {
        if (!m_bRecording) {
            return S_FALSE;
        }

        m_bRecording = false;

        // 停止录制
        m_pAudioClient->Stop();

        // 等待线程结束
        WaitForSingleObject(m_hThread, INFINITE);
        CloseHandle(m_hThread);

        return S_OK;
    }

    HRESULT PlayRecordedAudio() override {
        // TODO: 实现播放已录制音频的功能
        // 可以使用 WASAPI 渲染或者其他音频 API 进行播放
        return S_OK;
    }

private:
    static DWORD WINAPI CaptureThreadProc(LPVOID lpParameter) {
        WasapiAudioRecorder* pThis = (WasapiAudioRecorder*)lpParameter;
        return pThis->CaptureThread();
    }

    DWORD CaptureThread() {
        HRESULT hr;
        while (m_bRecording) {
            // 等待音频数据
            DWORD dwWaitResult = WaitForSingleObject(m_hEvent, INFINITE);
            if (dwWaitResult == WAIT_OBJECT_0) {
                // 获取音频数据
                BYTE* pData;
                UINT32 nFramesAvailable;
                DWORD dwFlags;
                hr = m_pCaptureClient->GetBuffer(&pData, &nFramesAvailable, &dwFlags, NULL, NULL);
                if (SUCCEEDED(hr)) {
                    if (nFramesAvailable > 0) {
                        // 处理音频数据
                        // ...

                        // 释放缓冲区
                        hr = m_pCaptureClient->ReleaseBuffer(nFramesAvailable);
                        if (FAILED(hr)) {
                            break;
                        }
                    }
                }
            }
        }
        return 0;
    }

private:
    IAudioClient* m_pAudioClient;
    IAudioCaptureClient* m_pCaptureClient;
    HANDLE m_hEvent;
    HANDLE m_hThread;
    bool m_bRecording;
    std::vector<BYTE> m_recordedAudio; // 存储录制的音频数据
};

int main() {
    // 创建 WasapiAudioRecorder 实例
    WasapiAudioRecorder recorder;

    // 获取默认音频设备 ID
    IMMDeviceEnumerator* pEnumerator = NULL;
    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
    if (FAILED(hr)) {
        return hr;
    }
    IMMDevice* pDevice = NULL;
    hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
    pEnumerator->Release();
    if (FAILED(hr)) {
        return hr;
    }
    LPWSTR deviceId = NULL;
    hr = pDevice->GetId(&deviceId);
    pDevice->Release();
    if (FAILED(hr)) {
        return hr;
    }

    // 开始录制
    hr = recorder.StartRecording(deviceId, 10000000); // 缓冲区时长为 1 秒
    if (FAILED(hr)) {
        CoTaskMemFree(deviceId);
        return hr;
    }
    CoTaskMemFree(deviceId);

    // 录制 10 秒
    Sleep(10000);

    // 停止录制
    hr = recorder.StopRecording();
    if (FAILED(hr)) {
        return hr;
    }

    // 播放录制的内容
    hr = recorder.PlayRecordedAudio();
    if (FAILED(hr)) {
        return hr;
    }

    return 0;
}