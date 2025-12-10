# Análisis y Mejoras: FSM de NPCs y Sistema de Pathfinding

## 🔍 Problemas Identificados por el Usuario

1. **Pocos waypoints en el pathfinding** (solo ~3 puntos)
2. **NPCs se quedan pegado en esquinas**
3. **¿Está iterando cada frame para encontrar una ruta?** (necesita verificación)
4. **FSM limitado** - falta más comportamientos

---

## 📊 Análisis del Sistema Actual

### 1. **Frecuencia de Pathfinding**

**Ubicación**: `Gunz/ZBrain.cpp:289-291`

```cpp
void ZBrain::ProcessBuildPath(float fDelta)
{
    if (!m_PathFindingTimer.Update(fDelta))  // ⚠️ NO es cada frame
        return;
    // ...
}
```

**Respuesta a tu pregunta**: ❌ **NO está iterando cada frame**

El pathfinding usa un **timer** (`m_PathFindingTimer`) que depende de la **inteligencia** del NPC:

- **Timer se inicializa** en `ZBrain::Init()` (línea 96)
- **Tiempo basado en inteligencia**: `MakePathFindingUpdateTime(nIntelligence)`
- **Valores típicos**: 0.5-2.0 segundos (depende de configuración)

**Problema**: Aunque no es cada frame, el pathfinding se ejecuta **muy frecuentemente** y es costoso.

---

### 2. **Pocos Waypoints (Solo ~3 Puntos)**

**Ubicación**: `RealSpace2/Source/RNavigationMesh.cpp:396-412`

**Problema**: El algoritmo usa **"Line of Sight" optimization** que reduce waypoints agresivamente:

```cpp
if (LineOfSightTest(pVantageNode, vantagePos, pTestNode, testPos))
{
    // No agrega waypoint si hay línea de vista directa
    pLastNode = pTestNode;
    lastPos = testPos;
    bPushed = false;  // ⚠️ No agrega waypoint
}
else
{
    // Solo agrega waypoint si NO hay línea de vista
    m_WaypointList.push_back(validatedLastPos);
    bPushed = true;
}
```

**Resultado**: 
- Si hay línea de vista directa → **muy pocos waypoints** (1-3)
- En esquinas estrechas → **falta precisión**, NPCs se quedan atascados

**Ejemplo**:
- NPC en (0, 0)
- Objetivo en (100, 100)
- Hay línea de vista → Solo 1 waypoint (destino)
- **Problema**: Si hay una esquina en el medio, el NPC intenta ir directo y se atasca

---

### 3. **NPCs Se Quedan Pegados en Esquinas**

**Ubicación**: `Gunz/ZBrain.cpp:766-852` - Sistema anti-stuck

**Problema Actual**:

1. **Detección tardía**: Solo detecta stuck después de **1 segundo** sin moverse
   ```cpp
   if (currTime - m_dwExPositionTime > 1000)  // ⚠️ 1 segundo es mucho
   ```

2. **Detección de warp muy tardía**: Solo después de **2 segundos**
   ```cpp
   if (currTime - m_dwExPositionTimeForWarp > 2000)  // ⚠️ 2 segundos
   ```

3. **Umbral muy pequeño**: Solo detecta si se movió menos de **10 unidades** (100 en distancia cuadrada)
   ```cpp
   if (MagnitudeSq(diff) < 100)  // ⚠️ 10 unidades = muy pequeño
   ```

4. **Escape aleatorio simple**: Solo gira y se mueve un poco
   ```cpp
   dir *= m_pBody->GetCollRadius() * 0.8f;  // ⚠️ Muy corto
   ```

**Resultado**: NPCs se quedan atascados en esquinas porque:
- El sistema anti-stuck es muy lento en reaccionar
- El escape es muy corto (solo 0.8 × radio de colisión)
- No hay detección proactiva de esquinas

---

### 4. **FSM Limitado**

**Estados Actuales** (`Gunz/ZBehavior.h`):
- ✅ `ZBEHAVIOR_STATE_IDLE` - Busca objetivos
- ✅ `ZBEHAVIOR_STATE_PATROL` - Patrulla (básico)
- ✅ `ZBEHAVIOR_STATE_ATTACK` - Ataca objetivo
- ✅ `ZBEHAVIOR_STATE_RETREAT` - Retirada (salud baja)
- ✅ `ZBEHAVIOR_STATE_SCRIPT` - Controlado por script

**Falta**:
- ❌ Estado para cuando está "stuck"
- ❌ Estado para manejo de esquinas
- ❌ Estado para seguir a aliados
- ❌ Estado para agruparse (flocking)

---

## 🎯 Recomendaciones de Mejoras

### **Prioridad 1: Mejorar Detección y Escape de Esquinas** ⭐⭐⭐

**Problema**: NPCs se quedan atascados en esquinas porque el sistema anti-stuck es lento.

**Soluciones**:

#### **1.1. Detección Más Rápida de Stuck**
```cpp
// Actual: 1000ms
// Propuesto: 300-500ms
if (currTime - m_dwExPositionTime > 300)  // Más rápido
```

#### **1.2. Detección Proactiva de Colisión con Paredes**
```cpp
void ZBrain::OnBody_CollisionWall()
{
    // OPTIMIZACIÓN: Detectar colisión con pared inmediatamente
    // En lugar de esperar 1 segundo sin moverse
    
    // Agregar waypoint de escape inmediatamente
    EscapeFromCorner();
}
```

#### **1.3. Escape Más Inteligente**
```cpp
bool ZBrain::EscapeFromCorner()
{
    // Intentar múltiples direcciones de escape
    rvector directions[] = {
        m_pBody->GetDirection(),           // Adelante
        rvector(-m_pBody->GetDirection().y, m_pBody->GetDirection().x, 0), // Izquierda
        rvector(m_pBody->GetDirection().y, -m_pBody->GetDirection().x, 0), // Derecha
        -m_pBody->GetDirection()           // Atrás
    };
    
    // Probar cada dirección hasta encontrar una válida
    for (auto& dir : directions)
    {
        rvector escapePos = m_pBody->GetPosition() + dir * (m_pBody->GetCollRadius() * 2.0f);
        // Verificar si es válido con navmesh
        // ...
    }
}
```

---

### **Prioridad 2: Mejorar Generación de Waypoints** ⭐⭐⭐

**Problema**: Solo ~3 waypoints, insuficiente para esquinas.

**Soluciones**:

#### **2.1. Agregar Waypoints Intermedios en Esquinas**
```cpp
// En BuildNavigationPath(), después de LineOfSightTest
if (!LineOfSightTest(...))
{
    // Agregar waypoint intermedio ANTES de la esquina
    rvector cornerPos = CalculateCornerPosition(pLastNode, pTestNode);
    m_WaypointList.push_back(cornerPos);
    
    // Agregar waypoint DESPUÉS de la esquina
    m_WaypointList.push_back(validatedLastPos);
}
```

#### **2.2. Mínimo de Waypoints en Rutas Largas**
```cpp
// Garantizar al menos 5-10 waypoints en rutas largas
if (m_WaypointList.size() < 5 && totalDistance > 1000.0f)
{
    // Agregar waypoints intermedios
    AddIntermediateWaypoints(m_WaypointList);
}
```

#### **2.3. Waypoints Más Cerca de Esquinas**
```cpp
// En lugar de usar el centro del nodo, usar puntos más cercanos a las esquinas
rvector cornerPos = FindCornerPoint(pLastNode, pTestNode);
```

---

### **Prioridad 3: Agregar Nuevos Estados al FSM** ⭐⭐

#### **3.1. Estado STUCK**
```cpp
enum ZBEHAVIOR_STATE
{
    ZBEHAVIOR_STATE_IDLE = 0,
    ZBEHAVIOR_STATE_PATROL,
    ZBEHAVIOR_STATE_ATTACK,
    ZBEHAVIOR_STATE_RETREAT,
    ZBEHAVIOR_STATE_STUCK,      // ✅ NUEVO: Cuando está atascado
    ZBEHAVIOR_STATE_SCRIPT,
    ZBEHAVIOR_STATE_END
};
```

**Comportamiento**:
- Detecta que está stuck
- Intenta múltiples direcciones de escape
- Después de escapar, vuelve al estado anterior

#### **3.2. Estado FOLLOW (Seguir Aliados)**
```cpp
ZBEHAVIOR_STATE_FOLLOW,  // ✅ NUEVO: Seguir a aliados
```

#### **3.3. Estado GROUP (Agruparse)**
```cpp
ZBEHAVIOR_STATE_GROUP,   // ✅ NUEVO: Agruparse con otros NPCs
```

---

### **Prioridad 4: Optimizar Frecuencia de Pathfinding** ⭐⭐

**Problema**: Aunque no es cada frame, puede ser muy frecuente.

**Soluciones**:

#### **4.1. Cache de Rutas**
```cpp
// Cachear rutas recientes para no recalcular si el objetivo no se movió mucho
struct CachedPath
{
    rvector startPos;
    rvector endPos;
    std::list<rvector> waypoints;
    float timestamp;
    static const float CACHE_DURATION = 2.0f;  // 2 segundos
};

std::map<MUID, CachedPath> m_PathCache;  // Cache por objetivo
```

#### **4.2. Pathfinding Solo si el Objetivo se Movió**
```cpp
void ZBrain::ProcessBuildPath(float fDelta)
{
    if (!m_PathFindingTimer.Update(fDelta))
        return;
    
    ZObject* pTarget = GetTarget();
    if (!pTarget) return;
    
    rvector targetPos = pTarget->GetPosition();
    
    // OPTIMIZACIÓN: Solo recalcular si el objetivo se movió significativamente
    float targetMovement = MagnitudeSq(targetPos - m_LastTargetPosition);
    if (targetMovement < 10000.0f)  // 100 unidades cuadradas
    {
        // Objetivo no se movió mucho, usar ruta cacheada
        return;
    }
    
    m_LastTargetPosition = targetPos;
    // ... calcular nueva ruta ...
}
```

---

## 📈 Plan de Implementación

### **Fase 1: Mejoras Críticas** (Alto Impacto)

1. ✅ **Mejorar detección de stuck** (300ms en lugar de 1000ms)
2. ✅ **Escape más inteligente** (múltiples direcciones)
3. ✅ **Agregar waypoints intermedios en esquinas**
4. ✅ **Cache de rutas** (evitar recalcular)

**Impacto esperado**: 
- Eliminación del problema de NPCs atascados en esquinas
- Rutas más suaves con más waypoints
- Mejor rendimiento (menos pathfinding)

---

### **Fase 2: Nuevos Estados FSM** (Medio Impacto)

1. ✅ **Estado STUCK** para manejo explícito
2. ✅ **Mejorar estado PATROL** (más comportamientos)
3. ✅ **Estado FOLLOW** (seguir aliados)

---

### **Fase 3: Optimizaciones Avanzadas** (Bajo Impacto)

1. ✅ **Pathfinding asíncrono** (en hilo separado)
2. ✅ **Spatial partitioning** para búsqueda de objetivos
3. ✅ **Flocking behavior** (agrupamiento de NPCs)

---

## ✅ Respuestas a Tus Preguntas

### **1. ¿Está iterando cada frame para encontrar una ruta?**

❌ **NO**, pero puede ser muy frecuente:
- Usa un **timer** basado en inteligencia
- Típicamente cada **0.5-2.0 segundos**
- **Problema**: Aún es frecuente y costoso

**Recomendación**: Implementar cache de rutas para evitar recalcular si el objetivo no se movió mucho.

---

### **2. ¿Por qué solo ~3 waypoints?**

**Razón**: El algoritmo usa **"Line of Sight" optimization** que reduce waypoints agresivamente.

**Problema**: En esquinas, esto causa que falten waypoints intermedios, haciendo que los NPCs se atascan.

**Recomendación**: Agregar waypoints intermedios específicamente en esquinas.

---

### **3. ¿Por qué se queda pegado en esquinas?**

**Razones**:
1. **Detección lenta**: Espera 1 segundo antes de detectar stuck
2. **Escape corto**: Solo se mueve 0.8 × radio de colisión
3. **Sin detección proactiva**: No detecta colisión con paredes inmediatamente
4. **Pocos waypoints**: Falta precisión en esquinas

**Recomendación**: Mejorar sistema anti-stuck con detección más rápida y escape más inteligente.

---

## 🎯 ¿Quieres que Implemente Estas Mejoras?

Puedo implementar:

1. ✅ **Mejoras críticas** (detección rápida de stuck, escape inteligente, más waypoints)
2. ✅ **Nuevo estado STUCK** en el FSM
3. ✅ **Cache de rutas** para optimizar pathfinding
4. ✅ **Detección proactiva** de colisiones con paredes

¿Con cuál empezamos?




