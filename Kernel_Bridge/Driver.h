#pragma once
/*++

Module Name:

    driver.h

Abstract:

    This file contains the driver definitions.

Environment:

    Kernel-mode Driver Framework

--*/

#include <ntddk.h>
#include <wdf.h>
#include <initguid.h>
#include <wdfcontrol.h>
#include <wdmsec.h>

EXTERN_C_START

DRIVER_INITIALIZE DriverEntry;

EVT_WDF_DRIVER_UNLOAD DriverUnload;

NTSTATUS
CreateControlDevice(
    _Inout_ PWDFDEVICE_INIT DeviceInit
);
VOID HandleIOCTL(
    _In_ WDFQUEUE Queue,   
    _In_ WDFREQUEST Request, 
    _In_ size_t OutputBufferLength,
    _In_ size_t InputBufferLength,
    _In_ ULONG IoControlCode 
);

struct DeviceRequest
{
    USHORT a;
    USHORT b;
};

struct DeviceResponse
{
    USHORT result;
};
EXTERN_C_END
