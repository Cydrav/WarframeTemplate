// By jasonfish4
#include "RWFunctions.h"

/* The KeWPM/KeRPM Functions are by github.com/Zer0Mem0ry/ */
NTSTATUS KeReadProcessMemory(PEPROCESS Process, PVOID SourceAddress, PVOID TargetAddress, SIZE_T Size)
{
	// Since the process we are reading from is the input process, we set
	// the source process variable for that.
	PEPROCESS SourceProcess = Process;
	// Since the "process" we read the output to is this driver
	// we set the target process as the current module.
	PEPROCESS TargetProcess = PsGetCurrentProcess();
	SIZE_T Result;
	if (NT_SUCCESS(MmCopyVirtualMemory(SourceProcess, SourceAddress, TargetProcess, TargetAddress, Size, KernelMode, &Result)))
		return STATUS_SUCCESS; // operation was successful
	else
		return STATUS_ACCESS_DENIED;
}
NTSTATUS KeWriteProcessMemory(PEPROCESS Process, PVOID SourceAddress, PVOID TargetAddress, SIZE_T Size)
{       // This write func is just like the read func, except vice versa.

		// Since the process writing from is our module
		// change the source process variable for that.
	PEPROCESS SourceProcess = PsGetCurrentProcess();
	// Since the process we write to is the input process
	// we set the target process as the argument
	PEPROCESS TargetProcess = Process;
	SIZE_T Result;

	if (NT_SUCCESS(MmCopyVirtualMemory(SourceProcess, SourceAddress, TargetProcess, TargetAddress, Size, KernelMode, &Result)))
		return STATUS_SUCCESS; // operation was successful
	else
		return STATUS_ACCESS_DENIED;

}
/* The KeWPM/KeRPM Functions are by github.com/Zer0Mem0ry/ */

//Write to memory in attachment, Use UserMode as KPROCESSOR_MODE when attaching to a usermode process
NTSTATUS WriteMemory(PVOID Destination, PVOID Buffer, SIZE_T BufferSize, ULONG fProtect, KPROCESSOR_MODE ProcessorMode) // Write memory
{

	PMDL mdl = IoAllocateMdl(Destination, BufferSize, FALSE, FALSE, NULL); // Allocate Memory Descriptor
	// Many MDL functions must be enclosed in a try/except statement
	__try
	{
		MmProbeAndLockPages(mdl, ProcessorMode, IoReadAccess); 
		Destination = MmGetSystemAddressForMdlSafe(mdl, HighPagePriority);

		MmProtectMdlSystemAddress(mdl, PAGE_EXECUTE_READWRITE); // Set the page rights to R/W/X
		RtlCopyMemory(Destination, Buffer, sizeof(DWORD32)); // Write Memory
		MmProtectMdlSystemAddress(mdl, fProtect); // Set back to old page rights
		
		MmUnmapLockedPages(Destination, mdl);
		MmUnlockPages(mdl);
		IoFreeMdl(mdl); // free MDL
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return 1;
	}
	
	return 0;
}

// Read Memory
VOID ReadMemory(PVOID Buffer, PVOID MemoryToRead, SIZE_T Size)
{
	RtlCopyMemory(MemoryToRead, Buffer, Size); // Copy from src to dest
}
