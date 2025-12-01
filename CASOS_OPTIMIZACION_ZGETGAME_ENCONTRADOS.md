# Casos de Optimización ZGetGame() Encontrados

## Resumen Ejecutivo

Se encontraron **múltiples casos** donde se puede optimizar usando `ZGetGame()` guardado en variable local para evitar llamadas redundantes. Este documento lista todos los casos identificados con prioridades y detalles.

---

## 🔴 Prioridad CRÍTICA (Funciones Llamadas Cada Frame)

### 1. ZWeapon.cpp - ZWeaponRocket::Update() (Líneas 124-228)

**Problema:** Múltiples accesos mezclados a `g_pGame` y `ZGetGame()`

```cpp
// ❌ INEFICIENTE
bool ZWeaponRocket::Update(float fElapsedTime)
{
    // ...
    if(g_pGame->GetTime() - m_fStartTime > ROCKET_LIFE ) { // Línea 134
        // ...
    }
    
    bool bPicked = ZGetGame()->Pick(nullptr, m_Position, dir, &zpi, dwPickPassFlag); // Línea 154
    
    // ...
    float this_time = g_pGame->GetTime(); // Línea 210
    // ...
}
```

**Optimización:**
```cpp
// ✅ OPTIMIZADO
bool ZWeaponRocket::Update(float fElapsedTime)
{
    ZGame* pGame = ZGetGame();
    if (!pGame) return false;
    
    // ...
    if(pGame->GetTime() - m_fStartTime > ROCKET_LIFE ) {
        // ...
    }
    
    bool bPicked = pGame->Pick(nullptr, m_Position, dir, &zpi, dwPickPassFlag);
    
    // ...
    float this_time = pGame->GetTime();
    // ...
}
```

**Impacto:** ALTO - Se llama cada frame para cada cohete activo
**Prioridad:** 🔴 CRÍTICA

---

### 2. ZWeapon.cpp - ZWeaponRocket::Create() (Líneas 91-120)

**Problema:** 2 accesos a `g_pGame->GetTime()` al inicio

```cpp
// ❌ INEFICIENTE
void ZWeaponRocket::Create(...)
{
    // ...
    m_fStartTime = g_pGame->GetTime(); // Línea 94
    m_fLastAddTime = g_pGame->GetTime(); // Línea 95
    // ...
}
```

**Optimización:**
```cpp
// ✅ OPTIMIZADO
void ZWeaponRocket::Create(...)
{
    ZGame* pGame = ZGetGame();
    if (!pGame) return;
    
    // ...
    float currentTime = pGame->GetTime();
    m_fStartTime = currentTime;
    m_fLastAddTime = currentTime;
    // ...
}
```

**Impacto:** MEDIO - Se llama al crear cada cohete
**Prioridad:** 🟡 ALTA

---

### 3. ZActor.cpp - UpdateBasicInfo() (Líneas 559-598)

**Problema:** 3 accesos a `ZGetGame()->GetTime()` en la misma función

```cpp
// ❌ INEFICIENTE
void ZActor::UpdateBasicInfo()
{
    // ...
    if (IsDead() && ZGetGame()->GetTime() - GetDeadTime() > 5.f) return; // Línea 564
    
    // ...
    pbi.fTime = ZGetGame()->GetTime(); // Línea 572
    
    // ...
    Item.fSendTime = Item.fReceivedTime = ZGetGame()->GetTime(); // Línea 595
}
```

**Optimización:**
```cpp
// ✅ OPTIMIZADO
void ZActor::UpdateBasicInfo()
{
    ZGame* pGame = ZGetGame();
    if (!pGame) return;
    
    // ...
    if (IsDead() && pGame->GetTime() - GetDeadTime() > 5.f) return;
    
    // ...
    float currentTime = pGame->GetTime();
    pbi.fTime = currentTime;
    
    // ...
    Item.fSendTime = Item.fReceivedTime = currentTime;
}
```

**Impacto:** ALTO - Se llama frecuentemente para cada NPC
**Prioridad:** 🔴 CRÍTICA

---

### 4. ZActor.cpp - OnUpdate() (Líneas 1011-1039)

**Problema:** 2 accesos a `ZGetGame()->GetTime()` en la misma función

```cpp
// ❌ INEFICIENTE
void ZActor::OnUpdate(float fDelta)
{
    // ...
    if (m_pModule_HPAP->GetHP() <= 0) {
        SetDeadTime(ZGetGame()->GetTime()); // Línea 1014
        // ...
    }
    // ...
    if (ZGetGame()->GetTime() - GetDeadTime() > GetNPCInfo()->fDyingTime) // Línea 1028
    {
        // ...
    }
}
```

**Optimización:**
```cpp
// ✅ OPTIMIZADO
void ZActor::OnUpdate(float fDelta)
{
    ZGame* pGame = ZGetGame();
    if (!pGame) return;
    
    // ...
    if (m_pModule_HPAP->GetHP() <= 0) {
        SetDeadTime(pGame->GetTime());
        // ...
    }
    // ...
    if (pGame->GetTime() - GetDeadTime() > GetNPCInfo()->fDyingTime)
    {
        // ...
    }
}
```

**Impacto:** ALTO - Se llama cada frame para cada NPC
**Prioridad:** 🔴 CRÍTICA

---

## 🟡 Prioridad ALTA (Funciones Llamadas Frecuentemente)

### 5. ZMatch.cpp - SoloSpawn() (Líneas 80-122)

**Problema:** Múltiples accesos mezclados a `g_pGame` y `ZApplication::GetGame()`

```cpp
// ❌ INEFICIENTE
void ZMatch::SoloSpawn()
{
    // ...
    if (!IsWaitForRoundEnd() && g_pGame->m_pMyCharacter) // Línea 85
    {
        if (g_pGame->m_pMyCharacter->IsDead()) // Línea 88
        {
            // ...
        }
        bLastDead = g_pGame->m_pMyCharacter->IsDead(); // Línea 119
    }
}
```

**Optimización:**
```cpp
// ✅ OPTIMIZADO
void ZMatch::SoloSpawn()
{
    ZGame* pGame = ZGetGame();
    if (!pGame) return;
    
    // ...
    if (!IsWaitForRoundEnd() && pGame->m_pMyCharacter)
    {
        if (pGame->m_pMyCharacter->IsDead())
        {
            // ...
        }
        bLastDead = pGame->m_pMyCharacter->IsDead();
    }
}
```

**Impacto:** MEDIO - Se llama cuando el jugador muere
**Prioridad:** 🟡 ALTA

---

### 6. ZMatch.cpp - InitCharactersPosition() (Líneas 175-248)

**Problema:** Múltiples accesos mezclados (8+ accesos)

```cpp
// ❌ INEFICIENTE
void ZMatch::InitCharactersPosition()
{
    // ...
    for (auto& pair : g_pGame->m_CharacterManager) // Línea 181
    {
        // ...
        ZMapSpawnData* pSpawnData = g_pGame->GetMapDesc()->GetSpawnManager()->GetTeamData(...); // Línea 188
        // ...
    }
    
    if (ZApplication::GetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUEL) // Línea 203
    {
        ZRuleDuel* pDuel = (ZRuleDuel*)ZGetGameInterface()->GetGame()->GetMatch()->GetRule(); // Línea 205
        // ...
        ZMapSpawnData* pSpawnData = g_pGame->GetMapDesc()->GetSpawnManager()->GetData(nIndex); // Línea 216
        g_pGame->m_pMyCharacter->SetPosition(pSpawnData->m_Pos); // Línea 219
        g_pGame->m_pMyCharacter->SetDirection(pSpawnData->m_Dir); // Línea 220
        // ...
        ZMapSpawnData* pSpawnData = g_pGame->GetMapDesc()->GetSpawnManager()->GetTeamData(nIndex, 0); // Línea 225
        g_pGame->m_pMyCharacter->SetPosition(pSpawnData->m_Pos); // Línea 228
        g_pGame->m_pMyCharacter->SetDirection(pSpawnData->m_Dir); // Línea 229
    }
    
    ZMapSpawnData* pSpawnData = g_pGame->GetMapDesc()->GetSpawnManager()->GetSoloRandomData(); // Línea 237
    g_pGame->m_pMyCharacter->SetPosition(pos); // Línea 246
    g_pGame->m_pMyCharacter->SetDirection(dir); // Línea 247
}
```

**Optimización:**
```cpp
// ✅ OPTIMIZADO
void ZMatch::InitCharactersPosition()
{
    ZGame* pGame = ZGetGame();
    if (!pGame) return;
    
    // ...
    for (auto& pair : pGame->m_CharacterManager)
    {
        // ...
        ZMapSpawnData* pSpawnData = pGame->GetMapDesc()->GetSpawnManager()->GetTeamData(...);
        // ...
    }
    
    if (pGame->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUEL)
    {
        ZRuleDuel* pDuel = (ZRuleDuel*)pGame->GetMatch()->GetRule();
        // ...
        ZMapSpawnData* pSpawnData = pGame->GetMapDesc()->GetSpawnManager()->GetData(nIndex);
        pGame->m_pMyCharacter->SetPosition(pSpawnData->m_Pos);
        pGame->m_pMyCharacter->SetDirection(pSpawnData->m_Dir);
        // ...
        ZMapSpawnData* pSpawnData = pGame->GetMapDesc()->GetSpawnManager()->GetTeamData(nIndex, 0);
        pGame->m_pMyCharacter->SetPosition(pSpawnData->m_Pos);
        pGame->m_pMyCharacter->SetDirection(pSpawnData->m_Dir);
    }
    
    ZMapSpawnData* pSpawnData = pGame->GetMapDesc()->GetSpawnManager()->GetSoloRandomData();
    pGame->m_pMyCharacter->SetPosition(pos);
    pGame->m_pMyCharacter->SetDirection(dir);
}
```

**Impacto:** MEDIO - Se llama al iniciar cada ronda
**Prioridad:** 🟡 ALTA

---

### 7. ZMatch.cpp - InitRound() (Líneas 250-260)

**Problema:** Múltiples accesos a `g_pGame`

```cpp
// ❌ INEFICIENTE
void ZMatch::InitRound()
{
    g_pGame->InitRound(); // Línea 252
    
    InitCharactersPosition();
    InitCharactersProperties();
    
    // ...
    rvector pos = g_pGame->m_pMyCharacter->GetPosition(); // Línea 259
    rvector dir = g_pGame->m_pMyCharacter->GetLowerDir(); // Línea 260
}
```

**Optimización:**
```cpp
// ✅ OPTIMIZADO
void ZMatch::InitRound()
{
    ZGame* pGame = ZGetGame();
    if (!pGame) return;
    
    pGame->InitRound();
    
    InitCharactersPosition();
    InitCharactersProperties();
    
    // ...
    rvector pos = pGame->m_pMyCharacter->GetPosition();
    rvector dir = pGame->m_pMyCharacter->GetLowerDir();
}
```

**Impacto:** MEDIO - Se llama al iniciar cada ronda
**Prioridad:** 🟡 ALTA

---

### 8. ZCombatInterface.cpp - DrawTDMScore() (Líneas 400-409)

**Problema:** 2 accesos a `ZGetGame()` en función de renderizado

```cpp
// ❌ INEFICIENTE
void ZCombatInterface::DrawTDMScore(MDrawContext* pDC)
{
    int nBlueKills = ZGetGame()->GetMatch()->GetTeamKills(MMT_BLUE); // Línea 402
    int nRedKills = ZGetGame()->GetMatch()->GetTeamKills(MMT_RED); // Línea 403
    // ...
}
```

**Optimización:**
```cpp
// ✅ OPTIMIZADO
void ZCombatInterface::DrawTDMScore(MDrawContext* pDC)
{
    ZGame* pGame = ZGetGame();
    if (!pGame) return;
    
    int nBlueKills = pGame->GetMatch()->GetTeamKills(MMT_BLUE);
    int nRedKills = pGame->GetMatch()->GetTeamKills(MMT_RED);
    // ...
}
```

**Impacto:** MEDIO - Se llama cada frame durante partidas por equipos
**Prioridad:** 🟡 ALTA

---

### 9. ZGameInterface.cpp - ReserveLeaveBattle() (Líneas 4643-4653)

**Problema:** 3 accesos a `ZGetGame()` en la misma condición

```cpp
// ❌ INEFICIENTE
void ZGameInterface::ReserveLeaveBattle()
{
    if (!m_pGame) return;
    
    if (ZGetGame()->GetTime() - ZGetGame()->m_pMyCharacter->LastDamagedTime > 5 // Línea 4647
        || !ZGetGame()->m_pMyCharacter->IsAlive() // Línea 4648
        || ZGetGame()->IsReplay()) // Línea 4649
    {
        LeaveBattle();
        return;
    }
}
```

**Optimización:**
```cpp
// ✅ OPTIMIZADO
void ZGameInterface::ReserveLeaveBattle()
{
    if (!m_pGame) return;
    
    ZGame* pGame = ZGetGame();
    if (!pGame) return;
    
    if (pGame->GetTime() - pGame->m_pMyCharacter->LastDamagedTime > 5
        || !pGame->m_pMyCharacter->IsAlive()
        || pGame->IsReplay())
    {
        LeaveBattle();
        return;
    }
}
```

**Impacto:** MEDIO - Se llama cuando se intenta salir de batalla
**Prioridad:** 🟡 ALTA

---

## 🟢 Prioridad MEDIA (Funciones con 2 Accesos)

### 10. ZWeapon.cpp - ZWeaponGrenade::Update() (Líneas 509-608)

**Problema:** Múltiples accesos a `g_pGame` en función de update

```cpp
// ❌ INEFICIENTE
bool ZWeaponGrenade::Update(float fElapsedTime)
{
    // ...
    if(g_pGame->GetTime() - m_fStartTime > GRENADE_LIFE) { // Línea 511
        // ...
    }
    
    // ...
    bool bPicked=g_pGame->Pick(ZGetCharacterManager()->Find(m_uidOwner),m_Position,dir,&zpi,dwPickPassFlag); // Línea 532
    // ...
    
    g_pGame->OnExplosionGrenade(m_uidOwner, v, m_fDamage, 400.f, .2f, 1.f, m_nTeamID); // Línea 608
}
```

**Optimización:**
```cpp
// ✅ OPTIMIZADO
bool ZWeaponGrenade::Update(float fElapsedTime)
{
    ZGame* pGame = ZGetGame();
    if (!pGame) return false;
    
    // ...
    if(pGame->GetTime() - m_fStartTime > GRENADE_LIFE) {
        // ...
    }
    
    // ...
    bool bPicked = pGame->Pick(ZGetCharacterManager()->Find(m_uidOwner), m_Position, dir, &zpi, dwPickPassFlag);
    // ...
    
    pGame->OnExplosionGrenade(m_uidOwner, v, m_fDamage, 400.f, .2f, 1.f, m_nTeamID);
}
```

**Impacto:** MEDIO - Se llama cada frame para cada granada activa
**Prioridad:** 🟢 MEDIA

---

### 11. ZWeapon.cpp - ZWeaponFlashBang::Update() (Líneas 630-693)

**Problema:** Múltiples accesos a `g_pGame`

```cpp
// ❌ INEFICIENTE
bool ZWeaponFlashBang::Update(float fElapsedTime)
{
    // ...
    if (g_pGame->m_pMyCharacter->IsDead()) // Línea 632
    {
        return;
    }
    
    // ...
    rvector temp = g_pGame->m_pMyCharacter->m_Position - m_Position; // Línea 645
    // ...
    rvector pos = g_pGame->m_pMyCharacter->GetPosition(); // Línea 659
    rvector dir = g_pGame->m_pMyCharacter->GetTargetDir(); // Línea 660
    // ...
    float lap = g_pGame->GetTime() - m_fStartTime; // Línea 671
    // ...
    bool bPicked = g_pGame->Pick(ZGetCharacterManager()->Find(m_uidOwner), m_Position, dir, &zpi, dwPickPassFlag); // Línea 693
}
```

**Optimización:**
```cpp
// ✅ OPTIMIZADO
bool ZWeaponFlashBang::Update(float fElapsedTime)
{
    ZGame* pGame = ZGetGame();
    if (!pGame) return false;
    
    // ...
    if (pGame->m_pMyCharacter->IsDead())
    {
        return;
    }
    
    // ...
    rvector temp = pGame->m_pMyCharacter->m_Position - m_Position;
    // ...
    rvector pos = pGame->m_pMyCharacter->GetPosition();
    rvector dir = pGame->m_pMyCharacter->GetTargetDir();
    // ...
    float lap = pGame->GetTime() - m_fStartTime;
    // ...
    bool bPicked = pGame->Pick(ZGetCharacterManager()->Find(m_uidOwner), m_Position, dir, &zpi, dwPickPassFlag);
}
```

**Impacto:** MEDIO - Se llama cada frame para cada flashbang activa
**Prioridad:** 🟢 MEDIA

---

### 12. ZWeapon.cpp - ZWeaponSmokeGrenade::Update() (Líneas 790-819)

**Problema:** Múltiples accesos a `g_pGame`

```cpp
// ❌ INEFICIENTE
bool ZWeaponSmokeGrenade::Update(float fElapsedTime)
{
    rvector oldPos = m_Position;
    float lap = g_pGame->GetTime() - m_fStartTime; // Línea 792
    
    // ...
    bool bPicked = g_pGame->Pick(ZGetCharacterManager()->Find(m_uidOwner), m_Position, dir, &zpi, dwPickPassFlag); // Línea 819
}
```

**Optimización:**
```cpp
// ✅ OPTIMIZADO
bool ZWeaponSmokeGrenade::Update(float fElapsedTime)
{
    ZGame* pGame = ZGetGame();
    if (!pGame) return false;
    
    rvector oldPos = m_Position;
    float lap = pGame->GetTime() - m_fStartTime;
    
    // ...
    bool bPicked = pGame->Pick(ZGetCharacterManager()->Find(m_uidOwner), m_Position, dir, &zpi, dwPickPassFlag);
}
```

**Impacto:** MEDIO - Se llama cada frame para cada granada de humo activa
**Prioridad:** 🟢 MEDIA

---

### 13. ZWeapon.cpp - ZWeaponMagic::Update() (Líneas 1074-1140)

**Problema:** Múltiples accesos a `g_pGame`

```cpp
// ❌ INEFICIENTE
bool ZWeaponMagic::Update(float fElapsedTime)
{
    // ...
    float this_time = g_pGame->GetTime(); // Línea 1076
    // ...
    bool bPicked = g_pGame->Pick(pOwnerObject, m_Position, dir, &zpi, dwPickPassFlag); // Línea 1009
    // ...
    if (g_pGame->ObjectColTest(pOwnerObject, m_Position, to, m_pSkillDesc->fColRadius, &pPickObject)) // Línea 1045
    // ...
}
```

**Optimización:**
```cpp
// ✅ OPTIMIZADO
bool ZWeaponMagic::Update(float fElapsedTime)
{
    ZGame* pGame = ZGetGame();
    if (!pGame) return false;
    
    // ...
    float this_time = pGame->GetTime();
    // ...
    bool bPicked = pGame->Pick(pOwnerObject, m_Position, dir, &zpi, dwPickPassFlag);
    // ...
    if (pGame->ObjectColTest(pOwnerObject, m_Position, to, m_pSkillDesc->fColRadius, &pPickObject))
    // ...
}
```

**Impacto:** MEDIO - Se llama cada frame para cada arma mágica activa
**Prioridad:** 🟢 MEDIA

---

## 📊 Resumen de Casos Encontrados

### Por Prioridad

| Prioridad | Cantidad | Archivos Afectados |
|-----------|----------|-------------------|
| 🔴 CRÍTICA | 4 casos | ZWeapon.cpp, ZActor.cpp |
| 🟡 ALTA | 5 casos | ZMatch.cpp, ZCombatInterface.cpp, ZGameInterface.cpp |
| 🟢 MEDIA | 4 casos | ZWeapon.cpp |

### Por Archivo

| Archivo | Casos | Prioridad |
|---------|-------|-----------|
| `ZWeapon.cpp` | 6 casos | 🔴🟡🟢 |
| `ZActor.cpp` | 2 casos | 🔴 |
| `ZMatch.cpp` | 3 casos | 🟡 |
| `ZCombatInterface.cpp` | 1 caso | 🟡 |
| `ZGameInterface.cpp` | 1 caso | 🟡 |

### Impacto Total

- **Total de casos:** 13 casos identificados
- **Funciones críticas (cada frame):** 4 casos
- **Funciones frecuentes:** 5 casos
- **Funciones ocasionales:** 4 casos

---

## 🎯 Recomendaciones

### Orden de Implementación

1. **Primero:** Casos CRÍTICOS (funciones llamadas cada frame)
   - `ZWeaponRocket::Update()`
   - `ZActor::UpdateBasicInfo()`
   - `ZActor::OnUpdate()`

2. **Segundo:** Casos ALTA prioridad (funciones frecuentes)
   - `ZMatch::InitCharactersPosition()`
   - `ZMatch::InitRound()`
   - `ZCombatInterface::DrawTDMScore()`
   - `ZGameInterface::ReserveLeaveBattle()`

3. **Tercero:** Casos MEDIA prioridad (funciones ocasionales)
   - Funciones de update de armas

### Beneficios Esperados

- **Rendimiento:** Reducción de ~10-20 ciclos de CPU por función optimizada
- **Seguridad:** Verificación de null consistente
- **Mantenibilidad:** Código más limpio y consistente
- **Consistencia:** Un solo patrón de acceso a `ZGetGame()`

---

**Fecha:** 2024
**Estado:** Casos identificados, pendiente implementación
**Próximos pasos:** Implementar optimizaciones en orden de prioridad

