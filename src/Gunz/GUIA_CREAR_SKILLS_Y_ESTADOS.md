# Guía Completa: Crear Nuevos Skills y Estados Alterados

## 📋 Tabla de Contenidos
1. [Crear Nuevos Skills](#crear-nuevos-skills)
2. [Estados Alterados Existentes](#estados-alterados-existentes)
3. [Crear Nuevos Módulos de Estado](#crear-nuevos-módulos-de-estado)
4. [Ejemplos Prácticos](#ejemplos-prácticos)

---

## 🎯 Crear Nuevos Skills

### Proceso de Creación

#### 1. Agregar Skill al XML (`zskill.xml`)

Los skills se definen en `build/win32/bin/Release/system/zskill.xml`. Cada skill tiene un `id` único.

**Estructura básica de un skill:**
```xml
<SKILL id="XXX" 
    name="Nombre del Skill"
    effecttype="0"              <!-- Tipo de efecto: 0=Enemigo, 1=Área Enemiga, 2=Área Propia, 3=Todo, 4=Slash Área, 5=Aliado, 6=Área Aliada -->
    hitcheck="true"             <!-- true=Proyectil que persigue, false=Efecto instantáneo -->
    guidable="false"            <!-- true=El proyectil persigue al objetivo, false=Vuela recto -->
    velocity="5000"             <!-- Velocidad del proyectil (si hitcheck=true) -->
    delay="2000"                <!-- Tiempo de delay antes de ejecutar (ms) -->
    lifetime="0"                <!-- Duración del proyectil (ms, 0=infinito hasta impacto) -->
    effectstarttime="100"       <!-- Tiempo antes de aplicar efecto (ms) -->
    effecttime="0"              <!-- Duración del efecto (ms) -->
    effectarea="2"              <!-- Radio del área de efecto (unidades * 100) -->
    effectareamin="1.5"         <!-- Radio mínimo (para skills de melee) -->
    effectangle="30"            <!-- Ángulo del efecto (grados) -->
    resisttype="1"              <!-- 0=Ninguna, 1=Fuego, 2=Frío, 3=Rayo, 4=Veneno -->
    difficulty="15"             <!-- Dificultad de resistencia (0-100) -->
    knockback="10"              <!-- Fuerza de knockback -->
    throughnpc="true"           <!-- true=Atraviesa NPCs, false=Colisiona -->
    
    <!-- Modificadores de Estado -->
    mod.damage="50"             <!-- Daño instantáneo -->
    mod.dot="0"                 <!-- Daño por segundo (Damage Over Time) -->
    mod.lastdamage="0"          <!-- Daño al final del efecto -->
    mod.criticalrate="30"       <!-- Tasa de crítico (%) -->
    mod.speed="50"              <!-- Velocidad de movimiento (%) - 100=normal, 50=mitad -->
    mod.antimotion="false"      <!-- Bloquea animaciones -->
    mod.root="true"             <!-- Inmoviliza al objetivo (aplica Stun) -->
    mod.heal="0"                <!-- Curación -->
    mod.repair="0"              <!-- Reparación de AP -->
    
    <!-- Efectos Visuales -->
    castinganimation="1"        <!-- 1-4: Animación de lanzamiento -->
    castingeffect="ef_fireball" <!-- Efecto visual al lanzar -->
    castingeffectType="1"       <!-- Tipo de posición: 1=Source, 2=Target, 3=Effect, etc. -->
    castingeffectAddPos="0.0 0.0 0.0" <!-- Offset de posición del efecto -->
    castingpreeffect="ef_cast"  <!-- Efecto antes de lanzar -->
    castingpreeffectType="4"    <!-- Posición del pre-efecto -->
    castingeffectSp="BlizzardEffect" <!-- Efecto especial -->
    castingeffectSpCount="5"   <!-- Cantidad de efectos especiales -->
    
    <!-- Efectos de Proyectil -->
    traileffect="fireball"      <!-- Nombre del mesh del proyectil -->
    traileffecttype="1"         <!-- 1=Fuego, 2=Frío, 3=Magia -->
    traileffectscale="2.0"      <!-- Escala del proyectil -->
    draw.track="true"           <!-- Dibujar rastro -->
    colradius="35"              <!-- Radio de colisión -->
    
    <!-- Sonidos -->
    effectsound="we_enfire"     <!-- Sonido al ejecutar -->
    sound.explosion="we_enfire" <!-- Sonido al explotar -->
    
    <!-- Cámara -->
    camera.shock="true"         <!-- Aplicar shake de cámara -->
    camera.power="1.0"          <!-- Intensidad del shake -->
    camera.duration="0.7"       <!-- Duración del shake (segundos) -->
    camera.range="600"          <!-- Rango del shake -->
    
    <!-- Mensaje -->
    message="¡Skill ejecutado!" <!-- Mensaje en chat -->
    
    <!-- Repeticiones (para múltiples proyectiles) -->
    >
    <REPEAT delay="0.0" angle="0.0 0.0 0.2" />  <!-- Repetición con ángulo -->
    <REPEAT delay="0.0" angle="0.0 0.0 -0.2" />
</SKILL>
```

#### 2. Tipos de Efecto (`effecttype`)

```cpp
enum ZSKILLEFFECTTYPE {
    ZSE_ENEMY = 0,          // Solo el objetivo específico
    ZSE_ENEMY_AREA = 1,     // Área de enemigos
    ZSE_OWNER_AREA = 2,     // Área alrededor del lanzador
    ZSE_WHOLE_AREA = 3,     // Todo el mapa
    ZSE_SLASH_AREA = 4,     // Área de slash (melee)
    ZSE_ALLIED = 5,         // Solo aliados
    ZSE_ALLIED_AREA = 6     // Área de aliados
};
```

#### 3. Ejemplo de Skill Completo

```xml
<!-- Skill de Veneno que persigue múltiples objetivos -->
<SKILL id="501" name="Poison Swarm" 
    effecttype="1" hitcheck="true" guidable="true" 
    velocity="3000" delay="5000" lifetime="0" 
    difficulty="20" colradius="25" knockback="5" 
    effectstarttime="500" effectarea="3"
    mod.damage="10" mod.dot="5" mod.criticalrate="20" 
    mod.speed="100" mod.root="false" mod.heal="0"
    resisttype="4" 
    castinganimation="2" castingeffectType="3"
    traileffect="ef_poisonball2" traileffecttype="1" 
    traileffectscale="2.0" sound.explosion="we_enpoison">
    <!-- Crea 5 proyectiles que persiguen diferentes objetivos -->
    <REPEAT delay="0.0" angle="0.0 0.0 0.3" />
    <REPEAT delay="0.0" angle="0.0 0.0 0.15" />
    <REPEAT delay="0.0" angle="0.0 0.0 0.0" />
    <REPEAT delay="0.0" angle="0.0 0.0 -0.15" />
    <REPEAT delay="0.0" angle="0.0 0.0 -0.3" />
</SKILL>
```

---

## 🔮 Estados Alterados Existentes

### Estados Disponibles Actualmente

#### 1. **Stun (Aturdimiento)**
- **Aplicación**: `pObject->OnStun(fDuration)`
- **Efecto**: Inmoviliza al personaje
- **XML**: `mod.root="true"`
- **Implementación**: En `ZCharacter::OnStun()` y `ZMyCharacter::OnStun()`

```cpp
// En ZSkill.cpp línea 635
if (m_pDesc->bModRoot) {
    float fDuration = m_pDesc->nEffectTime * 0.001f;
    pObject->OnStun(fDuration);
    // También reduce velocidad a 0
    ZModule_Movable *pMovableModule = (ZModule_Movable*)pObject->GetModule(ZMID_MOVABLE);
    if (pMovableModule)
        pMovableModule->SetMoveSpeedRatio(0, fDuration);
}
```

#### 2. **Slow (Ralentización)**
- **Aplicación**: `pMovableModule->SetMoveSpeedRatio(fRatio, fDuration)`
- **Efecto**: Reduce velocidad de movimiento
- **XML**: `mod.speed="50"` (50% de velocidad)
- **Implementación**: `ZModule_Movable::SetMoveSpeedRatio()`

```cpp
// En ZSkill.cpp línea 657
if (m_pDesc->nModSpeed < 100) {
    ZModule_Movable *pMovableModule = (ZModule_Movable*)pTargetObject->GetModule(ZMID_MOVABLE);
    if (pMovableModule)
        pMovableModule->SetMoveSpeedRatio(m_pDesc->nModSpeed*0.01f, m_pDesc->nEffectTime*0.001f);
}
```

#### 3. **Elemental Damage (Daño Elemental)**
- **Módulos**: `ZModule_FireDamage`, `ZModule_ColdDamage`, `ZModule_PoisonDamage`, `ZModule_LightningDamage`
- **Aplicación**: Automática cuando se recibe daño elemental
- **Efecto**: Daño continuo con efectos visuales

```cpp
// Los módulos elementales se activan automáticamente
// Ejemplo: ZModule_FireDamage se activa con BeginDamage()
ZModule_FireDamage *pMod = (ZModule_FireDamage*)pTarget->GetModule(ZMID_FIREDAMAGE);
if(pMod) 
    pMod->BeginDamage(pOwnerCharacter->GetUID(), damage, duration);
```

#### 4. **Knockback (Empuje)**
- **Aplicación**: `pObject->AddVelocity(dir * force)`
- **Efecto**: Empuja al objetivo
- **XML**: `knockback="100"`

```cpp
// En ZSkill.cpp línea 641
if (m_pDesc->fModKnockback != 0.0f) {
    rvector dir = pObject->GetPosition() - (targetPos + rvector(0, 0, 80));
    Normalize(dir);
    pObject->AddVelocity(m_pDesc->fModKnockback * 7.f * -dir);
}
```

#### 5. **Heal/Repair (Curación/Reparación)**
- **Aplicación**: `pObject->OnHealing(pOwner, nHP, nAP)`
- **Efecto**: Restaura HP/AP
- **XML**: `mod.heal="50"`, `mod.repair="30"`

---

## 🆕 Crear Nuevos Módulos de Estado

### Estructura de un Nuevo Módulo

#### 1. Crear el Header (`ZModule_NuevoEstado.h`)

```cpp
#pragma once

#include "ZModule.h"
#include "ZModuleID.h"

class ZModule_NuevoEstado : public ZModule {
private:
    float m_fDuration;
    float m_fStartTime;
    int m_nIntensity;  // Intensidad del estado
    
public:
    DECLARE_ID(ZMID_NUEVO_ESTADO)  // Definir en ZModuleID.h
    
    ZModule_NuevoEstado();
    virtual ~ZModule_NuevoEstado();
    
    virtual void OnAdd() override;
    virtual void OnUpdate(float fElapsed) override;
    virtual void InitStatus() override;
    
    // Métodos públicos para aplicar el estado
    void ApplyState(int nIntensity, float fDuration);
    bool IsActive() const { return Active && (g_pGame->GetTime() < m_fStartTime + m_fDuration); }
    int GetIntensity() const { return m_nIntensity; }
    
    // Métodos específicos del estado
    void OnStateTick();  // Se llama cada frame mientras está activo
};
```

#### 2. Crear la Implementación (`ZModule_NuevoEstado.cpp`)

```cpp
#include "stdafx.h"
#include "ZModule_NuevoEstado.h"
#include "ZGame.h"
#include "ZObject.h"

ZModule_NuevoEstado::ZModule_NuevoEstado()
    : m_fDuration(0.0f), m_fStartTime(0.0f), m_nIntensity(0)
{
}

ZModule_NuevoEstado::~ZModule_NuevoEstado()
{
}

void ZModule_NuevoEstado::OnAdd()
{
    // Verificar que el contenedor sea válido
    if (!m_pContainer)
        return;
    
    _ASSERT(MIsDerivedFromClass(ZObject, m_pContainer));
}

void ZModule_NuevoEstado::InitStatus()
{
    m_fDuration = 0.0f;
    m_fStartTime = 0.0f;
    m_nIntensity = 0;
    Active = false;
}

void ZModule_NuevoEstado::OnUpdate(float fElapsed)
{
    if (!Active) return;
    if (!m_pContainer) return;
    
    // Verificar si el estado expiró
    if (g_pGame->GetTime() >= m_fStartTime + m_fDuration) {
        Active = false;
        InitStatus();  // Resetear estado
        return;
    }
    
    // Aplicar efecto cada frame
    OnStateTick();
}

void ZModule_NuevoEstado::ApplyState(int nIntensity, float fDuration)
{
    if (!m_pContainer) return;
    
    m_nIntensity = nIntensity;
    m_fDuration = fDuration;
    m_fStartTime = g_pGame->GetTime();
    Active = true;
    
    // Aplicar efecto inicial
    ZObject* pObj = MStaticCast(ZObject, m_pContainer);
    if (pObj) {
        // Ejemplo: Aplicar efecto visual
        // ZGetEffectManager()->AddEffect(...);
    }
}

void ZModule_NuevoEstado::OnStateTick()
{
    if (!m_pContainer) return;
    
    ZObject* pObj = MStaticCast(ZObject, m_pContainer);
    if (!pObj || pObj->IsDead()) {
        Active = false;
        return;
    }
    
    // Aplicar efecto continuo
    // Ejemplo: Reducir velocidad, aplicar daño, etc.
    ZModule_Movable* pMovable = static_cast<ZModule_Movable*>(m_pContainer->GetModule(ZMID_MOVABLE));
    if (pMovable) {
        // Aplicar efecto según intensidad
        float fSpeedRatio = 1.0f - (m_nIntensity * 0.1f);  // Reducir velocidad
        pMovable->SetMoveSpeedRatio(fSpeedRatio, 0.1f);
    }
}
```

#### 3. Registrar el Módulo en `ZModuleID.h`

```cpp
#define ZMID_NUEVO_ESTADO    25  // Usar un ID único
```

#### 4. Agregar Declaración en `ZModule.h`

```cpp
class ZModule_NuevoEstado;
```

#### 5. Agregar el Módulo a `ZCharacterObject` (si es necesario)

```cpp
// En ZCharacterObject.h
protected:
    ZModule_NuevoEstado* m_pModule_NuevoEstado{};

// En ZCharacterObject.cpp constructor
ADD_MODULE(NuevoEstado);
```

#### 6. Usar el Módulo en Skills

```cpp
// En ZSkill.cpp, en Execute() o Use()
ZModule_NuevoEstado* pMod = static_cast<ZModule_NuevoEstado*>(pObject->GetModule(ZMID_NUEVO_ESTADO));
if (pMod) {
    pMod->ApplyState(5, 5.0f);  // Intensidad 5, duración 5 segundos
}
```

---

## 💡 Ejemplos Prácticos

### Ejemplo 1: Skill de Confusión (Nuevo Estado)

#### 1. Crear Módulo `ZModule_Confusion`

```cpp
// ZModule_Confusion.h
#pragma once
#include "ZModule.h"
#include "ZModuleID.h"

class ZModule_Confusion : public ZModule {
private:
    float m_fDuration;
    float m_fStartTime;
    float m_fConfusionStrength;  // 0.0-1.0, fuerza de confusión
    
public:
    DECLARE_ID(ZMID_CONFUSION)
    
    ZModule_Confusion();
    virtual void OnUpdate(float fElapsed) override;
    virtual void InitStatus() override;
    
    void ApplyConfusion(float fStrength, float fDuration);
    bool IsConfused() const;
};
```

```cpp
// ZModule_Confusion.cpp
#include "stdafx.h"
#include "ZModule_Confusion.h"
#include "ZGame.h"
#include "ZObject.h"
#include "ZModule_Movable.h"

ZModule_Confusion::ZModule_Confusion()
    : m_fDuration(0.0f), m_fStartTime(0.0f), m_fConfusionStrength(0.0f)
{
}

void ZModule_Confusion::InitStatus()
{
    m_fDuration = 0.0f;
    m_fStartTime = 0.0f;
    m_fConfusionStrength = 0.0f;
    Active = false;
}

void ZModule_Confusion::OnUpdate(float fElapsed)
{
    if (!Active || !m_pContainer) return;
    
    if (g_pGame->GetTime() >= m_fStartTime + m_fDuration) {
        Active = false;
        InitStatus();
        return;
    }
    
    // Aplicar confusión: invertir dirección ocasionalmente
    ZObject* pObj = MStaticCast(ZObject, m_pContainer);
    if (pObj && !pObj->IsDead()) {
        float fRandom = (float)(rand() % 100) / 100.0f;
        if (fRandom < m_fConfusionStrength * 0.1f) {  // 10% chance por frame
            // Invertir dirección
            rvector dir = pObj->GetDirection();
            dir.x = -dir.x;
            dir.y = -dir.y;
            pObj->SetDirection(dir);
        }
    }
}

void ZModule_Confusion::ApplyConfusion(float fStrength, float fDuration)
{
    if (!m_pContainer) return;
    
    m_fConfusionStrength = fStrength;
    m_fDuration = fDuration;
    m_fStartTime = g_pGame->GetTime();
    Active = true;
}

bool ZModule_Confusion::IsConfused() const
{
    return Active && (g_pGame->GetTime() < m_fStartTime + m_fDuration);
}
```

#### 2. Agregar Skill en XML

```xml
<SKILL id="502" name="Confusion Spell" 
    effecttype="1" hitcheck="false" guidable="false" 
    delay="3000" lifetime="0" 
    effectstarttime="500" effecttime="5000" effectarea="3"
    mod.damage="0" mod.speed="100" mod.root="false" 
    resisttype="0" difficulty="25"
    castinganimation="2" castingeffect="ef_confusion" 
    castingeffectType="2" />
```

#### 3. Aplicar en `ZSkill::Execute()`

```cpp
// Agregar después de línea 639 en ZSkill.cpp
// Aplicar confusión (nuevo estado personalizado)
ZModule_Confusion* pConfusionMod = static_cast<ZModule_Confusion*>(pObject->GetModule(ZMID_CONFUSION));
if (pConfusionMod) {
    float fDuration = m_pDesc->nEffectTime * 0.001f;
    pConfusionMod->ApplyConfusion(0.5f, fDuration);  // 50% de fuerza, duración del skill
}
```

### Ejemplo 2: Skill de Regeneración (Buff)

#### 1. Crear Módulo `ZModule_Regeneration`

```cpp
// ZModule_Regeneration.h
#pragma once
#include "ZModule.h"
#include "ZModuleID.h"

class ZModule_Regeneration : public ZModule {
private:
    float m_fDuration;
    float m_fStartTime;
    float m_fNextHealTime;
    int m_nHealPerSecond;
    
public:
    DECLARE_ID(ZMID_REGENERATION)
    
    ZModule_Regeneration();
    virtual void OnUpdate(float fElapsed) override;
    virtual void InitStatus() override;
    
    void StartRegeneration(int nHealPerSecond, float fDuration);
};
```

```cpp
// ZModule_Regeneration.cpp
#include "stdafx.h"
#include "ZModule_Regeneration.h"
#include "ZGame.h"
#include "ZObject.h"
#include "ZModule_HPAP.h"

ZModule_Regeneration::ZModule_Regeneration()
    : m_fDuration(0.0f), m_fStartTime(0.0f), m_fNextHealTime(0.0f), m_nHealPerSecond(0)
{
}

void ZModule_Regeneration::InitStatus()
{
    m_fDuration = 0.0f;
    m_fStartTime = 0.0f;
    m_fNextHealTime = 0.0f;
    m_nHealPerSecond = 0;
    Active = false;
}

void ZModule_Regeneration::OnUpdate(float fElapsed)
{
    if (!Active || !m_pContainer) return;
    
    ZObject* pObj = MStaticCast(ZObject, m_pContainer);
    if (!pObj || pObj->IsDead()) {
        Active = false;
        return;
    }
    
    // Verificar si expiró
    if (g_pGame->GetTime() >= m_fStartTime + m_fDuration) {
        Active = false;
        InitStatus();
        return;
    }
    
    // Curar cada segundo
    if (g_pGame->GetTime() >= m_fNextHealTime) {
        m_fNextHealTime = g_pGame->GetTime() + 1.0f;
        
        ZModule_HPAP* pHPAP = static_cast<ZModule_HPAP*>(m_pContainer->GetModule(ZMID_HPAP));
        if (pHPAP) {
            pObj->OnHealing(pObj, m_nHealPerSecond, 0);
        }
    }
}

void ZModule_Regeneration::StartRegeneration(int nHealPerSecond, float fDuration)
{
    if (!m_pContainer) return;
    
    m_nHealPerSecond = nHealPerSecond;
    m_fDuration = fDuration;
    m_fStartTime = g_pGame->GetTime();
    m_fNextHealTime = m_fStartTime + 1.0f;
    Active = true;
}
```

#### 2. Skill XML

```xml
<SKILL id="503" name="Regeneration" 
    effecttype="6" hitcheck="false" guidable="false" 
    delay="2000" lifetime="0" 
    effectstarttime="0" effecttime="10000" effectarea="2"
    mod.damage="0" mod.heal="0" mod.speed="100" 
    resisttype="0" difficulty="0"
    castingpreeffect="ef_healing" castingpreeffectType="4" />
```

### Ejemplo 3: Skill de Invisibilidad

#### 1. Crear Módulo `ZModule_Invisibility`

```cpp
// ZModule_Invisibility.h
#pragma once
#include "ZModule.h"
#include "ZModuleID.h"

class ZModule_Invisibility : public ZModule {
private:
    float m_fDuration;
    float m_fStartTime;
    float m_fOpacity;  // 0.0 = invisible, 1.0 = visible
    
public:
    DECLARE_ID(ZMID_INVISIBILITY)
    
    ZModule_Invisibility();
    virtual void OnUpdate(float fElapsed) override;
    virtual void InitStatus() override;
    
    void ApplyInvisibility(float fOpacity, float fDuration);
    bool IsInvisible() const;
};
```

```cpp
// ZModule_Invisibility.cpp
#include "stdafx.h"
#include "ZModule_Invisibility.h"
#include "ZGame.h"
#include "ZObject.h"

ZModule_Invisibility::ZModule_Invisibility()
    : m_fDuration(0.0f), m_fStartTime(0.0f), m_fOpacity(1.0f)
{
}

void ZModule_Invisibility::InitStatus()
{
    m_fDuration = 0.0f;
    m_fStartTime = 0.0f;
    m_fOpacity = 1.0f;
    Active = false;
}

void ZModule_Invisibility::OnUpdate(float fElapsed)
{
    if (!Active || !m_pContainer) return;
    
    ZObject* pObj = MStaticCast(ZObject, m_pContainer);
    if (!pObj || pObj->IsDead()) {
        Active = false;
        InitStatus();
        if (pObj && pObj->m_pVMesh) {
            pObj->m_pVMesh->SetVisibility(1.0f);  // Restaurar visibilidad
        }
        return;
    }
    
    // Verificar si expiró
    if (g_pGame->GetTime() >= m_fStartTime + m_fDuration) {
        Active = false;
        if (pObj->m_pVMesh) {
            pObj->m_pVMesh->SetVisibility(1.0f);  // Restaurar visibilidad
        }
        InitStatus();
        return;
    }
    
    // Aplicar opacidad
    if (pObj->m_pVMesh) {
        pObj->m_pVMesh->SetVisibility(m_fOpacity);
    }
}

void ZModule_Invisibility::ApplyInvisibility(float fOpacity, float fDuration)
{
    if (!m_pContainer) return;
    
    m_fOpacity = fOpacity;
    m_fDuration = fDuration;
    m_fStartTime = g_pGame->GetTime();
    Active = true;
}

bool ZModule_Invisibility::IsInvisible() const
{
    return Active && (g_pGame->GetTime() < m_fStartTime + m_fDuration) && (m_fOpacity < 0.5f);
}
```

---

## 📝 Resumen de Estados Disponibles

### Estados Implementados Actualmente:
1. ✅ **Stun** - `OnStun()` + `SetMoveSpeedRatio(0, duration)`
2. ✅ **Slow** - `SetMoveSpeedRatio(ratio, duration)`
3. ✅ **Fire Damage** - `ZModule_FireDamage::BeginDamage()`
4. ✅ **Cold Damage** - `ZModule_ColdDamage::BeginDamage()`
5. ✅ **Poison Damage** - `ZModule_PoisonDamage::BeginDamage()`
6. ✅ **Lightning Damage** - `ZModule_LightningDamage::BeginDamage()`
7. ✅ **Knockback** - `AddVelocity()`
8. ✅ **Heal/Repair** - `OnHealing()`

### Estados que se Pueden Crear:
- ✅ **Confusion** - Invertir controles
- ✅ **Regeneration** - Curación continua
- ✅ **Invisibility** - Reducir opacidad
- ✅ **Berserk** - Aumentar daño, reducir defensa
- ✅ **Shield** - Reducir daño recibido
- ✅ **Haste** - Aumentar velocidad
- ✅ **Freeze** - Congelar completamente
- ✅ **Silence** - Bloquear skills
- ✅ **Blind** - Reducir rango de visión
- ✅ **Curse** - Reducir todas las estadísticas

---

## 🔧 Pasos para Implementar un Nuevo Estado

1. **Crear archivos del módulo**:
   - `ZModule_NuevoEstado.h`
   - `ZModule_NuevoEstado.cpp`

2. **Registrar ID en `ZModuleID.h`**:
   ```cpp
   #define ZMID_NUEVO_ESTADO    XX
   ```

3. **Agregar declaración en `ZModule.h`**:
   ```cpp
   class ZModule_NuevoEstado;
   ```

4. **Agregar a `ZCharacterObject`** (si es necesario):
   - En constructor: `AddModule<ZModule_NuevoEstado>()`
   - En header: `ZModule_NuevoEstado* m_pModule_NuevoEstado{};`

5. **Implementar lógica en `OnUpdate()`**:
   - Verificar duración
   - Aplicar efectos cada frame
   - Limpiar al terminar

6. **Usar en Skills**:
   - En `ZSkill::Execute()` o `ZSkill::Use()`
   - Obtener módulo y aplicar estado

7. **Agregar al XML** (opcional):
   - Si quieres que se aplique automáticamente desde XML

---

## ⚠️ Notas Importantes

1. **IDs de Módulos**: Deben ser únicos. Revisar `ZModuleID.h` para IDs disponibles
2. **Limpieza**: Siempre limpiar estados en `InitStatus()` y cuando expiran
3. **Verificaciones**: Siempre verificar `m_pContainer` y objetos NULL
4. **Rendimiento**: Los módulos se actualizan cada frame, mantener lógica ligera
5. **Persistencia**: Los estados no persisten entre rounds (se resetean en `InitStatus()`)

---

## 🎮 Ejemplo Completo: Skill de Veneno con Múltiples Objetivos

```xml
<!-- Skill que crea 3 proyectiles de veneno que persiguen diferentes objetivos -->
<SKILL id="504" name="Poison Swarm" 
    effecttype="1" hitcheck="true" guidable="true" 
    velocity="2500" delay="4000" lifetime="0" 
    difficulty="20" colradius="20" knockback="5" 
    effectstarttime="300" effectarea="4"
    mod.damage="15" mod.dot="8" mod.criticalrate="25" 
    mod.speed="100" mod.root="false" mod.heal="0"
    resisttype="4" 
    castinganimation="2" castingeffectType="3"
    traileffect="ef_poisonball2" traileffecttype="1" 
    traileffectscale="1.5" sound.explosion="we_enpoison">
    <REPEAT delay="0.0" angle="0.0 0.0 0.25" />
    <REPEAT delay="0.0" angle="0.0 0.0 0.0" />
    <REPEAT delay="0.0" angle="0.0 0.0 -0.25" />
</SKILL>
```

Este skill crea 3 proyectiles que persiguen objetivos (gracias a `guidable="true"` y `REPEAT` con diferentes ángulos).

