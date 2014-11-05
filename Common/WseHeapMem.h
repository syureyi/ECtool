#ifndef HEAP_MEM_H_2009
#define HEAP_MEM_H_2009
#include "WseMem.h"

class CWseAlignedMem : public CWseMem
{
public:
	CWseAlignedMem(size_t alignment = 0);
	virtual ~CWseAlignedMem();
	bool Reallocate(size_t size);
	void Free();
	bool Allocate(size_t size);

	int Check();
	int Estimate(size_t want); // 0 < smaller, 0 match, >0 bigger
protected:
	virtual void* DoAllocate(size_t size,size_t& allocedsize) = 0;
	virtual void DoFree(void* p) = 0;
protected:
	size_t GetAllocatedSize();
	size_t GetWantedSize(size_t size);
	void PrepareMemChecking();
	void CorrectMemPointer();
protected:
	void* m_preal;
	size_t m_alignment;
	size_t m_allocsize;
};


class CWseHeapMem : public CWseAlignedMem
{
public:
	CWseHeapMem(size_t alignment = 0);
	virtual ~CWseHeapMem();
protected:
	void* DoAllocate(size_t size,size_t& allocedsize);
	void DoFree(void* p);
};

#endif
