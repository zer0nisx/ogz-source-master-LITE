// ImGui Win32 + DirectX9 binding for WorldEdit
// Modular implementation for WorldEdit tool

#pragma once

struct IDirect3DDevice9;
struct HWND__;
typedef HWND__* HWND;

// Initialize ImGui with DirectX 9
IMGUI_API bool        ImGui_ImplDX9_Init(HWND hwnd, IDirect3DDevice9* device);
IMGUI_API void        ImGui_ImplDX9_Shutdown();

// New frame - call this at the start of each frame
IMGUI_API void        ImGui_ImplDX9_NewFrame();

// Device management - call when device is lost/reset
IMGUI_API void        ImGui_ImplDX9_InvalidateDeviceObjects();
IMGUI_API bool        ImGui_ImplDX9_CreateDeviceObjects();

// Window procedure handler - call this in your WndProc
IMGUI_API LRESULT     ImGui_ImplDX9_WndProcHandler(HWND hWnd, unsigned int msg, unsigned long long wParam, long long lParam);

