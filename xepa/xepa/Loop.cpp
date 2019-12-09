#include <Windows.h>
#include <d3d9.h>
#include <iostream>
#include <d3dx9.h>
#include <Dwmapi.h> 
#include <TlHelp32.h>
#include "Globals.h"
#include "xor.h"
#include "Loop.h"
#include "SDK.h"

// ----------------------------------------------
// RENDER FUNCTIONS
// ----------------------------------------------

void DrawStr(int x, int y, DWORD color, LPD3DXFONT g_pFont, const char* fmt, ...)
{
	RECT FontPos = { x, y, x + 120, y + 16 };
	char buf[1024] = { '\0' };
	va_list va_alist;

	va_start(va_alist, fmt);
	vsprintf_s(buf, fmt, va_alist);
	va_end(va_alist);
	g_pFont->DrawText(NULL, buf, -1, &FontPos, DT_NOCLIP, color);
}

// ----------------------------------------------
// CHEAT FUNCTIONS
// ----------------------------------------------

D3DCOLOR red = D3DCOLOR_ARGB(255, 255, 0, 0);

void EntityLoop(LPD3DXFONT* font)
{
	DWORD64 entitylist = ProcessBase + o_entitylist;
	DWORD64 baseent = *(DWORD64*)(entitylist);
	if (baseent == 0)
	{
		return;
	}

	DWORD64 localentity = ProcessBase + o_locale;
	Vector viewangles = *(Vector*)(localentity + o_viewangles);

	for (int i = 0; i < 100; i++)
	{
		DWORD64 centity = *(DWORD64*)(entitylist + ((DWORD64)i << 5));
		if (centity == 0)
			continue;

		// I am bad at reading strings stfu
		/*DWORD64 name = *(DWORD64*)(centity + o_name);
		if (name != 125780153691248)  // "player.."
		{
			continue;
		}*/

		int health = *(int*)(centity + o_health);
		if (health < 1 || health > 100)
			continue;

		int shield = *(int*)(centity + 0x170);
		int total = health + shield;

		Vector location = *(Vector*)(centity + o_origin);

		Vector screen;
		if (W2S(location, screen)) 
		{
			DrawStr(screen.x, screen.y, red, *font, "player");
		}
	}
}