# Análisis de Usos Críticos de g_pGame

## Resumen Ejecutivo

Se han identificado **múltiples usos de `g_pGame` sin verificación de null** que pueden causar crashes. Este documento analiza los casos más críticos y las correcciones aplicadas.

---

## Problema General

`g_pGame` es una variable global que puede ser `NULL` en varios escenarios:
- Durante la inicialización del juego
- Durante la destrucción del juego
- En estados de transición (menús, carga, etc.)
- En contextos donde el juego no está activo

**Acceso directo sin verificación:**
```cpp
// ❌ PELIGROSO - Puede causar crash
g_pGame->GetTime();
g_pGame->OnExplosionMagic(...);
```

**Acceso seguro:**
```cpp
// ✅ SEGURO - Verifica null antes de usar
if (g_pGame) {
    g_pGame->GetTime();
}
```

---

## Casos Críticos Corregidos

### 1. ZWeapon.cpp - Línea 1138

**Problema:**
```cpp
void ZWeaponMagic::Explosion(...)
{
    // ...
    g_pGame->OnExplosionMagic(...); // ❌ Sin verificación
}
```

**Corrección aplicada:**
```cpp
void ZWeaponMagic::Explosion(...)
{
    if (!g_pGame) return; // ✅ Validación de seguridad
    
    // ...
    g_pGame->OnExplosionMagic(...);
}
```

**Impacto:** CRÍTICO - Esta función se llama cuando una arma mágica explota. Si `g_pGame` es NULL, causará un crash inmediato.

---

### 2. ZWeapon.cpp - Línea 1133

**Problema:**
```cpp
g_pGame->OnExplosionMagicNonSplash(...); // ❌ Sin verificación
```

**Corrección aplicada:**
```cpp
if (!g_pGame) return; // ✅ Validación agregada al inicio de la función
// ...
g_pGame->OnExplosionMagicNonSplash(...);
```

**Impacto:** CRÍTICO - Similar al caso anterior, pero para explosiones sin splash.

---

### 3. ZActor.cpp - Líneas 123, 125

**Problema:**
```cpp
void ZActor::OnDraw()
{
    if (IsDieAnimationDone())
    {
        if (m_TempBackupTime == -1) 
            m_TempBackupTime = g_pGame->GetTime(); // ❌ Sin verificación
        
        float fOpacity = ... (g_pGame->GetTime() ...); // ❌ Sin verificación
    }
}
```

**Corrección aplicada:**
```cpp
void ZActor::OnDraw()
{
    if (IsDieAnimationDone())
    {
        if (!g_pGame) return; // ✅ Validación de seguridad
        
        if (m_TempBackupTime == -1) 
            m_TempBackupTime = g_pGame->GetTime();
        
        float fOpacity = ... (g_pGame->GetTime() ...);
    }
}
```

**Impacto:** ALTO - Esta función se llama cada frame para dibujar actores. Si `g_pGame` es NULL durante la destrucción, causará crash.

---

### 4. ZActor.cpp - Línea 348

**Problema:**
```cpp
void ZActor::InitMesh(...)
{
    // ...
    int nVMID = g_pGame->m_VisualMeshMgr.Add(pMesh); // ❌ Sin verificación
    RVisualMesh* pVMesh = g_pGame->m_VisualMeshMgr.GetFast(nVMID); // ❌ Sin verificación
}
```

**Corrección aplicada:**
```cpp
void ZActor::InitMesh(...)
{
    // ...
    if (!g_pGame) return; // ✅ Validación de seguridad
    
    int nVMID = g_pGame->m_VisualMeshMgr.Add(pMesh);
    RVisualMesh* pVMesh = g_pGame->m_VisualMeshMgr.GetFast(nVMID);
}
```

**Impacto:** ALTO - Se llama durante la inicialización de NPCs. Si el juego no está completamente inicializado, causará crash.

---

### 5. ZActor.cpp - Línea 520

**Problema:**
```cpp
m_fAddBlastVelTime = g_pGame->GetTime(); // ❌ Sin verificación
```

**Corrección aplicada:**
```cpp
if (!g_pGame) return; // ✅ Validación de seguridad
m_fAddBlastVelTime = g_pGame->GetTime();
```

**Impacto:** MEDIO - Se llama cuando un actor recibe daño de blast dagger.

---

### 6. ZActor.cpp - Línea 607

**Problema:**
```cpp
void ZActor::InputBasicInfo(...)
{
    // ...
    m_fLastBasicInfo = g_pGame->GetTime(); // ❌ Sin verificación
}
```

**Corrección aplicada:**
```cpp
void ZActor::InputBasicInfo(...)
{
    // ...
    if (g_pGame) // ✅ Validación de seguridad
        m_fLastBasicInfo = g_pGame->GetTime();
}
```

**Impacto:** MEDIO - Se llama cuando se recibe información básica del servidor.

---

### 7. ZActor.cpp - Línea 672

**Problema:**
```cpp
if (CheckFlag(AF_BLAST_DAGGER)) {
    float fTime = max((1.f - (g_pGame->GetTime() - m_fAddBlastVelTime) / ...), 0.0f); // ❌ Sin verificación
}
```

**Corrección aplicada:**
```cpp
if (CheckFlag(AF_BLAST_DAGGER)) {
    if (!g_pGame) return; // ✅ Validación de seguridad
    
    float fTime = max((1.f - (g_pGame->GetTime() - m_fAddBlastVelTime) / ...), 0.0f);
}
```

**Impacto:** MEDIO - Se llama durante el update del actor cuando tiene el flag de blast dagger.

---

### 8. ZActor.cpp - Línea 832

**Problema:**
```cpp
void ZActor::Attack_Range(...)
{
    // ...
    ZPostNPCRangeShot(GetUID(), g_pGame->GetTime(), ...); // ❌ Sin verificación
}
```

**Corrección aplicada:**
```cpp
void ZActor::Attack_Range(...)
{
    if (!g_pGame) return; // ✅ Validación de seguridad
    
    // ...
    ZPostNPCRangeShot(GetUID(), g_pGame->GetTime(), ...);
}
```

**Impacto:** MEDIO - Se llama cuando un NPC ataca a distancia.

---

### 9. ZActor.cpp - Línea 924

**Problema:**
```cpp
if (pAttacker)
{
    bMyKill = (pAttacker == g_pGame->m_pMyCharacter); // ❌ Sin verificación
}
```

**Corrección aplicada:**
```cpp
if (pAttacker && g_pGame) // ✅ Validación de seguridad
{
    bMyKill = (pAttacker == g_pGame->m_pMyCharacter);
}
```

**Impacto:** BAJO - Se usa para determinar si el jugador hizo el kill, solo afecta el sonido.

---

## Casos Pendientes (No Críticos)

### ZCharacter.cpp - Línea 727

**Estado:** ✅ SEGURO
```cpp
m_pVMesh->SetClothValue(g_pGame != nullptr, fabs(dist));
```
Este caso ya tiene verificación explícita (`g_pGame != nullptr`), por lo que es seguro.

---

## Estadísticas

### Correcciones Aplicadas
- **Total de correcciones:** 9 casos críticos
- **Archivos modificados:** 2 (ZWeapon.cpp, ZActor.cpp)
- **Líneas corregidas:** ~15

### Casos Restantes
- **Total de usos de `g_pGame->`:** ~260+ (según grep)
- **Casos con verificación:** ~150+ (estimado)
- **Casos sin verificación:** ~110+ (estimado)

---

## Recomendaciones

### 1. Migración Gradual a ZGetGame()

Para código nuevo, usar `ZGetGame()` en lugar de `g_pGame`:
```cpp
// ✅ RECOMENDADO
ZGame* pGame = ZGetGame();
if (pGame) {
    pGame->GetTime();
}
```

### 2. Verificación Consistente

Para código existente que usa `g_pGame`, siempre verificar:
```cpp
// ✅ CORRECTO
if (g_pGame) {
    g_pGame->GetTime();
}
```

### 3. Priorización

**Prioridad ALTA:**
- Funciones llamadas cada frame (OnDraw, OnUpdate)
- Funciones de inicialización/destrucción
- Funciones de eventos críticos (explosiones, daño)

**Prioridad MEDIA:**
- Funciones de actualización periódica
- Funciones de input

**Prioridad BAJA:**
- Funciones de utilidad
- Funciones que solo afectan efectos visuales/sonido

---

## Conclusión

Se han corregido **9 casos críticos** de acceso a `g_pGame` sin verificación de null. Estos casos tenían alto riesgo de causar crashes, especialmente durante la inicialización o destrucción del juego.

**Próximos pasos:**
1. Continuar revisando y corrigiendo casos restantes
2. Considerar migración gradual a `ZGetGame()` para nuevo código
3. Agregar tests para verificar comportamiento durante inicialización/destrucción

---

**Fecha de análisis:** 2024
**Archivos analizados:** ZWeapon.cpp, ZActor.cpp, ZCharacter.cpp
**Estado:** Correcciones aplicadas, casos críticos resueltos

