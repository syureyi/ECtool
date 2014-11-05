
#pragma once
#include "afxwin.h"

// CECinfoDlg dialog

class CECinfoDlg : public CDialog
{
	DECLARE_DYNAMIC(CECinfoDlg)

public:
	CECinfoDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CECinfoDlg();

// Dialog Data
	enum { IDD = IDD_INFO_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	int m_iFrameNoOrig;
	double m_fPsnrOrig;
	int m_iScore;
	
	int m_iAvgScore;
	int m_iFrameNum;
	int m_iFrameNoDec1;
	int m_iFrameNoDec2;
	double m_fPSNRDec1;
	double m_fPSNRDec2;
	int m_iScoreDec1;
	int m_iScoreDec2;
	int m_iAvgScoreDec1;
	int m_iAvgScoreDec2;
	int m_iFrameNumDec1;
	int m_iFrameNumDec2;
		afx_msg void OnDestroy();
	afx_msg void OnClose();
	afx_msg void OnEnChangeEditScoreDec1();
	afx_msg void OnEnChangeEditScoreDec2();
};
