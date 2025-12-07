# Oportunidades Adicionales para D3DPtr

## Resumen

Después de convertir `ZWater::m_pIndexBuffer` a `D3DPtr`, hay **varios lugares más** donde se pueden aplicar `D3DPtr` para gestión automática de memoria COM.

---

## 1. Variables Globales Estáticas en ZWater.cpp

### ✅ **g_pTexReflection** (LPDIRECT3DTEXTURE9)
- **Tipo**: Variable global estática
- **Creación**: `CreateTexture()` en `SetSurface()`
- **Gestión actual**: `SAFE_RELEASE()` en `SetSurface(false)`
- **Viabilidad**: ✅ **ALTA** - Ownership claro, se crea y destruye explícitamente
- **Desafío**: Es una variable global estática, requiere manejo especial

### ✅ **g_pSufRefDepthBuffer** (LPDIRECT3DSURFACE9)
- **Tipo**: Variable global estática
- **Creación**: `CreateDepthStencilSurface()` en `SetSurface()`
- **Gestión actual**: `SAFE_RELEASE()` en `SetSurface(false)`
- **Viabilidad**: ✅ **ALTA** - Ownership claro
- **Desafío**: Variable global estática

### ✅ **g_pVBForWaterMesh** (LPDIRECT3DVERTEXBUFFER9)
- **Tipo**: Variable global estática
- **Creación**: `CreateVertexBuffer()` en `OnRestore()`
- **Gestión actual**: `SAFE_RELEASE()` en destructor y `OnInvalidate()`
- **Viabilidad**: ✅ **ALTA** - Ownership claro
- **Desafío**: Variable global estática

### ⚠️ **g_pSufReflection** (LPDIRECT3DSURFACE9)
- **Tipo**: Variable global estática
- **Creación**: `GetSurfaceLevel()` desde `g_pTexReflection`
- **Gestión actual**: `SAFE_RELEASE()` en `SetSurface(false)`
- **Viabilidad**: 🟡 **MEDIA** - Es una referencia desde `g_pTexReflection`, no ownership completo
- **Nota**: `GetSurfaceLevel()` incrementa el ref count, así que sí necesita Release()

### ❌ **g_pSufBackBuffer** y **g_pSufDepthBuffer**
- **Tipo**: Variables temporales en `RenderReflectionSurface()`
- **Obtención**: `GetRenderTarget()` y `GetDepthStencilSurface()`
- **Viabilidad**: ❌ **NO** - Son referencias temporales, NO ownership. Solo se obtienen para restaurar después.

---

## 2. RFontTexture::m_pTexture

### ✅ **RFontTexture::m_pTexture** (LPDIRECT3DTEXTURE9)
- **Tipo**: Miembro de clase
- **Creación**: `CreateTexture()` en `RFontTexture::Create()`
- **Gestión actual**: `SAFE_RELEASE()` en `Destroy()`
- **Viabilidad**: ✅ **ALTA** - Ownership claro, miembro de clase
- **Archivo**: `RealSpace2/Source/RFont.cpp` y `RealSpace2/Include/RFont.h`

---

## 3. Análisis Detallado

### 3.1 Variables Globales Estáticas - Desafío

Las variables globales estáticas requieren un enfoque especial:

```cpp
// ACTUAL (en ZWater.cpp)
static LPDIRECT3DTEXTURE9 g_pTexReflection = 0;
static LPDIRECT3DSURFACE9 g_pSufRefDepthBuffer = 0;
static LPDIRECT3DVERTEXBUFFER9 g_pVBForWaterMesh = 0;
```

**Opción A: Mantener como estáticas pero usar D3DPtr**
```cpp
static D3DPtr<IDirect3DTexture9> g_pTexReflection;
static D3DPtr<IDirect3DDepthStencilSurface9> g_pSufRefDepthBuffer;
static D3DPtr<IDirect3DVertexBuffer9> g_pVBForWaterMesh;
```

**Opción B: Crear una clase singleton/manager**
```cpp
class ZWaterResourceManager {
    D3DPtr<IDirect3DTexture9> m_pTexReflection;
    D3DPtr<IDirect3DDepthStencilSurface9> m_pSufRefDepthBuffer;
    // ...
};
```

**Recomendación**: Opción A es más simple y mantiene la estructura actual.

---

## 4. Plan de Implementación por Prioridad

### Fase 1: RFontTexture (Más Simple) ⭐⭐⭐
- ✅ Cambio local a una clase
- ✅ No afecta otras partes del código
- ✅ Ownership claro

### Fase 2: Variables Globales de ZWater ⭐⭐
- 🟡 Requiere cambiar variables globales estáticas
- 🟡 Afecta múltiples funciones
- ✅ Ownership claro

---

## 5. Implementación Detallada

### 5.1 RFontTexture::m_pTexture

#### Cambios en `RFont.h`
```cpp
#include "MUtil.h"  // Para D3DPtr

class RFontTexture {
private:
    D3DPtr<IDirect3DTexture9> m_pTexture;  // ✅ unique_ptr
    // ... resto igual ...
};
```

#### Cambios en `RFont.cpp`
```cpp
bool RFontTexture::Create() {
    // ...
    LPDIRECT3DTEXTURE9 pTmp = nullptr;
    HRESULT hr = RGetDevice()->CreateTexture(..., &pTmp, NULL);
    if (hr != D3D_OK) {
        return false;
    }
    m_pTexture.reset(pTmp);  // ✅ Transferir ownership
    // ...
}

void RFontTexture::Destroy() {
    m_pTexture.reset();  // ✅ Automático
    // ...
}

LPDIRECT3DTEXTURE9 RFontTexture::GetTexture() {
    return m_pTexture.get();  // ✅ Retornar puntero raw
}
```

### 5.2 Variables Globales en ZWater.cpp

#### Cambios en `ZWater.cpp`
```cpp
// Variables globales estáticas
static D3DPtr<IDirect3DTexture9> g_pTexReflection;
static D3DPtr<IDirect3DDepthStencilSurface9> g_pSufRefDepthBuffer;
static D3DPtr<IDirect3DVertexBuffer9> g_pVBForWaterMesh;
static LPDIRECT3DSURFACE9 g_pSufReflection = nullptr;  // Referencia, no ownership
static LPDIRECT3DSURFACE9 g_pSufBackBuffer = nullptr;  // Temporal
static LPDIRECT3DSURFACE9 g_pSufDepthBuffer = nullptr; // Temporal
```

#### `SetSurface()` - Crear Textura
```cpp
if (g_pTexReflection == nullptr) {
    LPDIRECT3DTEXTURE9 pTmp = nullptr;
    if (FAILED(RGetDevice()->CreateTexture(..., &pTmp, nullptr))) {
        // fallback...
    }
    g_pTexReflection.reset(pTmp);
}

// Obtener surface (referencia, no ownership)
LPDIRECT3DSURFACE9 pTmpSurf = nullptr;
if (FAILED(g_pTexReflection->GetSurfaceLevel(0, &pTmpSurf))) {
    return false;
}
g_pSufReflection = pTmpSurf;  // Referencia, necesita Release() manual
```

#### `SetSurface()` - Crear Depth Buffer
```cpp
if (g_pSufRefDepthBuffer == nullptr) {
    LPDIRECT3DDepthStencilSurface9 pTmp = nullptr;
    if (FAILED(g_pDevice->CreateDepthStencilSurface(..., &pTmp, NULL))) {
        return false;
    }
    g_pSufRefDepthBuffer.reset(pTmp);
}
```

#### `OnRestore()` - Crear Vertex Buffer
```cpp
if (g_pVBForWaterMesh == nullptr) {
    LPDIRECT3DVertexBuffer9 pTmp = nullptr;
    if (FAILED(g_pDevice->CreateVertexBuffer(..., &pTmp, NULL))) {
        return false;
    }
    g_pVBForWaterMesh.reset(pTmp);
}
```

#### `SetSurface(false)` - Limpiar
```cpp
g_pTexReflection.reset();        // ✅ Automático
g_pSufRefDepthBuffer.reset();    // ✅ Automático
// g_pSufReflection necesita Release() manual porque es referencia
if (g_pSufReflection) {
    g_pSufReflection->Release();
    g_pSufReflection = nullptr;
}
```

---

## 6. Consideraciones Especiales

### 6.1 g_pSufReflection (Surface desde Texture)

**Problema**: `GetSurfaceLevel()` retorna una referencia con ref count incrementado, pero el ownership real es de la textura padre.

**Opciones**:
1. **Mantener como puntero raw** - Es más una referencia que ownership
2. **Usar D3DPtr** - Funciona, pero conceptualmente es raro porque el surface pertenece a la texture

**Recomendación**: Mantener como puntero raw con Release() manual, ya que es una referencia.

### 6.2 Variables Temporales (BackBuffer, DepthBuffer)

Estas son **referencias temporales** para restaurar el estado después de renderizar el reflejo. NO son ownership, así que NO deben usar D3DPtr.

---

## 7. Resumen de Oportunidades

| Recurso | Tipo | Ownership | Viabilidad | Prioridad |
|---------|------|-----------|------------|-----------|
| `RFontTexture::m_pTexture` | Miembro clase | ✅ Completo | ✅ ALTA | ⭐⭐⭐ Alta |
| `g_pTexReflection` | Global estática | ✅ Completo | ✅ ALTA | ⭐⭐ Media |
| `g_pSufRefDepthBuffer` | Global estática | ✅ Completo | ✅ ALTA | ⭐⭐ Media |
| `g_pVBForWaterMesh` | Global estática | ✅ Completo | ✅ ALTA | ⭐⭐ Media |
| `g_pSufReflection` | Global estática | 🟡 Referencia | 🟡 MEDIA | ⭐ Baja |
| `g_pSufBackBuffer` | Temporal | ❌ No ownership | ❌ NO | - |
| `g_pSufDepthBuffer` | Temporal | ❌ No ownership | ❌ NO | - |

---

## 8. Recomendación Final

### ✅ **Empezar con RFontTexture** (Más simple y claro)
1. Cambio localizado
2. Ownership claro
3. Bajo riesgo

### ⭐ **Luego variables globales de ZWater** (Más complejo)
1. Requiere cambiar variables globales
2. Afecta múltiples funciones
3. Requiere cuidado con g_pSufReflection (referencia)

---

## 9. Código de Ejemplo

Ver implementación completa en los cambios propuestos arriba.

