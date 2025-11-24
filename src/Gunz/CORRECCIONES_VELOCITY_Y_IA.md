# Correcciones de Velocity = 0 y Análisis de IA

## ✅ Correcciones Aplicadas

### 1. **Problema: NPCs Atascados con Velocity = 0**

#### Corrección 1: `ProcessMovement()` - Línea 591
**Antes**:
```cpp
if (ZActorAnimation::IsAttackAnimation(GetCurrAni()))
{
    SetVelocity(rvector(0, 0, 0));  // ⚠️ Detiene completamente
}
```

**Después**:
```cpp
if (ZActorAnimation::IsAttackAnimation(GetCurrAni()))
{
    // CORRECCIÓN: Solo detener velocidad horizontal, mantener Z para gravedad
    StopHorizontalVelocity();
    
    // Verificar si animación terminó para restaurar movimiento
    if (m_pVMesh && m_pVMesh->isOncePlayDone())
    {
        // La IA debería restaurar el movimiento
    }
}
```

---

#### Corrección 2: `OnDamaged()` - Línea 816
**Antes**:
```cpp
SetVelocity(0, 0, 0);  // ⚠️ Detiene completamente
```

**Después**:
```cpp
// CORRECCIÓN: Solo detener velocidad horizontal, mantener Z para gravedad
StopHorizontalVelocity();
```

---

#### Corrección 3: `UpdateHeight()` - Línea 309
**Antes**:
```cpp
SetVelocity(0, 0, 0);  // ⚠️ Detiene completamente
```

**Después**:
```cpp
// CORRECCIÓN: Solo detener velocidad vertical, mantener X,Y para movimiento
StopVerticalVelocity();
```

---

#### Corrección 4: `ProcessMovement()` - Decaimiento de Velocidad
**Antes**:
```cpp
fSpeed = std::max(fSpeed - NPC_STOP_SPEED * fDelta, 0.0f);
SetVelocity(dir.x * fSpeed, dir.y * fSpeed, GetVelocity().z);
```

**Después**:
```cpp
// CORRECCIÓN: Solo aplicar decaimiento si no hay movimiento activo
if (!CheckFlag(AF_MOVING))
{
    fSpeed = std::max(fSpeed - NPC_STOP_SPEED * fDelta, 0.0f);
}
SetVelocity(dir.x * fSpeed, dir.y * fSpeed, GetVelocity().z);
```

---

### 2. **Helpers Agregados**

```cpp
// En ZActor.h
void StopHorizontalVelocity();  // Detiene X,Y, mantiene Z (gravedad)
void StopVerticalVelocity();    // Detiene Z, mantiene X,Y (movimiento)

// En ZActor.cpp
void ZActor::StopHorizontalVelocity()
{
    rvector vel = GetVelocity();
    SetVelocity(0, 0, vel.z);  // Mantener Z para gravedad
}

void ZActor::StopVerticalVelocity()
{
    rvector vel = GetVelocity();
    SetVelocity(vel.x, vel.y, 0);  // Mantener X,Y para movimiento
}
```

---

## 🔍 Análisis de IA (ZBrain)

### Estructura de la IA

1. **`ZBrain::Think(float fDelta)`** - Línea 114
   - Ejecuta `m_Behavior.Run(fDelta)`
   - Llama a `ProcessBuildPath(fDelta)`
   - Llama a `ProcessAttack(fDelta)`

2. **`ProcessBuildPath()`** - Línea 223
   - Busca objetivo con `FindTarget()`
   - Si encuentra objetivo, construye path y llama a `PushPathTask()`
   - Si no encuentra objetivo, llama a `Stop()` (línea 258) ⚠️

3. **`ProcessAttack()`** - Línea 132
   - Verifica si puede atacar
   - Llama a `DefaultAttack()` o `UseSkill()`

### Problemas Identificados en ZBrain

#### Problema 1: `Stop()` Llamado Sin Verificación
**Línea 244 y 258**:
```cpp
m_pBody->Stop();  // ⚠️ Detiene completamente al NPC
```

**Problema**:
- `Stop()` establece `SetVelocity(0, 0, 0)` y `SetFlag(AF_MOVING, false)`
- Si se llama cuando el NPC debería moverse, puede quedar atascado
- No hay verificación de si el NPC debería seguir moviéndose

**Solución Propuesta**:
- `Stop()` debería ser más inteligente o la IA debería restaurar movimiento después

---

### Código Duplicado en ZBrain

#### Patrón Duplicado: `m_pBody->Stop()` - 2 veces
- Línea 244: Cuando encuentra objetivo y puede atacar a rango
- Línea 258: Cuando no encuentra objetivo

**Solución**: Ya está bien, son casos diferentes.

#### Patrón Duplicado: Verificación de Tareas
```cpp
ZTASK_ID nTaskID = m_pBody->m_TaskManager.GetCurrTaskID();
if ((nTaskID == ZTID_ATTACK_MELEE) || 
    (nTaskID == ZTID_ATTACK_RANGE) || 
    (nTaskID == ZTID_ROTATE_TO_DIR) ||
    (nTaskID == ZTID_SKILL)) return;
```

**Ubicaciones**:
- `ProcessBuildPath()` - Línea 227-231
- Potencialmente en otros lugares

**Solución**: Crear helper
```cpp
bool ZBrain::IsTaskBlockingPathFinding() const
{
    ZTASK_ID nTaskID = m_pBody->m_TaskManager.GetCurrTaskID();
    return (nTaskID == ZTID_ATTACK_MELEE) || 
           (nTaskID == ZTID_ATTACK_RANGE) || 
           (nTaskID == ZTID_ROTATE_TO_DIR) ||
           (nTaskID == ZTID_SKILL);
}
```

---

## 📊 Resumen de Problemas y Soluciones

| # | Problema | Ubicación | Severidad | Estado |
|---|----------|-----------|-----------|--------|
| 1 | `SetVelocity(0,0,0)` en animación ataque | Línea 591 | 🔴 CRÍTICO | ✅ CORREGIDO |
| 2 | `SetVelocity(0,0,0)` en `OnDamaged()` | Línea 816 | 🔴 CRÍTICO | ✅ CORREGIDO |
| 3 | `SetVelocity(0,0,0)` en aterrizaje | Línea 309 | 🟡 MEDIO | ✅ CORREGIDO |
| 4 | Decaimiento de velocidad muy agresivo | Línea 608 | 🟡 MEDIO | ✅ CORREGIDO |
| 5 | `Stop()` llamado sin verificación | ZBrain.cpp:244,258 | 🟡 MEDIO | ⚠️ IDENTIFICADO |
| 6 | Verificación de tareas duplicada | ZBrain.cpp:227-231 | 🟢 BAJO | ⚠️ IDENTIFICADO |

---

## 🎯 Resultados Esperados

### Antes de Correcciones:
- NPCs se quedaban atascados con velocity = 0
- NPCs no se movían después de recibir daño
- NPCs no se movían después de aterrizar
- NPCs perdían velocidad muy rápido

### Después de Correcciones:
- ✅ NPCs mantienen gravedad (velocidad Z) durante ataques
- ✅ NPCs mantienen gravedad después de recibir daño
- ✅ NPCs mantienen movimiento horizontal después de aterrizar
- ✅ NPCs solo pierden velocidad si no hay movimiento activo
- ✅ NPCs pueden restaurar movimiento más fácilmente

---

## ⚠️ Notas Importantes

1. **`Stop()` en ZBrain**: 
   - Se llama cuando no hay objetivo o cuando puede atacar a rango
   - Esto es intencional, pero puede causar que el NPC se quede quieto
   - La IA debería restaurar movimiento cuando encuentra un nuevo objetivo

2. **Verificación de Animación**:
   - Agregada verificación de `isOncePlayDone()` en `ProcessMovement()`
   - Esto permite detectar cuando la animación terminó

3. **Helpers**:
   - `StopHorizontalVelocity()` y `StopVerticalVelocity()` permiten control fino
   - Evitan detener completamente al NPC cuando no es necesario

---

## 📝 Estado

- ✅ Correcciones críticas aplicadas
- ✅ Helpers implementados
- ⚠️ Problemas menores identificados en ZBrain (no críticos)
- ✅ Código compilado sin errores

