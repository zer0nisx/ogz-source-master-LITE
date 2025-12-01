// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__9B9C5456_EF8F_4C55_BD95_835C64A3A9F1__INCLUDED_)
#define AFX_STDAFX_H__9B9C5456_EF8F_4C55_BD95_835C64A3A9F1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "targetver.h"

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#define NOMINMAX            // Prevent Windows.h from defining min/max macros

// Workaround for wincodec.h requiring DXGI types not available in DirectX 9 SDK
// Define these types before Windows headers are included
#ifndef DXGI_JPEG_AC_HUFFMAN_TABLE
typedef struct DXGI_JPEG_AC_HUFFMAN_TABLE {
    unsigned char CodeCounts[16];
    unsigned char CodeValues[162];
} DXGI_JPEG_AC_HUFFMAN_TABLE;
#endif
#ifndef DXGI_JPEG_DC_HUFFMAN_TABLE
typedef struct DXGI_JPEG_DC_HUFFMAN_TABLE {
    unsigned char CodeCounts[12];
    unsigned char CodeValues[12];
} DXGI_JPEG_DC_HUFFMAN_TABLE;
#endif
#ifndef DXGI_JPEG_QUANTIZATION_TABLE
typedef struct DXGI_JPEG_QUANTIZATION_TABLE {
    unsigned char Elements[64];
} DXGI_JPEG_QUANTIZATION_TABLE;
#endif

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

// DirectX 9 includes for D3DX functions
#include <d3dx9math.h>

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__9B9C5456_EF8F_4C55_BD95_835C64A3A9F1__INCLUDED_)
