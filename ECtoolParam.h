#ifndef _TOOL_PARAM_H_
#define _TOOL_PARAM_H_

#include <list>
#include "api.h"
#define MAX_BUFFER_LEN 1000000
typedef struct
{
	unsigned char ucData[MAX_BUFFER_LEN];
	unsigned long ulLen;
}DATABLOCK, *PDATABLOCK;

typedef enum
{
    DEMO_WINDOW_NONE		= 0,
    DEMO_WINDOW_ORIG		= 1,
    DEMO_WINDOW_DEC1	    = 2,
	DEMO_WINDOW_DEC2		= 3,
	DEMO_WINDOW_DESKSHARING = 4, // whsu
}DEMO_WINDOW_TYPE;

#define WM_MESSAGE_INFO_WINDOW_CLOSE (WM_USER+400)
#define WM_MESSAGE_CLOSE_VIDEO_WINDOW (WM_USER+401)
#define WM_MESSAGE_VIDEO_WINDOW_POSITION_CHANGE (WM_USER+402)
#define WM_MESSAGE_VIDEO_WINDOW_DISPLAY_CHANGE (WM_USER+403)
#define WM_MESSAGE_PAINT (WM_USER+404)
#define WM_MESSAGE_CHANGE_EDIT (WM_USER+405)

typedef struct
{
    DEMO_WINDOW_TYPE type;
    void* vp;
    void* windowHandle;
}WINDOW_INFORMATION;

typedef struct
{
	WseVideoFormat format;
	IWseVideoSample *sample;
}FrameData;

typedef struct
{
	IWseVideoSample* pSample;
	int iFrameNo;
	int iFrameWidth;
	int iFrameHeight;
	double fPsnr;
	int iScore;
}SPlayFrameInfo;

typedef struct
{
	int iFrameNo;
	int iFrameWidth;
	int iFrameHeight;
	double fPsnr;
	int iScore;
}SFrameInfo;

#endif