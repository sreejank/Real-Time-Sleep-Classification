// MFCSimpleRead.h : main header file for the MFCSIMPLEREAD application
//

#if !defined(AFX_MFCSIMPLEREAD_H__412D2770_DD96_11D3_A359_00C04F796B88__INCLUDED_)
#define AFX_MFCSIMPLEREAD_H__412D2770_DD96_11D3_A359_00C04F796B88__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CMFCSimpleReadApp:
// See MFCSimpleRead.cpp for the implementation of this class
//

class CMFCSimpleReadApp : public CWinApp
{
public:
	CMFCSimpleReadApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMFCSimpleReadApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CMFCSimpleReadApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MFCSIMPLEREAD_H__412D2770_DD96_11D3_A359_00C04F796B88__INCLUDED_)
