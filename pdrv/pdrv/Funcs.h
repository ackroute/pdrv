#pragma once

HANDLE OpenThread(DWORD dwDesiredAccess, BOOLEAN bInheritHandle, DWORD dwThreadId);
NTSTATUS SuspendThread(__in HANDLE ThreadHandle);
NTSTATUS TerminateThread(__in HANDLE ThreadHandle);
NTSTATUS ResumeThread(HANDLE hThread);
PVOID GetModuleBase(IN char* ModuleName);