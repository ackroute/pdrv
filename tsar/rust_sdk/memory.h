#pragma once

HANDLE prochandle = nullptr;

enum ProcessAccess : DWORD
{
	Full = PROCESS_ALL_ACCESS,
	ReadOnly = PROCESS_VM_OPERATION | PROCESS_VM_READ,
	WriteOnly = PROCESS_VM_OPERATION | PROCESS_VM_WRITE,
	ReadWrite = ReadOnly | WriteOnly
};

struct LargeMem 
{
	char buffer[1024];
};

bool open() 
{
	prochandle = OpenProcess(ProcessAccess::ReadWrite, false, GetCurrentProcessId());
	if (prochandle == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	else 
	{
		printf("[+] Handle %p opened\n", prochandle);
		return true;
	}
}

template<typename T>
inline T read(uint64_t addr)
{
	if (prochandle == INVALID_HANDLE_VALUE) 
	{
		if (TEST_LOGS) printf(xorstr_("[-] Invalid handle value\n"));
	}

	T val = T();
	bool status = ReadProcessMemory(prochandle, (LPCVOID)addr, &val, sizeof(T), NULL);
	if (!status) 
	{
		if (TEST_LOGS) printf(xorstr_("[-] ReadProcessMemory failed with code %d\n"), GetLastError());
	}

	return val;
}

template<typename T>
inline T readl(uint64_t addr)
{
	LargeMem lm = read<LargeMem>(addr);
	return *(T*)&lm;
}

template<typename T>
inline void write(uint64_t addr, T value)
{
	if (prochandle == INVALID_HANDLE_VALUE)
	{
		printf(xorstr_("[-] Invalid handle value\n"));
		return;
	}

	bool status = WriteProcessMemory(prochandle, (LPVOID)addr, &value, sizeof(T), NULL);
	if (!status)
	{
		if (TEST_LOGS) printf(xorstr_("[-] WriteProcessMemory failed with code %d\n"), GetLastError());
	}
}