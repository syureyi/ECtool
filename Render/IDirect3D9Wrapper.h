#ifndef IDIRECT3D9_WRAAPER_H
#define IDIRECT3D9_WRAAPER_H

#include "../vendor/DXSDK_August2009/Include/d3d9.h"
class IDirect3D9DeviceWrapper;

class IDirect3D9Wrapper
{
public:
	virtual ~IDirect3D9Wrapper() = 0{};
	STDMETHOD_(UINT, GetAdapterCount)(THIS) PURE;
	STDMETHOD(GetAdapterIdentifier)(THIS_ UINT Adapter,DWORD Flags,D3DADAPTER_IDENTIFIER9* pIdentifier) PURE;
	STDMETHOD_(HMONITOR, GetAdapterMonitor)(THIS_ UINT Adapter) PURE;
	STDMETHOD(GetAdapterDisplayModeEx)(THIS_ UINT Adapter,D3DDISPLAYMODEEX* pMode,D3DDISPLAYROTATION* pRotation) PURE;
	STDMETHOD(CheckDeviceFormat)(THIS_ UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT AdapterFormat,DWORD Usage,
		D3DRESOURCETYPE RType,D3DFORMAT CheckFormat) PURE;
	STDMETHOD(CheckDeviceFormatConversion)(THIS_ UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT SourceFormat,
		D3DFORMAT TargetFormat) PURE;
	STDMETHOD(CreateDeviceEx)(THIS_ UINT Adapter,D3DDEVTYPE DeviceType,HWND hFocusWindow,DWORD BehaviorFlags,
		D3DPRESENT_PARAMETERS* pPresentationParameters,D3DDISPLAYMODEEX* pFullscreenDisplayMode,
		IDirect3D9DeviceWrapper** ppReturnedD3D9WrappterInterface) PURE;

	virtual BOOL IsSupportedVA() = 0;
	virtual BOOL IsAlwaysNeedCheckDeviceState() = 0;
	virtual BOOL IsDirect3D9ExObject() = 0;
};

#endif // IDIRECT3D9_WRAAPER_H