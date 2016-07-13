// MFCNIDAQReadDlg.h : header file
//

#if !defined(AFX_MFCNIDAQREADDLG_H__412D278E_DD96_11D3_A359_00C04F796B88__INCLUDED_)
#define AFX_MFCNIDAQREADDLG_H__412D278E_DD96_11D3_A359_00C04F796B88__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CMFCNIDAQReadDlg dialog

class CMFCNIDAQReadDlg : public CDialog
{
// Construction
public:
	CMFCNIDAQReadDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CMFCNIDAQReadDlg)
	enum { IDD = IDD_MFCNIDAQREAD_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMFCNIDAQReadDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CMFCNIDAQReadDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnConnect();
	afx_msg void OnPause();
	afx_msg void OnOk();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MFCNIDAQREADDLG_H__412D278E_DD96_11D3_A359_00C04F796B88__INCLUDED_)
