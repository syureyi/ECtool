#ifndef _EC_PLAYCONTROL_H_
#define _EC_PLAYCONTROL_H_
#pragma once
#include "ECtoolParam.h"



class ECplayControl
{

public:
	ECplayControl();
	virtual ~ECplayControl();
	void ECstart(DEMO_WINDOW_TYPE eWindowType,HWND* hwnd);
	void ECplay();
	void ECpause();
	void ECresume();
	void ECstepAfter();
	void ECstepBefore();
	void ECstop(DEMO_WINDOW_TYPE eWindowType);	
	void ECsetPlayComment(DEMO_WINDOW_TYPE eWindowType,SPlayFrameInfo sFrameInfo);
	void ECInitPlayPosition(DEMO_WINDOW_TYPE eWindowType);
	void ECrenderPosChange(DEMO_WINDOW_TYPE eWindowType);
	void ECplayThread(int iRound);
	void ECreDraw(DEMO_WINDOW_TYPE eWindowType);
	void ECsetPlayFrameRate(int iFrameRate);
	void ECdisplayChange(DEMO_WINDOW_TYPE eWindowType);
	SFrameInfo ECgetFrameInfo(DEMO_WINDOW_TYPE eWindowType);
	void ECsetFrameInfo(int iScore1,int iScore2);
	void ECsetInfo(int iScore1,int iScore2);
	SFrameInfo ECgetInfo(DEMO_WINDOW_TYPE eWindowType); 
	SFrameInfo ECinitFrameInfo(DEMO_WINDOW_TYPE eWindowType);
	void ECunInit(DEMO_WINDOW_TYPE eWindowType);
	double ECsetPsnrInfo(unsigned char* pDecBuf);
	

private:
	IWseVideoRenderer* g_pOrigVideoRenderer;
	IWseVideoRenderer* g_pDec1VideoRenderer;
	IWseVideoRenderer* g_pDec2VideoRenderer;
	int m_iRound;// 
	bool m_bOrigExist;
	bool m_bDec1Exist;
	bool m_bDec2Exist;//judge render which
	int m_iFrameRate;//play rate	
	std::list<SPlayFrameInfo> m_listFrameInfoOrig;
    std::list<SPlayFrameInfo> m_listFrameInfoDec1;
    std::list<SPlayFrameInfo> m_listFrameInfoDec2;
	std::list<SPlayFrameInfo>::iterator m_iFrameInfoOrig;
	std::list<SPlayFrameInfo>::iterator m_iFrameInfoDec1;
	std::list<SPlayFrameInfo>::iterator m_iFrameInfoDec2;
	SFrameInfo m_sInfoOrig;
	SFrameInfo m_sInfoDec1;
	SFrameInfo m_sInfoDec2;
	bool m_bOrigInit;
	bool m_bDec1Init;
	bool m_bDec2Init;
};
#endif