#include <Windows.h>
#include <iostream>
#include <d3d11.h>
#include "Hook.h"
#include "xor.h"
#include "VMTHook.hpp"

#define _PTR_MAX_VALUE 0x7FFFFFFEFFFF

IDXGISwapChain* tempSwapChain = nullptr;
ID3D11Device* tempDevice = nullptr;
ID3D11DeviceContext* tempContext = nullptr;

DWORD_PTR* pSwapChainVtable = nullptr;
DWORD_PTR dwVTableAddress = NULL;

typedef HRESULT(__stdcall* D3D11PresentHook) (IDXGISwapChain* This, UINT SyncInterval, UINT Flags);
D3D11PresentHook oPresent = NULL;

HRESULT __stdcall hookD3D11Present(IDXGISwapChain* This, UINT SyncInterval, UINT Flags)
{
	printf(xorstr_("[+] hookD3D11Present called\n"));
	return oPresent(This, SyncInterval, Flags);
}

bool __stdcall Hook::CreateDeviceAndSwapChain()
{
	HWND hWnd = GetForegroundWindow();
	if (hWnd == nullptr)
		return false;

	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hWnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.Windowed = (GetWindowLong(hWnd, GWL_STYLE) & WS_POPUP) != 0 ? FALSE : TRUE;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	if (FAILED(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, &featureLevel, 1
		, D3D11_SDK_VERSION, &swapChainDesc, &tempSwapChain, &tempDevice, NULL, &tempContext)))
	{
		printf(xorstr_("[-] Failed to create device and swap chain\n"));
		return false;
	}

	pSwapChainVtable = (DWORD_PTR*)tempSwapChain;
	pSwapChainVtable = (DWORD_PTR*)pSwapChainVtable[0];
	
	printf(xorstr_("[+] SwapChain on %p\n"), pSwapChainVtable);
	return HookPresent((IDXGISwapChain*)pSwapChainVtable);
}

bool __stdcall Hook::HookPresent(IDXGISwapChain* swapChain)
{
	if (!swapChain)
		return false;

	//pSwapChain = swapChain;

	VMTHook dx_hook((void*)swapChain);
	oPresent = (D3D11PresentHook)dx_hook.Hook(8, (void*)hookD3D11Present);
}  

void Hook::RunHooks()
{
	if (!CreateDeviceAndSwapChain())
	{
		printf(xorstr_("[-] Failed to hook directx\n"));
	}
}   