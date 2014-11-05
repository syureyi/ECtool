#ifndef _DRAW_H_
#define _DRAW_H_

#if defined(WIN32)
#if !defined(POINTER_64) && _MSC_VER > 1200
#define POINTER_64 __ptr64
#endif
#pragma warning(disable:4786)
#endif
#include "tchar.h"
#include <winsock2.h>
#include "windows.h"
#include "map"

// ARGB bitmap should be premultiplied alpha format
typedef enum
{
	BT_HBITMAP = 0,		// bitmap handle,maybe ddb or dibsection
	BT_DIBGMO,			// global memory object,GlobalLock
	BT_DIB				// memory address
}BITMAPTYPE;

typedef struct
{
	BITMAPTYPE bmptype;
	void* bitmap;
}BITMAP_DESC, *PBITMAP_DESC;

#define BMP_DESC_MASK_REVERSE_ORIGIN_LEFT		0x00000001
#define BMP_DESC_MASK_REVERSE_ORIGIN_RIGHT		0x00000002
#define BMP_DESC_MASK_REVERSE_ORIGIN_TOP		0x00000004
#define BMP_DESC_MASK_REVERSE_ORIGIN_BOTTOM		0x00000008
#define BMP_DESC_MASK_PERCENTAGEPOSITION		0x00000010
#define BMP_DESC_MASK_ACTONSRC					0x00000020

#define IS_REVERSE_ORIGIN_LEFT(x)				((BMP_DESC_MASK_REVERSE_ORIGIN_LEFT & x) != 0)
#define IS_REVERSE_ORIGIN_RIGHT(x)				((BMP_DESC_MASK_REVERSE_ORIGIN_RIGHT & x) != 0)
#define IS_REVERSE_ORIGIN_TOP(x)				((BMP_DESC_MASK_REVERSE_ORIGIN_TOP & x) != 0)
#define IS_REVERSE_ORIGIN_BOTTOM(x)				((BMP_DESC_MASK_REVERSE_ORIGIN_BOTTOM & x) != 0)
#define IS_PERCENTAGEPOSITION(x)				((BMP_DESC_MASK_PERCENTAGEPOSITION & x) != 0)
#define IS_ACTONSRC(x)							((BMP_DESC_MASK_ACTONSRC & x) != 0)

#define PERCENTAGEPOS_MIN						(0.0f)
#define PERCENTAGEPOS_MAX						(1.0f)
#define IS_PERCENTAGEPOS_OUTOFRANGE(x)			(x < PERCENTAGEPOS_MIN && x > PERCENTAGEPOS_MAX)
typedef struct 
{
	BITMAPTYPE bmptype;
	void* bitmap;
	COLORREF clrkey;
	DWORD dwMask;
	float fleft;
	float ftop;
	float fright;
	float fbottom;
} BMP_DESC, *PBMP_DESC;
/*
dwMask: Optional flags.One or more of the following flags.
BMP_DESC_MASK_REVERSE_ORIGIN_LEFT
	The fleft member's origin is right.
BMP_DESC_MASK_REVERSE_ORIGIN_RIGHT
	The fright member's origin is right.
BMP_DESC_MASK_REVERSE_ORIGIN_TOP
	The ftop member's origin is bottom.
BMP_DESC_MASK_REVERSE_ORIGIN_BOTTOM
	The fbottom member's origin is bottom.
BMP_DESC_MASK_PERCENTAGEPOSITION
	The position parameters are percent value.The range is 0.0 ~ 1.0.The origin is Upper-left corner,BMP_DESC_MASK_REVERSE_ORIGIN_XXXX will be ignored.
BMP_DESC_MASK_ACTONSRC
	The bitmap will act on the source image.
*/

typedef struct 
{
	BMP_DESC bd;
	LPVOID pRef;
} BMP_PARAM, *PBMP_PARAM;
typedef std::map<unsigned int,BMP_PARAM> ID2BPMap;

typedef enum
{
	BKT_COLOR = 0,
	BKT_BITMAP
}BKTYPE;
typedef struct
{
	BKTYPE bktype;
    union
    {
		BITMAP_DESC bitmapdesc;
		COLORREF clr;
    };
} BK_DESC, *PBK_DESC;
typedef struct 
{
	BK_DESC bkd;
	LPVOID pRef;
} BK_PARAM, *PBK_PARAM;

typedef enum
{
	ARM_NONE      = 0,
	ARM_LETTER_BOX,
	ARM_STRETCH
} ASPECTRATIOMODE;

#define IMAGEEFFECT_MIRRORLEFTRIGHT			0x00000001
#define IMAGEEFFECT_MIRRORUPDOWN			0x00000002

LPBYTE GetBits(HBITMAP hBitmap,LPBITMAPINFO lpbi);
void CalcBitmapDrawToRect(LPCRECT prcEntire,PBMP_DESC pbd,DWORD dwBmpWidth,DWORD dwBmpHeight,LPRECT prcTarget);
HBITMAP MakeBitmap(LPBYTE pData,LPBITMAPINFOHEADER lpbi);

#define GetAValue(rgb)      ((BYTE)((rgb)>>24))

class IDrawHelper
{
public:
	virtual void TriggerWindowThreadAction()=0;
};

#define CDraw_Success				1
#define CDraw_Failure				-1
#define CDraw_UnsupportedFormat		-2
#define CDraw_NeedData				-3

typedef enum
{
	VAT_Unsupported = 0,
	VAT_D3D9Ex_ShareHandle_OffscreenPlainSurfaceEx
}VA_TYPE;

typedef struct
{
	void* hSharedHandle;		// HANDLE
	unsigned int uiWidthSH;		// UINT
	unsigned int uiHeightSH;	// UINT
	int formatSH;				// D3DFORMAT
	int poolSH;					// D3DPOOL
}VAS_D3D9Ex_ShareHandle_OffscreenPlainSurfaceEx, *PVAS_D3D9Ex_ShareHandle_OffscreenPlainSurfaceEx;

class CDraw
{
public:
	virtual ~CDraw(){};
	virtual BOOL Init(HWND hWnd,IDrawHelper* pHelper)=0;
	virtual void UnInit()=0;
	virtual BOOL IsFourCCSupported(DWORD dwFourCC)=0;
	virtual VA_TYPE GetSupportedVAType()=0;
	//virtual BOOL VADraw(VA_TYPE type,void* pContent,const RECT& rcSource,const RECT& rcTarget)=0;
	virtual long Draw(LPBYTE pImage,LPBITMAPINFOHEADER pbmi,HDC hDrawToDC,const RECT& rcSource,const RECT& rcTarget)=0;
	virtual BOOL DrawBkgnd(HDC hDrawToDC,const RECT& rcTarget)=0;
	virtual void DiscardStream()=0;		// just discard video buffer,keep others
	virtual BOOL AddDrawBitmap(unsigned int uID,PBMP_DESC pParam)=0;
	virtual void RemoveDrawBitmap(unsigned int uID)=0;
	virtual void SetBKColor(BK_DESC bk)=0;
	virtual void SetAspectRatioMode(ASPECTRATIOMODE mode)=0;
	virtual void SetImageEffect(DWORD dwImageEffect)=0;
	
	virtual BOOL UpdateDrawTargetRectangle(const RECT& rcTarget)=0;
	virtual void WindowThreadAction()=0;
	virtual BOOL IsNeedWindowThreadDestroy()=0;
};

class CDraw2 : public CDraw
{
public:
	CDraw2();
	virtual ~CDraw2();
	enum emBlackListType 
	{
		kNeed128BytesAligned = 0,
	};
	BOOL AddDrawBitmap(unsigned int uID,PBMP_DESC pParam);
	void RemoveDrawBitmap(unsigned int uID);
	void SetBKColor(BK_DESC bk);
	void SetAspectRatioMode(ASPECTRATIOMODE mode);
	void SetImageEffect(DWORD dwImageEffect);

	//VA_TYPE GetSupportedVAType();
	//BOOL VADraw(VA_TYPE type,void* pContent,const RECT& rcSource,const RECT& rcTarget);

	BOOL UpdateDrawTargetRectangle(const RECT& rcTarget);
	void WindowThreadAction();
	BOOL IsNeedWindowThreadDestroy();
protected:
	BOOL HasEffect();
	BOOL HasEffectActOnSrc();
	BOOL HasEffectActOnDest();

	void TuneTargetRectangleByAspectRatioMode(RECT& rcDestTuned,RECT& rcSrcTuned,const RECT& rcSrc,const RECT& rcDest, DWORD dwSrcFourCC);
	
	DWORD m_dwImageEffect;
	ASPECTRATIOMODE m_armode;
	BK_PARAM m_bk;

	ID2BPMap m_mapID2BP;
};

//////////////////////////////////////////////////////////////////////////
//  helper function
int GetCompatibleFourCC(DWORD dwFourCC,/*out*/LPDWORD* ppdwCompatibleFourCCs);
BOOL IsFourCCMatched(DWORD dwFourCCSrc,DWORD dwFourCCDest);
BOOL BltYUV_Packed(LPBYTE pSrc,int nSrcStride,DWORD dwSrcFourCC,
				   LPBYTE pDst,int nDstPitch,DWORD dwDstFourCC,
				   UINT uiWidth,UINT uiHeight);
BOOL BltYUV_Planar(LPBYTE pSrcY,LPBYTE pSrcU,LPBYTE pSrcV,int nSrcStrideY,int nSrcStrideUV,
				   LPBYTE pDst,int nDstPitch,DWORD dwDstFourCC,
				   UINT uiWidth,UINT uiHeight);
typedef enum
{
	fmt_X8R8G8B8,
	fmt_R8G8B8,
	fmt_R5G6B5,
	fmt_X1R5G5B5
} fmt_BGR;
BOOL BltBGR(LPBYTE pSrc,int nSrcStride,
			LPBYTE pDst,int nDstPitch,fmt_BGR fmtDst,
			UINT uiWidth,UINT uiHeight);
BOOL BltYUV_Packed_Safe(LPBYTE pSrc,int nSrcStride,DWORD dwSrcFourCC,
						LPBYTE pDst,int nDstPitch,DWORD dwDstFourCC,
						UINT uiWidth,UINT uiHeight,BOOL& bException);
BOOL BltYUV_Planar_Safe(LPBYTE pSrcY,LPBYTE pSrcU,LPBYTE pSrcV,int nSrcStrideY,int nSrcStrideUV,
						LPBYTE pDst,int nDstPitch,DWORD dwDstFourCC,
						UINT uiWidth,UINT uiHeight,BOOL& bException);
BOOL BltBGR_Safe(LPBYTE pSrc,int nSrcStride,
				 LPBYTE pDst,int nDstPitch,fmt_BGR fmtDst,
				 UINT uiWidth,UINT uiHeight,BOOL& bException);
//////////////////////////////////////////////////////////////////////////
CDraw* CreateDirect3D(HWND hWnd,IDrawHelper* pHelper);
//CDraw* CreateDirect2D(HWND hWnd,IDrawHelper* pHelper);
//CDraw* CreateGDIDraw(HWND hWnd,IDrawHelper* pHelper);
//CDraw* CreateGDIPlusDraw(HWND hWnd,IDrawHelper* pHelper);

void DestoryDraw(CDraw* pDraw);

#endif // _MYDRAW_H_