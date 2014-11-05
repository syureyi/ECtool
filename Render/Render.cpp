#include "Render.h"
//#include "Convert.h"
//#include "WseErrorTypes.h"
//#include "WseDebug.h"

//#include "WseConfig.h"
//#include "WseWindowLessVideoRenderer.h"
//#include "colorspacecvt.h"

//////////////////////////////////////////////////////////////////////////
WSERESULT CreateVideoRenderer(void* hRenderTargetWnd,IWseVideoRenderer** ppVideoRenderer)
{
	if(NULL == ppVideoRenderer)
		return WSE_E_INVALIDARG;
	CWseVideoRenderer* p = new CWseVideoRenderer;
	if(NULL == p)
		return WSE_E_OUTOFMEMORY;

	//p->AddRef();
	if(p->Init((HWND)hRenderTargetWnd))
	{
		*ppVideoRenderer = p;
		return WSE_S_OK;
	}
	//p->Release();
	return WSE_E_FAIL;
}

//////////////////////////////////////////////////////////////////////////
// CWseVideoRenderer
//////////////////////////////////////////////////////////////////////////
CWseVideoRenderer::CWseVideoRenderer()
{
	m_hWnd = NULL;
	m_dwTIDWindow = 0;
	memset(&m_format,0,sizeof(m_format));
	m_pSample = NULL;

	m_pDraw = NULL;
	m_bWindowThreadAction = FALSE;

	m_dwImageEffect = 0;
	
	m_clrBK = RGB(0,0,0);
	m_aspectratiomode = Wse_RARM_NONE;

	//modified by xuwei 05/21/2012
	//add a proper name for draw level
	m_nCurDrawLevel = m_nDrawLevel = kD3DDraw;
	m_dwRDCCount = 0;

	m_vtFinalType = WseUnknown;

	m_iht = IHT_None;

	memset(&m_rcTarget,0,sizeof(m_rcTarget));

	m_uMsgCustom = RegisterWindowMessage(_T("CWseVideoRenderer_CustomMessage"));

	m_bUpdateOnlyViaReDraw = FALSE;

	//m_pfnCreateVpInterface = NULL;
	//m_pfnDestroyVpInterface = NULL;
	//m_pWelsVP = NULL;
}
CWseVideoRenderer::~CWseVideoRenderer()
{
	Uninit();
}
//static std::string GetCurrentLibPath()
//{
//	std::string tempPath;
//	tempPath.resize(MAX_PATH);
//	GetModuleFileNameA((HMODULE)ghInstance, (char*)tempPath.c_str(), tempPath.size());
//	size_t found = tempPath.rfind('\\');
//	if (found != std::string::npos) {
//		tempPath.replace(found, std::string::npos, "\\");
//	}
//	return tempPath;
//}

//BOOL CWseVideoRenderer::InitWelsVP()
//{
//	m_pfnCreateVpInterface = CreateWseVpInterface;
//	m_pfnDestroyVpInterface = DestroyWseVpInterface;
//	
//	nsWseVP::vResult vr = m_pfnCreateVpInterface((void**)&m_pWelsVP,WELSVP_INTERFACE_VERION);
//	if(nsWseVP::vRet_Success != vr || NULL == m_pWelsVP)
//	{
//		WSE_ERROR_TRACE("CWseVideoRenderer::Init() m_pfnCreateVpInterface failed! vr = "<<vr<<", m_pWelsVP = "<<m_pWelsVP);
//		return FALSE;
//	}
//	return TRUE;
//}
//need modification Xuwei 05/18/2012
//we have no HWND since it's widowless mode
BOOL CWseVideoRenderer::Init(HWND hRenderTargetWnd)
{
	CWseMutexGuard lck(m_lock);
	//WSE_INFO_TRACE("CWseVideoRenderer::Init() hRenderTargetWnd = "<<hRenderTargetWnd<<", this = "<<this);

	if(FALSE == IsWindow(hRenderTargetWnd))
	{
		//WSE_ERROR_TRACE("CWseVideoRenderer::Init() IsWindow("<<hRenderTargetWnd<<") failed! errorcode = "<<GetLastError());
		return FALSE;
	}

	// welsvp
	/*if ( !InitWelsVP() )
		return FALSE;
		*/

	m_dwTIDWindow = GetWindowThreadProcessId(hRenderTargetWnd,NULL);

	if(!IsWindowThread())
	{// want windows thread!
		//WSE_WARN_TRACE("CWseVideoRenderer::Init, !IsWindowThread(),Init() expect be done using the same thread as the window!");
	}

	//if(FALSE == SetCentralizedHook(m_dwTIDWindow,WH_GETMESSAGE,this))
	//{
	//	WSE_ERROR_TRACE("CWseVideoRenderer::Init, SetCentralizedHook failed!");
	//	return FALSE;
	//}
	
	m_hWnd = hRenderTargetWnd;
	InitDrawContext();
	GetClientRect(m_hWnd,&m_rcTarget);// init m_rcTarget
	return (m_pDraw != NULL);
}
//need modification Xuwei 05/18/2012

void CWseVideoRenderer::Uninit()
{
	CWseMutexGuard lck(m_lock);
	//WSE_INFO_TRACE("CWseVideoRenderer::Uninit() m_hWnd = "<<m_hWnd<<", this = "<<this);

	if(!IsWindowThread())
	{// want windows thread!
		//WSE_ERROR_TRACE("CWseVideoRenderer::Uninit,!IsWindowThread(),Uninit() must be done using the same thread as the window!");
	}

	//UnhookCentralizedHook(m_dwTIDWindow,WH_GETMESSAGE,this);

	DestroyDrawContext();

	ClearWTDList();

	m_clrBK = RGB(0,0,0);

	m_iht = IHT_None;

	memset(&m_format,0,sizeof(m_format));
	if(m_pSample)
	{
		//m_pSample->Release();
		m_pSample = NULL;
	}

	m_vtFinalType = WseUnknown;

	memset(&m_rcTarget,0,sizeof(m_rcTarget));

	//{// welsvp
	//	if(m_pWelsVP && m_pfnDestroyVpInterface)
	//		m_pfnDestroyVpInterface(m_pWelsVP,WELSVP_INTERFACE_VERION);
	//	m_pWelsVP = NULL;
	//	m_pfnCreateVpInterface = NULL;
	//	m_pfnDestroyVpInterface = NULL;
	//}
}
//WSERESULT CWseVideoRenderer::VAGetCurrentSupportedType(/* out */unsigned long *pulVAType)
//{
//	if(NULL == pulVAType)
//	{
//		WSE_ERROR_TRACE("CWseVideoRenderer::VAGetCurrentSupportedType() invalid argument. pulVAType is empty!");
//		return WSE_E_INVALIDARG;
//	}
//
//	CWseMutexGuard lck(m_lock);
//
//	if(NULL == m_pDraw)
//	{
//		WSE_ERROR_TRACE("CWseVideoRenderer::VAGetCurrentSupportedType() ,NULL == m_pDraw!!!");
//		return WSE_E_FAIL;
//	}
//
//	*pulVAType = 0;
//	VA_TYPE vatype = m_pDraw->GetSupportedVAType();
//	switch(vatype)
//	{
//	case VAT_D3D9Ex_ShareHandle_OffscreenPlainSurfaceEx:
//		*pulVAType |= VA_D3D9Ex_ShareHandle;
//		break;
//	}
//	return WSE_S_OK;
//}
//
//WSERESULT CWseVideoRenderer::VADeliverImage(const VAContent *pContent)
//{
//	WSERESULT wseRet = WSE_E_FAIL;
//	__try
//	{
//		wseRet = DoVADeliverImage(pContent);
//	}
//	__except(EXCEPTION_EXECUTE_HANDLER)
//	{
//		wseRet = WSE_E_FAIL;
//		WSE_DEBUG_ERROR("Exception raised in CWseVideoRenderer::DoVADeliverImage");
//	}
//
//	return wseRet;
//}
//
//WSERESULT CWseVideoRenderer::DoVADeliverImage(const VAContent *pContent)
//{
//	if(NULL == pContent)
//	{
//		WSE_ERROR_TRACE("CWseVideoRenderer::VADeliverImage() invalid argument. pContent is empty!");
//		return WSE_E_INVALIDARG;
//	}
//
//	CWseMutexGuard lck(m_lock);
//
//	if(NULL == m_pDraw)
//	{
//		WSE_ERROR_TRACE("CWseVideoRenderer::VADeliverImage() ,NULL == m_pDraw!!!");
//		return WSE_E_FAIL;
//	}
//
//	VA_TYPE vatype = m_pDraw->GetSupportedVAType();
//
//	switch(pContent->vatype)
//	{
//	case VA_Unsupported:
//		WSE_ERROR_TRACE("CWseVideoRenderer::VADeliverImage() ,VA_Unsupported");
//		return WSE_E_FAIL;
//	case VA_D3D9Ex_ShareHandle:
//		{
//			if(VAT_D3D9Ex_ShareHandle_OffscreenPlainSurfaceEx != vatype)
//			{
//				WSE_ERROR_TRACE("CWseVideoRenderer::VADeliverImage() type unmatch! pContent->vatype = "<<pContent->vatype<<",vatype = "<<vatype);
//				return WSE_E_FAIL;
//			}
//
//			VAInfo_D3D9Ex_ShareHandle * pinfo = (VAInfo_D3D9Ex_ShareHandle *)pContent->pVAInfo;
//
//			RECT rcAll;
//			SetRect(&rcAll,0,0,pinfo->uiWidthSH,pinfo->uiHeightSH);
//			RECT rcSource;
//			SetRect(&rcSource,pinfo->lImageLeft,pinfo->lImageTop,pinfo->lImageRight,pinfo->lImageBottom);
//			IntersectRect(&rcSource,&rcSource,&rcAll);
//			if(IsRectEmpty(&rcSource))
//			{
//				WSE_ERROR_TRACE("CWseVideoRenderer::VADeliverImage() invalid argument! pinfo->uiWidthSH = "<<pinfo->uiWidthSH<<",pinfo->uiHeightSH = "<<pinfo->uiHeightSH<<",pinfo->lImageLeft = "<<pinfo->lImageLeft<<",pinfo->lImageTop = "<<pinfo->lImageTop<<",pinfo->lImageRight = "<<pinfo->lImageRight<<",pinfo->lImageBottom = "<<pinfo->lImageBottom);
//				return WSE_E_INVALIDARG;
//			}
//			if(0 == pinfo->formatSH)// D3DFMT_UNKNOWN
//			{
//				WSE_ERROR_TRACE("CWseVideoRenderer::VADeliverImage() invalid argument! pinfo->formatSH = D3DFMT_UNKNOWN");
//				return WSE_E_INVALIDARG;
//			}
//
//			m_iht = IHT_Object;
//			memset(&m_format,0,sizeof(m_format));
//			m_format.width = rcSource.right - rcSource.left;
//			m_format.height = rcSource.bottom - rcSource.top;
//			m_format.video_type = WseUnknown;
//			m_vtFinalType = WseUnknown;
//
//			// clear globle hold
//			if(m_pSample)
//			{
//				m_pSample->Release();
//				m_pSample = NULL;
//			}
//			m_mem.Free();
//			m_memTmp.Free();
//
//			VAS_D3D9Ex_ShareHandle_OffscreenPlainSurfaceEx vas;
//			vas.hSharedHandle = pinfo->hSharedHandle;
//			vas.uiWidthSH = pinfo->uiWidthSH;
//			vas.uiHeightSH = pinfo->uiHeightSH;
//			vas.formatSH = pinfo->formatSH;
//			vas.poolSH = pinfo->poolSH;
//			if(!m_pDraw->VADraw(VAT_D3D9Ex_ShareHandle_OffscreenPlainSurfaceEx,(void*)&vas,rcSource,m_rcTarget))
//			{
//				WSE_ERROR_TRACE("CWseVideoRenderer::VADeliverImage() m_pDraw->VADraw failed!");
//				return WSE_E_FAIL;
//			}
//		}
//		return WSE_S_OK;
//	}
//	WSE_ERROR_TRACE("CWseVideoRenderer::VADeliverImage() invalid argument. type unknown! pContent->vatype = "<<pContent->vatype);
//	return WSE_E_INVALIDARG;
//}

WSERESULT CWseVideoRenderer::DeliverImage(IWseVideoSample* pSample)
{
	WSERESULT wseRet = WSE_E_FAIL;
	__try
	{
		wseRet = DoDeliverImage(pSample);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		wseRet = WSE_E_FAIL;
		//WSE_DEBUG_ERROR("Exception raised in CWseVideoRenderer::DeliverImage");
	}
	return wseRet;
}

WSERESULT CWseVideoRenderer::DoDeliverImage(IWseVideoSample* pSample)
{
	// check parameters
	if(NULL == pSample)
	{
		//WSE_ERROR_TRACE("CWseVideoRenderer::DeliverImage() invalid argument. pSample is empty!");
		return WSE_E_INVALIDARG;
	}
	
	WseVideoFormat format;
	unsigned char *pData;
	unsigned long iLen;

	long lrt = WSE_S_OK;
	lrt = pSample->GetVideoFormat(&format);
	if(WSE_S_OK != lrt)
	{
		//WSE_ERROR_TRACE("CWseVideoRenderer::DeliverImage() pSample->GetVideoFormat() failed. lrt = "<<lrt);
		return WSE_E_INVALIDARG;
	}
	lrt = pSample->GetDataPointer(&pData);
	if(WSE_S_OK != lrt)
	{
		//WSE_ERROR_TRACE("CWseVideoRenderer::DeliverImage() pSample->GetDataPointer() failed. lrt = "<<lrt);
		return WSE_E_INVALIDARG;
	}
	lrt = pSample->GetDataLength(&iLen);
	if(WSE_S_OK != lrt)
	{
		//WSE_ERROR_TRACE("CWseVideoRenderer::DeliverImage() pSample->GetDataLength() failed. lrt = "<<lrt);
		return WSE_E_INVALIDARG;
	}

	if(NULL == pData ||
		0 == iLen)
	{
		//WSE_ERROR_TRACE("CWseVideoRenderer::DeliverImage() invalid argument. pData = "<<pData<<",iLen = "<<iLen);
		return WSE_E_INVALIDARG;
	}
	if(0 == format.width ||
		0 == format.height)
	{
		//WSE_ERROR_TRACE("CWseVideoRenderer::DeliverImage() invalid argument. format.width = "<<format.width<<",format.width = "<<format.height);
		return WSE_E_INVALIDARG;
	}
	switch(format.video_type)
	{
	case WseBGR24:
	case WseBGR24Flip:
		break;
	case WseI420:
	case WseYV12:
	case WseYUY2:
		if(format.width % 2 || format.height % 2)
		{
			//WSE_ERROR_TRACE("CWseVideoRenderer::DeliverImage() invalid argument. the size of yuv data is not normally. format.width = "<<format.width<<",format.height = "<<format.height);
			return WSE_E_INVALIDARG;
		}
		break;
	case WseRGB24:
	case WseRGB24Flip:
		//WSE_ERROR_TRACE("CWseVideoRenderer::DeliverImage() invalid argument. WseRGB24 is not implemented.");
		return WSE_E_NOTIMPL;
	default:
		//WSE_ERROR_TRACE("CWseVideoRenderer::DeliverImage() invalid argument. unknown videotype "<<format.video_type);
		return WSE_E_INVALIDARG;
	}

	CWseMutexGuard lck(m_lock);
	// release old sample
	if(m_pSample)
	{
		//m_pSample->Release();
		m_pSample = NULL;
	}
	m_pSample = pSample;
	//m_pSample->AddRef();
	m_format = format;
	m_iht = IHT_Globle;

	if(FALSE == m_bUpdateOnlyViaReDraw)
	{
		 return DoDraw(FALSE);
	}
	else
	{
		if(m_pDraw)
			m_pDraw->DiscardStream();
		return WSE_S_OK;
	}
}

WSERESULT CWseVideoRenderer::AddRenderPicture(unsigned int uiID,PMMD_BMP_RENDER_STRUCT pPicData)
{
	// check parameters
	if(NULL == pPicData)
	{
		//WSE_ERROR_TRACE("CWseVideoRenderer::AddRenderPicture() invalid argument. uiID = "<<uiID<<",pPicData = "<<pPicData);
		return WSE_E_INVALIDARG;
	}
	if(NULL == pPicData->bitMap)
	{
		//WSE_ERROR_TRACE("CWseVideoRenderer::AddRenderPicture() invalid argument. pPicData->bitMap is empty.");
		return WSE_E_INVALIDARG;
	}

	CWseMutexGuard lck(m_lock);
	ID2BMPMap::iterator itfind = m_mapBitmaps.find(uiID);
	if(itfind != m_mapBitmaps.end())
		itfind->second = *pPicData;
	else
		m_mapBitmaps[uiID] = *pPicData;

	if(m_pDraw)
	{
		BMP_DESC bd;
		bd.bitmap = pPicData->bitMap;
		bd.bmptype = BT_HBITMAP;
		bd.clrkey = 0xFFFFFFFF;//RGB(255,255,255);//
		bd.fleft = pPicData->fleft;
		bd.ftop = pPicData->ftop;
		bd.fright = pPicData->fright;
		bd.fbottom = pPicData->fbottom;
		bd.dwMask = 0;
		if(pPicData->lMask & BMP_RENDER_MASK_REVERSE_ORIGIN_LEFT)
			bd.dwMask |= BMP_DESC_MASK_REVERSE_ORIGIN_LEFT;
		if(pPicData->lMask & BMP_RENDER_MASK_REVERSE_ORIGIN_RIGHT)
			bd.dwMask |= BMP_DESC_MASK_REVERSE_ORIGIN_RIGHT;
		if(pPicData->lMask & BMP_RENDER_MASK_REVERSE_ORIGIN_TOP)
			bd.dwMask |= BMP_DESC_MASK_REVERSE_ORIGIN_TOP;
		if(pPicData->lMask & BMP_RENDER_MASK_REVERSE_ORIGIN_BOTTOM)
			bd.dwMask |= BMP_DESC_MASK_REVERSE_ORIGIN_BOTTOM;
		m_pDraw->AddDrawBitmap(uiID,&bd);
	}
	return WSE_S_OK;
}
WSERESULT CWseVideoRenderer::RemovePicture(unsigned int uiID)
{
	CWseMutexGuard lck(m_lock);
	ID2BMPMap::iterator itfind = m_mapBitmaps.find(uiID);
	if(itfind != m_mapBitmaps.end())
	{
		m_mapBitmaps.erase(itfind);
		if(m_pDraw)
			m_pDraw->RemoveDrawBitmap(uiID);
	}
	return WSE_S_OK;
}
WSERESULT CWseVideoRenderer::GetRenderPictureDesc(unsigned int uiID,PMMD_BMP_RENDER_STRUCT pPicData)
{
	// check parameters
	if(NULL == pPicData)
	{
		//WSE_ERROR_TRACE("CWseVideoRenderer::GetRenderPictureDesc() invalid argument. uiID = "<<uiID<<",pPicData = "<<pPicData);
		return WSE_E_INVALIDARG;
	}

	CWseMutexGuard lck(m_lock);
	ID2BMPMap::iterator itfind = m_mapBitmaps.find(uiID);
	if(itfind != m_mapBitmaps.end())
	{
		*pPicData = itfind->second;
		return WSE_S_OK;
	}
	return WSE_E_FAIL;
}
WSERESULT CWseVideoRenderer::ReDraw()
{
	WSERESULT wseRet = WSE_E_FAIL;
	__try
	{
		wseRet = DoDraw(TRUE);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		wseRet = WSE_E_FAIL;
		//WSE_DEBUG_ERROR("Exception raised in CWseVideoRenderer::DoDraw()");
	}

	return wseRet;
	
}
WSERESULT CWseVideoRenderer::LockDraw()
{
	m_lock.Lock();
	return WSE_S_OK;
}
WSERESULT CWseVideoRenderer::UnlockDraw()
{
	m_lock.UnLock();
	return WSE_S_OK;
}
void CWseVideoRenderer::InitDrawContext()
{
	if(m_pDraw)
		return;

	m_dwRDCCount = GetTickCount();

	switch(m_nDrawLevel)
	{
	case kD3DDraw:
		m_nCurDrawLevel = kD3DDraw;
		m_pDraw = CreateDirect3D(m_hWnd,this);
		if(m_pDraw)
			break;
	//case kGDIDraw:// gdi always success
	//	m_nCurDrawLevel = kGDIDraw;
	//	m_pDraw = CreateGDIDraw(m_hWnd,this);
	//	if (m_pDraw)
	//		break;
	default:
		break;
	};

	UpdateMiscInfo();
}
BOOL CWseVideoRenderer::UpdateDrawContext(int nDrawLevel)
{
	if(m_nCurDrawLevel == nDrawLevel)
		return TRUE;

	DestroyDrawContext();

	m_dwRDCCount = GetTickCount();
	
	switch(nDrawLevel)
	{
	/*case kGDIDraw:
		m_nCurDrawLevel = kGDIDraw;
		m_pDraw = CreateGDIDraw(m_hWnd,this);
		break;*/
	case kD3DDraw:
		m_nCurDrawLevel = kD3DDraw;
		m_pDraw = CreateDirect3D(m_hWnd,this);
		break;
	default:
		return FALSE;
	}
	m_nCurDrawLevel = nDrawLevel;
	if(m_pDraw)
		UpdateMiscInfo();
	return (m_pDraw != NULL);
}
void CWseVideoRenderer::UpdateMiscInfo()
{
	if(NULL == m_pDraw)
		return;

	{// effect
		m_pDraw->SetImageEffect(m_dwImageEffect);
	}

	{// aspect ratio
		ASPECTRATIOMODE asmode = ARM_NONE;
		if(Wse_RARM_LETTER_BOX == m_aspectratiomode)
			asmode = ARM_LETTER_BOX;
		else if(Wse_RARM_STRETCH == m_aspectratiomode)
			asmode = ARM_STRETCH;
		m_pDraw->SetAspectRatioMode(asmode);
	}
	
	{// bkgnd color
		BK_DESC bkd;
		bkd.bktype = BKT_COLOR;
		bkd.clr = m_clrBK;
		m_pDraw->SetBKColor(bkd);
	}

	{// bitmap
		ID2BMPMap::iterator it = m_mapBitmaps.begin();
		for(;it != m_mapBitmaps.end();++it)
		{
			BMP_DESC bd;
			bd.bitmap = it->second.bitMap;
			bd.bmptype = BT_HBITMAP;
			bd.clrkey = 0xFFFFFFFF;//RGB(255,255,255);
			bd.fleft = it->second.fleft;
			bd.ftop = it->second.ftop;
			bd.fright = it->second.fright;
			bd.fbottom = it->second.fbottom;
			bd.dwMask = 0;
			if(it->second.lMask & BMP_RENDER_MASK_REVERSE_ORIGIN_LEFT)
				bd.dwMask |= BMP_DESC_MASK_REVERSE_ORIGIN_LEFT;
			if(it->second.lMask & BMP_RENDER_MASK_REVERSE_ORIGIN_RIGHT)
				bd.dwMask |= BMP_DESC_MASK_REVERSE_ORIGIN_RIGHT;
			if(it->second.lMask & BMP_RENDER_MASK_REVERSE_ORIGIN_TOP)
				bd.dwMask |= BMP_DESC_MASK_REVERSE_ORIGIN_TOP;
			if(it->second.lMask & BMP_RENDER_MASK_REVERSE_ORIGIN_BOTTOM)
			bd.dwMask |= BMP_DESC_MASK_REVERSE_ORIGIN_BOTTOM;
			m_pDraw->AddDrawBitmap(it->first,&bd);
		}
	}
}
//
//#define INIT_WELSVP_STRUCT_CSC_BGR24VFlip(S,WIDTH,HEIGHT,BUFF)				\
//	memset(&S,0,sizeof(S));													\
//	S.nSizeInBits = sizeof(unsigned char) * 8;								\
//	S.Rect.width      = WIDTH;												\
//	S.Rect.height     = HEIGHT;												\
//	S.eFormat     = (nsWseVP::vVideoFormat)(nsWseVP::vVideoFormat_BGR|nsWseVP::vVideoFormat_VFlip);	\
//	S.nStride[2] = S.nStride[1] = S.nStride[0] = ((WIDTH*24+31)&~31)/8;		\
//	S.pPixel[2] = S.pPixel[1] = S.pPixel[0] = BUFF
//
//#define INIT_WELSVP_STRUCT_CSC_BGR24(S,WIDTH,HEIGHT,BUFF)					\
//	memset(&S,0,sizeof(S));													\
//	S.nSizeInBits = sizeof(unsigned char) * 8;								\
//	S.Rect.width      = WIDTH;												\
//	S.Rect.height     = HEIGHT;												\
//	S.eFormat     = nsWseVP::vVideoFormat_BGR;										\
//	S.nStride[2] = S.nStride[1] = S.nStride[0] = ((WIDTH*24+31)&~31)/8;		\
//	S.pPixel[2] = S.pPixel[1] = S.pPixel[0] = BUFF
//
//#define INIT_WELSVP_STRUCT_CSC_RGB24VFlip(S,WIDTH,HEIGHT,BUFF)				\
//	memset(&S,0,sizeof(S));													\
//	S.nSizeInBits = sizeof(unsigned char) * 8;								\
//	S.Rect.width      = WIDTH;												\
//	S.Rect.height     = HEIGHT;												\
//	S.eFormat     = (nsWseVP::vVideoFormat)(nsWseVP::vVideoFormat_RGB24|nsWseVP::vVideoFormat_VFlip);	\
//	S.nStride[2] = S.nStride[1] = S.nStride[0] = ((WIDTH*24+31)&~31)/8;		\
//	S.pPixel[2] = S.pPixel[1] = S.pPixel[0] = BUFF
//
//#define INIT_WELSVP_STRUCT_CSC_RGB24(S,WIDTH,HEIGHT,BUFF)					\
//	memset(&S,0,sizeof(S));													\
//	S.nSizeInBits = sizeof(unsigned char) * 8;								\
//	S.Rect.width      = WIDTH;												\
//	S.Rect.height     = HEIGHT;												\
//	S.eFormat     = nsWseVP::vVideoFormat_RGB24;										\
//	S.nStride[2] = S.nStride[1] = S.nStride[0] = ((WIDTH*24+31)&~31)/8;		\
//	S.pPixel[2] = S.pPixel[1] = S.pPixel[0] = BUFF
//
//#define INIT_WELSVP_STRUCT_CSC_YUY2(S,WIDTH,HEIGHT,BUFF)					\
//	memset(&S,0,sizeof(S));													\
//	S.nSizeInBits = sizeof(unsigned char) * 8;								\
//	S.Rect.width      = WIDTH;												\
//	S.Rect.height     = HEIGHT;												\
//	S.eFormat     = nsWseVP::vVideoFormat_YUY2;										\
//	S.nStride[2] = S.nStride[1] = S.nStride[0] = ((WIDTH*16+31)&~31)/8;		\
//	S.pPixel[2] = S.pPixel[1] = S.pPixel[0] = BUFF
//
//#define INIT_WELSVP_STRUCT_CSC_I420(S,WIDTH,HEIGHT,BUFF)		\
//	memset(&S,0,sizeof(S));										\
//	S.nSizeInBits = sizeof(unsigned char) * 8;					\
//	S.Rect.width      = WIDTH;									\
//	S.Rect.height     = HEIGHT;									\
//	S.eFormat     = nsWseVP::vVideoFormat_I420;							\
//	S.nStride[0] = WIDTH;										\
//	S.nStride[1] = S.nStride[2] = WIDTH / 2;					\
//	S.pPixel[0] = BUFF;											\
//	S.pPixel[1] = BUFF + WIDTH * HEIGHT;						\
//	S.pPixel[2] = BUFF + WIDTH * HEIGHT * 5 / 4
//
//#define INIT_WELSVP_STRUCT_CSC_YV12(S,WIDTH,HEIGHT,BUFF)		\
//	memset(&S,0,sizeof(S));										\
//	S.nSizeInBits = sizeof(unsigned char) * 8;					\
//	S.Rect.width      = WIDTH;									\
//	S.Rect.height     = HEIGHT;									\
//	S.eFormat     = nsWseVP::vVideoFormat_YV12;							\
//	S.nStride[0] = WIDTH;										\
//	S.nStride[1] = S.nStride[2] = WIDTH / 2;					\
//	S.pPixel[0] = BUFF;											\
//	S.pPixel[1] = BUFF + WIDTH * HEIGHT;						\
//	S.pPixel[2] = BUFF + WIDTH * HEIGHT * 5 / 4
//
////WSERESULT CWseVideoRenderer::DoColorspaceConvertToBGR24VFlip()
////{
////	if(NULL == m_pSample)
////		return WSE_E_FAIL;// impossible
////
////	unsigned char *pData;
////	m_pSample->GetDataPointer(&pData);
////
////	unsigned long width = m_format.width;
////	unsigned long height = m_format.height;
////
////	int rgb24stride = ((width*24+31)&~31)/8;
////	int rgblen = rgb24stride*height;
////	if(!m_mem.Reallocate(rgblen))
////		return WSE_E_OUTOFMEMORY;
////
////	LPBYTE pDst = (LPBYTE)m_mem.GetPointer();
////	nsWseVP::vPixMap Dst;
////	INIT_WELSVP_STRUCT_CSC_BGR24VFlip(Dst,width,height,pDst);
////
////	switch(m_format.video_type)
////	{
////	case WseI420:
////		{
////			nsWseVP::vPixMap Src;
////			INIT_WELSVP_STRUCT_CSC_I420(Src,width,height,pData);
////
////			nsWseVP::vResult ret = m_pWelsVP->Process(nsWseVP::vMethods_ColorSpaceConvert, &Src, &Dst);
////			if(nsWseVP::vRet_Success != ret)
////			{
////				WSE_ERROR_TRACE("CWseVideoRenderer::DoColorspaceConvertToBGR24VFlip() , colorspace process(I420) failed(I420 -> BGR24)! ret = "<<ret);
////				return WSE_E_FAIL;
////			}
////		}
////		break;
////	case WseYV12:
////		{
////			nsWseVP::vPixMap Src;
////			INIT_WELSVP_STRUCT_CSC_YV12(Src,width,height,pData);
////
////			nsWseVP::vResult ret = m_pWelsVP->Process(nsWseVP::vMethods_ColorSpaceConvert, &Src, &Dst);
////			if(nsWseVP::vRet_Success != ret)
////			{
////				WSE_ERROR_TRACE("CWseVideoRenderer::DoColorspaceConvertToBGR24VFlip() , colorspace process(YV12) failed(YV12 -> BGR24)! ret = "<<ret);
////				return WSE_E_FAIL;
////			}
////		}
////		break;
////	case WseYUY2:
////		{
////			nsWseVP::vPixMap Src;
////			INIT_WELSVP_STRUCT_CSC_YUY2(Src,width,height,pData);
////
////			int i420len = width*height*3/2;
////			if(!m_memTmp.Reallocate(i420len))
////				return WSE_E_OUTOFMEMORY;
////			LPBYTE pTmp = (LPBYTE)m_memTmp.GetPointer();
////			nsWseVP::vPixMap Tmp;
////			INIT_WELSVP_STRUCT_CSC_I420(Tmp,width,height,pTmp);
////
////			nsWseVP::vResult ret = m_pWelsVP->Process(nsWseVP::vMethods_ColorSpaceConvert, &Src, &Tmp);
////			if(nsWseVP::vRet_Success != ret)
////			{
////				WSE_ERROR_TRACE("CWseVideoRenderer::DoColorspaceConvertToBGR24VFlip() , colorspace process(YUY2) failed(YUY2 -> I420)! ret = "<<ret);
////				return WSE_E_FAIL;
////			}
////
////			ret = m_pWelsVP->Process(nsWseVP::vMethods_ColorSpaceConvert, &Tmp, &Dst);
////			if(nsWseVP::vRet_Success != ret)
////			{
////				WSE_ERROR_TRACE("CWseVideoRenderer::DoColorspaceConvertToBGR24VFlip() , colorspace process(YUY2) failed(I420 -> BGR24)! ret = "<<ret);
////				return WSE_E_FAIL;
////			}
////		}
////		break;
////	}
////
////	m_format.video_type = WseBGR24Flip;
////	
////	// release sample,since we hold data on m_mem
////	if(m_pSample)
////	{
////		m_pSample->Release();
////		m_pSample = NULL;
////	}
////	return WSE_S_OK;
////}

WSERESULT CWseVideoRenderer::DoDraw(BOOL bRedraw)
{
	CWseMutexGuard lck(m_lock);

	TrytoRecoverDrawContext();

	if(NULL == m_pDraw)
	{
		//WSE_ERROR_TRACE("CWseVideoRenderer::DoDraw() ,NULL == m_pDraw!!!");
		return WSE_E_FAIL;
	}
	
	RECT rcTarget = m_rcTarget;
	if (Wse_RARM_ORIGINAL == m_aspectratiomode)
		SetRect(&rcTarget,0,0,m_format.width,m_format.height);
//	InflateRect(&rcTarget,-100,-100);
//	rcTarget.left += 100;
//	Modified by Xuwei 05/22/2012 we need to get DC first
	
	if(IHT_None == m_iht)
	{// draw bk...
_DRAWBKGND:
		while(1)
		{
			//WSE_INFO_TRACE_THIS("CWseVideoRenderer::DoDraw background m_pDraw = " << m_pDraw);
			if(m_pDraw->DrawBkgnd(NULL,rcTarget))
				break;
			
			while(1)
			{
				if(UpdateDrawContext(m_nCurDrawLevel-1))
					break;
			}
		}
	}
	else
	{// draw stream
		RECT rcSource;
		SetRect(&rcSource,0,0,m_format.width,m_format.height);
		
		while(1)
		{
			long lrt;
			if(TRUE == bRedraw)
			{// not need provide image data
				//WSE_INFO_TRACE_THIS("CWseVideoRenderer::DoDraw last frame m_pDraw = " << m_pDraw);
				lrt = m_pDraw->Draw(NULL,NULL,NULL,rcSource,rcTarget);
				//WSE_INFO_TRACE_THIS("CWseVideoRenderer::DoDraw last frame lrt = " << lrt);
				if(CDraw_Success == lrt)
					break;
				
				if(IHT_Object == m_iht)
					goto _DRAWBKGND;// the globle does not hold the image data,the only way is draw bkgnd

				bRedraw = FALSE;// flag change, need provide data

				if(CDraw_NeedData != lrt)
					goto _DOWNGRADE;// failed, downgrade draw

				// CDraw_NeedData, go normal route
			}

			BITMAPINFO bi;
			memset(&bi, 0, sizeof(BITMAPINFO));
			bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bi.bmiHeader.biHeight = m_format.height;
			bi.bmiHeader.biWidth = m_format.width;
			bi.bmiHeader.biPlanes = 1;

			switch(m_format.video_type)
			{
			case WseBGR24:
				bi.bmiHeader.biHeight = -(long)m_format.height;
			case WseBGR24Flip:
				bi.bmiHeader.biBitCount = 24;
				bi.bmiHeader.biCompression = BI_RGB;
				break;
			case WseI420:
				bi.bmiHeader.biBitCount = 12;
				bi.bmiHeader.biCompression = MAKEFOURCC('I','4','2','0');
				break;
			case WseYV12:
				bi.bmiHeader.biBitCount = 12;
				bi.bmiHeader.biCompression = MAKEFOURCC('Y','V','1','2');
				break;
			case WseYUY2:
				bi.bmiHeader.biBitCount = 16;
				bi.bmiHeader.biCompression = MAKEFOURCC('Y','U','Y','2');
				break;
			}
			LPBYTE pData;
			if(m_pSample)
			{
				m_pSample->GetDataLength(&bi.bmiHeader.biSizeImage);
				m_pSample->GetDataPointer(&pData);
			}
			else
			{
				bi.bmiHeader.biSizeImage = m_mem.GetSize();
				pData = (LPBYTE)m_mem.GetPointer();
			}
			lrt = m_pDraw->Draw(pData,(LPBITMAPINFOHEADER)&bi,NULL,rcSource,rcTarget);
			if(CDraw_Success == lrt)
			{
				m_vtFinalType = m_format.video_type;
				break;
			}
			if(CDraw_UnsupportedFormat == lrt)
			{// meet unsupportedformat value on some case
				//long rt = DoColorspaceConvertToBGR24VFlip();
				/*if(WSE_S_OK != rt)
					return rt;*/
				continue;// retry
			}
_DOWNGRADE:
			while(1)
			{
				if(UpdateDrawContext(m_nCurDrawLevel-1))
					break;
			}
		}
	}
	
//	Modified by Xuwei 05/22/2012 we need to get DC first
	return WSE_S_OK;
}
WSERESULT CWseVideoRenderer::OnDisplayChange()
{
	CWseMutexGuard lck(m_lock);
	if(!IsWindowThread())
	{// want windows thread!
		//WSE_ERROR_TRACE("CWseVideoRenderer::OnDisplayChange, !IsWindowThread(),OnDisplayChange() expect be done using the same thread as the window!");
	}
	DestroyDrawContext();
	InitDrawContext();
	return WSE_S_OK;
}
WSERESULT CWseVideoRenderer::OnWindowPositionChange()
{

	CWseMutexGuard lck(m_lock);
	if(!IsWindowThread())
	{// want windows thread!
		//WSE_ERROR_TRACE("CWseVideoRenderer::OnWindowPositionChange, !IsWindowThread(),OnWindowPositionChange() expect be done using the same thread as the window!");
	}

	//WSE_INFO_TRACE_THIS("CWseVideoRenderer::OnWindowPositionChange before  m_rcTarget = [" << m_rcTarget.left << ", "<< m_rcTarget.top 
	//	<<", "<< m_rcTarget.right <<", "<< m_rcTarget.bottom<<"]");
	//GetClientRect(m_hWnd,&m_rcTarget);// update m_rcTarget

	while(1)
	{
		/*WSE_INFO_TRACE_THIS("CWseVideoRenderer::OnWindowPositionChange after m_rcTarget = [" << m_rcTarget.left << ", "
			<< m_rcTarget.top <<", "<< m_rcTarget.right <<", "<< m_rcTarget.bottom<<"]");*/
		if(m_pDraw->UpdateDrawTargetRectangle(m_rcTarget))
		{
			ReDraw();
			break;
		}
		while(1)
		{
			if(UpdateDrawContext(m_nCurDrawLevel-1))
				break;
		}
	}

	return WSE_S_OK;
}
WSERESULT CWseVideoRenderer::SetRendererMode(WseRendererMode mode)
{
	//WSE_INFO_TRACE("CWseVideoRenderer::SetRendererMode(), mode = "<<mode);
	if(Wse_RM_D3D != mode && Wse_RM_GDI != mode )
	{
		//WSE_ERROR_TRACE("CWseVideoRenderer::SetRendererMode() invalid argument. mode = "<<mode);
		return WSE_E_INVALIDARG;
	}

	CWseMutexGuard lck(m_lock);
	if(Wse_RM_D3D == mode)
		m_nDrawLevel = kD3DDraw;
	/*else if(Wse_RM_GDI == mode)
		m_nDrawLevel = kGDIDraw;*/

	if(m_nCurDrawLevel == m_nDrawLevel)
		return WSE_S_OK;

	switch(mode)
	{
	case Wse_RM_D3D:
		if(UpdateDrawContext(kD3DDraw))
			break;
	//case Wse_RM_GDI:
	//	if(UpdateDrawContext(kGDIDraw))
	//		break;
	default:
		return WSE_E_FAIL;
	}
	return WSE_S_OK;
}
WSERESULT CWseVideoRenderer::GetRendererMode(WseRendererMode *pmode)
{
	if(NULL == pmode)
	{
		//WSE_ERROR_TRACE("CWseVideoRenderer::GetRendererMode() invalid argument. pmode is empty.");
		return WSE_E_INVALIDARG;
	}
	switch(m_nCurDrawLevel)
	{
	case kD3DDraw:
		*pmode = Wse_RM_D3D;
		break;
	/*case kGDIDraw:
		*pmode = Wse_RM_GDI;
		break;
	case kD2DDraw:
		*pmode = WSE_RM_D2D;
		break;*/
	}
	return WSE_S_OK;
}
WSERESULT CWseVideoRenderer::SetBKColor(void* color)
{
	//WSE_INFO_TRACE("CWseVideoRenderer::SetBKColor(), color = "<<color);
	CWseMutexGuard lck(m_lock);
	m_clrBK = (COLORREF)color;
	if(m_pDraw)
	{
		BK_DESC bkd;
		bkd.bktype = BKT_COLOR;
		bkd.clr = m_clrBK;
		m_pDraw->SetBKColor(bkd);
	}
// 	BK_DESC bkd;
// 	bkd.bktype = BKT_BITMAP;
// 	bkd.bitmapdesc.bmptype = BT_HBITMAP;
// 	bkd.bitmapdesc.bitmap = color;
// 	m_pDraw->SetBKColor(bkd);
	return WSE_S_OK;
}
WSERESULT CWseVideoRenderer::ClearStream()
{
	//WSE_INFO_TRACE_THIS("CWseVideoRenderer::ClearStream()");
	CWseMutexGuard lck(m_lock);
	m_iht = IHT_None;
	if(m_pSample)
	{
		//m_pSample->Release();
		m_pSample = NULL;
	}
	m_mem.Free();
	m_memTmp.Free();
	memset(&m_format,0,sizeof(m_format));
	m_vtFinalType = WseUnknown;
	if(m_pDraw)
		m_pDraw->DiscardStream();
	return WSE_S_OK;
}
WSERESULT CWseVideoRenderer::SetAspectRatioMode(WseRendererAspectRatioMode mode)
{
	//WSE_INFO_TRACE_THIS("CWseVideoRenderer::SetAspectRatioMode mode = "<< mode);
	if(Wse_RARM_NONE != mode && Wse_RARM_LETTER_BOX != mode && Wse_RARM_STRETCH != mode && Wse_RARM_ORIGINAL != mode)
	{
		//WSE_ERROR_TRACE("CWseVideoRenderer::SetAspectRatioMode() invalid argument. mode = "<<mode);
		return WSE_E_INVALIDARG;
	}

	CWseMutexGuard lck(m_lock);
	m_aspectratiomode = mode;
	if(m_pDraw)
	{
		ASPECTRATIOMODE asmode = ARM_NONE;
		if(Wse_RARM_LETTER_BOX == mode)
			asmode = ARM_LETTER_BOX;
		else if( Wse_RARM_STRETCH == mode)
			asmode = ARM_STRETCH;
		m_pDraw->SetAspectRatioMode(asmode);
	}
	return WSE_S_OK;
}
WSERESULT CWseVideoRenderer::GetAspectRatioMode(WseRendererAspectRatioMode *pmode)
{
	if(NULL == pmode)
		return WSE_E_INVALIDARG;
	*pmode = m_aspectratiomode;
	return WSE_S_OK;
}
//WSERESULT CWseVideoRenderer::SetEffect(unsigned int type,void* detail)
//{
//	//WSE_INFO_TRACE("CWseVideoRenderer::SetEffect(), type = "<<type<<",detail = "<<detail);
//	if(EFFECT_RENDER_MIRRORUPDOWN != type && EFFECT_RENDER_MIRRORLEFTRIGHT != type)
//	{
//		WSE_ERROR_TRACE("CWseVideoRenderer::SetEffect() invalid argument. type = "<<type);
//		return WSE_E_INVALIDARG;
//	}
//
//	CWseMutexGuard lck(m_lock);
//	switch(type)
//	{
//	case EFFECT_RENDER_MIRRORUPDOWN:
//		if((unsigned long)detail == 0)
//			m_dwImageEffect &= ~IMAGEEFFECT_MIRRORUPDOWN;
//		else
//			m_dwImageEffect |= IMAGEEFFECT_MIRRORUPDOWN;
//		break;
//	case EFFECT_RENDER_MIRRORLEFTRIGHT:
//		if((unsigned long)detail == 0)
//			m_dwImageEffect &= ~IMAGEEFFECT_MIRRORLEFTRIGHT;
//		else
//			m_dwImageEffect |= IMAGEEFFECT_MIRRORLEFTRIGHT;
//		break;
//	}
//	if(m_pDraw)
//		m_pDraw->SetImageEffect(m_dwImageEffect);
//	return WSE_S_OK;
//}
//WSERESULT CWseVideoRenderer::ClearEffect()
//{
//	//WSE_INFO_TRACE("CWseVideoRenderer::ClearEffect()");
//	//CWseMutexGuard lck(m_lock);
//	m_dwImageEffect = 0;
//	if(m_pDraw)
//		m_pDraw->SetImageEffect(m_dwImageEffect);
//	return WSE_S_OK;
//}
void CWseVideoRenderer::TrytoRecoverDrawContext()
{
	if(m_nCurDrawLevel == m_nDrawLevel)
		return;
#define RDC_INTERVAL	30000	// 30s
	DWORD dwNow = GetTickCount();
	DWORD dwInterval = dwNow - m_dwRDCCount;
	if(dwInterval < RDC_INTERVAL)
		return;
	
	m_dwRDCCount = dwNow;
	CDraw* pDraw = NULL;
	switch(m_nDrawLevel)
	{
	/*case kD2DDraw:
		pDraw = CreateDirect2D(m_hWnd,this);
		break;
	case kGDIDraw:
		pDraw = CreateGDIDraw(m_hWnd,this);
		break;*/
	case kD3DDraw:
		pDraw = CreateDirect3D(m_hWnd,this);
		break;
	default:
		return;
	}

	if(pDraw)
	{// recovered
		DestroyDrawContext();

		m_pDraw = pDraw;

		m_nCurDrawLevel = m_nDrawLevel;
		
		UpdateMiscInfo();
	}
}
WSERESULT CWseVideoRenderer::GetFinalRenderPixelFormat(WseVideoType *ptype)
{
	if(NULL == ptype)
		return WSE_E_INVALIDARG;
	*ptype = m_vtFinalType;
	return WSE_S_OK;
}
WSERESULT CWseVideoRenderer::UpdateOnlyViaReDraw(bool bYes)
{
	//WSE_INFO_TRACE_THIS("CWseVideoRenderer::UpdateOnlyViaReDraw(),bYes = "<<bYes);
	CWseMutexGuard lck(m_lock);
	m_bUpdateOnlyViaReDraw = bYes?TRUE:FALSE;
	return WSE_S_OK;
}
//LRESULT CWseVideoRenderer::HookProc(int code,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
//{
//	if(code == HC_ACTION)
//	{
//		LPMSG pMsg = (LPMSG)lParam;
//		if(m_hWnd == pMsg->hwnd
//			&& m_uMsgCustom == pMsg->message)
//		{
//			if(wParam == PM_REMOVE)
//			{
//				CWseMutexGuard lck(m_lock);
//				switch(pMsg->wParam)
//				{
//				case msg_WTA:
//					if(TRUE == m_bWindowThreadAction)
//					{
//						m_bWindowThreadAction = FALSE;
//						if(m_pDraw)
//							m_pDraw->WindowThreadAction();
//					}
//					break;
//				case msg_WTD:
//					{
//						ClearWTDList();
//					}
//					break;
//				}
//			}
//			bHandled = TRUE;
//			return 0;
//		}
//	}
//	return 0;
//}
void CWseVideoRenderer::TriggerWindowThreadAction()
{// called by draw object
	m_bWindowThreadAction = TRUE;
	PostMessage(m_hWnd,m_uMsgCustom,msg_WTA,0);
}
BOOL CWseVideoRenderer::IsWindowThread()
{
	return (GetCurrentThreadId() == m_dwTIDWindow);
}
void CWseVideoRenderer::DestroyDrawContext()
{
	if(m_pDraw)
	{
		if(m_pDraw->IsNeedWindowThreadDestroy() && !IsWindowThread())
		{
			AddToWTDList(m_pDraw);
			PostMessage(m_hWnd,m_uMsgCustom,msg_WTD,0);
		}
		else
			DestoryDraw(m_pDraw);
	}
	m_pDraw = NULL;
	m_bWindowThreadAction = FALSE;// reset WindowThreadAction flag belong to Draw object
}
void CWseVideoRenderer::AddToWTDList(CDraw* pDraw)
{
	m_listWindowThreadDestroy.push_back(m_pDraw);
}
void CWseVideoRenderer::ClearWTDList()
{
	DrawObjectList::iterator it = m_listWindowThreadDestroy.begin();
	for(;it != m_listWindowThreadDestroy.end();++it)
		DestoryDraw(*it);
	m_listWindowThreadDestroy.clear();
}
