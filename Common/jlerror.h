//===================================================================
// Copyright (C) 2002-2010 WebEx Communications
// All rights reserved.
//	
//	Creator:	jinj@hz.webex.com
//	Date:		07/07/2010
//	Notes :		
//===================================================================

//===================================================================
//	History:
//	Version	Name		Date			Description
//	1.00	jinj		07/07/2010		Created
//
//===================================================================

#ifndef _JLERROR_H_
#define _JLERROR_H_

#define JL_SUCCEEDED(Status)	((long)(Status) >= 0)
#define JL_FAILED(Status)		((long)(Status) < 0)

// Success codes
#define JL_S_FALSE							(0x00000001)
#define JL_S_OK								(0x00000000)

//  Unspecified error
#define JL_E_FAIL							(0x80000001)

//  Ran out of memory
#define JL_E_OUTOFMEMORY					(0x80000002)

//  One or more arguments are invalid
#define JL_E_INVALIDARG						(0x80000003)

//  Not implemented
#define JL_E_NOTIMPL						(0x80000004)

//  No such interface supported
#define JL_E_NOINTERFACE					(0x80000005)

//  Invalid pointer
#define JL_E_POINTER						(0x80000006)


#endif // _JLERROR_H_