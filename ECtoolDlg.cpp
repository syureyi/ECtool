// ECtoolDlg.cpp : implementation file
//
#include "stdafx.h"
#include "ECtool.h"
#include "ECtoolDlg.h"
#include "api.h"
#include "./common/Util.h"
#include "./render/d3d9_utils.h"
#include "ECplayControl.h"
#include "./common/typedef.h"
#include "./PaketLoss/SimulatePacketLoss.h"
#include "ECinfoDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define LEN_OF_BUFFER 2048
TCHAR gClassNameInterWindow[] = _T("INTERWINDOW");
HWND					m_previewerWindowHandle =0;
HWND                    m_dec1viewerWindowHandle=0;
HWND                    m_dec2viewerWindowHandle=0;
//IWseVideoRenderer * g_pOrigVideoRenderer = 0;
//IWseVideoRenderer * g_pDec1VideoRenderer = 0;
//IWseVideoRenderer * g_pDec2VideoRenderer = 0;


//global buffer
DATABLOCK g_DataBlock;

// CAboutDlg dialog used for App About
LRESULT CALLBACK backInterWindow(HWND hWnd,UINT message,WPARAM wParam,LPARAM lparam)
{
	HWND *pWnd = new HWND;
	PAINTSTRUCT ps;
	*pWnd = hWnd;
	HDC hdc;
	switch(message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd,&ps);
		theApp.m_pMainWnd->SendMessage(WM_MESSAGE_PAINT,(UINT_PTR)(pWnd),0);
		EndPaint(hWnd,&ps);
        return DefWindowProc(hWnd,message,wParam,lparam);
	case WM_CLOSE:
		theApp.m_pMainWnd->SendMessage(WM_MESSAGE_CLOSE_VIDEO_WINDOW,(UINT_PTR)(pWnd),0);
		return DefWindowProc(hWnd,message,wParam,lparam);
	case WM_MOVE:
		return DefWindowProc(hWnd,message,wParam,lparam);
	case WM_SIZE:
		theApp.m_pMainWnd->SendMessage(WM_MESSAGE_VIDEO_WINDOW_POSITION_CHANGE,(UINT_PTR)(pWnd),0);
		return DefWindowProc(hWnd,message,wParam,lparam);
	case WM_DISPLAYCHANGE:
		theApp.m_pMainWnd->SendMessage(WM_MESSAGE_VIDEO_WINDOW_DISPLAY_CHANGE,0,0);
		return DefWindowProc(hWnd,message,wParam,lparam);
	default:
		return DefWindowProc(hWnd,message,wParam,lparam);
		
	}
	return 0;
}

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CECtoolDlg dialog




CECtoolDlg::CECtoolDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CECtoolDlg::IDD, pParent)
	,m_strLoadFileName(_T("E:\\ECtool\\ECtool\\test.264"))
	,m_strDec1FileName(_T(""))
	,m_strDec2FileName(_T(""))
	,m_nFrameWidth(1024)
	,m_nFrameHeight(768)
	//,m_dec1viewerWindowHandle(0)
	//,m_previewerWindowHandle(0)
	//,m_dec2viewerWindowHandle(0)
	,m_bDec1Checked(FALSE)
	,m_bDec2Checked(FALSE)
	,m_bOrigChecked(TRUE)
	,m_nRadioLossType(5)
	,m_bEnableEC(TRUE)
	,m_bInitOrig(FALSE)
	,m_strLossSimulatorCfg("E:\\ECtool\\ECtool\\3.dat")
	
	, m_ifps(30)
	, m_nRadioFileType(1)
{
	m_pDecoder = NULL;
	m_bInitDec1 = FALSE;
	m_bInitDec2 = FALSE;
	m_bViewStepInfo = FALSE;
	m_bViewAllInfo = FALSE;
	m_pPlayControl = new ECplayControl();
	m_aryDecoder.Add(_T("E:\\openh264-master\\bin\\Win32\\Debug\\welsdec.dll"));
	
	
	
	
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	/*m_pInfoDlg = new CECinfoDlg(this);*/

}

void CECtoolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX,IDC_EDIT_LOAD_FILE,m_strLoadFileName);
	DDX_Text(pDX, IDC_EDIT_WIDTH, m_nFrameWidth);
	DDX_Text(pDX, IDC_EDIT_HEIGHT, m_nFrameHeight);
	DDX_Check(pDX,IDC_CHECK_ORIG,m_bOrigChecked);
	DDX_Check(pDX,IDC_CHECK_DEC1,m_bDec1Checked);
	DDX_Check(pDX,IDC_CHECK_DEC2,m_bDec2Checked);

	DDX_Radio(pDX, IDC_RADIO_3, m_nRadioLossType);
	DDX_Text(pDX, IDC_EDIT_FPS, m_ifps);
	
	DDX_Radio(pDX, IDC_RADIO_YUV, m_nRadioFileType);
}

BEGIN_MESSAGE_MAP(CECtoolDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_HSCROLL()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(ID_BUTTON_LOAD_FILE, &CECtoolDlg::OnBnClickedButtonLoadFile)
	ON_MESSAGE(WM_MESSAGE_CLOSE_VIDEO_WINDOW,&CECtoolDlg::OnRenderWindowClosed)
	ON_MESSAGE(WM_MESSAGE_VIDEO_WINDOW_DISPLAY_CHANGE,&CECtoolDlg::OnRenderDisplayResolutionChanged)
	ON_MESSAGE(WM_MESSAGE_PAINT,&CECtoolDlg::OnRenderDisplay)
	ON_MESSAGE(WM_MESSAGE_VIDEO_WINDOW_POSITION_CHANGE,&CECtoolDlg::OnRenderWindowPositionChanged)
	ON_MESSAGE(WM_MESSAGE_CHANGE_EDIT,&CECtoolDlg::OnScoreChanged)
	ON_MESSAGE(WM_MESSAGE_INFO_WINDOW_CLOSE,&CECtoolDlg::OnInfoWindowClosed)
    ON_BN_CLICKED(IDC_BUTTON_PAUSE, &CECtoolDlg::OnBnClickedButtonPause)
	ON_BN_CLICKED(IDC_BUTTON_STEP, &CECtoolDlg::OnBnClickedButtonStepAfter)
	ON_BN_CLICKED(IDC_BUTTON_PLAY, &CECtoolDlg::OnBnClickedButtonPlay)
	ON_BN_CLICKED(IDC_BUTTON_STEP_BEFORE, &CECtoolDlg::OnBnClickedButtonStepBefore)
	ON_BN_CLICKED(IDC_BUTTON_DEC1_DLL, &CECtoolDlg::OnBnClickedButtonDec1Dll)
	ON_BN_CLICKED(IDC_BUTTON_SHOW_STEP_INFO, &CECtoolDlg::OnBnClickedButtonShowStepInfo)
	ON_BN_CLICKED(IDC_BUTTON_SHOW_ALL_INFO, &CECtoolDlg::OnBnClickedButtonShowAllInfo)	
	ON_BN_CLICKED(IDC_RADIO_3, &CECtoolDlg::OnBnClickedRadio3)
	
END_MESSAGE_MAP()


// CECtoolDlg message handlers

BOOL CECtoolDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);
	/*GetDlgItem(IDC_BUTTON_PLAY)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_PLAY)->EnableWindow(FALSE);*/


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
	
	((CWnd*)GetDlgItem(IDC_BUTTON_SHOW_STEP_INFO))->EnableWindow(FALSE);
	((CWnd*)GetDlgItem(IDC_BUTTON_SHOW_ALL_INFO))->EnableWindow(FALSE);
	((CWnd*)GetDlgItem(IDC_BUTTON_PAUSE))->EnableWindow(FALSE);
	((CWnd*)GetDlgItem(IDC_BUTTON_STEP_BEFORE))->EnableWindow(FALSE);



	// TODO: Add extra initialization here
	WNDCLASS wcInter;
	wcInter.style = 0;
	wcInter.lpfnWndProc = &backInterWindow;
	wcInter.cbClsExtra = 0;
	wcInter.cbWndExtra = 0;
	wcInter.hInstance = AfxGetInstanceHandle();
	wcInter.hIcon = LoadIcon(NULL,IDI_APPLICATION);
	wcInter.hCursor = LoadCursor(NULL,IDC_ARROW);
	wcInter.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wcInter.lpszMenuName = NULL;
	wcInter.lpszClassName = gClassNameInterWindow;
	if(!RegisterClass(&wcInter))
	{
		return FALSE;
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}




void CECtoolDlg::OnSysCommand(UINT nID, LPARAM lParam)
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
void CECtoolDlg::OnClose()
{
	CleanUp();
	CDialog::OnClose();
}
// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
void CECtoolDlg::OnDestroy()
{
	CDialog::OnDestroy();

}
void CECtoolDlg::CleanUp()
{

	m_previewerWindowHandle = NULL;
	m_dec1viewerWindowHandle = NULL;
	//g_pDec1VideoRenderer = NULL;
	//g_pOrigVideoRenderer = NULL;
}
void CECtoolDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

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

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CECtoolDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
HWND CECtoolDlg::createWindow(char *pName, DEMO_WINDOW_TYPE eWindowType)
{
	
	HWND windowHandle = NULL;
	CString sName(pName);
	LPCTSTR lpszName = sName;
	int nScreenX = GetSystemMetrics(SM_CXSCREEN);
	int nScreenY = GetSystemMetrics(SM_CYSCREEN);
	int nStartY = 0;
	unsigned int x = 0;
	unsigned int y = 0;
	int nWidth =  m_nFrameWidth;
	int nHeight = m_nFrameHeight;
	int nGap = 0;
	if(nScreenY > (nHeight+20)*2)
	{
		nGap = (nScreenY - (nHeight+20)*2)/3;
	}
	nStartY = nGap;
	if (DEMO_WINDOW_DEC1 == eWindowType)
	{
		x = 600;
		y = nStartY;
	}
	else if (DEMO_WINDOW_DEC2 == eWindowType)
	{
		x = 600;
		y = nStartY + nHeight + 20 + nGap ;
	}
	else if(DEMO_WINDOW_ORIG == eWindowType)
	{
		x = 600;
		y = nStartY;	
	}
	windowHandle = ::CreateWindow(gClassNameInterWindow,
		lpszName,
		WS_VISIBLE|WS_SYSMENU|WS_THICKFRAME|WS_MINIMIZEBOX|WS_MAXIMIZEBOX|WS_CAPTION|WS_OVERLAPPEDWINDOW ,
		x,
		y,
		nWidth,
		nHeight+20,
		m_hWnd,
		NULL,
		AfxGetInstanceHandle(),
		0);
	//windowHandle = ::CreateWindow(_T("RE"), _T("LOCA"),WS_OVERLAPPEDWINDOW|WS_VISIBLE);

	return windowHandle;
}
void CECtoolDlg::OnBnClickedButtonLoadFile()
{
	UpdateData(TRUE);

	TCHAR szCurDir[MAX_PATH];
		
	
	int retval;
	CStringArray  m_aryYUVFile;

	memset(szCurDir,0,sizeof(szCurDir));
	GetModuleFileName(NULL, szCurDir, MAX_PATH);
	for(int i= MAX_PATH-1; i>=0; i--)
	{
		if(szCurDir[i] == '\\')
		{
			szCurDir[i] = '\0';
			break;
		}
	}
	if(m_nRadioFileType==0)
	{
		//Load yuv file from the same file folder, max frame number is 3
		CFileDialog dlg(TRUE, _T(".yuv"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT |OFN_ALLOWMULTISELECT|OFN_ENABLESIZING,NULL,NULL);
		dlg.m_ofn.lpstrInitialDir = szCurDir;
		dlg.m_ofn.nMaxFile = 3* MAX_PATH; 
		dlg.m_ofn.lpstrFile = new TCHAR[dlg.m_ofn.nMaxFile]; 
		ZeroMemory(dlg.m_ofn.lpstrFile, sizeof(TCHAR) * dlg.m_ofn.nMaxFile); 
	
		retval = dlg.DoModal();
		if(retval==IDCANCEL)
			return ;
		POSITION pos_file;
		pos_file = dlg.GetStartPosition();
		while(pos_file != NULL)
			m_aryYUVFile.Add(dlg.GetNextPathName(pos_file));
		int count = m_aryYUVFile.GetCount();
		switch(count)
		{
		case 3:
			m_strDec2FileName = m_aryYUVFile.GetAt(2);
		case 2:
			m_strDec1FileName = m_aryYUVFile.GetAt(1);
		case 1:
			m_strLoadFileName = m_aryYUVFile.GetAt(0);
			break;
		default:
			MessageBox(_T("Just support at most 3 YUVs rendering here"));
			break;
			
		}
		

	}
	else
	{
	// TODO: Add file into 
	CFileDialog dlg(TRUE, _T(".264"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, this);
  
//USES_CONVERSION;
////szCurDir = A2T("E:\openh264-master\res");
//_tcscpy(szCurDir, A2T("E:\openh264-master\res"));
	dlg.m_ofn.lpstrInitialDir =szCurDir;
	if(dlg.DoModal() == IDOK)
	{
		m_strLoadFileName = dlg.GetPathName();	
	}
	}
	UpdateData(FALSE);
	
}



	  
	
//****************************************************************
	

	//Just Use D3D9 to test input yuv data
	//unsigned char* pData[3] = {NULL};    
   // pData[0]	= pDataBlock;	
	//memcpy(pData[0],pDataBlock,sizeof(unsigned char)*iLength);
	//pData[1] = pDataBlock+iLength;
 //   memcpy(pData[1],pDataBlock+iLength,sizeof(unsigned char)*iLength/4);
	//pData[2] = pDataBlock+iLength*5/4;
	//memcpy(pData[2],pDataBlock+(iLength*5/4),sizeof(unsigned char)*iLength/4);
 //   CUtils cOutputModule;
	//SBufferInfo sDstBufInfo;
	//memset (&sDstBufInfo, 0, sizeof (SBufferInfo));
	//FILE* pYuvFile	  = NULL;
	////pYuvFile = fopen ("test.yuv", "wb");
	//sDstBufInfo.UsrData.sSystemBuffer.iFormat = WseI420;
	//sDstBufInfo.UsrData.sSystemBuffer.iWidth = 1024;
	//sDstBufInfo.UsrData.sSystemBuffer.iHeight = 768;

	//sDstBufInfo.UsrData.sSystemBuffer.iStride[0] = 1024;
	//sDstBufInfo.UsrData.sSystemBuffer.iStride[1] = 512;
	//cOutputModule.Process ((void**)pData, &sDstBufInfo, pYuvFile);	
	//fclose(pYuvFile);
  
//*********************************************************************	

	
LRESULT CECtoolDlg::OnRenderWindowClosed(WPARAM wParam,LPARAM lparam)
{
	HWND *pWnd = (HWND*)wParam;
	DEMO_WINDOW_TYPE eWindowType;
	if(!(pWnd))
		return S_OK;
	if (m_previewerWindowHandle == *pWnd)

		eWindowType = DEMO_WINDOW_ORIG;
	else if(m_dec1viewerWindowHandle == *pWnd)
		eWindowType = DEMO_WINDOW_DEC1;
	else if(m_dec2viewerWindowHandle == *pWnd)
		eWindowType = DEMO_WINDOW_DEC2;
	
	
	/*switch(*pWnd)
	{
	case m_previewerWindowHandle:
		eWindowType = DEMO_WINDOW_ORIG;
		break;
	case m_dec1viewerWindowHandle:
		eWindowType = DEMO_WINDOW_DEC1;
		break;
	case m_dec2viewerWindowHandle:
		eWindowType = DEMO_WINDOW_DEC2;
		break;
	default:
		break;
	}*/
	m_pPlayControl->ECstop(eWindowType);

	if(m_previewerWindowHandle == *pWnd)
	{
		m_previewerWindowHandle = 0;
		m_bInitOrig = false;
		((CButton*)GetDlgItem(IDC_CHECK_ORIG))->SetCheck(0);
		m_pPlayControl->ECunInit(DEMO_WINDOW_ORIG);
	}
	else if(m_dec1viewerWindowHandle == *pWnd)
	{
		m_dec1viewerWindowHandle = 0;
		m_bInitDec1 = false;
		((CButton*)GetDlgItem(IDC_CHECK_DEC1))->SetCheck(0);
		m_pPlayControl->ECunInit(DEMO_WINDOW_DEC1);

	}
	else if(m_dec2viewerWindowHandle == *pWnd)
	{
		m_dec2viewerWindowHandle = 0;
		m_bInitDec2 = false;
		((CButton*)GetDlgItem(IDC_CHECK_DEC2))->SetCheck(0);
		m_pPlayControl->ECunInit(DEMO_WINDOW_DEC2);
	}
	
	return S_OK;
}
LRESULT CECtoolDlg::OnRenderDisplayResolutionChanged(WPARAM wParam,LPARAM lparam)
{
	HWND *pWnd = (HWND*)wParam;
	DEMO_WINDOW_TYPE eWindowType;
	if(!(pWnd))
		return S_OK;
	if (m_previewerWindowHandle == *pWnd)
		eWindowType = DEMO_WINDOW_ORIG;
	else if(m_dec1viewerWindowHandle == *pWnd)
		eWindowType = DEMO_WINDOW_DEC1;
	else if(m_dec2viewerWindowHandle == *pWnd)
		eWindowType = DEMO_WINDOW_DEC2;
	m_pPlayControl->ECdisplayChange(eWindowType);

	return S_OK;
}
LRESULT CECtoolDlg::OnRenderWindowPositionChanged(WPARAM wParam,LPARAM lparam)
{
	HWND *pWnd = (HWND*)wParam;
	DEMO_WINDOW_TYPE eWindowType;
	if(!(pWnd))
		return S_OK;
	if (m_previewerWindowHandle == *pWnd)
		eWindowType = DEMO_WINDOW_ORIG;
	else if(m_dec1viewerWindowHandle == *pWnd)
		eWindowType = DEMO_WINDOW_DEC1;
	else if(m_dec2viewerWindowHandle == *pWnd)
		eWindowType = DEMO_WINDOW_DEC2;
	else
		return S_OK;
	
	m_pPlayControl->ECrenderPosChange(eWindowType);

	return S_OK;

}

LRESULT CECtoolDlg::OnRenderDisplay(WPARAM wParam,LPARAM lparam)
{
	HWND *pWnd = (HWND*)wParam;
	DEMO_WINDOW_TYPE eWindowType;
	if(!(pWnd))
		return S_OK;
	if (m_previewerWindowHandle == *pWnd)
		eWindowType = DEMO_WINDOW_ORIG;
	else if(m_dec1viewerWindowHandle == *pWnd)
		eWindowType = DEMO_WINDOW_DEC1;
	else if(m_dec2viewerWindowHandle == *pWnd)
		eWindowType = DEMO_WINDOW_DEC2;
	//m_pInfoDlg->m_iFrameNoOrig = m_pPlayControl->ECgetFrameInfo(eWindowType).iFrameNo;
	//m_pInfoDlg->UpdateData(FALSE);
	m_pPlayControl->ECreDraw(eWindowType);

	return S_OK;
}
LRESULT CECtoolDlg::OnScoreChanged(WPARAM wParam,LPARAM lparam)
{

	/*if (m_previewerWindowHandle == *pWnd)
		eWindowType = DEMO_WINDOW_ORIG;
	else if(m_dec1viewerWindowHandle == *pWnd)
		eWindowType = DEMO_WINDOW_DEC1;
	else if(m_dec2viewerWindowHandle == *pWnd)
		eWindowType = DEMO_WINDOW_DEC2;*/
	if(m_bViewStepInfo)
	{
		m_pPlayControl->ECsetFrameInfo(m_pInfoDlg->m_iScoreDec1,m_pInfoDlg->m_iScoreDec2);
		m_pInfoDlg->m_iAvgScoreDec1 = (m_pPlayControl->ECgetInfo(DEMO_WINDOW_DEC1)).iScore;
		m_pInfoDlg->m_iAvgScoreDec2 = (m_pPlayControl->ECgetInfo(DEMO_WINDOW_DEC2)).iScore;
	}
	if(m_bViewAllInfo)
	{
		m_pPlayControl->ECsetInfo(m_pInfoDlg->m_iScoreDec1,m_pInfoDlg->m_iScoreDec2);
		m_pInfoDlg->m_iAvgScoreDec1 = (m_pPlayControl->ECgetInfo(DEMO_WINDOW_DEC1)).iScore;
		m_pInfoDlg->m_iAvgScoreDec2 = (m_pPlayControl->ECgetInfo(DEMO_WINDOW_DEC2)).iScore;
		
	}
	
	return S_OK;
}

void CECtoolDlg::OnBnClickedButtonPause()
{
	// TODO: Add your control notification handler code here
	
	m_pPlayControl->ECpause();


}
void CECtoolDlg::ReadFileToSample(DEMO_WINDOW_TYPE eWindowType)
{  
	unsigned long iLength = m_nFrameWidth*m_nFrameHeight;
	unsigned long iLen =  m_nFrameWidth*m_nFrameHeight*1.5;
	unsigned long iFileSize,iUsed;
	SPlayFrameInfo sFrameInfo;
	CString strFileName;

	switch(eWindowType)
	{
		case DEMO_WINDOW_ORIG:
			strFileName = m_strLoadFileName;
			m_bInitOrig = TRUE; 
			break;
		case DEMO_WINDOW_DEC1:
			strFileName = m_strDec1FileName;
			m_bInitDec1 = TRUE;
			break;
		case DEMO_WINDOW_DEC2:
			strFileName = m_strDec2FileName;
			m_bInitDec2 = TRUE;
			break;
	}  
	CFile Infile(strFileName,CFile::modeRead);
	if(Infile.GetLength()<2)
		return;
	iFileSize = Infile.GetLength();
	iUsed = 0;
	int frameCount=0;
	
	while(iUsed<iFileSize)
	  {
		//VideoSample
		IWseVideoSample* pSample = NULL;
		m_pAlloc = NULL;
		CreateVideoSampleAllocator(64,&m_pAlloc);
		m_pAlloc->GetSample(iLen,&pSample);		
		pSample->GetDataLength(&iLen);
		m_format.height = m_nFrameHeight;
		m_format.video_type = WseI420;
		m_format.width = m_nFrameWidth;
		pSample->SetVideoFormat(&m_format);
		unsigned char* pDataBlock = new unsigned char[iLen];
		memset(pDataBlock,0,iLen+1);
		pSample->GetDataPointer(&pDataBlock);
		Infile.Read(pDataBlock,iLen);
		Infile.Seek(0,CFile::current);
		sFrameInfo.pSample = pSample;
		sFrameInfo.iFrameNo = frameCount;
		
		//if(eWindowType == DEMO_WINDOW_ORIG)
            sFrameInfo.fPsnr = 20;
		//else
			//sFrameInfo.fPsnr = m_pPlayControl->ECsetPsnrInfo(pDataBlock);
		m_pPlayControl->ECsetPlayComment(eWindowType,sFrameInfo);	
		iUsed+=iLen;
		frameCount++;
		pDataBlock =NULL;
		delete[] pDataBlock;
		pSample = NULL;
    }
	  Infile.Close();
	
	
}
int CECtoolDlg::ReadBSToSample(DEMO_WINDOW_TYPE eWindowType)
{
    SSliceLossRatioInPercent sSliceLossRatio={0,0,0,0,0,0,0,0};
	int iErrorConMethod;
	int iFrameCount;
	//SFrameInfo sFrameInfo;
	//char* windowTitle;
	// packete loss simulator init related
		
	switch(m_nRadioLossType){
		case 0:
		case 1:
		case 2:
			m_Sample.SetLossFileName(m_strLossSimulatorCfg);
			m_bEnableEC =TRUE;
			break;
		case 3:
			m_Sample.CleanLossFileName();
			sSliceLossRatio.iAVCPSliceLossRatio = 1;
			m_bEnableEC =TRUE;
			break;
		case 4:
			m_Sample.CleanLossFileName();
			sSliceLossRatio.iAVCISliceLossRatio = 1;
			m_bEnableEC =TRUE;
			break;
		case 5:
			m_Sample.CleanLossFileName();
			m_bEnableEC=FALSE;
			break;
		default:
			break;

		}
    switch(eWindowType)
	{
		case DEMO_WINDOW_ORIG:
			m_bEnableEC = false;
			iErrorConMethod = (int)ERROR_CON_DISABLE;
			m_bInitOrig = TRUE; 
			break;
		case DEMO_WINDOW_DEC1:
			m_bEnableEC = true;
			iErrorConMethod = (int)ERROR_CON_SLICE_COPY;
			m_bInitDec1 = TRUE;
			break;
		case DEMO_WINDOW_DEC2:
			m_bEnableEC =false;
			iErrorConMethod = (int)ERROR_CON_FRAME_COPY;
			m_bInitDec2 = TRUE;
			break;
	}
		
	m_Sample.SetFileName(m_strLoadFileName);// the render yuv or 264 file name.
	if(m_strLoadFileName.GetLength()>0 && m_strLoadFileName.Find(_T(".264")))
	{
		m_Sample.InitDecoder(m_aryDecoder.GetAt(0));//Get the decoder dll name
		m_Sample.SetOption (DECODER_OPTION_ERROR_CON_IDC, &iErrorConMethod);
		if(m_bEnableEC)				
		{  
			
			m_Sample.SetEnableEC();
			m_Sample.SetLossRatio(&sSliceLossRatio);
			m_Sample.LossSimulator();
		}
		else
		{
			if(eWindowType == DEMO_WINDOW_ORIG)
				m_Sample.SetUnEnableEC();
		}
		iFrameCount=m_Sample.Decoder(m_pPlayControl,eWindowType);	
		m_Sample.UninitDecoder();
		return iFrameCount;
		
	}
}


void CECtoolDlg::InitSampleAndPlay(DEMO_WINDOW_TYPE eWindowType,HWND* hWnd)
{
	char* windowTitle;
	SFrameInfo sFrameInfo;
	int iFrameCount=0;
	switch(eWindowType)
	{
		case DEMO_WINDOW_ORIG:
			windowTitle = "OrigViewRender";
			break;
		case DEMO_WINDOW_DEC1:
			windowTitle = "Dec1ViewRender";
			break;
		case DEMO_WINDOW_DEC2:
			windowTitle = "Dec2ViewRender";
			break;
	}
	if(m_nRadioFileType ==1)
	{
	
		iFrameCount=ReadBSToSample(eWindowType);
	}
	else
	{
		ReadFileToSample(eWindowType);

	}//TODO, YUV render
	if(iFrameCount >0)
	{
		m_pPlayControl->ECInitPlayPosition(eWindowType);//Init play position
		// Get the first frame info in play list 
		sFrameInfo = m_pPlayControl->ECgetFrameInfo(eWindowType);
	
	if(m_nRadioFileType ==1)
	{
		m_nFrameWidth = sFrameInfo.iFrameWidth;
		m_nFrameHeight = sFrameInfo.iFrameHeight;
		UpdateData(FALSE);
	}
	}
	else
		MessageBox(_T("No frame be decoded"));
	if(*hWnd == 0 )
	 *hWnd = createWindow(windowTitle,eWindowType);
	m_pPlayControl->ECstart(eWindowType,hWnd);

			
}
void CECtoolDlg::InitInfoDlg()
{
	SFrameInfo sFrameInfo;
	
	
	if(((CButton*)GetDlgItem(IDC_CHECK_ORIG))->GetCheck() ==1)
	{
		if(m_bInitOrig)
		{
		sFrameInfo = m_pPlayControl->ECgetFrameInfo(DEMO_WINDOW_ORIG);
		m_pInfoDlg->m_iFrameNoOrig = sFrameInfo.iFrameNo;
		m_pInfoDlg->m_iFrameNum = (m_pPlayControl->ECgetInfo(DEMO_WINDOW_ORIG)).iFrameNo;
		}
		m_pInfoDlg->m_iScore = 0;
		m_pInfoDlg->m_iAvgScore = 0;
		m_pInfoDlg->m_fPsnrOrig = 0;
		
	}
	if(((CButton*)GetDlgItem(IDC_CHECK_DEC1))->GetCheck() ==1)
	{
		if(m_bInitDec1)
		{
		sFrameInfo = m_pPlayControl->ECgetFrameInfo(DEMO_WINDOW_DEC1);
	  m_pInfoDlg->m_iScoreDec1 = sFrameInfo.iScore;
	  m_pInfoDlg->m_iAvgScoreDec1 = (m_pPlayControl->ECgetInfo(DEMO_WINDOW_DEC1)).iScore;
	  m_pInfoDlg->m_fPSNRDec1 = sFrameInfo.fPsnr;
	  m_pInfoDlg->m_iFrameNoDec1 = sFrameInfo.iFrameNo;
	  m_pInfoDlg->m_iFrameNumDec1 = (m_pPlayControl->ECgetInfo(DEMO_WINDOW_DEC1)).iFrameNo;
		}

	}
	if(((CButton*)GetDlgItem(IDC_CHECK_DEC2))->GetCheck() ==1)
	{
		if(m_bInitDec2)
		{
		sFrameInfo = m_pPlayControl->ECgetFrameInfo(DEMO_WINDOW_DEC2);
	  m_pInfoDlg->m_iScoreDec2 = sFrameInfo.iScore;
	  m_pInfoDlg->m_iAvgScoreDec2 = (m_pPlayControl->ECgetInfo(DEMO_WINDOW_DEC2)).iScore;
	  m_pInfoDlg->m_fPSNRDec2 = sFrameInfo.fPsnr;
	  m_pInfoDlg->m_iFrameNoDec2 = sFrameInfo.iFrameNo;
	  m_pInfoDlg->m_iFrameNumDec2 = (m_pPlayControl->ECgetInfo(DEMO_WINDOW_DEC2)).iFrameNo;
		}

	}

}



void CECtoolDlg::ShowInfoDlg()
{
	SFrameInfo sFrameInfo;
	
	
	if(((CButton*)GetDlgItem(IDC_CHECK_ORIG))->GetCheck() ==1)
	{
		if(m_bInitOrig)
		{
		sFrameInfo = m_pPlayControl->ECgetInfo(DEMO_WINDOW_ORIG);
		m_pInfoDlg->m_iFrameNoOrig = sFrameInfo.iFrameNo;
		m_pInfoDlg->m_iFrameNum = sFrameInfo.iFrameNo;
		m_pInfoDlg->m_iScore = sFrameInfo.iScore;
		m_pInfoDlg->m_iAvgScore = sFrameInfo.iScore;
		m_pInfoDlg->m_fPsnrOrig = sFrameInfo.fPsnr;
		}
		
	}
	if(((CButton*)GetDlgItem(IDC_CHECK_DEC1))->GetCheck() ==1)
	{
		if(m_bInitDec1)
		{
		sFrameInfo = m_pPlayControl->ECgetInfo(DEMO_WINDOW_DEC1);
	    m_pInfoDlg->m_iScoreDec1 = sFrameInfo.iScore;
	    m_pInfoDlg->m_iAvgScoreDec1 = sFrameInfo.iScore;
	    m_pInfoDlg->m_fPSNRDec1 = sFrameInfo.fPsnr;
	    m_pInfoDlg->m_iFrameNoDec1 = sFrameInfo.iFrameNo;
	    m_pInfoDlg->m_iFrameNumDec1 = sFrameInfo.iFrameNo;
		}

	}
	if(((CButton*)GetDlgItem(IDC_CHECK_DEC2))->GetCheck() ==1)
	{
		if(m_bInitDec2)
		{
		sFrameInfo = m_pPlayControl->ECgetInfo(DEMO_WINDOW_DEC2);
	    m_pInfoDlg->m_iScoreDec2 = sFrameInfo.iScore;
	    m_pInfoDlg->m_iAvgScoreDec2 = sFrameInfo.iScore;
	    m_pInfoDlg->m_fPSNRDec2 = sFrameInfo.fPsnr;
	    m_pInfoDlg->m_iFrameNoDec2 = sFrameInfo.iFrameNo;
	    m_pInfoDlg->m_iFrameNumDec2 = sFrameInfo.iFrameNo;
		}

	}

}



void CECtoolDlg::UninitSampleAndPlay(DEMO_WINDOW_TYPE eWindowType,HWND hWnd)
{
	hWnd = 0;
	m_pPlayControl->ECstop(eWindowType);
	m_bInitOrig = FALSE;
}


void CECtoolDlg::OnBnClickedButtonPlay()
{
	// Play, init play comments,render, and also window
	UpdateData(TRUE);
	unsigned int iPlayFrameRate=30;
	if(!m_bViewAllInfo && !m_bViewStepInfo)
	{
	((CWnd*)GetDlgItem(IDC_BUTTON_SHOW_STEP_INFO))->EnableWindow(TRUE);
	((CWnd*)GetDlgItem(IDC_BUTTON_SHOW_ALL_INFO))->EnableWindow(TRUE);
	}	
	((CWnd*)GetDlgItem(IDC_BUTTON_PAUSE))->EnableWindow(TRUE);
	
	unsigned int iRender = 0;
	
	//judge which to be render
	if(((CButton*)GetDlgItem(IDC_CHECK_ORIG))->GetCheck() ==1)
	{
		if(!m_bInitOrig)
			InitSampleAndPlay(DEMO_WINDOW_ORIG,&m_previewerWindowHandle);	
		iRender++;

	}
	if(((CButton*)GetDlgItem(IDC_CHECK_DEC1))->GetCheck() ==1)
	{
		if(!m_bInitDec1)
			InitSampleAndPlay(DEMO_WINDOW_DEC1,&m_dec1viewerWindowHandle);
		iRender++;
	}
	if(((CButton*)GetDlgItem(IDC_CHECK_DEC2))->GetCheck() ==1)
	{
		if(!m_bInitDec2)
			InitSampleAndPlay(DEMO_WINDOW_DEC2,&m_dec2viewerWindowHandle);
	
		iRender++;
	}
	
	if(iRender == 0)
	{
		MessageBox(_T("please check box,to select witch window to render"));
	    return;
	}
	else
	{		
		if(m_bViewAllInfo)
		{
			ShowInfoDlg();
		    m_pInfoDlg->UpdateData(FALSE);
		}
		m_pPlayControl->ECsetPlayFrameRate(m_ifps);
		m_pPlayControl->ECplayThread(iRender);
	}

	
	return;
}

void CECtoolDlg::OnBnClickedButtonStepBefore()
{
	// TODO: Add your control notification handler code here
	// Play, init play comments,render, and also window
	UpdateData(TRUE);
	unsigned int iRound = 0;

	
	//judge which to be render
	if(((CButton*)GetDlgItem(IDC_CHECK_ORIG))->GetCheck() ==1)
	{
		if(!m_bInitOrig)
			InitSampleAndPlay(DEMO_WINDOW_ORIG,&m_previewerWindowHandle);
		
		iRound++;

	}
	if(((CButton*)GetDlgItem(IDC_CHECK_DEC1))->GetCheck() ==1)
	{
		if(!m_bInitDec1)
			InitSampleAndPlay(DEMO_WINDOW_DEC1,&m_dec1viewerWindowHandle);
		
		iRound++;
	}
	if(((CButton*)GetDlgItem(IDC_CHECK_DEC2))->GetCheck() ==1)
	{
		if(!m_bInitDec2)
			InitSampleAndPlay(DEMO_WINDOW_DEC2,&m_dec2viewerWindowHandle);
		
		iRound++;
	}
	
	if(iRound == 0)
	{
		MessageBox(_T("please check box,to select witch window to render"));
	    return;
	}
	else
	{
		m_pPlayControl->ECstepBefore();
		if(m_bViewStepInfo)
		{
		 InitInfoDlg();
		 m_pInfoDlg->UpdateData(FALSE);
		}
	}

	
	return;
}

void CECtoolDlg::OnBnClickedButtonStepAfter()
{
	// Play, init play comments,render, and also window
	UpdateData(TRUE);
	unsigned int iRound = 0;
		((CWnd*)GetDlgItem(IDC_BUTTON_SHOW_STEP_INFO))->EnableWindow(TRUE);
	((CWnd*)GetDlgItem(IDC_BUTTON_SHOW_ALL_INFO))->EnableWindow(TRUE);
	((CWnd*)GetDlgItem(IDC_BUTTON_STEP_BEFORE))->EnableWindow(TRUE);
	
	
	//judge which to be render
	if(((CButton*)GetDlgItem(IDC_CHECK_ORIG))->GetCheck() ==1)
	{
		if(!m_bInitOrig)
			InitSampleAndPlay(DEMO_WINDOW_ORIG,&m_previewerWindowHandle);
		/*if(m_bViewInfo)
		InitInfoDlg(DEMO_WINDOW_ORIG);*/
	
		iRound++;

	}
	if(((CButton*)GetDlgItem(IDC_CHECK_DEC1))->GetCheck() ==1)
	{
		if(!m_bInitDec1)
			InitSampleAndPlay(DEMO_WINDOW_DEC1,&m_dec1viewerWindowHandle);
		/*if(m_bViewInfo)
		InitInfoDlg(DEMO_WINDOW_DEC1);*/
		iRound++;
	}
	if(((CButton*)GetDlgItem(IDC_CHECK_DEC2))->GetCheck() ==1)
	{
		if(!m_bInitDec2)
			InitSampleAndPlay(DEMO_WINDOW_DEC2,&m_dec2viewerWindowHandle);
		/*if(m_bViewInfo)
		InitInfoDlg(DEMO_WINDOW_DEC2);*/
		iRound++;
	}
	
	if(iRound == 0)
	{
		MessageBox(_T("please check box,to select which window to render"));
	    return;
	}
	else
	{
		//m_pInfoDlg->m_iFrameNoOrig = (m_pPlayControl->ECgetFrameInfo()).iFrameNo;
		//m_pInfoDlg->m_iScore = (m_pPlayControl->ECgetFrameInfo()).iScore;
		//UpdateData(FALSE);		
		m_pPlayControl->ECstepAfter();
		if(m_bViewStepInfo)
		{
		 InitInfoDlg();
		 m_pInfoDlg->UpdateData(FALSE);
		}
       

	}

	
	return;
	
}

void CECtoolDlg::OnBnClickedButtonDec1Dll()
{
	// TODO: Add your control notification handler code here
	CFileDialog dlg(TRUE, _T("dll"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT |OFN_ALLOWMULTISELECT|OFN_ENABLESIZING,NULL,NULL);
	TCHAR szCurDir[MAX_PATH];
	memset(szCurDir,0,sizeof(szCurDir));
	GetModuleFileName(NULL, szCurDir, MAX_PATH);
	for(int i= MAX_PATH-1; i>=0; i--)
	{
		if(szCurDir[i] == '\\')
		{
			szCurDir[i] = '\0';
			break;
		}
	}

	dlg.m_ofn.lpstrInitialDir = szCurDir;
    dlg.m_ofn.nMaxFile = 2* MAX_PATH; 
    dlg.m_ofn.lpstrFile = new TCHAR[dlg.m_ofn.nMaxFile]; 
    ZeroMemory(dlg.m_ofn.lpstrFile, sizeof(TCHAR) * dlg.m_ofn.nMaxFile); 

	
	int retval = dlg.DoModal();
	if(retval==IDCANCEL)
	return ;
	POSITION pos_file;
	pos_file = dlg.GetStartPosition();
	//CArray<CString, CString> ary_filename;
	while(pos_file != NULL)
		m_aryDecoder.Add(dlg.GetNextPathName(pos_file));
	//ary_filename.Add(dlg.GetNextPathName(pos_file));
	
}


LRESULT CECtoolDlg::OnInfoWindowClosed(WPARAM wParam,LPARAM lparam)
{
	((CWnd*)GetDlgItem(IDC_BUTTON_SHOW_STEP_INFO))->EnableWindow(TRUE);
	((CWnd*)GetDlgItem(IDC_BUTTON_SHOW_ALL_INFO))->EnableWindow(TRUE);
	m_bViewStepInfo = FALSE;
	m_bViewAllInfo = FALSE;
	return S_OK;
}

void CECtoolDlg::OnBnClickedButtonShowStepInfo()
{
	// TODO: Add your control notification handler code here
	m_pInfoDlg = new CECinfoDlg(this);
	m_pInfoDlg->Create(CECinfoDlg::IDD);
	m_pInfoDlg->ShowWindow(SW_SHOW);
	InitInfoDlg();
	m_pInfoDlg->UpdateData(FALSE);
	m_bViewStepInfo = TRUE;
	((CWnd*)GetDlgItem(IDC_BUTTON_SHOW_STEP_INFO))->EnableWindow(FALSE);
	((CWnd*)GetDlgItem(IDC_BUTTON_SHOW_ALL_INFO))->EnableWindow(FALSE);
}

void CECtoolDlg::OnBnClickedButtonShowAllInfo()
{
	// TODO: Add your control notification handler code here
	m_pInfoDlg = new CECinfoDlg(this);
	m_pInfoDlg->Create(CECinfoDlg::IDD);
	m_pInfoDlg->ShowWindow(SW_SHOW);
	ShowInfoDlg();
	m_pInfoDlg->UpdateData(FALSE);
	m_bViewAllInfo = TRUE;
	((CWnd*)GetDlgItem(IDC_BUTTON_SHOW_STEP_INFO))->EnableWindow(FALSE);
	((CWnd*)GetDlgItem(IDC_BUTTON_SHOW_ALL_INFO))->EnableWindow(FALSE);
}




void CECtoolDlg::OnBnClickedRadio3()
{
	// TODO: Add your control notification handler code here
	// TODO: Add file into 
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, this);
	TCHAR szCurDir[MAX_PATH];
	memset(szCurDir,0,sizeof(szCurDir));
	GetModuleFileName(NULL, szCurDir, MAX_PATH);
	for(int i= MAX_PATH-1; i>=0; i--)
	{
		if(szCurDir[i] == '\\')
		{
			szCurDir[i] = '\0';
			break;
		}
	}

	dlg.m_ofn.lpstrInitialDir = szCurDir;
	CString strFilePath;
	if(dlg.DoModal() == IDOK)
	{
		m_strLossSimulatorCfg = dlg.GetPathName();	
	}
	

}


