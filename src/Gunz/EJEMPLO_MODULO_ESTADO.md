# Ejemplo Práctico: Módulo de Estado Personalizado

## Ejemplo: Módulo de Berserk (Aumenta Daño, Reduce Defensa)

Este ejemplo muestra cómo crear un módulo de estado completo desde cero.

### Paso 1: Crear ZModule_Berserk.h

```cpp
#pragma once

#include "ZModule.h"
#include "ZModuleID.h"

class ZModule_Berserk : public ZModule {
private:
    float m_fDuration;
    float m_fStartTime;
    float m_fDamageMultiplier;    // Multiplicador de daño (ej: 1.5 = 50% más daño)
    float m_fDefenseReduction;    // Reducción de defensa (ej: 0.5 = 50% menos defensa)
    
public:
    DECLARE_ID(ZMID_BERSERK)
    
    ZModule_Berserk();
    virtual ~ZModule_Berserk();
    
    virtual void OnAdd() override;
    virtual void OnUpdate(float fElapsed) override;
    virtual void InitStatus() override;
    
    // Aplicar estado berserk
    void ApplyBerserk(float fDamageMultiplier, float fDefenseReduction, float fDuration);
    
    // Verificar si está activo
    bool IsBerserk() const;
    
    // Obtener multiplicadores
    float GetDamageMultiplier() const { return m_fDamageMultiplier; }
    float GetDefenseReduction() const { return m_fDefenseReduction; }
};
```

### Paso 2: Crear ZModule_Berserk.cpp

```cpp
#include "stdafx.h"
#include "ZModule_Berserk.h"
#include "ZGame.h"
#include "ZObject.h"
#include "ZModule_Resistance.h"
#include "ZEffectManager.h"

ZModule_Berserk::ZModule_Berserk()
    : m_fDuration(0.0f), m_fStartTime(0.0f), 
      m_fDamageMultiplier(1.0f), m_fDefenseReduction(1.0f)
{
}

ZModule_Berserk::~ZModule_Berserk()
{
}

void ZModule_Berserk::OnAdd()
{
    if (!m_pContainer)
        return;
    
    _ASSERT(MIsDerivedFromClass(ZObject, m_pContainer));
}

void ZModule_Berserk::InitStatus()
{
    m_fDuration = 0.0f;
    m_fStartTime = 0.0f;
    m_fDamageMultiplier = 1.0f;
    m_fDefenseReduction = 1.0f;
    Active = false;
}

void ZModule_Berserk::OnUpdate(float fElapsed)
{
    if (!Active || !m_pContainer) return;
    
    ZObject* pObj = MStaticCast(ZObject, m_pContainer);
    if (!pObj || pObj->IsDead()) {
        Active = false;
        InitStatus();
        return;
    }
    
    // Verificar si expiró
    if (g_pGame->GetTime() >= m_fStartTime + m_fDuration) {
        Active = false;
        InitStatus();
        
        // Restaurar resistencia normal
        ZModule_Resistance* pResist = static_cast<ZModule_Resistance*>(m_pContainer->GetModule(ZMID_RESISTANCE));
        if (pResist) {
            // Restaurar valores (se hace automáticamente en InitStatus del módulo)
        }
        
        return;
    }
    
    // Aplicar efecto visual continuo (opcional)
    // ZGetEffectManager()->AddEffect(...);
}

void ZModule_Berserk::ApplyBerserk(float fDamageMultiplier, float fDefenseReduction, float fDuration)
{
    if (!m_pContainer) return;
    
    m_fDamageMultiplier = fDamageMultiplier;
    m_fDefenseReduction = fDefenseReduction;
    m_fDuration = fDuration;
    m_fStartTime = g_pGame->GetTime();
    Active = true;
    
    // Aplicar reducción de resistencia
    ZModule_Resistance* pResist = static_cast<ZModule_Resistance*>(m_pContainer->GetModule(ZMID_RESISTANCE));
    if (pResist) {
        // Reducir todas las resistencias
        // Nota: Esto requeriría modificar ZModule_Resistance para permitir modificadores temporales
    }
    
    // Efecto visual inicial
    ZObject* pObj = MStaticCast(ZObject, m_pContainer);
    if (pObj) {
        // ZGetEffectManager()->AddBerserkEffect(pObj);
    }
}

bool ZModule_Berserk::IsBerserk() const
{
    return Active && (g_pGame->GetTime() < m_fStartTime + m_fDuration);
}
```

### Paso 3: Registrar en ZModuleID.h

```cpp
#define ZMID_BERSERK    26  // Agregar después de ZMID_SKILLS
```

### Paso 4: Agregar a ZModule.h

```cpp
class ZModule_Berserk;
```

### Paso 5: Usar en ZSkill.cpp

Agregar en `ZSkill::Execute()` después de línea 639:

```cpp
// Aplicar Berserk (nuevo estado)
// Nota: Esto requeriría agregar campos al XML como mod.berserk="true" mod.berserkdamage="1.5"
// Por ahora, lo aplicamos manualmente para skills específicos
if (m_pDesc->nID == 505) {  // ID del skill de berserk
    ZModule_Berserk* pBerserkMod = static_cast<ZModule_Berserk*>(pObject->GetModule(ZMID_BERSERK));
    if (pBerserkMod) {
        float fDuration = m_pDesc->nEffectTime * 0.001f;
        pBerserkMod->ApplyBerserk(1.5f, 0.5f, fDuration);  // 50% más daño, 50% menos defensa
    }
}
```

### Paso 6: Modificar ZModule_Resistance para Soporte de Modificadores Temporales

Para que Berserk funcione completamente, necesitarías modificar `ZModule_Resistance`:

```cpp
// En ZModule_Resistance.h, agregar:
private:
    float m_fTemporaryDefenseMultiplier;  // Multiplicador temporal (ej: 0.5 = 50% menos defensa)

public:
    void SetTemporaryDefenseMultiplier(float fMultiplier, float fDuration);
    float GetEffectiveResistance(int nResistType) const;  // Retorna resistencia con modificadores
```

---

## Uso de Estados Existentes en Skills

### Ejemplo: Combinar Múltiples Estados

```xml
<!-- Skill que aplica Slow + Poison + Stun -->
<SKILL id="505" name="Crippling Poison" 
    effecttype="1" hitcheck="false" guidable="false" 
    delay="3000" lifetime="0" 
    effectstarttime="500" effecttime="8000" effectarea="2"
    mod.damage="10" mod.dot="5" mod.speed="30" 
    mod.root="true" mod.heal="0"
    resisttype="4" difficulty="25"
    castinganimation="2" castingeffect="ef_poisonstorm" 
    castingeffectType="2" />
```

Este skill aplica:
- **Slow**: `mod.speed="30"` → 30% de velocidad
- **Poison DoT**: `mod.dot="5"` → 5 daño por segundo
- **Stun**: `mod.root="true"` → Inmoviliza

### Ejemplo: Skill de Curación con Regeneración

```xml
<!-- Skill que cura instantáneamente + regeneración -->
<SKILL id="506" name="Greater Heal" 
    effecttype="6" hitcheck="false" guidable="false" 
    delay="5000" lifetime="0" 
    effectstarttime="0" effecttime="10000" effectarea="3"
    mod.damage="0" mod.heal="100" mod.repair="50" 
    mod.speed="100" mod.root="false"
    resisttype="0" difficulty="0"
    castingpreeffect="ef_healing" castingpreeffectType="4" />
```

Este skill:
- Cura 100 HP instantáneamente
- Repara 50 AP
- Si tienes `ZModule_Regeneration`, puedes agregar regeneración continua

---

## Notas de Implementación

### Para Efectos que Persiguen Múltiples Objetivos

El sistema actual usa `REPEAT` con diferentes ángulos. Para efectos que persiguen **objetivos específicos diferentes**, necesitarías:

1. **Modificar `ZWeaponMagic`** para soportar múltiples objetivos:
   - Cambiar `MUID m_uidTarget` a `std::vector<MUID> m_vTargets`
   - En `Update()`, perseguir el objetivo más cercano o dividir en múltiples instancias

2. **O crear múltiples instancias del skill**:
   - Usar `REPEAT` con diferentes ángulos
   - Cada proyectil persigue el objetivo más cercano en esa dirección

### Límites Actuales

- **REPEAT máximo**: 10 (configurado en optimizaciones)
- **Objetos procesados**: 20-30 por skill
- **Efectos especiales**: 5 máximo por skill

Estos límites se pueden ajustar en `ZSkill.cpp` si es necesario.

