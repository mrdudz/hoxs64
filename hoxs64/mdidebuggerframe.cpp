#include <windows.h>
#include <windowsx.h>
#include "dx_version.h"
#include <d3d9.h>
#include <d3dx9core.h>
#include <dinput.h>
#include <dsound.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>
#include <assert.h>
#include "boost2005.h"
#include "user_message.h"
#include "defines.h"
#include "mlist.h"
#include "carray.h"
#include "cevent.h"
#include "CDPI.h"
#include "bits.h"
#include "util.h"
#include "utils.h"
#include "register.h"
#include "errormsg.h"
#include "hconfig.h"
#include "appstatus.h"
#include "dxstuff9.h"
#include "c6502.h"
#include "ram64.h"
#include "cpu6510.h"
#include "cia6526.h"
#include "cia1.h"
#include "cia2.h"
#include "vic6569.h"
#include "tap.h"
#include "filter.h"
#include "sid.h"
#include "sidfile.h"
#include "d64.h"
#include "d1541.h"
#include "via6522.h"
#include "via1.h"
#include "via2.h"
#include "diskinterface.h"
#include "t64.h"
#include "C64.h"
#include "monitor.h"
#include "edln.h"
#include "wpanel.h"
#include "wpanelmanager.h"
#include "wpcbreakpoint.h"
#include "disassemblyreg.h"
#include "disassemblyeditchild.h"
#include "disassemblychild.h"
#include "disassemblyframe.h"
#include "mdidebuggerframe.h"
#include "resource.h"

#define TOOLBUTTON_WIDTH_96 (16)
#define TOOLBUTTON_HEIGHT_96 (16)


const TCHAR CMDIDebuggerFrame::ClassName[] = TEXT("Hoxs64MDIDebuggerFrame");
const TCHAR CMDIDebuggerFrame::MenuName[] = TEXT("MENU_MDI_DEBUGGER");

//TCHAR CMDIDebuggerFrame::ChildClassName[] = TEXT("Hoxs64MDIDebuggerChild");

const ImageInfo CMDIDebuggerFrame::TB_ImageList[] = 
{
	{IDB_DEBUGGERTRACE, IDB_DEBUGGERTRACEMASK},
	{IDB_DEBUGGERTRACEFRAME, IDB_DEBUGGERTRACEFRAMEMASK},
	{IDB_DEBUGGERSTOP, IDB_DEBUGGERSTOPMASK}
};

const ButtonInfo CMDIDebuggerFrame::TB_StepButtons[] = 
{
	{0, TEXT("Trace"), ID_STEP_TRACE},
	{1, TEXT("Trace Frame"), ID_STEP_TRACEFRAME},
	{2, TEXT("Stop"), ID_STEP_STOP}
};

CMDIDebuggerFrame::CMDIDebuggerFrame()
{
	m_hWndMDIClient = NULL;
	m_hWndRebar = NULL;
	m_hWndTooBar = NULL;
	m_hImageListToolBarNormal = NULL;
	m_bIsCreated = false;
	cfg = NULL;
	appStatus = NULL;
	c64 = NULL;
	m_monitorCommand = NULL;
}

CMDIDebuggerFrame::~CMDIDebuggerFrame()
{
	Cleanup();
}

HRESULT CMDIDebuggerFrame::RegisterClass(HINSTANCE hInstance)
{
WNDCLASSEX wc; 
 
    // Register the frame window class. 
	ZeroMemory(&wc, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);

    wc.style         = 0; 
    wc.lpfnWndProc   = (WNDPROC)::WindowProc; 
    wc.cbClsExtra    = 0; 
    wc.cbWndExtra    = sizeof(CMDIDebuggerFrame *); 
    wc.hInstance     = hInstance; 
    wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON_BIG)); 
	wc.hIconSm       = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON_SMALL)); 
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW); 
    wc.hbrBackground = (HBRUSH) (COLOR_APPWORKSPACE + 1); 
    wc.lpszMenuName  = MenuName; 
    wc.lpszClassName = ClassName; 
    if (!RegisterClassEx(&wc) ) 
        return E_FAIL; 
 
    //// Register the MDI child window class.  
    //wc.lpfnWndProc   = (WNDPROC) MPMDIChildWndProc; 
    //wc.hIcon         = LoadIcon(hInst, IDNOTE); 
    //wc.lpszMenuName  = (LPCTSTR) NULL; 
    //wc.cbWndExtra    = CBWNDEXTRA; 
    //wc.lpszClassName = ChildClassName; 
 
    //if (!RegisterClassEx(&wc)) 
    //    return FALSE; 
 
    return S_OK; 

};

void CMDIDebuggerFrame::OnTrace(void *sender, EventArgs& e)
{
	SetMenuState();
}

void CMDIDebuggerFrame::OnShowDevelopment(void *sender, EventArgs& e)
{
	SetMenuState();
}

void CMDIDebuggerFrame::SetMenuState()
{
	if (!m_monitorCommand)
		return ;
	if (!m_hWnd)
		return ;
	HMENU hMenu = GetMenu(m_hWnd);
	if (!hMenu)
		return ;
	UINT state;
	UINT stateOpp;
	UINT stateTb;
	UINT stateTbOpp;
	if (m_monitorCommand->IsRunning())
	{
		state = MF_DISABLED | MF_GRAYED;
		stateOpp = MF_ENABLED;
		stateTb = 0;
		stateTbOpp = TBSTATE_ENABLED;
	}
	else
	{
		state = MF_ENABLED;
		stateOpp = MF_DISABLED | MF_GRAYED;
		stateTb = TBSTATE_ENABLED;
		stateTbOpp = 0;
	}
	EnableMenuItem(hMenu, ID_STEP_TRACE, MF_BYCOMMAND | state);
	EnableMenuItem(hMenu, ID_STEP_TRACEFRAME, MF_BYCOMMAND | state);
	EnableMenuItem(hMenu, ID_STEP_STOP, MF_BYCOMMAND | stateOpp);

	if (m_hWndTooBar!=NULL)
	{
		SendMessage(m_hWndTooBar, TB_SETSTATE, ID_STEP_TRACE, stateTb);
		SendMessage(m_hWndTooBar, TB_SETSTATE, ID_STEP_TRACEFRAME, stateTb);
		SendMessage(m_hWndTooBar, TB_SETSTATE, ID_STEP_STOP, stateTbOpp);
	}
}

HWND CMDIDebuggerFrame::Create(HINSTANCE hInstance, HWND hWndParent, const TCHAR title[], int x,int y, int w, int h, HMENU hMenu)
{
	return CVirWindow::CreateVirWindow(0L, ClassName, title, WS_OVERLAPPED | WS_SIZEBOX | WS_SYSMENU, x, y, w, h, hWndParent, hMenu, hInstance);
}

HRESULT CMDIDebuggerFrame::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HRESULT hr;
	hr = CreateMDIToolBars();
	if (FAILED(hr))
		return hr;
	HWND hWndMdiClient = CreateMDIClientWindow(IDC_MAIN_MDI, IDM_WINDOWCHILD);
	if (hWndMdiClient==NULL)
		return E_FAIL;

	hr = m_WPanelManager.Init(this->GetHinstance(), this, m_hWndRebar);
	if (FAILED(hr))
		return hr;

	WpcBreakpoint *pWin = new WpcBreakpoint();	
	hr = pWin->Init(&m_monitorC64);
	if (SUCCEEDED(hr))
	{
		hr = m_WPanelManager.CreateNewPanel(WPanel::InsertionStyle::Bottom, pWin);
		if (SUCCEEDED(hr))
		{
			pWin->m_AutoDelete = true;
			pWin = NULL;
		}
	}
	if (pWin)
	{
		delete pWin;
		pWin = NULL;
	}
	m_bIsCreated = SUCCEEDED(hr);
	return hr;
}

void CMDIDebuggerFrame::OnClose(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HWND hWndParent = ::GetParent(hWnd);
	if (hWndParent)
	{
		::SetForegroundWindow(hWndParent);
	}
}

void CMDIDebuggerFrame::OnDestroy(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (::IsWindow(m_hWnd))
	{
		if (m_bIsCreated)
			CConfig::SaveMDIWindowSetting(m_hWnd);
	}
	m_bIsCreated = false;
}

void CMDIDebuggerFrame::OnMove(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
}

void CMDIDebuggerFrame::OnSize(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (hWnd != this->m_hWnd)
		return;
	if (wParam == SIZE_MAXHIDE || wParam == SIZE_MAXSHOW)
		return;
	
	if (wParam == SIZE_MINIMIZED)
		return;

	int w = LOWORD(lParam);
	int h = HIWORD(lParam);

	RECT rcRootPanel;
	SetRect(&rcRootPanel, 0, 0, w, h);

	if (m_hWndRebar != NULL)
	{
		RECT rcAbsRebar;
		BOOL br = GetWindowRect(m_hWndRebar, &rcAbsRebar);
		if (br)
		{
			int heightRebar = rcAbsRebar.bottom - rcAbsRebar.top;

			SetWindowPos(m_hWndRebar, 0, 0, 0, w, heightRebar, SWP_NOREPOSITION | SWP_NOZORDER);

			SetRect(&rcRootPanel, 0, heightRebar, w, h);
		}
	}

	this->m_WPanelManager.SizePanels(hWnd, rcRootPanel.left, rcRootPanel.top, rcRootPanel.right - rcRootPanel.left, rcRootPanel.bottom - rcRootPanel.top);
}

HRESULT CMDIDebuggerFrame::CreateMDIToolBars()
{
	if (m_hImageListToolBarNormal == NULL)
	{
		m_hImageListToolBarNormal = CreateImageListNormal(m_hWnd);
		if (m_hImageListToolBarNormal == NULL)
			return E_FAIL;
	}

	m_hWndTooBar = G::CreateToolBar(m_hInst, m_hWnd, ID_TOOLBAR, m_hImageListToolBarNormal, TB_StepButtons, _countof(TB_StepButtons), m_dpi.ScaleX(TOOLBUTTON_WIDTH_96), m_dpi.ScaleY(TOOLBUTTON_HEIGHT_96));
	if (m_hWndTooBar == NULL)
		return E_FAIL;
	m_hWndRebar = G::CreateRebar(m_hInst, m_hWnd, m_hWndTooBar, ID_RERBAR, IDB_REBARBKGND1);
	if (m_hWndRebar == NULL)
		return E_FAIL;
	return S_OK;
}

HIMAGELIST CMDIDebuggerFrame::CreateImageListNormal(HWND hWnd)
{
	int tool_dx = m_dpi.ScaleX(TOOLBUTTON_WIDTH_96);
	int tool_dy = m_dpi.ScaleY(TOOLBUTTON_HEIGHT_96);
	return G::CreateImageListNormal(m_hInst, hWnd, tool_dx, tool_dy, TB_ImageList, _countof(TB_ImageList));
}

void CMDIDebuggerFrame::ShowDebugCpuC64(bool bSeekPC)
{
	m_debugCpuC64.Show(bSeekPC);
}

void CMDIDebuggerFrame::ShowDebugCpuDisk(bool bSeekPC)
{
	m_debugCpuDisk.Show(bSeekPC);
}

void CMDIDebuggerFrame::OnGetMinMaxSizeInfo(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
int w,h;
MINMAXINFO *pMinMax = (MINMAXINFO *)lParam;
	this->GetMinWindowSize(w, h);
	pMinMax->ptMinTrackSize.x = w;
	pMinMax->ptMinTrackSize.y = h;
}

void CMDIDebuggerFrame::GetMinWindowSize(int &w, int &h)
{
	w = GetSystemMetrics(SM_CXSIZEFRAME) * 2;
	h = GetSystemMetrics(SM_CYSIZEFRAME) * 2 + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYMENU);
	
	UINT heightRebar = 0;
	if (m_hWndRebar!=0)
	{
		UINT t =(UINT)SendMessage(m_hWndRebar, RB_GETBARHEIGHT, 0 , 0);
		heightRebar = t;
	}

	h += heightRebar;
}

bool CMDIDebuggerFrame::OnCommand(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
int wmId, wmEvent;
	wmId    = LOWORD(wParam);
	wmEvent = HIWORD(wParam);
	switch (wmId) 
	{
	case ID_DEBUG_CPUC64:
		ShowDebugCpuC64(false);
		return true;
	case ID_DEBUG_CPUDISK:
		ShowDebugCpuDisk(false);
		return true;
	case ID_STEP_TRACEFRAME:
		if (!m_monitorCommand)
			return false;
		if (m_monitorCommand->IsRunning())
			return false;
		m_monitorCommand->TraceFrame();
		m_monitorCommand->UpdateApplication();
		return true;
	case ID_STEP_TRACE:
		if (!m_monitorCommand)
			return false;
		this->m_monitorCommand->Trace();
		return true;
	case ID_FILE_MONITOR:
	case ID_STEP_STOP:
		if (!m_monitorCommand)
			return false;
		this->m_monitorCommand->ShowDevelopment();
		return true;
	default:
		return false;
	}
}

LRESULT CMDIDebuggerFrame::OnSetCursor(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
bool bHandled = m_WPanelManager.Splitter_OnSetCursor(hWnd, uMsg, wParam, lParam);
	if(bHandled)
		return TRUE;
	else
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CMDIDebuggerFrame::OnLButtonDown(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	bool bHandled = m_WPanelManager.Splitter_OnLButtonDown(hWnd, uMsg, wParam, lParam);
	if(bHandled)
		return 0;
	else
		return DefWindowProc(hWnd, uMsg, wParam, lParam);	
}

LRESULT CMDIDebuggerFrame::OnMouseMove(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	bool bHandled = m_WPanelManager.Splitter_OnMouseMove(hWnd, uMsg, wParam, lParam);
	if(bHandled)
		return 0;
	else
		return DefWindowProc(hWnd, uMsg, wParam, lParam);	
}

LRESULT CMDIDebuggerFrame::OnLButtonUp(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	bool bHandled = m_WPanelManager.Splitter_OnLButtonUp(hWnd, uMsg, wParam, lParam);
	if(bHandled)
		return 0;
	else
		return DefWindowProc(hWnd, uMsg, wParam, lParam);	
}


LRESULT CMDIDebuggerFrame::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
HRESULT hr;

	switch(uMsg)
	{
	case WM_CREATE:
		hr = OnCreate(uMsg, wParam, lParam);
		if (FAILED(hr))
			return -1;
		ShowWindow(m_hWndMDIClient, SW_SHOW); 
		return 0;
	case WM_COMMAND:
		if (OnCommand(hWnd, uMsg, wParam, lParam))
			return 0;
		else
			return ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
	case WM_SETCURSOR:
		return OnSetCursor(hWnd, uMsg, wParam, lParam);
	case WM_LBUTTONDOWN:
		return OnLButtonDown(hWnd, uMsg, wParam, lParam);
	case WM_MOUSEMOVE:
		return OnMouseMove(hWnd, uMsg, wParam, lParam);
	case WM_LBUTTONUP:
		return OnLButtonUp(hWnd, uMsg, wParam, lParam);
	case WM_MOVE:
		OnMove(hWnd, uMsg, wParam, lParam);
		return 0;
	case WM_CLOSE:
		if (m_monitorCommand)
			m_monitorCommand->Resume();
		OnClose(m_hWnd, uMsg, wParam, lParam);
		return ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
	case WM_DESTROY:
		OnDestroy(m_hWnd, uMsg, wParam, lParam);
		return ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
	case WM_SIZE:
		OnSize(hWnd, uMsg, wParam, lParam);
		return 0;
	case WM_MONITOR_BREAK_CPU64:
		OnBreakCpu64(m_hWnd, uMsg, wParam, lParam);
		return 0;
	case WM_MONITOR_BREAK_CPUDISK:
		OnBreakCpuDisk(m_hWnd, uMsg, wParam, lParam);
		return 0;
	case WM_ENTERMENULOOP:
		m_monitorCommand->SoundOff();
		return 0;
	case WM_EXITMENULOOP:
		m_monitorCommand->SoundOn();
		return 0;
	case WM_ENTERSIZEMOVE:
		m_monitorCommand->SoundOff();
		return 0;
	case WM_EXITSIZEMOVE:
		m_monitorCommand->SoundOn();
		return 0;
	default:
		return ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
	}
	return 0;	
}

void CMDIDebuggerFrame::OnBreakCpu64(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	m_debugCpuC64.Show(true);
}

void CMDIDebuggerFrame::OnBreakCpuDisk(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	m_debugCpuDisk.Show(true);
}

HRESULT CMDIDebuggerFrame::Init(IMonitorCommand *monitorCommand, CConfig *cfg, CAppStatus *appStatus, C64 *c64)
{
HRESULT hr;
	this->cfg = cfg;
	this->appStatus = appStatus;
	this->c64 = c64;
	this->m_monitorCommand = monitorCommand;

	IMonitorCpu *monitorMainCpu = &c64->cpu;
	IMonitorCpu *monitorDiskCpu = &c64->diskdrive.cpu;
	IMonitorVic *monitorVic = &c64->vic;
	IMonitorDisk *monitorDisk = &c64->diskdrive;

	m_monitorC64.Init(CPUID_MAIN, monitorMainCpu, monitorDiskCpu, monitorVic, monitorDisk);
	m_monitorDisk.Init(CPUID_DISK, monitorMainCpu, monitorDiskCpu, monitorVic, monitorDisk);

	do
	{
		hr = m_debugCpuC64.Init(this, monitorCommand, &m_monitorC64, TEXT("C64 - cpu"));
		if (FAILED(hr))
			break;

		hr = m_debugCpuDisk.Init(this, monitorCommand, &m_monitorDisk, TEXT("Disk - cpu"));
		if (FAILED(hr))
			break;

		hr = AdviseEvents();
		if (FAILED(hr))
			break;

		hr = S_OK;
	} while (false);
	if (FAILED(hr))
	{
		Cleanup();
		return hr;
	}
	else
	{
		return S_OK;
	}
};

void CMDIDebuggerFrame::Cleanup()
{
	UnadviseEvents();

	if (m_hImageListToolBarNormal != NULL)
	{
		ImageList_Destroy(m_hImageListToolBarNormal);
		m_hImageListToolBarNormal = NULL;
	}
}

HRESULT CMDIDebuggerFrame::AdviseEvents()
{
	HRESULT hr;
	HSink hs;
	hr = S_OK;
	do
	{

		hs = m_monitorCommand->EsShowDevelopment.Advise((CMDIDebuggerFrame_EventSink_OnShowDevelopment *)this);
		if (hs == NULL)
		{
			hr = E_FAIL;
			break;
		}
		hs = m_monitorCommand->EsTrace.Advise((CMDIDebuggerFrame_EventSink_OnTrace *)this);
		if (hs == NULL)
		{
			hr = E_FAIL;
			break;
		}
		hr = S_OK;
	} while (false);
	return hr;
}

void CMDIDebuggerFrame::UnadviseEvents()
{
	((CMDIDebuggerFrame_EventSink_OnShowDevelopment *)this)->UnadviseAll();
	((CMDIDebuggerFrame_EventSink_OnTrace *)this)->UnadviseAll();
}
