/*
		   _
  _ __  __| |_ ___ __
 | '_ \/ _` | '_\ V /
 | .__/\__,_|_|  \_/
 |_|

 Copyright (c) 2019 Samuel Tulach - All rights reserved

*/

#include <ntifs.h>
#include <ntstrsafe.h>
#include "Log.h"
#include "Structs.h"
#include "Private.h"
#include "Funcs.h"
#include "Imports.h"

#pragma warning( disable : 4152 )

/// <summary>
/// Function executed when our hooked func is called
/// </summary>
/// <param name="dont_use1">Dummy arg</param>
/// <param name="dont_use2">Dummy arg</param>
/// <param name="param3">Argument used to indentify request</param>
/// <returns>Status</returns>
NTSTATUS HookHandler(UINT_PTR DontUse1, UINT_PTR DontUse2, PULONG32 Code)
{
	UNREFERENCED_PARAMETER(DontUse1);
	UNREFERENCED_PARAMETER(DontUse2);

	Log("[+] Hook call with code %x", *Code);

	return STATUS_SUCCESS;
}

/// <summary>
/// Driver main entry point
/// </summary>
/// <param name="DriverObject">DriverObject pointer</param>
/// <returns>Status</returns>
NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath) 
{
	// Both are undefined when we manual map the driver
	UNREFERENCED_PARAMETER(DriverObject);
	UNREFERENCED_PARAMETER(RegistryPath);
	
	// Print some copyright because that's what matters the most
	Log("\n\npdrv\nCopyright (c) 2019 xcheats.cc - All rights reserved\n\n");

	// Hook NtQueryIntervalProfile
	Log("[>] Hooking functions...");
	
	PVOID ntosbase = GetKernelBase(NULL);
	if (!ntosbase) 
	{
		Log("[-] Failed to get kernel base");
		return STATUS_CANCELLED;
	}
	Log("[+] Kernel base: %p", ntosbase);

	PVOID* dsptbl = (PVOID*)(RtlFindExportedRoutineByName(ntosbase, "HalDispatchTable"));
	if (!dsptbl)
	{
		Log("[-] Failed to get HalDispatchTable");
		return STATUS_CANCELLED;
	}
	Log("[+] HalDispatchTable: %p", dsptbl);

	dsptbl[1] = &HookHandler;

	Log("[+] Functions hoooked");

	// Return dummy status
	return STATUS_SUCCESS;
}