#include "../includes/AudioAPI.h"

WASRecorder::WASRecorder()
{
	HRESULT hr;
	// 尝试先取消初始化
	CoUninitialize();
	hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);	//指定COM组件多线程模型多线程单元 (MTA) 
	if (FAILED(hr)) {
		throw "cant init hr1";
	}
}

WASRecorder::~WASRecorder()
{
}

void WASRecorder::init(const InitParams& param)
{
	try {
	
		HRESULT hr;
		//查找枚举音频设备
		IMMDeviceEnumerator* enum_device = nullptr;
		hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&enum_device);

		if (FAILED(hr)) {
			throw "cant init hr2";
		}

		//通过指定的设备ID去获取IMMDevice音频设备


	}
	catch (const std::exception& err) {
		std::cout << err.what() << std::endl;
		return;
	}


}

std::vector<std::pair<std::wstring, std::wstring>> WASRecorder::get_audio_devices_ids()
{
	IMMDeviceEnumerator* enum_device = nullptr;
	IMMDeviceCollection* ptr_devices_collection = nullptr;
	HRESULT hr;
	std::vector<std::pair<std::wstring, std::wstring>> vec_rets;
	try {	
		hr = CoCreateInstance(
			__uuidof(MMDeviceEnumerator), NULL,
			CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
			(LPVOID*)&enum_device);
		if (FAILED(hr))
			throw "2";		
		hr = enum_device->EnumAudioEndpoints(eAll, DEVICE_STATE_ACTIVE, &ptr_devices_collection);
		if (FAILED(hr)) {
			throw "3";
		}
		UINT count;
		hr = ptr_devices_collection->GetCount(&count);
		if (FAILED(hr)) {
			throw "4";
		}
		for (UINT i = 0; i < count; i++) {
			IMMDevice* ptr_device = nullptr;
			hr = ptr_devices_collection->Item(i, &ptr_device);
			if (FAILED(hr)) {
				std::cerr << "Item failed: " << hr << std::endl;
				continue;
			}
			LPWSTR pwszID = NULL;
			hr = ptr_device->GetId(&pwszID);
			if (FAILED(hr)) {
				std::cerr << "GetId failed: " << hr << std::endl;
				ptr_device->Release();
				continue;
			}

			//还需要获得名称
			IPropertyStore* pProps = NULL;
			hr = ptr_device->OpenPropertyStore(STGM_READ, &pProps);
			std::wstring device_name;
			if (SUCCEEDED(hr)) {
				PROPVARIANT varName;
				PropVariantInit(&varName);
				hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
				if (SUCCEEDED(hr)) {
					// 使用 varName.pwszVal 获取设备名称
					std::wcout << L"Device Name: " << varName.pwszVal << std::endl;
					device_name.append(varName.pwszVal);
				}
				PropVariantClear(&varName);
				pProps->Release();
			}

			std::pair<std::wstring, std::wstring> pair;
			pair.first = device_name;
			pair.second = pwszID;

			vec_rets.push_back(pair);
		}

		return vec_rets;

	}
	catch (const std::exception& err) {
		std::cout << err.what() << std::endl;
		if (enum_device != nullptr)
			enum_device->Release();
		if (ptr_devices_collection != nullptr)
			ptr_devices_collection->Release();
		return std::vector<std::pair<std::wstring, std::wstring>>();
	}
	return std::vector<std::pair<std::wstring, std::wstring>>();
}
