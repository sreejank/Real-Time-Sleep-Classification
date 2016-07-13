///////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   MFCVTReadDlg.cpp 
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
//   Simple dialog-based MFC app that shows how to read and interpret CinePlex coordinate data.
//
//   Built using Microsoft Visual C++ 12.0.  Must include Plexon.h and link with PlexClient.lib.
//
//   See SampleClients.rtf for more information.  Plexon technical support is available at 
//   support@plexon.com.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MFCVTRead.h"
#include "MFCVTReadDlg.h"

#include "vt_interpret.h"

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
BOOL          bFetchingTimeStamps = FALSE;  // TRUE when we're getting timestamps from the Server (i.e. not paused)
UINT          nTimerID = -1;                // ID of the one second timer
PL_Event*     pServerEventBuffer;           // buffer in which the Server will return data blocks
int           NumDataBlocks;                // number of data blocks returned from the Server
int           ServerDropped;                // nonzero if server dropped any data
int           MMFDropped;                   // nonzero if MMF dropped any data
int           PollHigh;                     // high 32 bits of polling time
int           PollLow;                      // low 32 bits of polling time
char          szTemp[255];          
int           Dummy[64];
VT_Data       data;                         // decoded VT data
VT_Acc        acc;                          // accumulator of VT data
unsigned __int64 acceptable_delay;          // maximal timestamp delta between consequtive items of VT package


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
// CMFCVTReadDlg dialog

CMFCVTReadDlg::CMFCVTReadDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMFCVTReadDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMFCVTReadDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCVTReadDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMFCVTReadDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMFCVTReadDlg, CDialog)
	//{{AFX_MSG_MAP(CMFCVTReadDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDCONNECT, OnConnect)
	ON_BN_CLICKED(IDPAUSE, OnPause)
	ON_BN_CLICKED(IDOK, OnOk)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMFCVTReadDlg message handlers

BOOL CMFCVTReadDlg::OnInitDialog()
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

void CMFCVTReadDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CMFCVTReadDlg::OnPaint() 
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
HCURSOR CMFCVTReadDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CMFCVTReadDlg::OnConnect() 
{
	if (!bConnectedToServer)
  {
    // connect to the server and register ourselves as a client
    PL_InitClientEx3(0, NULL, GetSafeHwnd());
    
    // initialize the VT accumulator
    VT_Acc_Init(&acc);

    // calculate VT delta using the current sampling rate (timestamps)
    acceptable_delay = (unsigned __int64)(1e6 / PL_GetTimeStampTick() / 105.0 + 0.5);

    // allocate memory in which the server will return data blocks
    pServerEventBuffer = (PL_Event*)malloc(sizeof(PL_Event)*MAX_DATA_BLOCKS_PER_READ);
    if (pServerEventBuffer == NULL)
    {
      MessageBox("Couldn't allocate memory!\r\n");
      return;
    }

    bConnectedToServer = bFetchingTimeStamps = TRUE;

    // disable Connect button, since we're connected now
    GetDlgItem(IDCONNECT)->EnableWindow(FALSE);

    // set 1000 msec timer
    nTimerID = SetTimer(1, 1000, NULL);
  }
}

void CMFCVTReadDlg::OnPause() 
{
  if (bConnectedToServer)
  {
    // toggle this boolean to indicate whether we should read timestamps from the server
    bFetchingTimeStamps = !bFetchingTimeStamps;	
    SetDlgItemText(IDPAUSE, bFetchingTimeStamps ? "Pause" : "Resume");
  }
}

void CMFCVTReadDlg::OnOk() 
{
	CDialog::OnOK();
}

void CMFCVTReadDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
  // kill the one second timer
  if (nTimerID != -1)
    KillTimer(nTimerID);

  // release the memory allocated for the timestamps
  free(pServerEventBuffer);
}

void CMFCVTReadDlg::OnTimer(UINT nIDEvent) 
{
    CString buf;
    VT_Acc  temp;
    VT_Acc_Init(&temp);

    if (bFetchingTimeStamps && (nIDEvent == nTimerID))
    {
      int NumDataBlocks = MAX_DATA_BLOCKS_PER_READ;

      // call the Server to get all the data blocks since the last time we called PL_GetTimeStampStructures
      PL_GetTimeStampStructures(&NumDataBlocks, pServerEventBuffer);

      // total number of data blocks read
      buf.Format("%d", NumDataBlocks);
      SetDlgItemText(IDC_EVENTSREAD, buf);

      // step through the array of data blocks, processing only the strobed event timestamps
      for (int i = 0; i < NumDataBlocks; i++)
      {
        // is this the timestamp of a strobed event?
        if (pServerEventBuffer[i].Type == PL_ExtEventType &&          // event timestamp
            pServerEventBuffer[i].Channel == PL_StrobedExtChannel)    // strobed event only
        {
          // extract VT data and add it to the accumulator
          VT_Data_Init(&data, &pServerEventBuffer[i]);
          bool accepted = VT_Acc_Accept(&acc, &data, acceptable_delay);
          if (!accepted) 
          {
            // data was rejected
            if (VT_Acc_Mode(&acc) != UNKNOWN) 
            {
              // save valid VT data
              temp = acc;
            }
            // reset the accumulator
            VT_Acc_Clear(&acc);
            // add VT data to it
            VT_Acc_Accept(&acc, &data, acceptable_delay);
          }
        }
      }
      // show last valid VT package
      SetDlgItemText(IDC_STATIC_TEXT, VT_Acc_Print(&temp));
    }

    CDialog::OnTimer(nIDEvent);
}
