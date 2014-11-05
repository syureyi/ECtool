// WseMutexGuardT.h: interface for the CWseMutexGuardT class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WSEMUTEXGUARDT_H__4BF35F7C_8483_49EA_BC48_C92680D98085__INCLUDED_)
#define AFX_WSEMUTEXGUARDT_H__4BF35F7C_8483_49EA_BC48_C92680D98085__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "WseErrorTypes.h"
template <class MutexType>
class CWseMutexGuardT
{
public:
	CWseMutexGuardT(MutexType& aMutex)
		: m_Mutex(aMutex)
		, m_bLocked(false)
	{
		Lock();
	}
	
	virtual ~CWseMutexGuardT()
	{
		UnLock();
	}
	
	long Lock() 
	{
		long rv = m_Mutex.Lock();
		m_bLocked = ((rv == 0)/*SUCCEEDED(rv)*/ ? true : false);
		return rv;
	}
	
	long UnLock() 
	{
		if (m_bLocked) {
			m_bLocked = false;
			return m_Mutex.UnLock();
		}
		else {
			return WSE_S_OK;
		}
	}
	
private:
	MutexType& m_Mutex;
	bool m_bLocked;
	// = Prevent assignment and initialization.
	CWseMutexGuardT& operator = (const CWseMutexGuardT&);
	CWseMutexGuardT(const CWseMutexGuardT&);
	CWseMutexGuardT();
};



#endif // !defined(AFX_WSEMUTEXGUARDT_H__4BF35F7C_8483_49EA_BC48_C92680D98085__INCLUDED_)


