#ifndef FUNCTIONS
#define FUNCTIONS

#include <ntifs.h>
#include <ntdef.h>
#include <wdf.h>

NTKERNELAPI NTSTATUS PsLookupProcessByProcessId
(
	_In_ HANDLE ProcessId,
	_Outptr_ PEPROCESS *Process
	);

NTSTATUS NTAPI MmCopyVirtualMemory
(
	PEPROCESS SourceProcess,
	PVOID SourceAddress,
	PEPROCESS TargetProcess,
	PVOID TargetAddress,
	SIZE_T BufferSize,
	KPROCESSOR_MODE PreviousMode,
	PSIZE_T ReturnSize
	);

NTSTATUS KeReadProcessMemory(PEPROCESS Process, PVOID SourceAddress, PVOID TargetAddress, SIZE_T Size);

NTSTATUS KeWriteProcessMemory(PEPROCESS Process, PVOID SourceAddress, PVOID TargetAddress, SIZE_T Size);

KAPC_STATE AttachProcess(HANDLE ProcessId);

NTSTATUS WriteMemory(PVOID Destination, PVOID Buffer, SIZE_T BufferSize, ULONG fProtect, KPROCESSOR_MODE ProcessorMode);

VOID ReadMemory(PVOID Buffer, PVOID MemoryToRead, SIZE_T Size);

void DetachProcess(KAPC_STATE* apc);

#endif
