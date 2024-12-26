#include <ntddk.h> // 包含 Windows 驱动程序开发所需的基本头文件
#include <wdf.h>   // 包含 Windows Driver Framework (WDF) 的头文件

// 声明驱动程序入口函数 DriverEntry
DRIVER_INITIALIZE DriverEntry;

// 声明设备添加回调函数 KmdfHelloWorldEvtDeviceAdd
EVT_WDF_DRIVER_DEVICE_ADD KmdfHelloWorldEvtDeviceAdd;
NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT  DriverObject,  // 驱动程序对象指针
    _In_ PUNICODE_STRING RegistryPath   // 驱动程序注册表路径指针
)
{
    // 用于记录成功或失败的 NTSTATUS 变量
    NTSTATUS status = STATUS_SUCCESS;

    // 分配驱动程序配置对象
    WDF_DRIVER_CONFIG config;

    // 打印 "Hello World" 到调试输出
    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "HelloWolrd_WindowsDriver: First Time inject Venture made windows driver!\n"));

    // 初始化驱动程序配置对象，并注册 EvtDeviceAdd 回调函数 KmdfHelloWorldEvtDeviceAdd
    WDF_DRIVER_CONFIG_INIT(&config,
        KmdfHelloWorldEvtDeviceAdd);
    // 创建驱动程序对象
    status = WdfDriverCreate(DriverObject,          // 驱动程序对象指针
        RegistryPath,         // 驱动程序注册表路径指针
        WDF_NO_OBJECT_ATTRIBUTES, // 驱动程序对象属性 (此处使用默认值)
        &config,              // 驱动程序配置对象指针
        WDF_NO_HANDLE          // 可选的驱动程序对象句柄 (此处不使用)
    );
    return status; // 返回创建驱动程序对象的状态
}

// 设备添加回调函数
NTSTATUS
KmdfHelloWorldEvtDeviceAdd(
    _In_    WDFDRIVER       Driver,     // 驱动程序对象指针
    _Inout_ PWDFDEVICE_INIT DeviceInit  // 设备初始化结构体指针
)
{
    // 我们没有使用驱动程序对象，所以将其标记为未引用
    UNREFERENCED_PARAMETER(Driver);

    NTSTATUS status; // 用于记录成功或失败的 NTSTATUS 变量
    // 分配设备对象
    WDFDEVICE hDevice;
    // 打印 "Hello World" 到调试输出
    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "HelloWolrd_WindowsDriver: Hello world!\n"));

    // 创建设备对象
    status = WdfDeviceCreate(&DeviceInit,            // 设备初始化结构体指针
        WDF_NO_OBJECT_ATTRIBUTES, // 设备对象属性 (此处使用默认值)
        &hDevice                 // 设备对象句柄指针
    );
    return status; // 返回创建设备对象的状态
}


