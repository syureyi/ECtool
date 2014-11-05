#ifndef DIRECT3D9DEVICE_WRAPPER_IMPL_H
#define DIRECT3D9DEVICE_WRAPPER_IMPL_H
#include "IDirect3D9DeviceWrapper.h"


class Direct3D9DeviceWrapperImpl : public IDirect3D9DeviceWrapper
{
public:
	Direct3D9DeviceWrapperImpl(IDirect3DDevice9* pD3D9Device);
	virtual ~Direct3D9DeviceWrapperImpl();

	STDMETHOD(GetDeviceCaps)(THIS_ D3DCAPS9* pCaps);
	STDMETHOD(SetRenderState)(THIS_ D3DRENDERSTATETYPE State,DWORD Value);
	STDMETHOD(ResetEx)(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters,D3DDISPLAYMODEEX *pFullscreenDisplayMode);
	STDMETHOD(GetBackBuffer)(THIS_ UINT iSwapChain,UINT iBackBuffer,D3DBACKBUFFER_TYPE Type,IDirect3DSurface9** ppBackBuffer);
	STDMETHOD(CheckDeviceState)(THIS_ HWND hDestinationWindow);
	STDMETHOD(PresentEx)(THIS_ CONST RECT* pSourceRect,CONST RECT* pDestRect,HWND hDestWindowOverride,CONST RGNDATA* pDirtyRegion,DWORD dwFlags);
	STDMETHOD(CreateTexture)(THIS_ UINT Width,UINT Height,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DTexture9** ppTexture,HANDLE* pSharedHandle);
	STDMETHOD(CreateRenderTargetEx)(THIS_ UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Lockable,
		IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle,DWORD Usage);
	STDMETHOD(CreateOffscreenPlainSurfaceEx)(THIS_ UINT Width,UINT Height,D3DFORMAT Format,D3DPOOL Pool,
		IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle,DWORD Usage);
	STDMETHOD(StretchRect)(THIS_ IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface9* pDestSurface,
		CONST RECT* pDestRect,D3DTEXTUREFILTERTYPE Filter);
	STDMETHOD(SetRenderTarget)(THIS_ DWORD RenderTargetIndex,IDirect3DSurface9* pRenderTarget);
	STDMETHOD(BeginScene)(THIS);
	STDMETHOD(EndScene)(THIS);
	STDMETHOD(ColorFill)(THIS_ IDirect3DSurface9* pSurface,CONST RECT* pRect,D3DCOLOR color);
	virtual IDirect3DDevice9* GetDirect3DDevice9();

private:
	IDirect3DDevice9 *m_pD3D9Device;
};

#endif