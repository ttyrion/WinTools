
// FocusRecorderDlg.h: 头文件
//

#pragma once


// CFocusRecorderDlg 对话框
class CFocusRecorderDlg : public CDialogEx
{
// 构造
public:
	CFocusRecorderDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FOCUSRECORDER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
	void InitializeFocusHook();
	void ShutdownFocusHook();

public:
	static SRWLOCK m_RWLock;
	static CEdit m_FocusLogCtrl;
	static CString m_FocusLog;
// 实现
protected:
	HICON m_hIcon;
	HWINEVENTHOOK m_EventHook;
	
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedClear();
	afx_msg void OnBnClickedCancel();
};
