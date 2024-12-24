
#include <string>
#include <windows.h>
#include <iostream>
#include <mmdeviceapi.h>
#include <initguid.h>  // 必须包含此头文件
DEFINE_GUID(IID_IMMDeviceEnumerator, 0xA95664D2, 0x9614, 0x4F35, 0xA7, 0x46, 0xDE, 0x8D, 0xB6, 0x36, 0x17, 0xE6);
#include <vector>
#include "Functiondiscoverykeys_devpkey.h"

#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "Uuid.lib")
struct InitParams {
	wchar_t* device_id = nullptr;

};

//参考Qt写的录音机来写一个WASAPI的录音机
class WASRecorder {
public :
	WASRecorder();
	~WASRecorder();
	bool blnInit = false;

	void init(const InitParams& param);
	/// <summary>
	/// 获得当前所有设备的id
	/// </summary>
	/// <returns>返回一个vector :key : 设备名称 value : 设备ID</returns>
	std::vector<std::pair<std::wstring,std::wstring>> get_audio_devices_ids();
	
	

};
