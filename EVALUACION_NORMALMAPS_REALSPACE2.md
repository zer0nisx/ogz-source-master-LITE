# Evaluación del Sistema de Normal Maps en RealSpace2 - Sistema Antiguo RMtrl

## Resumen Ejecutivo

**El sistema antiguo de materiales RMtrl NO tiene soporte para normal maps**. A diferencia del sistema moderno (EluMaterial) que tiene campos para normal maps, RMtrl solo soporta textura difusa (`m_pTexture`) y textura toon (`m_pToonTexture`). Para implementar normal maps en RMtrl se necesitarían modificaciones significativas en la estructura de la clase, el sistema de carga, y el pipeline de renderizado.

---

## ⚠️ ENFOQUE: Sistema Antiguo RMtrl

Esta evaluación se enfoca específicamente en el **sistema antiguo de materiales RMtrl**, que es el sistema principal usado para renderizado de personajes y objetos en el juego.

---

## 1. Análisis del Sistema de Materiales Antiguo (RMtrl)

### 1.1 Sistema de Materiales Moderno (EluMaterial)

**Archivo:** `src/RealSpace2/Include/EluLoader.h`

El sistema moderno `EluMaterial` **SÍ soporta normal maps**:

```cpp
struct EluMaterial
{
    v4 cDiffuse;
    v4 cAmbient;
    v4 cSpecular;
    v4 cEmissive;
    
    float shininess;
    float roughness;
    
    TextureType tDiffuse;
    TextureType tNormal;      // ✅ SOPORTADO
    TextureType tSpecular;
    TextureType tOpacity;
    TextureType tEmissive;
    
    int AlphaTestValue = -1;
    bool TwoSided{};
};
```

**Estado:** ✅ Los normal maps se pueden cargar y almacenar como texto (ruta del archivo).

### 1.2 Sistema de Materiales Antiguo (RMtrl) - **ENFOQUE PRINCIPAL**

**Archivo:** `src/RealSpace2/Include/RMtrl.h`

El sistema antiguo `RMtrl` **NO tiene soporte para normal maps**. Esta es la estructura actual:

```cpp
class RMtrl
{
public:
    // Texturas existentes
    RBaseTexture*  m_pTexture;      // Solo textura difusa
    RBaseTexture*  m_pToonTexture;  // Solo textura toon
    
    // Flags de materiales existentes
    bool	m_bDiffuseMap;
    bool	m_bTwoSided;
    bool	m_bAlphaMap;
    bool	m_bAlphaTestMap;
    bool	m_bAdditive;
    
    // Nombres de archivo existentes
    char	m_name[255];         // Nombre textura difusa
    char	m_opa_name[255];     // Nombre textura opacidad
    
    // ❌ FALTANTE: RBaseTexture* m_pNormalMap;
    // ❌ FALTANTE: bool m_bNormalMap;
    // ❌ FALTANTE: char m_normal_name[255];  // Nombre archivo normal map
    // ❌ FALTANTE: D3DTEXTUREFILTERTYPE m_NormalFilterType;
};
```

**Problema Principal:** El sistema RMtrl es el que se usa para renderizar personajes y objetos principales del juego, pero **no tiene ninguna infraestructura para normal maps**.

**Estado:** ❌ **COMPLETAMENTE FALTANTE** - No hay campos, no hay carga, no hay renderizado.

---

## 2. Carga de Normal Maps en Sistema RMtrl

### 2.1 ❌ RMtrl NO Carga Normal Maps desde XML

**Archivo:** `src/RealSpace2/Source/RMtrl.cpp`

El sistema RMtrl **NO tiene código para cargar normal maps**. La función `Restore()` solo carga:

```cpp
void RMtrl::Restore(LPDIRECT3DDEVICE9 dev, char* path)
{
    // Solo carga m_pTexture (textura difusa)
    // NO hay código para cargar normal maps
    m_pTexture = RCreateBaseTextureMg(name, level);
}
```

**Estado:** ❌ **NO EXISTE** - RMtrl no tiene código para cargar normal maps.

### 2.2 ❌ RMtrlMgr NO Procesa Normal Maps

**Archivo:** `src/RealSpace2/Source/RMtrl.cpp`

El manager de materiales `RMtrlMgr` no procesa normal maps:

```cpp
int RMtrlMgr::LoadList(char* fname)
{
    return 1;  // ❌ Función stub, no carga nada
}
```

**Estado:** ❌ **NO IMPLEMENTADO** - No hay sistema de carga para RMtrl.

### 2.3 ⚠️ XMLParser Carga Normal Maps pero para Sistema Nuevo

**Archivo:** `src/RealSpace2/Source/XMLParser.cpp`

El parser XML SÍ tiene código para cargar normal maps, pero **solo para el sistema nuevo (EluMaterial)**, no para RMtrl:

```cpp
// Esto carga para XMLMaterial/EluMaterial, NO para RMtrl
GetTexture(MAT_NORMALMAP_TAG, tmp.NormalMap, FLAG_NORMAL);
```

**Estado:** ⚠️ Existe pero **NO se integra con RMtrl** - Solo para sistema moderno.

---

## 3. Sistema de Renderizado

### 3.1 Funciones de Shader (Sistema Antiguo)

**Archivo:** `src/RealSpace2/Source/RMesh_Mtrl.cpp`

Existen funciones para configurar normal maps, pero están **prácticamente vacías**:

```cpp
void RMesh::SetShaderNormalMap()
{
    LPDIRECT3DDEVICE9 dev = RGetDevice();
    
    dev->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE);
    
    if(m_isNPCMesh || m_isCharacterMesh && !GetToolMesh())
        dev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE2X );
    else
        dev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
    
    dev->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
}

void RMesh::SetShaderNormalMap_OFF()
{
    // ❌ VACÍA - No hace nada
}
```

**Problemas identificados:**
- ❌ No configura un sampler para la textura de normal map
- ❌ No configura el texture stage 1 para normal mapping
- ❌ No hay código que realmente procese el normal map
- ❌ La función OFF está completamente vacía

**Estado:** ❌ **NO FUNCIONAL** - Las funciones existen pero no implementan normal mapping real.

### 3.2 Vertex Declarations

**Archivo:** `src/RealSpace2/Source/RBspObjectDrawD3D9.cpp`

El sistema moderno **SÍ tiene tangentes** en el vertex declaration:

```cpp
static D3DPtr<IDirect3DVertexDeclaration9> CreateLitVertexDecl()
{
    D3DVERTEXELEMENT9 Decl[] = {
        { 0, 0,     D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
        { 1, 0,     D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
        { 2, 0,     D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0 },
        { 3, 0,     D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT,  0 },  // ✅ TANGENT SOPORTADO
        D3DDECL_END()
    };
    return CreateVertexDeclaration(Decl);
}
```

**Estado:** ✅ Los tangentes están disponibles en los vértices (necesarios para normal mapping).

---

## 4. Análisis de Shaders

### 4.1 Vertex Shader (SceneVS.hlsl)

**Archivo:** `src/RealSpace2/Source/SceneVS.hlsl`

```hlsl
void main(
    float4 inPos               : POSITION,
    float3 inNormal            : NORMAL,
    float2 inTex               : TEXCOORD0,
    float2 inLightmapTex       : TEXCOORD1,
    out float3 outNormal       : TEXCOORD3, // World space normal
    ...
)
{
    outNormal = mul(inNormal, (float3x3)World);
    ...
}
```

**Problemas identificados:**
- ❌ No acepta tangentes como input
- ❌ No calcula espacio tangente (TBN matrix)
- ❌ Solo transforma normales geométricas, no normales de textura

**Estado:** ❌ **NO SOPORTA** normal maps - Solo procesa normales geométricas.

### 4.2 Pixel Shader (SceneDirLightPS.hlsl)

**Archivo:** `src/RealSpace2/Source/SceneDirLightPS.hlsl`

```hlsl
float4 main(
    float2 Tex         : TEXCOORD0,
    float2 LightmapTex : TEXCOORD1,
    float4 Pos         : TEXCOORD2,
    float3 Normal      : TEXCOORD3,  // Solo normal geométrica
    float4 PosLight    : TEXCOORD4) : COLOR
{
    // Usa Normal directamente, no samplea normal map
    return SceneColor * Diffuse;
}
```

**Problemas identificados:**
- ❌ No tiene sampler para normal map
- ❌ No samplea la textura de normal map
- ❌ No convierte del espacio tangente al espacio del mundo
- ❌ Solo usa la normal geométrica

**Estado:** ❌ **NO SOPORTA** normal maps.

### 4.3 Shaders con Referencias a Normal Maps

**Archivo:** `src/RealSpace2/Source/Lighting.hlsl`

```hlsl
sampler2D normalTexture  : register(s1);  // ✅ DECLARADO
```

**Archivo:** `src/RealSpace2/Source/Deferred.hlsl`

```hlsl
sampler2D normalTexture    : register(s1);  // ✅ DECLARADO
```

**Estado:** ⚠️ **PARCIALMENTE DECLARADO** - Los samplers están declarados pero no se usa ampliamente.

---

## 5. Problemas Críticos Identificados

### 5.1 Problemas de Arquitectura

1. **Sistema Dual de Materiales:**
   - El sistema moderno (EluMaterial) tiene normal maps
   - El sistema antiguo (RMtrl) no los tiene
   - No hay integración entre ambos sistemas

2. **Falta de Implementación en Shaders:**
   - Los shaders principales (SceneVS, SceneDirLightPS) no procesan normal maps
   - Falta el cálculo de matriz TBN (Tangent-Bitangent-Normal)
   - Falta el sampleo y transformación de normales de textura

3. **Funciones Vacías:**
   - `SetShaderNormalMap()` existe pero no hace nada útil
   - `SetShaderNormalMap_OFF()` está completamente vacía

### 5.2 Problemas de Implementación

1. **Falta Binding de Texturas:**
   - No se vincula el normal map al texture stage 1
   - No se configura el sampler para normal maps

2. **Falta Cálculo de Espacio Tangente:**
   - Aunque hay tangentes en los vértices, no se usan para crear la matriz TBN
   - No se transforma la normal del mapa al espacio del mundo

3. **Falta en Pipeline de Renderizado:**
   - Los normal maps no se procesan en el renderizado real
   - Solo se cargan como datos pero no se usan

---

## 6. Estado Actual por Componente

| Componente | Estado | Detalles |
|------------|--------|----------|
| **Carga desde XML** | ✅ Funcional | Los normal maps se cargan correctamente |
| **Carga desde ELU** | ✅ Funcional | Los normal maps se almacenan como rutas |
| **Conversión a D3D** | ✅ Funcional | Se convierten a texturas DirectX |
| **Almacenamiento (EluMaterial)** | ✅ Funcional | El sistema moderno tiene campo tNormal |
| **Almacenamiento (RMtrl)** | ❌ No soportado | El sistema antiguo no tiene campos |
| **Vertex Declaration** | ✅ Funcional | Los tangentes están disponibles |
| **Vertex Shader** | ❌ No implementado | No procesa normal maps |
| **Pixel Shader** | ❌ No implementado | No samplea normal maps |
| **Funciones RMesh** | ❌ No funcional | Funciones vacías o incompletas |
| **Renderizado Real** | ❌ No funcional | No se usan en el pipeline |

---

## 7. Requisitos para Implementación Completa

### 7.1 Cambios Necesarios en Shaders

1. **Vertex Shader:**
   - Aceptar tangentes como input
   - Calcular matriz TBN (Tangent-Bitangent-Normal)
   - Pasar matriz TBN o componentes al pixel shader

2. **Pixel Shader:**
   - Añadir sampler para normal map
   - Samplear normal map
   - Transformar normal del espacio tangente al espacio del mundo
   - Usar normal transformada para iluminación

### 7.2 Cambios Necesarios en Código C++

1. **RMtrl (Sistema Antiguo):**
   - Añadir campo `RBaseTexture* m_pNormalMap`
   - Añadir flag `bool m_bNormalMap`
   - Implementar carga de normal maps

2. **Funciones de Renderizado:**
   - Implementar `SetShaderNormalMap()` completamente
   - Vincular normal map al texture stage 1
   - Configurar samplers apropiados

3. **Pipeline de Renderizado:**
   - Verificar si material tiene normal map
   - Activar shaders apropiados cuando hay normal map
   - Pasar parámetros necesarios a shaders

### 7.3 Ejemplo de Implementación Requerida

**Vertex Shader (ejemplo):**
```hlsl
struct VS_INPUT
{
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float2 TexCoord : TEXCOORD0;
    float4 Tangent  : TANGENT;  // ✅ NECESARIO
};

struct VS_OUTPUT
{
    float4 Pos       : POSITION;
    float2 TexCoord  : TEXCOORD0;
    float3x3 TBN     : TEXCOORD1;  // ✅ MATRIZ TBN
};
```

**Pixel Shader (ejemplo):**
```hlsl
sampler2D DiffuseMap  : register(s0);
sampler2D NormalMap   : register(s1);  // ✅ NECESARIO

float4 main(VS_OUTPUT input) : COLOR
{
    // Samplear normal map
    float3 normalTS = tex2D(NormalMap, input.TexCoord).xyz * 2.0 - 1.0;
    
    // Transformar a espacio del mundo
    float3 normalWS = mul(normalTS, input.TBN);
    normalize(normalWS);
    
    // Usar normal para iluminación
    // ...
}
```

---

## 8. Lo Que Se Necesita para Implementar Normal Maps en RMtrl

### 8.1 Cambios Necesarios en RMtrl.h

**Archivo:** `src/RealSpace2/Include/RMtrl.h`

Añadir los siguientes campos a la clase `RMtrl`:

```cpp
class RMtrl
{
public:
    // ... campos existentes ...
    
    // ✅ AÑADIR: Campos para normal map
    RBaseTexture*  m_pNormalMap;              // Textura de normal map
    bool           m_bNormalMap;               // Flag si tiene normal map
    char           m_normal_name[255];         // Nombre archivo normal map
    D3DTEXTUREFILTERTYPE m_NormalFilterType;   // Filtro para normal map
    
    // ✅ AÑADIR: Método para obtener normal map
    LPDIRECT3DTEXTURE9 GetNormalMap();
};
```

### 8.2 Cambios Necesarios en RMtrl.cpp

**Archivo:** `src/RealSpace2/Source/RMtrl.cpp`

#### 8.2.1 Constructor

```cpp
RMtrl::RMtrl()
{
    // ... código existente ...
    
    // ✅ AÑADIR: Inicialización de normal map
    m_pNormalMap = NULL;
    m_bNormalMap = false;
    m_normal_name[0] = 0;
    m_NormalFilterType = D3DTEXF_LINEAR;
}
```

#### 8.2.2 Destructor

```cpp
RMtrl::~RMtrl()
{
    // ... código existente para m_pTexture ...
    
    // ✅ AÑADIR: Liberar normal map
    RDestroyBaseTexture(m_pNormalMap);
    m_pNormalMap = NULL;
}
```

#### 8.2.3 Función Restore()

```cpp
void RMtrl::Restore(LPDIRECT3DDEVICE9 dev, char* path)
{
    // ... código existente para m_pTexture ...
    
    // ✅ AÑADIR: Cargar normal map
    if(m_bNormalMap && m_normal_name[0] != 0)
    {
        char normal_path[256];
        if(path && path[0])
        {
            strcpy_safe(normal_path, path);
            strcat_safe(normal_path, m_normal_name);
            m_pNormalMap = RCreateBaseTextureMg(normal_path, level);
            if(!m_pNormalMap)
                m_pNormalMap = RCreateBaseTextureMg(m_normal_name, level);
        }
        else
        {
            m_pNormalMap = RCreateBaseTextureMg(m_normal_name, level);
        }
    }
}
```

#### 8.2.4 Función GetNormalMap()

```cpp
LPDIRECT3DTEXTURE9 RMtrl::GetNormalMap()
{
    if(!m_pNormalMap || !m_bNormalMap)
        return NULL;
    return m_pNormalMap->GetTexture();
}
```

### 8.3 Cambios Necesarios en Sistema de Carga

**Necesario crear/modificar:**

1. **Cargar desde XML:** Añadir parsing del tag `NORMALMAP` para RMtrl
2. **Cargar desde ELU:** Integrar carga de normal maps en el sistema de carga de materiales
3. **Nombre automático:** Generar nombre de normal map basado en nombre de textura difusa (ej: `texture_d.tga` → `texture_n.tga`)

### 8.4 Cambios Necesarios en Renderizado

**Archivo:** `src/RealSpace2/Source/RMesh_Mtrl.cpp`

#### 8.4.1 Implementar SetShaderNormalMap() completamente

```cpp
void RMesh::SetShaderNormalMap(RMtrl* pMtrl)
{
    LPDIRECT3DDEVICE9 dev = RGetDevice();
    
    if(!pMtrl || !pMtrl->m_bNormalMap)
        return;
    
    // Configurar textura difusa en stage 0
    dev->SetTexture(0, pMtrl->GetTexture());
    
    // ✅ AÑADIR: Configurar normal map en stage 1
    dev->SetTexture(1, pMtrl->GetNormalMap());
    
    // Configurar sampler para normal map
    dev->SetSamplerState(1, D3DSAMP_MAGFILTER, pMtrl->m_NormalFilterType);
    dev->SetSamplerState(1, D3DSAMP_MINFILTER, pMtrl->m_NormalFilterType);
    dev->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
    
    // ✅ AÑADIR: Configurar texture stage 1 para normal map
    dev->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 0); // Usar misma UV que difusa
    dev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    dev->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
}
```

#### 8.4.2 Implementar SetShaderNormalMap_OFF()

```cpp
void RMesh::SetShaderNormalMap_OFF()
{
    LPDIRECT3DDEVICE9 dev = RGetDevice();
    
    // ✅ AÑADIR: Deshabilitar texture stage 1
    dev->SetTexture(1, NULL);
    dev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
}
```

### 8.5 Cambios Necesarios en Shaders

Los shaders necesitarían:
- Vertex shader: Calcular matriz TBN y pasarla al pixel shader
- Pixel shader: Samplear normal map y transformarlo al espacio del mundo

**Estado actual:** ❌ **NO IMPLEMENTADO** - Los shaders actuales no procesan normal maps.

---

## 9. Conclusión - Sistema RMtrl

### 9.1 Estado Actual del Sistema RMtrl

**❌ COMPLETAMENTE FALTANTE:**
- ❌ No hay campos en la estructura RMtrl
- ❌ No hay sistema de carga
- ❌ No hay renderizado funcional
- ❌ Las funciones helper están vacías o incompletas
- ❌ Los shaders no procesan normal maps

**⚠️ Infraestructura Existente (pero no integrada):**
- ⚠️ Los tangentes están en algunos vertex declarations
- ⚠️ El parser XML puede leer normal maps (pero no para RMtrl)

### 9.2 Esfuerzo Requerido

**Alto-Medio:**
- **Alto:** Modificar estructura RMtrl y todo el sistema de carga
- **Medio:** Implementar funciones de renderizado
- **Alto:** Crear/actualizar shaders para normal mapping
- **Bajo-Medio:** Testing y debugging

**Tiempo Estimado:**
- Estructura y carga: 2-3 días
- Renderizado y funciones: 2-3 días  
- Shaders: 3-5 días
- Testing: 2-3 días
- **Total: 9-14 días de trabajo**

### 9.3 Recomendaciones

1. **Para Uso Inmediato:**
   - Los normal maps **NO funcionan** en el sistema RMtrl
   - No hay manera de usarlos actualmente

2. **Para Implementación:**
   - Requiere cambios significativos en RMtrl
   - Requiere nuevos shaders o modificar existentes
   - Requiere integrar sistema de carga desde XML/archivos

3. **Alternativa:**
   - Considerar migrar a usar el sistema moderno (EluMaterial) que ya tiene infraestructura
   - O implementar normal maps solo en el sistema moderno

### 8.3 Prioridad

**Prioridad Media-Baja:**
- Los normal maps son una característica de calidad visual, no crítica para funcionalidad
- Requiere trabajo significativo en shaders
- Mejoraría la calidad visual pero no es bloqueante

---

## 10. Archivos Clave para Implementación en RMtrl

### 10.1 Archivos Principales a Modificar

**Estructura de Datos:**
- `src/RealSpace2/Include/RMtrl.h` - ⚠️ **AÑADIR** campos para normal map
- `src/RealSpace2/Source/RMtrl.cpp` - ⚠️ **AÑADIR** carga y gestión de normal maps

**Sistema de Carga:**
- `src/RealSpace2/Source/XMLParser.cpp` - ⚠️ **MODIFICAR** para cargar normal maps en RMtrl
- `src/RealSpace2/Source/RMeshMgr.cpp` - ⚠️ **REVISAR** integración con carga de materiales

**Renderizado:**
- `src/RealSpace2/Source/RMesh_Mtrl.cpp` - ⚠️ **COMPLETAR** funciones SetShaderNormalMap()
- `src/RealSpace2/Source/RMesh.cpp` - ⚠️ **REVISAR** donde se llama SetShaderNormalMap()

**Shaders (si se implementan):**
- `src/RealSpace2/Source/skin.hlsl` - ⚠️ **MODIFICAR** para soportar normal maps
- `src/RealSpace2/Source/SceneVS.hlsl` - ⚠️ **MODIFICAR** vertex shader
- `src/RealSpace2/Source/SceneDirLightPS.hlsl` - ⚠️ **MODIFICAR** pixel shader

### 10.2 Archivos de Referencia (No modificar directamente)

- `src/RealSpace2/Include/EluLoader.h` - Referencia: sistema moderno con normal maps
- `src/RealSpace2/Source/EluLoader.cpp` - Referencia: cómo carga normal maps el sistema nuevo
- `src/RealSpace2/Include/XMLFileStructs.h` - Referencia: tags XML para normal maps

---

## Resumen Final - Sistema RMtrl

### Estado Actual
**❌ NO HAY SOPORTE** para normal maps en el sistema RMtrl. Todo necesita ser implementado desde cero.

### Lo Que Existe
- ⚠️ Funciones stub vacías (`SetShaderNormalMap()`)
- ⚠️ Tags XML para normal maps (pero no se usan para RMtrl)
- ⚠️ Tangentes en algunos vertex declarations

### Lo Que Falta
- ❌ Campos en estructura RMtrl
- ❌ Sistema de carga
- ❌ Renderizado funcional
- ❌ Shaders completos
- ❌ Integración completa

**Fecha de Evaluación:** Diciembre 2024  
**Versión Analizada:** Código fuente actual de RealSpace2  
**Sistema Evaluado:** Sistema Antiguo RMtrl  
**Evaluado por:** Análisis automatizado del código base

