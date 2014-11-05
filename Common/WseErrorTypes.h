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

#ifndef IWSEERRORTYPES_2009_5_4_H_
#define IWSEERRORTYPES_2009_5_4_H_

#include "jlerror.h"

//common errors	
const long WSE_S_OK = JL_S_OK;
const long WSE_S_FALSE = JL_S_FALSE;
const long WSE_E_FAIL = JL_E_FAIL;
const long WSE_E_OUTOFMEMORY = JL_E_OUTOFMEMORY;
const long WSE_E_INVALIDARG = JL_E_INVALIDARG;
const long WSE_E_POINTER = JL_E_POINTER;
const long WSE_E_NOTIMPL = JL_E_NOTIMPL;
const long WSE_E_NOINTERFACE = JL_E_NOINTERFACE;

//SVC engine specific
const long WSE_E_BASE = 10000;
const long WSE_E_INVALID_CHANNEL = WSE_E_BASE + 1;
const long WSE_E_INIT_FAIL = WSE_E_BASE + 2;
const long WSE_E_CHANNEL_NO_SESSION = WSE_E_BASE + 3;
const long WSE_E_NO_RTP_CHANNEL = WSE_E_BASE + 4;
const long WSE_E_MODULE_NOT_EXIST = WSE_E_BASE + 5;
const long WSE_E_EXTERNAL_PROCESS_NOT_EXIST = WSE_E_BASE + 6;
const long WSE_E_DEL_CHANNEL_FAIL = WSE_E_BASE + 7;
const long WSE_E_NO_SESSION_FOUND = WSE_E_BASE + 8;
const long WSE_E_NO_SOURCE_CHANNEL_FOUND = WSE_E_BASE + 9;
const long WSE_E_LOAD_DLL_FAIL = WSE_E_BASE + 10;
const long WSE_E_GETPROCESS_FAIL = WSE_E_BASE + 11;
const long WSE_E_INPUT_VIDEO_TYPE = WSE_E_BASE + 12;
//const long WSE_E_RTP_NOT_EXIST = WSE_ERROR_BASE + 7;
const long WSE_E_NOENCODER = WSE_E_BASE + 200;
const long WSE_E_NOTENOUGHMEMORY = WSE_E_BASE + 201;

//rendering
const long WSE_E_RENDER_DSHOW_FAIL = WSE_E_BASE + 300;
const long WSE_E_RENDER_GDI_NEED_RGB_FORMAT = WSE_E_BASE + 301;
const long WSE_E_PAUSED = WSE_E_BASE + 302;
const long WSE_E_NO_HIT = WSE_E_BASE + 310;

//transcoding
const long WSE_E_INVALID_PARAMS  = WSE_E_BASE + 400;

//device
const long WSE_E_DEVICE_BASE = 0x46024100;
const long WSE_E_VIDEO_CAMERA_FAIL = WSE_E_DEVICE_BASE + 1;

#define WSE_SUCCEEDED(hr) (WSE_S_OK == hr)
#define WSE_FAILED(hr) (WSE_S_OK != hr)

#endif
