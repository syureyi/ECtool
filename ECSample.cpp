#include "stdafx.h"
#include "ECSample.h"
#include "./common/typedef.h"
#include "./psnr/JSVMPsnr.h"


ECSample::ECSample()
{
	m_pSample = NULL;
	m_pDecoder = NULL;
	m_bEnableEC = false;
	m_strLossDatFileName = NULL;
	CreateVideoSampleAllocator(64,&m_pAlloc);

	
}	

ECSample::~ECSample()
{
	if(m_pSample !=NULL)
	{
		m_pSample = NULL;
		delete m_pSample;
	}
	if(m_pDecoder != NULL)
	{
		m_pDecoder = NULL;
		delete m_pDecoder;
	}
}

void ECSample::SetFileName(CString strFileName)
{
    //CStringW to char*
	const size_t newsizew = (strFileName.GetLength() + 1)*2;
     m_strBitsName= new char[newsizew];
	 m_strLossFileName = new char[newsizew+10];
    size_t convertedCharsw = 0;
    wcstombs_s(&convertedCharsw, m_strBitsName, newsizew, strFileName, _TRUNCATE );
	strcpy(m_strLossFileName,m_strBitsName);
	strcat(m_strLossFileName,".loss");
}
void ECSample::SetLossFileName(CString strFileName)
{
 	const size_t newsizew = (strFileName.GetLength() + 1)*2;
     m_strLossDatFileName= new char[newsizew];
    size_t convertedCharsw = 0;
    wcstombs_s(&convertedCharsw, m_strLossDatFileName, newsizew, strFileName, _TRUNCATE );
}

void ECSample::CleanLossFileName()
{
 	m_strLossDatFileName = NULL;
}
void ECSample::SetEnableEC()
{
	m_bEnableEC = true;
}

void ECSample::SetUnEnableEC()
{
	m_bEnableEC = false;
}

void ECSample::SetLossRatio(SSliceLossRatioInPercent *pSliceLossRatio)
{
	m_cPaketLossSimulator.SetLossRatio(pSliceLossRatio);
}

void ECSample::SetOption(DECODER_OPTION eOptionId, void* pOption)
{
	m_pDecoder->SetOption(eOptionId,pOption);
}

void ECSample::InitDecoder(CString strDecoder)
{
	CString errorMsg;
	HINSTANCE hDll;
	m_pfnWelsCreateDecoder =NULL;
	m_pfnWelsDestroyDecoder = NULL;
	SDecodingParam sDecParam = {0};
	int iErrorEnable =2;
	hDll = LoadLibrary(strDecoder);
	if(hDll ==NULL)
	{
		return;
	}
	m_pfnWelsDestroyDecoder = (WelsDestroyDecoder)GetProcAddress(hDll,"WelsDestroyDecoder");
	m_pfnWelsCreateDecoder = (WelsCreateDecoder)GetProcAddress(hDll,"WelsCreateDecoder");
	m_pfnWelsCreateDecoder(&m_pDecoder);
	sDecParam.eOutputColorFormat          = videoFormatI420;
    sDecParam.uiTargetDqLayer	          = (unsigned char) - 1;
	sDecParam.eEcActiveIdc = ERROR_CON_DISABLE;//ERROR_CON_SLICE_COPY;
    sDecParam.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_DEFAULT;
	m_pDecoder->Initialize (&sDecParam);


}

void ECSample::LossSimulator()
{

  FILE* pH264File	  = NULL;
  int64_t iStart = 0, iEnd = 0, iTotal = 0;
  int32_t iSliceSize;
  int32_t iSliceIndex = 0;
  uint8_t* pBuf = NULL;
  uint8_t uiStartCode[4] = {0, 0, 0, 1};
  FILE* pOutLossFile	  = NULL;

  uint8_t* pData[3] = {NULL};
  uint8_t* pDst[3] = {NULL};
  SBufferInfo sDstBufInfo;
  SPlayFrameInfo sFrameInfo;

  int32_t iBufPos = 0;
  int32_t iFileSize;
  int32_t i = 0;
  int32_t iLastWidth = 0, iLastHeight = 0;
  int32_t iFrameCount = 0;
  int32_t iEndOfStreamFlag = 0;
  int32_t iColorFormat = videoFormatInternal;

 FILE* pInLossFile	  = NULL;
  int iFileSizeInLossFile = 0;
   unsigned char* pBufInLoss = NULL;
 
 /* CUtils cOutputModule;
  double dElapsed = 0;*/
  if (m_strBitsName) {
	 
	  pH264File = fopen (m_strBitsName, "rb");
    if (pH264File == NULL) {
      fprintf (stderr, "Can not open h264 source file, check its legal path related please..\n");
      return;
    }
    fprintf (stderr, "H264 source file name: %s..\n", m_strBitsName);
  } else {
    fprintf (stderr, "Can not find any h264 bitstream file to read..\n");
    fprintf (stderr, "----------------decoder return------------------------\n");
    return;
  }

  fseek (pH264File, 0L, SEEK_END);
  iFileSize = (int32_t) ftell (pH264File);
  if (iFileSize <= 0) {
    fprintf (stderr, "Current Bit Stream File is too small, read error!!!!\n");
    goto label_exit;
  }
  fseek (pH264File, 0L, SEEK_SET);

  pBuf = new uint8_t[iFileSize + 4];
  if (pBuf == NULL) {
    fprintf (stderr, "new buffer failed!\n");
    goto label_exit;
  }

  if (fread (pBuf, 1, iFileSize, pH264File) != (uint32_t)iFileSize) {
    fprintf (stderr, "Unable to read whole file\n");
    goto label_exit;
  }

  memcpy (pBuf + iFileSize, &uiStartCode[0], 4); //confirmed_safe_unsafe_usage
  //if EC on
  if(m_bEnableEC)
  {
	  SOutBuffStatics sOutBuff;
	  //
	  if(m_strLossDatFileName !=NULL)
	  {

		  pInLossFile = fopen (m_strLossDatFileName, "rb");
            if (pInLossFile == NULL) {
                fprintf (stderr, "Can not open input loss file, check its legal path %s related please..\n", m_strLossDatFileName);
                return;
            } else {
                fseek (pInLossFile, 0L, SEEK_END);
                iFileSizeInLossFile = (int) ftell (pInLossFile);
                pBufInLoss = new unsigned char[iFileSizeInLossFile+4];
                fseek (pInLossFile, 0L, SEEK_SET);
                if (fread (pBufInLoss, 1, iFileSizeInLossFile, pInLossFile) != (unsigned int)iFileSizeInLossFile) {
                    fprintf (stderr, "Unable to read whole pInLossFile file\n");
                    fclose(pInLossFile);
                    return ;
                }
            }

	  }
	  if(iFileSizeInLossFile>0)
		  sOutBuff = m_cPaketLossSimulator.SimulateNALLoss(pBuf,iFileSize,pBufInLoss,iFileSizeInLossFile);
	  else
	  sOutBuff = m_cPaketLossSimulator.SimulateNALLoss(pBuf,iFileSize);

	  memcpy(pBuf,sOutBuff.pBuff,sizeof(uint8_t)*sOutBuff.len);
	  iFileSize = sOutBuff.len;
  }
  pOutLossFile = fopen(m_strLossFileName, "wb");
  if (pOutLossFile == NULL) {
                fprintf (stderr, "Can not open output loss file, check its legal path %s related please..\n", m_strLossFileName);
                return ;
            }
   if (pOutLossFile&&iFileSize>0) {
        // store actual Loss Status to file. 1: lost, 0: remain;
	   fwrite (pBuf, 1, iFileSize, pOutLossFile);
    }
   label_exit:
  if (pBuf) {
    delete[] pBuf;
    pBuf = NULL;
  }
  if (pH264File) {
    fclose (pH264File);
    pH264File = NULL;
  }
   if (pOutLossFile) {
        fclose(pOutLossFile);
    }
   if(pInLossFile)
   {
	   fclose(pInLossFile);
	   
   }
   if(pBufInLoss)
   {
	   delete[] pBufInLoss;
	   pBufInLoss = NULL;
   }
    

}
int ECSample::Decoder(ECplayControl* pPlayControl,DEMO_WINDOW_TYPE eWindowType)
{

  FILE* pH264File	  = NULL;
  int64_t iStart = 0, iEnd = 0, iTotal = 0;
  int32_t iSliceSize;
  int32_t iSliceIndex = 0;
  uint8_t* pBuf = NULL;
  uint8_t uiStartCode[4] = {0, 0, 0, 1};

  uint8_t* pData[3] = {NULL};
  uint8_t* pDst[3] = {NULL};
  SBufferInfo sDstBufInfo;
  SPlayFrameInfo sFrameInfo;

  int32_t iBufPos = 0;
  int32_t iFileSize;
  int32_t i = 0;
  int32_t iLastWidth = 0, iLastHeight = 0;
  int32_t iFrameCount = 0;
  int32_t iEndOfStreamFlag = 0;
  int32_t iColorFormat = videoFormatInternal;
 
 /* CUtils cOutputModule;
  double dElapsed = 0;*/
  char* inFileName;
  if(!m_bEnableEC)
	  inFileName = m_strBitsName;
  else
	  inFileName = m_strLossFileName;

  if (m_pDecoder == NULL) return 0;
  if (inFileName) {
	 
	  pH264File = fopen (inFileName, "rb");
    if (pH264File == NULL) {
      fprintf (stderr, "Can not open h264 source file, check its legal path related please..\n");
      return 0;
    }
    fprintf (stderr, "H264 source file name: %s..\n", inFileName);
  } else {
    fprintf (stderr, "Can not find any h264 bitstream file to read..\n");
    fprintf (stderr, "----------------decoder return------------------------\n");
    return 0;
  }

  fseek (pH264File, 0L, SEEK_END);
  iFileSize = (int32_t) ftell (pH264File);
  if (iFileSize <= 0) {
    fprintf (stderr, "Current Bit Stream File is too small, read error!!!!\n");
    goto label_exit;
  }
  fseek (pH264File, 0L, SEEK_SET);

  pBuf = new uint8_t[iFileSize + 4];
  if (pBuf == NULL) {
    fprintf (stderr, "new buffer failed!\n");
    goto label_exit;
  }

  if (fread (pBuf, 1, iFileSize, pH264File) != (uint32_t)iFileSize) {
    fprintf (stderr, "Unable to read whole file\n");
    goto label_exit;
  }

  memcpy (pBuf + iFileSize, &uiStartCode[0], 4); //confirmed_safe_unsafe_usage

 /* if (m_pDecoder->SetOption (DECODER_OPTION_DATAFORMAT,  &iColorFormat)) {
    fprintf (stderr, "SetOption() failed, opt_id : %d  ..\n", DECODER_OPTION_DATAFORMAT);
    goto label_exit;
  }*/
  //if EC on
  //if(m_bEnableEC)
  //{
	 // SOutBuffStatics sOutBuff;
	 // sOutBuff = m_cPaketLossSimulator.SimulateNALLoss(pBuf,iFileSize);
	 // memcpy(pBuf,sOutBuff.pBuff,sizeof(uint8_t)*sOutBuff.len);
	 // memcpy (pBuf + iFileSize, &uiStartCode[0], 4); 
	 // iFileSize = sOutBuff.len;
  //}
  while (true) {

    if (iBufPos >= iFileSize) {
      iEndOfStreamFlag = true;
      if (iEndOfStreamFlag)
        m_pDecoder->SetOption (DECODER_OPTION_END_OF_STREAM, (void*)&iEndOfStreamFlag);
      break;
    }


    for (i = 0; i < iFileSize; i++) {
      if ((pBuf[iBufPos + i] == 0 && pBuf[iBufPos + i + 1] == 0 && pBuf[iBufPos + i + 2] == 0 && pBuf[iBufPos + i + 3] == 1
		  && i > 0)) {//|| (pBuf[iBufPos + i] == 0 && pBuf[iBufPos + i + 1] == 0 && pBuf[iBufPos + i + 2] == 1 && i > 0)) {
        break;
      }
    }
    iSliceSize = i;   
    pData[0] = NULL;
    pData[1] = NULL;
    pData[2] = NULL;
    memset (&sDstBufInfo, 0, sizeof (SBufferInfo));

    m_pDecoder->DecodeFrame2 (pBuf + iBufPos, iSliceSize, pData, &sDstBufInfo);
   // CUtils cOutputModule;
    if (sDstBufInfo.iBufferStatus == 1) {
      pDst[0] = pData[0];
      pDst[1] = pData[1];
      pDst[2] = pData[2];
	  FrameToPlay(pPlayControl,eWindowType,sDstBufInfo,pData,iFrameCount);

      ++ iFrameCount;
     
    }

    iBufPos += iSliceSize;
    ++ iSliceIndex;
  }

  // Get pending last frame
  pData[0] = NULL;
  pData[1] = NULL;
  pData[2] = NULL;
  memset (&sDstBufInfo, 0, sizeof (SBufferInfo));

  m_pDecoder->DecodeFrame2 (NULL, 0, pData, &sDstBufInfo);
  if (sDstBufInfo.iBufferStatus == 1) {
    pDst[0] = pData[0];
    pDst[1] = pData[1];
    pDst[2] = pData[2];
	FrameToPlay(pPlayControl,eWindowType,sDstBufInfo,pData,iFrameCount);  
    ++ iFrameCount;
  }

  // coverity scan uninitial
label_exit:
  if (pBuf) {
    delete[] pBuf;
    pBuf = NULL;
  }
  if (pH264File) {
    fclose (pH264File);
    pH264File = NULL;
  }
  return iFrameCount;
  
}
void ECSample::FrameToPlay(ECplayControl* pPlayControl,DEMO_WINDOW_TYPE eWindowType,SBufferInfo sDstBufInfo,unsigned char** ppDst,int iFrameCount)
{

	IWseVideoSample* pSample = NULL;
	SPlayFrameInfo sFrameInfo;
	
	unsigned long iHeight = sDstBufInfo.UsrData.sSystemBuffer.iHeight;
	unsigned long iWidth  = sDstBufInfo.UsrData.sSystemBuffer.iWidth;
	unsigned long iLen = iHeight*iWidth*3/2;
	unsigned long iLength = iHeight*iWidth;
	unsigned long iStrideY = sDstBufInfo.UsrData.sSystemBuffer.iStride[0];
	unsigned long iStrideUV = sDstBufInfo.UsrData.sSystemBuffer.iStride[1];
	sFrameInfo.iFrameHeight =iHeight;
	sFrameInfo.iFrameWidth =iWidth;
	m_pAlloc->GetSample(iLen,&pSample);		
	pSample->GetDataLength(&iLen);
	m_format.height = iHeight;
	// m_format.video_type = (WseVideoType)sDstBufInfo.UsrData.sSystemBuffer.iFormat;
	m_format.video_type = WseI420;
    m_format.width = iWidth;
	pSample->SetVideoFormat(&m_format);
	unsigned char* pDataBlock = new unsigned char[iLen];
	memset(pDataBlock,0,iLen+1);
	pSample->GetDataPointer(&pDataBlock);
	if(iStrideY != iWidth)
	  {
		  for(int i=0;i<iHeight;i++)
		  {
			  memcpy(pDataBlock+(i*iWidth),ppDst[0],sizeof(unsigned char)*iWidth);
			  ppDst[0]+=iStrideY;
		  }
		  for(int j=0;j<(iHeight/2);j++)
		  {
			  memcpy(pDataBlock+iLength+(j*iWidth/2),ppDst[1],sizeof(unsigned char)*iWidth/2);
				  ppDst[1]+=iStrideUV;
		  }
		  for(int k=0;k<(iHeight/2);k++)
		  {
			  memcpy(pDataBlock+(iLength*5/4)+(k*iWidth/2),ppDst[2],sizeof(unsigned char)*iWidth/2);
				  ppDst[2]+=iStrideUV;
		  }
		  

	  }
	else
	  {
	  memcpy(pDataBlock,ppDst[0],sizeof(unsigned char)*iLength);
	  memcpy(pDataBlock+iLength,ppDst[1],sizeof(unsigned char)*iLength*1/4);
	  memcpy(pDataBlock+(iLength*5/4),ppDst[2],sizeof(unsigned char)*iLength*1/4);
	  }

	sFrameInfo.pSample = pSample;
	sFrameInfo.iFrameNo = iFrameCount;
	//if(eWindowType == DEMO_WINDOW_ORIG)
         sFrameInfo.fPsnr= 0;
	//else
	//sFrameInfo.fPsnr = pPlayControl->ECsetPsnrInfo(pDataBlock);

	sFrameInfo.iScore = 0;
	pPlayControl->ECsetPlayComment(eWindowType,sFrameInfo);
	

	 /* unsigned char* p1Data[3] = {NULL};    
      p1Data[0]	= pDataBlock;	
	  memcpy(p1Data[0],pDataBlock,sizeof(unsigned char)*iLength);
	  p1Data[1] = pDataBlock+iLength;
      memcpy(p1Data[1],pDataBlock+iLength,sizeof(unsigned char)*iLength/4);
	  p1Data[2] = pDataBlock+iLength*5/4;
	  memcpy(p1Data[2],pDataBlock+(iLength*5/4),sizeof(unsigned char)*iLength/4);*/
 
	  //FILE* pYuvFile	  = NULL;
	//  pYuvFile = fopen ("test.yuv", "wb");
	  //sDstBufInfo.UsrData.sSystemBuffer.iStride[0] = 384;
      //sDstBufInfo.UsrData.sSystemBuffer.iStride[1] = 192;
	 // cOutputModule.Process ((void**)p1Data, &sDstBufInfo, pYuvFile);	
	 // fclose(pYuvFile);
	 //delete[] pDataBlock;
	 pDataBlock =NULL;
	 pSample = NULL;
	 // goto label_exit;
}


void ECSample::UninitDecoder()
{

	m_pDecoder->Uninitialize();
	m_pfnWelsDestroyDecoder(m_pDecoder);

}