#include <ntifs.h>
#include <ntstrsafe.h>
#include "Log.h"
#include "Structs.h"
#include "Private.h"
#include "Imports.h"

#define SSDT_NTSUSPENDTHRED		436
#define SSDT_RESUMETHREAD		82
#define SSDT_TERMINATETHREAD	83

static NTSTATUS
(__fastcall* NtSuspendThread)(
	__in HANDLE ThreadHandle,
	__out_opt PULONG PreviousSuspendCount
	);

static NTSTATUS
(__fastcall* NtTerminateThread)(
	__in HANDLE ThreadHandle,
	DWORD  dwExitCode
	);


static NTSTATUS
(__fastcall* NtResumeThread)(
	__in HANDLE ThreadHandle,
	__out_opt PULONG PreviousSuspendCount
	);

/// <summary>
/// Open HANDLE to the thread
/// </summary>
/// <param name="dwDesiredAccess">Desired access</param>
/// <param name="bInheritHandle">Inherit handle</param>
/// <param name="dwThreadId">Thread ID</param>
/// <returns>HANDLE for thread</returns>
HANDLE OpenThread(DWORD dwDesiredAccess, BOOLEAN bInheritHandle, DWORD dwThreadId)
{
	OBJECT_ATTRIBUTES      ObjectAttributes = { 0, };
	CLIENT_ID              ClientId = { 0, };
	HANDLE                 hThread = NULL;
	NTSTATUS               Status;

	InitializeObjectAttributes(&ObjectAttributes, NULL, 0, NULL, NULL);

	if (bInheritHandle) {
		ObjectAttributes.Attributes = OBJ_INHERIT;
	}

	ClientId.UniqueProcess = NULL;
	ClientId.UniqueThread = (HANDLE)dwThreadId;

	Status = ZwOpenThread(&hThread,
		dwDesiredAccess,
		&ObjectAttributes,
		&ClientId);
	return hThread;
}

/// <summary>
/// Suspend (pause) thread
/// </summary>
/// <param name="ThreadHandle">HANDLE to desired thread</param>
/// <returns>Status</returns>
NTSTATUS SuspendThread(__in HANDLE ThreadHandle)
{
	NTSTATUS               Status;
	NtSuspendThread = (NTSTATUS(__cdecl*)(HANDLE, PULONG))GetSSDTEntry(SSDT_NTSUSPENDTHRED);
	Status = NtSuspendThread(ThreadHandle, NULL);
	return Status;
}

/// <summary>
/// Terminate thread
/// </summary>
/// <param name="ThreadHandle">HANDLE to desired thread</param>
/// <returns>Status</returns>
NTSTATUS TerminateThread(__in HANDLE ThreadHandle)
{
	NTSTATUS               Status;
	NtTerminateThread = (NTSTATUS(__cdecl*)(HANDLE, DWORD))GetSSDTEntry(SSDT_TERMINATETHREAD);
	Status = NtTerminateThread(ThreadHandle, 0);
	return Status;
}

/// <summary>
/// Resume (unpause) thread
/// </summary>
/// <param name="ThreadHandle">HANDLE to desired thread</param>
/// <returns>Status</returns>
NTSTATUS ResumeThread(__in HANDLE ThreadHandle)
{
	NTSTATUS               Status;
	NtResumeThread = (NTSTATUS(__cdecl*)(HANDLE, PULONG))GetSSDTEntry(SSDT_RESUMETHREAD);
	Status = NtResumeThread(ThreadHandle, NULL);
	return Status;
}

/// <summary>
/// Get base address of system module
/// </summary>
/// <param name="ModuleName">Name of module</param>
/// <returns>Found address, 0 if not found</returns>
PVOID GetModuleBase(IN char* ModuleName)
{
	NTSTATUS status = STATUS_SUCCESS;
	ULONG bytes = 0;
	PRTL_PROCESS_MODULES pMods = NULL;
	UNICODE_STRING routineName;

	RtlUnicodeStringInit(&routineName, L"NtOpenFile");

	// Protect from UserMode AV
	status = ZwQuerySystemInformation(SystemModuleInformation, 0, bytes, &bytes);
	if (bytes == 0)
	{
		//DPRINT("BlackBone: %s: Invalid SystemModuleInformation size\n", __FUNCTION__);
		Log("[-] Invalid SystemModuleInformation size");
		return NULL;
	}

	pMods = (PRTL_PROCESS_MODULES)ExAllocatePoolWithTag(NonPagedPool, bytes, POOL_TAG);
	RtlZeroMemory(pMods, bytes);

	status = ZwQuerySystemInformation(SystemModuleInformation, pMods, bytes, &bytes);

	if (NT_SUCCESS(status))
	{
		for (ULONG i = 0; i < pMods->NumberOfModules; i++)
		{
			// System routine is inside module
			if ((PVOID)pMods->Modules[i].ImageBase > (PVOID)0x8000000000000000)
			{
				char* pDrvName = (char*)(pMods->Modules[i].FullPathName) + pMods->Modules[i].OffsetToFileName;
				if (_stricmp(pDrvName, ModuleName) == 0)
					return (PVOID)(pMods->Modules[i].ImageBase);
			}
		}
	}

	if (pMods)
		ExFreePoolWithTag(pMods, POOL_TAG);

	return 0;
}