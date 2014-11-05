// Mem.h: interface for the CWseMem class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MEM_H__F6A24589_BE0C_4169_A913_857344E0AD77__INCLUDED_)
#define AFX_MEM_H__F6A24589_BE0C_4169_A913_857344E0AD77__INCLUDED_

#include "WseCommonTypes.h"


class CWseMem
{
public:
	virtual bool Reallocate(size_t size) = 0;
	virtual void Free() = 0;
	virtual bool Allocate(size_t size) = 0;
	CWseMem():m_p(NULL),m_size(0){};
	virtual ~CWseMem(){};
	
	size_t GetSize()
	{
		return m_size;
	}
	LPVOID GetPointer()
	{
		return m_p;
	}
protected:
	LPVOID m_p;
	size_t m_size;
};
#endif // !defined(AFX_MEM_H__F6A24589_BE0C_4169_A913_857344E0AD77__INCLUDED_)
