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
// Define these types BEFORE any Windows headers are included
// These types are required by wincodec.h from Windows SDK but not available in DirectX 9 SDK
// Must be defined as struct types (not typedef) to match Windows SDK expectations
#ifdef __cplusplus
extern "C" {
#endif
struct DXGI_JPEG_AC_HUFFMAN_TABLE {
    unsigned char CodeCounts[16];
    unsigned char CodeValues[162];
};
struct DXGI_JPEG_DC_HUFFMAN_TABLE {
    unsigned char CodeCounts[12];
    unsigned char CodeValues[12];
};
struct DXGI_JPEG_QUANTIZATION_TABLE {
    unsigned char Elements[64];
};
#ifdef __cplusplus
}
#endif

// Prevent redefinition warnings from DirectX SDK headers conflicting with Windows SDK
// These macros are already defined in winerror.h from Windows SDK 10.0.26100.0+
#ifndef D2DERR_WRONG_RESOURCE_DOMAIN
#define D2DERR_WRONG_RESOURCE_DOMAIN 0x88990001L
#endif
#ifndef D2DERR_PUSH_POP_UNBALANCED
#define D2DERR_PUSH_POP_UNBALANCED 0x88990002L
#endif
#ifndef D2DERR_RENDER_TARGET_HAS_LAYER_OR_CLIPRECT
#define D2DERR_RENDER_TARGET_HAS_LAYER_OR_CLIPRECT 0x88990003L
#endif
#ifndef D2DERR_INCOMPATIBLE_BRUSH_TYPES
#define D2DERR_INCOMPATIBLE_BRUSH_TYPES 0x88990004L
#endif
#ifndef D2DERR_WIN32_ERROR
#define D2DERR_WIN32_ERROR 0x88990005L
#endif
#ifndef D2DERR_TARGET_NOT_GDI_COMPATIBLE
#define D2DERR_TARGET_NOT_GDI_COMPATIBLE 0x88990006L
#endif
#ifndef D2DERR_TEXT_EFFECT_IS_WRONG_TYPE
#define D2DERR_TEXT_EFFECT_IS_WRONG_TYPE 0x88990007L
#endif
#ifndef D2DERR_TEXT_RENDERER_NOT_RELEASED
#define D2DERR_TEXT_RENDERER_NOT_RELEASED 0x88990008L
#endif
#ifndef D2DERR_EXCEEDS_MAX_BITMAP_SIZE
#define D2DERR_EXCEEDS_MAX_BITMAP_SIZE 0x88990009L
#endif

#ifndef D3D10_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS
#define D3D10_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS 0x88790001L
#endif
#ifndef D3D10_ERROR_FILE_NOT_FOUND
#define D3D10_ERROR_FILE_NOT_FOUND 0x88790002L
#endif

#ifndef DWRITE_E_FILEFORMAT
#define DWRITE_E_FILEFORMAT 0x88985001L
#endif
#ifndef DWRITE_E_UNEXPECTED
#define DWRITE_E_UNEXPECTED 0x88985002L
#endif
#ifndef DWRITE_E_NOFONT
#define DWRITE_E_NOFONT 0x88985003L
#endif
#ifndef DWRITE_E_FILENOTFOUND
#define DWRITE_E_FILENOTFOUND 0x88985004L
#endif
#ifndef DWRITE_E_FILEACCESS
#define DWRITE_E_FILEACCESS 0x88985005L
#endif
#ifndef DWRITE_E_FONTCOLLECTIONOBSOLETE
#define DWRITE_E_FONTCOLLECTIONOBSOLETE 0x88985006L
#endif

// DWRITE_E_ALREADYREGISTERED may be defined in Windows SDK 10.0.26100.0+
// We'll define it after Windows headers if needed

// Windows headers
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shlobj.h>
#include <shellapi.h>  // For drag and drop (HDROP, DragQueryFileW, etc.)

// Define DWRITE_E_ALREADYREGISTERED if Windows SDK doesn't have it
// (Windows SDK 10.0.26100.0+ includes it, so this prevents redefinition)
#ifndef DWRITE_E_ALREADYREGISTERED
#define DWRITE_E_ALREADYREGISTERED 0x88985007L
#endif

// Protect against Windows macros that conflict with ImGui
#ifdef GetWindowFont
#undef GetWindowFont
#endif

// DirectX 9 includes
#include <d3d9.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

// Include MUtil.h before RealSpace2 to get EXPAND_VECTOR macro
#include "MUtil.h"

// ImGui del SDK (must be after Windows headers to avoid macro conflicts)
#include <imgui.h>
#include "imgui_impl_dx9.h"

// RealSpace2
#include "RealSpace2.h"

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__9B9C5456_EF8F_4C55_BD95_835C64A3A9F1__INCLUDED_)
