#ifndef DIRECT3D9EX_WRAPPER_IMPL_H
#define DIRECT3D9EX_WRAPPER_IMPL_H
#include "IDirect3D9Wrapper.h"

class Direct3D9ExWrapperImpl : public IDirect3D9Wrapper
{
public:
	Direct3D9ExWrapperImpl(IDirect3D9Ex *pD3D9Ex);
	virtual ~Direct3D9ExWrapperImpl();

	STDMETHOD_(UINT, GetAdapterCount)(THIS);
	STDMETHOD(GetAdapterIdentifier)(THIS_ UINT Adapter,DWORD Flags,D3DADAPTER_IDENTIFIER9* pIdentifier);
	STDMETHOD_(HMONITOR, GetAdapterMonitor)(THIS_ UINT Adapter);
	STDMETHOD(GetAdapterDisplayModeEx)(THIS_ UINT Adapter,D3DDISPLAYMODEEX* pMode,D3DDISPLAYROTATION* pRotation);
	STDMETHOD(CheckDeviceFormat)(THIS_ UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT AdapterFormat,DWORD Usage,
		D3DRESOURCETYPE RType,D3DFORMAT CheckFormat);
	STDMETHOD(CheckDeviceFormatConversion)(THIS_ UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT SourceFormat,
		D3DFORMAT TargetFormat);
	STDMETHOD(CreateDeviceEx)(THIS_ UINT Adapter,D3DDEVTYPE DeviceType,HWND hFocusWindow,DWORD BehaviorFlags,
		D3DPRESENT_PARAMETERS* pPresentationParameters,D3DDISPLAYMODEEX* pFullscreenDisplayMode,
		IDirect3D9DeviceWrapper** ppReturnedD3D9WrappterInterface);

	virtual BOOL IsSupportedVA();
	virtual BOOL IsAlwaysNeedCheckDeviceState();
	virtual BOOL IsDirect3D9ExObject();
private:
	IDirect3D9Ex *m_pD3D9Ex;

};

#endif