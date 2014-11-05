#include "Direct3D9WrapperFactory.h"
//#include "WseDebug.h"
//#include "Direct3D9WrapperImpl.h"
#include "Direct3D9ExWrapperImpl.h"
#ifdef WIN32
#include <tchar.h>
#endif
Direct3D9WrapperFactory::Direct3D9WrapperFactory()
{
	m_pfnDirect3DCreate9Ex = NULL;
	m_pfnDirect3DCreate9 = NULL;
	m_hDll = LoadLibrary(_T("d3d9.dll"));
	if(m_hDll)
	{
		m_pfnDirect3DCreate9Ex = (DIRECT3DCREATE9EX)GetProcAddress(m_hDll,"Direct3DCreate9Ex");
		//m_pfnDirect3DCreate9 = (DIRECT3DCREATE9)GetProcAddress(m_hDll, "Direct3DCreate9");
	}
};

Direct3D9WrapperFactory::~Direct3D9WrapperFactory()
{
	if(m_hDll)
	{
		FreeLibrary(m_hDll);
		m_hDll = NULL;
	}
}

IDirect3D9Wrapper* Direct3D9WrapperFactory::CreateDirect3D9Wrapper()
{
	if(m_pfnDirect3DCreate9Ex)
	{
		LPDIRECT3D9EX lpD3D9Ex = NULL;
		HRESULT hr = m_pfnDirect3DCreate9Ex(D3D_SDK_VERSION, &lpD3D9Ex);
		//SetDllDirectory(NULL);
		if(FAILED(hr) || !lpD3D9Ex)
		{
			//WSE_ERROR_TRACE("Direct3D9WrapperFactory::CreateDirect3D9Wrapper(), Direct3DCreate9Ex failed! errorcode = "<<hr);
		}
		else
		{
			// create the Direct3D9ExWrapper
			Direct3D9ExWrapperImpl *pD3D9ExWrapper = new Direct3D9ExWrapperImpl(lpD3D9Ex);
			if(pD3D9ExWrapper)
			{
				return pD3D9ExWrapper;
			}
		}
	}

	//if(m_pfnDirect3DCreate9)
	//{
	//	LPDIRECT3D9 lpD3D9 = m_pfnDirect3DCreate9(D3D_SDK_VERSION);
	//	//SetDllDirectory(NULL);
	//	if(!lpD3D9)
	//	{
	//		WSE_ERROR_TRACE("Direct3D9WrapperFactory::CreateDirect3D9Wrapper(), Direct3DCreate failed");
	//	}
	//	else
	//	{
	//		Direct3D9WrapperImpl *pD3D9Wrapper = new Direct3D9WrapperImpl(lpD3D9);
	//		if(lpD3D9)
	//		{
	//			return pD3D9Wrapper;
	//		}
	//	}
	//}

	return NULL;
}

BOOL Direct3D9WrapperFactory::CanCreateD3D9()
{
	if( m_pfnDirect3DCreate9Ex)
		return TRUE;
	return FALSE;
}