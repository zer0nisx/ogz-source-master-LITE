#include "stdafx.h"
#include "MaterialTestApp.h"
#include "RMath.h"
#include "RMeshUtil.h"
#include "RShaderMgr.h"
#include "MDebug.h"
#include "MTime.h"
#include <d3d9.h>

void MaterialTestApp::OnCreate()
{
	m_fRotationAngle = 0.0f;
	m_fTime = 0.0f;

	// Inicializar shader manager
	if (RIsSupportVS())
	{
		auto* pShaderMgr = RGetShaderMgr();
		if (pShaderMgr)
		{
			if (!pShaderMgr->Initialize())
			{
				MLog("MaterialTestApp: Error al inicializar shader manager\n");
			}
			else
			{
				pShaderMgr->SetEnable();
				MLog("MaterialTestApp: Shader manager inicializado\n");
			}
		}
	}

	// Configurar material PRIMERO (necesario antes de modificar materiales del mesh)
	SetupMaterial();

	// Crear mesh de prueba (esfera simple)
	CreateTestMesh();

	// Configurar luces
	SetupLights();

	// Configurar cámara
	SetupCamera();

	MLog("MaterialTestApp: Inicializado correctamente\n");
}

void MaterialTestApp::OnDestroy()
{
	// Destruir RVisualMesh antes del RMesh
	if (m_pVisualMesh)
	{
		m_pVisualMesh->Destroy();
		delete m_pVisualMesh;
		m_pVisualMesh = nullptr;
	}
	m_pTestMesh.reset();
	MLog("MaterialTestApp: Destruido\n");
}

void MaterialTestApp::OnUpdate()
{
	// Actualizar tiempo
	static auto lastTime = GetGlobalTimeMS();
	auto currentTime = GetGlobalTimeMS();
	float deltaTime = (currentTime - lastTime) / 1000.0f;
	lastTime = currentTime;

	m_fTime += deltaTime;
	m_fRotationAngle += deltaTime * 45.0f;  // Rotar 45 grados por segundo

	// Actualizar posición de las luces (opcional - para pruebas dinámicas)
	// m_Light0.Position = rvector(cos(m_fTime) * 100.0f, sin(m_fTime) * 100.0f, 50.0f);
}

void MaterialTestApp::OnRender()
{
	auto* pDevice = RGetDevice();
	if (!pDevice)
		return;

	// Actualizar cámara antes de renderizar
	RUpdateCamera();

	// Limpiar pantalla (RealSpace2 ya llama BeginScene antes de este callback)
	pDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xFF000020, 1.0f, 0);

	// Configurar material y luces en el shader manager
	if (RIsSupportVS())
	{
		auto* pShaderMgr = RGetShaderMgr();
		if (pShaderMgr)
		{
			// Configurar material
			pShaderMgr->setMtrl(&m_TestMaterial, 1.0f);

			// Actualizar shader (las luces ya están configuradas en SetupLights)
			pShaderMgr->Update();
		}
	}

	// Configurar transformación del mundo (rotación)
	rmatrix WorldMatrix = RGetRotYRad(ToRadian(m_fRotationAngle));

	// Renderizar mesh o geometría de prueba
	if (m_pVisualMesh)
	{
		// Actualizar matriz del mundo del visual mesh
		m_pVisualMesh->SetWorldMatrix(WorldMatrix);

		// Actualizar frame del modelo (necesario para que se renderice correctamente)
		m_pVisualMesh->Frame();

		// Renderizar el modelo cargado
		m_pVisualMesh->Render();
	}

	if (m_pTestMesh)
	{
		// Si hay mesh pero no visual mesh, renderizar directamente
		RSetTransform(D3DTS_WORLD, WorldMatrix);
		m_pTestMesh->Render(&WorldMatrix);
	}
	else
	{
		// Si no hay mesh, dibujar una esfera simple para pruebas
		RSetTransform(D3DTS_WORLD, WorldMatrix);
		RDrawSphere(rvector(0, 0, 0), 50.0f, 16, 0xFFFFFFFF);
	}

	// RealSpace2 maneja EndScene automáticamente después de este callback
}

void MaterialTestApp::OnRestore()
{
	// Recrear recursos si es necesario
	if (m_pVisualMesh)
	{
		// El visual mesh debería restaurarse automáticamente
	}
}

void MaterialTestApp::OnInvalidate()
{
	// Invalidar recursos si es necesario
}

void MaterialTestApp::CreateTestMesh()
{
	// Cargar modelo .elu desde archivo
	const char* modelPath = "woman-parts_angel.elu";

	m_pTestMesh = std::make_unique<RMesh>();

	// Asegurar que las texturas se carguen automáticamente desde el XML
	m_pTestMesh->SetMtrlAutoLoad(true);

	if (m_pTestMesh->ReadElu(modelPath))
	{
		// Modificar materiales de todos los nodos para usar nuestro material de prueba
		// Esto asegura que la iluminación se aplique correctamente
		for (int i = 0; i < m_pTestMesh->m_data_num; i++)
		{
			RMeshNode* pNode = m_pTestMesh->m_data[i];
			if (pNode)
			{
				// Modificar todos los materiales de este nodo
				for (int j = 0; j < pNode->m_nMtrlCnt; j++)
				{
					RMtrl* pMtrl = pNode->GetMtrl(j);
					if (pMtrl)
					{
						// Aplicar nuestro material de prueba
						pMtrl->m_diffuse = m_TestMaterial.m_diffuse;
						pMtrl->m_ambient = m_TestMaterial.m_ambient;
						pMtrl->m_specular = m_TestMaterial.m_specular;
						pMtrl->m_power = m_TestMaterial.m_power;
					}
				}
			}
		}

		// Crear RVisualMesh desde el RMesh
		m_pVisualMesh = new RVisualMesh();
		if (m_pVisualMesh->Create(m_pTestMesh.get()))
		{
			// Configurar posición inicial del modelo en el origen
			m_pVisualMesh->SetPos(rvector(0, 0, 0), rvector(0, 0, 1), rvector(0, 1, 0));

			// Calcular bounding box
			m_pVisualMesh->CalcBox();

			// Establecer visibilidad completa
			m_pVisualMesh->SetVisibility(1.0f);

			// Desactivar verificación de view frustum para debug (temporalmente)
			m_pVisualMesh->SetCheckViewFrustum(false);

			MLog("MaterialTestApp: RVisualMesh creado - BBox: min(%.1f, %.1f, %.1f) max(%.1f, %.1f, %.1f)\n",
				m_pVisualMesh->m_vBMin.x, m_pVisualMesh->m_vBMin.y, m_pVisualMesh->m_vBMin.z,
				m_pVisualMesh->m_vBMax.x, m_pVisualMesh->m_vBMax.y, m_pVisualMesh->m_vBMax.z);
		}
		else
		{
			MLog("MaterialTestApp: Error al crear RVisualMesh\n");
			delete m_pVisualMesh;
			m_pVisualMesh = nullptr;
		}
	}
	else
	{
		MLog("MaterialTestApp: Error al cargar modelo: %s\n", modelPath);
		m_pTestMesh.reset();
		m_pVisualMesh = nullptr;
	}
}

void MaterialTestApp::SetupMaterial()
{
	// Configurar material de prueba con propiedades de iluminación
	ZeroMemory(&m_TestMaterial, sizeof(RMtrl));

	// Color difuso (blanco brillante para máxima visibilidad)
	m_TestMaterial.m_diffuse.r = 1.0f;
	m_TestMaterial.m_diffuse.g = 1.0f;
	m_TestMaterial.m_diffuse.b = 1.0f;
	m_TestMaterial.m_diffuse.a = 1.0f;

	// Color ambiente (muy alto para que sea visible incluso sin luces)
	m_TestMaterial.m_ambient.r = 0.8f;
	m_TestMaterial.m_ambient.g = 0.8f;
	m_TestMaterial.m_ambient.b = 0.8f;
	m_TestMaterial.m_ambient.a = 1.0f;

	// Color especular (blanco brillante)
	m_TestMaterial.m_specular.r = 1.0f;
	m_TestMaterial.m_specular.g = 1.0f;
	m_TestMaterial.m_specular.b = 1.0f;
	m_TestMaterial.m_specular.a = 1.0f;

	// Power (brillo especular) - valores típicos: 8-32
	m_TestMaterial.m_power = 16.0f;

	MLog("MaterialTestApp: Material configurado\n");
}

void MaterialTestApp::SetupLights()
{
	auto* pShaderMgr = RGetShaderMgr();
	if (!pShaderMgr)
		return;

	// Luz 0: Luz blanca muy brillante cerca del modelo
	ZeroMemory(&m_Light0, sizeof(D3DLIGHT9));
	m_Light0.Type = D3DLIGHT_POINT;
	m_Light0.Diffuse.r = 2.0f;  // Muy brillante
	m_Light0.Diffuse.g = 2.0f;
	m_Light0.Diffuse.b = 2.0f;
	m_Light0.Diffuse.a = 1.0f;
	m_Light0.Specular.r = 2.0f;
	m_Light0.Specular.g = 2.0f;
	m_Light0.Specular.b = 2.0f;
	m_Light0.Specular.a = 1.0f;
	m_Light0.Ambient.r = 0.5f;  // Ambiente alto
	m_Light0.Ambient.g = 0.5f;
	m_Light0.Ambient.b = 0.5f;
	m_Light0.Ambient.a = 1.0f;
	m_Light0.Position = rvector(0.0f, 150.0f, -200.0f);  // Cerca de la cámara
	m_Light0.Range = 2000.0f;  // Rango muy grande
	m_Light0.Attenuation0 = 1.0f;
	m_Light0.Attenuation1 = 0.001f;  // Muy poca atenuación
	m_Light0.Attenuation2 = 0.00001f;

	// Luz 1: Luz blanca adicional desde el lado
	ZeroMemory(&m_Light1, sizeof(D3DLIGHT9));
	m_Light1.Type = D3DLIGHT_POINT;
	m_Light1.Diffuse.r = 1.5f;  // Muy brillante
	m_Light1.Diffuse.g = 1.5f;
	m_Light1.Diffuse.b = 1.5f;
	m_Light1.Diffuse.a = 1.0f;
	m_Light1.Specular.r = 1.5f;
	m_Light1.Specular.g = 1.5f;
	m_Light1.Specular.b = 1.5f;
	m_Light1.Specular.a = 1.0f;
	m_Light1.Ambient.r = 0.3f;
	m_Light1.Ambient.g = 0.3f;
	m_Light1.Ambient.b = 0.3f;
	m_Light1.Ambient.a = 1.0f;
	m_Light1.Position = rvector(200.0f, 150.0f, -200.0f);  // Desde el lado
	m_Light1.Range = 2000.0f;  // Rango muy grande
	m_Light1.Attenuation0 = 1.0f;
	m_Light1.Attenuation1 = 0.001f;  // Muy poca atenuación
	m_Light1.Attenuation2 = 0.00001f;

	// Configurar luces en el shader manager y habilitarlas
	pShaderMgr->setLight(0, &m_Light0);
	pShaderMgr->setLight(1, &m_Light1);
	pShaderMgr->LightEnable(0, true);
	pShaderMgr->LightEnable(1, true);

	// Configurar ambiente global (muy alto para máxima visibilidad)
	pShaderMgr->setAmbient(0x808080);  // Ambiente muy brillante (gris medio)

	MLog("MaterialTestApp: Luces configuradas y habilitadas\n");
}

void MaterialTestApp::SetupCamera()
{
	// Configurar cámara - posicionar más cerca para ver mejor el modelo
	// La cámara está en (0, 100, -300) mirando hacia (0, 0, 0)
	RCameraPosition = rvector(0.0f, 100.0f, -300.0f);
	rvector lookAt = rvector(0.0f, 0.0f, 0.0f);
	RCameraDirection = lookAt - RCameraPosition;
	RCameraUp = rvector(0.0f, 1.0f, 0.0f);

	// Configurar proyección
	float fov = ToRadian(60.0f);
	float aspect = float(RGetScreenWidth()) / float(RGetScreenHeight());
	float nearZ = 1.0f;
	float farZ = 2000.0f;

	// Usar RSetProjection que actualiza RProjection automáticamente
	RSetProjection(fov, aspect, nearZ, farZ);

	// Configurar vista usando RSetCamera que actualiza automáticamente
	RSetCamera(RCameraPosition, lookAt, RCameraUp);

	MLog("MaterialTestApp: Cámara configurada - Pos: (%.1f, %.1f, %.1f), LookAt: (%.1f, %.1f, %.1f)\n",
		RCameraPosition.x, RCameraPosition.y, RCameraPosition.z,
		lookAt.x, lookAt.y, lookAt.z);
}