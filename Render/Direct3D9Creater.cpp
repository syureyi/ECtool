#include "Direct3d9Creater.h"
#include "Direct3D9Draw.h"
CDraw* CreateDirect3D9ExDraw(HWND hWnd,IDrawHelper* pHelper)
{

	CDraw* pDraw = new CDirect3D9ExDraw;
	if(pDraw)
	{
		if(pDraw->Init(hWnd,pHelper))
			return pDraw;
		delete pDraw;
	}
	return NULL;
}