


#include "atlbase.h"
//#include "WseDebug.h"
#include "../common/Util.h"
//#define COMPILE_MULTIMON_STUBS
#include "multimon.h"
#include "Direct3D9WrapperFactory.h"
#include "IDirect3D9Wrapper.h"
#include "IDirect3D9DeviceWrapper.h"
#include "Direct3D9Draw.h"
inline void CalcTextureSize(const D3DCAPS9 &d3d9DeviceCapsRef,const UINT& ulWidth,const UINT& ulHeight,UINT& ulWidthFitted,UINT& ulHeightFitted)
{
	ulWidthFitted = ulWidth;
	ulHeightFitted = ulHeight;
//	if(d3d9DeviceCapsRef.TextureCaps & D3DPTEXTURECAPS_POW2)
	{
		for(ulWidthFitted=1;ulWidth>ulWidthFitted;ulWidthFitted<<=1);
		for(ulHeightFitted=1;ulHeight>ulHeightFitted;ulHeightFitted<<=1);
	}
	if(d3d9DeviceCapsRef.TextureCaps & D3DPTEXTURECAPS_SQUAREONLY)
	{
		if(ulHeightFitted > ulWidthFitted)
			ulWidthFitted = ulHeightFitted;
		if(ulWidthFitted > ulHeightFitted)
			ulHeightFitted = ulWidthFitted;
	}
}
//////////////////////////////////////////////////////////////////////////
// CDirect3D9ExDraw
//////////////////////////////////////////////////////////////////////////
CDirect3D9ExDraw::CDirect3D9ExDraw()
{
	//WSE_INFO_TRACE("CDirect3D9ExDraw::CDirect3D9ExDraw(), this = "<<this);
	m_pfnDirect3DCreate9Ex = NULL;
	m_hDll = LoadLibrary(_T("d3d9.dll"));
    if(m_hDll)
        m_pfnDirect3DCreate9Ex = (DIRECT3DCREATE9EX)GetProcAddress(m_hDll,"Direct3DCreate9Ex");

	m_pD3D9WrapperFactory = new Direct3D9WrapperFactory;

	ZeroMemory(&m_d3dpp,sizeof(m_d3dpp));
	ZeroMemory(&m_d3d9ExDeviceCaps,sizeof(m_d3d9ExDeviceCaps));
	//m_lpD3D9Ex = NULL;
	//m_lpD3D9ExDevice = NULL;
	m_pD3D9Wrapper = NULL;
	m_pD3D9DeviceWrapper = NULL;
	m_lpD3D9RawSurface = NULL;
	m_lpD3D9ImageSurface = NULL;
	m_lpD3D9ImageTexture = NULL;
	m_hWnd = NULL;

	m_hmon = 0;

	ZeroMemory(&m_d3dsdImage,sizeof(m_d3dsdImage));
	ZeroMemory(&m_d3dsdRaw,sizeof(m_d3dsdRaw));
	
	m_pHelper = NULL;
	m_dwTIDWindow = 0;

	m_bRawExpired = FALSE;
	m_bImageExpired = FALSE;

	m_stateCode = StateCode_Invalid;
	m_bNeedCheckDevice = FALSE;

	SetRect(&m_rcTarget,0,0,1,1);

	m_hSharedHandle = NULL;
	m_lpD3D9SharedSurface = NULL;
	m_mapBlackList.clear();
	ZeroMemory(&m_d3dsdShared,sizeof(m_d3dsdShared));
}
CDirect3D9ExDraw::~CDirect3D9ExDraw()
{
	//WSE_INFO_TRACE("CDirect3D9ExDraw::~CDirect3D9ExDraw(), this = "<<this);
	UnInit();

	if(m_pD3D9WrapperFactory)
		delete m_pD3D9WrapperFactory;

	if(m_hDll)
	{
		m_pfnDirect3DCreate9Ex = NULL;
		FreeLibrary(m_hDll);
		m_hDll = NULL;
	}
}
BOOL CDirect3D9ExDraw::Init(HWND hWnd,IDrawHelper* pHelper)
{
	m_pHelper = pHelper;

	m_dwTIDWindow = GetWindowThreadProcessId(hWnd,NULL);

	if(!InitD3D9Ex(hWnd))
		return FALSE;
	
	m_hWnd = hWnd;

	// init m_rcTarget and m_hmon
	GetClientRect(m_hWnd,&m_rcTarget);
	// m_rcTarget never be empty
	if(IsRectEmpty(&m_rcTarget))
		SetRect(&m_rcTarget,0,0,1,1);
	// m_hmon belong to m_rcTarget
	RECT rcVSC = m_rcTarget;
	MapWindowPoints(m_hWnd,NULL,(LPPOINT)&rcVSC,2);
    m_hmon =(HMONITOR)m_hWnd;//MonitorFromRect(&rcVSC,MONITOR_DEFAULTTONEAREST);
	return TRUE;
}
void CDirect3D9ExDraw::UnInit()
{
	UnInitD3D9Ex();

	m_hWnd = NULL;
	m_dwTIDWindow = 0;

	m_pHelper = NULL;

	m_hmon = 0;

	SetRect(&m_rcTarget,0,0,1,1);
}
BOOL CDirect3D9ExDraw::InitD3D9Ex(HWND hWnd)
{
	//WSE_INFO_TRACE("Begin CDirect3D9ExDraw::InitD3D9Ex()");
	/*
	if(NULL == m_pfnDirect3DCreate9Ex)
		return FALSE;

	HRESULT hr = m_pfnDirect3DCreate9Ex(D3D_SDK_VERSION,&m_lpD3D9Ex);
	if(FAILED(hr))
	{
		WSE_WARN_TRACE("CDirect3D9ExDraw::InitD3D9Ex(), Direct3DCreate9Ex failed! errorcode = "<<hr);
		return FALSE;
	}
	if(NULL == m_lpD3D9Ex)
		return FALSE;
	*/

	if(!m_pD3D9WrapperFactory)
		return FALSE;

	if(!m_pD3D9WrapperFactory->CanCreateD3D9())
		return FALSE;

	m_pD3D9Wrapper = m_pD3D9WrapperFactory->CreateDirect3D9Wrapper();
	if(!m_pD3D9Wrapper)
		return FALSE;

	UINT nAdapterCount = m_pD3D9Wrapper->GetAdapterCount();
	for (UINT nAdapterIdx = 0; nAdapterIdx < nAdapterCount; nAdapterIdx++)
	{//some display adapters need 128 alignment for I420, put it into black list to treat specially
		D3DADAPTER_IDENTIFIER9 di;
		if( m_pD3D9Wrapper->GetAdapterIdentifier(nAdapterIdx, 0,&di) == S_OK )
		{
			//WSE_INFO_TRACE_THIS("CDirect3D9ExDraw::InitD3D9Ex adapter info, adapter index = "<< nAdapterIdx
			//	<<", adapter driver = "<< di.Driver<<", adapter description = "<< di.Description
			//	//<<", Driver version = "<<di.DriverVersionHighPart<<", "<<di.DriverVersionLowPart
			//	<<",adapter device = "<<di.DeviceName);

			if(strcmp(di.Description, "Intel(R) Q35 Express Chipset Family") ==0)
			{
				//this specail video card needs to pass 128Bytes width aligned I420 data, 
				//so we convert the data to RGB, thus we do not need to do the alignment
				m_mapBlackList[kNeed128BytesAligned] = 1;
				break;
			}
		}
		else
		{
			m_mapBlackList[kNeed128BytesAligned] = 0;
			//WSE_WARN_TRACE_THIS("CDirect3D9ExDraw::InitD3D9Ex get adapter info, cannot get right adapter info");
		}
		
	}
	m_stateCode = StateCode_Uncreated;

	//WSE_INFO_TRACE("End CDirect3D9ExDraw::InitD3D9Ex()");
	return TRUE;
}

void CDirect3D9ExDraw::UnInitD3D9Ex()
{
	//WSE_INFO_TRACE("Begin CDirect3D9ExDraw::UnInitD3D9Ex()");
	UnInitD3D9DeviceExChain();

	//SAFE_RELEASE(m_lpD3D9Ex);
	if(m_pD3D9Wrapper)
		delete m_pD3D9Wrapper;

	m_stateCode = StateCode_Invalid;
	//WSE_INFO_TRACE("End CDirect3D9ExDraw::UnInitD3D9Ex()");
}

void CDirect3D9ExDraw::UnInitD3D9DeviceExChain()
{
	FreeBKP(&m_bk);

	{
		ID2BPMap::iterator it = m_mapID2BP.begin();
		for(;it!=m_mapID2BP.end();++it)
			FreeBP(&it->second);
	}

	SAFE_RELEASE(m_lpD3D9RawSurface);
	SAFE_RELEASE(m_lpD3D9ImageTexture);
	SAFE_RELEASE(m_lpD3D9ImageSurface);
	//SAFE_RELEASE(m_lpD3D9ExDevice);
	if(m_pD3D9DeviceWrapper)
	{
		delete m_pD3D9DeviceWrapper;
		m_pD3D9DeviceWrapper = NULL;
	}

	ZeroMemory(&m_d3dpp,sizeof(m_d3dpp));
	ZeroMemory(&m_d3d9ExDeviceCaps,sizeof(m_d3d9ExDeviceCaps));

	ZeroMemory(&m_d3dsdImage,sizeof(m_d3dsdImage));
	ZeroMemory(&m_d3dsdRaw,sizeof(m_d3dsdRaw));

	m_bRawExpired = FALSE;
	m_bImageExpired = FALSE;

	m_listFourCC.clear();

	m_hSharedHandle = NULL;
	SAFE_RELEASE(m_lpD3D9SharedSurface);
	ZeroMemory(&m_d3dsdShared,sizeof(m_d3dsdShared));

	m_bNeedCheckDevice = FALSE;
}
void CDirect3D9ExDraw::DiscardStream()
{
	SAFE_RELEASE(m_lpD3D9RawSurface);
	SAFE_RELEASE(m_lpD3D9ImageTexture);
	SAFE_RELEASE(m_lpD3D9ImageSurface);

	ZeroMemory(&m_d3dsdImage,sizeof(m_d3dsdImage));
	ZeroMemory(&m_d3dsdRaw,sizeof(m_d3dsdRaw));

	m_bRawExpired = FALSE;
	m_bImageExpired = FALSE;

	m_hSharedHandle = NULL;
	SAFE_RELEASE(m_lpD3D9SharedSurface);
	ZeroMemory(&m_d3dsdShared,sizeof(m_d3dsdShared));
}
void CDirect3D9ExDraw::UpdateFourCCList(UINT uAdapter,D3DFORMAT TargetFormat)
{
	m_listFourCC.clear();
	
	HRESULT hr;
	static const DWORD dwSupportFourCCs[] = 
	{
		MAKEFOURCC('I','4','2','0')
		,MAKEFOURCC('Y','V','1','2')
		,MAKEFOURCC('Y','U','Y','2')
		,MAKEFOURCC('U','Y','V','Y')
	};

	for(int j = 0;j < sizeof(dwSupportFourCCs)/sizeof(dwSupportFourCCs[0]);j++)
	{
		//hr = m_lpD3D9Ex->CheckDeviceFormat(uAdapter,D3DDEVTYPE_HAL,TargetFormat,0,D3DRTYPE_SURFACE,(D3DFORMAT)dwSupportFourCCs[j]);
		hr = m_pD3D9Wrapper->CheckDeviceFormat(uAdapter, D3DDEVTYPE_HAL, TargetFormat, 0, D3DRTYPE_SURFACE, (D3DFORMAT)dwSupportFourCCs[j]);
		if(D3D_OK == hr)
		{
			const DWORD* pixelformats = NULL;
			int cnt = GetCompatibleFourCC(dwSupportFourCCs[j],(LPDWORD*)&pixelformats);
			for(int i = 0;i < cnt;i++)
			{
				//hr = m_lpD3D9Ex->CheckDeviceFormatConversion(uAdapter,D3DDEVTYPE_HAL,(D3DFORMAT)pixelformats[i],TargetFormat);
				hr = m_pD3D9Wrapper->CheckDeviceFormatConversion(uAdapter, D3DDEVTYPE_HAL, (D3DFORMAT)pixelformats[i],TargetFormat);
				if(D3D_OK == hr)
				{
					FourCCList::iterator it = m_listFourCC.begin();
					for(;it!=m_listFourCC.end();++it)
					{
						if(*it == pixelformats[i])
							break;
					}
					if(it == m_listFourCC.end())
						m_listFourCC.push_back(pixelformats[i]);
				}
			}
		}
	}
}
HRESULT CDirect3D9ExDraw::CreateDevice(HMONITOR hmonWnd,UINT uiWidth,UINT uiHeight)
{
	UINT uAdapter = D3DADAPTER_DEFAULT;
	//UINT uCnt = m_lpD3D9Ex->GetAdapterCount();
	UINT uCnt = m_pD3D9Wrapper->GetAdapterCount();
	for(UINT i=0;i<uCnt;i++)
	{
		//HMONITOR hmon = m_lpD3D9Ex->GetAdapterMonitor(i);
		HMONITOR hmon = m_pD3D9Wrapper->GetAdapterMonitor(i);
		if(hmon == hmonWnd)
		{
			uAdapter = i;
			break;
		}
	}
	
	HRESULT hr;
	D3DDISPLAYMODEEX d3ddm;
	d3ddm.Size = sizeof(d3ddm);
	//hr = m_lpD3D9Ex->GetAdapterDisplayModeEx(uAdapter,&d3ddm,NULL);
	hr = m_pD3D9Wrapper->GetAdapterDisplayModeEx(uAdapter, &d3ddm, NULL);
	//SetDllDirectory(NULL);
	if(FAILED(hr))
	{
		//WSE_ERROR_TRACE("CDirect3D9ExDraw::CreateDevice(), GetAdapterDisplayModeEx failed! errorcode = "<<hr);
		return hr;
	}

	D3DDEVTYPE devtype = D3DDEVTYPE_HAL;
	DWORD BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	BehaviorFlags |= D3DCREATE_NOWINDOWCHANGES | D3DCREATE_FPU_PRESERVE;
	
	ZeroMemory(&m_d3dpp,sizeof(m_d3dpp));
	m_d3dpp.Flags = D3DPRESENTFLAG_VIDEO;//|D3DPRESENTFLAG_DEVICECLIP;
	m_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	m_d3dpp.BackBufferFormat = d3ddm.Format;
	m_d3dpp.BackBufferWidth = uiWidth;
	m_d3dpp.BackBufferHeight = uiHeight;
	m_d3dpp.Windowed = TRUE;
	m_d3dpp.hDeviceWindow = m_hWnd;
	m_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	//hr = m_lpD3D9Ex->CreateDeviceEx(uAdapter,devtype,NULL,BehaviorFlags,&m_d3dpp,NULL,&m_lpD3D9ExDevice);
	hr = m_pD3D9Wrapper->CreateDeviceEx(uAdapter, devtype, NULL, BehaviorFlags, &m_d3dpp, NULL, &m_pD3D9DeviceWrapper);
	if(FAILED(hr))
	{
		//WSE_ERROR_TRACE("CDirect3D9ExDraw::CreateDevice(), CreateDeviceEx failed! errorcode = "<<hr);
		return hr;
	}

	//hr = m_lpD3D9ExDevice->GetDeviceCaps(&m_d3d9ExDeviceCaps);
	hr = m_pD3D9DeviceWrapper->GetDeviceCaps(&m_d3d9ExDeviceCaps);
	if(FAILED(hr))
	{
		//WSE_ERROR_TRACE("CDirect3D9ExDraw::CreateDevice(), GetDeviceCaps failed! errorcode = "<<hr);
		return hr;
	}

	UpdateFourCCList(uAdapter,d3ddm.Format);

	/*
	hr = m_lpD3D9ExDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	hr = m_lpD3D9ExDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
	hr = m_lpD3D9ExDevice->SetRenderState( D3DRS_ZENABLE, FALSE);
	*/
	hr = m_pD3D9DeviceWrapper->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	hr = m_pD3D9DeviceWrapper->SetRenderState( D3DRS_LIGHTING, FALSE );
	hr = m_pD3D9DeviceWrapper->SetRenderState( D3DRS_ZENABLE, FALSE);

	m_hmon = hmonWnd;// update m_hmon
	return hr;
}
HRESULT CDirect3D9ExDraw::Reset(UINT uiWidth,UINT uiHeight)
{
	UINT uAdapter = D3DADAPTER_DEFAULT;
	//UINT uCnt = m_lpD3D9Ex->GetAdapterCount();
	UINT uCnt = m_pD3D9Wrapper->GetAdapterCount();
	for(UINT i=0;i<uCnt;i++)
	{
		//HMONITOR hmon = m_lpD3D9Ex->GetAdapterMonitor(i);
		HMONITOR hmon = m_pD3D9Wrapper->GetAdapterMonitor(i);
		if(hmon == m_hmon)
		{
			uAdapter = i;
			break;
		}
	}
	
	HRESULT hr;
	D3DDISPLAYMODEEX d3ddm;
	d3ddm.Size = sizeof(d3ddm);
	//hr = m_lpD3D9Ex->GetAdapterDisplayModeEx(uAdapter,&d3ddm,NULL);
	hr = m_pD3D9Wrapper->GetAdapterDisplayModeEx(uAdapter, &d3ddm, NULL);
	if(FAILED(hr))
	{
		//WSE_ERROR_TRACE("CDirect3D9ExDraw::Reset(), GetAdapterDisplayModeEx failed! errorcode = "<<hr);
		return hr;
	}

	m_d3dpp.BackBufferFormat = d3ddm.Format;
	m_d3dpp.BackBufferWidth = uiWidth;
	m_d3dpp.BackBufferHeight = uiHeight;
	//hr = m_lpD3D9ExDevice->ResetEx(&m_d3dpp,NULL);
	hr = m_pD3D9DeviceWrapper->ResetEx(&m_d3dpp,NULL);
	if(FAILED(hr))
	{
		//WSE_ERROR_TRACE("CDirect3D9ExDraw::Reset(), ResetEx failed! errorcode = "<<hr);
		return hr;
	}

	UpdateFourCCList(uAdapter,d3ddm.Format);

	return hr;
}
inline BOOL CDirect3D9ExDraw::GetBackBuffer(IDirect3DSurface9 **ppBackBuffer)
{
	/*
	if(!m_lpD3D9ExDevice)
		return FALSE;
	*/
	if(!m_pD3D9DeviceWrapper)
		return FALSE;

	HRESULT hr;
	//hr = m_lpD3D9ExDevice->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,ppBackBuffer);
	hr = m_pD3D9DeviceWrapper->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,ppBackBuffer);
	if(FAILED(hr))
	{
		//WSE_ERROR_TRACE("CDirect3D9ExDraw::GetBackBuffer(), GetBackBuffer failed! errorcode = "<<hr);
		return FALSE;
	}
	return TRUE;
}
inline CDirect3D9ExDraw::CheckingCode CDirect3D9ExDraw::CheckDevice()
{
	if(NULL == m_pD3D9DeviceWrapper)
	{
		m_stateCode = StateCode_Uncreated;
		//WSE_INFO_TRACE("CDirect3D9ExDraw::CheckDevice(), NULL == m_lpD3D9ExDevice.");
		return CheckingCode_StateChange;
	}
	
	HRESULT hr;
	hr = m_pD3D9DeviceWrapper->CheckDeviceState(m_hWnd);
	if(FAILED(hr))
	{
		switch(hr)
		{
		case D3DERR_DEVICELOST:
			{
				// The device has been lost but cannot be reset at this time. Therefore, rendering is not possible.A Direct 3D device object other than the one that returned this code caused the hardware adapter to be reset by the OS. Delete all video memory objects (surfaces, textures, state blocks) and call Reset() to return the device to a default state. If the application continues rendering without a reset, the rendering calls will succeed.
				m_stateCode = StateCode_NeedRecreate;
			//	WSE_WARN_TRACE("CDirect3D9ExDraw::CheckDevice(), CheckDeviceState D3DERR_DEVICELOST.");
				return CheckingCode_StateChange;
			}
		case D3DERR_DEVICEHUNG:
			{
				// The device that returned this code caused the hardware adapter to be reset by the OS. Most applications should destroy the device and quit. Applications that must continue should destroy all video memory objects (surfaces, textures, state blocks etc) and call Reset() to put the device in a default state. If the application then continues rendering in the same way, the device will return to this state.
				// Applies to Direct3D 9Ex only
				m_stateCode = StateCode_NeedRecreate;
			//	WSE_WARN_TRACE("CDirect3D9ExDraw::CheckDevice(), CheckDeviceState D3DERR_DEVICEHUNG.");
				return CheckingCode_StateChange;
			}
		case D3DERR_DEVICEREMOVED:
			{
				// The hardware adapter has been removed. Application must destroy the device, do enumeration of adapters and create another Direct3D device. If application continues rendering without calling Reset, the rendering calls will succeed.
				// Applies to Direct3D 9Ex only.
				m_stateCode = StateCode_Invalid;
			//	WSE_WARN_TRACE("CDirect3D9ExDraw::CheckDevice(), CheckDeviceState D3DERR_DEVICEREMOVED!");
				return CheckingCode_Error;
			}
		}
		///WSE_ERROR_TRACE("CDirect3D9ExDraw::CheckDevice(), CheckDeviceState failed! errorcode = "<<hr);
		return CheckingCode_Error;
	}
	else
	{
		if(hr == S_PRESENT_OCCLUDED)
		{
			// The presentation area is occluded. Occlusion means that the presentation window is minimized or another device entered the fullscreen mode on the same monitor as the presentation window and the presentation window is completely on that monitor. Occlusion will not occur if the client area is covered by another Window.
			// Occluded applications can continue rendering and all calls will succeed, but the occluded presentation window will not be updated. Preferably the application should stop rendering to the presentation window using the device and keep calling CheckDeviceState until S_OK or S_PRESENT_MODE_CHANGED returns.
			//WSE_INFO_TRACE("CDirect3D9ExDraw::CheckDevice(), CheckDeviceState S_PRESENT_OCCLUDED.");
		}
		if(S_PRESENT_MODE_CHANGED == hr)
		{
			// The desktop display mode has been changed. The application can continue rendering, but there might be color conversion/stretching. Pick a back buffer format similar to the current display mode, and call Reset to recreate the swap chains. The device will leave this state after a Reset is called.
			m_stateCode = StateCode_NeedReset;
		//	WSE_INFO_TRACE("CDirect3D9ExDraw::CheckDevice(), CheckDeviceState S_PRESENT_MODE_CHANGED.");
			return CheckingCode_StateChange;
		}
	}
	return CheckingCode_Success;
}
inline CDirect3D9ExDraw::CheckingCode CDirect3D9ExDraw::CheckAndRecoverD3D9ExObjects(HMONITOR hmonWnd,UINT uiWidth,UINT uiHeight)
{
	CDirect3D9ExDraw::CheckingCode cc = CheckingCode_Error;
	do 
	{
		// m_lpD3D9ExDevice uncreate
		if(NULL == m_pD3D9DeviceWrapper)
		{
			m_stateCode = StateCode_Uncreated;
		//	WSE_INFO_TRACE("CDirect3D9ExDraw::CheckAndRecoverD3D9ExObjects(), NULL == m_lpD3D9ExDevice.");
			cc = CheckingCode_StateChange;
			break;
		}
		
		// window move to another monitor
		if(m_hmon != hmonWnd)
		{
			m_stateCode = StateCode_NeedRecreate;
		//	WSE_INFO_TRACE("CDirect3D9ExDraw::CheckAndRecoverD3D9ExObjects(), monitor change! "<<m_hmon<<" -> "<<hmonWnd);
			cc = CheckingCode_StateChange;
			break;
		}
		
		// window size change
		if(m_d3dpp.BackBufferWidth != uiWidth || m_d3dpp.BackBufferHeight != uiHeight)
		{
			if(m_pD3D9Wrapper->IsDirect3D9ExObject())
				m_stateCode = StateCode_NeedReset;
			else
				m_stateCode = StateCode_NeedRecreate;

		//	WSE_INFO_TRACE("CDirect3D9ExDraw::CheckAndRecoverD3D9ExObjects(), window size change! ("<<m_d3dpp.BackBufferWidth<<","<<m_d3dpp.BackBufferHeight<<")->("<<uiWidth<<","<<uiHeight<<")");
			cc = CheckingCode_StateChange;
			break;
		}

		if(m_pD3D9Wrapper->IsAlwaysNeedCheckDeviceState() || 
			(!m_pD3D9Wrapper->IsAlwaysNeedCheckDeviceState() && TRUE == m_bNeedCheckDevice ))
		{
			cc = CheckDevice();
			m_bNeedCheckDevice = FALSE;
		}
		else
			cc = CheckingCode_Success;
	}
	while(0);

	switch(cc)
	{
	case CheckingCode_Success:
		break;
	case CheckingCode_StateChange:
		if(IsWindowThread())
		{
			PrepareD3D9ExObjects(hmonWnd,uiWidth,uiHeight);
			if(StateCode_Active != m_stateCode)
				return CheckingCode_Error;
			
			// recover success
		}
		else
		{
			NotifyD3D9ObjectsNormalization();
			return CheckingCode_StateChange;
		}
		break;
	default:
		return CheckingCode_Error;
	}
	return CheckingCode_Success;
}
inline CDirect3D9ExDraw::CheckingCode CDirect3D9ExDraw::Present(const RECT* prcSrc,const RECT* prcDest)
{
	HRESULT hr;
	hr = m_pD3D9DeviceWrapper->PresentEx(prcSrc,prcDest,NULL,NULL,0);
	if(FAILED(hr))
	{
		switch(hr)
		{
		case D3DERR_DEVICELOST:
			m_stateCode = StateCode_NeedRecreate;
			//WSE_WARN_TRACE("CDirect3D9ExDraw::Present(), PresentEx D3DERR_DEVICELOST.");
			m_bNeedCheckDevice = TRUE;
			return CheckingCode_StateChange;
		case D3DERR_DEVICEHUNG:
			m_stateCode = StateCode_NeedRecreate;
			//WSE_WARN_TRACE("CDirect3D9ExDraw::Present(), PresentEx D3DERR_DEVICEHUNG.");
			m_bNeedCheckDevice = TRUE;
			return CheckingCode_StateChange;
		case D3DERR_DEVICEREMOVED:
			m_stateCode = StateCode_Invalid;
			//WSE_ERROR_TRACE("CDirect3D9ExDraw::Present(), PresentEx D3DERR_DEVICEREMOVED!");
			return CheckingCode_Error;
		}
		//WSE_ERROR_TRACE("CDirect3D9ExDraw::Present(), PresentEx failed! errorcode = "<<hr);
		return CheckingCode_Error;
	}
	else
	{
		switch(hr)
		{
		case S_PRESENT_OCCLUDED:
			//WSE_INFO_TRACE("CDirect3D9ExDraw::Present(), PresentEx S_PRESENT_OCCLUDED.");
		case S_PRESENT_MODE_CHANGED:
			m_stateCode = StateCode_NeedReset;
			//WSE_INFO_TRACE("CDirect3D9ExDraw::Present(), PresentEx S_PRESENT_MODE_CHANGED.");
			m_bNeedCheckDevice = TRUE;
			return CheckingCode_StateChange;
		}
	}
	return CheckingCode_Success;
}

inline void CDirect3D9ExDraw::PrepareD3D9ExObjects(HMONITOR hmonWnd,UINT uiWidth,UINT uiHeight)
{// NOTE: this function always runs under the window thread
	if(NULL == hmonWnd)
	{
		//WSE_ERROR_TRACE("CDirect3D9ExDraw::PrepareD3D9ExObjects(), hmonWnd is empty!");
		return;
	}
	
	if(0 == uiWidth || 0 == uiHeight)
	{
		//WSE_ERROR_TRACE("CDirect3D9ExDraw::PrepareD3D9ExObjects(), Width and Height invalid! Width = "<<uiWidth<<",Height = "<<uiHeight);
		return;
	}
	
	if(StateCode_Active == m_stateCode
		|| StateCode_Invalid == m_stateCode)
		return;// do nothing

	HRESULT hr;
	
	switch(m_stateCode)
	{
	case StateCode_Uncreated:
		{
		//	WSE_INFO_TRACE("CDirect3D9ExDraw::PrepareD3D9ExObjects(), m_stateCode = StateCode_Uncreated");
			hr = CreateDevice(hmonWnd,uiWidth,uiHeight);
			if(SUCCEEDED(hr))
				m_stateCode = StateCode_Active;
			else
			{
				m_stateCode = StateCode_Invalid;
			//	WSE_ERROR_TRACE("CDirect3D9ExDraw::PrepareD3D9ExObjects(), CreateDevice failed! errorcode = "<<hr);
			}
		}
		break;
	case StateCode_NeedRecreate:
		{
		//	WSE_INFO_TRACE("CDirect3D9ExDraw::PrepareD3D9ExObjects(), m_stateCode = StateCode_NeedRecreate");
			UnInitD3D9DeviceExChain();
			
			hr = CreateDevice(hmonWnd,uiWidth,uiHeight);
			if(SUCCEEDED(hr))
				m_stateCode = StateCode_Active;
			else
			{
				m_stateCode = StateCode_Invalid;
			//	WSE_ERROR_TRACE("CDirect3D9ExDraw::PrepareD3D9ExObjects(), CreateDevice failed! errorcode = "<<hr);
			}
		}
		break;
	case StateCode_NeedReset:
		{
		//	WSE_INFO_TRACE("CDirect3D9ExDraw::PrepareD3D9ExObjects(), m_stateCode = StateCode_NeedReset");

			hr = Reset(uiWidth,uiHeight);
			if(SUCCEEDED(hr))
				m_stateCode = StateCode_Active;
			else
			{
				m_stateCode = StateCode_Invalid;
			//	WSE_ERROR_TRACE("CDirect3D9ExDraw::PrepareD3D9ExObjects(), Reset failed! errorcode = "<<hr);
			}
		}
		break;
	}
}
inline BOOL CDirect3D9ExDraw::PrepareImageSurface(DWORD dwWidth,DWORD dwHeight,D3DFORMAT Format,BOOL &bLost)
{
	bLost = FALSE;

	UINT ulWidth = dwWidth;
	UINT ulHeight = dwHeight;
	if(TRUE == DecideImageUseTexture())
		CalcTextureSize(m_d3d9ExDeviceCaps,dwWidth,dwHeight,ulWidth,ulHeight);

	HRESULT hr;
	BOOL bNeedCreate = FALSE;
	if(m_lpD3D9ImageSurface)
	{
		if(NULL != m_lpD3D9ImageTexture && FALSE == DecideImageUseTexture())
		{// current image surface is created as RT texture && no image effect
		//	WSE_INFO_TRACE("CDirect3D9ExDraw::PrepareImageSurface(), RTTexture -> RT");
			bNeedCreate = TRUE;
		}
		else if(NULL == m_lpD3D9ImageTexture && TRUE == DecideImageUseTexture())
		{
		//	WSE_INFO_TRACE("CDirect3D9ExDraw::PrepareImageSurface(), RT -> RTTexture");
			bNeedCreate = TRUE;
		}
		else if(m_d3dsdImage.Width != ulWidth || m_d3dsdImage.Height != ulHeight)
		{
		//	WSE_INFO_TRACE("CDirect3D9ExDraw::PrepareImageSurface(), Image size change! ("<<m_d3dsdImage.Width<<","<<m_d3dsdImage.Height<<")->("<<ulWidth<<","<<ulHeight<<")("<<dwWidth<<","<<dwHeight<<")");
			bNeedCreate = TRUE;
		}
		else if(m_d3dsdImage.Format != Format)
		{
		//	WSE_INFO_TRACE("CDirect3D9ExDraw::PrepareImageSurface(), Image format change! ("<<m_d3dsdImage.Format<<")->("<<Format<<")");
			bNeedCreate = TRUE;
		}
	}
	else
		bNeedCreate = TRUE;
	if(bNeedCreate == TRUE)
	{
		// release
		SAFE_RELEASE(m_lpD3D9ImageTexture);
		SAFE_RELEASE(m_lpD3D9ImageSurface);

		bLost = TRUE;

		if(TRUE == DecideImageUseTexture())
		{// RT texture
			hr = m_pD3D9DeviceWrapper->CreateTexture(ulWidth,
				ulHeight,
				1,
				D3DUSAGE_RENDERTARGET,
				Format,
				D3DPOOL_DEFAULT,
				&m_lpD3D9ImageTexture,
				NULL);
			if(FAILED(hr))
			{
			//	WSE_ERROR_TRACE("CDirect3D9ExDraw::PrepareImageSurface(), CreateTexture(D3DUSAGE_RENDERTARGET) failed! errorcode = "<<hr);
				return FALSE;
			}
			
		//	WSE_INFO_TRACE("Image texture is created! texture size("<<ulWidth<<","<<ulHeight<<"),original size("<<dwWidth<<","<<dwHeight<<")");
			
			hr = m_lpD3D9ImageTexture->GetSurfaceLevel(0,&m_lpD3D9ImageSurface);
			if(FAILED(hr))
			{
		//		WSE_ERROR_TRACE("CDirect3D9ExDraw::PrepareImageSurface(), GetSurfaceLevel failed! errorcode = "<<hr);
				return FALSE;
			}
			
			hr = m_lpD3D9ImageSurface->GetDesc(&m_d3dsdImage);
			if(FAILED(hr))
			{
		//		WSE_ERROR_TRACE("CDirect3D9ExDraw::PrepareImageSurface(), GetDesc failed! errorcode = "<<hr);
				return FALSE;
			}
		}
		else
		{// RT
			hr = m_pD3D9DeviceWrapper->CreateRenderTargetEx(ulWidth,ulHeight,
				Format,
				D3DMULTISAMPLE_NONE,0,FALSE,
				&m_lpD3D9ImageSurface,
				NULL,0);
			if(FAILED(hr))
			{
		//		WSE_ERROR_TRACE("CDirect3D9ExDraw::PrepareImageSurface(), CreateRenderTargetEx failed! errorcode = "<<hr);
				return FALSE;
			}

		//	WSE_INFO_TRACE("Image Surface is created! surface size("<<ulWidth<<","<<ulHeight<<")");

			hr = m_lpD3D9ImageSurface->GetDesc(&m_d3dsdImage);
			if(FAILED(hr))
			{
		//		WSE_ERROR_TRACE("CDirect3D9ExDraw::PrepareImageSurface(), GetDesc failed! errorcode = "<<hr);
				return FALSE;
			}
		}
	}
	return TRUE;
}
inline BOOL CDirect3D9ExDraw::PrepareRawSurface(DWORD dwWidth,DWORD dwHeight,DWORD dwPixelFormat)
{
	BOOL bNeedCreateSurface = FALSE;
	
	HRESULT hr = E_FAIL;
	if(m_lpD3D9RawSurface)
	{
		do 
		{
			if(dwWidth != m_d3dsdRaw.Width || dwHeight != m_d3dsdRaw.Height)
			{
			//	WSE_INFO_TRACE("CDirect3D9ExDraw::PrepareRawSurface(), Raw size change! ("<<m_d3dsdRaw.Width<<","<<m_d3dsdRaw.Height<<")->("<<dwWidth<<","<<dwHeight<<")");
				bNeedCreateSurface = TRUE;
				break;
			}

			if(dwPixelFormat == BI_RGB)
			{
				if(m_d3dsdRaw.Format != m_d3dsdImage.Format)// RGB use imagesurface's format
				{
			//		WSE_INFO_TRACE("CDirect3D9ExDraw::PrepareRawSurface(), Raw pixel format unmatch! (current:"<<m_d3dsdRaw.Format<<")->(wanted:"<<m_d3dsdImage.Format<<")");
					bNeedCreateSurface = TRUE;
					break;
				}
			}
			else
			{
				const DWORD* pixelformats = NULL;
				int cnt = GetCompatibleFourCC(dwPixelFormat,(LPDWORD*)&pixelformats);
				if(0 == cnt)
				{
			//		WSE_ERROR_TRACE("CDirect3D9ExDraw::PrepareRawSurface(), GetCompatibleFourCC failed! (dwPixelFormat:"<<dwPixelFormat<<")");	
					bNeedCreateSurface = TRUE;
					break;
				}

				int i = 0;
				for(i = 0;i < cnt;i++)
				{
					if(m_d3dsdRaw.Format == pixelformats[i])
						break;
				}
				if(i >= cnt)
				{
			//		WSE_INFO_TRACE("CDirect3D9ExDraw::PrepareRawSurface(), Raw pixel format unmatch! (current:"<<m_d3dsdRaw.Format<<")->(wanted:"<<dwPixelFormat<<")");	
					bNeedCreateSurface = TRUE;
					break;
				}
			}
		}
		while(0);
	}
	else
		bNeedCreateSurface = TRUE;
	if(bNeedCreateSurface)
	{
		// release
		SAFE_RELEASE(m_lpD3D9RawSurface);

		if(BI_RGB == dwPixelFormat)
		{
			hr = m_pD3D9DeviceWrapper->CreateOffscreenPlainSurfaceEx(dwWidth,
				dwHeight,
				m_d3dsdImage.Format,
				D3DPOOL_DEFAULT,
				&m_lpD3D9RawSurface,
				NULL,0);
			if(FAILED(hr))
			{
		//		WSE_ERROR_TRACE("CDirect3D9ExDraw::PrepareRawSurface(), CreateOffscreenPlainSurfaceEx failed! errorcode = "<<hr);
				return FALSE;
			}

		//	WSE_INFO_TRACE("CDirect3D9ExDraw::PrepareRawSurface(), Raw Surface is created! dwWidth = "<<dwWidth<<",dwHeight = "<<dwHeight<<",dwFourCC = BI_RGB");
		}
		else
		{
			const DWORD* pixelformats = NULL;
			int cnt = GetCompatibleFourCC(dwPixelFormat,(LPDWORD*)&pixelformats);
			if(0 == cnt)
			{
		//		WSE_ERROR_TRACE("CDirect3D9ExDraw::PrepareRawSurface(), GetCompatibleFourCC failed! (dwPixelFormat:"<<dwPixelFormat<<")");	
				return FALSE;
			}

			int i = 0;
			for(i = 0;i < cnt;i++)
			{
				hr = m_pD3D9DeviceWrapper->CreateOffscreenPlainSurfaceEx(dwWidth,
					dwHeight,
					(D3DFORMAT)pixelformats[i],
					D3DPOOL_DEFAULT,
					&m_lpD3D9RawSurface,
					NULL,0);
				if(D3D_OK == hr)
					break;
			}
			if(i >= cnt)
			{
		//		WSE_ERROR_TRACE("CDirect3D9ExDraw::PrepareRawSurface(), CreateOffscreenPlainSurfaceEx failed! errorcode = "<<hr);
				return FALSE;
			}

		//	WSE_INFO_TRACE("CDirect3D9ExDraw::PrepareRawSurface(), Raw Surface is created! dwWidth = "<<dwWidth<<",dwHeight = "<<dwHeight<<",dwFourCC = "<<pixelformats[i]<<",dwPixelFormat(wanted) = "<<dwPixelFormat);
		}

		hr = m_lpD3D9RawSurface->GetDesc(&m_d3dsdRaw);
		if(FAILED(hr))
		{
		//	WSE_ERROR_TRACE("CDirect3D9ExDraw::PrepareRawSurface(), GetDesc failed! errorcode = "<<hr);
			return FALSE;
		}
	}
	return TRUE;
}
inline HRESULT CDirect3D9ExDraw::FillSurface_YUY2AndUYVY(LPDIRECT3DSURFACE9 pd3dsDest,const D3DSURFACE_DESC& d3dsdDest,LPBYTE pImage,LPBITMAPINFOHEADER pbmi,const RECT& rect)
{
	HRESULT hr;
	
 	D3DLOCKED_RECT dr;
 	hr = pd3dsDest->LockRect(&dr,NULL,0);
	if(D3D_OK != hr)
	{
	//	WSE_ERROR_TRACE("CDirect3D9ExDraw::FillSurface_YUY2AndUYVY(), LockRect failed! errorcode = "<<hr);
		return hr;
	}

	if(dr.pBits)
	{
		// 1.assume top-down image
		// 2.assume macro-pixel boundary offset
		// odd left offset and odd blt width are not expected for YUYV(4:2:2) UYVY(4:2:2).
		int stride = pbmi->biWidth * 2;
		int offset = rect.top * stride + rect.left * 2;
		LPBYTE p = pImage + offset;

		BOOL bException = FALSE;
		if(FALSE == BltYUV_Packed_Safe(p,stride,pbmi->biCompression,(LPBYTE)dr.pBits,dr.Pitch,d3dsdDest.Format,rect.right - rect.left,rect.bottom - rect.top,bException))
		{
			if(TRUE == bException)
			{
	//			WSE_ERROR_TRACE("CDirect3D9ExDraw::FillSurface_YUY2AndUYVY(), BltYUV_Packed_Safe an exception occured! pSrc = "<<p<<",pDst = "<<dr.pBits);
			}
			else
			{
		//		WSE_ERROR_TRACE("CDirect3D9ExDraw::FillSurface_YUY2AndUYVY(), BltYUV_Packed_Safe failed! ("<<pbmi->biCompression<<") -> ("<<d3dsdDest.Format<<")");
			}
			hr = E_FAIL;
		}
	}
	else
	{
	//	WSE_ERROR_TRACE("CDirect3D9ExDraw::FillSurface_YUY2AndUYVY(), dr.pBits is empty!)");
		hr = E_FAIL;
	}

 	pd3dsDest->UnlockRect();
	return hr;
}
inline HRESULT CDirect3D9ExDraw::FillSurface_I420AndYV12(LPDIRECT3DSURFACE9 pd3dsDest,const D3DSURFACE_DESC& d3dsdDest,LPBYTE pImage,LPBITMAPINFOHEADER pbmi,const RECT& rect)
{
	HRESULT hr;
	
 	D3DLOCKED_RECT dr;
 	hr = pd3dsDest->LockRect(&dr,NULL,0);
	if(D3D_OK != hr)
	{
	//	WSE_ERROR_TRACE("CDirect3D9ExDraw::FillSurface_I420AndYV12(), LockRect failed! errorcode = "<<hr);
		return hr;
	}

	if(dr.pBits)
	{
		// 1.assume top-down image
		// 2.assume macro-pixel boundary offset
		// odd left-top offset and odd blt size are not expected for YV12(4:2:0) I420(4:2:0).
		UINT uiHeight = abs(pbmi->biHeight);

		int strideY = pbmi->biWidth;
		int strideUV = pbmi->biWidth / 2;

		LPBYTE lpY = pImage;
		LPBYTE lpV;
		LPBYTE lpU;
		if(pbmi->biCompression == MAKEFOURCC('Y','V','1','2'))
		{
			lpV = pImage + strideY * uiHeight;
			lpU = pImage + strideY * uiHeight + strideUV * uiHeight / 2;
		}
		else if(pbmi->biCompression == MAKEFOURCC('I','4','2','0'))
		{
			lpU = pImage + strideY * uiHeight;
			lpV = pImage + strideY * uiHeight + strideUV * uiHeight / 2;
		}else{
			pd3dsDest->UnlockRect();
	//		WSE_ERROR_TRACE("CDirect3D9ExDraw::FillSurface_I420AndYV12(), wrong compression occured!  "<<pbmi->biCompression);
			hr = E_FAIL;
			return hr;
		}

		int offsetY = rect.top * strideY + rect.left;
		int offsetUV = rect.top/2 * strideUV + rect.left/2;
		LPBYTE pY = lpY + offsetY;
		LPBYTE pV = lpV + offsetUV;
		LPBYTE pU = lpU + offsetUV;

		BOOL bException = FALSE;
		if(FALSE == BltYUV_Planar_Safe(pY,pU,pV,strideY,strideUV,(LPBYTE)dr.pBits,dr.Pitch,d3dsdDest.Format,rect.right - rect.left,rect.bottom - rect.top,bException))
		{
			if(TRUE == bException)
			{
	//			WSE_ERROR_TRACE("CDirect3D9ExDraw::FillSurface_I420AndYV12(), BltYUV_Planar_Safe an exception occured! pY = "<<pY<<",pU = "<<pU<<",pV = "<<pV<<",pDst = "<<dr.pBits);
			}
			else
			{
	//			WSE_ERROR_TRACE("CDirect3D9ExDraw::FillSurface_I420AndYV12(), BltYUV_Planar_Safe failed! ("<<pbmi->biCompression<<") -> ("<<d3dsdDest.Format<<")");
			}
			hr = E_FAIL;
		}
	}
	else
	{
	//	WSE_ERROR_TRACE("CDirect3D9ExDraw::FillSurface_I420AndYV12(), dr.pBits is empty!)");
		hr = E_FAIL;
	}

 	pd3dsDest->UnlockRect();
	return hr;
}
inline HRESULT CDirect3D9ExDraw::FillSurface_OtherPixelFormat(LPDIRECT3DSURFACE9 pd3dsDest,const D3DSURFACE_DESC& d3dsdDest,LPBYTE pImage,LPBITMAPINFOHEADER pbmi,const RECT& rect)
{
	if(pbmi->biCompression == MAKEFOURCC('I','4','2','0')
		|| pbmi->biCompression == MAKEFOURCC('Y','V','1','2'))
		return FillSurface_I420AndYV12(pd3dsDest,d3dsdDest,pImage,pbmi,rect);
	else if(pbmi->biCompression == MAKEFOURCC('Y','U','Y','2')
		|| pbmi->biCompression == MAKEFOURCC('U','Y','V','Y'))
		return FillSurface_YUY2AndUYVY(pd3dsDest,d3dsdDest,pImage,pbmi,rect);
	
	//WSE_ERROR_TRACE("CDirect3D9ExDraw::FillSurface_OtherPixelFormat(), i can not believe i am meeting unknown pixel format!!!");
	return E_ABORT;
}
inline HRESULT CDirect3D9ExDraw::FillSurface_BGR(LPDIRECT3DSURFACE9 pd3dsDest,const D3DSURFACE_DESC& d3dsdDest,LPBYTE pImage,LPBITMAPINFOHEADER pbmi,const RECT& rect)
{
	HRESULT hr;

	if(rect.left >= 0 && rect.right <= pbmi->biWidth && rect.top >= 0 && rect.bottom <= abs(pbmi->biHeight))
	{// the rectangle is inside of the image
		fmt_BGR fmt;
		switch(d3dsdDest.Format)
		{
		case D3DFMT_X8R8G8B8:
			fmt = fmt_X8R8G8B8;
			break;
		case D3DFMT_R8G8B8:
			fmt = fmt_R8G8B8;
			break;
		case D3DFMT_R5G6B5:
			fmt = fmt_R5G6B5;
			break;
		case D3DFMT_X1R5G5B5:
			fmt = fmt_X1R5G5B5;
			break;
		default:
			goto __NEXT;// unsupport format
		}

		D3DLOCKED_RECT dr;
		hr = pd3dsDest->LockRect(&dr,NULL,0);
		if(D3D_OK != hr)
		{
	//		WSE_ERROR_TRACE("CDirect3D9ExDraw::FillSurface_BGR(), LockRect failed! errorcode = "<<hr);
			return hr;
		}

		if(dr.pBits)
		{
			// input image always RGB24
			UINT uiWidth = pbmi->biWidth;
			UINT uiHeight = abs(pbmi->biHeight);

			int stride = ((pbmi->biWidth*pbmi->biBitCount+31)&~31)/8;// input image always RGB24
			LPBYTE p = NULL;

			if(pbmi->biHeight < 0)
			{
				p = pImage;

				int offset = rect.top * stride + rect.left * 3;
				p += offset;
			}
			else
			{
				p = pImage + stride*(uiHeight-1);
				stride = -stride;

				int offset = rect.top * stride + rect.left * 3;
				p += offset;
			}

			BOOL bException = FALSE;
			if(FALSE == BltBGR_Safe(p,stride,(LPBYTE)dr.pBits,dr.Pitch,fmt,rect.right - rect.left,rect.bottom - rect.top,bException))
			{
				if(TRUE == bException)
				{
			//		WSE_ERROR_TRACE("CDirect3D9ExDraw::FillSurface_BGR(), BltBGR_Safe an exception occured! pSrc = "<<p<<",pDst = "<<dr.pBits);
				}
				else
				{
		//			WSE_ERROR_TRACE("CDirect3D9ExDraw::FillSurface_BGR(), BltBGR_Safe failed! fmtDst = %d"<<fmt);
				}
				hr = E_ABORT;
			}
		}
		else
		{
	//		WSE_ERROR_TRACE("CDirect3D9ExDraw::FillSurface_BGR(), dr.pBits is empty!");
			hr = E_FAIL;
		}

		pd3dsDest->UnlockRect();
	}
	else
	{
__NEXT:
		HDC hD3DDC;
		hr = m_lpD3D9RawSurface->GetDC(&hD3DDC);
		if(D3D_OK != hr)
		{
	//		WSE_ERROR_TRACE("CDirect3D9ExDraw::FillSurface_BGR(), GetDC failed! errorcode = "<<hr);
			return hr;
		}

		RECT rc = rect;
		if(pbmi->biHeight > 0)
		{//bottom top
			rc.top = pbmi->biHeight - rect.bottom;
			rc.bottom = pbmi->biHeight - rect.top;
		}
		else
		{
			rc.bottom = -pbmi->biHeight - rect.top;
			rc.top = -pbmi->biHeight - rect.bottom;
		}
		
		StretchDIBits(hD3DDC,
			0,0,rect.right - rect.left,rect.bottom - rect.top,
			rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top,
			pImage,(BITMAPINFO *)pbmi,DIB_RGB_COLORS,SRCCOPY);
		
		hr = m_lpD3D9RawSurface->ReleaseDC(hD3DDC);
		if(D3D_OK != hr)
		{
	//		WSE_ERROR_TRACE("CDirect3D9ExDraw::FillSurface_BGR(), ReleaseDC failed! errorcode = "<<hr);
		}
		DeleteDC(hD3DDC);
	}
	return hr;
}
inline HRESULT CDirect3D9ExDraw::FillRawSurface(LPBYTE pImage,LPBITMAPINFOHEADER pbmi,const RECT& rect)
{
	HRESULT hr = E_ABORT;
	do
	{
		if(BI_RGB == pbmi->biCompression)
		{// rgb
			hr = FillSurface_BGR(m_lpD3D9RawSurface,m_d3dsdRaw,pImage,pbmi,rect);
			if(D3D_OK != hr)
			{
		//		WSE_ERROR_TRACE("CDirect3D9ExDraw::FillRawSurface() FillSurface_BGR failed! errorcode = "<<hr);
				break;
			}
		}
		else
		{
			hr = FillSurface_OtherPixelFormat(m_lpD3D9RawSurface,m_d3dsdRaw,pImage,pbmi,rect);
			if(D3D_OK != hr)
			{
		//		WSE_ERROR_TRACE("CDirect3D9ExDraw::FillRawSurface() FillSurface_OtherPixelFormat failed! errorcode = "<<hr);
				break;
			}
		}
	}
	while(0);
	return hr;
}
BOOL CDirect3D9ExDraw::IsWindowThread()
{
	return (GetCurrentThreadId() == m_dwTIDWindow);
}
void CDirect3D9ExDraw::NotifyD3D9ObjectsNormalization()
{
	m_pHelper->TriggerWindowThreadAction();
}
void CDirect3D9ExDraw::WindowThreadAction()
{
	//WSE_INFO_TRACE("CDirect3D9ExDraw::WindowThreadAction()...........");

	RECT rcVSC = m_rcTarget;
	MapWindowPoints(m_hWnd,NULL,(LPPOINT)&rcVSC,2);
	HMONITOR hmonWnd = (HMONITOR)m_hWnd;//MonitorFromRect(&rcVSC,MONITOR_DEFAULTTONEAREST);

	UINT uiWidth = m_rcTarget.right - m_rcTarget.left;
	UINT uiHeight = m_rcTarget.bottom - m_rcTarget.top;
	PrepareD3D9ExObjects(hmonWnd,uiWidth,uiHeight);
}
BOOL CDirect3D9ExDraw::IsNeedWindowThreadDestroy()
{
	return TRUE;
}
BOOL CDirect3D9ExDraw::IsFourCCSupported(DWORD dwFourCC)
{
	if(dwFourCC == BI_RGB)
		return TRUE;

	const DWORD* pixelformats = NULL;
	int cnt = GetCompatibleFourCC(dwFourCC,(LPDWORD*)&pixelformats);
	if(0 == cnt)
		return FALSE;

	if(0 == m_listFourCC.size())
	{
		HRESULT hr;
		
		// following process may be inexact.
		// since the adapter will be changed by move the window(m_hmon may be inexact).
		// but this place we not to do monitor checking.
		UINT uAdapter = D3DADAPTER_DEFAULT;
		//UINT uCnt = m_lpD3D9Ex->GetAdapterCount();
		UINT uCnt = m_pD3D9Wrapper->GetAdapterCount();
		for(UINT j=0;j<uCnt;j++)
		{
			//HMONITOR hmon = m_lpD3D9Ex->GetAdapterMonitor(j);
			HMONITOR hmon = m_pD3D9Wrapper->GetAdapterMonitor(j);
			if(hmon == m_hmon)
			{
				uAdapter = j;
				break;
			}
		}
		
		// get target format
		D3DDISPLAYMODEEX d3ddm;
		d3ddm.Size = sizeof(d3ddm);
		//hr = m_lpD3D9Ex->GetAdapterDisplayModeEx(uAdapter,&d3ddm,NULL);
		hr = m_pD3D9Wrapper->GetAdapterDisplayModeEx(uAdapter, &d3ddm, NULL);
		if(FAILED(hr))
		{
		//	WSE_ERROR_TRACE("CDirect3D9ExDraw::IsFourCCSupported(), GetAdapterDisplayModeEx failed! errorcode = "<<hr);
			return FALSE;
		}

		UpdateFourCCList(uAdapter,d3ddm.Format);
	}

	for(FourCCList::iterator it = m_listFourCC.begin();it!=m_listFourCC.end();++it)
	{
		if(TRUE == IsFourCCMatched(dwFourCC,*it))
			return TRUE;
	}

	return FALSE;
}
BOOL CDirect3D9ExDraw::UpdateDrawTargetRectangle(const RECT& rcTarget)
{// this function intends to do prepare job AT WINDOW THREAD, if not at windows thread it is out of design
	if(!IsWindowThread())
	{// want windows thread!
	//	WSE_WARN_TRACE("CDirect3D9ExDraw::UpdateDrawTargetRectangle(),!IsWindowThread(),UpdateDrawTargetRectangle() expect be done using the same thread as the window!");
	}

	if(StateCode_Invalid == m_stateCode)
		return FALSE;

	if(IsRectEmpty(&rcTarget))
		return TRUE;

	RECT rcVSC = rcTarget;
	MapWindowPoints(m_hWnd,NULL,(LPPOINT)&rcVSC,2);
	HMONITOR hmonWnd = (HMONITOR)m_hWnd;//MonitorFromRect(&rcVSC,MONITOR_DEFAULTTONULL);
//	WSE_INFO_TRACE_THIS("CDirect3D9ExDraw::UpdateDrawTargetRectangle hmonWnd = " << hmonWnd);
	if(NULL == hmonWnd)
		return TRUE;// out of monitor
	
	// record
	m_rcTarget = rcTarget;

	UINT uiWidth = m_rcTarget.right-m_rcTarget.left;
	UINT uiHeight = m_rcTarget.bottom-m_rcTarget.top;

	CDirect3D9ExDraw::CheckingCode cc = CheckAndRecoverD3D9ExObjects(hmonWnd,uiWidth,uiHeight);
	switch(cc)
	{
	case CheckingCode_Success:
		break;
	case CheckingCode_StateChange:
		return TRUE;
	default:
		return FALSE;
	}

	return TRUE;
}
inline BOOL CDirect3D9ExDraw::DecideImageUseTexture()
{
	if((m_dwImageEffect & IMAGEEFFECT_MIRRORUPDOWN) || (m_dwImageEffect & IMAGEEFFECT_MIRRORLEFTRIGHT))
		return TRUE;
	return FALSE;
}
inline BOOL CDirect3D9ExDraw::Do_Image2BackBuffer(const RECT& rcSrcRect,LPDIRECT3DSURFACE9 pBackBufferSuface,const RECT& rcDestRect)
{
	HRESULT hr;
	if(FALSE == DecideImageUseTexture())
	{
		hr = m_pD3D9DeviceWrapper->StretchRect(m_lpD3D9ImageSurface,&rcSrcRect,pBackBufferSuface,&rcDestRect,D3DTEXF_LINEAR);
		if(D3D_OK != hr)
		{
//			WSE_ERROR_TRACE("CDirect3D9ExDraw::Do_Image2BackBuffer(), StretchRect(Image->BackBuffer) failed! errorcode = "<<hr);
			return FALSE;
		}
	}
	else
	{
		hr = m_pD3D9DeviceWrapper->SetRenderTarget(0,pBackBufferSuface);
		if(D3D_OK != hr)
		{
//			WSE_ERROR_TRACE("CDirect3D9ExDraw::Do_Image2BackBuffer(), SetRenderTarget failed! errorcode = "<<hr);
			return FALSE;
		}
		
		hr = m_pD3D9DeviceWrapper->BeginScene();
		if(D3D_OK != hr)
		{
	//		WSE_ERROR_TRACE("CDirect3D9ExDraw::Do_Image2BackBuffer(),BeginScene failed! errorcode = "<<hr);
			return FALSE;
		}
		
		RECT rcTarget = rcDestRect;
		RECT rcSrc = rcSrcRect;
		if(m_dwImageEffect & IMAGEEFFECT_MIRRORUPDOWN)
		{
			rcSrc.top = rcSrcRect.bottom;
			rcSrc.bottom = rcSrcRect.top;
		}
		if(m_dwImageEffect & IMAGEEFFECT_MIRRORLEFTRIGHT)
		{
			rcSrc.left = rcSrcRect.right;
			rcSrc.right = rcSrcRect.left;
		}
		
		hr = DoBlt(m_pD3D9DeviceWrapper->GetDirect3DDevice9(),&rcTarget,m_lpD3D9ImageTexture,m_d3dsdImage,&rcSrc,0xff,FALSE);
		
		HRESULT hrEnd = m_pD3D9DeviceWrapper->EndScene();
		if(D3D_OK != hrEnd)
		{
	//		WSE_ERROR_TRACE("CDirect3D9ExDraw::Do_Image2BackBuffer(),EndScene failed! errorcode = "<<hrEnd);
			return FALSE;
		}
		
		if(D3D_OK != hr)
			return FALSE;
	}
	return TRUE;
}
long CDirect3D9ExDraw::Draw(LPBYTE pImage,LPBITMAPINFOHEADER pbmi,HDC hDrawToDC,const RECT& rcSource,const RECT& rcTarget)
{
	BOOL bUpdateImage = TRUE;
	if(NULL == pImage || NULL == pbmi)
		bUpdateImage = FALSE;

	if(bUpdateImage)
	{// set flag that both surface expired
		m_bRawExpired = TRUE;
		m_bImageExpired = TRUE;
		if(m_mapBlackList[kNeed128BytesAligned] == 1 && pbmi->biCompression != BI_RGB)
		{
			//WSE_INFO_TRACE_THIS("black list matched, need 128Byte Aligned");
			return CDraw_UnsupportedFormat;
		}
	}

	if(StateCode_Invalid == m_stateCode)
		return CDraw_Failure;
	
	if(IsRectEmpty(&rcTarget))
		return CDraw_Success;

	RECT rcVSC = rcTarget;
	MapWindowPoints(m_hWnd,NULL,(LPPOINT)&rcVSC,2);
	HMONITOR hmonWnd = (HMONITOR)m_hWnd;//MonitorFromRect(&rcVSC,MONITOR_DEFAULTTONULL);
	if(NULL == hmonWnd)
		return CDraw_Success;// out of monitor

	// record
	m_rcTarget = rcTarget;

	RECT rcBackBuffer;
	SetRect(&rcBackBuffer,0,0,m_rcTarget.right-m_rcTarget.left,m_rcTarget.bottom-m_rcTarget.top);

	CDirect3D9ExDraw::CheckingCode cc = CheckAndRecoverD3D9ExObjects(hmonWnd,rcBackBuffer.right,rcBackBuffer.bottom);
	switch(cc)
	{
	case CheckingCode_Success:
		break;
	case CheckingCode_StateChange:
		return CDraw_Success;
	default:
		return CDraw_Failure;
	}

	if(bUpdateImage)
	{// check fourcc
		if(!IsFourCCSupported(pbmi->biCompression))
			return CDraw_UnsupportedFormat;
	}

	CComPtr<IDirect3DSurface9> pBackBuffer;
	if(FALSE == GetBackBuffer(&pBackBuffer))
		return CDraw_Failure;

	BOOL bImageLost = FALSE;
	RECT rcImage;
	RECT rcVideoTarget;
	RECT rcVideoSrc;
	TuneTargetRectangleByAspectRatioMode(rcVideoTarget, rcVideoSrc, rcSource,rcBackBuffer,pbmi?pbmi->biCompression:0);

	SetRect(&rcImage,0,0,rcVideoSrc.right-rcVideoSrc.left,rcVideoSrc.bottom-rcVideoSrc.top);
	if(FALSE == PrepareImageSurface(rcImage.right,rcImage.bottom,m_d3dpp.BackBufferFormat,bImageLost))
		return CDraw_Failure;
	if(TRUE == bImageLost)
		m_bImageExpired = TRUE;// the data held by image surface was lost

	if(bUpdateImage)
	{
		if(BI_RGB == pbmi->biCompression)
		{
			if(!PrepareRawSurface(rcImage.right,rcImage.bottom,BI_RGB))
				return CDraw_Failure;
		}
		else
		{// prepare Raw surface
			if(!PrepareRawSurface(rcImage.right,rcImage.bottom,pbmi->biCompression))
				return CDraw_UnsupportedFormat;// need RGB data
		}
	}
	else
	{
		if(TRUE == m_bImageExpired)
		{// image surface's data is lost or dirty,so need to check raw surface whether has valid data
			if(TRUE == m_bRawExpired)
			{// raw surface expired
				//WSE_INFO_TRACE("CDirect3D9ExDraw::Draw(),current image and raw data is expired.");
				return CDraw_NeedData;	
			}
			if(NULL == m_lpD3D9RawSurface)
			{// raw surface uncreated
				//WSE_INFO_TRACE("CDirect3D9ExDraw::Draw(), raw surface is not created!");
				return CDraw_NeedData;
			}
		}
	}

	// prepare all textures
	if(!PrepareTextures(0))
		return CDraw_Failure;

	HRESULT hr;
	if(bUpdateImage)
	{
		hr = FillRawSurface(pImage,pbmi,rcVideoSrc);
		if(D3D_OK != hr)
		{
			//WSE_ERROR_TRACE("CDirect3D9ExDraw::Draw() FillRawSurface failed! errorcode = "<<hr);
			return CDraw_Failure;
		}
		
		m_bRawExpired = FALSE;// set flag that raw surface available for redraw
		
		hr = m_pD3D9DeviceWrapper->StretchRect(m_lpD3D9RawSurface,NULL,m_lpD3D9ImageSurface,&rcImage,D3DTEXF_NONE);
		if(D3D_OK != hr)
		{
			//WSE_ERROR_TRACE("CDirect3D9ExDraw::Draw(), StretchRect(Raw->Image) failed! errorcode = "<<hr);
			return CDraw_Failure;
		}
	}
	else
	{
		if(TRUE == m_bImageExpired)
		{
			// raw surface has been checked above
			// raw surface -> image surface
			hr = m_pD3D9DeviceWrapper->StretchRect(m_lpD3D9RawSurface,NULL,m_lpD3D9ImageSurface,&rcImage,D3DTEXF_NONE);
			if(D3D_OK != hr)
			{
				//WSE_ERROR_TRACE("CDirect3D9ExDraw::Draw(), StretchRect(Raw->Image) failed! errorcode = "<<hr);
				return CDraw_Failure;
			}
		}
		else
			goto _STARTBACKBUFFERPROCESS;// use image surface directly	
	}
	// before Zoom
	if(HasEffectActOnSrc())
	{
		hr = Do_Effect(m_lpD3D9ImageSurface,rcImage.right,rcImage.bottom,TRUE);
 		if(D3D_OK != hr)
 		{
			//WSE_ERROR_TRACE("CDirect3D9ExDraw::Draw(), Do_Effect_OnSrc failed! errorcode = "<<hr);
			return CDraw_Failure;
		}
	}
	
	m_bImageExpired = FALSE;// set flag that image surface available for redraw

_STARTBACKBUFFERPROCESS:
	
	if(FALSE == EqualRect(&rcVideoTarget,&rcBackBuffer))
	{// need fill bkgnd firstly
		PrepareBkgnd();
		FillBkgnd(pBackBuffer,&rcBackBuffer);
	}

	if(FALSE == Do_Image2BackBuffer(rcImage,pBackBuffer,rcVideoTarget))
	{
		//WSE_ERROR_TRACE("CDirect3D9ExDraw::Draw(), Do_Image2BackBuffer failed!");
		return CDraw_Failure;
	}

	// after Zoom
	if(HasEffectActOnDest())
	{
		hr = Do_Effect(pBackBuffer,rcBackBuffer.right,rcBackBuffer.bottom,FALSE);
		if(D3D_OK != hr)
		{
			//WSE_ERROR_TRACE("CDirect3D9ExDraw::Draw(), Do_Effect_OnDest failed! errorcode = "<<hr);
			return CDraw_Failure;
		}
	}

	cc = Present(&rcBackBuffer,&m_rcTarget);
	if(CheckingCode_Error == cc)
		return CDraw_Failure;
	return CDraw_Success;
}
BOOL CDirect3D9ExDraw::DrawBkgnd(HDC hDrawToDC,const RECT& rcTarget)
{
	if(StateCode_Invalid == m_stateCode)
		return FALSE;

	if(IsRectEmpty(&rcTarget))
		return TRUE;
	
	RECT rcVSC = rcTarget;
	MapWindowPoints(m_hWnd,NULL,(LPPOINT)&rcVSC,2);
	HMONITOR hmonWnd = (HMONITOR)m_hWnd;//MonitorFromRect(&rcVSC,MONITOR_DEFAULTTONULL);
	if(NULL == hmonWnd)
		return TRUE;// out of monitor
	
	// record
	m_rcTarget = rcTarget;

	RECT rcBackBuffer;
	SetRect(&rcBackBuffer,0,0,m_rcTarget.right-m_rcTarget.left,m_rcTarget.bottom-m_rcTarget.top);

	CDirect3D9ExDraw::CheckingCode cc = CheckAndRecoverD3D9ExObjects(hmonWnd,rcBackBuffer.right,rcBackBuffer.bottom);
	switch(cc)
	{
	case CheckingCode_Success:
		break;
	case CheckingCode_StateChange:
		return TRUE;
	default:
		return FALSE;
	}

	CComPtr<IDirect3DSurface9> pBackBuffer;
	if(FALSE == GetBackBuffer(&pBackBuffer))
		return FALSE;

	// prepare all textures
	if(!PrepareTextures(2))// just prepare backbuffer's textures
		return FALSE;

	PrepareBkgnd();

	HRESULT hr;
	hr = FillBkgnd(pBackBuffer,&rcBackBuffer);
	if(D3D_OK != hr)
	{
		//WSE_ERROR_TRACE("CDirect3D9ExDraw::DrawBkgnd(), FillBkgnd failed! errorcode = "<<hr);
		return FALSE;
	}
	
	// after Zoom
	if(HasEffectActOnDest())
	{
		hr = Do_Effect(pBackBuffer,rcBackBuffer.right,rcBackBuffer.bottom,FALSE);
		if(D3D_OK != hr)
		{
			//WSE_ERROR_TRACE("CDirect3D9ExDraw::DrawBkgnd(), Do_Effect_OnDest failed! errorcode = "<<hr);
			return FALSE;
		}
	}

	if(CheckingCode_Error == Present(&rcBackBuffer,&m_rcTarget))
		return FALSE;
	return TRUE;
}
BOOL CDirect3D9ExDraw::AddDrawBitmap(unsigned int uID,PBMP_DESC pParam)
{
	ID2BPMap::iterator itfind = m_mapID2BP.find(uID);
	if(itfind != m_mapID2BP.end())
	{
		if(itfind->second.bd.bmptype != pParam->bmptype ||
			itfind->second.bd.bitmap != pParam->bitmap ||
			itfind->second.bd.clrkey != pParam->clrkey)
		{
			FreeBP(&itfind->second);
		}
		itfind->second.bd = *pParam;
	}
	else
	{
		BMP_PARAM bp;
		memset(&bp,0,sizeof(bp));
		bp.bd = *pParam;
		m_mapID2BP[uID] = bp;
	}

 	if(IS_ACTONSRC(pParam->dwMask))
 		m_bImageExpired = TRUE;

	return TRUE;
}
void CDirect3D9ExDraw::RemoveDrawBitmap(unsigned int uID)
{
	ID2BPMap::iterator itfind = m_mapID2BP.find(uID);
	if(itfind != m_mapID2BP.end())
	{
 		if(IS_ACTONSRC(itfind->second.bd.dwMask))
 			m_bImageExpired = TRUE;

		FreeBP(&itfind->second);
		m_mapID2BP.erase(itfind);
	}
}
void CDirect3D9ExDraw::SetBKColor(BK_DESC bk)
{
	FreeBKP(&m_bk);
	m_bk.bkd = bk;
}
void CDirect3D9ExDraw::FreeBP(PBMP_PARAM pbp)
{
	if(pbp && pbp->pRef)
	{
		PD3D9_TEXTURE_PARAM pParam = (PD3D9_TEXTURE_PARAM)pbp->pRef;
		pParam->pd3d9Texture->Release();
		delete pParam;
		pbp->pRef = NULL;
	}
}
void CDirect3D9ExDraw::FreeBKP(PBK_PARAM pbkp)
{
	if(pbkp && pbkp->pRef)
	{
		switch(pbkp->bkd.bktype)
		{
		case BKT_COLOR:
			break;
		case BKT_BITMAP:
			{
				LPDIRECT3DSURFACE9 pd3d9Surface = (LPDIRECT3DSURFACE9)pbkp->pRef;
				pd3d9Surface->Release();
				pbkp->pRef = NULL;
			}
			break;
		}
	}
}
inline HRESULT CDirect3D9ExDraw::FillBkgnd(LPDIRECT3DSURFACE9 lpDrawToD3D9Surface,LPRECT lprcDest)
{
	HRESULT hr = D3D_OK;
	switch(m_bk.bkd.bktype)
	{
	case BKT_COLOR:
		hr = m_pD3D9DeviceWrapper->ColorFill(lpDrawToD3D9Surface,lprcDest,D3DCOLOR_XRGB(GetRValue(m_bk.bkd.clr),GetGValue(m_bk.bkd.clr),GetBValue(m_bk.bkd.clr)));
		if(D3D_OK != hr)
		{
			//WSE_ERROR_TRACE("CDirect3D9ExDraw::FillBkgnd(),ColorFill failed! errorcode = "<<hr);
		}
		break;
	case BKT_BITMAP:
		if(m_bk.pRef != NULL)
		{
			LPDIRECT3DSURFACE9 pd3d9Surface = (LPDIRECT3DSURFACE9)m_bk.pRef;
			hr = m_pD3D9DeviceWrapper->StretchRect(pd3d9Surface,NULL,lpDrawToD3D9Surface,lprcDest,D3DTEXF_LINEAR);
			if(D3D_OK != hr)
			{
				//WSE_ERROR_TRACE("CDirect3D9ExDraw::FillBkgnd(),StretchRect failed! errorcode = "<<hr);
				return hr;
			}
		}
		else
		{
			//WSE_ERROR_TRACE("CDirect3D9ExDraw::FillBkgnd(),m_bk.pRef is empty!");
		}
		break;
	default:
		//WSE_ERROR_TRACE("CDirect3D9ExDraw::FillBkgnd(),unknown m_bk.bkd.bktype! type = "<<m_bk.bkd.bktype);
		break;
	}
	return hr;
}
inline BOOL CDirect3D9ExDraw::PrepareBkgnd()
{
	switch(m_bk.bkd.bktype)
	{
	case BKT_COLOR:
		break;
	case BKT_BITMAP:
		if(NULL == m_bk.pRef)
		{
			LPDIRECT3DSURFACE9 pd3d9Surface = NULL;
			if(TRUE == CreateSurfaceByDesc(&m_bk.bkd.bitmapdesc,m_pD3D9DeviceWrapper,m_d3d9ExDeviceCaps,&pd3d9Surface,NULL))
				m_bk.pRef = (LPVOID)pd3d9Surface;
		}
		break;
	}
	return TRUE;// always success
}
BOOL CDirect3D9ExDraw::CreateTexture(LPBYTE pImage,LPBITMAPINFOHEADER pbmi,COLORREF clrkey,LPDIRECT3DDEVICE9 pd3d9Device,const D3DCAPS9 &d3d9DeviceCapsRef,/*out*/LPDIRECT3DTEXTURE9* ppd3d9Texture,/*out*/D3DSURFACE_DESC* pd3dsdTexture)
{
	if(!pd3d9Device)
		return FALSE;
	
	HRESULT hr;

	DWORD dwWidth = pbmi->biWidth;
	DWORD dwHeight = abs(pbmi->biHeight);

	if(pbmi->biBitCount == 32 && 0xFFFFFFFF == clrkey)
	{// argb
		UINT ulWidth = dwWidth;
		UINT ulHeight = dwHeight;
		CalcTextureSize(d3d9DeviceCapsRef,dwWidth,dwHeight,ulWidth,ulHeight);
		
		CComPtr<IDirect3DTexture9> pd3d9Texture;
		hr = pd3d9Device->CreateTexture(ulWidth,ulHeight,1,0,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,&pd3d9Texture,NULL);
		if(D3D_OK != hr)
		{
			//WSE_ERROR_TRACE("CDirect3D9ExDraw::CreateTexture(),CreateTexture failed! errorcode = "<<hr);
			return FALSE;
		}
		
		CComPtr<IDirect3DTexture9> pd3d9TextureSysMem;
		hr = pd3d9Device->CreateTexture(ulWidth,ulHeight,1,0,D3DFMT_A8R8G8B8,D3DPOOL_SYSTEMMEM,&pd3d9TextureSysMem,NULL);
		if(D3D_OK != hr)
		{
			//WSE_ERROR_TRACE("CDirect3D9ExDraw::CreateTexture(),CreateTexture(SYSTEMMEM) failed! errorcode = "<<hr);
			return FALSE;
		}

		if(TRUE != FillDataToTexture_ARGB2ARGB(pImage,pbmi,pd3d9TextureSysMem))
		{
			//WSE_ERROR_TRACE("CDirect3D9ExDraw::CreateTexture(),FillDataToTexture_ARGB2ARGB failed!");
			return FALSE;
		}

		hr = pd3d9Device->UpdateTexture(pd3d9TextureSysMem,pd3d9Texture);
		if(D3D_OK != hr)
		{
			//WSE_ERROR_TRACE("CDirect3D9ExDraw::CreateTexture(),UpdateTexture() failed! errorcode = "<<hr);
			return FALSE;
		}
		
		if(pd3dsdTexture)
		{// fill d3dsd
			ZeroMemory(pd3dsdTexture,sizeof(*pd3dsdTexture));
			hr = pd3d9Texture->GetLevelDesc(0,pd3dsdTexture);
			if(D3D_OK != hr)
			{
				//WSE_ERROR_TRACE("CDirect3D9ExDraw::CreateTexture(),GetLevelDesc failed! errorcode = "<<hr);
				return FALSE;
			}
		}

		*ppd3d9Texture = pd3d9Texture.Detach();
		return TRUE;
	}
	else
	{
		HBITMAP bmp = MakeBitmap(pImage,pbmi);
		if(NULL == bmp)
			return FALSE;
		BOOL rt = CreateTextureFromDDB(bmp,clrkey,pd3d9Device,d3d9DeviceCapsRef,ppd3d9Texture,pd3dsdTexture);
		DeleteObject(bmp);
		return rt;
	}
	return FALSE;// impossible arrive here
}
BOOL CDirect3D9ExDraw::CreateTextureFromDDB(HBITMAP hBitmap,COLORREF clrkey,LPDIRECT3DDEVICE9 pd3d9Device,const D3DCAPS9 &d3d9DeviceCapsRef,/*out*/LPDIRECT3DTEXTURE9* ppd3d9Texture,/*out*/D3DSURFACE_DESC* pd3dsdTexture)
{
	if(!pd3d9Device)
		return FALSE;
	
	HRESULT hr;

	BITMAP bm;
	GetObject(hBitmap,sizeof(bm),&bm);
	DWORD dwWidth = bm.bmWidth;
	DWORD dwHeight = bm.bmHeight;

	UINT ulWidth = dwWidth;
	UINT ulHeight = dwHeight;
	CalcTextureSize(d3d9DeviceCapsRef,dwWidth,dwHeight,ulWidth,ulHeight);

	CComPtr<IDirect3DTexture9> pd3d9Texture;
	hr = pd3d9Device->CreateTexture(ulWidth,ulHeight,1,0,D3DFMT_X8R8G8B8,D3DPOOL_DEFAULT,&pd3d9Texture,NULL);
	if(D3D_OK != hr)
	{
		//WSE_ERROR_TRACE("CDirect3D9ExDraw::CreateTextureFromDDB(),CreateTexture failed! errorcode = "<<hr);
		return FALSE;
	}

	CComPtr<IDirect3DTexture9> pd3d9TextureSysMem;
	hr = pd3d9Device->CreateTexture(ulWidth,ulHeight,1,0,D3DFMT_X8R8G8B8,D3DPOOL_SYSTEMMEM,&pd3d9TextureSysMem,NULL);
	if(D3D_OK != hr)
	{
		//WSE_ERROR_TRACE("CDirect3D9ExDraw::CreateTextureFromDDB(),CreateTexture(SYSTEMMEM) failed! errorcode = "<<hr);
		return FALSE;
	}
	
	if(TRUE != FillDataToTexture_DDB2ARGB(hBitmap,clrkey,pd3d9TextureSysMem))
	{
		//WSE_ERROR_TRACE("CDirect3D9ExDraw::CreateTextureFromDDB(),FillDataToTexture_DDB2ARGB failed!");
		return FALSE;
	}
	
	hr = pd3d9Device->UpdateTexture(pd3d9TextureSysMem,pd3d9Texture);
	if(D3D_OK != hr)
	{
		//WSE_ERROR_TRACE("CDirect3D9ExDraw::CreateTextureFromDDB(),UpdateTexture() failed! errorcode = "<<hr);
		return FALSE;
	}
		
	if(pd3dsdTexture)
	{// fill d3dsd
		ZeroMemory(pd3dsdTexture,sizeof(*pd3dsdTexture));
		hr = pd3d9Texture->GetLevelDesc(0,pd3dsdTexture);
		if(D3D_OK != hr)
		{
			//WSE_ERROR_TRACE("CDirect3D9ExDraw::CreateTextureFromDDB(),GetLevelDesc failed! errorcode = "<<hr);
			return FALSE;
		}
	}

	*ppd3d9Texture = pd3d9Texture.Detach();
	return TRUE;
}
BOOL CDirect3D9ExDraw::CreateTextureFromHBITMAP(HBITMAP hBitmap,COLORREF clrkey,LPDIRECT3DDEVICE9 pd3d9Device,const D3DCAPS9 &d3d9DeviceCapsRef,/*out*/LPDIRECT3DTEXTURE9* ppd3d9Texture,/*out*/D3DSURFACE_DESC* pd3dsdTexture)
{
	DIBSECTION ds;
	int rt = GetObject(hBitmap,sizeof(ds),&ds);
	if(rt != sizeof(DIBSECTION) || ds.dsBm.bmBitsPixel != 32 || 0xFFFFFFFF != clrkey)
	{// not dibsection OR 32 bitcount OR clrkey valid
		return CreateTextureFromDDB(hBitmap,clrkey,pd3d9Device,d3d9DeviceCapsRef,ppd3d9Texture,pd3dsdTexture);
	}

	BITMAPINFO bi;
	memset(&bi,0,sizeof(bi));
	bi.bmiHeader = ds.dsBmih;
	LPBYTE pImage = GetBits(hBitmap,&bi);
	if(NULL == pImage)
	{
		//WSE_ERROR_TRACE("CDirect3D9ExDraw::CreateTextureFromHBITMAP(),GetBits failed!");
		return FALSE;
	}

	BOOL brt = CreateTexture(pImage,(LPBITMAPINFOHEADER)&bi,clrkey,pd3d9Device,d3d9DeviceCapsRef,ppd3d9Texture,pd3dsdTexture);
	if(FALSE == brt)
	{
		//WSE_ERROR_TRACE("CDirect3D9ExDraw::CreateTextureFromHBITMAP(),CreateTexture failed!");
	}
	delete []pImage;
	return brt;	
}
BOOL CDirect3D9ExDraw::CreateTextureByDesc(PBITMAP_DESC pbd,COLORREF clrkey,LPDIRECT3DDEVICE9 pd3d9Device,const D3DCAPS9 &d3d9DeviceCapsRef,/*out*/LPDIRECT3DTEXTURE9* ppd3d9Texture,/*out*/LPDWORD pdwBitmapWidth,/*out*/LPDWORD pdwBitmapHeight,/*out*/D3DSURFACE_DESC* pd3dsdTexture)
{
	switch(pbd->bmptype)
	{
	case BT_HBITMAP:
		{
			BITMAP bm;
			GetObject((HBITMAP)pbd->bitmap,sizeof(bm),&bm);
			if(pdwBitmapWidth) *pdwBitmapWidth = bm.bmWidth;
			if(pdwBitmapHeight) *pdwBitmapHeight = bm.bmHeight;
			return CreateTextureFromHBITMAP((HBITMAP)pbd->bitmap,clrkey,pd3d9Device,d3d9DeviceCapsRef,ppd3d9Texture,pd3dsdTexture);
		}
		break;
	case BT_DIBGMO:
		{
			LPVOID pData = GlobalLock((HGLOBAL)pbd->bitmap);
			if(pData)
			{
				LPBITMAPINFOHEADER lpbi = (LPBITMAPINFOHEADER)pData;
				WORD wBitCount = lpbi->biBitCount;
				int nColors = (1 << wBitCount);
				if( nColors > 256 )
					nColors = 0;
				if(wBitCount >= 32)
					nColors = 0;
				DWORD dwPaletteSize  = nColors * sizeof(RGBQUAD);
				LPBYTE pImageData = (LPBYTE)pData + sizeof(BITMAPINFOHEADER)+dwPaletteSize;
				
				if(pdwBitmapWidth) *pdwBitmapWidth = lpbi->biWidth;
				if(pdwBitmapHeight) *pdwBitmapHeight = abs(lpbi->biHeight);
				BOOL bret = CreateTexture(pImageData,lpbi,clrkey,pd3d9Device,d3d9DeviceCapsRef,ppd3d9Texture,pd3dsdTexture);
				GlobalUnlock((HGLOBAL)pbd->bitmap);
				return bret;
			}
			else
			{
				//WSE_ERROR_TRACE("CDirect3D9ExDraw::CreateTextureByDesc(), GlobalLock failed! errorcode = "<<GetLastError());
			}
		}
		break;
	case BT_DIB:
		{
			LPBITMAPINFOHEADER lpbi = (LPBITMAPINFOHEADER)pbd->bitmap;
			WORD wBitCount = lpbi->biBitCount;
			int nColors = (1 << wBitCount);
			if( nColors > 256 )
				nColors = 0;
			if(wBitCount >= 32)
				nColors = 0;
			DWORD dwPaletteSize  = nColors * sizeof(RGBQUAD);
			LPBYTE pImageData = (LPBYTE)pbd->bitmap + sizeof(BITMAPINFOHEADER)+dwPaletteSize;
			
			if(pdwBitmapWidth) *pdwBitmapWidth = lpbi->biWidth;
			if(pdwBitmapHeight) *pdwBitmapHeight = abs(lpbi->biHeight);
			return CreateTexture(pImageData,lpbi,clrkey,pd3d9Device,d3d9DeviceCapsRef,ppd3d9Texture,pd3dsdTexture);
		}
		break;
	default:
		//WSE_ERROR_TRACE("CDirect3D9ExDraw::CreateTextureByDesc(), unknown pbd->bmptype! type = "<<pbd->bmptype);
		break;
	}
	return FALSE;
}
inline BOOL CDirect3D9ExDraw::PrepareTextures(int mode)
{
	ID2BPMap::iterator it = m_mapID2BP.begin();
	for(;it!=m_mapID2BP.end();++it)
	{
		BOOL bSkip = FALSE;
		switch(mode)
		{
		case 1:
			if(!IS_ACTONSRC(it->second.bd.dwMask))
				bSkip = TRUE;
			break;
		case 2:
			if(IS_ACTONSRC(it->second.bd.dwMask))
				bSkip = TRUE;
			break;
		}
		if(TRUE == bSkip)
			continue;

		if(it->second.pRef == NULL)
		{
			LPDIRECT3DTEXTURE9 pd3d9Texture = NULL;
			DWORD dwBitmapWidth = 0;
			DWORD dwBitmapHeight = 0;
			D3DSURFACE_DESC d3dsdTexture;
			
			BITMAP_DESC bitmapdesc;
			bitmapdesc.bmptype = it->second.bd.bmptype;
			bitmapdesc.bitmap = it->second.bd.bitmap;
			if(TRUE == CreateTextureByDesc(&bitmapdesc,it->second.bd.clrkey,m_pD3D9DeviceWrapper->GetDirect3DDevice9(),m_d3d9ExDeviceCaps,&pd3d9Texture,&dwBitmapWidth,&dwBitmapHeight,&d3dsdTexture))
			{
				PD3D9_TEXTURE_PARAM pD3d9Param = new D3D9_TEXTURE_PARAM;
				memset(pD3d9Param,0,sizeof(D3D9_TEXTURE_PARAM));
				pD3d9Param->pd3d9Texture = pd3d9Texture;
				pD3d9Param->dwBitmapWidth = dwBitmapWidth;
				pD3d9Param->dwBitmapHeight = dwBitmapHeight;
				pD3d9Param->d3dsdTexture = d3dsdTexture;
				it->second.pRef = (LPVOID)pD3d9Param;
			}
			else
			{
				//WSE_ERROR_TRACE("CDirect3D9ExDraw::PrepareTextures(), CreateTextureByDesc failed!");
				return FALSE;
			}
		}
	}
	return TRUE;
}
BOOL CDirect3D9ExDraw::FillDataToTexture_ARGB2ARGB(LPBYTE pImage,LPBITMAPINFOHEADER pbmi,LPDIRECT3DTEXTURE9 pd3d9Texture)
{
	HRESULT hr = E_FAIL;
	
	if(pbmi->biBitCount != 32)
		return FALSE;// source image was not 32 bit count

	D3DLOCKED_RECT dr;
	hr = pd3d9Texture->LockRect(0,&dr,NULL,0);
	if(D3D_OK != hr)
	{
		//WSE_ERROR_TRACE("CDirect3D9ExDraw::FillDataToTexture_ARGB2ARGB(),LockRect failed! errorcode = "<<hr);
		return FALSE;
	}
	
	DWORD dwWidth = pbmi->biWidth;
	DWORD dwHeight = abs(pbmi->biHeight);

	DWORD* pDDSColor = (DWORD*) dr.pBits;
	DWORD dwLineBytes = dwWidth*32/8;// 32bitcount
	DWORD* pImageColor = NULL;
	if(pbmi->biHeight < 0)
		pImageColor = (DWORD*)pImage;
	else
		pImageColor = (DWORD*)(pImage + dwLineBytes*(dwHeight-1));
	for( DWORD iY = 0; iY < dwHeight; iY++ )
	{
		memcpy(pDDSColor,pImageColor,(DWORD)dr.Pitch<dwLineBytes?dr.Pitch:dwLineBytes); //confirmed_safe_unsafe_usage
		
		if(pbmi->biHeight < 0)
			pImageColor = (DWORD*)(pImage + (iY + 1 ) * dwLineBytes);
		else
			pImageColor = (DWORD*)(pImage + (dwHeight-iY-2) * dwLineBytes);
		pDDSColor = (DWORD*) ( (BYTE*) dr.pBits + ( iY + 1 ) * dr.Pitch );
	}
	pd3d9Texture->UnlockRect(0);
	return TRUE;
}
BOOL CDirect3D9ExDraw::FillDataToTexture_DDB2ARGB(HBITMAP hBitmap,COLORREF clrkey,LPDIRECT3DTEXTURE9 pd3d9Texture)
{
	HRESULT hr = E_FAIL;
	
	BOOL bHasKey = (clrkey != 0xFFFFFFFF);
	
	BITMAP bm;
	GetObject(hBitmap,sizeof(bm),&bm);
	DWORD dwWidth = bm.bmWidth;
	DWORD dwHeight = bm.bmHeight;
	
	D3DLOCKED_RECT dr;
	hr = pd3d9Texture->LockRect(0,&dr,NULL,0);
	if(D3D_OK != hr)
	{
		//WSE_ERROR_TRACE("CDirect3D9ExDraw::FillDataToTexture_DDB2ARGB(),LockRect failed! errorcode = "<<hr);
		return FALSE;
	}
	
	DWORD* pDDSColor = (DWORD*)dr.pBits;
	
	HDC hMemDC = CreateCompatibleDC(NULL);
	if(NULL == hMemDC)
	{
		//WSE_ERROR_TRACE("CDirect3D9ExDraw::FillDataToTexture_DDB2ARGB(),CreateCompatibleDC failed.errorcode = "<<GetLastError());
		pd3d9Texture->UnlockRect(0);
		return FALSE;
	}
	HBITMAP old = (HBITMAP)SelectObject(hMemDC,hBitmap);
	for( DWORD iY = 0; iY < dwHeight; iY++ )
	{
		for( DWORD iX = 0; iX < dwWidth; iX++ )
		{
			COLORREF clr = GetPixel(hMemDC,iX,iY);
			if(TRUE == bHasKey && clr == clrkey)
				*pDDSColor = D3DCOLOR_RGBA(0,0,0,0);
			else
			{
				BYTE r = GetRValue(clr);
				BYTE g = GetGValue(clr);
				BYTE b = GetBValue(clr);
				*pDDSColor = D3DCOLOR_RGBA(r,g,b,0xff);
			}
			pDDSColor++;
		}
		pDDSColor = (DWORD*) ( (BYTE*) dr.pBits + ( iY + 1 ) * dr.Pitch );
	}
	SelectObject(hMemDC,old);
	DeleteDC(hMemDC);
	
	pd3d9Texture->UnlockRect(0);
	return TRUE;
}
inline HRESULT CDirect3D9ExDraw::DoBlt(LPDIRECT3DDEVICE9 pd3d9Device,RECT* lpDst,LPDIRECT3DTEXTURE9 pd3d9Texture,const D3DSURFACE_DESC &d3dsdTexture,RECT* lpSrc,BYTE bAlpha,BOOL bNearestInterpolation)
{
	HRESULT hr = E_FAIL;

	if(bAlpha == 0xff)
	{
		struct std3d9Vertex{
			float x, y, z, rhw;
			float tu, tv;
		} pVertices[4];
		float fWid = (float)d3dsdTexture.Width;
		float fHgt = (float)d3dsdTexture.Height;
		
		pVertices[0].x = lpDst->left-0.5f;
		pVertices[0].y = lpDst->top-0.5f;
		pVertices[0].z = 0.5f;
		pVertices[0].rhw = 2.0f;
		
		pVertices[1].x = lpDst->right-0.5f;
		pVertices[1].y = lpDst->top-0.5f;
		pVertices[1].z = 0.5f;
		pVertices[1].rhw = 2.0f;
		
		pVertices[2].x = lpDst->left-0.5f;
		pVertices[2].y = lpDst->bottom-0.5f;
		pVertices[2].z = 0.5f;
		pVertices[2].rhw = 2.0f;
		
		pVertices[3].x = lpDst->right-0.5f;
		pVertices[3].y = lpDst->bottom-0.5f;
		pVertices[3].z = 0.5f;
		pVertices[3].rhw = 2.0f;
		
		pVertices[0].tu = (float)(lpSrc->left) / fWid;
		pVertices[0].tv = (float)(lpSrc->top) / fHgt;
		
		pVertices[1].tu = (float)(lpSrc->right) / fWid;
		pVertices[1].tv = (float)(lpSrc->top) / fHgt;
		
		pVertices[2].tu = (float)(lpSrc->left) / fWid;
		pVertices[2].tv = (float)(lpSrc->bottom) / fHgt;
		
		pVertices[3].tu = (float)(lpSrc->right) / fWid;
		pVertices[3].tv = (float)(lpSrc->bottom) / fHgt;
		
		hr = pd3d9Device->SetTexture(0,pd3d9Texture);
		hr = pd3d9Device->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE);
		if(bNearestInterpolation)
		{
			hr = pd3d9Device->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_POINT);
			hr = pd3d9Device->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_POINT);
		}
		else
		{
			hr = pd3d9Device->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_LINEAR);
			hr = pd3d9Device->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_LINEAR);	
		}
		hr = pd3d9Device->SetSamplerState(0,D3DSAMP_MIPFILTER,D3DTEXF_NONE);
		do 
		{
			hr = pd3d9Device->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);
			if(D3D_OK != hr)
			{
				//WSE_ERROR_TRACE("CDirect3D9ExDraw::DoBlt(),SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1) failed! errorcode = "<<hr);
				break;
			}
			hr = pd3d9Device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,pVertices,sizeof(std3d9Vertex));
			if(D3D_OK != hr)
			{
				//WSE_ERROR_TRACE("CDirect3D9ExDraw::DoBlt(),DrawPrimitive failed! errorcode = "<<hr);
				break;
			}
		}while(0);
		pd3d9Device->SetTexture(0,NULL);
	}
	else
	{
		struct std3d9Vertex{
			float x, y, z, rhw;
			D3DCOLOR clr;
			float tu, tv;
		} pVertices[4];
		float fWid = (float)d3dsdTexture.Width;
		float fHgt = (float)d3dsdTexture.Height;
		
		pVertices[0].x = lpDst->left-0.5f;
		pVertices[0].y = lpDst->top-0.5f;
		pVertices[0].z = 0.5f;
		pVertices[0].rhw = 2.0f;
		pVertices[0].clr = D3DCOLOR_RGBA(bAlpha, bAlpha, bAlpha, bAlpha);
		
		pVertices[1].x = lpDst->right-0.5f;
		pVertices[1].y = lpDst->top-0.5f;
		pVertices[1].z = 0.5f;
		pVertices[1].rhw = 2.0f;
		pVertices[1].clr = D3DCOLOR_RGBA(bAlpha, bAlpha, bAlpha, bAlpha);
		
		pVertices[2].x = lpDst->left-0.5f;
		pVertices[2].y = lpDst->bottom-0.5f;
		pVertices[2].z = 0.5f;
		pVertices[2].rhw = 2.0f;
		pVertices[2].clr = D3DCOLOR_RGBA(bAlpha, bAlpha, bAlpha, bAlpha);
		
		pVertices[3].x = lpDst->right-0.5f;
		pVertices[3].y = lpDst->bottom-0.5f;
		pVertices[3].z = 0.5f;
		pVertices[3].rhw = 2.0f;
		pVertices[3].clr = D3DCOLOR_RGBA(bAlpha, bAlpha, bAlpha, bAlpha);
		
		pVertices[0].tu = (float)(lpSrc->left) / fWid;
		pVertices[0].tv = (float)(lpSrc->top) / fHgt;
		
		pVertices[1].tu = (float)(lpSrc->right) / fWid;
		pVertices[1].tv = (float)(lpSrc->top) / fHgt;
		
		pVertices[2].tu = (float)(lpSrc->left) / fWid;
		pVertices[2].tv = (float)(lpSrc->bottom) / fHgt;
		
		pVertices[3].tu = (float)(lpSrc->right) / fWid;
		pVertices[3].tv = (float)(lpSrc->bottom) / fHgt;
		
		hr = pd3d9Device->SetTexture(0,pd3d9Texture);
		hr = pd3d9Device->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
		hr = pd3d9Device->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_ONE); // process pargb
//		hr = pd3d9Device->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA); // process pargb
		hr = pd3d9Device->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);
		if(bNearestInterpolation)
		{
			hr = pd3d9Device->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_POINT);
			hr = pd3d9Device->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_POINT);
		}
		else
		{
			hr = pd3d9Device->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_LINEAR);
			hr = pd3d9Device->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_LINEAR);
		}
		hr = pd3d9Device->SetSamplerState(0,D3DSAMP_MIPFILTER,D3DTEXF_NONE);
		hr = pd3d9Device->SetTextureStageState(0,D3DTSS_ALPHAARG1,D3DTA_TEXTURE);
		hr = pd3d9Device->SetTextureStageState(0,D3DTSS_ALPHAARG2,D3DTA_DIFFUSE);
		hr = pd3d9Device->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_MODULATE);
		do 
		{
			hr = pd3d9Device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);
			if(D3D_OK != hr)
			{
				//WSE_ERROR_TRACE("CDirect3D9ExDraw::DoBlt(),SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1) failed! errorcode = "<<hr);
				break;
			}
			hr = pd3d9Device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,pVertices,sizeof(std3d9Vertex));
			if(D3D_OK != hr)
			{
				//WSE_ERROR_TRACE("CDirect3D9ExDraw::DoBlt(),DrawPrimitiveUP failed! errorcode = "<<hr);
				break;
			}
		}while(0);
		pd3d9Device->SetTexture(0,NULL);
	}
	return hr;
}
inline HRESULT CDirect3D9ExDraw::AlphaBlt(LPDIRECT3DDEVICE9 pd3d9Device,RECT* lpDst,LPDIRECT3DTEXTURE9 pd3d9Texture,const D3DSURFACE_DESC &d3dsdTexture,RECT* lpSrc,BYTE bAlpha,BOOL bNearestInterpolation)
{
	HRESULT hr = E_FAIL;

	if(bAlpha == 0xff)
	{
		struct std3d9Vertex{
			float x, y, z, rhw;
			float tu, tv;
		} pVertices[4];
		float fWid = (float)d3dsdTexture.Width;
		float fHgt = (float)d3dsdTexture.Height;
		
		pVertices[0].x = lpDst->left-0.5f;
		pVertices[0].y = lpDst->top-0.5f;
		pVertices[0].z = 0.5f;
		pVertices[0].rhw = 2.0f;
		
		pVertices[1].x = lpDst->right-0.5f;
		pVertices[1].y = lpDst->top-0.5f;
		pVertices[1].z = 0.5f;
		pVertices[1].rhw = 2.0f;
		
		pVertices[2].x = lpDst->left-0.5f;
		pVertices[2].y = lpDst->bottom-0.5f;
		pVertices[2].z = 0.5f;
		pVertices[2].rhw = 2.0f;
		
		pVertices[3].x = lpDst->right-0.5f;
		pVertices[3].y = lpDst->bottom-0.5f;
		pVertices[3].z = 0.5f;
		pVertices[3].rhw = 2.0f;
		
		pVertices[0].tu = (float)(lpSrc->left) / fWid;
		pVertices[0].tv = (float)(lpSrc->top) / fHgt;
		
		pVertices[1].tu = (float)(lpSrc->right) / fWid;
		pVertices[1].tv = (float)(lpSrc->top) / fHgt;
		
		pVertices[2].tu = (float)(lpSrc->left) / fWid;
		pVertices[2].tv = (float)(lpSrc->bottom) / fHgt;
		
		pVertices[3].tu = (float)(lpSrc->right) / fWid;
		pVertices[3].tv = (float)(lpSrc->bottom) / fHgt;
		
		hr = pd3d9Device->SetTexture(0,pd3d9Texture);
		hr = pd3d9Device->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
//		hr = pd3d9Device->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_ONE); // process pargb
		hr = pd3d9Device->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA); // process pargb
		hr = pd3d9Device->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);
		if(bNearestInterpolation)
		{
			hr = pd3d9Device->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_POINT);
			hr = pd3d9Device->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_POINT);
		}
		else
		{
			hr = pd3d9Device->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_LINEAR);
			hr = pd3d9Device->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_LINEAR);
		}
		hr = pd3d9Device->SetSamplerState(0,D3DSAMP_MIPFILTER,D3DTEXF_NONE);
		hr = pd3d9Device->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_SELECTARG1);
		do 
		{
			hr = pd3d9Device->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);
			if(D3D_OK != hr)
			{
				//WSE_ERROR_TRACE("CDirect3D9ExDraw::AlphaBlt(),SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1) failed! errorcode = "<<hr);
				break;
			}
			hr = pd3d9Device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,pVertices,sizeof(std3d9Vertex));
			if(D3D_OK != hr)
			{
				//WSE_ERROR_TRACE("CDirect3D9ExDraw::AlphaBlt(),DrawPrimitiveUP failed! errorcode = "<<hr);
				break;
			}
		}while(0);
		pd3d9Device->SetTexture(0,NULL);
	}
	else
	{
		struct std3d9Vertex{
			float x, y, z, rhw;
			D3DCOLOR clr;
			float tu, tv;
		} pVertices[4];
		float fWid = (float)d3dsdTexture.Width;
		float fHgt = (float)d3dsdTexture.Height;
		
		pVertices[0].x = lpDst->left-0.5f;
		pVertices[0].y = lpDst->top-0.5f;
		pVertices[0].z = 0.5f;
		pVertices[0].rhw = 2.0f;
		pVertices[0].clr = D3DCOLOR_RGBA(bAlpha, bAlpha, bAlpha, bAlpha);
		
		pVertices[1].x = lpDst->right-0.5f;
		pVertices[1].y = lpDst->top-0.5f;
		pVertices[1].z = 0.5f;
		pVertices[1].rhw = 2.0f;
		pVertices[1].clr = D3DCOLOR_RGBA(bAlpha, bAlpha, bAlpha, bAlpha);
		
		pVertices[2].x = lpDst->left-0.5f;
		pVertices[2].y = lpDst->bottom-0.5f;
		pVertices[2].z = 0.5f;
		pVertices[2].rhw = 2.0f;
		pVertices[2].clr = D3DCOLOR_RGBA(bAlpha, bAlpha, bAlpha, bAlpha);
		
		pVertices[3].x = lpDst->right-0.5f;
		pVertices[3].y = lpDst->bottom-0.5f;
		pVertices[3].z = 0.5f;
		pVertices[3].rhw = 2.0f;
		pVertices[3].clr = D3DCOLOR_RGBA(bAlpha, bAlpha, bAlpha, bAlpha);
		
		pVertices[0].tu = (float)(lpSrc->left) / fWid;
		pVertices[0].tv = (float)(lpSrc->top) / fHgt;
		
		pVertices[1].tu = (float)(lpSrc->right) / fWid;
		pVertices[1].tv = (float)(lpSrc->top) / fHgt;
		
		pVertices[2].tu = (float)(lpSrc->left) / fWid;
		pVertices[2].tv = (float)(lpSrc->bottom) / fHgt;
		
		pVertices[3].tu = (float)(lpSrc->right) / fWid;
		pVertices[3].tv = (float)(lpSrc->bottom) / fHgt;
		
		hr = pd3d9Device->SetTexture(0,pd3d9Texture);
		hr = pd3d9Device->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
		hr = pd3d9Device->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_ONE); // process pargb
//		hr = pd3d9Device->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA); // process pargb
		hr = pd3d9Device->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);
		if(bNearestInterpolation)
		{
			hr = pd3d9Device->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_POINT);
			hr = pd3d9Device->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_POINT);
		}
		else
		{
			hr = pd3d9Device->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_LINEAR);
			hr = pd3d9Device->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_LINEAR);
		}
		hr = pd3d9Device->SetSamplerState(0,D3DSAMP_MIPFILTER,D3DTEXF_NONE);
		hr = pd3d9Device->SetTextureStageState(0,D3DTSS_ALPHAARG1,D3DTA_TEXTURE);
		hr = pd3d9Device->SetTextureStageState(0,D3DTSS_ALPHAARG2,D3DTA_DIFFUSE);
		hr = pd3d9Device->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_MODULATE);
		do 
		{
			hr = pd3d9Device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);
			if(D3D_OK != hr)
			{
				//WSE_ERROR_TRACE("CDirect3D9ExDraw::AlphaBlt(),SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1) failed! errorcode = "<<hr);
				break;
			}
			hr = pd3d9Device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,pVertices,sizeof(std3d9Vertex));
			if(D3D_OK != hr)
			{
				//WSE_ERROR_TRACE("CDirect3D9ExDraw::AlphaBlt(),DrawPrimitiveUP failed! errorcode = "<<hr);
				break;
			}
		}while(0);
		pd3d9Device->SetTexture(0,NULL);
	}
	return hr;
}
inline HRESULT CDirect3D9ExDraw::Do_Effect(LPDIRECT3DSURFACE9 pSurfaceDrawTo,DWORD dwWidth,DWORD dwHeight,BOOL bActOnSrc)
{
	HRESULT hr;

	RECT rcDrawTo;
	SetRect(&rcDrawTo,0,0,dwWidth,dwHeight);

	hr = m_pD3D9DeviceWrapper->SetRenderTarget(0,pSurfaceDrawTo);
	if(D3D_OK != hr)
	{
		//WSE_ERROR_TRACE("CDirect3D9ExDraw::Do_Effect(), SetRenderTarget failed! errorcode = "<<hr);
		return hr;
	}

	hr = m_pD3D9DeviceWrapper->BeginScene();
	if(D3D_OK != hr)
	{
		//WSE_ERROR_TRACE("CDirect3D9ExDraw::Do_Effect(),BeginScene failed! errorcode = "<<hr);
		return hr;
	}

	ID2BPMap::iterator it = m_mapID2BP.begin();
	for(;it!=m_mapID2BP.end();++it)
	{
		if((IS_ACTONSRC(it->second.bd.dwMask)?TRUE:FALSE) == bActOnSrc && it->second.pRef != NULL)
		{
			PD3D9_TEXTURE_PARAM pParam = (PD3D9_TEXTURE_PARAM)it->second.pRef;
			LPDIRECT3DTEXTURE9 pd3d9Texture = pParam->pd3d9Texture;
			DWORD dwBitmapWidth = pParam->dwBitmapWidth;
			DWORD dwBitmapHeight = pParam->dwBitmapHeight;
			D3DSURFACE_DESC &d3dsdTexture = pParam->d3dsdTexture;

			RECT rcTarget;
			CalcBitmapDrawToRect(&rcDrawTo,&it->second.bd,dwBitmapWidth,dwBitmapHeight,&rcTarget);
			
			RECT rcSrc;
			SetRect(&rcSrc,0,0,dwBitmapWidth,dwBitmapHeight);
			
			if(d3dsdTexture.Format == D3DFMT_A8R8G8B8)
			{
				hr = AlphaBlt(m_pD3D9DeviceWrapper->GetDirect3DDevice9(),&rcTarget,pd3d9Texture,d3dsdTexture,&rcSrc,0xff);
				if(D3D_OK != hr)
					break;
			}
			else
			{
				hr = DoBlt(m_pD3D9DeviceWrapper->GetDirect3DDevice9(),&rcTarget,pd3d9Texture,d3dsdTexture,&rcSrc,0xff);
				if(D3D_OK != hr)
					break;
			}
		}
	}
	
	HRESULT hrEnd = m_pD3D9DeviceWrapper->EndScene();
	if(D3D_OK != hrEnd)
	{
		//WSE_ERROR_TRACE("CDirect3D9ExDraw::Do_Effect(),EndScene failed! errorcode = "<<hrEnd);
		if(D3D_OK == hr)
			return hrEnd;
	}
	return hr;
}
BOOL CDirect3D9ExDraw::CreateSurfaceByDesc(PBITMAP_DESC pbd,IDirect3D9DeviceWrapper* pD3D9DeviceWrapper,const D3DCAPS9 &d3d9DeviceCapsRef,/*out*/LPDIRECT3DSURFACE9* ppd3d9Surface,/*out*/D3DSURFACE_DESC* pd3dsdSurface)
{
	switch(pbd->bmptype)
	{
	case BT_HBITMAP:
		{
			BITMAP bm;
			GetObject((HBITMAP)pbd->bitmap,sizeof(bm),&bm);
			return CreateSurfaceFromHBITMAP((HBITMAP)pbd->bitmap,pD3D9DeviceWrapper,d3d9DeviceCapsRef,ppd3d9Surface,pd3dsdSurface);
		}
		break;
	case BT_DIBGMO:
		{
			LPVOID pData = GlobalLock((HGLOBAL)pbd->bitmap);
			if(pData)
			{
				LPBITMAPINFOHEADER lpbi = (LPBITMAPINFOHEADER)pData;
				WORD wBitCount = lpbi->biBitCount;
				int nColors = (1 << wBitCount);
				if( nColors > 256 )
					nColors = 0;
				if(wBitCount >= 32)
					nColors = 0;
				DWORD dwPaletteSize  = nColors * sizeof(RGBQUAD);
				LPBYTE pImageData = (LPBYTE)pData + sizeof(BITMAPINFOHEADER)+dwPaletteSize;

				BOOL bret;
				if(wBitCount == 32)
					bret = CreateSurface_XRGB(pImageData,lpbi,m_pD3D9DeviceWrapper,d3d9DeviceCapsRef,ppd3d9Surface,pd3dsdSurface);
				else
				{
					HBITMAP bmp = MakeBitmap(pImageData,lpbi);
					if(NULL == bmp)
					{
						//WSE_ERROR_TRACE("CDirect3D9ExDraw::CreateSurfaceByDesc(), MakeBitmap failed!");
						bret = FALSE;
					}
					else
					{
						bret = CreateSurfaceFromHBITMAP(bmp,pD3D9DeviceWrapper,d3d9DeviceCapsRef,ppd3d9Surface,pd3dsdSurface);
						DeleteObject(bmp);
					}
				}
				GlobalUnlock((HGLOBAL)pbd->bitmap);
				return bret;
			}
			//else
				//WSE_ERROR_TRACE("CDirect3D9ExDraw::CreateSurfaceByDesc(), GlobalLock failed! errorcode = "<<GetLastError());
		}
		break;
	case BT_DIB:
		{
			LPBITMAPINFOHEADER lpbi = (LPBITMAPINFOHEADER)pbd->bitmap;
			WORD wBitCount = lpbi->biBitCount;
			int nColors = (1 << wBitCount);
			if( nColors > 256 )
				nColors = 0;
			if(wBitCount >= 32)
				nColors = 0;
			DWORD dwPaletteSize  = nColors * sizeof(RGBQUAD);
			LPBYTE pImageData = (LPBYTE)pbd->bitmap + sizeof(BITMAPINFOHEADER)+dwPaletteSize;
			
			BOOL bret;
			if(wBitCount == 32)
				bret = CreateSurface_XRGB(pImageData,lpbi,pD3D9DeviceWrapper,d3d9DeviceCapsRef,ppd3d9Surface,pd3dsdSurface);
			else
			{
				HBITMAP bmp = MakeBitmap(pImageData,lpbi);
				if(NULL == bmp)
				{
					//WSE_ERROR_TRACE("CDirect3D9ExDraw::CreateSurfaceByDesc(), MakeBitmap failed!");
					bret = FALSE;
				}
				else
				{
					bret = CreateSurfaceFromHBITMAP(bmp,pD3D9DeviceWrapper,d3d9DeviceCapsRef,ppd3d9Surface,pd3dsdSurface);
					DeleteObject(bmp);
				}
			}
			return bret;
		}
		break;
	default:
		//WSE_ERROR_TRACE("CDirect3D9ExDraw::CreateSurfaceByDesc(), unknown pbd->bmptype! type = "<<pbd->bmptype);
		break;
	}
	return FALSE;
}
BOOL CDirect3D9ExDraw::CreateSurface_XRGB(LPBYTE pImage,LPBITMAPINFOHEADER pbmi,IDirect3D9DeviceWrapper* pD3D9DeviceWrapper,const D3DCAPS9 &d3d9DeviceCapsRef,/*out*/LPDIRECT3DSURFACE9* ppd3d9Surface,/*out*/D3DSURFACE_DESC* pd3dsdSurface)
{
	if(!pD3D9DeviceWrapper)
		return FALSE;
	if(pbmi->biBitCount != 32)
		return FALSE;

	// xrgb
	DWORD dwWidth = pbmi->biWidth;
	DWORD dwHeight = abs(pbmi->biHeight);

	HRESULT hr;

	CComPtr<IDirect3DSurface9> pd3d9Surface;
	hr = pD3D9DeviceWrapper->CreateOffscreenPlainSurfaceEx(dwWidth,
		dwHeight,
		D3DFMT_X8R8G8B8,
		D3DPOOL_DEFAULT,
		&pd3d9Surface,
		NULL,0);
	if(D3D_OK != hr)
	{
		//WSE_ERROR_TRACE("CDirect3D9ExDraw::CreateSurface_XRGB(),CreateOffscreenPlainSurfaceEx failed! errorcode = "<<hr);
		return FALSE;
	}
	
	if(TRUE != FillDataToSurface_XRGB2XRGB(pImage,pbmi,pd3d9Surface))
	{
		//WSE_ERROR_TRACE("CDirect3D9ExDraw::CreateSurface_XRGB(),FillDataToSurface_XRGB2XRGB failed!");
		return FALSE;
	}
	
	if(pd3dsdSurface)
	{// fill d3dsd
		ZeroMemory(pd3dsdSurface,sizeof(*pd3dsdSurface));
		hr = pd3d9Surface->GetDesc(pd3dsdSurface);
		if(D3D_OK != hr)
		{
			//WSE_ERROR_TRACE("CDirect3D9ExDraw::CreateSurface_XRGB(),GetDesc failed! errorcode = "<<hr);
			return FALSE;
		}
	}
	
	*ppd3d9Surface = pd3d9Surface.Detach();
	return TRUE;
}
BOOL CDirect3D9ExDraw::CreateSurfaceFromHBITMAP(HBITMAP hBitmap,IDirect3D9DeviceWrapper* pD3D9DeviceWrapper,const D3DCAPS9 &d3d9DeviceCapsRef,/*out*/LPDIRECT3DSURFACE9* ppd3d9Surface,/*out*/D3DSURFACE_DESC* pd3dsdSurface)
{
	BITMAP bm;
	GetObject(hBitmap,sizeof(bm),&bm);

	BITMAPINFO bi;
	memset(&bi,0,sizeof(bi));
	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth			= bm.bmWidth;
	bi.bmiHeader.biHeight			= bm.bmHeight;
	bi.bmiHeader.biPlanes			= 1;
	bi.bmiHeader.biBitCount			= 32;
	bi.bmiHeader.biCompression		= BI_RGB;
	LPBYTE pImage = GetBits(hBitmap,&bi);
	if(NULL == pImage)
	{
		//WSE_ERROR_TRACE("CDirect3D9ExDraw::CreateSurfaceFromHBITMAP(),GetBits failed!");
		return FALSE;
	}

	BOOL brt = CreateSurface_XRGB(pImage,(LPBITMAPINFOHEADER)&bi,pD3D9DeviceWrapper,d3d9DeviceCapsRef,ppd3d9Surface,pd3dsdSurface);
	if(FALSE == brt)
	{
		//WSE_ERROR_TRACE("CDirect3D9ExDraw::CreateSurfaceFromHBITMAP(),CreateSurface_XRGB failed!");
	}
	delete []pImage;
	return brt;
}
BOOL CDirect3D9ExDraw::FillDataToSurface_XRGB2XRGB(LPBYTE pImage,LPBITMAPINFOHEADER pbmi,LPDIRECT3DSURFACE9 pd3d9Surface)
{
	HRESULT hr = E_FAIL;
	
	if(pbmi->biBitCount != 32)
		return FALSE;// source image was not 32 bit count

	D3DLOCKED_RECT dr;
	hr = pd3d9Surface->LockRect(&dr,NULL,0);
	if(D3D_OK != hr)
	{
		//WSE_ERROR_TRACE("CDirect3D9ExDraw::FillDataToSurface_XRGB2XRGB(),LockRect failed! errorcode = "<<hr);
		return FALSE;
	}
	
	DWORD dwWidth = pbmi->biWidth;
	DWORD dwHeight = abs(pbmi->biHeight);

	DWORD* pDDSColor = (DWORD*) dr.pBits;
	DWORD dwLineBytes = dwWidth*32/8;// 32bitcount
	DWORD* pImageColor = NULL;
	if(pbmi->biHeight < 0)
		pImageColor = (DWORD*)pImage;
	else
		pImageColor = (DWORD*)(pImage + dwLineBytes*(dwHeight-1));
	for( DWORD iY = 0; iY < dwHeight; iY++ )
	{
		memcpy(pDDSColor,pImageColor,(DWORD)dr.Pitch<dwLineBytes?dr.Pitch:dwLineBytes); //confirmed_safe_unsafe_usage
		
		if(pbmi->biHeight < 0)
			pImageColor = (DWORD*)(pImage + (iY + 1 ) * dwLineBytes);
		else
			pImageColor = (DWORD*)(pImage + (dwHeight-iY-2) * dwLineBytes);
		pDDSColor = (DWORD*) ( (BYTE*) dr.pBits + ( iY + 1 ) * dr.Pitch );
	}
	pd3d9Surface->UnlockRect();
	return TRUE;
}
VA_TYPE CDirect3D9ExDraw::GetSupportedVAType()
{
	if(!m_pD3D9Wrapper->IsSupportedVA())
		return VAT_Unsupported;

	VA_TYPE vatype = VAT_D3D9Ex_ShareHandle_OffscreenPlainSurfaceEx;// default,assume support sharehandle
	if(m_pD3D9DeviceWrapper)
	{
		if(m_d3d9ExDeviceCaps.Caps2 & D3DCAPS2_CANSHARERESOURCE)
			vatype = VAT_D3D9Ex_ShareHandle_OffscreenPlainSurfaceEx;
		else
			vatype = VAT_Unsupported;
	}
	return vatype;
}
BOOL CDirect3D9ExDraw::VADraw(VA_TYPE type,void* pContent,const RECT& rcSource,const RECT& rcTarget)
{
	// set flag that both surface expired
	m_bImageExpired = TRUE;
	m_bRawExpired = TRUE;

	if(StateCode_Invalid == m_stateCode)
		return FALSE;

	BOOL bCanPresent = FALSE;// check whether the image can be displayed
	HMONITOR hmonWnd = m_hmon;
	do 
	{
		if(IsRectEmpty(&rcTarget))
			break;

		RECT rcVSC = rcTarget;
		MapWindowPoints(m_hWnd,NULL,(LPPOINT)&rcVSC,2);
		HMONITOR hmon = (HMONITOR)m_hWnd;//MonitorFromRect(&rcVSC,MONITOR_DEFAULTTONULL);
		if(NULL == hmon)
			break;
		
		hmonWnd = hmon;
		
		// update m_rcTarget
		m_rcTarget = rcTarget;
		bCanPresent = TRUE;
	}
	while(0);

	RECT rcBackBuffer;
	SetRect(&rcBackBuffer,0,0,m_rcTarget.right-m_rcTarget.left,m_rcTarget.bottom-m_rcTarget.top);

	CDirect3D9ExDraw::CheckingCode cc = CheckAndRecoverD3D9ExObjects(hmonWnd,rcBackBuffer.right,rcBackBuffer.bottom);
	switch(cc)
	{
	case CheckingCode_Success:
		break;
	case CheckingCode_StateChange:
		if(StateCode_NeedReset == m_stateCode)
		{
			bCanPresent = FALSE;
			break;// reset does not impact the share surface,so can buffer share data
		}
		return TRUE;
	default:
		return FALSE;
	}

	if(FALSE == bCanPresent)
	{// only buffer
		BOOL bImageLost = FALSE;
		RECT rcImage;
		SetRect(&rcImage,0,0,rcSource.right-rcSource.left,rcSource.bottom-rcSource.top);
		if(FALSE == PrepareImageSurface(rcImage.right,rcImage.bottom,m_d3dpp.BackBufferFormat,bImageLost))
			return FALSE;
		
		if(!PrepareRawSurface(rcImage.right,rcImage.bottom,BI_RGB))
			return FALSE;

		PVAS_D3D9Ex_ShareHandle_OffscreenPlainSurfaceEx pvas = (PVAS_D3D9Ex_ShareHandle_OffscreenPlainSurfaceEx)pContent;
		if(!PrepareSharedSurface((HANDLE)pvas->hSharedHandle,
			(UINT)pvas->uiWidthSH,
			(UINT)pvas->uiHeightSH,
			(D3DFORMAT)pvas->formatSH,
			(D3DPOOL)pvas->poolSH))
			return FALSE;
		
		HRESULT hr;
		hr = m_pD3D9DeviceWrapper->StretchRect(m_lpD3D9SharedSurface,&rcSource,m_lpD3D9RawSurface,NULL,D3DTEXF_NONE);
		if(D3D_OK != hr)
		{
			//WSE_ERROR_TRACE("CDirect3D9ExDraw::VADraw(), StretchRect(Shared->Raw) failed! errorcode = "<<hr);
			return FALSE;
		}
		
		m_bRawExpired = FALSE;// set flag that raw surface available for redraw
		return TRUE;
	}

	// normal process
	CComPtr<IDirect3DSurface9> pBackBuffer;
	if(FALSE == GetBackBuffer(&pBackBuffer))
		return FALSE;

	BOOL bImageLost = FALSE;
	RECT rcImage;
	SetRect(&rcImage,0,0,rcSource.right-rcSource.left,rcSource.bottom-rcSource.top);
	if(FALSE == PrepareImageSurface(rcImage.right,rcImage.bottom,m_d3dpp.BackBufferFormat,bImageLost))
		return FALSE;

	if(!PrepareRawSurface(rcImage.right,rcImage.bottom,BI_RGB))
		return FALSE;

	PVAS_D3D9Ex_ShareHandle_OffscreenPlainSurfaceEx pvas = (PVAS_D3D9Ex_ShareHandle_OffscreenPlainSurfaceEx)pContent;
	if(!PrepareSharedSurface((HANDLE)pvas->hSharedHandle,
		(UINT)pvas->uiWidthSH,
		(UINT)pvas->uiHeightSH,
		(D3DFORMAT)pvas->formatSH,
		(D3DPOOL)pvas->poolSH))
		return FALSE;

	// prepare all textures
	if(!PrepareTextures(0))
		return FALSE;

	RECT rcVideoTarget;
	RECT rcVideoSrc;
	TuneTargetRectangleByAspectRatioMode( rcVideoTarget,rcVideoSrc,rcSource,rcBackBuffer,BI_RGB);
	HRESULT hr;

	hr = m_pD3D9DeviceWrapper->StretchRect(m_lpD3D9SharedSurface,&rcVideoSrc,m_lpD3D9RawSurface,NULL,D3DTEXF_NONE);
	if(D3D_OK != hr)
	{
		//WSE_ERROR_TRACE("CDirect3D9ExDraw::VADraw(), StretchRect(Shared->Raw) failed! errorcode = "<<hr);
		return FALSE;
	}

	m_bRawExpired = FALSE;// set flag that raw surface available for redraw

	hr = m_pD3D9DeviceWrapper->StretchRect(m_lpD3D9RawSurface,NULL,m_lpD3D9ImageSurface,&rcImage,D3DTEXF_NONE);
	if(D3D_OK != hr)
	{
		//WSE_ERROR_TRACE("CDirect3D9ExDraw::VADraw(), StretchRect(Raw->Image) failed! errorcode = "<<hr);
		return FALSE;
	}

	// before Zoom
	if(HasEffectActOnSrc())
	{
		hr = Do_Effect(m_lpD3D9ImageSurface,rcImage.right,rcImage.bottom,TRUE);
		if(D3D_OK != hr)
		{
			//WSE_ERROR_TRACE("CDirect3D9ExDraw::VADraw(), Do_Effect_OnSrc failed! errorcode = "<<hr);
			return FALSE;
		}
	}

	m_bImageExpired = FALSE;// set flag that image surface available for redraw


	if(FALSE == EqualRect(&rcVideoTarget,&rcBackBuffer))
	{// need fill bkgnd firstly
		PrepareBkgnd();
		FillBkgnd(pBackBuffer,&rcBackBuffer);
	}

	if(FALSE == Do_Image2BackBuffer(rcImage,pBackBuffer,rcVideoTarget))
	{
		//WSE_ERROR_TRACE("CDirect3D9ExDraw::VADraw(), Do_Image2BackBuffer failed!");
		return FALSE;
	}

	// after Zoom
	if(HasEffectActOnDest())
	{
		hr = Do_Effect(pBackBuffer,rcBackBuffer.right,rcBackBuffer.bottom,FALSE);
		if(D3D_OK != hr)
		{
			//WSE_ERROR_TRACE("CDirect3D9ExDraw::VADraw(), Do_Effect_OnDest failed! errorcode = "<<hr);
			return FALSE;
		}
	}

	cc = Present(&rcBackBuffer,&m_rcTarget);
	if(CheckingCode_Error == cc)
		return FALSE;
	return TRUE;
}
BOOL CDirect3D9ExDraw::PrepareSharedSurface(HANDLE hSharedHandle,UINT Width,UINT Height,D3DFORMAT Format,D3DPOOL Pool)
{
	HRESULT hr;
	BOOL bNeedCreate = FALSE;
	if(m_lpD3D9SharedSurface)
	{
		if(m_hSharedHandle != hSharedHandle)
		{
			//WSE_INFO_TRACE("CDirect3D9ExDraw::PrepareSharedSurface(), hSharedHandle change! ("<<m_hSharedHandle<<")->("<<hSharedHandle<<")");
			bNeedCreate = TRUE;
		}
		if(m_d3dsdShared.Width != Width || m_d3dsdShared.Height != Height)
		{
			//WSE_INFO_TRACE("CDirect3D9ExDraw::PrepareSharedSurface(), SharedSurface size change! ("<<m_d3dsdShared.Width<<","<<m_d3dsdShared.Height<<")->("<<Width<<","<<Height<<")");
			bNeedCreate = TRUE;
		}
		if(m_d3dsdShared.Format != Format)
		{
			//WSE_INFO_TRACE("CDirect3D9ExDraw::PrepareSharedSurface(), SharedSurface format change! ("<<m_d3dsdShared.Format<<")->("<<Format<<")");
			bNeedCreate = TRUE;
		}
		if(m_d3dsdShared.Pool != Pool)
		{
			//WSE_INFO_TRACE("CDirect3D9ExDraw::PrepareSharedSurface(), SharedSurface pool change! ("<<m_d3dsdShared.Pool<<")->("<<Pool<<")");
			bNeedCreate = TRUE;
		}
	}
	else
		bNeedCreate = TRUE;
	if(bNeedCreate == TRUE)
	{
		// release
		SAFE_RELEASE(m_lpD3D9SharedSurface);
		m_hSharedHandle = NULL;

		hr = m_pD3D9DeviceWrapper->CreateOffscreenPlainSurfaceEx(Width,
			Height,
			Format,
			Pool,
			&m_lpD3D9SharedSurface,
			&hSharedHandle,0);
		if(FAILED(hr))
		{
			//WSE_ERROR_TRACE("CDirect3D9ExDraw::PrepareSharedSurface(), CreateOffscreenPlainSurfaceEx failed! errorcode = "<<hr);
			return FALSE;
		}

		m_hSharedHandle = hSharedHandle;

		//WSE_INFO_TRACE("Shared Surface is created! Width = "<<Width<<",Height = "<<Height<<",Format = "<<Format<<",Pool = "<<Pool<<",hSharedHandle = "<<hSharedHandle);

		hr = m_lpD3D9SharedSurface->GetDesc(&m_d3dsdShared);
		if(FAILED(hr))
		{
			//WSE_ERROR_TRACE("CDirect3D9ExDraw::PrepareSharedSurface(), GetDesc failed! errorcode = "<<hr);
			return FALSE;
		}
	}
	return TRUE;
}