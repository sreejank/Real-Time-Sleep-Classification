///////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   MFCTimeStampReadDlg.cpp 
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
//   and displays a count of spike timestamps on the first two channels, every 250 ms.  
//
//   Built using Microsoft Visual C++ 12.0.  Must include Plexon.h and link with PlexClient.lib.
//
//   See SampleClients.rtf for more information.  Plexon technical support is available at 
//   support@plexon.com.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MFCTimeStampRead.h"
#include "MFCTimeStampReadDlg.h"

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
int           SampleRate;                   // samples/sec for neural channels

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
// CMFCTimeStampReadDlg dialog

CMFCTimeStampReadDlg::CMFCTimeStampReadDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMFCTimeStampReadDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMFCTimeStampReadDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCTimeStampReadDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMFCTimeStampReadDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMFCTimeStampReadDlg, CDialog)
	//{{AFX_MSG_MAP(CMFCTimeStampReadDlg)
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
// CMFCTimeStampReadDlg message handlers

BOOL CMFCTimeStampReadDlg::OnInitDialog()
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

void CMFCTimeStampReadDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CMFCTimeStampReadDlg::OnPaint() 
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
HCURSOR CMFCTimeStampReadDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CMFCTimeStampReadDlg::OnConnect() 
{
	if (!bConnectedToServer)
  {
    // connect to the server and register ourselves as a client
    PL_InitClientEx3(0, NULL, GetSafeHwnd());
    
    // get the sampling rate (spike timestamps)
    switch (PL_GetTimeStampTick()) // returns timestamp resolution in microseconds
    {
      case 25: // 25 usec = 40 kHz, default
        SampleRate = 40000;
        break;
      case 40: // 40 usec = 25 kHz
        SampleRate = 25000;
        break;
      case 50: // 50 usec = 20 kHz
        SampleRate = 20000;
        break;
      default:
        MessageBox("Unsupported sampling rate!");
        return;
    }

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

void CMFCTimeStampReadDlg::OnPause() 
{
  if (bConnectedToServer)
  {
    // toggle this boolean to indicate whether we should read data blocks from the server
    bFetchingEvents = !bFetchingEvents;	
    SetDlgItemText(IDPAUSE, bFetchingEvents ? "Pause" : "Resume");
  }
}

void CMFCTimeStampReadDlg::OnOK() 
{
	// TODO: Add extra validation here
	
	CDialog::OnOK();
}

void CMFCTimeStampReadDlg::OnTimer(UINT nIDEvent) 
{
  char sz[16];

	if (bFetchingEvents && (nIDEvent == nTimerID))
  {
    int NumDataBlocks = MAX_DATA_BLOCKS_PER_READ;

    // read the data blocks from the server
    PL_GetTimeStampStructures(&NumDataBlocks, pServerEventBuffer);

    // total number of events read
    sprintf(sz, "%d", NumDataBlocks);
    SetDlgItemText(IDC_SPIKESREAD, sz); 
    
    // number of timestamps for the first four units on the first two channels
    int NumChannelTimeStamps[8]; 
    memset(NumChannelTimeStamps, 0, sizeof(int)*8);

    // count the number of timestamps on spike channels SPK01a..SPK02d (ignore other channels and units)
    for (int DataBlock = 0; DataBlock < NumDataBlocks; DataBlock++)
    {
      // is it an event we're interested in?
      BOOL bSpikeEvent = 
        (pServerEventBuffer[DataBlock].Type == PL_SingleWFType &&
         pServerEventBuffer[DataBlock].Unit >= 1 && pServerEventBuffer[DataBlock].Unit <= 4);

      // only look at DSP01 and DSP02 channels
      UINT DSPChannel = pServerEventBuffer[DataBlock].Channel;
      if (bSpikeEvent && (DSPChannel == 1 || DSPChannel == 2))
      {
        UINT Unit = pServerEventBuffer[DataBlock].Unit; // 1,2,3,4 = a,b,c,d unit channels

        // increment the count for this unit channel
        UINT UnitChan = (DSPChannel-1)*4 + (Unit-1);
        NumChannelTimeStamps[UnitChan]++;

        // time (in seconds) for this timestamp
        float SpikeTime = (float)pServerEventBuffer[DataBlock].TimeStamp/(float)SampleRate;
        // this generates output in the Visual C++ debug output window only
        TRACE("SPK%d%c   %f\r\n", DSPChannel, 'a'+Unit-1, SpikeTime);
      }
    }  // for

    // update the counts displayed in dialog box
    for (UINT UnitChan = 0; UnitChan < 8; UnitChan++)
    {
      char sz[16];
      sprintf(sz, "%d", NumChannelTimeStamps[UnitChan]);
      SetDlgItemText(IDC_DSP01A+UnitChan, sz);
    }
  }
	
	CDialog::OnTimer(nIDEvent);
}

void CMFCTimeStampReadDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
  // kill the one second timer
  if (nTimerID != -1)
    KillTimer(nTimerID);

  // release the memory allocated for the buffer
  free(pServerEventBuffer);
}
