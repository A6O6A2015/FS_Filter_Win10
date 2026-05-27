#pragma once

#include <wdm.h>
#include <wdf.h>

struct DeviceRequest {
    int a;
    int b;
};

struct DeviceResponse {
    int result;
};

#define IOCTL_CODE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x3000, METHOD_BUFFERED, GENERIC_READ | GENERIC_WRITE)

NTSTATUS CreateControlDevice(_Inout_ PWDFDEVICE_INIT DeviceInit);
VOID HandleIOCTL(_In_ WDFQUEUE Queue, _In_ WDFREQUEST Request, _In_ size_t OutputBufferLength, _In_ size_t InputBufferLength, _In_ ULONG IoControlCode);
VOID DriverUnload(_In_ WDFDRIVER Driver);
NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject,_In_ PUNICODE_STRING RegistryPath);