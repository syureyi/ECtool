/** 
 *************************************************************************************
 * \copy	Copyright (C) 2004-2009 by Cisco WebEx Communications, Inc.
 *
 *
 * \file	VideoSourceChannelInterface.h
 *
 * \author	Smith Guo(smithg@hz.webex.com)
 *			
 *
 * \brief	module interfaces for video source channel
 *
 * \date	4/30/2009 Created by Smith Guo
 *
 *************************************************************************************
*/
#ifndef COMMONTYPES_2009_4_30_H_
#define COMMONTYPES_2009_4_30_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef WIN32 
#pragma warning (disable : 4786)
#if !defined(_WIN32_WINNT)
#define _WIN32_WINNT 0x0400
#endif
#if !defined(POINTER_64) && _MSC_VER > 1200
#define POINTER_64 __ptr64
#endif
#include <winsock2.h>
#include <windows.h>
#include <windef.h>
#include <string>
#include <map>
#include <list>
typedef int	BOOL;
typedef std::string CWseString;
//typedef ........ WSE_GetlA
#else
#include <pthread.h>
#include <string>
#include <map>
#include <list>

typedef unsigned char* PBYTE;

#if defined(MACOS) || defined(IPHONEOS)
#include <sys/types.h>
#else
#ifdef LINUX
#include <linux/types.h>
#endif
#endif

typedef unsigned int          DWORD;
typedef long                  LONG;
#if defined(MACOS) || defined(IPHONEOS)
typedef signed char           BOOL;
#else
typedef int                   BOOL;
#endif
typedef unsigned char         BYTE;
typedef unsigned short        WORD;
typedef float                 FLOAT;
typedef int                   INT;
typedef unsigned int          UINT;
typedef FLOAT                *PFLOAT;
typedef BOOL                 *LPBOOL;
typedef INT                  *LPINT;
typedef WORD                 *LPWORD;
typedef LONG                 *LPLONG;
typedef DWORD                *LPDWORD;
typedef UINT                 *LPUINT;
typedef void                 *LPVOID;
typedef void                 *PVOID;
typedef const void           *LPCVOID;
typedef char                  CHAR;
typedef char                  TCHAR;
typedef char                 *LPSTR;
typedef const char           *LPCSTR;
/*
#ifndef LINUX
typedef wchar_t				  WCHAR;
typedef WCHAR				 *LPWSTR;
typedef const WCHAR			 *LPCWSTR;
#endif
*/
typedef BYTE                 *LPBYTE;
typedef const BYTE           *LPCBYTE;

typedef long CWseResult;
typedef std::string CWseString;
#ifndef MAX_PATH
#define MAX_PATH          260
#endif
//typedef void				*HANDLE;
//typedef void				*HWND;
#ifdef CM_LINUX
typedef long long int _int64;
#else

#ifdef UNIX
typedef long long int int64_t;
#endif

typedef int64_t				_int64_t;
//typedef uint64_t			_uint64_t;
typedef _int64_t			_int64;
#endif

#ifndef FALSE
#define FALSE 0
#endif // FALSE
#ifndef TRUE
#define TRUE 1
#endif // TRUE

#if defined(MACOS) || defined(IPHONEOS)

#else
// typedef unsigned long ulong;
// typedef long int32;
// typedef short int16;
// typedef unsigned int uint;
// typedef unsigned long uint32;
// typedef int	BOOL;
const int WSE_TRACE_VERSION = 1;
//typedef ........ WSE_GetlA
#endif //MACOS and IPHONEOS
#endif

#ifdef _U_TEST_
#ifndef protected
#define protected public
#endif
#ifndef private
#define private public
#endif
#endif


#endif
