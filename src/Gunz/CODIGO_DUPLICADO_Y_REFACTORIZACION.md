# Código Duplicado y Oportunidades de Refactorización

## 🔍 Análisis de Duplicación

### 1. **Configuración de Iluminación Ambiente** - ⚠️ DUPLICADO

**Ubicaciones**:
- `ZCharacterObject.cpp:480-482` - `Draw_SetLight()`
- `ZCharacterSelectView.cpp:194-196`
- `ZClothEmblem.cpp:436-437`
- `ZMeshView.cpp:131-133`

**Código Duplicado**:
```cpp
u32 AmbientColor = 0xCCCCCC;  // o 0x00cccccc
RGetDevice()->SetRenderState(D3DRS_AMBIENT, AmbientColor);
RGetShaderMgr()->setAmbient(AmbientColor);
```

**Solución**: Crear función helper estática
```cpp
// En ZCharacterObject.cpp o nuevo archivo LightingHelpers.h
static void SetAmbientLight(u32 color = 0xCCCCCC)
{
    RGetDevice()->SetRenderState(D3DRS_AMBIENT, color);
    RGetShaderMgr()->setAmbient(color);
}
```

---

### 2. **Establecer Luces a nullptr** - ⚠️ DUPLICADO

**Ubicaciones**:
- `ZCharacterObject.cpp:487-489` - `Draw_SetLight()` (early exit)
- Múltiples lugares donde se desactivan luces

**Código Duplicado**:
```cpp
m_pVMesh->SetLight(0, nullptr, false);
m_pVMesh->SetLight(1, nullptr, false);
m_pVMesh->SetLight(2, nullptr, false);
RGetDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);
```

**Solución**: Crear función helper
```cpp
static void DisableAllLights(RVisualMesh* pVMesh)
{
    if (!pVMesh) return;
    pVMesh->SetLight(0, nullptr, false);
    pVMesh->SetLight(1, nullptr, false);
    pVMesh->SetLight(2, nullptr, false);
    RGetDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);
}
```

---

### 3. **ZActor::OnDraw() - No Usa Draw_SetLight()** - ⚠️ OPORTUNIDAD

**Problema Actual**:
```cpp
void ZActor::OnDraw()
{
    if (m_pVMesh == NULL) return;
    // ❌ NO llama Draw_SetLight() - NPCs no tienen iluminación
    // ... código de opacidad ...
    m_pVMesh->Render();
}
```

**Análisis**:
- `ZActor` hereda de `ZCharacterObjectHistory` → `ZCharacterObject`
- `ZCharacterObject::Draw_SetLight()` ya existe y está optimizado
- **PERO**: `Draw_SetLight()` llama `SetGunLight()` que es innecesario para NPCs

**Solución**: Crear versión optimizada para NPCs
```cpp
// En ZCharacterObject.h
protected:
    void Draw_SetLight_ForNPC(const rvector& vPosition);  // Versión sin SetGunLight()

// En ZCharacterObject.cpp
void ZCharacterObject::Draw_SetLight_ForNPC(const rvector& vPosition)
{
    SetAmbientLight();  // Helper reutilizable
    
    if (!ZGetConfiguration()->GetVideo()->bDynamicLight)
    {
        DisableAllLights(m_pVMesh);  // Helper reutilizable
        return;
    }
    
    // Solo buscar 1 luz del mapa (sin SetGunLight)
    rvector char_pos = vPosition;
    char_pos.z += 180.f;
    SetMapLight(char_pos, m_pVMesh, 1, nullptr);
    
    m_pVMesh->SetLight(0, nullptr, false);  // No hay arma
    m_pVMesh->SetLight(2, nullptr, false);
    RGetDevice()->SetRenderState(D3DRS_LIGHTING, TRUE);
}

// En ZActor.cpp
void ZActor::OnDraw()
{
    if (m_pVMesh == NULL) return;
    
    Draw_SetLight_ForNPC(m_Position);  // ✅ Reutiliza código optimizado
    
    // ... código de opacidad ...
    m_pVMesh->Render();
}
```

---

### 4. **SetMapLight() - Función Estática** - ✅ BIEN DISEÑADA

**Estado Actual**:
- `SetMapLight()` es una función `static` en `ZCharacterObject.cpp`
- Solo se usa dentro de `Draw_SetLight()`
- **No está duplicada**, pero podría ser más reutilizable

**Oportunidad**: Si necesitamos usarla desde otros lugares, moverla a namespace o clase helper.

---

### 5. **Inicialización de D3DLIGHT9** - ⚠️ DUPLICADO

**Ubicaciones**:
- `ZCharacterObject.cpp:312-324` - `SetGunLight()`
- `ZCharacterObject.cpp:384-393` - `SetMapLight()`
- Potencialmente en otros lugares

**Código Duplicado**:
```cpp
D3DLIGHT9 Light{};
Light.Type = D3DLIGHT_POINT;
Light.Specular.r = 1.f;
Light.Specular.g = 1.f;
Light.Specular.b = 1.f;
Light.Specular.a = 1.f;
```

**Solución**: Crear función helper
```cpp
static void InitD3DLight(D3DLIGHT9& Light, D3DLIGHTTYPE Type = D3DLIGHT_POINT)
{
    ZeroMemory(&Light, sizeof(D3DLIGHT9));
    Light.Type = Type;
    Light.Specular.r = 1.0f;
    Light.Specular.g = 1.0f;
    Light.Specular.b = 1.0f;
    Light.Specular.a = 1.0f;
}
```

---

## 📋 Resumen de Refactorizaciones Propuestas

| # | Duplicación | Ubicaciones | Solución | Prioridad |
|---|------------|-------------|----------|-----------|
| 1 | Configuración ambiente | 4+ archivos | `SetAmbientLight()` helper | ⭐⭐⭐ |
| 2 | Desactivar luces | 3+ lugares | `DisableAllLights()` helper | ⭐⭐⭐ |
| 3 | `ZActor::OnDraw()` sin iluminación | `ZActor.cpp` | `Draw_SetLight_ForNPC()` | ⭐⭐⭐⭐⭐ |
| 4 | Inicialización `D3DLIGHT9` | 2+ lugares | `InitD3DLight()` helper | ⭐⭐ |
| 5 | `SetMapLight()` estática | `ZCharacterObject.cpp` | Ya está bien, mantener | ✅ |

---

## 🎯 Implementación Recomendada

### Paso 1: Crear Helpers Reutilizables

```cpp
// En ZCharacterObject.cpp (al inicio, después de includes)

namespace LightingHelpers
{
    // Helper 1: Configurar iluminación ambiente
    void SetAmbientLight(u32 color = 0xCCCCCC)
    {
        RGetDevice()->SetRenderState(D3DRS_AMBIENT, color);
        RGetShaderMgr()->setAmbient(color);
    }
    
    // Helper 2: Desactivar todas las luces
    void DisableAllLights(RVisualMesh* pVMesh)
    {
        if (!pVMesh) return;
        pVMesh->SetLight(0, nullptr, false);
        pVMesh->SetLight(1, nullptr, false);
        pVMesh->SetLight(2, nullptr, false);
        RGetDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);
    }
    
    // Helper 3: Inicializar estructura D3DLIGHT9
    void InitD3DLight(D3DLIGHT9& Light, D3DLIGHTTYPE Type = D3DLIGHT_POINT)
    {
        ZeroMemory(&Light, sizeof(D3DLIGHT9));
        Light.Type = Type;
        Light.Specular.r = 1.0f;
        Light.Specular.g = 1.0f;
        Light.Specular.b = 1.0f;
        Light.Specular.a = 1.0f;
    }
}
```

### Paso 2: Refactorizar `Draw_SetLight()`

```cpp
void ZCharacterObject::Draw_SetLight(const rvector& vPosition)
{
    LightingHelpers::SetAmbientLight();
    
    if (!ZGetConfiguration()->GetVideo()->bDynamicLight)
    {
        LightingHelpers::DisableAllLights(m_pVMesh);
        return;
    }
    
    SetGunLight();
    
    rvector char_pos = vPosition;
    char_pos.z += 180.f;
    auto* FirstLight = SetMapLight(char_pos, m_pVMesh, 1, nullptr);
    if (FirstLight)
        SetMapLight(char_pos, m_pVMesh, 2, FirstLight);
    
    RGetDevice()->SetRenderState(D3DRS_LIGHTING, TRUE);
}
```

### Paso 3: Crear `Draw_SetLight_ForNPC()`

```cpp
// En ZCharacterObject.h (protected)
protected:
    void Draw_SetLight_ForNPC(const rvector& vPosition);

// En ZCharacterObject.cpp
void ZCharacterObject::Draw_SetLight_ForNPC(const rvector& vPosition)
{
    LightingHelpers::SetAmbientLight();
    
    if (!ZGetConfiguration()->GetVideo()->bDynamicLight)
    {
        LightingHelpers::DisableAllLights(m_pVMesh);
        return;
    }
    
    // NPCs no tienen armas, solo buscar 1 luz del mapa
    rvector char_pos = vPosition;
    char_pos.z += 180.f;
    
    m_pVMesh->SetLight(0, nullptr, false);  // No hay arma
    SetMapLight(char_pos, m_pVMesh, 1, nullptr);
    m_pVMesh->SetLight(2, nullptr, false);
    
    RGetDevice()->SetRenderState(D3DRS_LIGHTING, TRUE);
}
```

### Paso 4: Usar en `ZActor::OnDraw()`

```cpp
void ZActor::OnDraw()
{
    if (m_pVMesh == NULL) return;
    
    Draw_SetLight_ForNPC(m_Position);  // ✅ Reutiliza código optimizado
    
    if( IsDieAnimationDone() )
    {
        // ... código de opacidad ...
    }
    else {
        if(!m_bHero) m_pVMesh->SetVisibility(1.f);
        m_TempBackupTime = -1;
    }
    
    m_pVMesh->Render();
}
```

---

## ✅ Beneficios de la Refactorización

1. **Eliminación de Duplicación**: 
   - Código de iluminación ambiente centralizado
   - Helpers reutilizables en múltiples lugares

2. **Mantenibilidad**:
   - Cambios en un solo lugar se propagan a todos los usos
   - Menos errores por inconsistencias

3. **Rendimiento**:
   - `Draw_SetLight_ForNPC()` optimizado para NPCs (sin `SetGunLight()`)
   - Reutiliza código existente sin duplicar lógica

4. **Legibilidad**:
   - Código más claro y expresivo
   - Funciones con nombres descriptivos

---

## ⚠️ Consideraciones

1. **Namespace vs Static Functions**:
   - Usar `namespace LightingHelpers` para evitar colisiones
   - O hacer funciones `static` en `ZCharacterObject.cpp`

2. **Compatibilidad**:
   - Mantener `Draw_SetLight()` para jugadores
   - Agregar `Draw_SetLight_ForNPC()` sin romper código existente

3. **Testing**:
   - Verificar que NPCs se vean correctamente iluminados
   - Verificar que jugadores mantengan iluminación completa

---

## 📝 Estado Actual

- ✅ `SetMapLight()` ya está bien diseñada (función estática)
- ⚠️ Código de iluminación ambiente duplicado en 4+ lugares
- ⚠️ Código de desactivar luces duplicado
- ⚠️ `ZActor::OnDraw()` no usa iluminación (debería usar versión optimizada)
- ⚠️ Inicialización de `D3DLIGHT9` duplicada

**Prioridad**: Implementar `Draw_SetLight_ForNPC()` y helpers básicos (Paso 1-3)

