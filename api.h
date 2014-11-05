#ifndef _API_H_
#define _API_H_


#define WSERESULT  unsigned long
#ifdef WIN32
#define JL_NOVTABLE		__declspec(novtable)
#define JLCALLTYPE		__stdcall
#else
#define JL_NOVTABLE
#define JLCALLTYPE
#endif

#ifdef WIN32
#if defined(WSE_DLL_EXPORT)
#define WSE_EXPORT _declspec(dllexport)
#else
#define WSE_EXPORT _declspec(dllimport)
#endif
#endif

#define MAX_PLANAR_NUM 4


typedef struct 
{
    HBITMAP bitMap;	// 32bit pargb bitmap
	long lMask;
	float fleft;
	float ftop;
	float fright;
	float fbottom;
} MMD_BMP_RENDER_STRUCT, *PMMD_BMP_RENDER_STRUCT;
typedef enum
{
	Wse_RM_D3D      = 0,
	Wse_RM_GDI		= 1,
	WSE_RM_D2D		= 2
} WseRendererMode;
typedef enum
{
	Wse_RARM_NONE      = 0,
	Wse_RARM_LETTER_BOX,
	Wse_RARM_STRETCH,
	Wse_RARM_ORIGINAL,
} WseRendererAspectRatioMode;

#define BMP_RENDER_MASK_REVERSE_ORIGIN_LEFT			0x00000001
#define BMP_RENDER_MASK_REVERSE_ORIGIN_RIGHT		0x00000002
#define BMP_RENDER_MASK_REVERSE_ORIGIN_TOP			0x00000004
#define BMP_RENDER_MASK_REVERSE_ORIGIN_BOTTOM		0x00000008

typedef enum
{
    WseUnknown   = 0,
	/*yuv color formats*/    
	WseI420,      		   
	WseYV12,
	WseNV12,
	WseNV21,
	WseYUY2,
	/*rgb color formats*/
	WseRGB24, 
	WseBGR24,  
	WseRGB24Flip,
	WseBGR24Flip,
	WseRGBA32,
	WseBGRA32,
	WseARGB32,
	WseABGR32,
	WseRGBA32Flip,
	WseBGRA32Flip,
	WseARGB32Flip,
	WseABGR32Flip,

	WseRTPStream,
	WseSVCStream,
	WseAVCStream,
	WseHEVCStream,
} WseVideoType;
typedef struct video_frame_struct_
{
	WseVideoType	video_type;
	unsigned long	width;			
	unsigned long	height;
	float	        frame_rate;
	unsigned long   time_stamp;
} WseVideoFormat;
typedef struct
{
    unsigned char *pSrcData[MAX_PLANAR_NUM];
    unsigned int   uiSrcStride[MAX_PLANAR_NUM];
    WseVideoFormat SrcFormat;
    unsigned int   rotation;
    unsigned long  uDataLen;
} VideoRawDataPack;

class IWseVideoSample
{
public:
	virtual WSERESULT GetSize(unsigned long* pulSize)=0;
	virtual WSERESULT GetPointer(unsigned char **ppBuffer)=0;
	virtual WSERESULT GetDataPointer(unsigned char **ppBuffer)=0;
	virtual WSERESULT GetDataLength(unsigned long* pulDataLength)=0;
	virtual WSERESULT GetDataPlanarPointer(unsigned char **ppBuffer, int idx)=0;
	virtual WSERESULT GetDataStride(unsigned int *puiStride, int idx)=0;
    virtual WSERESULT GetVideoFormat(WseVideoFormat *pFormat)=0;
    virtual WSERESULT SetDataPointer(unsigned char *pBuffer)=0;
	virtual WSERESULT SetDataLength(unsigned long ulDataLength)=0;
	virtual WSERESULT SetDataPlanarPointer(unsigned char *pBuffer, int idx)=0;
	virtual WSERESULT SetDataStride(unsigned int uiStride, int idx)=0;
    virtual WSERESULT SetVideoFormat(WseVideoFormat *pFormat)=0;
};

class  IWseVideoSampleAllocator 
{
public:
	virtual WSERESULT GetSample(unsigned long ulBufferSize,IWseVideoSample** ppSample)=0;
};

class JL_NOVTABLE IWseVideoDeliverer
{
public:
//	virtual WSERESULT DeliverImage(unsigned char* pData,unsigned int iLen,WseVideoFormat* format)=0;
	virtual WSERESULT DeliverImage(IWseVideoSample* pSample)=0;
};

class JL_NOVTABLE IWseVideoRendererWinProfile
{
public:
	virtual WSERESULT OnDisplayChange()=0;
	virtual WSERESULT OnWindowPositionChange()=0;
	//virtual WSERESULT SetRendererMode(WseRenderMode mode)=0;

};
class JL_NOVTABLE IWseVideoRenderer : public IWseVideoDeliverer,
	                                  public IWseVideoRendererWinProfile
{
public:
	virtual WSERESULT AddRenderPicture(unsigned int uiID,PMMD_BMP_RENDER_STRUCT pPicData) = 0;
	virtual WSERESULT RemovePicture(unsigned int uiID) = 0;
	virtual WSERESULT GetRenderPictureDesc(unsigned int uiID,PMMD_BMP_RENDER_STRUCT pPicData) = 0;
	virtual WSERESULT SetBKColor(void* color)=0;
	virtual WSERESULT ClearStream()=0;

	virtual WSERESULT LockDraw()=0;
	virtual WSERESULT UnlockDraw()=0;
	virtual WSERESULT ReDraw()=0;

	virtual WSERESULT SetAspectRatioMode(WseRendererAspectRatioMode mode)=0;
	virtual WSERESULT GetAspectRatioMode(WseRendererAspectRatioMode *pmode)=0;
};

//WSE_EXPORT WSERESULT CreateVideoRenderer(void* hRenderWnd,IWseVideoRenderer** ppVideoRenderer);
//WSE_EXPORT WSERESULT CreateVideoSampleAllocator(unsigned long ulAlignment,IWseVideoSampleAllocator** ppVideoSampleAllocator);
WSERESULT CreateVideoRenderer(void* hRenderWnd,IWseVideoRenderer** ppVideoRenderer);
WSERESULT CreateVideoSampleAllocator(unsigned long ulAlignment,IWseVideoSampleAllocator** ppVideoSampleAllocator);

#endif