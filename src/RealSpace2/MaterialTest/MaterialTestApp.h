#pragma once

#include "RealSpace2.h"
#include "RShaderMgr.h"
#include "RMesh.h"
#include "RVisualMesh.h"
#include "RMtrl.h"
#include <memory>

class MaterialTestApp
{
public:
    static MaterialTestApp& GetInstance()
    {
        static MaterialTestApp instance;
        return instance;
    }

    void OnCreate();
    void OnDestroy();
    void OnUpdate();
    void OnRender();
    void OnRestore();
    void OnInvalidate();

private:
    MaterialTestApp() = default;
    ~MaterialTestApp() = default;
    MaterialTestApp(const MaterialTestApp&) = delete;
    MaterialTestApp& operator=(const MaterialTestApp&) = delete;

    // Mesh de prueba (opcional - puede ser NULL para usar RDrawSphere)
    std::unique_ptr<RMesh> m_pTestMesh;
    RVisualMesh* m_pVisualMesh;  // Puntero al visual mesh del RMesh (no se posee)
    
    // Material de prueba
    RMtrl m_TestMaterial;
    
    // Luces de prueba
    D3DLIGHT9 m_Light0;
    D3DLIGHT9 m_Light1;
    
    // Variables de control
    float m_fRotationAngle;
    float m_fTime;
    
    // Inicialización
    void CreateTestMesh();
    void SetupLights();
    void SetupMaterial();
    void SetupCamera();
};

