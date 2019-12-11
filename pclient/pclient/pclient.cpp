/*
		   _
  _ __  __| |_ ___ __
 | '_ \/ _` | '_\ V /
 | .__/\__,_|_|  \_/
 |_|

 Copyright (c) 2019 Samuel Tulach - All rights reserved

 This is client application intended to be used with pdrv.
 It calls pdrv hooked function to remove and ac callbacks
 and it manual maps dll plus hijacks thread.

 Based on:
 Thread Inject
 Web:			https://github.com/hrt/ThreadJect-x64
 License:		MIT
 Copyright:		(c) 2017 Bill Demirkapi

 When building make sure you have all the required settings
 to disable any kind of optimalization and that you are linking
 against ntdll.lib and not some shit VC libs.
*/

// TODO: Use blank space instead of allocating memory
// TODO: Load driver automatically (add kdmapper)

#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>
#include "XorCompileTime.hpp"
#include <string>

#define TARGET_DLL_ADDRESS L"C:\\Users\\Samuel Tulach\\Desktop\\pdrv\\tsar\\x64\\Release\\tsur.dll"
//#define TARGET_DLL_ADDRESS L"tsur.dll"
#define TARGET_PROCESS L"RustClient.exe"
#define TARGET_THREAD 3

#define CODE_DISABLE 0x1601
#define CODE_RESTORE 0x1602

typedef HMODULE(WINAPI* pLoadLibraryA)(LPCSTR);
typedef FARPROC(WINAPI* pGetProcAddress)(HMODULE, LPCSTR);

typedef BOOL(WINAPI* PDLL_MAIN)(HMODULE, DWORD, PVOID);

typedef struct _MANUAL_INJECT
{
	PVOID ImageBase;
	PIMAGE_NT_HEADERS NtHeaders;
	PIMAGE_BASE_RELOCATION BaseRelocation;
	PIMAGE_IMPORT_DESCRIPTOR ImportDirectory;
	pLoadLibraryA fnLoadLibraryA;
	pGetProcAddress fnGetProcAddress;
}MANUAL_INJECT, * PMANUAL_INJECT;

DWORD WINAPI LoadDll(PVOID p)
{
	PMANUAL_INJECT ManualInject;

	HMODULE hModule;
	DWORD64 i, Function, count, delta;

	DWORD64* ptr;
	PWORD list;

	PIMAGE_BASE_RELOCATION pIBR;
	PIMAGE_IMPORT_DESCRIPTOR pIID;
	PIMAGE_IMPORT_BY_NAME pIBN;
	PIMAGE_THUNK_DATA FirstThunk, OrigFirstThunk;

	PDLL_MAIN EntryPoint;

	ManualInject = (PMANUAL_INJECT)p;

	pIBR = ManualInject->BaseRelocation;
	delta = (DWORD64)((LPBYTE)ManualInject->ImageBase - ManualInject->NtHeaders->OptionalHeader.ImageBase); // Calculate the delta

	while (pIBR->VirtualAddress)
	{
		if (pIBR->SizeOfBlock >= sizeof(IMAGE_BASE_RELOCATION))
		{
			count = (pIBR->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
			list = (PWORD)(pIBR + 1);

			for (i = 0; i < count; i++)
			{
				if (list[i])
				{
					ptr = (DWORD64*)((LPBYTE)ManualInject->ImageBase + (pIBR->VirtualAddress + (list[i] & 0xFFF)));
					*ptr += delta;
				}
			}
		}

		pIBR = (PIMAGE_BASE_RELOCATION)((LPBYTE)pIBR + pIBR->SizeOfBlock);
	}

	pIID = ManualInject->ImportDirectory;

	// Resolve DLL imports

	while (pIID->Characteristics)
	{
		OrigFirstThunk = (PIMAGE_THUNK_DATA)((LPBYTE)ManualInject->ImageBase + pIID->OriginalFirstThunk);
		FirstThunk = (PIMAGE_THUNK_DATA)((LPBYTE)ManualInject->ImageBase + pIID->FirstThunk);

		hModule = ManualInject->fnLoadLibraryA((LPCSTR)ManualInject->ImageBase + pIID->Name);

		if (!hModule)
		{
			return FALSE;
		}

		while (OrigFirstThunk->u1.AddressOfData)
		{
			if (OrigFirstThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG)
			{
				// Import by ordinal

				Function = (DWORD64)ManualInject->fnGetProcAddress(hModule, (LPCSTR)(OrigFirstThunk->u1.Ordinal & 0xFFFF));

				if (!Function)
				{
					return FALSE;
				}

				FirstThunk->u1.Function = Function;
			}

			else
			{
				// Import by name

				pIBN = (PIMAGE_IMPORT_BY_NAME)((LPBYTE)ManualInject->ImageBase + OrigFirstThunk->u1.AddressOfData);
				Function = (DWORD64)ManualInject->fnGetProcAddress(hModule, (LPCSTR)pIBN->Name);

				if (!Function)
				{
					return FALSE;
				}

				FirstThunk->u1.Function = Function;
			}

			OrigFirstThunk++;
			FirstThunk++;
		}

		pIID++;
	}

	if (ManualInject->NtHeaders->OptionalHeader.AddressOfEntryPoint)
	{
		EntryPoint = (PDLL_MAIN)((LPBYTE)ManualInject->ImageBase + ManualInject->NtHeaders->OptionalHeader.AddressOfEntryPoint);
		return EntryPoint((HMODULE)ManualInject->ImageBase, DLL_PROCESS_ATTACH, NULL); // Call the entry point
	}

	return TRUE;
}

DWORD WINAPI LoadDllEnd()
{
	return 0;
}

#pragma comment(lib, "ntdll.lib")

extern "C" NTSTATUS NTAPI RtlAdjustPrivilege(ULONG Privilege, BOOLEAN Enable, BOOLEAN CurrentThread, PBOOLEAN Enabled);

UCHAR code[] = {
  0x48, 0xB8, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   // mov -16 to rax
  0x48, 0x21, 0xC4,                                             // and rsp, rax
  0x48, 0x83, 0xEC, 0x20,                                       // subtract 32 from rsp
  0x48, 0x8b, 0xEC,                                             // mov rbp, rsp
  0x90, 0x90,                                                   // nop nop
  0x48, 0xB9, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,   // mov rcx,CCCCCCCCCCCCCCCC
  0x48, 0xB8, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,   // mov rax,AAAAAAAAAAAAAAAA
  0xFF, 0xD0,                                                   // call rax
  0x90,                                                         // nop
  0x90,                                                         // nop
  0xEB, 0xFC                                                    // JMP to nop
};

void CallbackSwitch(bool restore) 
{
	FARPROC fnNtQueryIntervalProfile = GetProcAddress(LoadLibrary(L"ntdll.dll"), "NtQueryIntervalProfile");
	typedef HRESULT(__stdcall* tNtQueryIntervalProfile)(ULONG64 ProfileSource, PULONG Interval);

	tNtQueryIntervalProfile NtQueryIntervalProfile = reinterpret_cast<tNtQueryIntervalProfile>(fnNtQueryIntervalProfile);

	ULONG a2 = 0;
	if (restore) 
	{
		NtQueryIntervalProfile(CODE_RESTORE, &a2);
	}
	else 
	{
		NtQueryIntervalProfile(CODE_DISABLE, &a2);
	}

}

DWORD GetPID() 
{
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (Process32First(snapshot, &entry) == TRUE)
	{
		while (Process32Next(snapshot, &entry) == TRUE)
		{
			if (wcscmp(entry.szExeFile, TARGET_PROCESS) == 0)
			{
				return entry.th32ProcessID;
			}
		}
	}

	CloseHandle(snapshot);
	return 0;
}

void End() 
{
	printf(xorstr_("\nPlease exit the program\n"));
	getchar();
	while (true) 
	{
		Sleep(1000);
	}
}

int main()
{
	printf(xorstr_("\n\tpclient\n"));
	printf(xorstr_("\tCopyright (c) xcheats.cc - All rights reserved\n\n"));

	printf(xorstr_("[>] Initialiting...\n"));

	LPBYTE ptr;
	HANDLE hProcess, hThread, hSnap, hFile;
	PVOID mem, mem1;
	DWORD ProcessId, FileSize, read, i;
	PVOID buffer, image;
	BOOLEAN bl;
	PIMAGE_DOS_HEADER pIDH;
	PIMAGE_NT_HEADERS pINH;
	PIMAGE_SECTION_HEADER pISH;

	THREADENTRY32 te32;
	CONTEXT ctx;

	MANUAL_INJECT ManualInject;

	te32.dwSize = sizeof(te32);
	ctx.ContextFlags = CONTEXT_FULL;

	printf(xorstr_("[+] Initialized\n"));

	printf(xorstr_("[>] Disabling anticheat callbacks...\n"));
	CallbackSwitch(false);
	printf(xorstr_("[+] Callbacks disabled\n"));
	Sleep(100);
	
	printf(xorstr_("[>] Getting game PID...\n"));
	DWORD PID = GetPID();
	if (PID == 0) 
	{
		printf(xorstr_("[-] Game is not running\n"));
		End();
	}
	printf(xorstr_("[+] Found on PID %u\n"), PID);

	printf(xorstr_("[>] Injecting...\n"));
	hFile = CreateFile(TARGET_DLL_ADDRESS, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL); // Open the DLL

	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf(xorstr_("[-] Unable to open the DLL (%d)\n"), GetLastError());
		End();
	}

	FileSize = GetFileSize(hFile, NULL);
	buffer = VirtualAlloc(NULL, FileSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	if (!buffer)
	{
		printf(xorstr_("[-] Unable to allocate memory for DLL data (%d)\n"), GetLastError());

		CloseHandle(hFile);
		End();
	}

	// Read the DLL

	if (!ReadFile(hFile, buffer, FileSize, &read, NULL))
	{
		printf(xorstr_("[-] Unable to read the DLL (%d)\n"), GetLastError());

		VirtualFree(buffer, 0, MEM_RELEASE);
		CloseHandle(hFile);

		End();
	}

	CloseHandle(hFile);

	pIDH = (PIMAGE_DOS_HEADER)buffer;

	if (pIDH->e_magic != IMAGE_DOS_SIGNATURE)
	{
		printf(xorstr_("[-] Invalid executable image\n"));

		VirtualFree(buffer, 0, MEM_RELEASE);
		End();
	}

	pINH = (PIMAGE_NT_HEADERS)((LPBYTE)buffer + pIDH->e_lfanew);

	if (pINH->Signature != IMAGE_NT_SIGNATURE)
	{
		printf(xorstr_("[-] Invalid PE header\n"));

		VirtualFree(buffer, 0, MEM_RELEASE);
		End();
	}

	if (!(pINH->FileHeader.Characteristics & IMAGE_FILE_DLL))
	{
		printf(xorstr_("[-] The image is not DLL\n"));

		VirtualFree(buffer, 0, MEM_RELEASE);
		End();
	}

	RtlAdjustPrivilege(20, TRUE, FALSE, &bl);

	ProcessId = PID;
	hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessId);

	if (!hProcess)
	{
		printf(xorstr_("[-] Unable to open target process handle (%d)\n"), GetLastError());
		End();
	}

	image = VirtualAllocEx(hProcess, NULL, pINH->OptionalHeader.SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE); // Allocate memory for the DLL

	if (!image)
	{
		printf(xorstr_("[-] Unable to allocate memory for the DLL (%d)\n"), GetLastError());

		VirtualFree(buffer, 0, MEM_RELEASE);
		CloseHandle(hProcess);

		End();
	}

	// Copy the header to target process

	if (!WriteProcessMemory(hProcess, image, buffer, pINH->OptionalHeader.SizeOfHeaders, NULL))
	{
		printf(xorstr_("[-] Unable to copy headers to target process (%d)\n"), GetLastError());

		VirtualFreeEx(hProcess, image, 0, MEM_RELEASE);
		CloseHandle(hProcess);

		VirtualFree(buffer, 0, MEM_RELEASE);
		End();
	}

	pISH = (PIMAGE_SECTION_HEADER)(pINH + 1);

	// Copy the DLL to target process

	for (i = 0; i < pINH->FileHeader.NumberOfSections; i++)
	{
		WriteProcessMemory(hProcess, (PVOID)((LPBYTE)image + pISH[i].VirtualAddress), (PVOID)((LPBYTE)buffer + pISH[i].PointerToRawData), pISH[i].SizeOfRawData, NULL);
	}

	mem1 = VirtualAllocEx(hProcess, NULL, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE); // Allocate memory for the loader code

	if (!mem1)
	{
		printf(xorstr_("[-] Unable to allocate memory for the loader code (%d)\n"), GetLastError());

		VirtualFreeEx(hProcess, image, 0, MEM_RELEASE);
		CloseHandle(hProcess);

		VirtualFree(buffer, 0, MEM_RELEASE);
		End();
	}

	printf(xorstr_("[+] Loader code allocated at %#x\n"), mem1);
	memset(&ManualInject, 0, sizeof(MANUAL_INJECT));

	ManualInject.ImageBase = image;
	ManualInject.NtHeaders = (PIMAGE_NT_HEADERS)((LPBYTE)image + pIDH->e_lfanew);
	ManualInject.BaseRelocation = (PIMAGE_BASE_RELOCATION)((LPBYTE)image + pINH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
	ManualInject.ImportDirectory = (PIMAGE_IMPORT_DESCRIPTOR)((LPBYTE)image + pINH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
	ManualInject.fnLoadLibraryA = LoadLibraryA;
	ManualInject.fnGetProcAddress = GetProcAddress;


	if (!WriteProcessMemory(hProcess, mem1, &ManualInject, sizeof(MANUAL_INJECT), NULL))
		printf(xorstr_("[-] Memory write error (%d)\n"), GetLastError());
	//std::cout << "LoadDllSize " << std::dec << (DWORD64)LoadDllEnd - (DWORD64)LoadDll << std::endl;

	// FIXED by removing optimiations : some fat fucking error here.. writing LoadDll directly appears to write a bunch of JMP instructions to undefined memory and the sizes are messed
	if (!WriteProcessMemory(hProcess, (PVOID)((PMANUAL_INJECT)mem1 + 1), LoadDll, 4096 - sizeof(MANUAL_INJECT), NULL))
		printf(xorstr_("[-] Memory write error (%d)\n"), GetLastError());
	//std::cout << "LoadDllAddress " << std::hex << (PVOID)((PMANUAL_INJECT)mem1 + 1) << std::endl;

	hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

	Thread32First(hSnap, &te32);

	int number = 0;
	while (Thread32Next(hSnap, &te32))
	{
		if (te32.th32OwnerProcessID == ProcessId)
		{
			if (number == TARGET_THREAD)
			{
				break;
			}
			number++;
		}
	}
	printf(xorstr_("[+] Thread found on ID: %d\n"), te32.th32ThreadID);

	CloseHandle(hSnap);

	mem = VirtualAllocEx(hProcess, NULL, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	if (!mem)
	{
		printf(xorstr_("[-] Unable to allocate memory in target process (%d)\n"), GetLastError());

		CloseHandle(hProcess);
		End();
	}

	printf(xorstr_("[+] Memory allocated at %#x\n"), mem);

	hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, te32.th32ThreadID);

	if (!hThread)
	{
		printf(xorstr_("[-] Unable to open target thread handle (%d)\n"), GetLastError());

		VirtualFreeEx(hProcess, mem, 0, MEM_RELEASE);
		CloseHandle(hProcess);
		End();
	}

	SuspendThread(hThread);
	GetThreadContext(hThread, &ctx);

	buffer = VirtualAlloc(NULL, 65536, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	ptr = (LPBYTE)buffer;
	ZeroMemory(buffer, 65536);
	memcpy(buffer, code, sizeof(code));

	for (BYTE* ptr = (LPBYTE)buffer; ptr < ((LPBYTE)buffer + 300); ptr++)
	{
		DWORD64 address = *(DWORD64*)ptr;
		if (address == 0xCCCCCCCCCCCCCCCC)
		{
			printf(xorstr_("[>] Writing param 1 (rcx)...\n"));
			*(DWORD64*)ptr = (DWORD64)mem1;
		}

		if (address == 0xAAAAAAAAAAAAAAAA)
		{
			printf(xorstr_("[>] Writing function address (rax)...\n"));
			*(DWORD64*)ptr = (DWORD64)((PMANUAL_INJECT)mem1 + 1);
		}
	}

	if (!WriteProcessMemory(hProcess, mem, buffer, sizeof(code), NULL)) // + 0x4 because a DWORD is 0x4 big
	{
		printf(xorstr_("[-] Unable to write shellcode into target process (%d)\n"), GetLastError());

		VirtualFreeEx(hProcess, mem, 0, MEM_RELEASE);
		ResumeThread(hThread);

		CloseHandle(hThread);
		CloseHandle(hProcess);

		VirtualFree(buffer, 0, MEM_RELEASE);
		End();
	}

	ctx.Rip = (DWORD64)mem;

	if (!SetThreadContext(hThread, &ctx))
	{
		printf(xorstr_("[-] Unable to hijack target thread (%d)\n"), GetLastError());

		VirtualFreeEx(hProcess, mem, 0, MEM_RELEASE);
		ResumeThread(hThread);

		CloseHandle(hThread);
		CloseHandle(hProcess);

		VirtualFree(buffer, 0, MEM_RELEASE);
		End();
	}

	ResumeThread(hThread);

	CloseHandle(hThread);
	CloseHandle(hProcess);

	VirtualFree(buffer, 0, MEM_RELEASE);

	printf(xorstr_("[+] Injected successfully\n"));

	/*printf(xorstr_("[>] Waiting... "));
	for (int i = 1; i <= 10; i++) 
	{
		printf(xorstr_(" %i "), i);
		Sleep(1000);
	}
	printf(xorstr_("\n[+] Wait complete\n"));*/

	printf(xorstr_("[>] Restoring anticheat callbacks...\n"));
	CallbackSwitch(true);
	printf(xorstr_("[+] Callbacks restored\n"));

	End();
}
