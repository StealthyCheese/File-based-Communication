#include "ntos.h"

#define REQUEST_FILE_NAME L"\\SystemRoot\\Temp\\RequestFile.txt"
#define RESPONSE_FILE_NAME L"\\SystemRoot\\Temp\\ResponseFile.txt"

#define MAX_REQUEST_SIZE 256
#define MAX_RESPONSE_SIZE 256

#define READ_MEMORY CTL_CODE(FILE_DEVICE_UNKNOWN, 0x1337, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define WRITE_MEMORY CTL_CODE(FILE_DEVICE_UNKNOWN, 0x1338, METHOD_BUFFERED, FILE_ANY_ACCESS)

// Structure
typedef struct _MEMORY_OPERATION_REQUEST {
    ULONG ControlCode;
    ULONG ProcessId;
    PVOID Address;
    PVOID Buffer;
    SIZE_T Size;
} MEMORY_OPERATION_REQUEST, * PMEMORY_OPERATION_REQUEST;

UNICODE_STRING RequestFileName;
UNICODE_STRING ResponseFileName;

NTSTATUS ReadProcessMemory(ULONG ProcessId, PVOID Address, PVOID Buffer, SIZE_T Size)
{
    PEPROCESS Process;
    NTSTATUS Status = PsLookupProcessByProcessId((HANDLE)ProcessId, &Process);
    if (!NT_SUCCESS(Status))
    {
        return Status;
    }

    KAPC_STATE ApcState;
    KeStackAttachProcess(Process, &ApcState);

    Status = MmCopyVirtualMemory(Process, Address, PsGetCurrentProcess(), Buffer, Size, KernelMode, NULL);

    KeUnstackDetachProcess(&ApcState);
    ObDereferenceObject(Process);

    return Status;
}

NTSTATUS WriteProcessMemory(ULONG ProcessId, PVOID Address, PVOID Buffer, SIZE_T Size)
{
    PEPROCESS Process;
    NTSTATUS Status = PsLookupProcessByProcessId((HANDLE)ProcessId, &Process);
    if (!NT_SUCCESS(Status))
    {
        return Status;
    }

    KAPC_STATE ApcState;
    KeStackAttachProcess(Process, &ApcState);

    Status = MmCopyVirtualMemory(PsGetCurrentProcess(), Buffer, Process, Address, Size, KernelMode, NULL);

    KeUnstackDetachProcess(&ApcState);
    ObDereferenceObject(Process);

    return Status;
}

// main handler
NTSTATUS ProcessRequest()
{
    NTSTATUS status;
    HANDLE requestFileHandle;
    HANDLE responseFileHandle;
    OBJECT_ATTRIBUTES requestObjectAttributes;
    OBJECT_ATTRIBUTES responseObjectAttributes;
    IO_STATUS_BLOCK ioStatusBlock;
    MEMORY_OPERATION_REQUEST request;
    MEMORY_OPERATION_REQUEST response; // Added response structure

    InitializeObjectAttributes(&requestObjectAttributes, &RequestFileName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
    InitializeObjectAttributes(&responseObjectAttributes, &ResponseFileName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

    status = ZwCreateFile(&requestFileHandle, FILE_GENERIC_READ | FILE_GENERIC_WRITE, &requestObjectAttributes, &ioStatusBlock, NULL, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_OPEN_IF, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
    if (!NT_SUCCESS(status))
    {
        DbgPrint("Failed to open/create request file: %x\n", status);
        return status;
    }

    status = ZwReadFile(requestFileHandle, NULL, NULL, NULL, &ioStatusBlock, &request, sizeof(request), NULL, NULL);
    if (!NT_SUCCESS(status))
    {
        DbgPrint("Failed to read from request file: %x\n", status);
        ZwClose(requestFileHandle);
        return status;
    }
    ZwClose(requestFileHandle);

    // Process request
    switch (request.ControlCode)
    {
    case READ_MEMORY: // Read process memory
        status = ReadProcessMemory(request.ProcessId, request.Address, request.Buffer, request.Size);
        if (!NT_SUCCESS(status))
        {
            DbgPrint("Failed to read process memory: %x\n", status);
            return status;
        }
        break;
    case WRITE_MEMORY: // Write process memory
        status = WriteProcessMemory(request.ProcessId, request.Address, request.Buffer, request.Size);
        if (!NT_SUCCESS(status))
        {
            DbgPrint("Failed to write process memory: %x\n", status);
            return status;
        }
        break;
    default:
        DbgPrint("Invalid request type\n");
        return STATUS_INVALID_PARAMETER;
    }

    // Populate response structure
    RtlCopyMemory(&response, &request, sizeof(request)); // Copy request to response
    response.ControlCode = request.ControlCode; // Ensure the control code is retained

    status = ZwCreateFile(&responseFileHandle, FILE_GENERIC_WRITE, &responseObjectAttributes, &ioStatusBlock, NULL, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_OPEN_IF, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
    if (!NT_SUCCESS(status))
    {
        DbgPrint("Failed to open/create response file: %x\n", status);
        return status;
    }

    // Write the response structure to the response file
    status = ZwWriteFile(responseFileHandle, NULL, NULL, NULL, &ioStatusBlock, &response, sizeof(response), NULL, NULL);
    if (!NT_SUCCESS(status))
    {
        DbgPrint("Failed to write to response file: %x\n", status);
    }
    ZwClose(responseFileHandle);
    return status;
}

NTSTATUS DriverEntry()
{
    RtlInitUnicodeString(&RequestFileName, REQUEST_FILE_NAME);
    RtlInitUnicodeString(&ResponseFileName, RESPONSE_FILE_NAME);
    return ProcessRequest();
}