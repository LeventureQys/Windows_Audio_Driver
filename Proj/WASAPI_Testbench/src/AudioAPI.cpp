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
	this->stop_record();
	release_resource();
}

bool WASRecorder::init(const InitParams& param)
{
	try {
		if (!check_init_params(param)) {
			std::wstring msg(L"Init params error : ");
			msg.append(std::wstring(param.output_device_id));
			msg.append(std::wstring(param.input_device_id));
			throw msg;
		}
		HRESULT hr;
		// 1. 查找枚举音频设备
		IMMDeviceEnumerator* enum_device = nullptr;
		hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&enum_device);

		if (FAILED(hr)) {
			throw "cant init hr 1";
		}
		// 2. 初始化输入设备和输出设备
		hr = enum_device->GetDevice(param.input_device_id, &this->ptr_input_device);
		if (FAILED(hr)) throw std::exception("Cant Init Input Device!");

		hr = enum_device->GetDevice(param.output_device_id, &this->ptr_output_device);
		if (FAILED(hr)) throw std::exception("Cant Init Output Device!");

		//3. 激活IAudioClient接口
		hr = ptr_input_device->Activate(
			__uuidof(IAudioClient), CLSCTX_ALL,
			NULL, (LPVOID*)&this->ptr_audio_client);

		hr = ptr_output_device->Activate(__uuidof(IAudioClient), CLSCTX_ALL,
			NULL, (LPVOID*)&this->ptr_output_audio_client);

		//4.获得设备的默认和最小周期、混音格式
		REFERENCE_TIME hnsDefaultDevicePeriod;
		REFERENCE_TIME hnsMinimumDevicePeriod;
		hr = ptr_audio_client->GetDevicePeriod(&hnsDefaultDevicePeriod, &hnsMinimumDevicePeriod);
		if (FAILED(hr)) {
			std::cerr << "GetDevicePeriod failed: " << hr << std::endl;
			return hr;
		}
		// 获取输出设备的默认和最小周期
		REFERENCE_TIME hnsDefaultOutputDevicePeriod;
		REFERENCE_TIME hnsMinimumOutputDevicePeriod;
		hr = ptr_output_audio_client->GetDevicePeriod(&hnsDefaultOutputDevicePeriod, &hnsMinimumOutputDevicePeriod);
		if (FAILED(hr)) {
			std::cerr << "GetDevicePeriod (output) failed: " << hr << std::endl;
			return hr;
		}



		WAVEFORMATEX* format_wav = NULL;
		hr = ptr_audio_client->GetMixFormat(&format_wav);
		if (FAILED(hr)) throw std::exception("Cant Get Mix Format!");

		WAVEFORMATEX* format_wav_output = NULL;
		hr = ptr_output_audio_client->GetMixFormat(&format_wav_output);
		if (FAILED(hr)) throw std::exception("Cant Get Mix Format Output!");




		//初始化客户端
		//初始化之前检查一下设备的状态
		DWORD dw_state = 0;
		hr = this->ptr_input_device->GetState(&dw_state);
		if (FAILED(hr)) {
			std::cerr << "Failed to get device state!" << std::endl;
			return hr;
		}
		if (dw_state != DEVICE_STATE_ACTIVE) {
			std::cerr << "Device is not active!" << std::endl;
			return E_FAIL;
		}



		hr = this->ptr_audio_client->Initialize(
			AUDCLNT_SHAREMODE_SHARED,
			AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
			hnsDefaultDevicePeriod,
			hnsDefaultDevicePeriod,
			format_wav,
			NULL);
		if (FAILED(hr)) throw std::exception("Cant Init Audio Client");

		hr = this->ptr_output_audio_client->Initialize(
			AUDCLNT_SHAREMODE_SHARED,
			0,
			hnsDefaultDevicePeriod,
			hnsDefaultDevicePeriod,
			format_wav_output,
			NULL);


		if (FAILED(hr)) throw std::exception("Cant Init Audio Client");

		//5.创建IAudioCaptureClient接口
		hr = this->ptr_audio_client->GetService(__uuidof(IAudioCaptureClient),
			(void**)&this->ptr_audio_client_capture);
		hr = this->ptr_output_audio_client->GetService(__uuidof(IAudioRenderClient),
			(void**)&this->ptr_output_audio_client_render);
		if (FAILED(hr)) throw std::exception("Cant Create Audio Capture Client");
		// 释放音频格式内存
		CoTaskMemFree(format_wav);

		//6. 设置事件句柄，用于通知录音线程

		if (this->handle_event == nullptr)
			this->handle_event = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (this->handle_event == NULL) {
			throw std::exception("Cant Create Event Handle");
		}
		hr = this->ptr_audio_client->SetEventHandle(this->handle_event);
		

		if (FAILED(hr)) 
			throw std::exception("Cant Set Event Handle");
	}
	catch (const std::exception& err) {
		std::cout << "init" << err.what() << std::endl;
		return false;
	}
	this->bln_init = true;
	return true;
}

std::vector<std::pair<std::wstring, std::wstring>> WASRecorder::get_audio_devices_ids(bool blnInput)
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
		if (blnInput) {
			hr = enum_device->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE, &ptr_devices_collection);
		}
		else {
			hr = enum_device->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &ptr_devices_collection);
		}

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

void WASRecorder::set_monitor_state(bool bln_monitor)
{

}

bool WASRecorder::start_record()
{
	try {
		if (!this->bln_init) throw std::exception("Not Init Yet");
		if (this->bln_recorder_start) throw std::exception("Alread Recording!");
		HRESULT hr = this->ptr_audio_client->Start();
		if (FAILED(hr)) throw std::exception("Cant Start" + hr);
		hr = this->ptr_output_audio_client->Start();
		if (FAILED(hr)) throw std::exception("Cant Start Listen" + hr);

		//创建录音线程
		this->handle_thread = CreateThread(
			NULL, 0,
			StaticRecordingThread,
			this, 0, NULL);

		if (this->handle_thread == nullptr) throw std::exception("Create Thread Failed");
		this->bln_recorder_start = true;
	}
	catch (const std::exception& err) {
		if (this->handle_thread != nullptr) {
			if (WaitForSingleObject(this->handle_thread, INFINITE) != WAIT_OBJECT_0) return false;
			CloseHandle(this->handle_thread);
			this->handle_thread = nullptr;
		}
		std::cerr << "start_record" << err.what() << std::endl;
		return false;
	}
	return true;
}

bool WASRecorder::stop_record()
{
	try {
		if (!bln_recorder_start) throw std::exception("Not Recording!");
		if (WaitForSingleObject(this->handle_thread, INFINITE) != WAIT_OBJECT_0) throw std::exception("Cant Stop Thread");

		HRESULT  hr = this->ptr_audio_client->Stop();
		if (FAILED(hr)) throw std::exception("Cant Stop");

		CloseHandle(this->handle_thread);
		this->handle_thread = nullptr;
		CloseHandle(this->handle_event);
		this->handle_event = nullptr;

	}
	catch (const std::exception& err) {
		std::cerr << "stop_record" << err.what() << std::endl;
		return false;
	}
	return true;
}

DWORD WASRecorder::record_thread()
{
	while (bln_recorder_start) {
		// 等待事件通知
		if (WaitForSingleObject(this->handle_event, INFINITE) != WAIT_OBJECT_0) {
			std::cerr << "WaitForSingleObject failed: " << GetLastError() << std::endl;
			continue;
		}

		// 处理音频数据
		process_audio();
	}

	return 0;
}
HRESULT WASRecorder::process_audio()
{
	HRESULT hr;
	UINT32 packetLength = 0;
	hr = ptr_audio_client_capture->GetNextPacketSize(&packetLength);
	if (FAILED(hr)) {
		std::cerr << "GetNextPacketSize failed: " << hr << std::endl;
		return hr;
	}

	while (packetLength != 0) {
		BYTE* pData;
		UINT32 numFramesAvailable;
		DWORD flags;
		hr = ptr_audio_client_capture->GetBuffer(
			&pData,
			&numFramesAvailable,
			&flags, NULL, NULL);
		if (FAILED(hr)) {
			std::cerr << "GetBuffer failed: " << hr << std::endl;
			return hr;
		}

		if (flags & AUDCLNT_BUFFERFLAGS_SILENT) {
			// 处理静音数据
		}
		else {
			// 处理音频数据
			// ... 在此处添加处理音频数据的代码 ...

			// 将音频数据写入播放设备缓冲区
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
			//memcpy(pRenderData, pData, numFramesAvailable * sizeof(float));
			hr = ptr_output_audio_client_render->ReleaseBuffer(numFramesAvailable, 0);
			if (FAILED(hr)) {
				std::cerr << "ReleaseBuffer (render) failed: " << hr << std::endl;
				return hr;
			}
		}

		hr = ptr_audio_client_capture->ReleaseBuffer(numFramesAvailable);
		if (FAILED(hr)) {
			std::cerr << "ReleaseBuffer failed: " << hr << std::endl;
			return hr;
		}
	}

	return S_OK;
}
void WASRecorder::release_resource()
{
	if (ptr_input_device) {
		ptr_input_device->Release();
		ptr_input_device = NULL;
	}
	if (enum_device) {
		enum_device->Release();
		enum_device = NULL;
	}
	if (ptr_devices_collection) {
		ptr_devices_collection->Release();
		ptr_devices_collection = NULL;
	}
	if (ptr_audio_client) {
		ptr_audio_client->Release();
		ptr_audio_client = NULL;
	}
	if (ptr_audio_client_capture) {
		ptr_audio_client_capture->Release();
		ptr_audio_client_capture = NULL;
	}

	if (ptr_output_audio_client) {
		ptr_output_audio_client->Release();
		ptr_output_audio_client = NULL;
	}
	if (ptr_output_audio_client_render) {
		ptr_output_audio_client_render->Release();
		ptr_output_audio_client_render = NULL;
	}
	CoUninitialize();
}
bool WASRecorder::check_init_params(const InitParams& param)
{
	//检查其中参数，如果有为空的，则直接报错
	if (param.input_device_id != nullptr &&
		param.output_device_id != nullptr)
		return true;
	else
		return false;
}
