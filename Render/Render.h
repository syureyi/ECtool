#ifndef _RENDER_H_
#define _RENDER_H_

#if defined(WIN32)
#if !defined(POINTER_64) && _MSC_VER > 1200
#define POINTER_64 __ptr64
#endif
#endif

//#include "jlbaseimp.h"
//#include "WseEngine.h"

#include "Draw.h"
#include "../common/WseHeapMem.h"
#include "../api.h"
#include "../common/WseMutex.h"
//#include "hookcollector.h"

//#include "IWelsVP.h"
//#include "WseDebug.h"

WSERESULT CreateVideoRenderer(void* hRenderTargetWnd,IWseVideoRenderer** ppVideoRenderer);
//WSERESULT CreateWindowLessVideoRenderer(IWseVideoRenderer** ppVideoRenderer);

#include "map"
typedef std::map<unsigned int,MMD_BMP_RENDER_STRUCT> ID2BMPMap;

#include "list"
typedef std::list<CDraw*> DrawObjectList;


class CWseVideoRenderer :  public IWseVideoRenderer,
						   public IDrawHelper
{
							
public:
	CWseVideoRenderer();
	virtual ~CWseVideoRenderer();

public:
	// IJlUnknown
	//IMPLEMENT_JLREFERENCE
	//for debug purpose
	//JLMETHOD_(unsigned long,AddRef)()
	//{
	//	unsigned long dwRet = CJlUnknown::AddRef();
	//	WSE_INFO_TRACE_THIS("CWseVideoRenderer::AddRef(), m_uRef= "<<m_uRef);
	//	return dwRet;
	//};
	//BEGIN_QI_HANDLER(CWseVideoRenderer)
	//	WSE_QI_HANDLER(IWseVideoRenderer)
	//	WSE_QI_HANDLER(IWseVideoDeliverer)
	//	WSE_QI_HANDLER(IWseVideoDeliverer_VA)
	//	WSE_QI_HANDLER(IWseVideoRendererWinProfile)
	//	WSE_QI_HANDLER(IWseVideoRendererEffect)
	//END_QI_HANDLER()

	//// IWseVideoRenderer

	//JLMETHOD_(unsigned long,Release)()
	//{
	//	WSE_INFO_TRACE_THIS("CWseVideoRenderer::Release(), m_uRef= "<<m_uRef);
	//	return CJlUnknown::Release();
	//}
	virtual WSERESULT DeliverImage(IWseVideoSample* pSample);
	WSERESULT DeliverImage(VideoRawDataPack* pVideoPack)
	{
		return WSE_S_OK;
	}
	WSERESULT AddRenderPicture(unsigned int uiID,PMMD_BMP_RENDER_STRUCT pPicData);
	WSERESULT RemovePicture(unsigned int uiID);
	WSERESULT GetRenderPictureDesc(unsigned int uiID,PMMD_BMP_RENDER_STRUCT pPicData);
	WSERESULT ReDraw();
	WSERESULT LockDraw();
	WSERESULT UnlockDraw();
	WSERESULT SetBKColor(void* color);
	WSERESULT ClearStream();
	WSERESULT SetAspectRatioMode(WseRendererAspectRatioMode mode);
	WSERESULT GetAspectRatioMode(WseRendererAspectRatioMode *pmode);

	// IWseVideoRendererWinProfile
	WSERESULT OnDisplayChange();
	WSERESULT OnWindowPositionChange();
	WSERESULT SetRendererMode(WseRendererMode mode);
	WSERESULT GetRendererMode(WseRendererMode *pmode);
	WSERESULT GetFinalRenderPixelFormat(WseVideoType *ptype);
	WSERESULT UpdateOnlyViaReDraw(bool bYes);

	// IWseVideoDeliverer_VA
	//WSERESULT VAGetCurrentSupportedType(/* out */unsigned long *pulVAType);
	//WSERESULT VADeliverImage(const VAContent *pContent);

	// IWseVideoRendererEffect
	/*WSERESULT SetEffect(unsigned int type,void* detail);
	WSERESULT ClearEffect();*/

	// IDrawHelper (Draw object use)
	void TriggerWindowThreadAction();

	virtual BOOL Init(HWND hRenderTargetWnd);

protected:
	//WSERESULT DoColorspaceConvertToBGR24VFlip();
	virtual WSERESULT DoDraw(BOOL bRedraw);

	virtual void Uninit();
	virtual void InitDrawContext();
	//virtual BOOL InitWelsVP();
	virtual BOOL UpdateDrawContext(int nDrawLevel);
	void UpdateMiscInfo();
	void TrytoRecoverDrawContext();

	BOOL IsWindowThread();
	void DestroyDrawContext();
protected:
	/*LRESULT HookProc(int code,WPARAM wParam,LPARAM lParam,BOOL& bHandled);*/
	WSERESULT DoDeliverImage(IWseVideoSample* pSample);
	//WSERESULT DoVADeliverImage(const VAContent *pContent);
protected:
	void AddToWTDList(CDraw* pDraw);
	void ClearWTDList();
	DrawObjectList m_listWindowThreadDestroy;
protected:
	HWND m_hWnd;
	DWORD m_dwTIDWindow;

#define msg_WTA	0	// window thread action
#define msg_WTD	1	// window thread delete
	UINT m_uMsgCustom;

	CWseMutex m_lock;

	BOOL m_bUpdateOnlyViaReDraw;

	typedef enum
	{
		IHT_None,
		IHT_Object,
		IHT_Globle
	}
	ImageHoldType;
	ImageHoldType m_iht;

	CWseHeapMem m_mem;
	CWseHeapMem m_memTmp;
	WseVideoFormat m_format;
	IWseVideoSample* m_pSample;
	
	CDraw* m_pDraw;
	int m_nCurDrawLevel;//0gdi 1d3d
	int m_nDrawLevel;
	DWORD m_dwRDCCount;
	
	DWORD m_dwImageEffect;
	
	WseRendererAspectRatioMode m_aspectratiomode;
	COLORREF m_clrBK;
	ID2BMPMap m_mapBitmaps;

	WseVideoType m_vtFinalType;
	
	BOOL m_bWindowThreadAction;

	RECT m_rcTarget;
protected:
	CWseVideoRenderer(const CWseVideoRenderer& rhs);
	CWseVideoRenderer& operator = (const CWseVideoRenderer& rhs);

	//typedef nsWseVP::vResult (WELSAPI *CREATEVPINTERFACE)(void **h, int version /*= WELSVP_INTERFACE_VERION*/);
	//typedef nsWseVP::vResult (WELSAPI *DESTROYVPINTERFACE)(void *h , int version /*= WELSVP_INTERFACE_VERION*/);
	//CREATEVPINTERFACE m_pfnCreateVpInterface;
	//DESTROYVPINTERFACE m_pfnDestroyVpInterface;
	//nsWseVP::IWelsVP * m_pWelsVP;

	typedef enum 
	{
		kGDIDraw = 0,
		kD3DDraw,
		kD2DDraw
	} eDrawLevel;
};

#endif //_WSEVIDEORENDERER_H_
