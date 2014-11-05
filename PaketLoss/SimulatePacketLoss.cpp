#include "SimulatePacketLoss.h"
#include "assert.h"

CPacketLossSimulator::CPacketLossSimulator ()
{
  m_pOutPutBuff = NULL;
  m_pOutPutLossBuff = NULL;
  m_iOutPutLen = 0;
  m_ListSLostSim.clear();
  iLossIdx = 0;
  memset(m_SliceLossRatio,0,32*sizeof(int));
  memset(m_ActualLoss,0,32*sizeof(int));
  memset(m_TotalReceived,0,32*sizeof(int));
}

CPacketLossSimulator::~CPacketLossSimulator () {
  m_ListSLostSim.clear();
  if (m_pOutPutBuff) {
    delete [] m_pOutPutBuff;
  }
  if (m_pOutPutLossBuff) {
    delete [] m_pOutPutLossBuff;
  }
}

void CPacketLossSimulator::SetLossRatio (SSliceLossRatioInPercent* pLossRatio) {
  m_SliceLossRatio[PLS_NAL_UNIT_SPS] = pLossRatio->iSPSLossRatio;
  m_SliceLossRatio[PLS_NAL_UNIT_SUBSET_SPS] = pLossRatio->iSubSPSLossRatio;
  m_SliceLossRatio[PLS_NAL_UNIT_PPS] = pLossRatio->iPPSLossRatio;
  m_SliceLossRatio[PLS_NAL_UNIT_PREFIX] = pLossRatio->iPrefixLossRatio;
  m_SliceLossRatio[PLS_NAL_UNIT_CODED_SLICE_IDR] = pLossRatio->iAVCISliceLossRatio;
  m_SliceLossRatio[PLS_NAL_UNIT_CODED_SLICE] = pLossRatio->iAVCPSliceLossRatio;
  m_SliceLossRatio[PLS_NAL_UNIT_CODED_SLICE_EXT] = pLossRatio->iSVCSliceExtLossRatio;
}

SSliceLossRatioInPercent CPacketLossSimulator::GetPresetLossRatio () {
  SSliceLossRatioInPercent sLossRatio;
  sLossRatio.iSPSLossRatio = m_SliceLossRatio[PLS_NAL_UNIT_SPS];
  sLossRatio.iSubSPSLossRatio = m_SliceLossRatio[PLS_NAL_UNIT_SUBSET_SPS];
  sLossRatio.iPPSLossRatio = m_SliceLossRatio[PLS_NAL_UNIT_PPS];
  sLossRatio.iPrefixLossRatio = m_SliceLossRatio[PLS_NAL_UNIT_PREFIX];
  sLossRatio.iAVCISliceLossRatio = m_SliceLossRatio[PLS_NAL_UNIT_CODED_SLICE_IDR];
  sLossRatio.iAVCPSliceLossRatio = m_SliceLossRatio[PLS_NAL_UNIT_CODED_SLICE];
  sLossRatio.iSVCSliceExtLossRatio = m_SliceLossRatio[PLS_NAL_UNIT_CODED_SLICE_EXT];
  return sLossRatio;
}

SLossstatusInfo CPacketLossSimulator::GetLossStatus ( ) {
  SLossstatusInfo sLossInfo;
  int iTotalLostSlices = 0;
  int iTotalReceivedSlices = 0;
  for (int i=0; i<32; i++) {
    iTotalLostSlices+=m_ActualLoss[i];
    iTotalReceivedSlices+=m_TotalReceived[i];
  }
  sLossInfo.sActualLostSlicesNum.iSPSLossRatio = m_ActualLoss[PLS_NAL_UNIT_SPS];
  sLossInfo.sActualLostSlicesNum.iSubSPSLossRatio = m_ActualLoss[PLS_NAL_UNIT_SUBSET_SPS];
  sLossInfo.sActualLostSlicesNum.iPPSLossRatio = m_ActualLoss[PLS_NAL_UNIT_PPS];
  sLossInfo.sActualLostSlicesNum.iPrefixLossRatio = m_ActualLoss[PLS_NAL_UNIT_PREFIX];
  sLossInfo.sActualLostSlicesNum.iAVCISliceLossRatio = m_ActualLoss[PLS_NAL_UNIT_CODED_SLICE_IDR];
  sLossInfo.sActualLostSlicesNum.iAVCPSliceLossRatio = m_ActualLoss[PLS_NAL_UNIT_CODED_SLICE];
  sLossInfo.sActualLostSlicesNum.iSVCSliceExtLossRatio = m_ActualLoss[PLS_NAL_UNIT_CODED_SLICE_EXT];
  sLossInfo.sActualLostSlicesNum.iOther = iTotalLostSlices - m_ActualLoss[PLS_NAL_UNIT_SPS] - m_ActualLoss[PLS_NAL_UNIT_SUBSET_SPS]
                      - m_ActualLoss[PLS_NAL_UNIT_PPS] - m_ActualLoss[PLS_NAL_UNIT_PREFIX]
                      - m_ActualLoss[PLS_NAL_UNIT_CODED_SLICE_IDR] - m_ActualLoss[PLS_NAL_UNIT_CODED_SLICE]
                      - m_ActualLoss[PLS_NAL_UNIT_CODED_SLICE_EXT];

  sLossInfo.sTotalReceivedNum.iSPSLossRatio = m_TotalReceived[PLS_NAL_UNIT_SPS];
  sLossInfo.sTotalReceivedNum.iSubSPSLossRatio = m_TotalReceived[PLS_NAL_UNIT_SUBSET_SPS];
  sLossInfo.sTotalReceivedNum.iPPSLossRatio = m_TotalReceived[PLS_NAL_UNIT_PPS];
  sLossInfo.sTotalReceivedNum.iPrefixLossRatio = m_TotalReceived[PLS_NAL_UNIT_PREFIX];
  sLossInfo.sTotalReceivedNum.iAVCISliceLossRatio = m_TotalReceived[PLS_NAL_UNIT_CODED_SLICE_IDR];
  sLossInfo.sTotalReceivedNum.iAVCPSliceLossRatio = m_TotalReceived[PLS_NAL_UNIT_CODED_SLICE];
  sLossInfo.sTotalReceivedNum.iSVCSliceExtLossRatio = m_TotalReceived[PLS_NAL_UNIT_CODED_SLICE_EXT];
  sLossInfo.sTotalReceivedNum.iOther = iTotalReceivedSlices - m_TotalReceived[PLS_NAL_UNIT_SPS] - m_TotalReceived[PLS_NAL_UNIT_SUBSET_SPS]
                      - m_TotalReceived[PLS_NAL_UNIT_PPS] - m_TotalReceived[PLS_NAL_UNIT_PREFIX]
                      - m_TotalReceived[PLS_NAL_UNIT_CODED_SLICE_IDR] - m_TotalReceived[PLS_NAL_UNIT_CODED_SLICE]
                      - m_TotalReceived[PLS_NAL_UNIT_CODED_SLICE_EXT];

  sLossInfo.fTotalLostRatio = (iTotalReceivedSlices!=0)? (iTotalLostSlices*1.0/iTotalReceivedSlices): 0.0;
  sLossInfo.fSPSLossRatio = (sLossInfo.sTotalReceivedNum.iSPSLossRatio!=0)? (sLossInfo.sActualLostSlicesNum.iSPSLossRatio*1.0/sLossInfo.sTotalReceivedNum.iSPSLossRatio): 0.0;
  sLossInfo.fSubSPSLossRatio = (sLossInfo.sTotalReceivedNum.iSubSPSLossRatio!=0)? (sLossInfo.sActualLostSlicesNum.iSubSPSLossRatio*1.0/sLossInfo.sTotalReceivedNum.iSubSPSLossRatio): 0.0;
  sLossInfo.fPPSLossRatio = (sLossInfo.sTotalReceivedNum.iPPSLossRatio!=0)? (sLossInfo.sActualLostSlicesNum.iPPSLossRatio*1.0/sLossInfo.sTotalReceivedNum.iPPSLossRatio): 0.0;
  sLossInfo.fPrefixLossRatio = (sLossInfo.sTotalReceivedNum.iPrefixLossRatio!=0)? (sLossInfo.sActualLostSlicesNum.iPrefixLossRatio*1.0/sLossInfo.sTotalReceivedNum.iPrefixLossRatio): 0.0;
  sLossInfo.fSVCSliceExtLossRatio = (sLossInfo.sTotalReceivedNum.iSVCSliceExtLossRatio!=0)? (sLossInfo.sActualLostSlicesNum.iSVCSliceExtLossRatio*1.0/sLossInfo.sTotalReceivedNum.iSVCSliceExtLossRatio): 0.0;
  sLossInfo.fAVCISliceLossRatio = (sLossInfo.sTotalReceivedNum.iAVCISliceLossRatio!=0)? (sLossInfo.sActualLostSlicesNum.iAVCISliceLossRatio*1.0/sLossInfo.sTotalReceivedNum.iAVCISliceLossRatio): 0.0;
  sLossInfo.fAVCPSliceLossRatio = (sLossInfo.sTotalReceivedNum.iAVCPSliceLossRatio!=0)? (sLossInfo.sActualLostSlicesNum.iAVCPSliceLossRatio*1.0/sLossInfo.sTotalReceivedNum.iAVCPSliceLossRatio): 0.0;
  sLossInfo.fOther = (sLossInfo.sTotalReceivedNum.iOther!=0)? (sLossInfo.sActualLostSlicesNum.iOther*1.0/sLossInfo.sTotalReceivedNum.iOther): 0.0;
  
  sLossInfo.pLostDetail = GetLossDetail();

  delete [] m_pOutPutLossBuff;
  m_pOutPutLossBuff = NULL;
  m_pOutPutLossBuff = new unsigned char[m_ListSLostSim.size()];
  std::vector<SLostStatics>::iterator iter = m_ListSLostSim.begin();
  for (int i=0; i< m_ListSLostSim.size(); i++)
  {
    m_pOutPutLossBuff[i] = iter->isLost?'0':'1';
    iter++;
  }
  sLossInfo.pLossBuf = m_pOutPutLossBuff;
  sLossInfo.iLossBufLen = m_ListSLostSim.size();
  return sLossInfo;
}

SOutBuffStatics CPacketLossSimulator::SimulateNALLoss (const unsigned char* pSrc,  int iSrcLen) {
  delete [] m_pOutPutBuff;
  m_pOutPutBuff = NULL;
  m_pOutPutBuff = new unsigned char[iSrcLen];
  SOutBuffStatics sBuffStatics;
  m_iOutPutLen = 0;
  int iBufPos = 0;
  int ilastprefixlen = 0;
  int i = 0;
  bool bLost;
  SLostStatics tmpSLostSim;
  m_ListSLostSim.clear();
  for (i = 0; i < iSrcLen;) {
    if (pSrc[i] == 0 && pSrc[i + 1] == 0 && pSrc[i + 2] == 0 && pSrc[i + 3] == 1) {
      if (i - iBufPos) {
        tmpSLostSim.eNalType = (EPlsNalUnitType) ((* (pSrc + iBufPos + ilastprefixlen)) & 0x1f);	// eNalUnitType
        m_TotalReceived[tmpSLostSim.eNalType]++;
        bLost = (rand() % 100) < m_SliceLossRatio[tmpSLostSim.eNalType] ? true: false;
        tmpSLostSim.isLost = bLost;
        m_ListSLostSim.push_back (tmpSLostSim);
        if (!bLost) {
          memcpy (m_pOutPutBuff + m_iOutPutLen, pSrc + iBufPos, i - iBufPos);
          m_iOutPutLen += (i - iBufPos);
        } else {
          m_ActualLoss[tmpSLostSim.eNalType]++;
        }
      }
      ilastprefixlen = 4;
      iBufPos = i;
      i = i + 4;
    } else if (pSrc[i] == 0 && pSrc[i + 1] == 0 && pSrc[i + 2] == 1) {
      if (i - iBufPos) {
        tmpSLostSim.eNalType = (EPlsNalUnitType) ((* (pSrc + iBufPos + ilastprefixlen)) & 0x1f);	// eNalUnitType
        m_TotalReceived[tmpSLostSim.eNalType]++;
        bLost = (rand() % 100) < m_SliceLossRatio[tmpSLostSim.eNalType] ? true: false;
        tmpSLostSim.isLost = bLost;
        m_ListSLostSim.push_back (tmpSLostSim);
        if (!bLost) {
          memcpy (m_pOutPutBuff + m_iOutPutLen, pSrc + iBufPos, i - iBufPos);
          m_iOutPutLen += (i - iBufPos);
        } else {
          m_ActualLoss[tmpSLostSim.eNalType]++;
        }
      }
      ilastprefixlen = 3;
      iBufPos = i;
      i = i + 3;
    } else {
      i++;
    }
  }
  if (i - iBufPos) {
    tmpSLostSim.eNalType = (EPlsNalUnitType) ((* (pSrc + iBufPos + ilastprefixlen)) & 0x1f);	// eNalUnitType
    m_TotalReceived[tmpSLostSim.eNalType]++;
    bLost = (rand() % 100) < m_SliceLossRatio[tmpSLostSim.eNalType] ? true: false;
    tmpSLostSim.isLost = bLost;
    m_ListSLostSim.push_back (tmpSLostSim);
    if (!bLost) {
      memcpy (m_pOutPutBuff + m_iOutPutLen, pSrc + iBufPos, i - iBufPos);
      m_iOutPutLen += (i - iBufPos);
    } else {
      m_ActualLoss[tmpSLostSim.eNalType]++;
    }
  }
  sBuffStatics.pBuff = m_pOutPutBuff;
  sBuffStatics.len = m_iOutPutLen;
  return sBuffStatics;
}

SOutBuffStatics CPacketLossSimulator::SimulateNALLoss (const unsigned char* pSrc,  int iSrcLen, unsigned char* pLossChars, int iLossCharLen) {
  delete [] m_pOutPutBuff;
  m_pOutPutBuff = NULL;
  m_pOutPutBuff = new unsigned char[iSrcLen];
  SOutBuffStatics sBuffStatics;
  m_iOutPutLen = 0;
  int iBufPos = 0;
  int ilastprefixlen = 0;
  int i = 0;
  bool bLost;
  SLostStatics tmpSLostSim;
  m_ListSLostSim.clear();
  for (i = 0; i < iSrcLen;) {
    if (pSrc[i] == 0 && pSrc[i + 1] == 0 && pSrc[i + 2] == 0 && pSrc[i + 3] == 1) {
      if (i - iBufPos) {
        tmpSLostSim.eNalType = (EPlsNalUnitType) ((* (pSrc + iBufPos + ilastprefixlen)) & 0x1f);	// eNalUnitType
        m_TotalReceived[tmpSLostSim.eNalType]++;
        if(m_SliceLossRatio[tmpSLostSim.eNalType]==-1) {
            bLost = false;
        }
        else {
            bLost = iLossIdx < iLossCharLen ? (pLossChars[iLossIdx] == '0') : false;
            iLossIdx++;
        }
        tmpSLostSim.isLost = bLost;
        m_ListSLostSim.push_back (tmpSLostSim);
        if (!bLost) {
          memcpy (m_pOutPutBuff + m_iOutPutLen, pSrc + iBufPos, i - iBufPos);
          m_iOutPutLen += (i - iBufPos);
        } else {
          m_ActualLoss[tmpSLostSim.eNalType]++;
        }
      }
      ilastprefixlen = 4;
      iBufPos = i;
      i = i + 4;
    } else if (pSrc[i] == 0 && pSrc[i + 1] == 0 && pSrc[i + 2] == 1) {
      if (i - iBufPos) {
        tmpSLostSim.eNalType = (EPlsNalUnitType) ((* (pSrc + iBufPos + ilastprefixlen)) & 0x1f);	// eNalUnitType
        m_TotalReceived[tmpSLostSim.eNalType]++;
        if(m_SliceLossRatio[tmpSLostSim.eNalType]==-1) {
            bLost = false;
        }
        else {
            bLost = iLossIdx < iLossCharLen ? (pLossChars[iLossIdx] == '0') : false;
            iLossIdx++;
        }
        tmpSLostSim.isLost = bLost;
        m_ListSLostSim.push_back (tmpSLostSim);
        if (!bLost) {
          memcpy (m_pOutPutBuff + m_iOutPutLen, pSrc + iBufPos, i - iBufPos);
          m_iOutPutLen += (i - iBufPos);
        } else {
          m_ActualLoss[tmpSLostSim.eNalType]++;
        }
      }
      ilastprefixlen = 3;
      iBufPos = i;
      i = i + 3;
    } else {
      i++;
    }
  }
  if (i - iBufPos) {
    tmpSLostSim.eNalType = (EPlsNalUnitType) ((* (pSrc + iBufPos + ilastprefixlen)) & 0x1f);	// eNalUnitType
    m_TotalReceived[tmpSLostSim.eNalType]++;
    if(m_SliceLossRatio[tmpSLostSim.eNalType]==-1) {
        bLost = false;
    }
    else {
        bLost = iLossIdx < iLossCharLen ? (pLossChars[iLossIdx] == '0') : false;
        iLossIdx++;
    }
    tmpSLostSim.isLost = bLost;
    m_ListSLostSim.push_back (tmpSLostSim);
    if (!bLost) {
      memcpy (m_pOutPutBuff + m_iOutPutLen, pSrc + iBufPos, i - iBufPos);
      m_iOutPutLen += (i - iBufPos);
    } else {
      m_ActualLoss[tmpSLostSim.eNalType]++;
    }
  }
  sBuffStatics.pBuff = m_pOutPutBuff;
  sBuffStatics.len = m_iOutPutLen;
  return sBuffStatics;
}