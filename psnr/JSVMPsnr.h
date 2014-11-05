#ifndef _JSVM_PSNR_H_
#define _JSVM_PSNR_H_
#include "../stdafx.h"
#include <stdio.h>
#include <list>

typedef struct
{
  int             width;
  int             height;
  unsigned char*  data;
} ColorComponent;

typedef struct
{
  ColorComponent lum;
  ColorComponent cb;
  ColorComponent cr;
} YuvFrame;

class CJSVMPsnr
{
public:
	 CJSVMPsnr(int width,int height, unsigned char* orgBuf,unsigned char* recBuf);
	// CJSVMPsnr::CJSVMPsnr(int width,int height, unsigned char* orgBuf,unsigned char* recBuf);
	~CJSVMPsnr();
	void createColorComponent( ColorComponent* cc );
	void deleteColorComponent( ColorComponent* cc );
	void createFrame( YuvFrame* f, int width, int height );
	void deleteFrame( YuvFrame* f );
	void readColorComponent( ColorComponent* cc, FILE* file );
	void writeColorComponent( ColorComponent* cc, FILE* file, int downScale );
	double psnr( ColorComponent& rec, ColorComponent& org);
	void getPSNR( double& psnrY, double& psnrU, double& psnrV, YuvFrame& rcFrameOrg, YuvFrame& rcFrameRec );
	void readFrame( YuvFrame* f, FILE* file );
	void initYUVFrame();
	void uninitYUVFrame();
	void readYUVFrame(FILE* org,FILE* rec);
	double getFramePSNR();
private:
	int m_iWidth;
	int m_iHeight;
	YuvFrame m_OrigFile;
	YuvFrame m_RecFile;
};






#endif