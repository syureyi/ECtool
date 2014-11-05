#include "stdafx.h"
#include "ECplayControl.h"
#include "./psnr/JSVMPsnr.h"


BOOL m_bRun=TRUE;
HANDLE hThread; 
DWORD ThreadID;

void ThreadFunc(LPVOID *pInfo)
{

    ECplayControl* pECplayControl = (ECplayControl*)pInfo;
	while(m_bRun)
	{

		pECplayControl->ECplay();
		
	}

}


ECplayControl::ECplayControl()
{
   g_pOrigVideoRenderer = 0;
   g_pDec1VideoRenderer = 0;
   g_pDec2VideoRenderer = 0;
   m_bDec1Exist = false;
   m_bDec2Exist = false;
   m_bOrigExist = false;
 


}
ECplayControl::~ECplayControl()
{
	delete g_pOrigVideoRenderer;
    delete g_pDec1VideoRenderer;
    delete g_pDec2VideoRenderer;

}


void ECplayControl::ECsetPlayComment(DEMO_WINDOW_TYPE eWindowType,SPlayFrameInfo sFrameInfo)
{
	switch(eWindowType)
	{
	case DEMO_WINDOW_ORIG:
		m_listFrameInfoOrig.push_back(sFrameInfo);
       break;
	case DEMO_WINDOW_DEC1:
		m_listFrameInfoDec1.push_back(sFrameInfo);
		break;
	case DEMO_WINDOW_DEC2:
		m_listFrameInfoDec2.push_back(sFrameInfo);
	default:
		break;
	}

}
void ECplayControl::ECInitPlayPosition(DEMO_WINDOW_TYPE eWindowType)
{
	switch(eWindowType)
	{
		case DEMO_WINDOW_ORIG:			
			m_bOrigExist = true;
			break;
		case DEMO_WINDOW_DEC1:			
			m_bDec1Exist = true;
			break;
		case DEMO_WINDOW_DEC2:	
			m_bDec2Exist = true;
			break;
		default:
			break;
	}
	if(m_bOrigExist)
	{
		m_iFrameInfoOrig = m_listFrameInfoOrig.begin();
		m_sInfoOrig = ECinitFrameInfo(DEMO_WINDOW_ORIG);
		m_bOrigInit = true;
		
	}	
	if(m_bDec1Exist)
	{ 
		
		m_iFrameInfoDec1 = m_listFrameInfoDec1.begin();
		m_sInfoDec1 = ECinitFrameInfo(DEMO_WINDOW_DEC1);
		m_bDec1Init = true;
		
	}	
	if(m_bDec2Exist)
	{
		m_iFrameInfoDec2 = m_listFrameInfoDec2.begin();
		m_sInfoDec2 = ECinitFrameInfo(DEMO_WINDOW_DEC2);
		m_bDec2Init = true;
	
	}

	
}

void ECplayControl::ECunInit(DEMO_WINDOW_TYPE eWindowType)
{
	switch(eWindowType)
	{
		case DEMO_WINDOW_ORIG:			
			m_bOrigExist = false;
			break;
		case DEMO_WINDOW_DEC1:			
			m_bDec1Exist = false;
			break;
		case DEMO_WINDOW_DEC2:	
			m_bDec2Exist = false;
			break;
		default:
			break;
	}
}

SFrameInfo ECplayControl::ECinitFrameInfo(DEMO_WINDOW_TYPE eWindowType)
{
   std::list<SPlayFrameInfo>::iterator it;
	SFrameInfo sFrameInfo={0,0,0,0,0};
	int infoCount =0;
	std::list<SPlayFrameInfo> m_listFrameInfo;
	switch(eWindowType)
	{
		case DEMO_WINDOW_ORIG:
			m_listFrameInfo = m_listFrameInfoOrig;
			break;
		case DEMO_WINDOW_DEC1:
			m_listFrameInfo = m_listFrameInfoDec1;
			break;
		case DEMO_WINDOW_DEC2:
			m_listFrameInfo = m_listFrameInfoDec2;
			break;
		default:
			break;
	}

	for(it= m_listFrameInfo.begin();it != m_listFrameInfo.end();it++)
	{
		sFrameInfo.fPsnr += (*it).fPsnr;
		sFrameInfo.iScore += (*it).iScore;
        sFrameInfo.iFrameNo = (*it).iFrameNo; 
		infoCount++;
	
	}
	sFrameInfo.fPsnr = sFrameInfo.fPsnr / infoCount;
	
  return sFrameInfo;
}

double ECplayControl::ECsetPsnrInfo(unsigned char* pDecBuf)
{
	
	std::list<SPlayFrameInfo>::iterator itOrig= m_iFrameInfoOrig;
	unsigned char* pOrigBuf=NULL;	
	(*itOrig).pSample->GetDataPointer(&pOrigBuf);
	CJSVMPsnr* cPSNR = new CJSVMPsnr((*itOrig).iFrameWidth,(*itOrig).iFrameHeight,pOrigBuf,pDecBuf);
	itOrig++;
	m_iFrameInfoOrig=itOrig;
	return cPSNR->getFramePSNR();

}
void ECplayControl::ECstart(DEMO_WINDOW_TYPE eWindowType,HWND* hwnd)
{
	switch(eWindowType)
	{
		case DEMO_WINDOW_ORIG:
			if(g_pOrigVideoRenderer == 0)
			CreateVideoRenderer(*hwnd,&g_pOrigVideoRenderer);
			//m_bOrigExist = true;
			break;
		case DEMO_WINDOW_DEC1:
			if(g_pDec1VideoRenderer == 0)
			CreateVideoRenderer(*hwnd,&g_pDec1VideoRenderer);
			//m_bDec1Exist = true;
			break;
		case DEMO_WINDOW_DEC2:
			if(g_pDec2VideoRenderer == 0)
			CreateVideoRenderer(*hwnd,&g_pDec2VideoRenderer);
			//m_bDec2Exist = true;
			break;
		default:
			break;
	}

}

void ECplayControl::ECdisplayChange(DEMO_WINDOW_TYPE eWindowType)
{
	switch(eWindowType)
	{
	case DEMO_WINDOW_ORIG:
		g_pOrigVideoRenderer->OnDisplayChange();
		break;
	case DEMO_WINDOW_DEC1:
		g_pDec1VideoRenderer->OnDisplayChange();
		break;
	case DEMO_WINDOW_DEC2:
		g_pDec2VideoRenderer->OnDisplayChange();
		break;
	default:
		break;
	}
	
}

void ECplayControl::ECrenderPosChange(DEMO_WINDOW_TYPE eWindowType)
{
	switch(eWindowType)
	{
	case DEMO_WINDOW_ORIG:
		g_pOrigVideoRenderer->OnWindowPositionChange();
		break;
	case DEMO_WINDOW_DEC1:
		if(g_pDec1VideoRenderer !=0)
		g_pDec1VideoRenderer->OnWindowPositionChange();
		break;
	case DEMO_WINDOW_DEC2:
		if(g_pDec2VideoRenderer !=0)
		g_pDec2VideoRenderer->OnWindowPositionChange();
		break;
	default:
		break;
	}
}

void ECplayControl::ECreDraw(DEMO_WINDOW_TYPE eWindowType)
{
	switch(eWindowType)
	{
	case DEMO_WINDOW_ORIG:
		g_pOrigVideoRenderer->ReDraw();
		break;
	case DEMO_WINDOW_DEC1:
		g_pDec1VideoRenderer->ReDraw();
		break;
	case DEMO_WINDOW_DEC2:
		g_pDec2VideoRenderer->ReDraw();
		break;
	default:
		break;
	}
}

void ECplayControl::ECplayThread(int iRound)
{
	m_iRound = iRound;
	m_bRun = TRUE;
	//ECplay();
	hThread = CreateThread(NULL,
		0,
		(LPTHREAD_START_ROUTINE)ThreadFunc,
		(void*)this,
		0,
		&ThreadID);


}


void ECplayControl::ECsetPlayFrameRate(int iFrameRate)
{
  m_iFrameRate = iFrameRate;
}

SFrameInfo ECplayControl::ECgetFrameInfo(DEMO_WINDOW_TYPE eWindowType)
{
	SFrameInfo sFrameInfo={0,0,0,0,0};
	std::list<SPlayFrameInfo>::iterator it;
	switch(eWindowType)
	{
		case DEMO_WINDOW_ORIG:
			if(m_listFrameInfoOrig.empty())
				return sFrameInfo;
			else
			{
			it = m_iFrameInfoOrig;
			break;
			}
		case DEMO_WINDOW_DEC1:
			if(m_listFrameInfoDec1.empty())
				return sFrameInfo;
			else
			{
			it = m_iFrameInfoDec1;
			break;
			}
		case DEMO_WINDOW_DEC2:
			if(m_listFrameInfoDec2.empty())
				return sFrameInfo;
			else
			{
			it = m_iFrameInfoDec2;
			break;
			}
		default:
			break;
	}

	
	sFrameInfo.iFrameHeight = (*it).iFrameHeight;
	sFrameInfo.iFrameWidth = (*it).iFrameWidth;
	sFrameInfo.iFrameNo = (*it).iFrameNo;
	sFrameInfo.iScore = (*it).iScore;
	sFrameInfo.fPsnr = (*it).fPsnr;
	return sFrameInfo;

}


SFrameInfo ECplayControl::ECgetInfo(DEMO_WINDOW_TYPE eWindowType)
{   
	switch(eWindowType)
	{
	case DEMO_WINDOW_ORIG:
		return m_sInfoOrig;
		break;
	case DEMO_WINDOW_DEC1:
		return m_sInfoDec1;
		break;
	case DEMO_WINDOW_DEC2:
		return m_sInfoDec2;
		break;
	default:
		break;
	}

}

void ECplayControl::ECsetFrameInfo(int iScore1,int iScore2)
{
	(*m_iFrameInfoDec1).iScore = iScore1;
	(*m_iFrameInfoDec2).iScore = iScore2;
}

void ECplayControl::ECsetInfo(int iScore1,int iScore2)
{
	m_sInfoDec1.iScore = iScore1;
	m_sInfoDec2.iScore = iScore2;
}


void ECplayControl::ECplay()
{
	DWORD ms=0;
	bool bOrigExist = m_bOrigExist;
	bool bDec1Exist = m_bDec1Exist;
	bool bDec2Exist = m_bDec2Exist;
	ms= (1000/m_iFrameRate);
	while(m_iRound>0 && m_bRun)
	{
		if(bOrigExist)
		{
			if(m_bOrigInit)
			{
				m_iFrameInfoOrig=m_listFrameInfoOrig.begin();
				m_bOrigInit = false;
			}
			else
			{
				m_iFrameInfoOrig++;
				if(m_iFrameInfoOrig == m_listFrameInfoOrig.end())
				{
					m_iFrameInfoOrig--;
					m_bOrigInit = true;
					m_iRound = m_iRound - 1;
					bOrigExist = false;
				}
				else
                 g_pOrigVideoRenderer->DeliverImage(((*m_iFrameInfoOrig).pSample));

			}
		}
		if(bDec1Exist)
		{
			if(m_bDec1Init)
			{
				m_bDec1Init = false;
				m_iFrameInfoDec1 = m_listFrameInfoDec1.begin();
			}
			else
			{
				m_iFrameInfoDec1++;
				if(m_iFrameInfoDec1 == m_listFrameInfoDec1.end())
				{
					m_iFrameInfoDec1--;
					m_bDec1Init=true;
					m_iRound = m_iRound -1;
					bDec1Exist = FALSE;
				}
				else
					g_pDec1VideoRenderer->DeliverImage((*m_iFrameInfoDec1).pSample);
			}
		}
		if(bDec2Exist)
		{
			if(m_bDec2Init)
			{
				m_bDec2Init = false;
				m_iFrameInfoDec2 = m_listFrameInfoDec2.begin();
			}
			else
			{
				m_iFrameInfoDec2++;
				if(m_iFrameInfoDec2 == m_listFrameInfoDec2.end())
				{
					m_iFrameInfoDec2--;
					m_bDec2Init=true;
					m_iRound = m_iRound -1;
					bDec2Exist = FALSE;
				}
				else
					g_pDec2VideoRenderer->DeliverImage((*m_iFrameInfoDec2).pSample);
			}
		}
		Sleep(ms);
	}

	m_bRun = false;


}
void ECplayControl::ECstepBefore()
{
	if(m_bOrigExist)
		{
			
			if(m_iFrameInfoOrig != m_listFrameInfoOrig.begin())
			   m_iFrameInfoOrig--;
			g_pOrigVideoRenderer->DeliverImage((*m_iFrameInfoOrig).pSample);

		}
	
	if(m_bDec1Exist)
		{
			
			if(m_iFrameInfoDec1 != m_listFrameInfoDec1.begin())
				m_iFrameInfoDec1--;
			g_pDec1VideoRenderer->DeliverImage((*m_iFrameInfoDec1).pSample);
		 
		}
		
	if(m_bDec2Exist)
		{
			if(m_iFrameInfoDec2 != m_listFrameInfoDec2.begin())
				m_iFrameInfoDec2--;
			g_pDec2VideoRenderer->DeliverImage((*m_iFrameInfoDec2).pSample);
		
		 
		}

}

void ECplayControl::ECstepAfter()
{
	
	if(m_bOrigExist)
		{
			if(m_bOrigInit)
			{
				m_bOrigInit = false;
				m_iFrameInfoOrig = m_listFrameInfoOrig.begin();
			}
			else
			{
				m_iFrameInfoOrig++;
				if(m_iFrameInfoOrig ==m_listFrameInfoOrig.end())
				{
					m_iFrameInfoOrig--;
					m_bOrigInit = true;
				}
			}
			g_pOrigVideoRenderer->DeliverImage((*m_iFrameInfoOrig).pSample);
		
		}
	
	
	if(m_bDec1Exist)
		{			
			if(m_bDec1Init)
			{
				m_bDec1Init = false;
				m_iFrameInfoDec1 = m_listFrameInfoDec1.begin();
			}
			else
			{
				m_iFrameInfoDec1++;
				if(m_iFrameInfoDec1 ==m_listFrameInfoDec1.end())
				{
					m_iFrameInfoDec1--;
					m_bDec1Init = true;
				}
			}
			g_pDec1VideoRenderer->DeliverImage((*m_iFrameInfoDec1).pSample);
		 
		}
		
	if(m_bDec2Exist)
		{		
			if(m_bDec2Init)
			{
				m_bDec2Init = false;
				m_iFrameInfoDec2 = m_listFrameInfoDec2.begin();
			}
			else
			{
				m_iFrameInfoDec2++;
				if(m_iFrameInfoDec2 ==m_listFrameInfoDec2.end())
				{
					m_iFrameInfoDec2--;
					m_bDec2Init = true;
				}
			}
			g_pDec2VideoRenderer->DeliverImage((*m_iFrameInfoDec2).pSample);
		}

}


void ECplayControl::ECstop(DEMO_WINDOW_TYPE eWindowType)
{

	switch(eWindowType)
	{
	case DEMO_WINDOW_ORIG:
		g_pOrigVideoRenderer = 0;
		//m_listOrigFrames.clear();
		m_listFrameInfoOrig.clear();
		break;
	case DEMO_WINDOW_DEC1:
		g_pDec1VideoRenderer = 0;
		//m_listDecodedFrames.clear();
		m_listFrameInfoDec1.clear();
		break;
	case DEMO_WINDOW_DEC2:
		g_pDec2VideoRenderer = 0;
		//m_listDecodedFrames2.clear();
		m_listFrameInfoDec2.clear();
		break;
	default:
		break;
	}
	if(!m_bDec1Exist&&!m_bDec2Exist&&!m_bOrigExist)
	TerminateThread(hThread,ThreadID); 
	else
		m_bRun = false;
	
}

void ECplayControl::ECpause()
{
	
	m_bRun = FALSE;
	
}