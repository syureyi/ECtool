#include "stdafx.h"
#include "ECtool.h"
#include "ECinfoDlg.h"
#include "ECtoolParam.h"


// CECinfoDlg dialog

IMPLEMENT_DYNAMIC(CECinfoDlg, CDialog)

CECinfoDlg::CECinfoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CECinfoDlg::IDD, pParent)
	, m_iFrameNoOrig(0)
	, m_fPsnrOrig(0)
	, m_iScore(0)
	, m_iAvgScore(0)
	, m_iFrameNum(0)
	, m_iFrameNoDec1(0)
	, m_iFrameNoDec2(0)
	, m_fPSNRDec1(0)
	, m_fPSNRDec2(0)
	, m_iScoreDec1(0)
	, m_iScoreDec2(0)
	, m_iAvgScoreDec1(0)
	, m_iAvgScoreDec2(0)
	, m_iFrameNumDec1(0)
	, m_iFrameNumDec2(0)
{
	

	


}

CECinfoDlg::~CECinfoDlg()
{
}

void CECinfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_FRAMENO_ORIG, m_iFrameNoOrig);
	DDX_Text(pDX, IDC_EDIT_PSNR_ORIG, m_fPsnrOrig);
	//DDX_Control(pDX, IDC_COMBO_SCORE_ORIG, m_comboOrig);
	DDX_Text(pDX, IDC_EDIT_SCORE_ORIG, m_iScore);
	DDX_Text(pDX, IDC_EDIT_AVG_SCORE, m_iAvgScore);
	DDX_Text(pDX, IDC_EDIT_FRAME_NUM, m_iFrameNum);
	DDX_Text(pDX, IDC_EDIT_FRAMENO_DEC1, m_iFrameNoDec1);
	DDX_Text(pDX, IDC_EDIT_FRAMENO_DEC2, m_iFrameNoDec2);
	DDX_Text(pDX, IDC_EDIT_PSNR_DEC1, m_fPSNRDec1);
	DDX_Text(pDX, IDC_EDIT_PSNR_DEC2, m_fPSNRDec2);
	DDX_Text(pDX, IDC_EDIT_SCORE_DEC1, m_iScoreDec1);
	DDX_Text(pDX, IDC_EDIT_SCORE_DEC2, m_iScoreDec2);
	DDX_Text(pDX, IDC_EDIT_AVG_SCORE2, m_iAvgScoreDec1);
	DDX_Text(pDX, IDC_EDIT_AVG_SCORE3, m_iAvgScoreDec2);
	DDX_Text(pDX, IDC_EDIT_FRAME_NUM2, m_iFrameNumDec1);
	DDX_Text(pDX, IDC_EDIT_FRAME_NUM3, m_iFrameNumDec2);
}


BEGIN_MESSAGE_MAP(CECinfoDlg, CDialog)
	ON_WM_CLOSE()
	ON_EN_CHANGE(IDC_EDIT_SCORE_DEC1, &CECinfoDlg::OnEnChangeEditScoreDec1)
	ON_EN_CHANGE(IDC_EDIT_SCORE_DEC2, &CECinfoDlg::OnEnChangeEditScoreDec2)
END_MESSAGE_MAP()


// CECinfoDlg message handlers


void CECinfoDlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: Add your message handler code here

}

void CECinfoDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	if(GetParent())
	{
		GetParent()->SendMessage(WM_MESSAGE_INFO_WINDOW_CLOSE, 0, 0);
	}
	CDialog::OnClose();
}

void CECinfoDlg::OnEnChangeEditScoreDec1()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
    UpdateData(TRUE);

	GetParent()->SendMessage(WM_MESSAGE_CHANGE_EDIT,0,0);
	UpdateData(FALSE);
	// TODO:  Add your control notification handler code here
}

void CECinfoDlg::OnEnChangeEditScoreDec2()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
    UpdateData(TRUE);

	GetParent()->SendMessage(WM_MESSAGE_CHANGE_EDIT,0,0);
	UpdateData(FALSE);
	// TODO:  Add your control notification handler code here
}
