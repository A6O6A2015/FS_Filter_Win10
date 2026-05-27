#pragma once
typedef struct _DRIVER_OBJECT* PDRIVER_OBJECT;

NTSTATUS MiniFilterInit(PDRIVER_OBJECT DriverObject);
VOID     MiniFilterCleanup(VOID);