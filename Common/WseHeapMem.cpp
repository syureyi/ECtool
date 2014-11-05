// Mem.cpp: implementation of the CMem class.
//
//////////////////////////////////////////////////////////////////////
#include "WseHeapMem.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
size_t GetAlignedSize(size_t size,size_t alignment)
{
	size_t alignedSize = size;

	// alignment needed
	if(alignment > 1)
	{
		size_t remainder = alignedSize % alignment;
		if(remainder != 0)
			alignedSize += (alignment - remainder);
	}

	return alignedSize;
}
void* CorrectAlignedMemPointer(void* p,size_t alignment)
{
	unsigned char * correctedp = (unsigned char *)p;

	// correct aligned pointer
	if(alignment > 1)
	{
		size_t remainder = (size_t)correctedp % alignment;
		if(remainder != 0)
			correctedp += (alignment - remainder);
	}

	return (void*)correctedp;
}

//////////////////////////////////////////////////////////////////////////
// CWseAlignedMem
//////////////////////////////////////////////////////////////////////////
CWseAlignedMem::CWseAlignedMem(size_t alignment)
:m_preal(NULL),m_alignment(alignment),m_allocsize(0)
{

}
CWseAlignedMem::~CWseAlignedMem()
{
	Free();
}
bool CWseAlignedMem::Reallocate(size_t size)
{
	if(Estimate(size) >= 0)
	{
		m_size = size;
		m_p = m_preal;
		CorrectMemPointer();
		PrepareMemChecking();
		return true;
	}
	return Allocate(size);
}

void CWseAlignedMem::Free()
{
	Check();

	if(m_preal)
		DoFree(m_preal);

	m_preal = NULL;
	m_allocsize = 0;
	m_p = NULL;
	m_size = 0;
}

bool CWseAlignedMem::Allocate(size_t size)
{
	Free();

	size_t wantsize = GetWantedSize(size);

	size_t allocedsize = 0;
	m_preal = DoAllocate(wantsize,allocedsize);
	if(NULL == m_preal)
		return false;
	m_allocsize = allocedsize;

	m_size = size;
	m_p = m_preal;
	CorrectMemPointer();
	PrepareMemChecking();
	return true;
}

size_t CWseAlignedMem::GetWantedSize(size_t size)
{
	size_t alignedSize = size;

	// alignment needed
	if(m_alignment > 1)
	{
		alignedSize = GetAlignedSize(size,m_alignment);
		alignedSize += m_alignment; // to ensure the first pointer aligned, add 1 m_alignment size
	}
	return alignedSize;
}

void CWseAlignedMem::PrepareMemChecking()
{

}

void CWseAlignedMem::CorrectMemPointer()
{
	m_p = CorrectAlignedMemPointer(m_preal,m_alignment);

}

size_t CWseAlignedMem::GetAllocatedSize()
{
	return m_allocsize;
}

int CWseAlignedMem::Estimate(size_t want)
{
	return GetAllocatedSize() - GetWantedSize(want);
}

int CWseAlignedMem::Check()
{

	return 0;
}

//////////////////////////////////////////////////////////////////////
// CWseHeapMem
//////////////////////////////////////////////////////////////////////
CWseHeapMem::CWseHeapMem(size_t alignment)
:CWseAlignedMem(alignment)
{
}
CWseHeapMem::~CWseHeapMem()
{
	Free();// must here,if not DoFree() will be invoked by CAlignedMem::~CAlignedMem().
}
void* CWseHeapMem::DoAllocate(size_t size,size_t& allocedsize)
{
	allocedsize = 0;
	void *p = malloc(size);
	if(p)
		allocedsize = size;
	return p;
}
void CWseHeapMem::DoFree(void* p)
{
	free(p);
}
