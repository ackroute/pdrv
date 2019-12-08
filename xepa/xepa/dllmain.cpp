#include <Windows.h>
#include <iostream>
#include <d3d11.h>
#include "xor.h"
#include "Hook.h"

void Init() 
{
	AllocConsole();
	freopen_s((FILE**)stdin, "CONIN$", "r", stdin);
	freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
	SetConsoleTitleA(xorstr_("xepa"));

	printf(xorstr_("\n\txepa\n"));
	printf(xorstr_("\tCopyright (c) xcheats.cc - All rights reserved\n\n"));

	printf(xorstr_("[>] Hooking...\n"));
	Hook::RunHooks(); 
	printf(xorstr_("[+] Hooking complete\n"));

	while (true) 
	{
		Sleep(5000);
	}
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) 
	{
		Init();
	}
    return TRUE;
}

