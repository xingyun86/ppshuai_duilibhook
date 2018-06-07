#pragma once
#include "stdafx.h"
#include "resource.h"
#include "WindowCapManage.h"

class CMainDlg : public CDialogImpl<CMainDlg>,
	public CUpdateUI<CMainDlg>, public CWinDataExchange<CMainDlg>,
	public CMessageFilter, public CIdleHandler
{
public:
	CMainDlg() :
		m_bCapture(FALSE),
		m_nHideSpy(BST_UNCHECKED)
	{

	}
public:
	enum { IDD = IDD_MAINDLG };

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		return CWindow::IsDialogMessage(pMsg);
	}

	virtual BOOL OnIdle()
	{
		UIUpdateChildWindows();
		return FALSE;
	}

	BEGIN_UPDATE_UI_MAP(CMainDlg)
		UPDATE_ELEMENT(IDC_CHECK_HIDE_SPY, UPDUI_CHILDWINDOW)
	END_UPDATE_UI_MAP()

	BEGIN_DDX_MAP(CMainDlg)
		DDX_CHECK(IDC_CHECK_HIDE_SPY, m_nHideSpy)
		DDX_TEXT(IDC_EDIT_CTR_NAME, m_strCtrlName)
		DDX_TEXT(IDC_EDIT_CTR_TEXT, m_strCtrlText)
		DDX_TEXT(IDC_EDIT_CTR_ADDR, m_strCtrlAddr)
	END_DDX_MAP()

	BEGIN_MSG_MAP_EX(CMainDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER_EX(IDOK, OnOk)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel)
		MSG_WM_LBUTTONUP(OnLButtonUp)
		MSG_WM_LBUTTONDOWN(OnLButtonDown)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MSG_WM_COPYDATA(OnCopyData)
	END_MSG_MAP()

private:
	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
	{
		CenterWindow();

		HICON hIcon = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON));
		SetIcon(hIcon, TRUE);
		HICON hIconSmall = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON));
		SetIcon(hIconSmall, FALSE);

		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->AddMessageFilter(this);
		pLoop->AddIdleHandler(this);

		UIAddChildWindowContainer(m_hWnd);

		CStatic iconBtn = GetDlgItem(IDC_STATIC_ICON);
		iconBtn.SetIcon(AtlLoadIcon(IDI_ICON_DOWN));
		SetWindowText(_T("DUILibSPY --by Debugee"));
		//UISetCheck(IDC_CHECK_HIDE_SPY, true);
		DoDataExchange(DDX_LOAD);

		ChangeWindowMessageFilterEx(m_hWnd, WM_COPYDATA, MSGFLT_ALLOW, 0);

		return TRUE;
	}
	void OnCancel(UINT uNotifyCode, int nID, CWindow wndCtl)
	{
		DestroyWindow();
		::PostQuitMessage(nID);
	}
	void OnOk(UINT uNotifyCode, int nID, CWindow wndCtl)
	{
		DestroyWindow();
		::PostQuitMessage(nID);
	}
	void OnLButtonDown(UINT nFlags, CPoint point)
	{
		CWindow ChildWnd = ChildWindowFromPoint(point);
		if (ChildWnd == GetDlgItem(IDC_STATIC_ICON))
		{
			m_bCapture = TRUE;
			DoDataExchange(DDX_SAVE);
			if (m_nHideSpy == BST_CHECKED)
				ShowWindow(SW_HIDE);//在SetCapture之前调用，不然会取消Capture
			SetCapture();

			CStatic iconBtn = GetDlgItem(IDC_STATIC_ICON);
			iconBtn.SetIcon(AtlLoadIcon(IDI_ICON_UP));
			CCursor hCur = AtlLoadCursor(IDC_CURSOR_DST);
			SetCursor(hCur);

			m_CaptureDrawManager.Init();
		}
	}
	void OnLButtonUp(UINT nFlags, CPoint point)
	{
		if (m_bCapture)
		{
			m_bCapture = FALSE;
			::ReleaseCapture();
			if (m_nHideSpy == BST_CHECKED)
				ShowWindow(SW_SHOW);
			CStatic iconBtn = GetDlgItem(IDC_STATIC_ICON);
			iconBtn.SetIcon(AtlLoadIcon(IDI_ICON_DOWN));

			m_CaptureDrawManager.RedrawForwardCapWindow();
			CWindow hWndCapture = m_CaptureDrawManager.GetCapWindow();

			if (isDUIlibWindow(hWndCapture))
			{
				//send wm_copydata for get control info
				CPoint pt;
				::GetCursorPos(&pt);
				COPYDATASTRUCT cpDataStuc = { 0 };
				cpDataStuc.lpData = &pt;
				cpDataStuc.dwData = cpDataStuc.cbData = sizeof(pt);
				::SendMessage(hWndCapture, WM_COPYDATA, (WPARAM)m_hWnd, (LPARAM)&cpDataStuc);
			}
		}
	}
	void OnMouseMove(UINT nFlags, CPoint point)
	{
		if (m_bCapture)
		{
			ClientToScreen(&point);
			CWindow hCapWnd = ::WindowFromPoint(point);
			while ((hCapWnd.GetWindowLong(GWL_STYLE) & WS_CHILD))
			{
				hCapWnd = hCapWnd.GetParent();
			}
			
			m_CaptureDrawManager.DrawCapWindow(hCapWnd);
		}
	}
	BOOL OnCopyData(CWindow wnd, PCOPYDATASTRUCT pCopyDataStruct)
	{
		if (NULL == pCopyDataStruct->lpData || 1 != pCopyDataStruct->dwData)
			return FALSE;
		
		CStringA  strData;
		strData.Append((LPSTR)pCopyDataStruct->lpData, pCopyDataStruct->cbData);
		Parser(CString(strData));
		DoDataExchange(DDX_LOAD);
		return TRUE;
	}
private:
	BOOL isDUIlibWindow(HWND hWnd)
	{
		DWORD dwProcId = 0;
		::GetWindowThreadProcessId(hWnd, &dwProcId);
		if (0 == dwProcId)
			return FALSE;

		void *pWinObject = reinterpret_cast<void *>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
		if (!pWinObject)
			return FALSE;

		HWND *phWnd = reinterpret_cast<HWND *>(((uintptr_t)pWinObject + sizeof(uintptr_t)));
		HANDLE hProcess = ::OpenProcess(PROCESS_VM_READ, FALSE, dwProcId);
		if (!hProcess)
			return FALSE;

		HWND hWndRead = NULL;
		DWORD dwHadReadBytes = 0;
		::ReadProcessMemory(hProcess, phWnd, &hWndRead, sizeof(hWndRead), &dwHadReadBytes);
		::CloseHandle(hProcess);

		if (dwHadReadBytes != sizeof(hWndRead) || 
			hWndRead != hWnd)
			return FALSE;

		return TRUE;
	}
	BOOL SetAttr(const CString& strKey, const CString& strValue)
	{
		BOOL bUpdated = TRUE;
		if (strKey == _T("ctrl_name"))
		{
			m_strCtrlName = strValue;
		}
		else if (strKey == _T("ctrl_text"))
		{
			m_strCtrlText = strValue;
		}
		else if (strKey == _T("ctrl_addr"))
		{
			m_strCtrlAddr = strValue;
		}
		else
		{
			bUpdated = FALSE;
		}

		return bUpdated;
	}
	BOOL Parser(const CString& strHeader)
	{
		BOOL bUpdated = FALSE;
		CSimpleArray<CString> arrLines;
		Split(strHeader, _T("\r\n"), arrLines);
		for (int i = 0; i < arrLines.GetSize(); i++)
		{
			CString strKey, strValue;
			if (Split(arrLines[i], _T("="), strKey, strValue))
			{
				if (SetAttr(strKey, strValue))
					bUpdated = TRUE;
			}
		}
		return bUpdated;
	}
	BOOL Split(const CString& strLine, const CString& strSep, CString& strKey, CString& strVal)
	{
		int nPos = strLine.Find(strSep);
		if (-1 == nPos)
			return FALSE;
		
		strKey = strLine.Mid(0, nPos).Trim(_T(' '));
		strVal = strLine.Mid(nPos + strSep.GetLength()).Trim(_T(' '));

		return TRUE;
	}
	VOID Split(const CString& strHeader, const CString& strSep, CSimpleArray<CString> &arrLines)
	{
		CString strTemp = strHeader;
		while (true)
		{
			int nPos = strTemp.Find(strSep);
			if (nPos == -1)
			{
				if (!strTemp.Trim(_T(' ')).IsEmpty())
					arrLines.Add(strTemp);

				break;
			}

			CString strLine = strTemp.Mid(0, nPos);
			strTemp = strTemp.Mid(nPos + strSep.GetLength());

			if (!strLine.Trim(_T(' ')).IsEmpty())
				arrLines.Add(strLine);
		}
		
		return;
	}
private:
	BOOL m_bCapture;
	int m_nHideSpy;
	CString m_strCtrlName;
	CString m_strCtrlText;
	CString m_strCtrlAddr;
	CWindowCapManage m_CaptureDrawManager;
};