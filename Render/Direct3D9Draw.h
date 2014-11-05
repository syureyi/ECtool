//===================================================================
// Copyright (C) 2002-2010 WebEx Communications
// All rights reserved.
//	
//	Creator:	jinj@hz.webex.com
//	Date:		07/15/2010
//	Notes :		
//===================================================================

//===================================================================
//	History:
//	Version	Name		Date			Description
//	1.00	jinj		07/15/2010		Created
//
//===================================================================

#ifndef _DIRECT3D9DRAW_H_
#define _DIRECT3D9DRAW_H_

#include "Draw.h"

#include "../vendor/DXSDK_August2009/Include/d3d9.h"
class Direct3D9WrapperFactory;
class IDirect3D9Wrapper;
class IDirect3D9DeviceWrapper;

typedef struct 
{
	DWORD dwBitmapWidth;
	DWORD dwBitmapHeight;
	D3DSURFACE_DESC d3dsdTexture;
	LPDIRECT3DTEXTURE9 pd3d9Texture;
} D3D9_TEXTURE_PARAM, *PD3D9_TEXTURE_PARAM;

#include "list"
typedef std::list<DWORD> FourCCList;
class CDirect3D9ExDraw : public CDraw2
{
public:
	CDirect3D9ExDraw();
	virtual ~CDirect3D9ExDraw();
	
	BOOL Init(HWND hWnd,IDrawHelper* pHelper);
	void UnInit();
	BOOL IsFourCCSupported(DWORD dwFourCC);
	long Draw(LPBYTE pImage,LPBITMAPINFOHEADER pbmi,HDC hDrawToDC,const RECT& rcSource,const RECT& rcTarget);
	BOOL DrawBkgnd(HDC hDrawToDC,const RECT& rcTarget);
	void DiscardStream();
	
	BOOL AddDrawBitmap(unsigned int uID,PBMP_DESC pParam);
	void RemoveDrawBitmap(unsigned int uID);
	void SetBKColor(BK_DESC bk);

	VA_TYPE GetSupportedVAType();
	BOOL VADraw(VA_TYPE type,void* pContent,const RECT& rcSource,const RECT& rcTarget);

	BOOL UpdateDrawTargetRectangle(const RECT& rcTarget);
	void WindowThreadAction();
	BOOL IsNeedWindowThreadDestroy();
protected:
	void FreeBP(PBMP_PARAM pbp);
	void FreeBKP(PBK_PARAM pbkp);

	HRESULT AlphaBlt(LPDIRECT3DDEVICE9 pd3d9Device,RECT* lpDst,LPDIRECT3DTEXTURE9 pd3d9Texture,const D3DSURFACE_DESC &d3dsdTexture,RECT* lpSrc,BYTE bAlpha,BOOL bNearestInterpolation = TRUE);
	HRESULT DoBlt(LPDIRECT3DDEVICE9 pd3d9Device,RECT* lpDst,LPDIRECT3DTEXTURE9 pd3d9Texture,const D3DSURFACE_DESC &d3dsdTexture,RECT* lpSrc,BYTE bAlpha,BOOL bNearestInterpolation = TRUE);

	BOOL InitD3D9Ex(HWND hWnd);
	void UnInitD3D9Ex();
	void UnInitD3D9DeviceExChain();
	BOOL CreateTexture(LPBYTE pImage,LPBITMAPINFOHEADER pbmi,COLORREF clrkey,LPDIRECT3DDEVICE9 pd3d9Device,const D3DCAPS9 &d3d9DeviceCapsRef,/*out*/LPDIRECT3DTEXTURE9* ppd3d9Texture,/*out*/D3DSURFACE_DESC* pd3dsdTexture);
	BOOL CreateTextureFromDDB(HBITMAP hBitmap,COLORREF clrkey,LPDIRECT3DDEVICE9 pd3d9Device,const D3DCAPS9 &d3d9DeviceCapsRef,/*out*/LPDIRECT3DTEXTURE9* ppd3d9Texture,/*out*/D3DSURFACE_DESC* pd3dsdTexture);
	BOOL CreateTextureFromHBITMAP(HBITMAP hBitmap,COLORREF clrkey,LPDIRECT3DDEVICE9 pd3d9Device,const D3DCAPS9 &d3d9DeviceCapsRef,/*out*/LPDIRECT3DTEXTURE9* ppd3d9Texture,/*out*/D3DSURFACE_DESC* pd3dsdTexture);
	BOOL CreateTextureByDesc(PBITMAP_DESC pbd,COLORREF clrkey,LPDIRECT3DDEVICE9 pd3d9Device,const D3DCAPS9 &d3d9DeviceCapsRef,/*out*/LPDIRECT3DTEXTURE9* ppd3d9Texture,/*out*/LPDWORD pdwBitmapWidth,/*out*/LPDWORD pdwBitmapHeight,/*out*/D3DSURFACE_DESC* pd3dsdTexture);

	BOOL CreateSurface_XRGB(LPBYTE pImage,LPBITMAPINFOHEADER pbmi,IDirect3D9DeviceWrapper* pD3D9DeviceWrapper,const D3DCAPS9 &d3d9DeviceCapsRef,/*out*/LPDIRECT3DSURFACE9* ppd3d9Surface,/*out*/D3DSURFACE_DESC* pd3dsdSurface);
	BOOL CreateSurfaceFromHBITMAP(HBITMAP hBitmap,IDirect3D9DeviceWrapper* pD3D9DeviceWrapper,const D3DCAPS9 &d3d9DeviceCapsRef,/*out*/LPDIRECT3DSURFACE9* ppd3d9Surface,/*out*/D3DSURFACE_DESC* pd3dsdSurface);
	BOOL CreateSurfaceByDesc(PBITMAP_DESC pbd,IDirect3D9DeviceWrapper* pD3D9DeviceWrapper,const D3DCAPS9 &d3d9DeviceCapsRef,/*out*/LPDIRECT3DSURFACE9* ppd3d9Surface,/*out*/D3DSURFACE_DESC* pd3dsdSurface);
	BOOL FillDataToSurface_XRGB2XRGB(LPBYTE pImage,LPBITMAPINFOHEADER pbmi,LPDIRECT3DSURFACE9 pd3d9Surface);

	void UpdateFourCCList(UINT uAdapter,D3DFORMAT TargetFormat);
	HRESULT CreateDevice(HMONITOR hmonWnd,UINT uiWidth,UINT uiHeight);
	HRESULT Reset(UINT uiWidth,UINT uiHeight);

	HRESULT FillBkgnd(LPDIRECT3DSURFACE9 lpDrawToD3D9Surface,LPRECT lprcDest);
	HRESULT FillRawSurface(LPBYTE pImage,LPBITMAPINFOHEADER pbmi,const RECT& rect);
	HRESULT FillSurface_OtherPixelFormat(LPDIRECT3DSURFACE9 pd3dsDest,const D3DSURFACE_DESC& d3dsdDest,LPBYTE pImage,LPBITMAPINFOHEADER pbmi,const RECT& rect);
 	HRESULT FillSurface_YUY2AndUYVY(LPDIRECT3DSURFACE9 pd3dsDest,const D3DSURFACE_DESC& d3dsdDest,LPBYTE pImage,LPBITMAPINFOHEADER pbmi,const RECT& rect);
 	HRESULT FillSurface_I420AndYV12(LPDIRECT3DSURFACE9 pd3dsDest,const D3DSURFACE_DESC& d3dsdDest,LPBYTE pImage,LPBITMAPINFOHEADER pbmi,const RECT& rect);
	HRESULT FillSurface_BGR(LPDIRECT3DSURFACE9 pd3dsDest,const D3DSURFACE_DESC& d3dsdDest,LPBYTE pImage,LPBITMAPINFOHEADER pbmi,const RECT& rect);
	BOOL PrepareBkgnd();
	BOOL PrepareTextures(int mode);// 0both 1ActOnSrc 2ActOnDest

	BOOL PrepareImageSurface(DWORD dwWidth,DWORD dwHeight,D3DFORMAT Format,BOOL &bLost);
	BOOL PrepareRawSurface(DWORD dwWidth,DWORD dwHeight,DWORD dwPixelFormat);

	BOOL FillDataToTexture_ARGB2ARGB(LPBYTE pImage,LPBITMAPINFOHEADER pbmi,LPDIRECT3DTEXTURE9 pd3d9Texture);
	BOOL FillDataToTexture_DDB2ARGB(HBITMAP hBitmap,COLORREF clrkey,LPDIRECT3DTEXTURE9 pd3d9Texture);

	HRESULT Do_Effect(LPDIRECT3DSURFACE9 pSurfaceDrawTo,DWORD dwWidth,DWORD dwHeight,BOOL bActOnSrc);
	BOOL Do_Image2BackBuffer(const RECT& rcSrcRect,LPDIRECT3DSURFACE9 pBackBufferSuface,const RECT& rcDestRect);
	BOOL DecideImageUseTexture();

	typedef enum
	{
		CheckingCode_Success,
		CheckingCode_StateChange,
		CheckingCode_Error
	}
	CheckingCode;
	CheckingCode CheckDevice();
	CheckingCode CheckAndRecoverD3D9ExObjects(HMONITOR hmonWnd,UINT uiWidth,UINT uiHeight);
	void PrepareD3D9ExObjects(HMONITOR hmonWnd,UINT uiWidth,UINT uiHeight);
	BOOL GetBackBuffer(IDirect3DSurface9 **ppBackBuffer);
	CheckingCode Present(const RECT* prcSrc,const RECT* prcDest);
	void NotifyD3D9ObjectsNormalization();
	BOOL IsWindowThread();
protected:
	// video accelerate
	HANDLE m_hSharedHandle;
	LPDIRECT3DSURFACE9 m_lpD3D9SharedSurface;
	D3DSURFACE_DESC m_d3dsdShared;
	BOOL PrepareSharedSurface(HANDLE hSharedHandle,UINT Width,UINT Height,D3DFORMAT Format,D3DPOOL Pool);
protected:
 	//LPDIRECT3D9EX m_lpD3D9Ex;
	//LPDIRECT3DDEVICE9EX m_lpD3D9ExDevice;

	Direct3D9WrapperFactory *m_pD3D9WrapperFactory;
	IDirect3D9Wrapper* m_pD3D9Wrapper;
	IDirect3D9DeviceWrapper* m_pD3D9DeviceWrapper;

	D3DPRESENT_PARAMETERS m_d3dpp;
	D3DCAPS9 m_d3d9ExDeviceCaps;
	LPDIRECT3DSURFACE9 m_lpD3D9ImageSurface;
	LPDIRECT3DTEXTURE9 m_lpD3D9ImageTexture;
	LPDIRECT3DSURFACE9 m_lpD3D9RawSurface;
	
	D3DSURFACE_DESC m_d3dsdImage;
	D3DSURFACE_DESC m_d3dsdRaw;

	BOOL m_bRawExpired;
	BOOL m_bImageExpired;

	FourCCList m_listFourCC;

	HMONITOR m_hmon;

	HWND m_hWnd;
	
	DWORD m_dwTIDWindow;

	IDrawHelper* m_pHelper;
	
	typedef enum
	{
		StateCode_Active,
		StateCode_Invalid,
		StateCode_Uncreated,
		StateCode_NeedRecreate,
		StateCode_NeedReset
	}
	StateCode;
	StateCode m_stateCode;
	BOOL m_bNeedCheckDevice;

	RECT m_rcTarget;

	HMODULE m_hDll;
	typedef HRESULT (WINAPI *DIRECT3DCREATE9EX)(UINT,IDirect3D9Ex**);
	DIRECT3DCREATE9EX m_pfnDirect3DCreate9Ex;
	std::map<int, int> m_mapBlackList;
};



#endif // _DIRECT3D9EXDRAW_H_