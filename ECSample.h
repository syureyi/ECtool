#ifndef _EC_SAMPLE_H_
#define _EC_SAMPLE_H_
#include "stdafx.h"
#include "ECtoolParam.h"
#include "./api/svc/codec_api.h"
#include "././PaketLoss/SimulatePacketLoss.h"
#include "ECplayControl.h"
class ECSample
{
public:
	ECSample();
	virtual ~ECSample();
    void DoDecoder();
	void ReadYuvFile();
	void RenderSampleGenerate();
	void SetFileName(CString strFileName);
	void InitDecoder(CString strDecoder);
	int Decoder(ECplayControl* pPlaysControl,DEMO_WINDOW_TYPE eWindowType);
	void UninitDecoder();
	void SetEnableEC();
	void SetOption(DECODER_OPTION eOptionId, void* pOption);
	void SetLossRatio(SSliceLossRatioInPercent *pSliceLossRatio);
	void FrameToPlay(ECplayControl* pPlayControl,DEMO_WINDOW_TYPE eWindowType,SBufferInfo sDstBufInfo,unsigned char** ppDst,int iFrameCount);
	void LossSimulator();
	void SetUnEnableEC();
	void SetLossFileName(CString strFileName);
	void CalcPSNR();
	void CleanLossFileName();
private:
	IWseVideoSampleAllocator*  m_pAlloc;
	WseVideoFormat m_format;

	//ECplayControl m_Control;
	IWseVideoSample* m_pSample;
	ISVCDecoder* m_pDecoder;
	char* m_strBitsName;
	char* m_strLossFileName;
	char* m_strLossDatFileName;
	int m_iWidth;
	int m_iHeight;
	typedef void (*WelsCreateDecoder)(ISVCDecoder**);
	WelsCreateDecoder m_pfnWelsCreateDecoder;
	typedef void (*WelsDestroyDecoder)(ISVCDecoder*);
	WelsDestroyDecoder m_pfnWelsDestroyDecoder;
	CPacketLossSimulator m_cPaketLossSimulator;
	bool m_bEnableEC;
};
#endif