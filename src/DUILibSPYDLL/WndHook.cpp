#include "stdafx.h"
#include "WndHook.h"

HHOOK g_hHook;

LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	return  CallNextHookEx(g_hHook, nCode, wParam, lParam);
}

BOOL SetHook()
{
	g_hHook = SetWindowsHookEx(WH_CALLWNDPROC, CallWndProc, g_hModuleHookDll, 0);
	
	return g_hHook ? TRUE : FALSE;
}

BOOL UnSetHook()
{
	return UnhookWindowsHookEx(g_hHook);
}