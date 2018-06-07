#pragma once
#include "stdafx.h"

class CWindowCapManage
{
public:
	CWindowCapManage()
	{
		Init();
	}
	~CWindowCapManage()
	{

	}
	void Init()
	{
		m_hWndForward = NULL;
		m_bDrawed = FALSE;
	}
	HWND GetCapWindow()
	{
		return m_hWndForward;
	}
	void DrawCapWindow(HWND hWnd)
	{
		ATLASSERT(::IsWindow(hWnd));
		if (hWnd != m_hWndForward)
		{
			if (m_hWndForward)
				RedrawForwardCapWindow();
			m_hWndForward = hWnd;
			m_bDrawed = FALSE;
		}
		if (!m_bDrawed)
		{
			DrawWindowRect(hWnd);
			m_bDrawed = TRUE;
		}
	}
	void RedrawForwardCapWindow()
	{
		/*
		if (!m_hWndForward) return;
		::RedrawWindow(m_hWndForward,
			NULL, NULL,
			RDW_INTERNALPAINT | RDW_INVALIDATE | RDW_ERASENOW |
			RDW_UPDATENOW | RDW_ERASE | RDW_FRAME | RDW_ALLCHILDREN
			);
			*/

	}
private:
	void DrawWindowRect(HWND hWnd)
	{
		/*
		CWindow hWndCap = hWnd;
		CRect capRect;
		hWndCap.GetWindowRect(&capRect);
		CWindowDC hdc = hWndCap;
		CPen hpen;
		hpen.CreatePen(PS_SOLID, 5, RGB(255, 0, 0));
		CPenHandle hPenOld = hdc.SelectPen(hpen);
		hdc.MoveTo(0, 0);
		hdc.LineTo(capRect.Width(), 0);
		hdc.LineTo(capRect.Width(), capRect.Height());
		hdc.LineTo(0, capRect.Height());
		hdc.LineTo(0, 0);
		hdc.SelectPen(hPenOld);
		*/
		for (int i = 0; i < 6; i++)
		{
			DrawWindowNOT(hWnd);
			Sleep(110);
		}

	}

	void DrawWindowNOT(HWND hWnd)
	{
		int nxBorder = GetSystemMetrics(SM_CXBORDER);
		int nyBorder = GetSystemMetrics(SM_CYBORDER);
		int nxFrame = GetSystemMetrics(SM_CXFRAME);
		int nyFrame = GetSystemMetrics(SM_CYFRAME);
		int nxScreen = GetSystemMetrics(SM_CXSCREEN);
		int nyScreen = GetSystemMetrics(SM_CYSCREEN);

		HDC hDC = GetWindowDC(hWnd);
		SetROP2(hDC, R2_NOT);
		HPEN hPen = CreatePen(PS_INSIDEFRAME, nxBorder * 3, RGB(0, 0, 0));
		HPEN hPenOld = (HPEN)SelectObject(hDC, hPen);
		HBRUSH hBrushOld = (HBRUSH)SelectObject(hDC, GetStockObject(NULL_BRUSH));

		HRGN hRgn = CreateRectRgn(0, 0, 0, 0);
		if (0 == GetWindowRgn(hWnd, hRgn))
		{
			RECT rect = { 0 };
			GetWindowRect(hWnd, &rect);
			if (IsZoomed(hWnd))
			{
				Rectangle(hDC, nxFrame, nyFrame, nxFrame + nxScreen, nyFrame + nyScreen);
			}
			else
			{
				Rectangle(hDC, 0, 0, rect.right - rect.left, rect.bottom - rect.top);
			}

		}
		else
		{
			HBRUSH hBrushRgn = CreateHatchBrush(HS_DIAGCROSS, GetSysColor(COLOR_WINDOWFRAME));
			FrameRgn(hDC, hRgn, hBrushRgn, nxBorder * 3, nyBorder * 3);
			DeleteObject(hBrushRgn);
		}

		DeleteObject(hRgn);
		SelectObject(hDC, hBrushOld);
		SelectObject(hDC, hPenOld);
		DeleteObject(hPen);
		ReleaseDC(hWnd, hDC);

		return;
	};
private:
	HWND m_hWndForward;
	BOOL m_bDrawed;
};