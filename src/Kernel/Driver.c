// By jasonfish4
#include <ntifs.h>
#include <ntdef.h>
#include <wdf.h>
#include "RWFunctions.h"

DRIVER_INITIALIZE DriverEntry;
PDEVICE_OBJECT pDeviceObject; // our driver object
UNICODE_STRING dev;



NTKERNELAPI NTSTATUS PsLookupProcessByProcessId
(
	_In_ HANDLE ProcessId,
	_Outptr_ PEPROCESS *Process
	);



typedef struct _AddressTable
{
	HANDLE hack_pid;
	PEPROCESS hack_process;
	HANDLE client_pid;
	PEPROCESS client_process;
	PVOID read;
	char buffer[12];
	PVOID ImageBase;
} AddressTable;

ULONG Base;
ULONG Pid;
AddressTable sPtr;

VOID ObtainFileDriverIO(void)
{
	UNICODE_STRING     uniName;
	OBJECT_ATTRIBUTES  objAttr;

	RtlInitUnicodeString(&uniName, L"\\DosDevices\\C:\\dlog.txt");  // or L"\\SystemRoot\\example.txt"
	InitializeObjectAttributes(&objAttr, &uniName,
		OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
		NULL, NULL);

	HANDLE   handle;
	NTSTATUS ntstatus;
	IO_STATUS_BLOCK    ioStatusBlock;

	// Do not try to perform any file operations at higher IRQL levels.
	// Instead, you may use a work item or a system worker thread to perform file operations.

	if (KeGetCurrentIrql() != PASSIVE_LEVEL)
		return;

	LARGE_INTEGER byteOffset;

	ntstatus = ZwCreateFile(&handle,
		FILE_GENERIC_READ,
		&objAttr, &ioStatusBlock,
		NULL,
		FILE_ATTRIBUTE_NORMAL,
		0,
		FILE_OPEN_IF,
		FILE_SYNCHRONOUS_IO_NONALERT,
		NULL, 0);

	byteOffset.LowPart = byteOffset.HighPart = 0;

	DWORD64 buffer_one = 0;
	DWORD32 buffer_two = 0;

	ZwReadFile(handle, NULL, NULL, NULL, &ioStatusBlock, (PVOID)sPtr.buffer, 12, &byteOffset, NULL);

	memcpy(&buffer_one, sPtr.buffer, 8);
	memcpy(&buffer_two, &sPtr.buffer[8], 4);

	PVOID ReadPtr = (PVOID)buffer_one;
	sPtr.read = ReadPtr;

	sPtr.client_pid = (HANDLE)buffer_two;

	ZwClose(handle);
}


VOID ProcessLoadImageCallback(_In_opt_ PUNICODE_STRING FullImageName, IN HANDLE ProcessId, IN PIMAGE_INFO ImageInfo)
{
	DWORD64 test = 5192667906710704115;
	__try
	{
		ProbeForRead((PVOID)((DWORD32)ImageInfo->ImageBase + 0x******), 4, TYPE_ALIGNMENT(char));
		if (RtlCompareMemory((PVOID)((DWORD64)ImageInfo->ImageBase + 0x******), &test, sizeof(DWORD64)) == sizeof(DWORD32))
		{
			Base = ImageInfo->ImageBase;
			Pid = ProcessId;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return;
	}
}



VOID WorkThread(IN PVOID pContext)
{
	KAPC_STATE apc;

	sPtr.hack_pid = 0;
	sPtr.hack_process = NULL;
	sPtr.ImageBase = NULL;

	DWORD32 NoclipOffset = 0x******;
	unsigned char LoadedDriver[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
	unsigned char Noclip[4] = { 0x**, 0x**, 0x**, 0x** }; // Enable noclip
	unsigned char aNoclip[4] = { 0x**, 0x**, 0x**, 0x** }; // Disable noclip

	BOOLEAN bInit = TRUE;
	DWORD32 cmd = 0;

	LARGE_INTEGER sTime;
	sTime.QuadPart = 10000;
	PsSetLoadImageNotifyRoutine(ProcessLoadImageCallback);

	while (Base == NULL)
		KeDelayExecutionThread(KernelMode, FALSE, &sTime);

	sPtr.ImageBase = Base;
	sPtr.hack_pid = Pid;

	PsLookupProcessByProcessId(sPtr.client_pid, &sPtr.client_process);
	PsLookupProcessByProcessId(sPtr.hack_pid, &sPtr.hack_process);

	KeWriteProcessMemory(sPtr.client_process, LoadedDriver, sPtr.read, sizeof(LoadedDriver));

	while (bInit)
	{
		if (KeReadProcessMemory(sPtr.client_process, sPtr.read, &cmd, sizeof(DWORD32)) == STATUS_ACCESS_DENIED)
			break;

		switch (cmd)
		{
		case 0:
			KeDelayExecutionThread(KernelMode, FALSE, &sTime);
			break;
		case 1:
			KeStackAttachProcess(sPtr.hack_process, &apc);
			WriteMemory((PVOID)((DWORD32)sPtr.ImageBase + NoclipOffset), Noclip, sizeof(Noclip), PAGE_EXECUTE_READ, UserMode);
			KeUnstackDetachProcess(&apc);
			cmd = 0;
			break;
		case 2:
			KeStackAttachProcess(sPtr.hack_process, &apc);
			WriteMemory((PVOID)((DWORD32)sPtr.ImageBase + NoclipOffset), aNoclip, sizeof(aNoclip), PAGE_EXECUTE_READ, UserMode);
			KeUnstackDetachProcess(&apc);
			cmd = 0;
			break;
		case 3:
			bInit = FALSE;
			goto exitthread;
		}
	}

exitthread:
	sPtr.hack_pid = NULL;
	sPtr.client_pid = NULL;
	sPtr.ImageBase = NULL;
	ObDereferenceObject(sPtr.hack_process);
	ObDereferenceObject(sPtr.client_process);
	PsTerminateSystemThread(STATUS_SUCCESS);
}


NTSTATUS UnloadDriver(PDRIVER_OBJECT pDriverObject)
{
	PsRemoveLoadImageNotifyRoutine(ProcessLoadImageCallback);
	IoDeleteDevice(pDriverObject->DeviceObject);
	return 0;
}

NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT  DriverObject, _In_ PUNICODE_STRING RegistryPath)
{
	NTSTATUS status;
	WDF_DRIVER_CONFIG config;
	IoCreateDevice(DriverObject, 0, &dev, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &pDeviceObject);

	ObtainFileDriverIO();

	HANDLE hControl;
	status = PsCreateSystemThread(&hControl, (ACCESS_MASK)0, NULL, NULL, NULL, WorkThread, NULL);

	if (!NT_SUCCESS(status))
		return status;
	
	ZwClose(hControl);
	DriverObject->DriverUnload = UnloadDriver;
	return STATUS_SUCCESS;
}

