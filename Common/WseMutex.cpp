// WseMutex.cpp: implementation of the CWseMutex class.
//
//////////////////////////////////////////////////////////////////////

#include "WseMutex.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWseMutex::CWseMutex()
{
#ifdef _WIN32
	::InitializeCriticalSection(&m_Lock);
	//WSE_ASSERTE(m_Lock);
#elif defined(MACOS) || defined(IPHONEOS)
	pthread_mutexattr_t mutex_attr;
	::pthread_mutexattr_init(&mutex_attr);
	::pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_RECURSIVE);
	::pthread_mutex_init(&m_Lock,&mutex_attr);
	::pthread_mutexattr_destroy(&mutex_attr);
#else
	pthread_mutexattr_t mutex_attr;
	::pthread_mutexattr_init(&mutex_attr);
#if defined CM_LINUX
    ::pthread_mutexattr_settype(&mutex_attr,PTHREAD_MUTEX_RECURSIVE_NP);
#else
    ::pthread_mutexattr_settype(&mutex_attr,PTHREAD_MUTEX_RECURSIVE);
#endif
	::pthread_mutex_init(&m_Lock, &mutex_attr);
	::pthread_mutexattr_destroy(&mutex_attr);
#endif // _WIN32
}

CWseMutex::~CWseMutex()
{
#if _WIN32
	::DeleteCriticalSection(&m_Lock);
#else
	int nRet = ::pthread_mutex_destroy(&m_Lock);
	if (nRet != 0)  {
	}
#endif // CM_WIN32
}


HRESULT CWseMutex::Lock()
{
#if _WIN32
	::EnterCriticalSection(&m_Lock);
	return WSE_S_OK;
#else
	int nRet = ::pthread_mutex_lock(&m_Lock);
	if (nRet == 0)
		return WSE_S_OK;
	else {
		return WSE_E_FAIL;
	}
#endif // CM_WIN32
}

HRESULT CWseMutex::UnLock()
{
#if _WIN32
	::LeaveCriticalSection(&m_Lock);
	return WSE_S_OK;
#else
	int nRet = ::pthread_mutex_unlock(&m_Lock);
	if (nRet == 0)
		return WSE_S_OK;
	else {
		return WSE_E_FAIL;
	}
#endif // CM_WIN32
}

HRESULT CWseMutex::TryLock()
{
#ifdef _WIN32

#if(_WIN32_WINNT >= 0x0400)
	BOOL bRet = ::TryEnterCriticalSection(&m_Lock);
	return bRet ? WSE_S_OK : WSE_E_FAIL;
#else
	return WSE_E_FAIL;
#endif //

#else
	int nRet = ::pthread_mutex_trylock(&m_Lock);
	return (nRet == 0) ? WSE_S_OK : WSE_E_FAIL;
;
#endif // CM_WIN32
}

