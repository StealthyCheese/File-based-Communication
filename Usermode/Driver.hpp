#include <Windows.h>
#include <stdio.h>

#define REQUEST_FILE_NAME L"C:\\Windows\\Temp\\RequestFile.txt"
#define RESPONSE_FILE_NAME L"C:\\Windows\\Temp\\ResponseFile.txt"

#define READ_MEMORY CTL_CODE(FILE_DEVICE_UNKNOWN, 0x1337, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define WRITE_MEMORY CTL_CODE(FILE_DEVICE_UNKNOWN, 0x1338, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct _MEMORY_OPERATION_REQUEST {
    ULONG ControlCode;
    ULONG ProcessId;
    PVOID Address;
    PVOID Buffer;
    SIZE_T Size;
} MEMORY_OPERATION_REQUEST, * PMEMORY_OPERATION_REQUEST;

template<typename ReadData>
bool ReadVirt(ULONG ProcessId, uintptr_t Address, ReadData* Buffer) {

    MEMORY_OPERATION_REQUEST request;
    DWORD bytesWritten, bytesRead;
    request.ControlCode = READ_MEMORY;
    request.ProcessId = ProcessId;
    request.Address = reinterpret_cast<PVOID>(Address);
    request.Buffer = Buffer;
    request.Size = sizeof(ReadData);

    // Open Handle To Request
    HANDLE requestFile = CreateFile(REQUEST_FILE_NAME, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (requestFile == INVALID_HANDLE_VALUE) {
        printf("Failed to open request file: %lu\n", GetLastError());
        return false;
    }

    // Write Request To File
    if (!WriteFile(requestFile, &request, sizeof(request), &bytesWritten, NULL)) {
        printf("Failed to write to request file: %lu\n", GetLastError());
        CloseHandle(requestFile);
        return false;
    }
    CloseHandle(requestFile);

    // open Handle To Response
    HANDLE responseFile = CreateFile(RESPONSE_FILE_NAME, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (responseFile == INVALID_HANDLE_VALUE) {
        printf("Failed to open response file: %lu\n", GetLastError());
        return false;
    }

    // Read Response From File
    if (!ReadFile(responseFile, &request, sizeof(request), &bytesRead, NULL)) {
        printf("Failed to read from response file: %lu\n", GetLastError());
        CloseHandle(responseFile);
        return false;
    }
    CloseHandle(responseFile);
    return true;
}

template<typename WriteData>
bool WriteVirt(ULONG ProcessId, uintptr_t Address, WriteData* Buffer) {

    MEMORY_OPERATION_REQUEST request;
    DWORD bytesWritten, bytesRead;
    request.ControlCode = WRITE_MEMORY;
    request.ProcessId = ProcessId;
    request.Address = reinterpret_cast<PVOID>(Address);
    request.Buffer = Buffer;
    request.Size = sizeof(WriteData);

    // Open Handle To Request
    HANDLE requestFile = CreateFile(REQUEST_FILE_NAME, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (requestFile == INVALID_HANDLE_VALUE) {
        printf("Failed to open request file: %lu\n", GetLastError());
        return false;
    }

    // Write Request To File
    if (!WriteFile(requestFile, &request, sizeof(request), &bytesWritten, NULL)) {
        printf("Failed to write to request file: %lu\n", GetLastError());
        CloseHandle(requestFile);
        return false;
    }
    CloseHandle(requestFile);

    // open Handle To Response
    HANDLE responseFile = CreateFile(RESPONSE_FILE_NAME, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (responseFile == INVALID_HANDLE_VALUE) {
        printf("Failed to open response file: %lu\n", GetLastError());
        return false;
    }

    // Read Response From File
    if (!ReadFile(responseFile, &request, sizeof(request), &bytesRead, NULL)) {
        printf("Failed to read from response file: %lu\n", GetLastError());
        CloseHandle(responseFile);
        return false;
    }
    CloseHandle(responseFile);
    return true;
}