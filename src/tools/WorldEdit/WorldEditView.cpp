// WorldEditView.cpp - ImGui version (no MFC)
#include "stdafx.h"
#include "WorldEditView.h"
#include "WorldEditDoc.h"
#include "RBspObject.h"
#include "MDebug.h"
#include "RMeshMgr.h"
#include <algorithm>
#include "defer.h"

#define DEFAULTSIZE 100.f
static rvector g_LastPickPos;

WorldEditView::WorldEditView()
{
}

WorldEditView::~WorldEditView()
{
    Shutdown();
}

bool WorldEditView::Initialize(HWND hWnd)
{
    m_hWnd = hWnd;
    ResetCamera();
    return true;
}

void WorldEditView::Shutdown()
{
    m_hWnd = nullptr;
}

void WorldEditView::Update(float deltaTime)
{
    DoMovement(deltaTime);
}

void WorldEditView::DoMovement(float deltaTime)
{
    auto GetKey = [&](auto ch) {
        return (GetAsyncKeyState(ch) & 0x8000) != 0;
    };

    auto Forward = Normalized(RCameraDirection);
    auto Right = Normalized(CrossProduct(Forward, { 0, 0, -1 }));

    v3 dir{ 0, 0, 0 };

    if (GetKey('W'))
        dir += Forward;
    if (GetKey('A'))
        dir += -Right;
    if (GetKey('S'))
        dir += -Forward;
    if (GetKey('D'))
        dir += Right;

    Normalize(dir);

    RCameraPosition += dir * 2000 * deltaTime;
    RUpdateCamera();
}

// Global progress flag (simple version without MFC)
static bool g_bProgress = false;

void WorldEditView::Render()
{
    // Always render something, even if no document is open
    // This helps verify the render pipeline is working
    
    if (!m_pDoc || !m_pDoc->m_pBspObject)
    {
        // Render a simple test to verify 3D rendering works
        rmatrix id;
        GetIdentityMatrix(id);
        RSetTransform(D3DTS_WORLD, id);
        
        // Draw a simple line to verify rendering
        RDrawLine(rvector(0, 0, 0), rvector(100, 100, 0), 0xffff0000);
        return;
    }
    
    if (g_bProgress)
        return;

    rmatrix id;
    GetIdentityMatrix(id);
    RSetTransform(D3DTS_WORLD, id);

    if (m_bWireframe)
    {
        RGetDevice()->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
        RGetDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);
    }
    else
    {
        RGetDevice()->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    }

    m_pDoc->m_pBspObject->SetWireframeMode(m_bWireframe);
    m_pDoc->m_pBspObject->SetShowLightmapMode(m_bShowLightmap);

    if (m_EditMode != EDITMODE_PATH)
        m_pDoc->m_pBspObject->Draw();
        
    if (m_bDrawBoundingBox)
        m_pDoc->m_pBspObject->DrawBoundingBox();
        
    if (m_bDrawOcclusion)
        m_pDoc->m_pBspObject->DrawOcclusions();

    if (m_EditMode == EDITMODE_PATH)
    {
        auto& pBsp = m_pDoc->m_pBspObject;

        pBsp->Draw();

        RGetDevice()->SetTexture(0, nullptr);
        RGetDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);
        RGetDevice()->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);

        RBSPPICKINFO* ppi = &m_pDoc->m_LastPicked;
        if (m_pDoc->m_bLastPicked)
        {
            ppi->pNode->DrawWireFrame(ppi->nIndex, 0xffffffff);
            ppi->pNode->DrawBoundingBox(0xffff0000);

            POINT p;
            GetCursorPos(&p);
            ScreenToClient(m_hWnd, &p);

            rvector pos, dir, to;
            RGetScreenLine(p.x, p.y, &pos, &dir);
            to = pos + dir;

            rvector worldpos;
            IntersectLineSegmentPlane(&worldpos, ppi->pInfo->plane, pos, to);

            rvector normal;
            pBsp->GetNormal(ppi->pInfo->nConvexPolygon, worldpos, &normal);
            RDrawLine(worldpos, worldpos + normal * 100, 0xffff0000);

            RGetDevice()->SetRenderState(D3DRS_ZENABLE, FALSE);

            int nS = ppi->pInfo->nConvexPolygon;
            pBsp->DrawNormal(nS, 100);
        }
    }
}

void WorldEditView::ResetCamera()
{
    rboundingbox* pbb, defaultbb;
    defaultbb.vmin = { 0, 0, 0 };
    defaultbb.vmax = { DEFAULTSIZE, DEFAULTSIZE, DEFAULTSIZE };

    if (!m_pDoc || !m_pDoc->m_pBspObject || !m_pDoc->m_pBspObject->GetRootNode())
        pbb = &defaultbb;
    else
        pbb = &m_pDoc->m_pBspObject->GetRootNode()->bbTree;

    auto size = pbb->vmax - pbb->vmin;

    rvector targetpos = .5f * (pbb->vmax + pbb->vmin);
    targetpos.z = 0;
    rvector sourcepos = targetpos + rvector(0, 100, 100);
    RSetCamera(sourcepos, targetpos, rvector(0, 0, 1));
    RSetProjection(1.f / 3.f * PI_FLOAT, 100, 55000);
}

void WorldEditView::GetWorldCoordinate(rvector* ret, int x, int y)
{
    *ret = RGetIntersection(x, y, rplane(0, 0, 1, 0));
}

void WorldEditView::OnLButtonDown(int x, int y)
{
    m_bLastShiftState = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
    m_bLastAltState = (GetKeyState(VK_MENU) & 0x8000) != 0;
    m_LastMatrix = RViewProjectionViewport;
    m_LastCameraPosition = RCameraPosition;
    m_LastCursorX = x;
    m_LastCursorY = y;
    m_LastFrameCursorX = x;
    m_LastFrameCursorY = y;

    if (m_bLastShiftState)
    {
        GetWorldCoordinate(&m_LastWorldPosition, x, y);
    }

    if (m_bLastAltState)
    {
        GetWorldCoordinate(&m_LastWorldPosition, RGetScreenWidth() / 2, RGetScreenHeight() / 2);
    }

    if (m_EditMode == EDITMODE_PATH && m_pDoc && m_pDoc->m_pBspObject)
    {
        rvector pos, dir;
        RGetScreenLine(x, y, &pos, &dir);
        m_pDoc->m_bLastPicked = m_pDoc->m_pBspObject->PickOcTree(pos, dir, &m_pDoc->m_LastPicked, RM_FLAG_ADDITIVE | RM_FLAG_HIDE);
    }
}

void WorldEditView::OnLButtonUp(int x, int y)
{
    if (m_pDoc && m_pDoc->m_pBspObject)
    {
        RBSPPICKINFO bpi;
        rvector pos, dir;
        RGetScreenLine(x, y, &pos, &dir);
        if (m_pDoc->m_pBspObject->Pick(pos, dir, &bpi))
        {
            g_LastPickPos = bpi.PickPos;
        }
    }
}

void WorldEditView::OnMouseMove(int x, int y, bool lButton, bool shift, bool alt)
{
    m_LastFrameCursorX = x;
    m_LastFrameCursorY = y;

    if (lButton)
    {
        if (shift)
        {
            HandleCameraPan(x - m_LastCursorX, y - m_LastCursorY, x, y);
        }
        else if (alt)
        {
            HandleCameraOrbit(x - m_LastCursorX, y - m_LastCursorY);
        }
        else
        {
            HandleCameraRotation(x - m_LastFrameCursorX, y - m_LastFrameCursorY);
        }
    }
}

void WorldEditView::OnMouseWheel(int delta)
{
    GetWorldCoordinate(&m_LastWorldPosition, RGetScreenWidth() / 2, RGetScreenHeight() / 2);

    rvector dir = m_LastWorldPosition - RCameraPosition;
    Normalize(dir);

    const float CAMERA_WHEEL_STEP = 100.f;
    RCameraPosition += dir * CAMERA_WHEEL_STEP * (float)delta / (float)WHEEL_DELTA;
    RUpdateCamera();
}

void WorldEditView::HandleCameraRotation(int dx, int dy)
{
    auto dir = Normalized(RCameraDirection);

    float anglex{}, anglez{};
    {
        rvector a_dir = dir;
        float fAngleX = 0.0f, fAngleZ = 0.0f;

        fAngleX = acosf(a_dir.z);
        float fSinX = sinf(fAngleX);

        if (fSinX == 0) fAngleZ = 0.0f;
        else
        {
            float fT = (a_dir.x / fSinX);
            if (fT > 1.0f) fT = 1.0f;
            else if (fT < -1.0f) fT = -1.0f;

            float fZ1 = acosf(fT);

            if (IS_EQ((sinf(fZ1) * fSinX), dir.y))
            {
                fAngleZ = fZ1;
            }
            else
            {
                fAngleZ = 2 * PI_FLOAT - fZ1;
            }
        }

        anglex = fAngleX;
        anglez = fAngleZ;
    }

    auto clamp = [&](auto&& val, auto&& low, auto&& high) {
        return max(min(val, high), low);
    };

    anglex += 0.005f * dy;
    anglex = clamp(anglex, 0.001f, PI_FLOAT - 0.001f);
    anglez += 0.005f * dx;
    anglez = fmod(anglez, 2 * PI_FLOAT);

    v3 vec{
        cos(anglez) * sin(anglex),
        sin(anglez) * sin(anglex),
        cos(anglex) };

    RCameraDirection = vec;
    RUpdateCamera();
}

void WorldEditView::HandleCameraPan(int dx, int dy, int x, int y)
{
    rvector scrpoint = rvector((float)x, (float)y, 0.1f);

    rmatrix inv = Inverse(m_LastMatrix);
    rvector worldpoint = TransformCoord(scrpoint, inv);

    rplane plane = rplane(0, 0, 1, 0);
    rvector intpointdst;
    IntersectLineSegmentPlane(&intpointdst, plane, worldpoint, m_LastCameraPosition);

    rmatrix cameratm = TranslationMatrix(-m_LastCameraPosition) * m_LastMatrix;
    inv = Inverse(cameratm);

    rvector screenpoint = TransformCoord(intpointdst, m_LastMatrix);
    screenpoint = TransformCoord(screenpoint, inv);

    RCameraPosition = m_LastCameraPosition * 2 - (screenpoint - m_LastWorldPosition);
    RUpdateCamera();
}

void WorldEditView::HandleCameraOrbit(int dx, int dy)
{
    rvector relpos = m_LastCameraPosition - m_LastWorldPosition;
    float length = Magnitude(relpos);
    Normalize(relpos);

    float anglex, anglez;
    anglex = acos(relpos.z);
    anglez = asin(relpos.x / sin(anglex));
    if (relpos.y < 0)
        anglez = PI_FLOAT - anglez;

    anglex += -0.01f * dy;
    anglex = min(max(anglex, 0.001f), PI_FLOAT - 0.001f);
    anglez += -0.01f * dx;

    relpos = length * rvector(sin(anglez) * sin(anglex), cos(anglez) * sin(anglex), cos(anglex));

    rvector newcamerapos = m_LastWorldPosition + relpos;
    RSetCamera(newcamerapos, m_LastWorldPosition, rvector(0, 0, 1));
}
