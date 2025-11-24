#include "stdafx.h"
#include "MaterialTestApp.h"
#include "RealSpace2.h"
#include "RFrameWork.h"
#include "MDebug.h"
#include "MZFileSystem.h"
#include <Windows.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#ifdef _USE_GDIPLUS
#include "unknwn.h"
#include "gdiplus.h"
using namespace Gdiplus;
#endif

#pragma comment(lib, "winmm.lib")

// Callbacks de RealSpace2
RRESULT OnCreate(void* pParam)
{
    MaterialTestApp::GetInstance().OnCreate();
    return R_OK;
}

RRESULT OnDestroy(void* pParam)
{
    MaterialTestApp::GetInstance().OnDestroy();
    return R_OK;
}

RRESULT OnUpdate(void* pParam)
{
    MaterialTestApp::GetInstance().OnUpdate();
    return R_OK;
}

RRESULT OnRender(void* pParam)
{
    MaterialTestApp::GetInstance().OnRender();
    return R_OK;
}

RRESULT OnRestore(void* pParam)
{
    MaterialTestApp::GetInstance().OnRestore();
    return R_OK;
}

RRESULT OnInvalidate(void* pParam)
{
    MaterialTestApp::GetInstance().OnInvalidate();
    return R_OK;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE)
        {
            PostQuitMessage(0);
            return 0;
        }
        break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Obtener directorio del ejecutable y establecerlo como directorio de trabajo
    char szModuleFileName[_MAX_DIR];
    szModuleFileName[0] = 0;
    GetModuleFileName(NULL, szModuleFileName, _MAX_DIR);
    PathRemoveFileSpec(szModuleFileName);
    SetCurrentDirectory(szModuleFileName);

    // Inicializar log en la misma carpeta del ejecutable
    char szLogPath[_MAX_PATH];
    sprintf_safe(szLogPath, "%s\\MaterialTest_%s.log", szModuleFileName, MGetStrLocalTime());
    InitLog(MLOGSTYLE_DEBUGSTRING | MLOGSTYLE_FILE, szLogPath);

	// Inicializar sistema de archivos (requerido por RealSpace2)
	MZFileSystem* pFileSystem = new MZFileSystem;
	
	// Inicializar el sistema de archivos con el directorio del ejecutable
	// Esto es necesario para que MZFileSystem pueda encontrar los archivos
	if (!pFileSystem->Create(szModuleFileName))
	{
		MLog("MaterialTest: ERROR - No se pudo inicializar el sistema de archivos\n");
		delete pFileSystem;
		return -1;
	}
	
	RSetFileSystem(pFileSystem);

    // Parámetros de modo de video
    RMODEPARAMS ModeParams;
    ModeParams.nWidth = 1024;
    ModeParams.nHeight = 768;
    ModeParams.FullscreenMode = FullscreenType::Windowed;
    ModeParams.PixelFormat = D3DFMT_X8R8G8B8;

    // Configurar callbacks
    RSetFunction(RF_CREATE, OnCreate);
    RSetFunction(RF_DESTROY, OnDestroy);
    RSetFunction(RF_UPDATE, OnUpdate);
    RSetFunction(RF_RENDER, OnRender);
    RSetFunction(RF_RESTORE, OnRestore);
    RSetFunction(RF_INVALIDATE, OnInvalidate);

    // Iniciar RealSpace2
    int result = RMain(
        "Material Test - RealSpace2",
        hInstance,
        hPrevInstance,
        lpCmdLine,
        nCmdShow,
        &ModeParams,
        WndProc,
        0,  // Icon ID
        GraphicsAPI::D3D9
    );

    return result;
}

