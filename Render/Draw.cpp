#include "Draw.h"

#include <math.h>

LPBYTE GetBits(HBITMAP hBitmap,LPBITMAPINFO lpbi)
{
	int rt = 0;
	HDC hDC = NULL;
	BYTE *pDibData = NULL;
	BOOL bOK = FALSE;
	
	do 
	{
		hDC = GetDC(NULL);
		if(NULL == hDC)
		{
			//WSE_ERROR_TRACE("GetBits(),GetDC failed! errorcode = "<<GetLastError());
			break;
		}

		rt = GetDIBits(hDC,
			hBitmap,
			0,
			abs(lpbi->bmiHeader.biHeight),
			NULL,
			lpbi,
			DIB_RGB_COLORS);
		if(0 == rt)
		{
			//WSE_ERROR_TRACE("GetBits(),GetDIBits(fill the BITMAPINFO structure) failed! errorcode = "<<GetLastError());
			break;
		}

		pDibData = new BYTE[lpbi->bmiHeader.biSizeImage];
		if(NULL == pDibData)
		{
			//WSE_ERROR_TRACE("GetBits(),out of memory! size = "<<lpbi->bmiHeader.biSizeImage);
			break;
		}

		rt = GetDIBits(hDC,
			hBitmap,
			0,
			abs(lpbi->bmiHeader.biHeight),
			pDibData,
			lpbi,
			DIB_RGB_COLORS);
		if(0 == rt)
		{
			//WSE_ERROR_TRACE("GetBits(),GetDIBits(fill bits) failed! errorcode = "<<GetLastError());
			break;
		}

		bOK = TRUE;
	}
	while(0);
	
	if(hDC)
		ReleaseDC(NULL,hDC);

	if(FALSE == bOK && pDibData)
	{
		delete []pDibData;
		pDibData = NULL;
	}

	return pDibData;
}
HBITMAP MakeBitmap(LPBYTE pData,LPBITMAPINFOHEADER lpbi)
{
	HDC hMemDC = CreateCompatibleDC(NULL);
	if(NULL == hMemDC)
	{
		//WSE_ERROR_TRACE("MakeBitmap(),CreateCompatibleDC failed.errorcode="<<GetLastError());
		return NULL;
	}
	LPVOID pBit32;
	HBITMAP bmp = CreateDIBSection(hMemDC,(LPBITMAPINFO)lpbi,DIB_RGB_COLORS,&pBit32,NULL,0);
	if(NULL == bmp)
		//WSE_ERROR_TRACE("MakeBitmap(),CreateDIBSection failed.errorcode="<<GetLastError());
	DeleteDC(hMemDC);
	if(bmp && pBit32)
		memcpy(pBit32,pData,lpbi->biSizeImage); //confirmed_safe_unsafe_usage
	return bmp;
}
void CalcBitmapDrawToRect(LPCRECT prcEntire,PBMP_DESC pbd,DWORD dwBmpWidth,DWORD dwBmpHeight,LPRECT prcTarget)
{
	if(IS_PERCENTAGEPOSITION(pbd->dwMask))
	{
		if(IS_PERCENTAGEPOS_OUTOFRANGE(pbd->fleft) && IS_PERCENTAGEPOS_OUTOFRANGE(pbd->fright))
			pbd->fleft = 0.0f;
		if(IS_PERCENTAGEPOS_OUTOFRANGE(pbd->ftop) && IS_PERCENTAGEPOS_OUTOFRANGE(pbd->fbottom))
			pbd->ftop = 0.0f;
		
		if(IS_PERCENTAGEPOS_OUTOFRANGE(pbd->fleft))
		{
			prcTarget->right = prcEntire->left + (LONG)((prcEntire->right - prcEntire->left)*pbd->fright);
			prcTarget->left = prcTarget->right - dwBmpWidth;
		}
		else
		{
			prcTarget->left = prcEntire->left + (LONG)((prcEntire->right - prcEntire->left)*pbd->fleft);
			if(IS_PERCENTAGEPOS_OUTOFRANGE(pbd->fright))
				prcTarget->right = prcTarget->left + dwBmpWidth;
			else
				prcTarget->right = prcEntire->left + (LONG)((prcEntire->right - prcEntire->left)*pbd->fright);
		}
		if(IS_PERCENTAGEPOS_OUTOFRANGE(pbd->ftop))
		{
			prcTarget->bottom = prcEntire->top + (LONG)((prcEntire->bottom - prcEntire->top)*pbd->fbottom);
			prcTarget->top = prcTarget->bottom - dwBmpHeight;
		}
		else
		{
			prcTarget->top = prcEntire->top + (LONG)((prcEntire->bottom - prcEntire->top)*pbd->ftop);
			if(IS_PERCENTAGEPOS_OUTOFRANGE(pbd->fbottom))
				prcTarget->bottom = prcTarget->top + dwBmpHeight;
			else
				prcTarget->bottom = prcEntire->top + (LONG)((prcEntire->bottom - prcEntire->top)*pbd->fbottom);
		}
	}
	else
	{
		if(IS_REVERSE_ORIGIN_LEFT(pbd->dwMask))
			prcTarget->left = prcEntire->right - (LONG)pbd->fleft;
		else
			prcTarget->left = prcEntire->left + (LONG)pbd->fleft;
		
		if(IS_REVERSE_ORIGIN_RIGHT(pbd->dwMask))
			prcTarget->right = prcEntire->right - (LONG)pbd->fright;
		else
			prcTarget->right = prcEntire->left + (LONG)pbd->fright;
		
		if(IS_REVERSE_ORIGIN_TOP(pbd->dwMask))
			prcTarget->top = prcEntire->bottom - (LONG)pbd->ftop;
		else
			prcTarget->top = prcEntire->top + (LONG)pbd->ftop;
		
		if(IS_REVERSE_ORIGIN_BOTTOM(pbd->dwMask))
			prcTarget->bottom = prcEntire->bottom - (LONG)pbd->fbottom;
		else
			prcTarget->bottom = prcEntire->top + (LONG)pbd->fbottom;
	}
}

//////////////////////////////////////////////////////////////////////////
// helper function
int GetCompatibleFourCC(DWORD dwFourCC,/*out*/LPDWORD* ppdwCompatibleFourCCs)
{
	if(!ppdwCompatibleFourCCs)
		return 0;

	switch(dwFourCC)
	{
	case MAKEFOURCC('I','4','2','0'):
	case MAKEFOURCC('Y','V','1','2'):
		{
			static const DWORD planarformats[] = 
			{
				MAKEFOURCC('I','4','2','0')   // I420
				,MAKEFOURCC('I','Y','U','V')  // IYUV same as I420
				,MAKEFOURCC('Y','V','1','2')  // YV12
				,MAKEFOURCC('N','V','1','2')  // NV12
				,MAKEFOURCC('I','M','C','1')  // IMC1
				,MAKEFOURCC('I','M','C','3')  // IMC3
				,MAKEFOURCC('I','M','C','2')  // IMC2
				,MAKEFOURCC('I','M','C','4')  // IMC4
			};
			*ppdwCompatibleFourCCs = (LPDWORD)planarformats;
			return sizeof(planarformats)/sizeof(planarformats[0]);
		}
		break;
	case MAKEFOURCC('Y','U','Y','2'):
		{
			static const DWORD packedformats[] = 
			{
				MAKEFOURCC('Y','U','Y','2')   // YUY2
				,MAKEFOURCC('Y','U','N','V')   // YUNV same as YUY2
				,MAKEFOURCC('V','4','2','2')   // V422 same as YUY2
				,MAKEFOURCC('Y','U','Y','V')   // YUYV same as YUY2

				,
				MAKEFOURCC('U','Y','V','Y')   // UYVY
				,MAKEFOURCC('Y','4','2','2')   // Y422 same as UYVY
				,MAKEFOURCC('U','Y','N','V')   // UYNV same as UYVY
				,MAKEFOURCC('H','D','Y','C')   // HDYC same as UYVY
			};
			*ppdwCompatibleFourCCs = (LPDWORD)packedformats;
			return sizeof(packedformats)/sizeof(packedformats[0]);
		}
		break;
	case MAKEFOURCC('U','Y','V','Y'):
		{
			static const DWORD packedformats[] = 
			{
				MAKEFOURCC('U','Y','V','Y')   // UYVY
				,MAKEFOURCC('Y','4','2','2')   // Y422 same as UYVY
				,MAKEFOURCC('U','Y','N','V')   // UYNV same as UYVY
				,MAKEFOURCC('H','D','Y','C')   // HDYC same as UYVY

				,
				MAKEFOURCC('Y','U','Y','2')   // YUY2
				,MAKEFOURCC('Y','U','N','V')   // YUNV same as YUY2
				,MAKEFOURCC('V','4','2','2')   // V422 same as YUY2
				,MAKEFOURCC('Y','U','Y','V')   // YUYV same as YUY2
			};
			*ppdwCompatibleFourCCs = (LPDWORD)packedformats;
			return sizeof(packedformats)/sizeof(packedformats[0]);
		}
		break;
	}
	return 0;
}
BOOL IsFourCCMatched(DWORD dwFourCCSrc,DWORD dwFourCCDest)
{
	// planar formats
	if(dwFourCCSrc == MAKEFOURCC('I','4','2','0')
		|| dwFourCCSrc == MAKEFOURCC('Y','V','1','2'))
	{
		if(dwFourCCDest == MAKEFOURCC('I','4','2','0')
			|| dwFourCCDest == MAKEFOURCC('I','Y','U','V')
			|| dwFourCCDest == MAKEFOURCC('Y','V','1','2')
			|| dwFourCCDest == MAKEFOURCC('N','V','1','2')
			|| dwFourCCDest == MAKEFOURCC('I','M','C','1')
			|| dwFourCCDest == MAKEFOURCC('I','M','C','3')
			|| dwFourCCDest == MAKEFOURCC('I','M','C','2')
			|| dwFourCCDest == MAKEFOURCC('I','M','C','4')
			)
			return TRUE;
	}

	// packed formats
	if(dwFourCCSrc == MAKEFOURCC('Y','U','Y','2')
		|| dwFourCCSrc == MAKEFOURCC('U','Y','V','Y'))
	{
		if(dwFourCCDest == MAKEFOURCC('Y','U','Y','2')
			|| dwFourCCDest == MAKEFOURCC('Y','U','N','V')
			|| dwFourCCDest == MAKEFOURCC('V','4','2','2')
			|| dwFourCCDest == MAKEFOURCC('Y','U','Y','V')
			
			|| dwFourCCDest == MAKEFOURCC('U','Y','V','Y')
			|| dwFourCCDest == MAKEFOURCC('Y','4','2','2')
			|| dwFourCCDest == MAKEFOURCC('U','Y','N','V')
			|| dwFourCCDest == MAKEFOURCC('H','D','Y','C')
			)
			return TRUE;
	}

	return FALSE;
}
BOOL BltYUV_Packed(LPBYTE pSrc,int nSrcStride,DWORD dwSrcFourCC,
				   LPBYTE pDst,int nDstPitch,DWORD dwDstFourCC,
				   UINT uiWidth,UINT uiHeight)
{
	if(dwSrcFourCC == MAKEFOURCC('Y','U','Y','2'))
	{
		if(dwDstFourCC == MAKEFOURCC('Y','U','Y','2')
			|| dwDstFourCC == MAKEFOURCC('Y','U','N','V')
			|| dwDstFourCC == MAKEFOURCC('V','4','2','2')
			|| dwDstFourCC == MAKEFOURCC('Y','U','Y','V')
			)
		{
			for(UINT uiY = 0;uiY < uiHeight;uiY++)
			{
				memcpy(pDst,pSrc,uiWidth*2); //confirmed_safe_unsafe_usage
				pDst += nDstPitch;
				pSrc += nSrcStride;
			}
		}
		else if(dwDstFourCC == MAKEFOURCC('U','Y','V','Y')
			|| dwDstFourCC == MAKEFOURCC('Y','4','2','2')
			|| dwDstFourCC == MAKEFOURCC('U','Y','N','V')
			|| dwDstFourCC == MAKEFOURCC('H','D','Y','C')
			)
		{
			for(UINT uiY = 0;uiY < uiHeight;uiY++)
			{
				LPDWORD lpDest = (LPDWORD)pDst;
				LPDWORD lpSrc = (LPDWORD)pSrc;
				for(UINT uiX = 0; uiX < uiWidth/2; uiX++)
				{
					*lpDest = (((*lpSrc)<<8) & 0xFF00FF00) | (((*lpSrc)>>8) & 0x00FF00FF);
					lpDest++;
					lpSrc++;
				}
				pDst += nDstPitch;
				pSrc += nSrcStride;
			}
		}
		else
		{
			return FALSE;
		}
	}
	else if(dwSrcFourCC == MAKEFOURCC('U','Y','V','Y'))
	{
		if(dwDstFourCC == MAKEFOURCC('U','Y','V','Y')
			|| dwDstFourCC == MAKEFOURCC('Y','4','2','2')
			|| dwDstFourCC == MAKEFOURCC('U','Y','N','V')
			|| dwDstFourCC == MAKEFOURCC('H','D','Y','C')
			)
		{
			for(UINT uiY = 0;uiY < uiHeight;uiY++)
			{
				memcpy(pDst,pSrc,uiWidth*2); //confirmed_safe_unsafe_usage
				pDst += nDstPitch;
				pSrc += nSrcStride;
			}
		}
		else if(dwDstFourCC == MAKEFOURCC('Y','U','Y','2')
			|| dwDstFourCC == MAKEFOURCC('Y','U','N','V')
			|| dwDstFourCC == MAKEFOURCC('V','4','2','2')
			|| dwDstFourCC == MAKEFOURCC('Y','U','Y','V')
			)
		{
			for(UINT uiY = 0;uiY < uiHeight;uiY++)
			{
				LPDWORD lpDest = (LPDWORD)pDst;
				LPDWORD lpSrc = (LPDWORD)pSrc;
				for(UINT uiX = 0; uiX < uiWidth/2; uiX++)
				{
					*lpDest = (((*lpSrc)<<8) & 0xFF00FF00) | (((*lpSrc)>>8) & 0x00FF00FF);
					lpDest++;
					lpSrc++;
				}
				pDst += nDstPitch;
				pSrc += nSrcStride;
			}
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}
	return TRUE;
}
BOOL BltYUV_Planar(LPBYTE pSrcY,LPBYTE pSrcU,LPBYTE pSrcV,int nSrcStrideY,int nSrcStrideUV,
				   LPBYTE pDst,int nDstPitch,DWORD dwDstFourCC,
				   UINT uiWidth,UINT uiHeight)
{
	if(dwDstFourCC == MAKEFOURCC('Y','V','1','2'))
	{
		UINT i = 0;
		//   fill   Y   data
		for(i=0; i<uiHeight; i++)
		{
			memcpy(pDst,pSrcY,uiWidth); //confirmed_safe_unsafe_usage
			pSrcY += nSrcStrideY;
			pDst += nDstPitch;
		}
		
		//   fill   V   data
		for(i=0; i<uiHeight/2; i++)
		{
			memcpy(pDst,pSrcV,uiWidth/2); //confirmed_safe_unsafe_usage
			pSrcV += nSrcStrideUV;
			pDst += nDstPitch / 2;
		}
		
		//   fill   U   data
		for(i=0; i<uiHeight/2; i++)
		{
			memcpy(pDst,pSrcU,uiWidth/2); //confirmed_safe_unsafe_usage
			pSrcU += nSrcStrideUV;
			pDst += nDstPitch / 2;
		}
	}
	else if(dwDstFourCC == MAKEFOURCC('I','4','2','0')
		|| dwDstFourCC == MAKEFOURCC('I','Y','U','V'))
	{
		UINT i = 0;
		//   fill   Y   data
		for(i=0; i<uiHeight; i++)
		{
			memcpy(pDst,pSrcY,uiWidth); //confirmed_safe_unsafe_usage
			pSrcY += nSrcStrideY;
			pDst += nDstPitch;
		}
		
		//   fill   U   data
		for(i=0; i<uiHeight/2; i++)
		{
			memcpy(pDst,pSrcU,uiWidth/2); //confirmed_safe_unsafe_usage
			pSrcU += nSrcStrideUV;
			pDst += nDstPitch / 2;
		}
		
		//   fill   V   data
		for(i=0; i<uiHeight/2; i++)
		{
			memcpy(pDst,pSrcV,uiWidth/2); //confirmed_safe_unsafe_usage
			pSrcV += nSrcStrideUV;
			pDst += nDstPitch / 2;
		}
	}
	else if(dwDstFourCC == MAKEFOURCC('N','V','1','2'))
	{
		UINT i = 0;
		//   fill   Y   data
		for(i=0; i<uiHeight; i++)
		{
			memcpy(pDst,pSrcY,uiWidth); //confirmed_safe_unsafe_usage
			pSrcY += nSrcStrideY;
			pDst += nDstPitch;
		}
		
		//   fill   U   data   and   V   data
		// original BYTE array is [u0][u1][u2][u3]
		//                        [v0][v1][v2][v3]
		// aligned DWORD array is [u3 u2 u1 u0]
		//                        [v3 v2 v1 v0]
		// We want to transform it to BYTE array is [u0][v0][u1][v1][u2][v2][u3][v3]
		// aligned DWORD array is [v1 u1 v0 u0][v3 u3 v2 u2]
		for(i=0;i < uiHeight/2;i++)
		{
			register DWORD * pdwSurf = (DWORD*)pDst;
			register DWORD * pdwU = (DWORD*)pSrcU;
			register DWORD * pdwV = (DWORD*)pSrcV;
			
			UINT col, dwordWidth;
			dwordWidth = (uiWidth/2) / 4; // aligned width/2 of the row, in DWORDS
			for(col = 0;col < dwordWidth;col++ )
			{
				pdwSurf[0] = ((pdwV[0]<<16) & 0xFF000000)
					| ((pdwU[0]<<8) & 0x00FF0000)
					| ((pdwV[0]<<8) & 0x0000FF00)
					| (pdwU[0] & 0x000000FF);
				pdwSurf[1] = (pdwV[0] & 0xFF000000)
					| ((pdwU[0]>>8) & 0x00FF0000)
					| ((pdwV[0]>>8) & 0x0000FF00)
					| ((pdwU[0]>>16) & 0x000000FF);
				
				pdwSurf += 2;
				pdwU++;
				pdwV++;
			}
			
			// we might have remaining (misaligned) bytes here
			BYTE * pbU = (BYTE*)pdwU;
			BYTE * pbV = (BYTE*)pdwV;
			BYTE * pbSurf = (BYTE*)pdwSurf;
			for( col = 0; col < (uiWidth/2) % 4; col++)
			{
				*pbSurf++ = *pbU++;
				*pbSurf++ = *pbV++;
			}
			
			pDst += nDstPitch;
			pSrcU += nSrcStrideUV;
			pSrcV += nSrcStrideUV;
		}
	}
	else if(dwDstFourCC == MAKEFOURCC('I','M','C','1'))
	{
		UINT i = 0;
		//   fill   Y   data
		for(i=0; i<uiHeight; i++)
		{
			memcpy(pDst,pSrcY,uiWidth); //confirmed_safe_unsafe_usage
			pSrcY += nSrcStrideY;
			pDst += nDstPitch;
		}
		
		//   fill   V   data
		for(i=0; i<uiHeight/2; i++)
		{
			memcpy(pDst,pSrcV,uiWidth/2); //confirmed_safe_unsafe_usage
			pSrcV += nSrcStrideUV;
			pDst += nDstPitch;
		}
		
		//   fill   U   data
		for(i=0; i<uiHeight/2; i++)
		{
			memcpy(pDst,pSrcU,uiWidth/2); //confirmed_safe_unsafe_usage
			pSrcU += nSrcStrideUV;
			pDst += nDstPitch;
		}
	}
	else if(dwDstFourCC == MAKEFOURCC('I','M','C','3'))
	{
		UINT i = 0;
		//   fill   Y   data
		for(i=0; i<uiHeight; i++)
		{
			memcpy(pDst,pSrcY,uiWidth); //confirmed_safe_unsafe_usage
			pSrcY += nSrcStrideY;
			pDst += nDstPitch;
		}
		
		//   fill   U   data
		for(i=0; i<uiHeight/2; i++)
		{
			memcpy(pDst,pSrcU,uiWidth/2); //confirmed_safe_unsafe_usage
			pSrcU += nSrcStrideUV;
			pDst += nDstPitch;
		}
		
		//   fill   V   data
		for(i=0; i<uiHeight/2; i++)
		{
			memcpy(pDst,pSrcV,uiWidth/2); //confirmed_safe_unsafe_usage
			pSrcV += nSrcStrideUV;
			pDst += nDstPitch;
		}
	}
	else if(dwDstFourCC == MAKEFOURCC('I','M','C','2'))
	{
		UINT i = 0;
		//   fill   Y   data
		for(i=0; i<uiHeight; i++)
		{
			memcpy(pDst,pSrcY,uiWidth); //confirmed_safe_unsafe_usage
			pSrcY += nSrcStrideY;
			pDst += nDstPitch;
		}
		
		//   fill   V   data   and   U   data
		for(i=0; i<uiHeight/2; i++)
		{
			memcpy(pDst,pSrcV,uiWidth/2); //confirmed_safe_unsafe_usage
			pSrcV += nSrcStrideUV;
			pDst += nDstPitch / 2;

			memcpy(pDst,pSrcU,uiWidth/2); //confirmed_safe_unsafe_usage
			pSrcU += nSrcStrideUV;
			pDst += nDstPitch / 2;
		}
	}
	else if(dwDstFourCC == MAKEFOURCC('I','M','C','4'))
	{
		UINT i = 0;
		//   fill   Y   data
		for(i=0; i<uiHeight; i++)
		{
			memcpy(pDst,pSrcY,uiWidth); //confirmed_safe_unsafe_usage
			pSrcY += nSrcStrideY;
			pDst += nDstPitch;
		}
		
		//   fill   U   data   and   V   data
		for(i=0; i<uiHeight/2; i++)
		{
			memcpy(pDst,pSrcU,uiWidth/2); //confirmed_safe_unsafe_usage
			pSrcU += nSrcStrideUV;
			pDst += nDstPitch / 2;

			memcpy(pDst,pSrcV,uiWidth/2); //confirmed_safe_unsafe_usage
			pSrcV += nSrcStrideUV;
			pDst += nDstPitch / 2;
		}
	}
	else
	{
		return FALSE;
	}
	return TRUE;
}
BOOL BltBGR(LPBYTE pSrc,int nSrcStride,
			LPBYTE pDst,int nDstPitch,fmt_BGR fmtDst,
			UINT uiWidth,UINT uiHeight)
{// input image always RGB24
	switch(fmtDst)
	{
	case fmt_X8R8G8B8:
		{
			UINT col, dwordWidth;
			
			// Instead of copying data bytewise, we use DWORD alignment here.
			// We also unroll loop by copying 4 pixels at once.
			//
			// original BYTE array is [b0][g0][r0][b1][g1][r1][b2][g2][r2][b3][g3][r3]
			//
			// aligned DWORD array is     [b1 r0 g0 b0][g2 b2 r1 g1][r3 g3 b3 r2]
			//
			// We want to transform it to [ff r0 g0 b0][ff r1 g1 b1][ff r2 g2 b2][ff r3 b3 g3]
			// below, bitwise operations do exactly this.
			
			dwordWidth = uiWidth / 4; // aligned width of the row, in DWORDS
			// (pixel by 3 bytes over sizeof(DWORD))
			
			for(UINT iY = 0; iY< uiHeight; iY++)
			{
				register DWORD * pdwS = ( DWORD*)pSrc;
				register DWORD * pdwD = ( DWORD*)pDst;
				
				for( col = 0; col < dwordWidth; col ++ )
				{
					pdwD[0] =  pdwS[0] | 0xFF000000;
					pdwD[1] = ((pdwS[1]<<8)  | 0xFF000000) | (pdwS[0]>>24);
					pdwD[2] = ((pdwS[2]<<16) | 0xFF000000) | (pdwS[1]>>16);
					pdwD[3] = 0xFF000000 | (pdwS[2]>>8);
					pdwD +=4;
					pdwS +=3;
				}
				
				// we might have remaining (misaligned) bytes here
				BYTE  * pbS = (BYTE*) pdwS;
				for( col = 0; col < (UINT)uiWidth % 4; col++)
				{
					*pdwD = 0xFF000000 |
						(pbS[2] << 16) |
						(pbS[1] <<  8) |
						(pbS[0]);
					pdwD++;
					pbS += 3;
				}
				
				pSrc += nSrcStride;
				pDst += nDstPitch;
			}
		}
		break;
	case fmt_R8G8B8:
		{
			for(UINT iY = 0; iY< uiHeight; iY++)
			{
				memcpy(pDst,pSrc,nDstPitch<nSrcStride?nDstPitch:nSrcStride); //confirmed_safe_unsafe_usage
				pSrc += nSrcStride;
				pDst += nDstPitch;
			}
		}
		break;
	case fmt_X1R5G5B5:
		{// RGB 5:5:5
			UINT col, wordWidth;
			
			// original BYTE array is [b0][g0][r0][b1][g1][r1][b2][g2][r2][b3][g3][r3]
			//
			// aligned DWORD array is     [b1 r0 g0 b0][g2 b2 r1 g1][r3 g3 b3 r2]
			//
			// We want to transform it to [rg b1 rg b0][rg b3 rg b2]
			
			wordWidth = uiWidth / 4; // aligned width of the row, in DWORDS
			// (pixel by 3 bytes over sizeof(DWORD))
			
			for(UINT iY = 0; iY< uiHeight; iY++)
			{
				register DWORD * pdwS = ( DWORD*)pSrc;
				register DWORD * pdwD = ( DWORD*)pDst;
				
				for( col = 0; col < wordWidth; col ++ )
				{
					//          unused                   r1                              g1                                b1
					pdwD[0] = 0x80008000 | ((pdwS[1] & 0x0000F800) << 15) | ((pdwS[1] & 0x000000F8) << 18) | ((pdwS[0] & 0xF8000000) >> 11)
						//                   r0                            g0                            b0
						| ((pdwS[0] & 0x00F80000) >> 9) | ((pdwS[0] & 0x0000F800) >> 6) | ((pdwS[0] & 0x000000F8) >> 3);
					//          unused                   r3                             g3                           b3
					pdwD[1] = 0x80008000 | ((pdwS[2] & 0xF8000000) >> 1) | ((pdwS[2] & 0x00F80000) << 2) | ((pdwS[2] & 0x0000F800) << 5)
						//                   r2                             g2                             b2
						| ((pdwS[2] & 0x000000F8) << 7) | ((pdwS[1] & 0xF8000000) >> 22) | ((pdwS[1] & 0x00F80000) >> 19);
					pdwD +=2;
					pdwS +=3;
				}
				
				// we might have remaining (misaligned) bytes here
				BYTE  * pbS = (BYTE*) pdwS;
				WORD  * pwD = (WORD*) pdwD;
				for( col = 0; col < (UINT)uiWidth % 4; col++)
				{
					*pwD = (WORD)
						(0x8000 +
						((pbS[2] & 0xF8) << 7) +
						((pbS[1] & 0xF8) << 2) +
						(pbS[0] >> 3));
					pwD++;
					pbS += 3;
				}
				
				pSrc += nSrcStride;
				pDst += nDstPitch;
			}
		}
		break;
	case fmt_R5G6B5:
		{// RGB 5:6:5
			UINT col, wordWidth;
			
			// original BYTE array is [b0][g0][r0][b1][g1][r1][b2][g2][r2][b3][g3][r3]
			//
			// aligned DWORD array is     [b1 r0 g0 b0][g2 b2 r1 g1][r3 g3 b3 r2]
			//
			// We want to transform it to [rg b1 rg b0][rg b3 rg b2]
			
			wordWidth = uiWidth / 4; // aligned width of the row, in DWORDS
			// (pixel by 3 bytes over sizeof(DWORD))
			
			for(UINT iY = 0; iY< uiHeight; iY++)
			{
				register DWORD * pdwS = ( DWORD*)pSrc;
				register DWORD * pdwD = ( DWORD*)pDst;
				
				for( col = 0; col < wordWidth; col ++ )
				{
					//                     r1                              g1                                b1
					pdwD[0] = ((pdwS[1] & 0x0000F800) << 16) | ((pdwS[1] & 0x000000FC) << 19) | ((pdwS[0] & 0xF8000000) >> 11)
						//                  r0                              g0                            b0
						| ((pdwS[0] & 0x00F80000) >> 8) | ((pdwS[0] & 0x0000FC00) >> 5) | ((pdwS[0] & 0x000000F8) >> 3);
					//                    r3                           g3                           b3
					pdwD[1] = (pdwS[2] & 0xF8000000) | ((pdwS[2] & 0x00FC0000) << 3) | ((pdwS[2] & 0x0000F800) << 5)
						//                  r2                              g2                             b2
						| ((pdwS[2] & 0x000000F8) << 8) | ((pdwS[1] & 0xFC000000) >> 21) | ((pdwS[1] & 0x00F80000) >> 19);
					pdwD +=2;
					pdwS +=3;
				}
				
				// we might have remaining (misaligned) bytes here
				BYTE  * pbS = (BYTE*) pdwS;
				WORD  * pwD = (WORD*) pdwD;
				for( col = 0; col < (UINT)uiWidth % 4; col++)
				{
					*pwD = (WORD)
						(
						((pbS[2] & 0xF8) << 8) +
						((pbS[1] & 0xFC) << 3) +
						(pbS[0] >> 3));
					pwD++;
					pbS += 3;
				}
				
				pSrc += nSrcStride;
				pDst += nDstPitch;
			}
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}
BOOL BltYUV_Packed_Safe(LPBYTE pSrc,int nSrcStride,DWORD dwSrcFourCC,
						LPBYTE pDst,int nDstPitch,DWORD dwDstFourCC,
						UINT uiWidth,UINT uiHeight,BOOL& bException)
{
	bException = FALSE;
	__try
	{
		return BltYUV_Packed(pSrc,nSrcStride,dwSrcFourCC,pDst,nDstPitch,dwDstFourCC,uiWidth,uiHeight);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		bException = TRUE;
	}
	return FALSE;
}
BOOL BltYUV_Planar_Safe(LPBYTE pSrcY,LPBYTE pSrcU,LPBYTE pSrcV,int nSrcStrideY,int nSrcStrideUV,
						LPBYTE pDst,int nDstPitch,DWORD dwDstFourCC,
						UINT uiWidth,UINT uiHeight,BOOL& bException)
{
	bException = FALSE;
	__try
	{
		return BltYUV_Planar(pSrcY,pSrcU,pSrcV,nSrcStrideY,nSrcStrideUV,pDst,nDstPitch,dwDstFourCC,uiWidth,uiHeight);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		bException = TRUE;
	}
	return FALSE;
}
BOOL BltBGR_Safe(LPBYTE pSrc,int nSrcStride,
				 LPBYTE pDst,int nDstPitch,fmt_BGR fmtDst,
				 UINT uiWidth,UINT uiHeight,BOOL& bException)
{
	bException = FALSE;
	__try
	{
		return BltBGR(pSrc,nSrcStride,pDst,nDstPitch,fmtDst,uiWidth,uiHeight);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		bException = TRUE;
	}
	return FALSE;
}
//////////////////////////////////////////////////////////////////////////
// CDraw2
//////////////////////////////////////////////////////////////////////////
CDraw2::CDraw2()
{
	memset(&m_bk,0,sizeof(m_bk));
	m_bk.bkd.bktype = BKT_COLOR;
	m_bk.bkd.clr = RGB(0,0,0);

	m_armode = ARM_NONE;
	m_dwImageEffect = 0;
}
CDraw2::~CDraw2()
{
	
}
BOOL CDraw2::AddDrawBitmap(unsigned int uID,PBMP_DESC pParam)
{
	ID2BPMap::iterator itfind = m_mapID2BP.find(uID);
	if(itfind != m_mapID2BP.end())
	{
		itfind->second.bd = *pParam;
	}
	else
	{
		BMP_PARAM bp;
		memset(&bp,0,sizeof(bp));
		bp.bd = *pParam;
		m_mapID2BP[uID] = bp;
	}
	return TRUE;
}
void CDraw2::RemoveDrawBitmap(unsigned int uID)
{
	ID2BPMap::iterator itfind = m_mapID2BP.find(uID);
	if(itfind != m_mapID2BP.end())
	{
		m_mapID2BP.erase(itfind);
	}
}
void CDraw2::SetBKColor(BK_DESC bk)
{
	m_bk.bkd = bk;
}
void CDraw2::SetAspectRatioMode(ASPECTRATIOMODE mode)
{
	m_armode = mode;
}
void CDraw2::SetImageEffect(DWORD dwImageEffect)
{
	m_dwImageEffect = dwImageEffect;
}
//VA_TYPE CDraw2::GetSupportedVAType()
//{
//	return VAT_Unsupported;
//}
//BOOL CDraw2::VADraw(VA_TYPE type,void* pContent,const RECT& rcSource,const RECT& rcTarget)
//{
//	return FALSE;
//}
BOOL CDraw2::UpdateDrawTargetRectangle(const RECT& rcTarget)
{// nothing
	return TRUE;
}
void CDraw2::WindowThreadAction()
{// nothing
	
}
BOOL CDraw2::IsNeedWindowThreadDestroy()
{
	return FALSE;
}
BOOL CDraw2::HasEffect()
{
	return (m_mapID2BP.size() != 0);
}
BOOL CDraw2::HasEffectActOnSrc()
{
	ID2BPMap::iterator it = m_mapID2BP.begin();
	for(;it!=m_mapID2BP.end();++it)
	{
		if(IS_ACTONSRC(it->second.bd.dwMask))
			return TRUE;
	}
	return FALSE;
}
BOOL CDraw2::HasEffectActOnDest()
{
	ID2BPMap::iterator it = m_mapID2BP.begin();
	for(;it!=m_mapID2BP.end();++it)
	{
		if(!IS_ACTONSRC(it->second.bd.dwMask))
			return TRUE;
	}
	return FALSE;
}
void CDraw2::TuneTargetRectangleByAspectRatioMode(RECT& rcDestTuned,RECT& rcSrcTuned,const RECT& rcSrc,const RECT& rcDest, DWORD dwSrcFourCC)
{
	rcDestTuned = rcDest;
	rcSrcTuned = rcSrc;
	switch(m_armode)
	{
	case ARM_LETTER_BOX:
		{
			DWORD dwSrcWidth = abs(rcSrc.right - rcSrc.left);
			DWORD dwSrcHeight = abs(rcSrc.bottom - rcSrc.top);
			
			DWORD dwDestWidth = abs(rcDest.right - rcDest.left);
			DWORD dwDestHeight = abs(rcDest.bottom - rcDest.top);

			float fSrcAspectRatio = (float)dwSrcWidth/dwSrcHeight;
			float fDestAspectRatio = (float)dwDestWidth/dwDestHeight;

			if(fSrcAspectRatio > fDestAspectRatio)
			{
				DWORD dwTunedDestHeight = (DWORD)(dwDestWidth/fSrcAspectRatio);
				SetRect(&rcDestTuned,
					rcDest.left,
					rcDest.top + (dwDestHeight - dwTunedDestHeight)/2,
					rcDest.right,
					rcDest.top + (dwDestHeight - dwTunedDestHeight)/2 + dwTunedDestHeight);
				
			}
			else if(fSrcAspectRatio < fDestAspectRatio)
			{
				DWORD dwTunedDestWidth = (DWORD)(dwDestHeight*fSrcAspectRatio);
				SetRect(&rcDestTuned,
					rcDest.left + (dwDestWidth - dwTunedDestWidth)/2,
					rcDest.top,
					rcDest.left + (dwDestWidth - dwTunedDestWidth)/2 + dwTunedDestWidth,
					rcDest.bottom);
			}
		}
		break;
	case ARM_STRETCH:
		{
			DWORD dwSrcWidth = abs(rcSrc.right - rcSrc.left);
			DWORD dwSrcHeight = abs(rcSrc.bottom - rcSrc.top);

			DWORD dwDestWidth = abs(rcDest.right - rcDest.left);
			DWORD dwDestHeight = abs(rcDest.bottom - rcDest.top);

			float fSrcAspectRatio = (float)dwSrcWidth/dwSrcHeight;
			float fDestAspectRatio = (float)dwDestWidth/dwDestHeight;
 
 			if(fabs(fSrcAspectRatio - fDestAspectRatio) < 0.01)
			{
 				return;
 			}

			if(fSrcAspectRatio < fDestAspectRatio)
			{
				DWORD dwTunedSrcHeight = (DWORD)(dwSrcWidth/fDestAspectRatio);
				DWORD dwCropHeight = (dwSrcHeight - dwTunedSrcHeight)/2;
				if(dwSrcFourCC == MAKEFOURCC('Y','U','Y','2')
					|| dwSrcFourCC == MAKEFOURCC('U','Y','V','Y'))
				{
					dwCropHeight -=(dwCropHeight % 4);
					dwTunedSrcHeight -= (dwTunedSrcHeight % 4);
				}
				else if(dwSrcFourCC == MAKEFOURCC('I','4','2','0')
					|| dwSrcFourCC == MAKEFOURCC('Y','V','1','2'))
				{
					//make sure it can be divide by 2
					dwCropHeight -=(dwCropHeight % 2);
					dwTunedSrcHeight -= (dwTunedSrcHeight % 2);
				}
				SetRect(&rcSrcTuned,
					rcSrc.left,
					rcSrc.top + dwCropHeight,
					rcSrc.right,
					rcSrc.top + dwCropHeight + dwTunedSrcHeight);
			}
			else
			{
				DWORD dwTunedSrcWidth = (DWORD)(dwSrcHeight*fDestAspectRatio);
				DWORD dwCropWidth = (dwSrcWidth - dwTunedSrcWidth)/2;
				if(dwSrcFourCC == MAKEFOURCC('Y','U','Y','2')|| dwSrcFourCC == MAKEFOURCC('U','Y','V','Y'))
				{
					dwCropWidth -=(dwCropWidth % 4);
					dwTunedSrcWidth -= (dwTunedSrcWidth % 4);
				}
				else if(dwSrcFourCC == MAKEFOURCC('I','4','2','0')
					|| dwSrcFourCC == MAKEFOURCC('Y','V','1','2'))
				{
					//make sure it can be divide by 2
					dwCropWidth -=(dwCropWidth % 2);
					dwTunedSrcWidth -= (dwTunedSrcWidth % 2);
				}
				SetRect(&rcSrcTuned,
					rcSrc.left + dwCropWidth,
					rcSrc.top,
					rcSrc.left + dwCropWidth + dwTunedSrcWidth,
					rcSrc.bottom);
			}
		}
		break;
	default:
		break;
	}
}
//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
#include "direct3d9creater.h"

CDraw* CreateDirect3D(HWND hWnd,IDrawHelper* pHelper)
{
	CDraw* pDraw = NULL;
	pDraw = CreateDirect3D9ExDraw(hWnd,pHelper);
	if(pDraw)
		return pDraw;

 	/*pDraw = CreateDirect3D9Draw(hWnd,pHelper);*/
 //	if(pDraw)
 //		return pDraw;

	//pDraw = CreateDirect3D7Draw(hWnd,pHelper);
	//if(pDraw)
	//	return pDraw;

	return NULL;
}


void DestoryDraw(CDraw* pDraw)
{
	if(pDraw)
		delete pDraw;
}