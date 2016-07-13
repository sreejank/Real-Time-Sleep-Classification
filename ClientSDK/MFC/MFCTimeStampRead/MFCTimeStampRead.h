// MFCTimeStampRead.h : main header file for the MFCTIMESTAMPREAD application
//

#if !defined(AFX_MFCTIMESTAMPREAD_H__412D277E_DD96_11D3_A359_00C04F796B88__INCLUDED_)
#define AFX_MFCTIMESTAMPREAD_H__412D277E_DD96_11D3_A359_00C04F796B88__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CMFCTimeStampReadApp:
// See MFCTimeStampRead.cpp for the implementation of this class
//

class CMFCTimeStampReadApp : public CWinApp
{
public:
	CMFCTimeStampReadApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMFCTimeStampReadApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CMFCTimeStampReadApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MFCTIMESTAMPREAD_H__412D277E_DD96_11D3_A359_00C04F796B88__INCLUDED_)
