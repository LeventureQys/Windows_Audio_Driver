
#include <string>
#include <windows.h>
#include <iostream>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <initguid.h>  // 必须包含此头文件
DEFINE_GUID(IID_IMMDeviceEnumerator, 0xA95664D2, 0x9614, 0x4F35, 0xA7, 0x46, 0xDE, 0x8D, 0xB6, 0x36, 0x17, 0xE6);
#include <vector>
#include "Functiondiscoverykeys_devpkey.h"

#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "Uuid.lib")
struct InitParams {
	wchar_t* input_device_id = nullptr;
	wchar_t* output_device_id = nullptr;
};

//参考Qt写的录音机来写一个WASAPI的录音机
class WASRecorder {
public :
	WASRecorder();
	~WASRecorder();

	/// <summary>
	/// 因为是实验项目，所以这里将初始化和开始录制接口拆开，方便调试和验证
	/// </summary>
	/// <param name="param"></param>
	bool init(const InitParams& param);
	/// <summary>
	/// 获得当前所有设备的id
	/// </summary>
	/// <returns>返回一个vector， pair中，key : 设备名称 value : 设备ID</returns>
	static std::vector<std::pair<std::wstring,std::wstring>> get_audio_devices_ids();
	//是否监听此设备 
	void set_monitor_state(bool bln_monitor);
	/// <summary>
	/// 开始录音
	/// </summary>
	/// <returns>是否正确开始录音</returns>
	bool start_record();
	
	/// <summary>
	/// 停止录音
	/// </summary>
	/// <returns>是否正确停止录音</returns>
	bool stop_record();
private:
	//创建录音线程
	static DWORD WINAPI StaticRecordingThread(LPVOID lpParam) {
		WASRecorder* ptr_this = (WASRecorder*)lpParam;
		return ptr_this->record_thread();
	}
	// 录音线程函数
	DWORD record_thread();
	HRESULT process_audio();

	//释放所有资源
	void release_resource();
	bool check_init_params(const InitParams& param);
	IMMDevice* ptr_input_device = nullptr;					//输入设备
	IMMDevice* ptr_output_device = nullptr;					//输出设备

	IMMDeviceEnumerator* enum_device = nullptr;				//设备枚举器
	IMMDeviceCollection* ptr_devices_collection = nullptr;	//设备集合

	IAudioClient* ptr_audio_client;							// 音频流
	IAudioCaptureClient* ptr_audio_client_capture;			// 音频流捕获

	IAudioClient* ptr_output_audio_client;					//播放音频流
	IAudioRenderClient* ptr_output_audio_client_capture;	//播放音频流播放器 
	HANDLE handle_event;									// 事件句柄	
	HANDLE handle_thread;									// 录音线程句柄

	bool bln_monitor = false;								//是否监听此设备
	bool bln_recorder_start = false;						//是否开始录音
	bool bln_init = false;									//是否初始化设备
};
