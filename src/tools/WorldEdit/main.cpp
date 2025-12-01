// WorldEdit - Main entry point using ImGui instead of MFC
// Replaces WorldEdit.cpp

#include "stdafx.h"
#include "RBspObject.h"
#include "MDebug.h"
#include "RMeshMgr.h"
#include "MTime.h"
#include "defer.h"
#include "WorldEditView.h"
#include "WorldEditDoc.h"
#include <algorithm>
#include <commdlg.h>

_USING_NAMESPACE_REALSPACE2

// ============================================================================
// Forward Declarations
// ============================================================================

class WorldEditApp;

// Application state
static WorldEditApp* g_pApp = nullptr;
static WorldEditView* g_pView = nullptr;
static WorldEditDoc* g_pDoc = nullptr;
static HWND g_hMainWnd = nullptr;  // Renamed to avoid conflict with RealSpace2::g_hWnd
static LPDIRECT3DDEVICE9 g_pd3dDevice = nullptr;
static bool g_bRunning = true;
static bool g_bImGuiWantsMouse = false;
static bool g_bImGuiWantsKeyboard = false;

// ============================================================================
// Window Procedure
// ============================================================================

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Check if ImGui wants to handle this message
	ImGuiIO& io = ImGui::GetIO();
	g_bImGuiWantsMouse = io.WantCaptureMouse;
	g_bImGuiWantsKeyboard = io.WantCaptureKeyboard;

	if (ImGui_ImplDX9_WndProcHandler(hWnd, msg, wParam, lParam))
	{
		// ImGui handled it, but we might still need to process for 3D view
		if (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONUP || msg == WM_MOUSEMOVE || msg == WM_MOUSEWHEEL)
		{
			// If ImGui doesn't want mouse, pass to 3D view
			if (!g_bImGuiWantsMouse && g_pView)
			{
				int x = LOWORD(lParam);
				int y = HIWORD(lParam);
				bool shift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
				bool alt = (GetKeyState(VK_MENU) & 0x8000) != 0;

				switch (msg)
				{
				case WM_LBUTTONDOWN:
					g_pView->OnLButtonDown(x, y);
					break;
				case WM_LBUTTONUP:
					g_pView->OnLButtonUp(x, y);
					break;
				case WM_MOUSEMOVE:
					g_pView->OnMouseMove(x, y, (wParam & MK_LBUTTON) != 0, shift, alt);
					break;
				case WM_MOUSEWHEEL:
					g_pView->OnMouseWheel(GET_WHEEL_DELTA_WPARAM(wParam));
					break;
				}
			}
		}
		return true;
	}

	switch (msg)
	{
	case WM_SIZE:
		if (g_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED)
		{
			ImGui_ImplDX9_InvalidateDeviceObjects();
			D3DPRESENT_PARAMETERS d3dpp;
			ZeroMemory(&d3dpp, sizeof(d3dpp));
			d3dpp.Windowed = TRUE;
			d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
			d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
			d3dpp.EnableAutoDepthStencil = TRUE;
			d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
			d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
			d3dpp.BackBufferWidth = (UINT)LOWORD(lParam);
			d3dpp.BackBufferHeight = (UINT)HIWORD(lParam);

			HRESULT hr = g_pd3dDevice->Reset(&d3dpp);
			if (hr == D3DERR_INVALIDCALL)
				return 0;
			ImGui_ImplDX9_CreateDeviceObjects();

			// Update RealSpace2
			RMODEPARAMS mparams = {
				(int)d3dpp.BackBufferWidth,
				(int)d3dpp.BackBufferHeight,
				FullscreenType::Windowed,
				D3DFMT_R5G6B5
			};
			if (g_pView && g_pDoc && g_pDoc->m_pBspObject)
				g_pDoc->m_pBspObject->OnInvalidate();
			RResetDevice(&mparams);
			if (g_pView && g_pDoc && g_pDoc->m_pBspObject)
				g_pDoc->m_pBspObject->OnRestore();
		}
		return 0;

	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		g_bRunning = false;
		return 0;

	case WM_DROPFILES:
	{
		HDROP hDrop = (HDROP)wParam;
		wchar_t szFileName[MAX_PATH];
		DragQueryFileW(hDrop, 0, szFileName, MAX_PATH);
		DragFinish(hDrop);

		if (g_pDoc)
		{
			g_pDoc->OpenDocument(szFileName);
		}
	}
	return 0;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

// ============================================================================
// Application Class (simplified, replaces MFC CWorldEditApp)
// ============================================================================

class WorldEditApp
{
public:
	WorldEditApp()
	{
		CoInitialize(nullptr);
		InitLog(MLOGSTYLE_FILE | MLOGSTYLE_DEBUGSTRING);
	}

	~WorldEditApp()
	{
		CoUninitialize();
	}

	bool Initialize(HINSTANCE hInstance)
	{
		// Register window class
		WNDCLASSEXW wc = {
			sizeof(WNDCLASSEXW),
			CS_CLASSDC,
			WndProc,
			0L,
			0L,
			hInstance,
			nullptr,
			LoadCursor(nullptr, IDC_ARROW),
			nullptr,
			nullptr,
			L"WorldEditClass",
			nullptr
		};

		if (!RegisterClassExW(&wc))
			return false;

		// Create window
		g_hMainWnd = CreateWindowW(
			wc.lpszClassName,
			L"WorldEdit",
			WS_OVERLAPPEDWINDOW,
			100, 100, 1280, 800,
			nullptr, nullptr, hInstance, nullptr
		);

		if (!g_hMainWnd)
			return false;

		// Initialize DirectX 9
		LPDIRECT3D9 pD3D = Direct3DCreate9(D3D_SDK_VERSION);
		if (!pD3D)
			return false;

		D3DPRESENT_PARAMETERS d3dpp;
		ZeroMemory(&d3dpp, sizeof(d3dpp));
		d3dpp.Windowed = TRUE;
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
		d3dpp.EnableAutoDepthStencil = TRUE;
		d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
		d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

		RECT rect;
		GetClientRect(g_hMainWnd, &rect);
		d3dpp.BackBufferWidth = rect.right - rect.left;
		d3dpp.BackBufferHeight = rect.bottom - rect.top;

		if (pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, g_hMainWnd,
			D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &g_pd3dDevice) < 0)
		{
			pD3D->Release();
			return false;
		}

		pD3D->Release();

		// Initialize RealSpace2
		RMODEPARAMS mparams = {
			(int)d3dpp.BackBufferWidth,
			(int)d3dpp.BackBufferHeight,
			FullscreenType::Windowed,
			D3DFMT_R5G6B5
		};

		if (!RInitDisplay(g_hMainWnd, nullptr, &mparams, GraphicsAPI::D3D9))
		{
			MessageBoxW(g_hMainWnd, L"Cannot Initialize 3D Engine.", L"Error", MB_OK);
			return false;
		}

		RSetRenderFlags(RRENDER_CLEAR_BACKBUFFER);

		// Initialize ImGui
		ImGui_ImplDX9_Init(g_hMainWnd, g_pd3dDevice);

		// Setup ImGui style
		ImGui::StyleColorsClassic();

		// Set initial window size for ImGui
		ImGuiIO& io = ImGui::GetIO();
		// Reuse rect variable from above
		GetClientRect(g_hMainWnd, &rect);
		io.DisplaySize = ImVec2((float)(rect.right - rect.left), (float)(rect.bottom - rect.top));

		// Create document and view
		g_pDoc = new WorldEditDoc();
		g_pView = new WorldEditView();
		g_pView->m_pDoc = g_pDoc;
		g_pView->Initialize(g_hMainWnd);

		// Enable drag and drop
		DragAcceptFiles(g_hMainWnd, TRUE);

		ShowWindow(g_hMainWnd, SW_SHOWDEFAULT);
		UpdateWindow(g_hMainWnd);

		return true;
	}

	void Shutdown()
	{
		if (g_pView)
		{
			g_pView->Shutdown();
			delete g_pView;
			g_pView = nullptr;
		}

		if (g_pDoc)
		{
			delete g_pDoc;
			g_pDoc = nullptr;
		}

		if (g_pd3dDevice)
		{
			ImGui_ImplDX9_Shutdown();
			RCloseDisplay();
			g_pd3dDevice->Release();
			g_pd3dDevice = nullptr;
		}

		if (g_hMainWnd)
		{
			DestroyWindow(g_hMainWnd);
			g_hMainWnd = nullptr;
		}

		UnregisterClassW(L"WorldEditClass", GetModuleHandle(nullptr));
	}

	void Run()
	{
		MSG msg = {};
		u64 lastTime = GetGlobalTimeMS();

		while (g_bRunning && msg.message != WM_QUIT)
		{
			while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				if (msg.message == WM_QUIT)
					g_bRunning = false;
			}

			if (!g_bRunning)
				break;

			// Calculate delta time
			u64 currentTime = GetGlobalTimeMS();
			float deltaTime = min((currentTime - lastTime) / 1000.0f, 1.0f);
			lastTime = currentTime;

			// Update view
			if (g_pView)
				g_pView->Update(deltaTime);

			// Start ImGui frame
			ImGui_ImplDX9_NewFrame();

			// Render ImGui UI
			RenderUI();

			// Begin scene using RealSpace2
			if (RBeginScene())
			{
				// Render 3D scene (RealSpace2)
				if (g_pView)
				{
					g_pView->Render();
				}

				// Render ImGui (must be called while BeginScene is active)
				// ImGui::Render() will call RenderDrawListsFn which needs the device in BeginScene state
				ImGui::Render();

				// End scene
				REndScene();

				// Present and clear for next frame
				g_pd3dDevice->Present(nullptr, nullptr, nullptr, nullptr);
				RClear();
			}
		}
	}

private:
	void RenderUI()
	{
		// Main menu bar
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open...", "Ctrl+O"))
				{
					OpenFileDialog();
				}
				if (ImGui::MenuItem("Close"))
				{
					if (g_pDoc)
						g_pDoc->CloseDocument();
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Exit", "Alt+F4"))
				{
					PostMessage(g_hMainWnd, WM_CLOSE, 0, 0);
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View"))
			{
				if (g_pView)
				{
					ImGui::MenuItem("Wireframe", nullptr, &g_pView->m_bWireframe);
					ImGui::MenuItem("Bounding Box", nullptr, &g_pView->m_bDrawBoundingBox);
					ImGui::MenuItem("Occlusion", nullptr, &g_pView->m_bDrawOcclusion);
					ImGui::MenuItem("Show Lightmap", nullptr, &g_pView->m_bShowLightmap);
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Camera"))
			{
				if (ImGui::MenuItem("Reset Camera"))
				{
					if (g_pView)
						g_pView->ResetCamera();
				}
				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

		// Status/info window
		ImGui::Begin("Status");
		if (g_pDoc && g_pDoc->IsDocumentOpen())
		{
			ImGui::Text("Document: %ls", g_pDoc->GetFilePath().c_str());
			if (g_pDoc->m_pBspObject)
			{
				ImGui::Text("BSP Object: Loaded");
			}
		}
		else
		{
			ImGui::Text("No document open");
		}
		ImGui::End();

		// Properties window
		if (g_pView)
		{
			ImGui::Begin("Properties");
			ImGui::Text("Edit Mode:");
			const char* modes[] = { "Object", "Path", "Trigger" };
			int currentMode = (int)g_pView->m_EditMode;
			if (ImGui::Combo("##Mode", &currentMode, modes, 3))
			{
				g_pView->m_EditMode = (EDITMODE)currentMode;
			}
			ImGui::End();
		}
	}

	void OpenFileDialog()
	{
		OPENFILENAMEW ofn;
		wchar_t szFile[MAX_PATH] = L"";

		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = g_hMainWnd;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrFilter = L"Realspace Scene File (*.rs)\0*.rs\0All Files (*.*)\0*.*\0";
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = nullptr;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = nullptr;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

		if (GetOpenFileNameW(&ofn))
		{
			if (g_pDoc)
			{
				g_pDoc->OpenDocument(szFile);
				if (g_pView)
					g_pView->ResetCamera();
			}
		}
	}
};

// ============================================================================
// Main Entry Point
// ============================================================================

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	g_pApp = new WorldEditApp();

	if (!g_pApp->Initialize(hInstance))
	{
		delete g_pApp;
		return 1;
	}

	g_pApp->Run();
	g_pApp->Shutdown();

	delete g_pApp;
	return 0;
}