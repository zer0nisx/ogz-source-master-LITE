# Análisis de IA y Problemas de Velocity = 0

## 🔍 Problemas Identificados

### 1. **Velocity = 0 Causa NPCs Atascados** - ⚠️ CRÍTICO

#### Problema 1: `ProcessMovement()` - Línea 591
```cpp
if (ZActorAnimation::IsAttackAnimation(GetCurrAni()))
{
    SetVelocity(rvector(0, 0, 0));  // ⚠️ PROBLEMA: Si animación no termina, NPC queda atascado
}
```

**Problema**:
- Cuando el NPC está en animación de ataque, se establece velocity a 0
- Si la animación se interrumpe, hay un error, o no termina correctamente, el NPC se queda atascado
- No hay verificación de que la animación haya terminado antes de permitir movimiento

**Solución Propuesta**:
```cpp
if (ZActorAnimation::IsAttackAnimation(GetCurrAni()))
{
    // CORRECCIÓN: Solo detener velocidad horizontal, mantener Z para gravedad
    rvector vel = GetVelocity();
    SetVelocity(0, 0, vel.z);  // Mantener velocidad Z para gravedad
    
    // CORRECCIÓN: Verificar si la animación terminó y restaurar movimiento
    if (m_pVMesh && m_pVMesh->isOncePlayDone())
    {
        // Animación terminó, permitir movimiento de nuevo
        // La IA debería restaurar el movimiento
    }
}
```

---

#### Problema 2: `OnDamaged()` - Línea 816
```cpp
if ((damageType == ZD_MELEE) || (damageType == ZD_KATANA_SPLASH)) {
    // ...
    SetVelocity(0, 0, 0);  // ⚠️ PROBLEMA: Detiene completamente al NPC
}
```

**Problema**:
- Cuando el NPC recibe daño melee, se detiene completamente
- Si el NPC está en medio de un movimiento o ataque, se queda atascado
- No hay restauración automática del movimiento después del daño

**Solución Propuesta**:
```cpp
if ((damageType == ZD_MELEE) || (damageType == ZD_KATANA_SPLASH)) {
    // ...
    // CORRECCIÓN: Solo detener velocidad horizontal, mantener Z para gravedad
    rvector vel = GetVelocity();
    SetVelocity(0, 0, vel.z);  // Mantener velocidad Z para gravedad
    
    // CORRECCIÓN: La IA debería restaurar el movimiento después de la animación de daño
}
```

---

#### Problema 3: `UpdateHeight()` - Línea 309
```cpp
if (m_pModule_Movable->isLanding())
{
    OnReachGround();
    SetVelocity(0, 0, 0);  // ⚠️ PROBLEMA: Puede interferir con movimiento normal
}
```

**Problema**:
- Cuando el NPC aterriza, se detiene completamente
- Si el NPC estaba moviéndose, se queda atascado
- Debería solo detener la velocidad vertical, no la horizontal

**Solución Propuesta**:
```cpp
if (m_pModule_Movable->isLanding())
{
    OnReachGround();
    // CORRECCIÓN: Solo detener velocidad vertical, mantener horizontal
    rvector vel = GetVelocity();
    SetVelocity(vel.x, vel.y, 0);  // Solo detener Z
}
```

---

### 2. **Problema en `ProcessMovement()` - Decaimiento de Velocidad** - ⚠️ POTENCIAL

#### Línea 608
```cpp
fSpeed = std::max(fSpeed - NPC_STOP_SPEED * fDelta, 0.0f);
SetVelocity(dir.x * fSpeed, dir.y * fSpeed, GetVelocity().z);
```

**Problema**:
- Si `NPC_STOP_SPEED` (2000.f) es muy alto y `fDelta` es grande, la velocidad puede llegar a 0 muy rápido
- Si el NPC no está en animación de movimiento pero la IA quiere que se mueva, puede quedar atascado

**Solución Propuesta**:
```cpp
// CORRECCIÓN: Solo aplicar decaimiento si no hay movimiento activo
if (!CheckFlag(AF_MOVING))
{
    fSpeed = std::max(fSpeed - NPC_STOP_SPEED * fDelta, 0.0f);
}
// Si AF_MOVING está activo, la velocidad se mantiene o aumenta
SetVelocity(dir.x * fSpeed, dir.y * fSpeed, GetVelocity().z);
```

---

### 3. **Análisis de IA (ZBrain)** - Pendiente

Necesito revisar `ZBrain.cpp` para:
- Verificar si hay código duplicado
- Verificar si la IA restaura el movimiento correctamente
- Verificar si hay problemas de rendimiento

---

## 📋 Resumen de Problemas

| # | Problema | Ubicación | Severidad | Solución |
|---|----------|-----------|-----------|----------|
| 1 | `SetVelocity(0,0,0)` en animación ataque | Línea 591 | 🔴 CRÍTICO | Detener solo X,Y, mantener Z |
| 2 | `SetVelocity(0,0,0)` en `OnDamaged()` | Línea 816 | 🔴 CRÍTICO | Detener solo X,Y, mantener Z |
| 3 | `SetVelocity(0,0,0)` en aterrizaje | Línea 309 | 🟡 MEDIO | Detener solo Z, mantener X,Y |
| 4 | Decaimiento de velocidad muy agresivo | Línea 608 | 🟡 MEDIO | Solo aplicar si no hay movimiento activo |

---

## 🎯 Soluciones Propuestas

### Solución 1: Helper para Detener Velocidad Horizontal
```cpp
// En ZActor.h (private)
void StopHorizontalVelocity()
{
    rvector vel = GetVelocity();
    SetVelocity(0, 0, vel.z);  // Mantener Z para gravedad
}

void StopVerticalVelocity()
{
    rvector vel = GetVelocity();
    SetVelocity(vel.x, vel.y, 0);  // Mantener X,Y para movimiento
}
```

### Solución 2: Verificar Estado Antes de Detener
```cpp
// En ProcessMovement()
if (ZActorAnimation::IsAttackAnimation(GetCurrAni()))
{
    StopHorizontalVelocity();  // Usar helper
    
    // Verificar si animación terminó
    if (m_pVMesh && m_pVMesh->isOncePlayDone())
    {
        // La IA debería restaurar el movimiento
        // O restaurar automáticamente si no hay tarea activa
        if (!CheckFlag(AF_MOVING) && m_TaskManager.IsEmpty())
        {
            // Restaurar movimiento básico
        }
    }
}
```

---

## ⚠️ Estado Actual

- ✅ Problemas identificados
- ⚠️ Soluciones propuestas
- ⏳ Pendiente: Revisar ZBrain para código duplicado y problemas de IA

