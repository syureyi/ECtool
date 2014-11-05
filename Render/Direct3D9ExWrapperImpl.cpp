#include "../common/Util.h"
#include "Direct3D9ExWrapperImpl.h"
#include "Direct3D9DeviceWrapperImpl.h"
#include "Direct3D9DeviceExWrapperImpl.h"

Direct3D9ExWrapperImpl::Direct3D9ExWrapperImpl(IDirect3D9Ex *pD3D9Ex):
m_pD3D9Ex(pD3D9Ex)
{

}

Direct3D9ExWrapperImpl::~Direct3D9ExWrapperImpl()
{
	SAFE_RELEASE(m_pD3D9Ex);
}

UINT Direct3D9ExWrapperImpl::GetAdapterCount()
{
	return m_pD3D9Ex->GetAdapterCount();
}

HRESULT Direct3D9ExWrapperImpl::GetAdapterIdentifier(UINT Adapter,DWORD Flags,D3DADAPTER_IDENTIFIER9* pIdentifier)
{
	return m_pD3D9Ex->GetAdapterIdentifier(Adapter,Flags,pIdentifier);
}


HMONITOR Direct3D9ExWrapperImpl::GetAdapterMonitor(UINT Adapter)
{
	return m_pD3D9Ex->GetAdapterMonitor(Adapter);
}

HRESULT Direct3D9ExWrapperImpl::GetAdapterDisplayModeEx(UINT Adapter,D3DDISPLAYMODEEX* pMode,D3DDISPLAYROTATION* pRotation)
{
	return m_pD3D9Ex->GetAdapterDisplayModeEx(Adapter, pMode, pRotation);
}

HRESULT Direct3D9ExWrapperImpl::CheckDeviceFormat(UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT AdapterFormat,DWORD Usage,
												D3DRESOURCETYPE RType,D3DFORMAT CheckFormat)
{
	return m_pD3D9Ex->CheckDeviceFormat(Adapter, DeviceType, AdapterFormat, Usage, RType, CheckFormat);
}

HRESULT Direct3D9ExWrapperImpl::CheckDeviceFormatConversion(UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT SourceFormat,
														  D3DFORMAT TargetFormat)
{
	return m_pD3D9Ex->CheckDeviceFormatConversion(Adapter, DeviceType, SourceFormat, TargetFormat);
}

HRESULT Direct3D9ExWrapperImpl::CreateDeviceEx(UINT Adapter,D3DDEVTYPE DeviceType,HWND hFocusWindow,DWORD BehaviorFlags,
											 D3DPRESENT_PARAMETERS* pPresentationParameters,D3DDISPLAYMODEEX* pFullscreenDisplayMode,
											 IDirect3D9DeviceWrapper** ppReturnedD3D9WrappterInterface)
{
	if(!pPresentationParameters || !ppReturnedD3D9WrappterInterface)
		return E_INVALIDARG;

	IDirect3DDevice9Ex *pD3DDevice9Ex = NULL;

	HRESULT hr = m_pD3D9Ex->CreateDeviceEx(Adapter, DeviceType, hFocusWindow, BehaviorFlags, 
		pPresentationParameters, pFullscreenDisplayMode, &pD3DDevice9Ex);

	if(SUCCEEDED(hr))
	{
		Direct3D9DeviceExWrapperImpl *pD3D9DeviceWrapper = new Direct3D9DeviceExWrapperImpl(pD3DDevice9Ex);
		if(pD3D9DeviceWrapper)
		{
			*ppReturnedD3D9WrappterInterface = pD3D9DeviceWrapper;
			return hr;
		}
		else
		{
			return E_OUTOFMEMORY;
		}
	}
	return hr;
}

BOOL Direct3D9ExWrapperImpl::IsSupportedVA()
{
	return TRUE;
}

BOOL Direct3D9ExWrapperImpl::IsAlwaysNeedCheckDeviceState()
{
	return FALSE;
}

BOOL Direct3D9ExWrapperImpl::IsDirect3D9ExObject()
{
	return TRUE;
}