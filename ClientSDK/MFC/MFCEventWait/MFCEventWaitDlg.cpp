///////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   MFCEventWaitDlg.cpp
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
//   and displays a count of data blocks, once per Server polling cycle. 
//   Uses a Win32 event to synchronize with the Server, ensuring minimum
//   latency operation.
//
//   Built using Microsoft Visual C++ 12.0.  Must include Plexon.h and link with PlexClient.lib.
//
//   See SampleClients.rtf for more information.  Plexon technical support is available at 
//   support@plexon.com.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <afxmt.h>                         // MFC multithreading
#include "MFCEventWait.h"
#include "MFCEventWaitDlg.h"

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
BOOL          bConnectedToServer = FALSE; // TRUE once we've successfully connected to the Server
BOOL          bFetchingEvents = FALSE;    // TRUE when we're getting events from the Server (i.e. not paused)
PL_WaveLong*  pServerEventBuffer;         // buffer in which the Server will return data blocks
int           NumDataBlocks;              // number of data blocks returned from the Server
HANDLE        hServerPollEvent = NULL;    // handle to the event that the Server uses to tell us that events are ready
CEvent        KillDataThreadEvent;        // event which the main thread uses to tell the second thread to kill itself
CMFCEventWaitDlg* pDialog = NULL;         // pointer to the dialog object, for use by the thread which waits on the server
int           ServerDropped;              // nonzero if server dropped any data
int           MMFDropped;                 // nonzero if MMF dropped any data
int           PollHigh;                   // high 32 bits of polling time
int           PollLow;                    // low 32 bits of polling time


UINT ReadThreadFunc(LPVOID pParam); // forward declaration

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
// CMFCEventWaitDlg dialog

CMFCEventWaitDlg::CMFCEventWaitDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMFCEventWaitDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMFCEventWaitDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

  // save a pointer to the dialog object, so that the second thread can send a message to it  
  pDialog = this;
}

void CMFCEventWaitDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMFCEventWaitDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMFCEventWaitDlg, CDialog)
  // manually-added handler for our WM_USER+1 message, sent by the second thread to tell the main 
  // thread that the Server has more events ready for reading.  OnServerReady will be called when
  // the dialog receives a WM_USER+1 message
  ON_MESSAGE(WM_USER+1, OnServerReady)

	//{{AFX_MSG_MAP(CMFCEventWaitDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDCONNECT, OnConnect)
	ON_BN_CLICKED(IDPAUSE, OnPause)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMFCEventWaitDlg message handlers

BOOL CMFCEventWaitDlg::OnInitDialog()
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

void CMFCEventWaitDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CMFCEventWaitDlg::OnPaint() 
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
HCURSOR CMFCEventWaitDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CMFCEventWaitDlg::OnConnect() 
{
	if (!bConnectedToServer)
  {
    // connect to the server and register ourselves as a client
    PL_InitClientEx3(0, NULL, GetSafeHwnd());
    
    // allocate memory in which the server will return data blocks
    pServerEventBuffer = (PL_WaveLong*)malloc(sizeof(PL_WaveLong)*MAX_DATA_BLOCKS_PER_READ);
    if (pServerEventBuffer == NULL)
    {
      MessageBox("Couldn't allocate memory!");
      return;
    }
	
    bConnectedToServer = bFetchingEvents = TRUE;

    // disable Connect button, since we're connected now
    GetDlgItem(IDCONNECT)->EnableWindow(FALSE);

    // start a separate thread which will wait on the Server event
    StartReadThread();
  }
}

void CMFCEventWaitDlg::OnPause() 
{
  if (bConnectedToServer)
  {
    // toggle this boolean to indicate whether we should read events from the server
    bFetchingEvents = !bFetchingEvents;	
    SetDlgItemText(IDPAUSE, bFetchingEvents ? "Pause" : "Resume");
  }
}

void CMFCEventWaitDlg::OnOK() 
{
	// TODO: Add extra validation here
	
	CDialog::OnOK();
}

void CMFCEventWaitDlg::StartReadThread()
{
  // initialize the event used to signal the second thread to kill itself when the app closes
  KillDataThreadEvent.ResetEvent();

  // open the event used by the Server to signal to a client that more data blocks are ready to be read
  hServerPollEvent = OpenEvent(SYNCHRONIZE, FALSE, "PlexonServerEvent");
  if (!hServerPollEvent)
    MessageBox("Couldn't open server poll event!");

  // start the second thread.  ReadThreadFunc is the thread function where the second thread 
  // will begin execution
  AfxBeginThread(ReadThreadFunc, &KillDataThreadEvent);
}

// the main function for the read thread, which waits on the server event
UINT ReadThreadFunc(LPVOID pParam)
{
  // parameter passed in is a pointer to an event used to signal the thread to kill itself before 
  // the app shuts down
  CEvent* pKillThreadEvent = (CEvent *)pParam;

  // an infinite loop which only terminates when the kill event is signaled
	while (TRUE) 
  { 
    // are we being requested to kill ourself?  returns immediately (no wait) if the kill event
    // has not been signaled
    if (::WaitForSingleObject(pKillThreadEvent->m_hObject, 0) == WAIT_OBJECT_0) 
    {
      // yes - kill ourself (the read thread) - execution of this thread ends
      AfxEndThread(0);
      return((UINT)-1);
    }

    // send ourself a message to read events from the server and update the dialog box.
    // note: SendMessage will not return until the message has been processed, unlike 
    // PostMessage, which is asynchronous
    pDialog->SendMessage(WM_USER+1);

    // wait for next server poll to complete.  will wait up to 30 seconds before assuming 
    // there is a problem.  typically it will wait for less than the poll interval set in the 
    // Server options (usually 10 msec to 100 msec). once the server event has signalled, 
    // we unblock and continue at the top of the "while(TRUE)" loop
    if (::WaitForSingleObject(hServerPollEvent, 30000) == WAIT_TIMEOUT)
      return((UINT)-1);
  }
}

// this function handles the WM_USER+1 message that the second thread sends to the main 
// thread when the Server has signalled that it has a batch of data blocks ready for reading
afx_msg LONG CMFCEventWaitDlg::OnServerReady(WPARAM WParam, LPARAM LParam)
{
  // if we're paused, don't read the events
  if (!bFetchingEvents)
    return 0L;

  // this is passed to PL_GetLongWaveFormStructuresEx2 to tell the Server the maximum number of events
  // we want returned at one time.  on return from PL_GetLongWaveFormStructuresEx2, it indicates the 
  // actual number of events in the buffer
  int NumDataBlocks = MAX_DATA_BLOCKS_PER_READ;

  // read the data blocks from the server
  PL_GetLongWaveFormStructuresEx2(&NumDataBlocks, pServerEventBuffer, 
    &ServerDropped, &MMFDropped, &PollHigh, &PollLow);

  // display total number of events read
  char sz[16];
  sprintf(sz, "%d", NumDataBlocks);
  SetDlgItemText(IDC_SPIKESREAD, sz); 
  return 0L;
}

void CMFCEventWaitDlg::OnDestroy() 
{
  // set the kill event, signalling to the second thread that it should terminate
  KillDataThreadEvent.SetEvent();

  // close the event used to wait on the server
  CloseHandle(hServerPollEvent);	

  // release the memory allocated for the events
  free(pServerEventBuffer);
  
  // wait briefly to allow the second thread to kill itself
  Sleep(500);

	CDialog::OnDestroy();
}

