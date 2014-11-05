// ECtoolDlg.h : header file
//

#pragma once
#include "ECtoolParam.h"
#include "ECplayControl.h"
#include "./api/svc/codec_api.h"
#include "ECSample.h"
#include "ECinfoDlg.h"
#include "afxwin.h"
// CECtoolDlg dialog
class CECtoolDlg : public CDialog
{
// Construction
public:
	CECtoolDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_ECTOOL_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	CString m_strLoadFileName;
	CString m_strDec1FileName;
	CString m_strDec2FileName;
	CString m_strLosSimlFileName;
	CString m_strLossSimulatorCfg;


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedButtonLoadFile();
	afx_msg LRESULT OnRenderWindowClosed(WPARAM wParam,LPARAM lparam);
	afx_msg LRESULT OnRenderDisplayResolutionChanged(WPARAM wParam,LPARAM lparam);
	afx_msg LRESULT OnRenderWindowPositionChanged(WPARAM wParam,LPARAM lparam);
	afx_msg LRESULT OnScoreChanged(WPARAM wParam, LPARAM lparam);
	HWND createWindow(char *pName, DEMO_WINDOW_TYPE eWindowType);
	void ReadFileToSample(DEMO_WINDOW_TYPE eWindowType);
	int ReadBSToSample(DEMO_WINDOW_TYPE eWindowType);
	void CleanUp();
	void DoDecoder();
	void Decoder ();
	void InitSampleAndPlay(DEMO_WINDOW_TYPE eWindowType,HWND* hWnd);
	void UninitSampleAndPlay(DEMO_WINDOW_TYPE eWindowType,HWND hWnd);
	afx_msg LRESULT OnRenderDisplay(WPARAM wParam,LPARAM lparam);
	afx_msg LRESULT OnInfoWindowClosed(WPARAM wParam,LPARAM lparam);
	void InitInfoDlg();
	void ShowInfoDlg();
	
private:
	/*HWND					m_previewerWindowHandle;
	HWND                    m_dec1viewerWindowHandle;
	HWND                    m_dec2viewerWindowHandle;*/
	IWseVideoSampleAllocator*  m_pAlloc;
	WseVideoFormat m_format;
	ECplayControl* m_pPlayControl;
	//std::list<FrameData> m_listDecodedFrames;
	std::list<IWseVideoSample*> m_listDecodedFrames;
	std::list<IWseVideoSample*> m_listOrigFrames;
	std::list<IWseVideoSample*> m_listDecodedFrames2;
	
public:

	afx_msg void OnBnClickedButtonPause();
	afx_msg void OnBnClickedButtonStepAfter();
	afx_msg void OnBnClickedButtonPlay();
	afx_msg void OnBnClickedButtonStepBefore();
	afx_msg void OnBnClickedButtonPlayFirst();
	
private:
	UINT m_nFrameWidth;
	UINT m_nFrameHeight;
	BOOL m_bOrigChecked;
	BOOL m_bDec1Checked;
	BOOL m_bDec2Checked;
	ISVCDecoder* m_pDecoder;
	CStringArray  m_aryDecoder;
	CECinfoDlg * m_pInfoDlg;
	
	//CArray<CString, CString> m_aryDecoder;
	ECSample m_Sample;
	BOOL m_bEnableEC;
    BOOL m_bViewStepInfo;
	BOOL m_bViewAllInfo;

public:
	afx_msg void OnBnClickedButtonDec1Dll();
//	CButton m_Radio;
	afx_msg void OnBnClickedButtonLoss();
	int m_nRadioLossType;
	BOOL m_bInitOrig;
	BOOL m_bInitDec1;
	BOOL m_bInitDec2;
	UINT m_ifps;
	afx_msg void OnBnClickedButtonShowInfo();
	
	afx_msg void OnBnClickedButtonShowStepInfo();
	afx_msg void OnBnClickedButtonShowAllInfo();
	
	
	afx_msg void OnBnClickedRadio3();
	
	int m_nRadioFileType;
	
};
