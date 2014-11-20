#ifndef _SIMULATOR_PACKET_LOSS_H_
#define _SIMULATOR_PACKET_LOSS_H_
#include <vector>
#include <string>

typedef enum{
  PLS_NAL_UNIT_UNSPEC_0			= 0,
  PLS_NAL_UNIT_CODED_SLICE		= 1,
  PLS_NAL_UNIT_CODED_SLICE_DPA	= 2,
  PLS_NAL_UNIT_CODED_SLICE_DPB	= 3,
  PLS_NAL_UNIT_CODED_SLICE_DPC	= 4,
  PLS_NAL_UNIT_CODED_SLICE_IDR	= 5,
  PLS_NAL_UNIT_SEI				= 6,
  PLS_NAL_UNIT_SPS				= 7,
  PLS_NAL_UNIT_PPS				= 8,
  PLS_NAL_UNIT_AU_DELIMITER		= 9,
  PLS_NAL_UNIT_END_OF_SEQ			= 10,
  PLS_NAL_UNIT_END_OF_STR			= 11,
  PLS_NAL_UNIT_FILLER_DATA		= 12,
  PLS_NAL_UNIT_SPS_EXT			= 13,
  PLS_NAL_UNIT_PREFIX				= 14,
  PLS_NAL_UNIT_SUBSET_SPS			= 15,
  PLS_NAL_UNIT_RESV_16			= 16,
  PLS_NAL_UNIT_RESV_17			= 17,
  PLS_NAL_UNIT_RESV_18			= 18,
  PLS_NAL_UNIT_AUX_CODED_SLICE	= 19,
  PLS_NAL_UNIT_CODED_SLICE_EXT	= 20,
  PLS_NAL_UNIT_RESV_21			= 21,
  PLS_NAL_UNIT_RESV_22			= 22,
  PLS_NAL_UNIT_RESV_23			= 23,
  PLS_NAL_UNIT_UNSPEC_24			= 24,
  PLS_NAL_UNIT_UNSPEC_25			= 25,
  PLS_NAL_UNIT_UNSPEC_26			= 26,
  PLS_NAL_UNIT_UNSPEC_27			= 27,
  PLS_NAL_UNIT_UNSPEC_28			= 28,
  PLS_NAL_UNIT_UNSPEC_29			= 29,
  PLS_NAL_UNIT_UNSPEC_30			= 30,
  PLS_NAL_UNIT_UNSPEC_31			= 31
}EPlsNalUnitType;

typedef enum{
  PLS_NON_VCL			= 0,
  PLS_VCL				= 1,
  PLS_NOT_APP			= 2
}EPlsVclType;

const EPlsVclType g_kePlsTypeMap[32][2] = {
  { PLS_NON_VCL,	PLS_NON_VCL },	// 0: PLS_NAL_UNIT_UNSPEC_0
  { PLS_VCL,		PLS_VCL,	},	// 1: PLS_NAL_UNIT_CODED_SLICE
  { PLS_VCL,		PLS_NOT_APP },	// 2: PLS_NAL_UNIT_CODED_SLICE_DPA
  { PLS_VCL,		PLS_NOT_APP },	// 3: PLS_NAL_UNIT_CODED_SLICE_DPB
  { PLS_VCL,		PLS_NOT_APP },	// 4: PLS_NAL_UNIT_CODED_SLICE_DPC
  { PLS_VCL,		PLS_VCL		},	// 5: PLS_NAL_UNIT_CODED_SLICE_IDR
  { PLS_NON_VCL,	PLS_NON_VCL },	// 6: PLS_NAL_UNIT_SEI
  { PLS_NON_VCL,	PLS_NON_VCL },	// 7: PLS_NAL_UNIT_SPS
  { PLS_NON_VCL,	PLS_NON_VCL },	// 8: PLS_NAL_UNIT_PPS
  { PLS_NON_VCL,	PLS_NON_VCL },	// 9: PLS_NAL_UNIT_AU_DELIMITER
  { PLS_NON_VCL,	PLS_NON_VCL },	// 10: PLS_NAL_UNIT_END_OF_SEQ
  { PLS_NON_VCL,	PLS_NON_VCL },	// 11: PLS_NAL_UNIT_END_OF_STR
  { PLS_NON_VCL,	PLS_NON_VCL	},	// 12: PLS_NAL_UNIT_FILLER_DATA
  { PLS_NON_VCL,	PLS_NON_VCL },	// 13: PLS_NAL_UNIT_SPS_EXT
  { PLS_NON_VCL,	PLS_NON_VCL },	// 14: PLS_NAL_UNIT_PREFIX, NEED associate succeeded NAL to make a PLS_VCL
  { PLS_NON_VCL,	PLS_NON_VCL },	// 15: PLS_NAL_UNIT_SUBSET_SPS
  { PLS_NON_VCL,	PLS_NON_VCL },	// 16: PLS_NAL_UNIT_RESV_16
  { PLS_NON_VCL,	PLS_NON_VCL },	// 17: PLS_NAL_UNIT_RESV_17
  { PLS_NON_VCL,	PLS_NON_VCL },	// 18: PLS_NAL_UNIT_RESV_18
  { PLS_NON_VCL,	PLS_NON_VCL },	// 19: PLS_NAL_UNIT_AUX_CODED_SLICE
  { PLS_NON_VCL,	PLS_VCL		},	// 20: PLS_NAL_UNIT_CODED_SLICE_EXT
  { PLS_NON_VCL,	PLS_NON_VCL },	// 21: PLS_NAL_UNIT_RESV_21
  { PLS_NON_VCL,	PLS_NON_VCL },	// 22: PLS_NAL_UNIT_RESV_22
  { PLS_NON_VCL,	PLS_NON_VCL },	// 23: PLS_NAL_UNIT_RESV_23
  { PLS_NON_VCL,	PLS_NON_VCL },	// 24: PLS_NAL_UNIT_UNSPEC_24
  { PLS_NON_VCL,	PLS_NON_VCL },	// 25: PLS_NAL_UNIT_UNSPEC_25
  { PLS_NON_VCL,	PLS_NON_VCL },	// 26: PLS_NAL_UNIT_UNSPEC_26
  { PLS_NON_VCL,	PLS_NON_VCL	},	// 27: PLS_NAL_UNIT_UNSPEC_27
  { PLS_NON_VCL,	PLS_NON_VCL },	// 28: PLS_NAL_UNIT_UNSPEC_28
  { PLS_NON_VCL,	PLS_NON_VCL },	// 29: PLS_NAL_UNIT_UNSPEC_29
  { PLS_NON_VCL,	PLS_NON_VCL },	// 30: PLS_NAL_UNIT_UNSPEC_30
  { PLS_NON_VCL,	PLS_NON_VCL }	// 31: PLS_NAL_UNIT_UNSPEC_31
};

#define PLS_IS_VCL_NAL(t, ext_idx)			(g_kePlsTypeMap[t][ext_idx] == PLS_VCL)
#define PLS_IS_PARAM_SETS_NALS(t)			( (t) == PLS_NAL_UNIT_SPS || (t) == PLS_NAL_UNIT_PPS || (t) == PLS_NAL_UNIT_SUBSET_SPS )
#define PLS_IS_SPS_NAL(t)					( (t) == PLS_NAL_UNIT_SPS )
#define PLS_IS_SUBSET_SPS_NAL(t)			( (t) == PLS_NAL_UNIT_SUBSET_SPS )
#define PLS_IS_PPS_NAL(t)					( (t) == PLS_NAL_UNIT_PPS )
#define PLS_IS_SEI_NAL(t)					( (t) == PLS_NAL_UNIT_SEI )
#define PLS_IS_PREFIX_NAL(t)				( (t) == PLS_NAL_UNIT_PREFIX )
#define PLS_IS_SUBSET_SPS_USED(t)			( (t) == PLS_NAL_UNIT_SUBSET_SPS || (t) == PLS_NAL_UNIT_CODED_SLICE_EXT )
#define PLS_IS_VCL_NAL_AVC_BASE(t)			( (t) == PLS_NAL_UNIT_CODED_SLICE || (t) == PLS_NAL_UNIT_CODED_SLICE_IDR )
#define PLS_IS_NEW_INTRODUCED_SVC_NAL(t)	( (t) == PLS_NAL_UNIT_PREFIX || (t) == PLS_NAL_UNIT_CODED_SLICE_EXT )

typedef struct SLost_Statics {
  EPlsNalUnitType eNalType;
  bool isLost;
} SLostStatics;

typedef struct SSlice_LossRatio {
  int iSPSLossRatio;
  int iSubSPSLossRatio;
  int iPPSLossRatio;
  int iPrefixLossRatio;
  int iAVCISliceLossRatio;
  int iAVCPSliceLossRatio;
  int iSVCSliceExtLossRatio;
  int iOther; // used to count
  // add more to specfic other PLR
} SSliceLossRatioInPercent;

typedef struct SLossstatus_Info {
  SSliceLossRatioInPercent sActualLostSlicesNum;
  SSliceLossRatioInPercent sTotalReceivedNum;
  double fTotalLostRatio;
  double fSPSLossRatio;
  double fSubSPSLossRatio;
  double fPPSLossRatio;
  double fPrefixLossRatio;
  double fAVCISliceLossRatio;
  double fAVCPSliceLossRatio;
  double fSVCSliceExtLossRatio;
  double fOther;
  std::vector<SLostStatics>* pLostDetail;
  unsigned char* pLossBuf;
  int iLossBufLen;
} SLossstatusInfo;

typedef struct SOutBuff_Statics {
  unsigned char* pBuff;
  int len;
} SOutBuffStatics;




#define MAX_AU_IN_BYTES 1000000
class CPacketLossSimulator {
public:
  CPacketLossSimulator();
  ~CPacketLossSimulator();
  void SetLossRatio (SSliceLossRatioInPercent* pLossRatio);
  SSliceLossRatioInPercent GetPresetLossRatio ();
  // pSrc is pointer to AU buffer or H264 file buffer, iSrcLen is the buffer lenth
  SOutBuffStatics SimulateNALLoss (const unsigned char* pSrc,  int iSrcLen);
  // pSrc is pointer to AU buffer or H264 file buffer, iSrcLen is the buffer lenth, 
  // pLossChars is the pointer to input loss characters buffer, iLossCharLen is the buffer lenth
  // bResetPos, true, loss from 0 pos in pLossChars; false, pos will be recoded inside
  SOutBuffStatics SimulateNALLoss (const unsigned char* pSrc,  int iSrcLen, unsigned char* pLossChars, int iLossCharLen, bool bResetPos);
  SLossstatusInfo GetLossStatus ();
private:
  std::vector<SLostStatics>* GetLossDetail() {return &m_ListSLostSim;};
private:
  std::vector<SLostStatics> m_ListSLostSim;
  int m_ActualLoss[32];
  int m_TotalReceived[32];
  unsigned char* m_pOutPutBuff;
  unsigned char* m_pOutPutLossBuff;
  int m_SliceLossRatio[32];
  int m_iOutPutLen;
  int iLossIdx;
  //string cInPacketLossFile;
  //string cOutPacketLossFile;
};
#endif
