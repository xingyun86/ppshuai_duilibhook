#pragma once
#include "stdafx.h"
#include <UIlib.h>

using namespace DuiLib;

class CWinSubclass : public CWindowImpl<CWinSubclass>
{
	DECLARE_WND_CLASS(_T("DUILibSPYSubClass"))

	BEGIN_MSG_MAP_EX(CWinSubclass)
		MSG_WM_COPYDATA(OnCopyData)
	END_MSG_MAP()
public:
	CWinSubclass()
	{
	}
	virtual ~CWinSubclass()
	{
	}
	virtual void OnFinalMessage(_In_ HWND)
	{
		::InterlockedDecrement(&gs_m_nSubCount);
		delete this;
	}
	static BOOL SubclassDUILibWindow(HWND hWnd)
	{
		if (!isDUIlibWindow(hWnd))
			return FALSE;

		CWinSubclass *pSubclassWnd = new CWinSubclass;
		if (!pSubclassWnd)
			return FALSE;

		pSubclassWnd->SubclassWindow(hWnd);
		::InterlockedIncrement(&gs_m_nSubCount);

		return TRUE;
	}
protected:
	BOOL OnCopyData(CWindow wnd, PCOPYDATASTRUCT pCopyDataStruct)
	{
		if (pCopyDataStruct->cbData != sizeof(CPoint) || 
			NULL == pCopyDataStruct->lpData)
			return FALSE;

		CPoint pt = *(POINT *)pCopyDataStruct->lpData;	
		::ScreenToClient(m_hWnd, &pt);
	
		CPaintManagerUI *pm = GetPaintManager(m_hWnd);
		if (NULL == pm)
			return FALSE;
		
		CControlUI *pControlFind = FindControlFromPoint(GetRootControl(pm), pt);
		if (pControlFind)
		{
			CString strControlAddr;
			strControlAddr.Format(_T("%08x"), pControlFind);
			CString strControlName = GetControlName(pControlFind);
			CString strControlText = GetControlText(pControlFind);
			CStringA strData = CStringA("ctrl_name=") + CStringA(strControlName) + "\r\n";
			strData += CStringA("ctrl_text=") + CStringA(strControlText) + "\r\n";
			strData += CStringA("ctrl_addr=") + CStringA(strControlAddr) + "\r\n";
			strData += "\r\n";

			COPYDATASTRUCT cpDataStuc = { 0 };
			cpDataStuc.lpData = (LPVOID)strData.GetString();
			cpDataStuc.dwData = 1;
			cpDataStuc.cbData = strData.GetLength();
			::SendMessage(wnd, WM_COPYDATA, (WPARAM)m_hWnd, (LPARAM)&cpDataStuc);
		}

		return TRUE;
	}
private:
	//Wechat.exe v2.4.5.1 installpack(MD5:CB900B494738A10F69F79696D57196FE) v2.5.5.1000 v2.5.5.0
	//Wechat.exe v2.4.5.1 installpack(MD5:0CDD833ED1E293347C7D5687BA735EDD) v2.6.1.1000 v2.5.0.0

	//get offset hwnd->WindowImplBase->vtable[5](OnFinalMessage)
	//get findcontrol vtable offset hwnd->IMessageFilterUI[0]<-CPaintManagerUI::PreMessageHandler
	static BOOL isDUIlibWindow(HWND hWnd)
	{
		return GetWinObjectPtr(hWnd) != NULL ? TRUE : FALSE;
	}
	static void *GetWinObjectPtr(HWND hWnd)
	{
		void *pwinObject = reinterpret_cast<void *>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
		if (!pwinObject)
			return NULL;

		HWND *phWnd = reinterpret_cast<HWND *>(((uintptr_t)pwinObject + sizeof(uintptr_t)));
		if (IsBadReadPtr(phWnd, sizeof(HWND)))
		{
			return NULL;
		}

		if (*phWnd != hWnd)
			return NULL;

		return pwinObject;
	}
	static CPaintManagerUI *GetPaintManager(HWND hWnd)
	{
		void *pwinObject = GetWinObjectPtr(hWnd);
		if (!pwinObject)
			return NULL;

		return (CPaintManagerUI *)((void **)pwinObject + 0x2c / 4);
	}
	static CControlUI *GetRootControl(CPaintManagerUI *pm)
	{
		return (CControlUI *)*((void **)pm + 0x54 / 4);
	}
	static CString GetControlName(CControlUI * pControlUI)
	{
		LPCWSTR lpwstrName = (LPCWSTR)*((void **)pControlUI + 0xCC / 4);

		return lpwstrName;
	}
	static CString GetControlText(CControlUI * pControlUI)
	{
		LPCWSTR lpwstrText = (LPCWSTR)*((void **)pControlUI + 0x1B8 / 4);

		return lpwstrText;
	}
	static RECT GetControlPos(CControlUI * pControlUI)
	{
		return *(RECT *)((void **)pControlUI + 0x154 / 4);
	}
	static CControlUI* CALLBACK __FindControlFromPoint(CControlUI* pThis, LPVOID pData)
	{
		LPPOINT pPoint = static_cast<LPPOINT>(pData);
		RECT rcRect = GetControlPos(pThis);
		return ::PtInRect(&rcRect, *pPoint) ? pThis : NULL;
	}
	static CControlUI *FindControl(CControlUI * pControlUI, FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags)
	{
		CControlUI *pControlFind = NULL;
		__asm
		{
			pushad
			pushfd
			mov ecx, pControlUI
			mov eax, [ecx]
			push uFlags
			push pData
			push Proc
			//call[eax + 0x100] Wechat.exe v2.4.5.1 installpack v2.5.5.1000 v2.5.5.0
			call [eax + 0x104] //Wechat.exe v2.4.5.1 installpack v2.6.1.1000 v2.5.0.0
			mov pControlFind, eax
			popfd
			popad
		}

		return pControlFind;
	}
	static CControlUI* FindControlFromPoint(CControlUI* pParent, POINT pt)
	{
		return FindControl(pParent, __FindControlFromPoint, &pt, UIFIND_VISIBLE | UIFIND_HITTEST | UIFIND_TOP_FIRST);
	}
private:
	static volatile long gs_m_nSubCount;
};