#include "WseVideoSample.h"
//#include "timer.h"
#include "../common/WseErrorTypes.h"
#include <sstream>


//////////////////////////////////////////////////////////////////////////
WSERESULT CreateVideoSampleAllocator(unsigned long ulAlignment,IWseVideoSampleAllocator** ppVideoSampleAllocator)
{
	if(NULL == ppVideoSampleAllocator)
		return WSE_E_INVALIDARG;
	CWseVideoSampleAllocator* p = new CWseVideoSampleAllocator(ulAlignment);
	if(NULL == p)
		return WSE_E_OUTOFMEMORY;
	/*p->AddRef();*/
	*ppVideoSampleAllocator = p;
	return WSE_S_OK;
}
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CWseVideoSample::CWseVideoSample(unsigned long ulAlignment,CWseVideoSampleAllocator *pAllocator)
:CWseHeapMem(ulAlignment),
m_pAllocator(pAllocator),
m_pNext(NULL)
{
	m_pDataPointer = NULL;
	m_ulDataLength = 0;
	memset(&m_format,0,sizeof(m_format));
    memset(m_pDataPlanar, 0, sizeof(unsigned char*)*MAX_PLANAR_NUM);
    memset(m_iStride, 0, sizeof(unsigned int)*MAX_PLANAR_NUM);
}
CWseVideoSample::~CWseVideoSample()
{

}
//unsigned long CWseVideoSample::AddRef()
//{
//#ifdef WIN32
//	InterlockedIncrement((long *)&m_uRef);
//#else
//	CWseMutexGuard guard(m_lock);
//	++m_uRef;
//	guard.UnLock();
//#endif
//	return m_uRef;
//}
//unsigned long CWseVideoSample::Release()
//{
//#ifdef WIN32
//	unsigned long uRef = InterlockedDecrement((long *)&m_uRef);
//#else
//	CWseMutexGuard guard(m_lock);
//	unsigned long uRef = --m_uRef;
//	guard.UnLock();
//#endif
//	if(0 == uRef)
//	{
//		++m_uRef;
//		m_pAllocator->ReleaseSample(this);
//		return 0;
//	}
//	return m_uRef;
//}
//JLRESULT CWseVideoSample::QueryInterface(REFJLIID iid,void **ppvObject)
//{
//	if(ppvObject == NULL)
//		return JL_E_POINTER;
//
//	if(IsEqualJLIID(iid,WSEIID_IWseVideoSample)) {
//		return GetInterface((IWseVideoSample *)this,ppvObject);
//	} else if(IsEqualJLIID(iid,JLIID_IJlUnknown)){
//		return GetInterface((IJlUnknown *)this,ppvObject);
//	} else {
//		*ppvObject = NULL;
//		return JL_E_NOINTERFACE;
//	}
//}
WSERESULT CWseVideoSample::GetSize(unsigned long* pulSize)
{
	if(!pulSize)
		return WSE_E_INVALIDARG;
	*pulSize = CWseHeapMem::GetSize();
	return WSE_S_OK;
}
WSERESULT CWseVideoSample::GetPointer(unsigned char **ppBuffer)
{
	if(!ppBuffer)
		return WSE_E_INVALIDARG;
	*ppBuffer = (unsigned char *)CWseHeapMem::GetPointer();
	return WSE_S_OK;
}
WSERESULT CWseVideoSample::GetDataPointer(unsigned char **ppBuffer)
{
	if(!ppBuffer)
		return WSE_E_INVALIDARG;
	*ppBuffer = m_pDataPointer;
	return WSE_S_OK;
}
WSERESULT CWseVideoSample::GetDataLength(unsigned long* pulDataLength)
{
	if(!pulDataLength)
		return WSE_E_INVALIDARG;
	*pulDataLength = m_ulDataLength;
	return WSE_S_OK;
}
WSERESULT CWseVideoSample::GetDataPlanarPointer(unsigned char **ppBuffer, int idx)
{
	if(!ppBuffer || idx < 0 || idx >= MAX_PLANAR_NUM)
		return WSE_E_INVALIDARG;
	*ppBuffer = m_pDataPlanar[idx];
	return WSE_S_OK;
}
WSERESULT CWseVideoSample::GetDataStride(unsigned int *puiStride, int idx)
{
	if(!puiStride || idx < 0 || idx >= MAX_PLANAR_NUM)
		return WSE_E_INVALIDARG;
	*puiStride = m_iStride[idx];
	return WSE_S_OK;
}
WSERESULT CWseVideoSample::GetVideoFormat(WseVideoFormat *pFormat)
{
	if(!pFormat)
		return WSE_E_INVALIDARG;
	*pFormat = m_format;
	return WSE_S_OK;
}

WSERESULT CWseVideoSample::SetDataPointer(unsigned char *pBuffer)
{
	if(!pBuffer)
		return WSE_E_INVALIDARG;

	unsigned char *p1,*p2;
	GetPointer(&p1);
	unsigned long ulSize;
	GetSize(&ulSize);
	p2 = p1 + ulSize;
	if(pBuffer < p1 || pBuffer >= p2)
		return WSE_E_INVALIDARG;
	m_pDataPointer = pBuffer;
	return WSE_S_OK;
}
WSERESULT CWseVideoSample::SetDataLength(unsigned long ulDataLength)
{
	unsigned long ulSize;
	GetSize(&ulSize);
    if (ulDataLength > ulSize)
        return WSE_E_FAIL;
	m_ulDataLength = ulDataLength;
	return WSE_S_OK;
}
WSERESULT CWseVideoSample::SetDataPlanarPointer(unsigned char *pBuffer, int idx)
{
	if (!pBuffer || idx < 0 || idx >= MAX_PLANAR_NUM)
		return WSE_E_INVALIDARG;
	unsigned char *p1, *p2;
	GetPointer(&p1);
	unsigned long ulSize;
	GetSize(&ulSize);
	p2 = p1 + ulSize;
	if (pBuffer < p1 || pBuffer >= p2)
		return WSE_E_INVALIDARG;
	m_pDataPlanar[idx] = pBuffer;
	return WSE_S_OK;
}
WSERESULT CWseVideoSample::SetDataStride(unsigned int uiStride, int idx)
{
	if (idx < 0 || idx >= MAX_PLANAR_NUM)
		return WSE_E_INVALIDARG;
	unsigned long ulSize;
	GetSize(&ulSize);
	if (uiStride > ulSize)
		return WSE_E_FAIL;
	m_iStride[idx] = uiStride;
	return WSE_S_OK;
}
WSERESULT CWseVideoSample::SetVideoFormat(WseVideoFormat *pFormat)
{
	if(!pFormat)
		return WSE_E_INVALIDARG;
	m_format = *pFormat;
    SetDataPlanar();
    return WSE_S_OK;
}
BOOL CWseVideoSample::Reset(unsigned long ulSize)
{
	m_pDataPointer = NULL;
	m_ulDataLength = 0;
	bool brt = CWseHeapMem::Reallocate(ulSize);
	if(brt)
	{
		m_pDataPointer = (unsigned char *)CWseHeapMem::GetPointer();
		m_ulDataLength = ulSize;
	}
	return (true == brt);
}
//#include "WseDebug.h"
inline int CWseVideoSample::CheckMemory()
{
	int rt = CWseHeapMem::Check();
	if(0 != rt)
	{
		//WSE_WARN_TRACE("CWseVideoSample::CheckMemory, memory DAMAGE! this = "<<this);
	}
	return rt;
}
void CWseVideoSample::SetDataPlanar()
{
    switch (m_format.video_type) {
        case WseI420:
        case WseYV12:
            m_iStride[0] = m_format.width;
            m_iStride[1] =
            m_iStride[2] = m_format.width/2;
            m_pDataPlanar[0] = m_pDataPointer;
            m_pDataPlanar[1] = m_pDataPlanar[0] + m_iStride[0] * m_format.height;
            m_pDataPlanar[2] = m_pDataPlanar[1] + m_iStride[1] * m_format.height/2;
            break;
        case WseRGB24:
        case WseRGB24Flip:
        case WseBGR24:
        case WseBGR24Flip:
            m_iStride[0] =
            m_iStride[1] =
            m_iStride[2] = m_format.width;
            m_pDataPlanar[0] = m_pDataPointer;
            m_pDataPlanar[1] = m_pDataPlanar[0] + m_iStride[0] * m_format.height;
            m_pDataPlanar[2] = m_pDataPlanar[1] + m_iStride[1] * m_format.height;
            break;
        case WseABGR32:
        case WseABGR32Flip:
        case WseARGB32:
        case WseARGB32Flip:
        case WseBGRA32:
        case WseBGRA32Flip:
        case WseRGBA32:
        case WseRGBA32Flip:
            m_iStride[0] =
            m_iStride[1] =
            m_iStride[2] =
            m_iStride[3] = m_format.width;
            m_pDataPlanar[0] = m_pDataPointer;
            m_pDataPlanar[1] = m_pDataPlanar[0] + m_iStride[0] * m_format.height;
            m_pDataPlanar[2] = m_pDataPlanar[1] + m_iStride[1] * m_format.height;
            m_pDataPlanar[3] = m_pDataPlanar[2] + m_iStride[2] * m_format.height;
            break;
            
        default:
            break;
    }
}
//////////////////////////////////////////////////////////////////////////
// CWseVideoSampleAllocator
//////////////////////////////////////////////////////////////////////////
WSERESULT CWseVideoSampleAllocator::GetSample(unsigned long ulBufferSize,IWseVideoSample** ppSample)
{
	CWseVideoSample* pSample = GetSample(ulBufferSize);
	if(NULL == pSample)
		return WSE_E_OUTOFMEMORY;
	*ppSample = pSample;
	return WSE_S_OK;
}
inline void CWseVideoSampleAllocator::CWseVideoSampleList::Add(CWseVideoSample *pSample)
{
	// add warning trace for dead loop of bug 450353
	if(pSample == m_List)
	{
		//WSE_WARN_TRACE("CWseVideoSampleAllocator::CWseVideoSampleList::Add Sample is added repeatedly, pSample = " << pSample << ", listHead = " << m_List);
	}
	
	CWseVideoSampleAllocator::NextSample(pSample) = m_List;
	m_List = pSample;
	m_nOnList++;
}
void CWseVideoSampleAllocator::CWseVideoSampleList::Remove(CWseVideoSample * pSample)
{
    CWseVideoSample **pSearch;
    for (pSearch = &m_List;
	*pSearch != NULL;
	pSearch = &(CWseVideoSampleAllocator::NextSample(*pSearch))) {
		if (*pSearch == pSample) {
			*pSearch = CWseVideoSampleAllocator::NextSample(pSample);
			CWseVideoSampleAllocator::NextSample(pSample) = NULL;
			m_nOnList--;
			return;
		}
    }
}
CWseVideoSampleAllocator::CWseVideoSampleAllocator(unsigned long ulAlignment,DWORD dwReduceInterval,DWORD dwExpiredInterval)
:m_ulAlignment(ulAlignment),m_dwReduceInterval(dwReduceInterval),m_dwExpiredInterval(dwExpiredInterval)
{
	if(m_dwReduceInterval && m_dwExpiredInterval)
	{}
		// initialize the last reduce timestamp
		//m_dwLastReduceTimestamp = GetTimestamp();
}
CWseVideoSampleAllocator::~CWseVideoSampleAllocator()
{
	Free();
}
void CWseVideoSampleAllocator::Free()
{
	CWseMutexGuard lck(m_lck);
	CWseVideoSample *pSample;
	for(;;)
	{
		pSample = m_lFree.RemoveHead();
		if(pSample != NULL)
			delete pSample;
		else
			break;
	}
}
CWseVideoSample* CWseVideoSampleAllocator::GetSample(unsigned long ulSize)
{
#define GetFreeSample GetFreeSample_MostSharing	// GetFreeSample_StrictSize
	CWseVideoSample *pSample = GetFreeSample(ulSize);
	
	if(NULL == pSample)
	{
		pSample = new CWseVideoSample(m_ulAlignment,this);
		if(NULL == pSample)
			return NULL;
	}
	pSample->m_uRef = 1;
	if(FALSE == pSample->Reset(ulSize))
	{
	/*	pSample->Release();*/
		return NULL;
	}

	return pSample;
}
void CWseVideoSampleAllocator::ReleaseSample(CWseVideoSample * pSample)
{
	// memory checking
	if(0 != pSample->CheckMemory())
	{
		//WSE_WARN_TRACE("CWseVideoSampleAllocator::ReleaseSample, memory DAMAGE! this = "<<this);
	}

	CWseMutexGuard lck(m_lck);
	
	if(m_dwReduceInterval && m_dwExpiredInterval)
	{
		//DWORD dwTimestamp = GetTimestamp();

		// reduce free list
		//ReduceFreeList(dwTimestamp);
		
		// mark the free timestamp
		//pSample->m_dwFreeTimestamp = dwTimestamp;
	}
	
	m_lFree.Add(pSample);
}
//inline DWORD CWseVideoSampleAllocator::GetTimestamp()
//{
//	return static_cast<unsigned long>(wse_ticker::now()/1000);
//}
inline void CWseVideoSampleAllocator::ReduceFreeList(DWORD dwTimestamp)
{
	if(dwTimestamp - m_dwLastReduceTimestamp < m_dwReduceInterval)
		return;
	m_dwLastReduceTimestamp = dwTimestamp;
	
	int loopCount = 0, listCount = m_lFree.GetCount();
	
	CWseVideoSample **ppSample = &m_lFree.m_List;
	while(*ppSample != NULL)
	{
		// add warning trace and enhance loop limit for dead loop of bug 450353
		loopCount++;
		if(loopCount == listCount + 1)
		{
			/*WSE_WARN_TRACE("CWseVideoSampleAllocator::ReduceFreeList Loop count is over list count, dwTimestamp = " << dwTimestamp 
						   << ", listCount = " << listCount << ", pSample = " << *ppSample << ", listHead = " << m_lFree.Head() 
						   << ", restCount = " << m_lFree.GetCount());*/
			break;
		}
		
		if(dwTimestamp - (*ppSample)->m_dwFreeTimestamp > m_dwExpiredInterval)
		{
			CWseVideoSample *pSampleDel = *ppSample;
			*ppSample = NextSample(*ppSample);
			m_lFree.m_nOnList--;
			delete pSampleDel;
		}
		else
		{
			ppSample = &NextSample(*ppSample);
		}
	}
}
inline CWseVideoSample *CWseVideoSampleAllocator::GetFreeSample_MostSharing(unsigned long ulSize)
{
	CWseMutexGuard lck(m_lck);
//	return m_lFree.RemoveHead();

	size_t sNeed = ulSize;
	
	int loopCount = 0, listCount = m_lFree.GetCount();
#if !defined(ANDROID)	// stringstream can't support completely on android
		std::stringstream listInfo;
#endif
	
	CWseVideoSample *pCandidateSample = NULL;
	int candidate_estvalue = 0;
	CWseVideoSample *pMinimumSample = NULL;
	int minimum_estvalue = 0;
	CWseVideoSample *pSample = m_lFree.Head();
	while(pSample != NULL)
	{
		// add warning trace and enhance loop limit for dead loop of bug 450353
		loopCount++;
		if(loopCount == listCount + 1)
		{
//			WSE_WARN_TRACE("CWseVideoSampleAllocator::GetFreeSample_MostSharing Loop count is over list count, ulSize = " << ulSize 
//						   << ", listCount = " << listCount << ", pSample = " << pSample << ", listHead = " << m_lFree.Head());
//#if !defined(ANDROID)
//			// list detail info
//			WSE_WARN_TRACE("CWseVideoSampleAllocator::GetFreeSample_MostSharing List info: " << listInfo.str() << "OverSample[" << pSample << "]");
//#endif
			break;
		}
#if !defined(ANDROID)
		else if(loopCount <= listCount)
		{
			listInfo << "Sample" << loopCount << "[" << pSample << "] -> ";
		}
#endif		
		int estvalue = pSample->Estimate(sNeed);
		if(estvalue == 0)
		{// exact match
			pCandidateSample = pSample;
			candidate_estvalue = estvalue;
			break;
		}
		if(estvalue > 0)
		{
			if(NULL == pCandidateSample ||
				estvalue < candidate_estvalue)
			{
				pCandidateSample = pSample;// minimum bigger
				candidate_estvalue = estvalue;
			}
		}
		if(NULL == pMinimumSample ||
			estvalue < minimum_estvalue)
		{
			pMinimumSample = pSample;// minimum
			minimum_estvalue = estvalue;
		}
		pSample = NextSample(pSample);
	}
	
	if(NULL == pCandidateSample && pMinimumSample)
		pCandidateSample = pMinimumSample;// if have no bigger,choose minimum

	if(pCandidateSample)
		m_lFree.Remove(pCandidateSample);// remove from free list
	return pCandidateSample;
}
inline CWseVideoSample *CWseVideoSampleAllocator::GetFreeSample_StrictSize(unsigned long ulSize)
{
	CWseMutexGuard lck(m_lck);
	
	size_t sNeed = ulSize;
	
	int loopCount = 0, listCount = m_lFree.GetCount();
	
	CWseVideoSample *pSample = m_lFree.Head();
	while(pSample != NULL)
	{
		// enhance loop limit for dead loop of bug 450353
		loopCount++;
		if(loopCount == listCount + 1)
		{
			pSample = NULL;
			break;
		}
		
		if(pSample->Estimate(sNeed) == 0)
		{// exact match
			break;
		}
		pSample = NextSample(pSample);
	}
	
	if(pSample)
		m_lFree.Remove(pSample);// remove from free list
	return pSample;
}
