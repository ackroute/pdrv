#pragma once
class Hook {
public:
	static void RunHooks();
private:
	static bool __stdcall CreateDeviceAndSwapChain();
	static bool __stdcall HookPresent(IDXGISwapChain* swapChain);
};
