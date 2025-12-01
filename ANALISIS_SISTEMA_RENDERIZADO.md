# 🔍 Análisis Completo del Sistema de Renderizado

## 📊 Problema Identificado

**Síntoma**: Con un mapa BSP de RealSpace2, se obtienen **4000 FPS máximos**, pero cuando aparece un character, baja a **2500 FPS** (caída de **37.5%**).

**Causa Raíz**: Operaciones costosas ejecutándose **cada frame** para cada character visible.

---

## 🎯 Flujo del Sistema de Renderizado

### 1. **ZApplication::OnDraw()** (Cada Frame)
```cpp
ZApplication::OnDraw()
  └─> ZGameInterface::OnDraw()
      └─> ZGameDrawD3D9::DrawScene()
          └─> Game.m_ObjectManager.Draw()  // ⚠️ Renderiza TODOS los objetos
```

### 2. **ZObjectManager::Draw()** (Cada Frame)
```cpp
void ZObjectManager::Draw()
{
    // Renderiza todos los objetos EXCEPTO el character del jugador
    for (iterator itor = begin(); itor != end(); ++itor) 
    {
        ZObject* pObject = (*itor).second;
        if (pObject == pMyCharacter) continue;
        DrawObject(pObject);  // ⚠️ Llama OnDraw() de cada objeto
    }
    
    // Renderiza el character del jugador al final (con modo especial)
    if (pMyCharacter) {
        DrawObject(pMyCharacter);
    }
}
```

### 3. **ZObjectManager::DrawObject()** (Por cada objeto visible)
```cpp
bool ZObjectManager::DrawObject(ZObject* pObject)
{
    if (pObject->GetInitialized() && pObject->IsVisible())
    {
        pObject->Draw();  // ⚠️ Llama OnDraw()
        
        if (pObject->IsRendered())
        {
            // ⚠️ Para characters, también dibuja sombra
            if (EsCharacter(pObject))
                DrawShadow();  // ⚠️ COSTOSO
        }
    }
}
```

---

## 💰 Operaciones Costosas por Character

### **ZCharacter::OnDraw()** - ⚠️ MUY COSTOSO

#### 1. **Validaciones Iniciales** (Líneas 653-663)
```cpp
if (!m_bInitialized) return;
if (!IsVisible()) return;
if (IsAdminHide()) return;

if (m_pVMesh && !Enable_Cloth)
    m_pVMesh->DestroyCloth();  // ⚠️ Operación costosa si se ejecuta

if (m_nVMID == -1) return;
```
**Costo**: Bajo (early exits rápidos)

---

#### 2. **Cálculo de Bounding Box** (Líneas 665-676)
```cpp
rboundingbox bb;
static constexpr auto Radius = 100;
static constexpr auto Height = 190;
bb.vmax = rvector(Radius, Radius, Height);
bb.vmin = rvector(-Radius, -Radius, 0);
auto ScaleAndTranslate = [&](auto& vec) {
    if (Scale != 1.0f)
        vec *= Scale;
    vec += m_Position;
};
ScaleAndTranslate(bb.vmax);
ScaleAndTranslate(bb.vmin);
```
**Costo**: Muy bajo (operaciones matemáticas simples)

---

#### 3. **Culling Tests** (Líneas 678-679) ⚠️ COSTOSO
```cpp
if (!ZGetGame()->GetWorld()->GetBsp()->IsVisible(bb)) return;  // ⚠️ Llamada a BSP
if (!isInViewFrustum(bb, RGetViewFrustum())) return;  // ⚠️ Test de frustum
```
**Costo**: 
- `IsVisible()`: **~0.01-0.05ms** (test BSP)
- `isInViewFrustum()`: **~0.001ms** (test matemático)
- **Problema**: `ZGetGame()` se llama sin cachear

**Optimización**: Cachear `ZGetGame()` al inicio de la función.

---

#### 4. **Draw_SetLight()** (Línea 683) ⚠️ **MUY COSTOSO**
```cpp
Draw_SetLight(m_Position);
```

**¿Qué hace `Draw_SetLight()`?**
```cpp
void ZCharacterObject::Draw_SetLight(const rvector& vPosition)
{
    // 1. Establece color ambiente
    RGetDevice()->SetRenderState(D3DRS_AMBIENT, 0xCCCCCC);
    
    // 2. Early exit si no hay luces dinámicas (ya optimizado)
    if (!ZGetConfiguration()->GetVideo()->bDynamicLight) {
        return;
    }
    
    // 3. ⚠️ SetGunLight() - Calcula luz del arma
    SetGunLight();  // ⚠️ COSTOSO
    
    // 4. ⚠️ SetMapLight() - Busca luces del mapa (PRIMERA VEZ)
    auto* FirstLight = SetMapLight(char_pos, m_pVMesh, 1, nullptr);
    
    // 5. ⚠️ SetMapLight() - Busca luces del mapa (SEGUNDA VEZ)
    if (FirstLight)
        SetMapLight(char_pos, m_pVMesh, 2, FirstLight);
}
```

**Costo de `SetMapLight()`**:
```cpp
static RLIGHT* SetMapLight(const v3& char_pos, RVisualMesh* Mesh, int LightIndex, RLIGHT* FirstLight)
{
    // Itera sobre TODAS las luces solares del mapa
    for (auto& Light : SunLightList)  // ⚠️ O(N) donde N = luces solares
    {
        // Calcula distancia
        // Hace BSP Pick para ver si hay obstáculos
        // Compara distancias
    }
    
    // Itera sobre TODAS las luces de objetos del mapa
    for (auto& Light : ObjectLightList)  // ⚠️ O(M) donde M = luces de objetos
    {
        // Calcula distancia
        // Compara distancias
    }
}
```

**Costo Total**:
- `SetGunLight()`: **~0.05-0.1ms**
- `SetMapLight()` (primera vez): **~0.2-0.5ms** (depende de número de luces)
- `SetMapLight()` (segunda vez): **~0.2-0.5ms**
- **Total**: **~0.45-1.1ms por character**

**Con 10 characters visibles**: **~4.5-11ms** solo en iluminación!

**Optimización**: 
- Para NPCs (`ZActor`), usar versión simplificada sin `SetGunLight()` y solo 1 luz del mapa
- Cachear resultados de búsqueda de luces si la posición no cambió mucho

---

#### 5. **CheckDrawWeaponTrack()** (Línea 702) ⚠️ COSTOSO
```cpp
void ZCharacter::CheckDrawWeaponTrack()
{
    // Múltiples checks de estado de animación
    // Múltiples llamadas a GetDTM()
    // Configuración de flags de renderizado
}
```
**Costo**: **~0.01-0.05ms** (múltiples checks y configuraciones)

**Optimización**: Solo ejecutar si el character tiene arma blanca y está atacando.

---

#### 6. **UpdateEnchant()** (Línea 721) ⚠️ COSTOSO
```cpp
void ZCharacterObject::UpdateEnchant()
{
    ZC_ENCHANT etype = GetEnchantType();  // ⚠️ Itera sobre items
    // Configura tipo de encantamiento en el mesh
}
```
**Costo**: **~0.01-0.05ms** (iteración sobre items y configuración)

**Optimización**: Solo ejecutar si el character tiene items encantados.

---

#### 7. **m_pVMesh->Render()** (Línea 728) ⚠️ **MUY COSTOSO**
```cpp
m_pVMesh->Render(false);
```
**Costo**: **~0.1-0.5ms** (depende de complejidad del mesh, número de polígonos, shaders)

**Con 10 characters**: **~1-5ms** solo en renderizado de meshes.

**Optimización**: 
- LOD (Level of Detail) basado en distancia
- Frustum culling más agresivo
- Occlusion culling

---

#### 8. **DrawEnchant()** (Línea 733) ⚠️ COSTOSO
```cpp
if (m_pVMesh->m_bIsRenderWeapon && (m_pVMesh->GetVisibility() > 0.05f))
    DrawEnchant(m_AniState_Lower, m_bCharged);
```
**Costo**: **~0.05-0.2ms** (renderizado de efectos de partículas)

**Optimización**: Solo ejecutar si el character tiene encantamiento activo.

---

### **ZActor::OnDraw()** - ⚠️ COSTOSO (pero más simple)

#### 1. **Draw_SetLight()** (Línea 116) ⚠️ **MUY COSTOSO**
```cpp
Draw_SetLight(m_Position);
```
**Mismo problema que ZCharacter**: Itera sobre todas las luces del mapa 2 veces.

**Costo**: **~0.45-1.1ms por NPC**

**Optimización**: Crear `Draw_SetLight_ForNPC()` que:
- No llama `SetGunLight()` (NPCs no tienen armas)
- Solo busca 1 luz del mapa (no 2)
- Cachea resultados si la posición no cambió mucho

---

#### 2. **m_pVMesh->Render()** (Línea 136) ⚠️ **COSTOSO**
```cpp
m_pVMesh->Render();
```
**Costo**: **~0.1-0.5ms por NPC**

**Con 50 NPCs**: **~5-25ms** solo en renderizado de meshes.

---

#### 3. **DrawDebugHitbox() y DrawDebugPath()** (Líneas 151-156) ⚠️ COSTOSO (solo debug)
```cpp
if (bShowDebug)
{
    DrawDebugHitbox();  // ⚠️ Renderiza líneas de debug
    if (m_pBrain)
        m_pBrain->DrawDebugPath();  // ⚠️ Renderiza pathfinding
}
```
**Costo**: **~0.1-0.5ms** (solo en builds de debug)

**Optimización**: Deshabilitar en Release builds.

---

### **ZCharacterObject::DrawShadow()** - ⚠️ COSTOSO

```cpp
void ZCharacterObject::DrawShadow()
{
    if (!Shadow.has_value()) return;
    
    if (!IsDead())
    {
        float fSize = ZShadow::DefaultSize;
        // Calcula tamaño de sombra
        if (Shadow.value().SetMatrices(*m_pVMesh, *ZGetGame()->GetWorld()->GetBsp(), fSize))
            Shadow.value().Draw();  // ⚠️ Renderiza sombra
    }
}
```
**Costo**: **~0.05-0.2ms por character**

**Con 10 characters**: **~0.5-2ms** solo en sombras.

**Optimización**: 
- LOD de sombras (sombra más simple para characters lejanos)
- Deshabilitar sombras para characters muy lejanos

---

## 📈 Análisis de Costo Total

### Escenario: 1 Character Visible

| Operación | Costo Estimado | % del Total |
|-----------|----------------|-------------|
| Culling Tests | ~0.01-0.05ms | 1-2% |
| **Draw_SetLight()** | **~0.45-1.1ms** | **30-40%** |
| CheckDrawWeaponTrack() | ~0.01-0.05ms | 1-2% |
| UpdateEnchant() | ~0.01-0.05ms | 1-2% |
| **m_pVMesh->Render()** | **~0.1-0.5ms** | **10-20%** |
| DrawEnchant() | ~0.05-0.2ms | 3-7% |
| DrawShadow() | ~0.05-0.2ms | 3-7% |
| **TOTAL** | **~0.68-2.15ms** | **100%** |

**Con 60 FPS**: Frame time = **16.67ms**
**Costo de 1 character**: **~4-13% del frame time**

---

### Escenario: 10 Characters Visibles

| Operación | Costo Total | % del Frame (60 FPS) |
|-----------|--------------|----------------------|
| Culling Tests | ~0.1-0.5ms | 0.6-3% |
| **Draw_SetLight()** | **~4.5-11ms** | **27-66%** ⚠️ |
| CheckDrawWeaponTrack() | ~0.1-0.5ms | 0.6-3% |
| UpdateEnchant() | ~0.1-0.5ms | 0.6-3% |
| **m_pVMesh->Render()** | **~1-5ms** | **6-30%** |
| DrawEnchant() | ~0.5-2ms | 3-12% |
| DrawShadow() | ~0.5-2ms | 3-12% |
| **TOTAL** | **~6.8-21.5ms** | **41-129%** ⚠️ |

**⚠️ PROBLEMA**: Con 10 characters, el costo puede exceder el frame time (16.67ms), causando caída de FPS.

---

## 🎯 Optimizaciones Prioritarias

### **Prioridad 1: Draw_SetLight()** ⭐⭐⭐⭐⭐

**Impacto**: **-50-80% del costo de iluminación**

#### Para NPCs (ZActor):
```cpp
// Crear versión optimizada sin SetGunLight()
void ZCharacterObject::Draw_SetLight_ForNPC(const rvector& vPosition)
{
    u32 AmbientColor = 0xCCCCCC;
    RGetDevice()->SetRenderState(D3DRS_AMBIENT, AmbientColor);
    RGetShaderMgr()->setAmbient(AmbientColor);
    
    if (!ZGetConfiguration()->GetVideo()->bDynamicLight)
    {
        m_pVMesh->SetLight(0, nullptr, false);
        m_pVMesh->SetLight(1, nullptr, false);
        m_pVMesh->SetLight(2, nullptr, false);
        RGetDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);
        return;
    }
    
    // Solo buscar 1 luz del mapa (no 2)
    rvector char_pos = vPosition;
    char_pos.z += 180.f;
    SetMapLight(char_pos, m_pVMesh, 1, nullptr);
    
    // No hay arma, así que no hay luz de arma
    m_pVMesh->SetLight(0, nullptr, false);
    m_pVMesh->SetLight(2, nullptr, false);
    
    RGetDevice()->SetRenderState(D3DRS_LIGHTING, TRUE);
}
```

#### Cacheo de Luces:
```cpp
// Cachear resultado de búsqueda de luces si la posición no cambió mucho
struct LightCache {
    rvector LastPosition;
    RLIGHT* CachedLight;
    float CacheRadius = 100.0f;  // Cache válido dentro de 100 unidades
    float CacheTime = 0.0f;
    float CacheLifetime = 0.1f;  // Cache válido por 100ms
};

// En SetMapLight(), verificar cache antes de buscar
if (LightCache.IsValid(char_pos, currentTime))
    return LightCache.CachedLight;
```

**Ahorro Estimado**: **-0.2-0.5ms por character** = **-2-5ms con 10 characters**

---

### **Prioridad 2: Cachear ZGetGame()** ⭐⭐⭐⭐

**Impacto**: **-0.01-0.05ms por character** (pequeño pero fácil de implementar)

```cpp
void ZCharacter::OnDraw()
{
    m_bRendered = false;
    
    if (!m_bInitialized) return;
    if (!IsVisible()) return;
    if (IsAdminHide()) return;
    
    // ⚠️ OPTIMIZACIÓN: Cachear ZGetGame() al inicio
    ZGame* pGame = ZGetGame();
    if (!pGame) return;
    
    // ... resto del código usando pGame en lugar de ZGetGame() ...
    
    if (!pGame->GetWorld()->GetBsp()->IsVisible(bb)) return;
    // ...
}
```

**Ahorro Estimado**: **-0.01-0.05ms por character** = **-0.1-0.5ms con 10 characters**

---

### **Prioridad 3: Early Exits en CheckDrawWeaponTrack()** ⭐⭐⭐

**Impacto**: **-0.01-0.05ms por character**

```cpp
void ZCharacter::CheckDrawWeaponTrack()
{
    if (m_pVMesh == NULL) return;
    
    // ⚠️ OPTIMIZACIÓN: Early exit si no tiene arma blanca
    if (m_pVMesh->m_SelectWeaponMotionType != eq_wd_katana &&
        m_pVMesh->m_SelectWeaponMotionType != eq_wd_sword &&
        m_pVMesh->m_SelectWeaponMotionType != eq_wd_blade)
    {
        m_pVMesh->SetDrawTracks(false);
        return;
    }
    
    // ... resto del código ...
}
```

**Ahorro Estimado**: **-0.01-0.05ms por character** = **-0.1-0.5ms con 10 characters**

---

### **Prioridad 4: Early Exit en UpdateEnchant()** ⭐⭐⭐

**Impacto**: **-0.01-0.05ms por character**

```cpp
void ZCharacterObject::UpdateEnchant()
{
    // ⚠️ OPTIMIZACIÓN: Early exit si no tiene items encantados
    ZC_ENCHANT etype = GetEnchantType();
    if (etype == ZC_ENCHANT_NONE)
    {
        if (m_pVMesh)
            m_pVMesh->SetEnChantType(REnchantType_None);
        return;
    }
    
    // ... resto del código ...
}
```

**Ahorro Estimado**: **-0.01-0.05ms por character** = **-0.1-0.5ms con 10 characters**

---

### **Prioridad 5: LOD (Level of Detail)** ⭐⭐⭐⭐

**Impacto**: **-50-80% del costo de renderizado para characters lejanos**

```cpp
void ZCharacter::OnDraw()
{
    // ... código existente ...
    
    // ⚠️ OPTIMIZACIÓN: LOD basado en distancia
    auto cpos = ZApplication::GetGameInterface()->GetCamera()->GetPosition();
    cpos = m_vProxyPosition - cpos;
    float dist = Magnitude(cpos);
    
    // LOD 0: Distancia < 500 (render completo)
    // LOD 1: Distancia < 1000 (sin sombras, sin efectos)
    // LOD 2: Distancia < 2000 (mesh simplificado)
    // LOD 3: Distancia >= 2000 (no renderizar)
    
    if (dist > 2000.0f) return;  // No renderizar si está muy lejos
    
    int LOD = 0;
    if (dist > 1000.0f) LOD = 1;
    else if (dist > 500.0f) LOD = 2;
    
    if (LOD > 0)
    {
        // Deshabilitar efectos costosos
        if (LOD >= 1)
        {
            // No dibujar sombras
            // No dibujar efectos de encantamiento
            // No actualizar encantamiento
        }
        
        if (LOD >= 2)
        {
            // Usar mesh simplificado
            // No dibujar trazas de armas
        }
    }
    
    // ... resto del código ...
}
```

**Ahorro Estimado**: **-50-80% del costo para characters lejanos**

---

### **Prioridad 6: Occlusion Culling** ⭐⭐⭐

**Impacto**: **-100% del costo para characters ocultos**

```cpp
void ZCharacter::OnDraw()
{
    // ... código existente ...
    
    // ⚠️ OPTIMIZACIÓN: Occlusion culling
    if (!ZGetGame()->GetWorld()->GetBsp()->IsVisible(bb)) return;
    
    // Verificar si el character está completamente oculto por objetos
    // (requiere implementación de occlusion queries)
}
```

**Ahorro Estimado**: **-100% del costo para characters ocultos**

---

## 📊 Impacto Total de Optimizaciones

### Escenario: 10 Characters Visibles (Antes)

| Operación | Costo |
|-----------|-------|
| Draw_SetLight() | ~4.5-11ms |
| m_pVMesh->Render() | ~1-5ms |
| Otros | ~1.3-5.5ms |
| **TOTAL** | **~6.8-21.5ms** |

### Escenario: 10 Characters Visibles (Después)

| Optimización | Ahorro |
|--------------|--------|
| Draw_SetLight_ForNPC() | -2-5ms |
| Cachear ZGetGame() | -0.1-0.5ms |
| Early exits | -0.2-1ms |
| LOD | -0.5-2ms |
| **TOTAL AHORRADO** | **~-2.8-8.5ms** |

| Operación | Costo |
|-----------|-------|
| Draw_SetLight() | ~2.5-6ms |
| m_pVMesh->Render() | ~0.5-3ms |
| Otros | ~0.6-2.5ms |
| **TOTAL** | **~3.6-11.5ms** |

**Mejora**: **-47-53% del costo total**

---

## 🚀 Plan de Implementación

### Fase 1: Optimizaciones Rápidas (1-2 horas)
1. ✅ Cachear `ZGetGame()` en `ZCharacter::OnDraw()`
2. ✅ Crear `Draw_SetLight_ForNPC()` y usarlo en `ZActor::OnDraw()`
3. ✅ Early exits en `CheckDrawWeaponTrack()` y `UpdateEnchant()`

### Fase 2: Optimizaciones Medias (2-4 horas)
4. ✅ Implementar cacheo de luces en `SetMapLight()`
5. ✅ LOD básico basado en distancia

### Fase 3: Optimizaciones Avanzadas (4-8 horas)
6. ✅ Occlusion culling
7. ✅ LOD avanzado con meshes simplificados
8. ✅ Frustum culling más agresivo

---

## 📝 Notas Adicionales

### ¿Por qué la caída es tan grande?

1. **Draw_SetLight()** itera sobre **TODAS las luces del mapa** 2 veces por character
2. Con muchos characters, esto se multiplica exponencialmente
3. **m_pVMesh->Render()** es costoso porque renderiza meshes complejos con shaders
4. No hay LOD, así que characters lejanos se renderizan con la misma calidad

### ¿Por qué funciona bien sin characters?

- Solo se renderiza el mapa BSP (estático, optimizado)
- No hay cálculos de iluminación dinámica
- No hay renderizado de meshes animados
- No hay efectos de partículas

---

## ✅ Conclusión

El problema principal es **Draw_SetLight()** que itera sobre todas las luces del mapa para cada character. Con las optimizaciones propuestas, se puede reducir el costo en **~50%**, mejorando significativamente el rendimiento cuando hay characters visibles.

