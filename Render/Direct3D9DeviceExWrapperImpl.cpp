#include "../common/Util.h"
#include "Direct3D9DeviceExWrapperImpl.h"

//#include "WseDebug.h"

Direct3D9DeviceExWrapperImpl::Direct3D9DeviceExWrapperImpl(IDirect3DDevice9Ex* pD3D9DeviceEx):
m_pD3D9DeviceEx(pD3D9DeviceEx)
{
	//WSE_INFO_TRACE("Direct3D9DeviceExWrapperImpl::Direct3D9DeviceExWrapperImpl(IDirect3DDevice9Ex* pD3D9DeviceEx)");
}

Direct3D9DeviceExWrapperImpl::~Direct3D9DeviceExWrapperImpl()
{
	//WSE_INFO_TRACE("Direct3D9DeviceExWrapperImpl::~Direct3D9DeviceExWrapperImpl()");
	SAFE_RELEASE(m_pD3D9DeviceEx);
}

HRESULT Direct3D9DeviceExWrapperImpl::GetDeviceCaps(D3DCAPS9* pCaps)
{
	return m_pD3D9DeviceEx->GetDeviceCaps(pCaps);
}

HRESULT Direct3D9DeviceExWrapperImpl::SetRenderState(D3DRENDERSTATETYPE State,DWORD Value)
{
	return m_pD3D9DeviceEx->SetRenderState(State, Value);
}

HRESULT Direct3D9DeviceExWrapperImpl::ResetEx(D3DPRESENT_PARAMETERS* pPresentationParameters,D3DDISPLAYMODEEX *pFullscreenDisplayMode)
{
	return m_pD3D9DeviceEx->ResetEx(pPresentationParameters, pFullscreenDisplayMode);
}

HRESULT Direct3D9DeviceExWrapperImpl::GetBackBuffer(UINT iSwapChain,UINT iBackBuffer,D3DBACKBUFFER_TYPE Type,IDirect3DSurface9** ppBackBuffer)
{
	return m_pD3D9DeviceEx->GetBackBuffer(iSwapChain, iBackBuffer, Type, ppBackBuffer);
}

HRESULT Direct3D9DeviceExWrapperImpl::CheckDeviceState(HWND hDestinationWindow)
{
	return m_pD3D9DeviceEx->CheckDeviceState(hDestinationWindow);
}

HRESULT Direct3D9DeviceExWrapperImpl::PresentEx(CONST RECT* pSourceRect,CONST RECT* pDestRect,HWND hDestWindowOverride,CONST RGNDATA* pDirtyRegion,DWORD dwFlags)
{
	return m_pD3D9DeviceEx->PresentEx(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags);
}

HRESULT Direct3D9DeviceExWrapperImpl::CreateTexture(UINT Width,UINT Height,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DTexture9** ppTexture,HANDLE* pSharedHandle)
{
	return m_pD3D9DeviceEx->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
}

HRESULT Direct3D9DeviceExWrapperImpl::CreateRenderTargetEx(UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Lockable,
														 IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle,DWORD Usage)
{
	return m_pD3D9DeviceEx->CreateRenderTargetEx(Width, Height, Format, MultiSample, 
		MultisampleQuality, Lockable, ppSurface, pSharedHandle, Usage);
}

HRESULT Direct3D9DeviceExWrapperImpl::CreateOffscreenPlainSurfaceEx(UINT Width,UINT Height,D3DFORMAT Format,D3DPOOL Pool,
																  IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle,DWORD Usage)
{
	return m_pD3D9DeviceEx->CreateOffscreenPlainSurfaceEx(Width, Height, Format, Pool, ppSurface, pSharedHandle, Usage);
}

HRESULT Direct3D9DeviceExWrapperImpl::StretchRect(IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface9* pDestSurface,
												CONST RECT* pDestRect,D3DTEXTUREFILTERTYPE Filter)
{
	return m_pD3D9DeviceEx->StretchRect(pSourceSurface, pSourceRect, pDestSurface, pDestRect, Filter);
}

HRESULT Direct3D9DeviceExWrapperImpl::SetRenderTarget(DWORD RenderTargetIndex,IDirect3DSurface9* pRenderTarget)
{
	return m_pD3D9DeviceEx->SetRenderTarget(RenderTargetIndex, pRenderTarget);
}

HRESULT Direct3D9DeviceExWrapperImpl::BeginScene()
{
	return m_pD3D9DeviceEx->BeginScene();
}

HRESULT Direct3D9DeviceExWrapperImpl::EndScene()
{
	return m_pD3D9DeviceEx->EndScene();
}

HRESULT Direct3D9DeviceExWrapperImpl::ColorFill(IDirect3DSurface9* pSurface,CONST RECT* pRect,D3DCOLOR color)
{
	return m_pD3D9DeviceEx->ColorFill(pSurface, pRect, color);
}

IDirect3DDevice9* Direct3D9DeviceExWrapperImpl::GetDirect3DDevice9()
{
	return m_pD3D9DeviceEx;
}
