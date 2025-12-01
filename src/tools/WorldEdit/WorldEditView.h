// WorldEditView.h - ImGui version (no MFC)
#pragma once

#include "RTypes.h"
#include "RBspObject.h"
#include "GlobalTypes.h"
#include "MTime.h"
#include "WorldEditDoc.h"

_USING_NAMESPACE_REALSPACE2

enum EDITMODE {
    EDITMODE_OBJECT,
    EDITMODE_PATH,
    EDITMODE_TRIGGER,
};

class WorldEditView
{
public:
    WorldEditView();
    ~WorldEditView();

    bool Initialize(HWND hWnd);
    void Shutdown();
    void Update(float deltaTime);
    void Render();
    void HandleInput();

    // Camera control
    void ResetCamera();
    void GetWorldCoordinate(rvector* ret, int x, int y);

    // Mouse/Keyboard input
    void OnLButtonDown(int x, int y);
    void OnLButtonUp(int x, int y);
    void OnMouseMove(int x, int y, bool lButton, bool shift, bool alt);
    void OnMouseWheel(int delta);

    // Properties
    EDITMODE m_EditMode = EDITMODE_OBJECT;
    bool m_bWireframe = false;
    bool m_bDrawBoundingBox = false;
    bool m_bDrawOcclusion = false;
    bool m_bShowLightmap = false;

    // Document reference
    WorldEditDoc* m_pDoc = nullptr;

private:
    bool m_bLastShiftState = false;
    bool m_bLastAltState = false;
    v3 m_LastWorldPosition{ 0, 0, 0 };
    v3 m_LastCameraPosition{ 0, 0, 0 };
    rmatrix m_LastMatrix = GetIdentityMatrix();
    int m_LastCursorX = 0;
    int m_LastCursorY = 0;
    int m_LastFrameCursorX = 0;
    int m_LastFrameCursorY = 0;

    RSBspNode* m_pSelectedNode = nullptr;
    int m_nSelectedIndex = 0;
    int m_nSelectedEdge = 0;

    u64 m_LastTime = GetGlobalTimeMS();
    HWND m_hWnd = nullptr;

    void DoMovement(float deltaTime);
    void HandleCameraRotation(int dx, int dy);
    void HandleCameraPan(int dx, int dy, int x, int y);
    void HandleCameraOrbit(int dx, int dy);
};
