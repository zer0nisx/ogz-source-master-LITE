# Problemas de Rendimiento con Skills de NPCs

## Problemas Identificados

### 1. ⚠️ Iteración Costosa en `ZSkill::Update()` (Línea 380)
**Ubicación**: `ZSkill.cpp:380`
**Problema**: 
- Itera sobre **TODOS** los objetos en `ZObjectManager` cada segundo (DAMAGE_DELAY = 1.0f)
- Esto se ejecuta mientras el skill está activo (puede durar varios segundos)
- Si hay muchos NPCs u objetos, esto causa lag significativo

**Código problemático**:
```cpp
if(g_pGame->GetTime()>m_fNextDamageTime) {
    m_fNextDamageTime+=DAMAGE_DELAY;
    
    for(ZObjectManager::iterator i = ZGetObjectManager()->begin();i!=ZGetObjectManager()->end();i++) {
        ZObject *pTarget = i->second;
        // ... procesamiento ...
    }
}
```

**Solución**: 
- Cachear objetos válidos al inicio del skill
- Usar spatial partitioning o filtros más eficientes
- Limitar el número de objetos a procesar

### 2. ⚠️ Iteración Costosa en `ZSkill::Execute()` (Línea 544)
**Ubicación**: `ZSkill.cpp:544`
**Problema**:
- Itera sobre **TODOS** los objetos cuando se ejecuta el skill
- Se ejecuta inmediatamente, causando un spike de FPS

**Código problemático**:
```cpp
for ( ZObjectManager::iterator i = ZGetObjectManager()->begin();  i!=ZGetObjectManager()->end();  i++)
{
    ZObject *pObject = i->second;
    // ... procesamiento para cada objeto ...
}
```

**Solución**: Similar a #1

### 3. ⚠️ Efectos Visuales No Verificados
**Ubicación**: `ZSkill.cpp:613-615`
**Problema**:
- Se usa `pTargetObject` sin verificar si es válido después de obtenerlo
- Si el objeto fue eliminado, puede causar crash o efectos visuales corruptos

**Código problemático**:
```cpp
ZObject *pTargetObject = ZGetObjectManager()->GetObject( uidTarget);
if (pTargetObject == NULL)
    return;  // Sale temprano, pero luego se usa pTargetObject en línea 613

// ... más código ...

if(type != eq_parts_pos_info_etc)
    ZGetEffectManager()->AddPartsPosType(m_pDesc->szCastingEffect, pTargetObject->GetUID(),type,m_pDesc->nEffectTime);
```

**Solución**: Verificar `pTargetObject` antes de usarlo en todas las líneas

### 4. ⚠️ Módulo de Skills No Se Desactiva Correctamente
**Ubicación**: `ZModule_Skills.cpp:55`
**Problema**:
- `Active = true` se establece en `Excute()`, pero puede que no se desactive cuando el skill termina
- Esto mantiene el módulo actualizándose innecesariamente

**Código problemático**:
```cpp
void ZModule_Skills::Excute(int nSkill,MUID uidTarget,rvector targetPosition)
{
    if(nSkill<0 || nSkill>=MAX_SKILL) return;
    m_Skills[nSkill].Execute(uidTarget,targetPosition);
    
    Active = true;  // Se activa, pero ¿se desactiva?
}
```

**Solución**: Desactivar el módulo cuando todos los skills terminen

### 5. ⚠️ Efectos Especiales (AddSp) Sin Límite
**Ubicación**: `ZSkill.cpp:618-621`
**Problema**:
- `AddSp()` crea múltiples efectos especiales (`nCastingEffectSpCount`)
- Si se llama múltiples veces (por ejemplo, en Repeat()), puede crear muchos efectos
- No hay verificación de límite máximo de efectos

**Código problemático**:
```cpp
if(m_pDesc->szCastingEffectSp[0]) {
    ZGetEffectManager()->AddSp(m_pDesc->szCastingEffectSp,m_pDesc->nCastingEffectSpCount,
        vPos,vDir,m_pOwner->GetUID());
}
```

**Solución**: Limitar el número de efectos especiales o verificar si ya existen

### 6. ⚠️ Verificación NULL Faltante en `ZSkill::Use()`
**Ubicación**: `ZSkill.cpp:508-622`
**Problema**:
- Se verifica `pTargetObject == NULL` y se retorna, pero luego se usa en múltiples lugares sin verificar

**Solución**: Agregar verificaciones NULL antes de usar `pTargetObject`

## Recomendaciones de Corrección

### Corrección Crítica #1: Optimizar Iteraciones
```cpp
// En ZSkill::Update() y ZSkill::Execute()
// En lugar de iterar todos los objetos, usar un filtro espacial o cache
// Por ejemplo, solo procesar objetos dentro del área de efecto
```

### Corrección Crítica #2: Verificaciones NULL
```cpp
// En ZSkill::Use(), verificar pTargetObject antes de cada uso
if (!pTargetObject) return;
// ... usar pTargetObject ...
```

### Corrección Crítica #3: Desactivar Módulo Correctamente
```cpp
// En ZSkill::Update(), cuando el skill termina:
if(g_pGame->GetTime()-m_fLastBeginTime > m_pDesc->nEffectTime*0.001f) {
    m_bEnable = false;
    // Notificar al módulo que se desactive si no hay más skills activos
    return false;
}
```

### Corrección Crítica #4: Limitar Efectos Visuales
```cpp
// Agregar límite máximo de efectos especiales
const int MAX_SP_EFFECTS = 10;
if(m_pDesc->szCastingEffectSp[0] && m_nSpEffectCount < MAX_SP_EFFECTS) {
    ZGetEffectManager()->AddSp(...);
    m_nSpEffectCount++;
}
```

## Prioridad de Corrección

1. **ALTA**: Problema #1 y #2 (iteraciones costosas) - Causa principal del lag
2. **ALTA**: Problema #3 (verificaciones NULL) - Puede causar crashes
3. **MEDIA**: Problema #4 (desactivación de módulo) - Afecta rendimiento general
4. **BAJA**: Problema #5 y #6 (efectos sin límite) - Puede causar acumulación de efectos

