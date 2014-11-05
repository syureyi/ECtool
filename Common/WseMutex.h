// WseMutex.h: interface for the CWseMutex class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WSEMUTEX_H__0CEEE53B_97C4_4374_9574_8442B3DA57EA__INCLUDED_)
#define AFX_WSEMUTEX_H__0CEEE53B_97C4_4374_9574_8442B3DA57EA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef WSE_WBX_UNIX_TRACE
#else
//#include "Util.h"
#endif
#include "WseMutexGuardT.h"

#if defined(WIN32)
#include <winsock2.h>
#include <windows.h>
typedef CRITICAL_SECTION CM_THREAD_MUTEX_T;
typedef DWORD WSE_THREAD_ID;
typedef HANDLE WSE_THREAD_HANDLE;
#else
#include <pthread.h>
typedef pthread_mutex_t CM_THREAD_MUTEX_T;
typedef pthread_t WSE_THREAD_ID;
typedef WSE_THREAD_ID WSE_THREAD_HANDLE;
#endif

class CWseMutex
{
private:
	CWseMutex& operator = (const CWseMutex&);
	CWseMutex(const CWseMutex&);
public:
	CWseMutex();
	virtual ~CWseMutex();
	HRESULT Lock();
	HRESULT UnLock();
	HRESULT TryLock();
	
	CM_THREAD_MUTEX_T& GetMutexType() { return m_Lock;}
	
protected:
	CM_THREAD_MUTEX_T m_Lock;
};

typedef CWseMutexGuardT<CWseMutex> CWseMutexGuard;

#endif // !defined(AFX_WSEMUTEX_H__0CEEE53B_97C4_4374_9574_8442B3DA57EA__INCLUDED_)
