#ifndef _WSEVIDEOSAMPLE_H_
#define _WSEVIDEOSAMPLE_H_

//#include "jlbaseimp.h"


#include "../common/WseMutex.h"
#include "../common/WseHeapMem.h"
#include "../api.h"



class CWseVideoSampleAllocator;
class CWseVideoSample : public CWseHeapMem , public IWseVideoSample
{
	friend class CWseVideoSampleAllocator;
public:
	// IJlUnknown
	/*JLMETHOD_(unsigned long,AddRef)();
	JLMETHOD_(unsigned long,Release)();
	JLMETHOD_(JLRESULT,QueryInterface)(REFJLIID iid,void **ppvObject);*/

	// IWseVideoSample
	WSERESULT GetSize(unsigned long* pulSize);
	WSERESULT GetPointer(unsigned char **ppBuffer);
	WSERESULT GetDataPointer(unsigned char **ppBuffer);
	WSERESULT GetDataLength(unsigned long* pulDataLength);
	WSERESULT GetDataPlanarPointer(unsigned char **ppBuffer, int idx);
	WSERESULT GetDataStride(unsigned int *puiStride, int idx);
	WSERESULT GetVideoFormat(WseVideoFormat *pFormat);
	WSERESULT SetDataPointer(unsigned char *pBuffer);
	WSERESULT SetDataLength(unsigned long ulDataLength);
	WSERESULT SetDataPlanarPointer(unsigned char *pBuffer, int idx);
	WSERESULT SetDataStride(unsigned int uiStride, int idx);
	WSERESULT SetVideoFormat(WseVideoFormat *pFormat);
private:
	CWseVideoSample(unsigned long ulAlignment,CWseVideoSampleAllocator *pAllocator);
	virtual ~CWseVideoSample();

	BOOL Reset(unsigned long ulSize);
	int CheckMemory();
    void SetDataPlanar();
protected:
    //interface with packed data mode
	unsigned char *m_pDataPointer;
	unsigned long m_ulDataLength;
	WseVideoFormat m_format;

    //interface with planar data mode
	unsigned char *m_pDataPlanar[MAX_PLANAR_NUM];
	unsigned int m_iStride[MAX_PLANAR_NUM];

    //interface for external source data, should not be modified in the class
    
	volatile unsigned long m_uRef;

#ifndef WIN32
	CWseMutex m_lock;
#endif

	CWseVideoSample *m_pNext;          /* Chaining in free list */
    CWseVideoSampleAllocator *m_pAllocator;      /* The allocator who owns us */
	DWORD m_dwFreeTimestamp;
};

class CWseVideoSampleAllocator :public IWseVideoSampleAllocator
{
public:
	CWseVideoSampleAllocator(unsigned long ulAlignment = 0,DWORD dwReduceInterval = 10000,DWORD dwExpiredInterval = 10000);
	virtual ~CWseVideoSampleAllocator();

public:
	// IJlUnknown
	/*IMPLEMENT_JLREFERENCE
	BEGIN_QI_HANDLER(CWseVideoSampleAllocator)
		WSE_QI_HANDLER(IWseVideoSampleAllocator)
	END_QI_HANDLER()*/

	// IWseVideoSampleAllocator
	WSERESULT GetSample(unsigned long ulBufferSize,IWseVideoSample** ppSample);
	
public:
	void Free();
	
	static CWseVideoSample * &NextSample(CWseVideoSample *pSample)
	{
		return pSample->m_pNext;
	};
	
	/*  Mini list class for the free list */
	class CWseVideoSampleList
	{
	public:
		CWseVideoSampleList() : m_List(NULL), m_nOnList(0) {};
		~CWseVideoSampleList(){};
		CWseVideoSample *Head() const { return m_List; };
		CWseVideoSample *Next(CWseVideoSample *pSample) const { return CWseVideoSampleAllocator::NextSample(pSample); };
		int GetCount() const { return m_nOnList; };
		void Add(CWseVideoSample *pSample);
/*
		void Add(CWseVideoSample *pSample)
		{
			CWseVideoSampleAllocator::NextSample(pSample) = m_List;
			m_List = pSample;
			m_nOnList++;
		};
 */
		CWseVideoSample *RemoveHead()
		{
			CWseVideoSample *pSample = m_List;
			if (pSample != NULL) {
				m_List = CWseVideoSampleAllocator::NextSample(m_List);
				m_nOnList--;
			}
			return pSample;
		};
		void Remove(CWseVideoSample *pSample);
	public:
		CWseVideoSample *m_List;
		int m_nOnList;
	};
	
	CWseVideoSample* GetSample(unsigned long ulSize);
	void ReleaseSample(CWseVideoSample * pSample);
	void ReduceFreeList(DWORD dwTimestamp);
	//DWORD GetTimestamp();
	CWseVideoSample *GetFreeSample_MostSharing(unsigned long ulSize);
	CWseVideoSample *GetFreeSample_StrictSize(unsigned long ulSize);
protected:
	unsigned long m_ulAlignment;
	CWseVideoSampleList m_lFree;        // Free list
	
	CWseMutex m_lck;

	DWORD m_dwExpiredInterval;
	DWORD m_dwReduceInterval;
	DWORD m_dwLastReduceTimestamp;
};
#endif // _WSEVIDEOSAMPLE_H_
