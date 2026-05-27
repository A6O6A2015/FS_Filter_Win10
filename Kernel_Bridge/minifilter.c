#define RTL_RTLINITUNICODESTRINGEX_NO_DLLEXPORT

#include <fltKernel.h>
#include "minifilter.h"
#pragma comment(lib, "fltmgr.lib")
PFLT_FILTER gFilterHandle = NULL;

FLT_PREOP_CALLBACK_STATUS
DelProtectPreSetInformation(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID* CompletionContext
)
{
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);

    if (Data->RequestorMode == KernelMode)
        return FLT_PREOP_SUCCESS_NO_CALLBACK;

    PFLT_PARAMETERS params = &Data->Iopb->Parameters;
    if (params->SetFileInformation.FileInformationClass != FileDispositionInformation &&
        params->SetFileInformation.FileInformationClass != FileDispositionInformationEx)
        return FLT_PREOP_SUCCESS_NO_CALLBACK;

    FILE_DISPOSITION_INFORMATION* dispInfo =
        (FILE_DISPOSITION_INFORMATION*)params->SetFileInformation.InfoBuffer;
    if (dispInfo == NULL || !dispInfo->DeleteFile)
        return FLT_PREOP_SUCCESS_NO_CALLBACK;

    PFLT_FILE_NAME_INFORMATION nameInfo = NULL;
    NTSTATUS status = FltGetFileNameInformation(Data,
        FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &nameInfo);
    if (NT_SUCCESS(status))
    {
        FltParseFileNameInformation(nameInfo);
        DbgPrint("DelProtect: DELETE BLOCKED: %wZ\\%wZ\n",
            &nameInfo->ParentDir, &nameInfo->Name);
        FltReleaseFileNameInformation(nameInfo);
    }
    else
    {
        DbgPrint("DelProtect: DELETE BLOCKED (failed to get name)\n");
    }

    Data->IoStatus.Status = STATUS_ACCESS_DENIED;
    return FLT_PREOP_COMPLETE;
}

CONST FLT_OPERATION_REGISTRATION Callbacks[] = {
    { IRP_MJ_SET_INFORMATION, 0, DelProtectPreSetInformation, NULL },
    { IRP_MJ_OPERATION_END }
};

CONST FLT_REGISTRATION FilterRegistration = {
    sizeof(FLT_REGISTRATION),
    FLT_REGISTRATION_VERSION,
    0,
    NULL,
    Callbacks,
    NULL,   
    NULL,   
    NULL,   
    NULL,   
    NULL    
};

NTSTATUS MiniFilterInit(PDRIVER_OBJECT DriverObject)
{
    NTSTATUS status = FltRegisterFilter(DriverObject, &FilterRegistration, &gFilterHandle);
    if (!NT_SUCCESS(status))
        return status;

    status = FltStartFiltering(gFilterHandle);
    if (!NT_SUCCESS(status))
    {
        FltUnregisterFilter(gFilterHandle);
        gFilterHandle = NULL;
    }
    return status;
}

VOID MiniFilterCleanup(VOID)
{
    if (gFilterHandle != NULL)
    {
        FltUnregisterFilter(gFilterHandle);
        gFilterHandle = NULL;
    }
}