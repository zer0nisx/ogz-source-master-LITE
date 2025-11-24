# Código Duplicado y Reutilizable en ZActor.cpp

## 🔍 Análisis de Duplicación en ZActor

### 1. **CheckFlag(AF_MY_CONTROL)** - ⚠️ MUY REPETIDO (10+ veces)

**Ubicaciones**:
- Línea 166: `OnUpdate()` - if (CheckFlag(AF_MY_CONTROL))
- Línea 192: `OnUpdate()` - if (CheckFlag(AF_MY_CONTROL))
- Línea 354: `UpdatePosition()` - if (CheckFlag(AF_MY_CONTROL))
- Línea 376: `OnBlast()` - if (!CheckFlag(AF_MY_CONTROL)) return;
- Línea 396: `OnBlastDagger()` - if (!CheckFlag(AF_MY_CONTROL)) return;
- Línea 618: `IsDead()` - if (CheckFlag(AF_MY_CONTROL))
- Línea 769: `OnDamaged()` - if (CheckFlag(AF_MY_CONTROL))
- Línea 822: `OnKnockback()` - if (!CheckFlag(AF_MY_CONTROL)) return;
- Línea 835: `CheckDead()` - if (!CheckFlag(AF_MY_CONTROL)) return;
- Línea 899: `isThinkAble()` - if (!CheckFlag(AF_MY_CONTROL)) return false;

**Patrón Duplicado**:
```cpp
if (!CheckFlag(AF_MY_CONTROL)) return;  // Early exit
// o
if (CheckFlag(AF_MY_CONTROL)) { ... }
```

**Solución**: Crear helper inline
```cpp
// En ZActor.h (private)
inline bool IsMyControl() const { return CheckFlag(AF_MY_CONTROL); }

// Uso:
if (!IsMyControl()) return;
```

---

### 2. **Cálculo de Posición de Sonido** - ⚠️ DUPLICADO (2 veces)

**Ubicaciones**:
- Línea 750-751: `OnDamaged()` - `pos_sound = GetPosition() + rvector(0, 0, m_Collision.fHeight - 10.0f)`
- Línea 913-914: `OnPeerDie()` - `pos_sound = GetPosition() + rvector(0, 0, m_Collision.fHeight - 10.0f)`

**Código Duplicado**:
```cpp
rvector pos_sound = GetPosition();
pos_sound.z += m_Collision.fHeight - 10.0f;
```

**Solución**: Crear función helper
```cpp
// En ZActor.h (private)
rvector GetSoundPosition() const
{
    rvector pos = GetPosition();
    pos.z += m_Collision.fHeight - 10.0f;
    return pos;
}

// Uso:
rvector pos_sound = GetSoundPosition();
```

---

### 3. **Normalizar Dirección (dir.z = 0 + Normalize)** - ⚠️ DUPLICADO (4+ veces)

**Ubicaciones**:
- Línea 510-511: `ProcessMotion()` - `dir.z = 0;` (luego se usa en MakeWorldMatrix)
- Línea 605-606: `RunTo()` - `dir.z = 0.0f; Normalize(dir);`
- Línea 932: `CanSee()` - `vBodyDir.z = vTargetDir.z = 0.0f;`
- Línea 987: `CanAttackMelee()` - `vBodyDir.z = vTargetDir.z = 0.0f;`

**Código Duplicado**:
```cpp
dir.z = 0.0f;  // o dir.z = 0;
Normalize(dir);
```

**Solución**: Crear función helper
```cpp
// En ZActor.h (private)
static void NormalizeDirection2D(rvector& dir)
{
    dir.z = 0.0f;
    Normalize(dir);
}

// Uso:
NormalizeDirection2D(dir);
```

---

### 4. **SetFlag(AF_LAND) + m_Animation.Input(ZA_EVENT_REACH_GROUND)** - ⚠️ DUPLICADO (2 veces)

**Ubicaciones**:
- Línea 283-289: `UpdateHeight()` - Cuando detecta aterrizaje
- Línea 301-302: `UpdateHeight()` - Cuando `isLanding()` retorna true

**Código Duplicado**:
```cpp
SetFlag(AF_LAND, true);
m_Animation.Input(ZA_EVENT_REACH_GROUND);
```

**Solución**: Crear función helper
```cpp
// En ZActor.h (private)
void OnReachGround()
{
    SetFlag(AF_LAND, true);
    m_Animation.Input(ZA_EVENT_REACH_GROUND);
}

// Uso:
OnReachGround();
```

---

### 5. **CheckFlag(AF_LAND)** - ⚠️ REPETIDO (5+ veces)

**Ubicaciones**:
- Línea 286: `UpdateHeight()` - `if (!CheckFlag(AF_LAND))`
- Línea 293: `UpdateHeight()` - `if (!CheckFlag(AF_LAND))`
- Línea 301: `UpdateHeight()` - `SetFlag(AF_LAND, true);`
- Línea 535: `ProcessMovement()` - `bool bLand = CheckFlag(AF_LAND);`
- Línea 537: `ProcessMovement()` - `if (CheckFlag(AF_MOVING) && CheckFlag(AF_LAND) && ...)`

**Solución**: Crear helper inline
```cpp
// En ZActor.h (private)
inline bool IsOnLand() const { return CheckFlag(AF_LAND); }
inline void SetOnLand(bool bOnLand) { SetFlag(AF_LAND, bOnLand); }
```

---

### 6. **m_pVMesh Null Check** - ⚠️ REPETIDO (3+ veces)

**Ubicaciones**:
- Línea 112: `OnDraw()` - `if (m_pVMesh == NULL) return;`
- Línea 505: `ProcessMotion()` - `if (!m_pVMesh) return false;`
- Línea 713: `HitTest()` - `if (m_pVMesh) { ... }`

**Patrón Duplicado**:
```cpp
if (m_pVMesh == NULL) return;  // o if (!m_pVMesh) return false;
```

**Solución**: Crear helper inline
```cpp
// En ZActor.h (private)
inline bool HasVMesh() const { return m_pVMesh != nullptr; }

// Uso:
if (!HasVMesh()) return;
```

---

### 7. **GetCurrAni() Checks** - ⚠️ REPETIDO (4+ veces)

**Ubicaciones**:
- Línea 538: `ProcessMovement()` - `if ((GetCurrAni() == ZA_ANIM_WALK) || (GetCurrAni() == ZA_ANIM_RUN))`
- Línea 579: `ProcessMovement()` - `if (ZActorAnimation::IsAttackAnimation(GetCurrAni()))`
- Línea 877: `IsAttackable()` - `if ((nAnimState == ZA_ANIM_IDLE) || (nAnimState == ZA_ANIM_WALK) || (nAnimState == ZA_ANIM_RUN))`
- Línea 889: `IsCollideable()` - `if (nAnimState == ZA_ANIM_DIE) return false;`

**Solución**: Ya existe `ZActorAnimation::IsAttackAnimation()`, pero podríamos agregar más helpers
```cpp
// Ya existe en ZActorAnimation, pero podríamos agregar:
inline bool IsMovingAnimation() const
{
    ZA_ANIM_STATE state = GetCurrAni();
    return (state == ZA_ANIM_WALK) || (state == ZA_ANIM_RUN);
}
```

---

### 8. **Early Exit con CheckFlag(AF_MY_CONTROL)** - ⚠️ PATRÓN REPETIDO

**Ubicaciones**:
- Línea 376: `OnBlast()` - `if (!CheckFlag(AF_MY_CONTROL)) return;`
- Línea 396: `OnBlastDagger()` - `if (!CheckFlag(AF_MY_CONTROL)) return;`
- Línea 822: `OnKnockback()` - `if (!CheckFlag(AF_MY_CONTROL)) return;`
- Línea 835: `CheckDead()` - `if (!CheckFlag(AF_MY_CONTROL)) return;`

**Patrón Duplicado**:
```cpp
if (!CheckFlag(AF_MY_CONTROL)) return;
```

**Solución**: Usar helper `IsMyControl()` (ver #1)

---

### 9. **Variable `fSpeed = m_fSpeed` Duplicada** - ⚠️ REDUNDANTE

**Ubicaciones**:
- Línea 540: `ProcessMovement()` - `float fSpeed = m_fSpeed;`
- Línea 541: `ProcessMovement()` - `if (GetCurrAni() == ZA_ANIM_RUN) fSpeed = m_fSpeed;` ⚠️ **REDUNDANTE**
- Línea 558: `ProcessMovement()` - `float fSpeed = m_fSpeed;` (en bloque BLAST_DAGGER)

**Problema**:
```cpp
float fSpeed = m_fSpeed;
if (GetCurrAni() == ZA_ANIM_RUN) fSpeed = m_fSpeed;  // ⚠️ Línea 541 - Redundante!
```

**Solución**: Eliminar línea 541 (ya es `m_fSpeed`)

---

### 10. **Cálculo de Ángulo entre Vectores** - ⚠️ DUPLICADO (2 veces)

**Ubicaciones**:
- Línea 934: `CanSee()` - `float angle = fabs(GetAngleOfVectors(vTargetDir, vBodyDir));`
- Línea 989: `CanAttackMelee()` - `float angle = fabs(GetAngleOfVectors(vTargetDir, vBodyDir));`

**Código Duplicado**:
```cpp
rvector vTargetDir = pTarget->GetPosition() - GetPosition();
rvector vBodyDir = GetDirection();
vBodyDir.z = vTargetDir.z = 0.0f;
float angle = fabs(GetAngleOfVectors(vTargetDir, vBodyDir));
```

**Solución**: Crear función helper
```cpp
// En ZActor.h (private)
float GetAngleToTarget(ZObject* pTarget) const
{
    rvector vTargetDir = pTarget->GetPosition() - GetPosition();
    rvector vBodyDir = GetDirection();
    vBodyDir.z = vTargetDir.z = 0.0f;
    return fabs(GetAngleOfVectors(vTargetDir, vBodyDir));
}

// Uso:
float angle = GetAngleToTarget(pTarget);
```

---

## 📋 Resumen de Refactorizaciones Propuestas

| # | Duplicación | Ubicaciones | Solución | Prioridad |
|---|------------|-------------|----------|-----------|
| 1 | `CheckFlag(AF_MY_CONTROL)` | 10+ veces | `IsMyControl()` helper | ⭐⭐⭐⭐⭐ |
| 2 | Cálculo posición sonido | 2 veces | `GetSoundPosition()` | ⭐⭐⭐ |
| 3 | Normalizar dirección 2D | 4+ veces | `NormalizeDirection2D()` | ⭐⭐⭐ |
| 4 | `SetFlag(AF_LAND) + Input(REACH_GROUND)` | 2 veces | `OnReachGround()` | ⭐⭐⭐ |
| 5 | `CheckFlag(AF_LAND)` | 5+ veces | `IsOnLand()` helper | ⭐⭐ |
| 6 | `m_pVMesh` null check | 3+ veces | `HasVMesh()` helper | ⭐⭐ |
| 7 | `GetCurrAni()` checks | 4+ veces | Helpers de animación | ⭐⭐ |
| 8 | Early exit `AF_MY_CONTROL` | 4 veces | Usar `IsMyControl()` | ⭐⭐⭐ |
| 9 | `fSpeed = m_fSpeed` redundante | Línea 541 | Eliminar línea | ⭐⭐⭐ |
| 10 | Cálculo ángulo a objetivo | 2 veces | `GetAngleToTarget()` | ⭐⭐⭐ |

---

## 🎯 Implementación Recomendada

### Paso 1: Agregar Helpers en ZActor.h

```cpp
// En ZActor.h (private section)
private:
    // Helpers para flags
    inline bool IsMyControl() const { return CheckFlag(AF_MY_CONTROL); }
    inline bool IsOnLand() const { return CheckFlag(AF_LAND); }
    inline void SetOnLand(bool bOnLand) { SetFlag(AF_LAND, bOnLand); }
    
    // Helpers para validaciones
    inline bool HasVMesh() const { return m_pVMesh != nullptr; }
    
    // Helpers para cálculos
    rvector GetSoundPosition() const;
    static void NormalizeDirection2D(rvector& dir);
    float GetAngleToTarget(ZObject* pTarget) const;
    void OnReachGround();
```

### Paso 2: Implementar Helpers en ZActor.cpp

```cpp
// En ZActor.cpp

rvector ZActor::GetSoundPosition() const
{
    rvector pos = GetPosition();
    pos.z += m_Collision.fHeight - 10.0f;
    return pos;
}

void ZActor::NormalizeDirection2D(rvector& dir)
{
    dir.z = 0.0f;
    Normalize(dir);
}

float ZActor::GetAngleToTarget(ZObject* pTarget) const
{
    rvector vTargetDir = pTarget->GetPosition() - GetPosition();
    rvector vBodyDir = GetDirection();
    vBodyDir.z = vTargetDir.z = 0.0f;
    return fabs(GetAngleOfVectors(vTargetDir, vBodyDir));
}

void ZActor::OnReachGround()
{
    SetFlag(AF_LAND, true);
    m_Animation.Input(ZA_EVENT_REACH_GROUND);
}
```

### Paso 3: Refactorizar Código Existente

**Ejemplo 1: OnUpdate()**
```cpp
// ANTES:
if (CheckFlag(AF_MY_CONTROL))
{
    // ...
}
if (CheckFlag(AF_MY_CONTROL))
{
    UpdateHeight(fDelta);
}

// DESPUÉS:
if (IsMyControl())
{
    // ...
    UpdateHeight(fDelta);
}
```

**Ejemplo 2: OnDamaged() y OnPeerDie()**
```cpp
// ANTES:
rvector pos_sound = GetPosition();
pos_sound.z += m_Collision.fHeight - 10.0f;

// DESPUÉS:
rvector pos_sound = GetSoundPosition();
```

**Ejemplo 3: UpdateHeight()**
```cpp
// ANTES:
SetFlag(AF_LAND, true);
m_Animation.Input(ZA_EVENT_REACH_GROUND);

// DESPUÉS:
OnReachGround();
```

**Ejemplo 4: CanSee() y CanAttackMelee()**
```cpp
// ANTES:
rvector vTargetDir = pTarget->GetPosition() - GetPosition();
rvector vBodyDir = GetDirection();
vBodyDir.z = vTargetDir.z = 0.0f;
float angle = fabs(GetAngleOfVectors(vTargetDir, vBodyDir));

// DESPUÉS:
float angle = GetAngleToTarget(pTarget);
```

**Ejemplo 5: Eliminar línea redundante**
```cpp
// ANTES (línea 540-541):
float fSpeed = m_fSpeed;
if (GetCurrAni() == ZA_ANIM_RUN) fSpeed = m_fSpeed;  // ⚠️ Redundante

// DESPUÉS:
float fSpeed = m_fSpeed;
// Eliminar línea 541
```

---

## ✅ Beneficios

1. **Legibilidad**: Código más expresivo y fácil de leer
2. **Mantenibilidad**: Cambios en un solo lugar
3. **Consistencia**: Mismo comportamiento en todos los lugares
4. **Menos Errores**: Evita inconsistencias por copiar/pegar

---

## ⚠️ Consideraciones

1. **Performance**: Los helpers inline no tienen overhead
2. **Compatibilidad**: No cambia la lógica, solo refactoriza
3. **Testing**: Verificar que el comportamiento sea idéntico

---

## 📝 Estado Actual

- ✅ Código funcional pero con mucha duplicación
- ⚠️ `CheckFlag(AF_MY_CONTROL)` usado 10+ veces
- ⚠️ Cálculo de posición de sonido duplicado
- ⚠️ Normalización de direcciones duplicada
- ⚠️ Lógica de aterrizaje duplicada
- ⚠️ Línea 541 redundante (`fSpeed = m_fSpeed` dos veces)

**Prioridad**: Implementar helpers básicos (#1, #2, #4, #9) para mayor impacto

