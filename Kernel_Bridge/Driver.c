/*++

Module Name:

    driver.c

Abstract:

    This file contains the driver entry points and callbacks.

Environment:

    Kernel-mode Driver Framework

--*/

#include "driver.h"
#define IOCTL_CODE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x3000, METHOD_BUFFERED, GENERIC_READ | GENERIC_WRITE)

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#endif

VOID
DriverUnload(
    _In_ WDFDRIVER Driver
)
{
    UNREFERENCED_PARAMETER(Driver);

    DbgPrint("DriverUnload\n");
}


NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
)
{
    NTSTATUS status;

    WDF_DRIVER_CONFIG config;
    WDF_OBJECT_ATTRIBUTES attributes;

    WDFDRIVER hDriver;
    PWDFDEVICE_INIT deviceInit;

    UNICODE_STRING sddl;

    DbgPrint("DriverEntry start\n");
    WDF_DRIVER_CONFIG_INIT(&config, WDF_NO_EVENT_CALLBACK);

    config.DriverInitFlags |= WdfDriverInitNonPnpDriver;

    config.EvtDriverUnload = DriverUnload;

    WDF_OBJECT_ATTRIBUTES_INIT(&attributes);

    status = WdfDriverCreate(
        DriverObject,
        RegistryPath,
        &attributes,
        &config,
        &hDriver
    );

    if (!NT_SUCCESS(status))
    {
        DbgPrint("WdfDriverCreate failed: 0x%X\n", status);
        return status;
    }
    RtlInitUnicodeString(
        &sddl,
        L"D:P(A;;GA;;;SY)(A;;GA;;;BA)"
    );
    deviceInit = WdfControlDeviceInitAllocate(
        hDriver,
        &sddl
    );

    if (deviceInit == NULL)
    {
        DbgPrint("WdfControlDeviceInitAllocate failed\n");
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    status = CreateControlDevice(deviceInit);

    if (!NT_SUCCESS(status))
    {
        DbgPrint("CreateControlDevice failed: 0x%X\n", status);
        return status;
    }

    DbgPrint("DriverEntry success\n");

    return STATUS_SUCCESS;
}

NTSTATUS
CreateControlDevice(
    _Inout_ PWDFDEVICE_INIT DeviceInit
)
{
    NTSTATUS status;

    WDFDEVICE hDevice;

    UNICODE_STRING deviceName;
    UNICODE_STRING symLinkName;

    PAGED_CODE();

    DbgPrint("CreateControlDevice start\n");

    RtlInitUnicodeString(
        &deviceName,
        L"\\Device\\Kernel_Bridge_Dev"
    );

    RtlInitUnicodeString(
        &symLinkName,
        L"\\??\\Kernel_Bridge_Link"
    );

    WdfDeviceInitSetDeviceType(
        DeviceInit,
        FILE_DEVICE_UNKNOWN
    );

    WdfDeviceInitSetCharacteristics(
        DeviceInit,
        FILE_DEVICE_SECURE_OPEN,
        FALSE
    );
    status = WdfDeviceInitAssignName(
        DeviceInit,
        &deviceName
    );

    if (!NT_SUCCESS(status))
    {
        DbgPrint("WdfDeviceInitAssignName failed: 0x%X\n", status);

        WdfDeviceInitFree(DeviceInit);

        return status;
    }

    status = WdfDeviceCreate(
        &DeviceInit,
        WDF_NO_OBJECT_ATTRIBUTES,
        &hDevice
    );

    if (!NT_SUCCESS(status))
    {
        DbgPrint("WdfDeviceCreate failed: 0x%X\n", status);
        return status;
    }

    status = WdfDeviceCreateSymbolicLink(
        hDevice,
        &symLinkName
    );

    if (!NT_SUCCESS(status))
    {
        DbgPrint("WdfDeviceCreateSymbolicLink failed: 0x%X\n", status);

        WdfObjectDelete(hDevice);

        return status;
    }
    

    WDF_IO_QUEUE_CONFIG  ioQueueConfig;
    WDFQUEUE  hQueue;
    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(
        &ioQueueConfig,
        WdfIoQueueDispatchSequential
    );

    ioQueueConfig.EvtIoDeviceControl = HandleIOCTL;


    status = WdfIoQueueCreate(
        hDevice,   
        &ioQueueConfig,
        WDF_NO_OBJECT_ATTRIBUTES,
        &hQueue
    );
    if (!NT_SUCCESS(status)) {
        return status;
    }

    WdfControlFinishInitializing(hDevice);
    DbgPrint("CreateControlDevice success\n");

    return STATUS_SUCCESS;
}

VOID HandleIOCTL(
    _In_ WDFQUEUE Queue,  
    _In_ WDFREQUEST Request, 
    _In_ size_t OutputBufferLength,
    _In_ size_t InputBufferLength,
    _In_ ULONG IoControlCode 
)
{
    NTSTATUS status = STATUS_SUCCESS;

    UNREFERENCED_PARAMETER(Queue);
    UNREFERENCED_PARAMETER(InputBufferLength);
    UNREFERENCED_PARAMETER(OutputBufferLength);

    size_t returnBytes = 0;

    switch (IoControlCode)
    {
    case IOCTL_CODE:
    {
        struct DeviceRequest request_data = { 0 };

        struct DeviceResponse* response_data = { 0 };

        PVOID buffer = NULL;
        PVOID outputBuffer = NULL;
        size_t length = 0;
        status = WdfRequestRetrieveInputBuffer(Request,
            sizeof(struct DeviceRequest),
            &buffer,
            &length);

        if (length != sizeof(struct DeviceRequest) || !buffer)
        {
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
        }

        request_data = *((struct DeviceRequest*)buffer);
        status = WdfRequestRetrieveOutputBuffer(Request,
            sizeof(struct DeviceResponse),
            &outputBuffer,
            &length);
        if (length != sizeof(struct DeviceResponse) || !outputBuffer)
        {
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
        }
        response_data = (struct DeviceResponse*)outputBuffer;

        response_data->result = request_data.a + request_data.b;
        returnBytes = sizeof(struct DeviceResponse);

        break;
    }
    }

    WdfRequestCompleteWithInformation(Request, status, returnBytes);
}
