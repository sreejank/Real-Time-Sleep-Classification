///////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   MFCSimpleReadDlg.cpp
//
///////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   (c) 1999-2015 Plexon Inc. Dallas Texas 75206 
//   www.plexon.com
//
//   This code is provided for users of Plexon products. If you copy, disseminate, reproduce,
//   post or archive this code, please do not remove or alter this notice or the copyright above.
//   All sample code and libraries are provided as-is and Plexon cannot be responsible for errors 
//   or consequential damage caused by its use, including user code derived from or based on this code.  
//
///////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   Simple dialog-based MFC app that reads data blocks from the Server 
//   and displays a count of data blocks, every 250 ms.  
//
//   Built using Microsoft Visual C++ 12.0.  Must include Plexon.h and link with PlexClient.lib.
//
//   See SampleClients.rtf for more information.  Plexon technical support is available at 
//   support@plexon.com.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MFCSimpleRead.h"
#include "MFCSimpleReadDlg.h"

// header file containing the Plexon APIs (link with PlexClient.lib, run with PlexClient.dll)
#include "../../include/plexon.h"

// maximum number of data blocks to be read at one time from the Server
#define MAX_DATA_BLOCKS_PER_READ 500000

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// these are global variables, for simplicity
BOOL          bConnectedToServer = FALSE;   // TRUE once we've successfully connected to the Server
BOOL          bFetchingEvents = FALSE;      // TRUE when we're getting data blocks from the Server (i.e. not paused)
PL_Event*     pServerEventBuffer = NULL;    // pointer to allocated memory in which the Server returns data blocks   
UINT          nTimerID = -1;                // ID of the one second timer

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMFCSimpleReadDlg dialog

CMFCSimpleReadDlg::CMFCSimpleReadDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMFCSimpleReadDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMFCSimpleReadDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCSimpleReadDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMFCSimpleReadDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMFCSimpleReadDlg, CDialog)
	//{{AFX_MSG_MAP(CMFCSimpleReadDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDCONNECT, OnConnect)
	ON_BN_CLICKED(IDPAUSE, OnPause)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMFCSimpleReadDlg message handlers

BOOL CMFCSimpleReadDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMFCSimpleReadDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMFCSimpleReadDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMFCSimpleReadDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

// called when the user clicks on the Connect button to connect to the Server
void CMFCSimpleReadDlg::OnConnect() 
{
	if (!bConnectedToServer)
  {
    // connect to the server and register ourselves as a client
    PL_InitClientEx3(0, NULL, GetSafeHwnd());
    
    // allocate memory in which the server will return data blocks
    pServerEventBuffer = (PL_Event*)malloc(sizeof(PL_Event)*MAX_DATA_BLOCKS_PER_READ);
    if (!pServerEventBuffer)
    {
      MessageBox("Couldn't allocate memory!");
      return;
    }

    bConnectedToServer = bFetchingEvents = TRUE;

    // disable Connect button, since we're connected now
    GetDlgItem(IDCONNECT)->EnableWindow(FALSE);

    // set 250 msec timer
    nTimerID = SetTimer(1, 250, NULL);
  }
}

// called when the user clicks on the Pause button
void CMFCSimpleReadDlg::OnPause() 
{
  if (bConnectedToServer)
  {
    // toggle this boolean to indicate whether we should read data blocks from the server
    bFetchingEvents = !bFetchingEvents;	
    SetDlgItemText(IDPAUSE, bFetchingEvents ? "Pause" : "Resume");
  }
}

void CMFCSimpleReadDlg::OnOK() 
{
	CDialog::OnOK();
}

void CMFCSimpleReadDlg::OnTimer(UINT nIDEvent) 
{
  char sz[16];

	if (bFetchingEvents && (nIDEvent == nTimerID))
  {
    int NumEvents = MAX_DATA_BLOCKS_PER_READ;

    // read the data blocks from the server
    PL_GetTimeStampStructures(&NumEvents, pServerEventBuffer);

    // total number of data blocks read
    sprintf(sz, "%d", NumEvents);
    SetDlgItemText(IDC_SPIKESREAD, sz); 
  }
	
	CDialog::OnTimer(nIDEvent);
}

// called when the user clicks on the Close button
void CMFCSimpleReadDlg::OnDestroy() 
{

	CDialog::OnDestroy();
	
  // kill the one second timer
  if (nTimerID != -1)
    KillTimer(nTimerID);

  // release the memory allocated for the buffer
  free(pServerEventBuffer);
}
