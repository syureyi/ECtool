#ifndef DIRECT3D9_WRAPPER_FACTORY_H
#define DIRECT3D9_WRAPPER_FACTORY_H

#include "../vendor/DXSDK_August2009/Include/d3d9.h"
class IDirect3D9Wrapper;

class Direct3D9WrapperFactory
{
public:
	Direct3D9WrapperFactory();
	~Direct3D9WrapperFactory();
	
	IDirect3D9Wrapper* CreateDirect3D9Wrapper();

	BOOL CanCreateD3D9();
	
private:
	HMODULE m_hDll;
	typedef IDirect3D9* (WINAPI *DIRECT3DCREATE9) (UINT);
	DIRECT3DCREATE9 m_pfnDirect3DCreate9;
	typedef HRESULT (WINAPI *DIRECT3DCREATE9EX)(UINT,IDirect3D9Ex**);
	DIRECT3DCREATE9EX m_pfnDirect3DCreate9Ex;
};

#endif