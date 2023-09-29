
// FocusRecorderDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "FocusRecorder.h"
#include "FocusRecorderDlg.h"
#include "afxdialogex.h"
#include <cstring>
#include <cwchar>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CFocusRecorderDlg 对话框

SRWLOCK CFocusRecorderDlg::m_RWLock;
CEdit CFocusRecorderDlg::m_FocusLogCtrl;
CString CFocusRecorderDlg::m_FocusLog;

// Callback function that handles events.
//
void CALLBACK HandleWinEvent(HWINEVENTHOOK hook, DWORD event, HWND hwnd,
	LONG idObject, LONG idChild,
	DWORD dwEventThread, DWORD dwmsEventTime)
{
	if (event != EVENT_OBJECT_FOCUS) {
		return;
	}

	// ::AcquireSRWLockExclusive(&CFocusRecorderDlg::m_RWLock);
	IAccessible* pAcc = NULL;
	VARIANT varChild;
	HRESULT hr = AccessibleObjectFromEvent(hwnd, idObject, idChild, &pAcc, &varChild);
	if ((hr == S_OK) && (pAcc != NULL))
	{
		BSTR bstrName;
		pAcc->get_accName(varChild, &bstrName);

		DWORD pid;
		DWORD tid = ::GetWindowThreadProcessId(hwnd, &pid);
		if (tid != dwEventThread) {
			::MessageBox(NULL, L"", L"", MB_OK);
		}

		SYSTEMTIME st;
		::GetLocalTime(&st);

		WCHAR str[32] = { 0 };
		wsprintf(str, L"%04d/%02d/%02d %02d:%02d:%02d.%03d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

		WCHAR buf[128] = { 0 };
		wsprintf(buf, L"Focus: handle=%X pid=%d tid=%d accName=%s time=%s\r\n", hwnd, pid, dwEventThread, bstrName, str);

		CFocusRecorderDlg::m_FocusLog += buf;
		CFocusRecorderDlg::m_FocusLogCtrl.SetWindowText(CFocusRecorderDlg::m_FocusLog);
		
		SysFreeString(bstrName);
		pAcc->Release();
	}

	// ::ReleaseSRWLockExclusive(&CFocusRecorderDlg::m_RWLock);
}

CFocusRecorderDlg::CFocusRecorderDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_FOCUSRECORDER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CFocusRecorderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_LOG, m_FocusLogCtrl);
}

BEGIN_MESSAGE_MAP(CFocusRecorderDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CFocusRecorderDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCLEAR, &CFocusRecorderDlg::OnBnClickedClear)
	ON_BN_CLICKED(IDCANCEL, &CFocusRecorderDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CFocusRecorderDlg 消息处理程序

BOOL CFocusRecorderDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	InitializeFocusHook();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CFocusRecorderDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CFocusRecorderDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CFocusRecorderDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CFocusRecorderDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnOK();
}

// Initializes COM and sets up the event hook.
//
void CFocusRecorderDlg::InitializeFocusHook()
{
	InitializeSRWLock(&m_RWLock);
	CoInitialize(NULL);
	m_EventHook = SetWinEventHook(
		EVENT_OBJECT_FOCUS, EVENT_OBJECT_FOCUS,  // Range of events (4 to 5).
		NULL,                                          // Handle to DLL.
		HandleWinEvent,                                // The callback.
		0, 0,              // Process and thread IDs of interest (0 = all)
		WINEVENT_OUTOFCONTEXT /* | WINEVENT_SKIPOWNPROCESS*/); // Flags.
}

// Unhooks the event and shuts down COM.
//
void CFocusRecorderDlg::ShutdownFocusHook()
{
	UnhookWinEvent(m_EventHook);
	CoUninitialize();
}


void CFocusRecorderDlg::OnBnClickedClear()
{
	// TODO: 在此添加控件通知处理程序代码
	CFocusRecorderDlg::m_FocusLog = L"";
	CFocusRecorderDlg::m_FocusLogCtrl.SetWindowText(L"");
}


void CFocusRecorderDlg::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	ShutdownFocusHook();
	CDialogEx::OnCancel();
}
