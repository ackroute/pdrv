#pragma once

#include <tchar.h>

#define STATUS_SUCCESS ((NTSTATUS)0x00000000L)
#define ThreadQuerySetWin32StartAddress 9

BOOL MatchAddressToModule(__in DWORD dwProcId, __out_bcount(MAX_PATH + 1) LPTSTR lpstrModule, __in DWORD dwThreadStartAddr, __out_opt PDWORD pModuleStartAddr) // by Echo
{
	BOOL bRet = FALSE;
	HANDLE hSnapshot;
	MODULEENTRY32 moduleEntry32;

	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPALL, dwProcId);

	moduleEntry32.dwSize = sizeof(MODULEENTRY32);
	moduleEntry32.th32ModuleID = 1;

	if (Module32First(hSnapshot, &moduleEntry32)) {
		if (dwThreadStartAddr >= (DWORD)moduleEntry32.modBaseAddr && dwThreadStartAddr <= ((DWORD)moduleEntry32.modBaseAddr + moduleEntry32.modBaseSize)) {
			_tcscpy(lpstrModule, moduleEntry32.szExePath);
			printf(xorstr_("[+] %s\n"), moduleEntry32.szExePath);
		}
		else {
			while (Module32Next(hSnapshot, &moduleEntry32)) {
				if (dwThreadStartAddr >= (DWORD)moduleEntry32.modBaseAddr && dwThreadStartAddr <= ((DWORD)moduleEntry32.modBaseAddr + moduleEntry32.modBaseSize)) {
					_tcscpy(lpstrModule, moduleEntry32.szExePath);
					printf(xorstr_("[+] %s\n"), moduleEntry32.szExePath);
					break;
				}
			}
		}
	}

	if (pModuleStartAddr) *pModuleStartAddr = (DWORD)moduleEntry32.modBaseAddr;
	CloseHandle(hSnapshot);

	return bRet;
}