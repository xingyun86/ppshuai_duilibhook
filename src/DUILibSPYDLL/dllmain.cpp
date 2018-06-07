// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#include <NAKEDHOOK.h>
#include "WinSubclass.h"

HMODULE g_hModuleHookDll;
HOOK_INFO g_HookInfo_CrtWinExW;


BOOL CALLBACK _CurrWndEnumHelp(HWND hWnd, LPARAM lParam)
{
	DWORD dwProcId = 0;
	::GetWindowThreadProcessId(hWnd, &dwProcId);
	if (GetCurrentProcessId() == dwProcId)
	{
		CWinSubclass::SubclassDUILibWindow(hWnd);
	}

	return TRUE;
}

VOID WINAPI CreateWindowExW_OUT(HWND hWnd)
{
	CWinSubclass::SubclassDUILibWindow(hWnd);
}

#define NAKED __declspec(naked) 
VOID NAKED CreateWindowExW_Ret_Sub_Proxy()
{
	__asm pushfd
	__asm pushad
	__asm push eax
	__asm call CreateWindowExW_OUT
	__asm popad
	__asm popfd

	__asm ret
}

CString PathGetFileName(CString &strFullPath)
{
	int nPos = strFullPath.ReverseFind(_T('\\'));
	if (-1 == nPos)
		nPos = strFullPath.ReverseFind(_T('/'));
	
	if (-1 == nPos)
		return strFullPath;

	return strFullPath.Mid(nPos + 1);
}

BOOL InitSPYDll(HMODULE hModule)
{
	g_hModuleHookDll = hModule;

	
	CString strExePath;
	GetModuleFileName(NULL, strExePath.GetBuffer(MAX_PATH), MAX_PATH);
	strExePath.ReleaseBuffer();

	if (PathGetFileName(strExePath).MakeUpper() != _T("WECHAT.EXE"))
		return FALSE;

	CString strModuleFullPath;
	GetModuleFileName(hModule, strModuleFullPath.GetBuffer(MAX_PATH), MAX_PATH);
	strModuleFullPath.ReleaseBuffer();
	LoadLibrary(strModuleFullPath);//inc for lock dll

	EnumWindows(_CurrWndEnumHelp, NULL);

	return InstallInlineByName("user32.dll", "CreateWindowExW", CreateWindowExW_Ret_Sub_Proxy, &g_HookInfo_CrtWinExW);
	//return HookAPIByName("user32.dll", \
		"CreateWindowExW",\
		&g_HookInfo_CrtWinExW, \
		13, 12, NULL, NULL, NULL, CreateWindowExW_Ret_Sub_Proxy);
}

BOOL UnInitSPYDll()
{
	if (g_HookInfo_CrtWinExW.StubAddress)
	{
		UnInstallInline(&g_HookInfo_CrtWinExW);
		//UnHookAPI(&g_HookInfo_CrtWinExW);
	}
	return TRUE;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		InitSPYDll(hModule);
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		UnInitSPYDll();
		break;
	}
	return TRUE;
}