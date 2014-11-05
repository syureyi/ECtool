#ifndef IDIRECT3D9DEVICE_WRAPPER_H_
#define IDIRECT3D9DEVICE_WRAPPER_H_
#include "../vendor/DXSDK_August2009/Include/d3d9.h"

#define SAFE_RELEASE(p) \
	if(p) \
	p->Release();\
	p = NULL;
class IDirect3D9DeviceWrapper
{
public:
	virtual ~IDirect3D9DeviceWrapper() = 0{};
	STDMETHOD(GetDeviceCaps)(THIS_ D3DCAPS9* pCaps) PURE;
	STDMETHOD(SetRenderState)(THIS_ D3DRENDERSTATETYPE State,DWORD Value) PURE;
	STDMETHOD(ResetEx)(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters,D3DDISPLAYMODEEX *pFullscreenDisplayMode) PURE;
	STDMETHOD(GetBackBuffer)(THIS_ UINT iSwapChain,UINT iBackBuffer,D3DBACKBUFFER_TYPE Type,IDirect3DSurface9** ppBackBuffer) PURE;
	STDMETHOD(CheckDeviceState)(THIS_ HWND hDestinationWindow) PURE;
	STDMETHOD(PresentEx)(THIS_ CONST RECT* pSourceRect,CONST RECT* pDestRect,HWND hDestWindowOverride,CONST RGNDATA* pDirtyRegion,DWORD dwFlags) PURE;
	STDMETHOD(CreateTexture)(THIS_ UINT Width,UINT Height,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DTexture9** ppTexture,HANDLE* pSharedHandle) PURE;
	STDMETHOD(CreateRenderTargetEx)(THIS_ UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Lockable,
		IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle,DWORD Usage) PURE;
	STDMETHOD(CreateOffscreenPlainSurfaceEx)(THIS_ UINT Width,UINT Height,D3DFORMAT Format,D3DPOOL Pool,
		IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle,DWORD Usage) PURE;
	STDMETHOD(StretchRect)(THIS_ IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface9* pDestSurface,
		CONST RECT* pDestRect,D3DTEXTUREFILTERTYPE Filter) PURE;
	STDMETHOD(SetRenderTarget)(THIS_ DWORD RenderTargetIndex,IDirect3DSurface9* pRenderTarget) PURE;
	STDMETHOD(BeginScene)(THIS) PURE;
	STDMETHOD(EndScene)(THIS) PURE;
	STDMETHOD(ColorFill)(THIS_ IDirect3DSurface9* pSurface,CONST RECT* pRect,D3DCOLOR color) PURE;
	virtual IDirect3DDevice9* GetDirect3DDevice9() = 0;
};

#endif