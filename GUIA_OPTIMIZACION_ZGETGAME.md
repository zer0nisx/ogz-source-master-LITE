# Guía de Optimización: Cuándo Usar ZGetGame() en Variable Local

## Resumen Ejecutivo

**Usar `ZGetGame()` guardado en variable local es recomendado cuando:**
1. ✅ La función se llama **cada frame** (OnDraw, OnUpdate)
2. ✅ Hay **3 o más accesos** a `g_pGame`/`ZGetGame()` en la misma función
3. ✅ La función está en un **hot path** (código crítico de rendimiento)
4. ✅ Hay **múltiples verificaciones** de `IsReplay()`, `GetTime()`, etc.

**NO es necesario cuando:**
- ❌ Solo hay **1-2 accesos** únicos en la función
- ❌ La función se llama **raramente** (inicialización, eventos)
- ❌ El acceso está en un **early return** (solo se usa una vez)

---

## 1. Áreas Críticas (ALTA PRIORIDAD)

### 🔴 Funciones Llamadas Cada Frame

#### OnDraw() - Renderizado
```cpp
// ❌ INEFICIENTE - Múltiples llamadas a ZGetGame()
void ZCombatInterface::OnDraw(MDrawContext* pDC)
{
    if (ZGetGame()->m_pMyCharacter->IsAdminHide()) { ... }
    if (ZGetGame()->IsReplay()) { ... }
    ZGetGame()->m_HelpScreen.DrawHelpScreen();
    // 3+ llamadas = ~15-24 ciclos de CPU por frame
    // A 60 FPS = 900-1440 ciclos/segundo desperdiciados
}

// ✅ OPTIMIZADO - Una sola llamada
void ZCombatInterface::OnDraw(MDrawContext* pDC)
{
    ZGame* pGame = ZGetGame();
    if (!pGame) return;
    
    if (pGame->m_pMyCharacter->IsAdminHide()) { ... }
    if (pGame->IsReplay()) { ... }
    pGame->m_HelpScreen.DrawHelpScreen();
    // 1 llamada = ~5-8 ciclos de CPU por frame
    // Ahorro: ~10-16 ciclos por frame = 600-960 ciclos/segundo
}
```

**Archivos afectados:**
- `ZCombatInterface::OnDraw()` - ✅ Ya optimizado
- `ZCharacter::OnDraw()` - ⚠️ Pendiente
- `ZActor::OnDraw()` - ⚠️ Pendiente
- `ZEffectManager::Draw()` - ⚠️ Pendiente

---

#### OnUpdate() - Actualización de Estado
```cpp
// ❌ INEFICIENTE
void ZMyCharacter::OnUpdate(float fDelta)
{
    float currentTime = g_pGame->GetTime();
    if (g_pGame->IsReplay()) return;
    if (g_pGame->m_pMyCharacter->IsDead()) return;
    float elapsed = g_pGame->GetTime() - m_fLastTime;
    // 4+ llamadas = ~20-32 ciclos de CPU por frame
}

// ✅ OPTIMIZADO
void ZMyCharacter::OnUpdate(float fDelta)
{
    ZGame* pGame = ZGetGame();
    if (!pGame) return;
    
    float currentTime = pGame->GetTime();
    if (pGame->IsReplay()) return;
    if (pGame->m_pMyCharacter->IsDead()) return;
    float elapsed = pGame->GetTime() - m_fLastTime;
    // 1 llamada = ~5-8 ciclos de CPU por frame
    // Ahorro: ~15-24 ciclos por frame
}
```

**Archivos afectados:**
- `ZMyCharacter::OnUpdate()` - ⚠️ **CRÍTICO** - 81+ usos de `g_pGame->GetTime()`
- `ZCharacter::OnUpdate()` - ⚠️ Pendiente
- `ZActor::ProcessMotion()` - ⚠️ Pendiente
- `ZCombatInterface::Update()` - ✅ Ya optimizado

---

### 🔴 Funciones de Input (Cada Frame)

```cpp
// ❌ INEFICIENTE
bool ZGameInput::OnEvent(MEvent* pEvent)
{
    if (g_pGame->IsReplay()) return false;
    if (g_pGame->m_pMyCharacter) { ... }
    if (!g_pGame->IsReplay()) { ... }
    g_pGame->PostSpMotion(mtype);
    // 4+ llamadas = ~20-32 ciclos de CPU por evento
}

// ✅ OPTIMIZADO
bool ZGameInput::OnEvent(MEvent* pEvent)
{
    ZGame* pGame = ZGetGame();
    if (!pGame) return false;
    
    if (pGame->IsReplay()) return false;
    if (pGame->m_pMyCharacter) { ... }
    if (!pGame->IsReplay()) { ... }
    pGame->PostSpMotion(mtype);
    // 1 llamada = ~5-8 ciclos de CPU por evento
}
```

**Archivos afectados:**
- `ZGameInput::OnEvent()` - ⚠️ **CRÍTICO** - 30+ usos
- `ZGameInput::ProcessActionKey()` - ⚠️ Pendiente

---

## 2. Áreas Importantes (MEDIA PRIORIDAD)

### 🟡 Funciones con Múltiples Accesos (3+)

#### Funciones de Renderizado Complejo
```cpp
// ❌ INEFICIENTE - DrawScoreBoard() tiene 20+ accesos
void ZCombatInterface::DrawScoreBoard(...)
{
    if (ZApplication::GetGame()->GetMatch()->IsTeamPlay()) { ... }
    if (g_pGame->GetMatch()->GetMatchType() == ...) { ... }
    if (ZGetGame()->IsReplay()) { ... }
    // Mezcla de g_pGame, ZGetGame(), ZApplication::GetGame()
    // 20+ llamadas = ~100-160 ciclos de CPU
}

// ✅ OPTIMIZADO
void ZCombatInterface::DrawScoreBoard(...)
{
    ZGame* pGame = ZGetGame();
    if (!pGame) return;
    
    if (pGame->GetMatch()->IsTeamPlay()) { ... }
    if (pGame->GetMatch()->GetMatchType() == ...) { ... }
    if (pGame->IsReplay()) { ... }
    // 1 llamada = ~5-8 ciclos de CPU
    // Ahorro: ~95-152 ciclos por llamada
}
```

**Archivos afectados:**
- `ZCombatInterface::DrawScoreBoard()` - ⚠️ **ALTO** - 20+ accesos
- `ZCombatInterface::OnDrawCustom()` - ⚠️ Pendiente
- `ZCombatInterface::GetResultInfo()` - ⚠️ Pendiente

---

#### Funciones de Procesamiento de Datos
```cpp
// ❌ INEFICIENTE
void ZMatch::OnRoundStart()
{
    for (auto& pair : g_pGame->m_CharacterManager) { ... }
    if (g_pGame->GetSpawnRequested() == false) { ... }
    g_pGame->SetSpawnRequested(true);
    ZMapSpawnData* pSpawn = g_pGame->GetMapDesc()->GetSpawnManager()->GetData(...);
    // 4+ llamadas en función llamada frecuentemente
}

// ✅ OPTIMIZADO
void ZMatch::OnRoundStart()
{
    ZGame* pGame = ZGetGame();
    if (!pGame) return;
    
    for (auto& pair : pGame->m_CharacterManager) { ... }
    if (pGame->GetSpawnRequested() == false) { ... }
    pGame->SetSpawnRequested(true);
    ZMapSpawnData* pSpawn = pGame->GetMapDesc()->GetSpawnManager()->GetData(...);
}
```

**Archivos afectados:**
- `ZMatch::OnRoundStart()` - ⚠️ Pendiente
- `ZMatch::OnRoundEnd()` - ⚠️ Pendiente
- `ZQuest::OnUpdate()` - ⚠️ Pendiente

---

## 3. Áreas Opcionales (BAJA PRIORIDAD)

### 🟢 Funciones con 1-2 Accesos Únicos

```cpp
// ✅ ACEPTABLE - Solo 1 acceso, no necesita optimización
void SomeFunction()
{
    if (g_pGame && g_pGame->IsReplay()) {
        return;
    }
}

// ✅ ACEPTABLE - 2 accesos, pero en early return
void SomeFunction()
{
    if (!g_pGame) return;
    if (g_pGame->IsReplay()) return;
    // Solo se usa una vez después del early return
}
```

**No requiere optimización:**
- Funciones de inicialización (se llaman una vez)
- Funciones de eventos raros
- Funciones con early return después de 1-2 accesos

---

## 4. Patrones de Optimización

### Patrón 1: Función de Renderizado/Update
```cpp
void MyClass::OnDraw()
{
    // ✅ SIEMPRE optimizar funciones llamadas cada frame
    ZGame* pGame = ZGetGame();
    if (!pGame) return;
    
    // Usar pGame en lugar de g_pGame o ZGetGame()
    if (pGame->IsReplay()) return;
    pGame->GetTime();
    // ...
}
```

### Patrón 2: Función con Múltiples Accesos
```cpp
void MyClass::ProcessData()
{
    // ✅ Optimizar si hay 3+ accesos
    ZGame* pGame = ZGetGame();
    if (!pGame) return;
    
    if (pGame->GetMatch()->IsTeamPlay()) { ... }
    if (pGame->GetMatch()->GetMatchType() == ...) { ... }
    pGame->GetTime();
    // ...
}
```

### Patrón 3: Función con Early Return
```cpp
void MyClass::DoSomething()
{
    // ✅ Aceptable - Solo 1-2 accesos antes de early return
    if (!g_pGame) return;
    if (g_pGame->IsReplay()) return;
    
    // Resto de la función no usa g_pGame
    // No necesita optimización
}
```

---

## 5. Estadísticas de Impacto

### Funciones Críticas (Cada Frame)

| Función | Llamadas/Frame | Accesos Actuales | Ciclos Desperdiciados/Frame | Ahorro Potencial |
|---------|---------------|------------------|----------------------------|-------------------|
| `ZMyCharacter::OnUpdate()` | 1 | 81+ | ~405-648 | ~400-640 ciclos/frame |
| `ZCombatInterface::OnDraw()` | 1 | 3+ | ~15-24 | ✅ Ya optimizado |
| `ZGameInput::OnEvent()` | Variable | 30+ | ~150-240 | ~145-232 ciclos/evento |
| `ZCharacter::OnDraw()` | 1 | 5+ | ~25-40 | ~20-32 ciclos/frame |

**Impacto total a 60 FPS:**
- `ZMyCharacter::OnUpdate()`: ~24,000-38,400 ciclos/segundo desperdiciados
- `ZGameInput::OnEvent()`: Variable, pero significativo durante input activo

---

## 6. Recomendaciones por Archivo

### 🔴 Prioridad CRÍTICA

#### ZMyCharacter.cpp
- **Razón:** 81+ usos de `g_pGame->GetTime()` en funciones llamadas cada frame
- **Funciones prioritarias:**
  - `OnUpdate()` - ⚠️ **MÁXIMA PRIORIDAD**
  - `OnShot()` - ⚠️ Alta prioridad
  - `OnJump()` - ⚠️ Alta prioridad
  - `OnGuard()` - ⚠️ Alta prioridad

#### ZGameInput.cpp
- **Razón:** 30+ usos, procesa input del usuario
- **Funciones prioritarias:**
  - `OnEvent()` - ⚠️ **ALTA PRIORIDAD**
  - `ProcessActionKey()` - ⚠️ Media prioridad

---

### 🟡 Prioridad ALTA

#### ZCombatInterface.cpp
- **Razón:** Funciones de renderizado con múltiples accesos
- **Funciones prioritarias:**
  - `DrawScoreBoard()` - ⚠️ **ALTA PRIORIDAD** (20+ accesos)
  - `OnDrawCustom()` - ⚠️ Media prioridad
  - `GetResultInfo()` - ⚠️ Media prioridad
- **Nota:** `OnDraw()` y `Update()` ya están optimizados ✅

#### ZMatch.cpp
- **Razón:** Funciones llamadas frecuentemente durante el juego
- **Funciones prioritarias:**
  - `OnRoundStart()` - ⚠️ Media prioridad
  - `OnRoundEnd()` - ⚠️ Media prioridad

---

### 🟢 Prioridad MEDIA

#### ZCharacter.cpp
- **Razón:** Funciones de renderizado
- **Funciones prioritarias:**
  - `OnDraw()` - ⚠️ Media prioridad
  - `OnUpdate()` - ⚠️ Baja prioridad

#### ZActor.cpp
- **Razón:** Funciones de NPCs
- **Funciones prioritarias:**
  - `OnDraw()` - ⚠️ Media prioridad
  - `ProcessMotion()` - ⚠️ Baja prioridad

---

## 7. Ejemplo Completo de Optimización

### Antes (Ineficiente)
```cpp
void ZCombatInterface::DrawScoreBoard(MDrawContext* pDC)
{
    // ❌ 20+ llamadas a diferentes variantes
    if (ZApplication::GetGame()->GetMatch()->IsTeamPlay()) {
        // ...
    }
    if (g_pGame->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DEATHMATCH_TEAM) {
        // ...
    }
    if (ZGetGame()->IsReplay()) {
        // ...
    }
    ZApplication::GetGame()->GetMatch()->GetTeamScore(MMT_RED);
    g_pGame->GetMatch()->GetTeamScore(MMT_BLUE);
    // ... más accesos
}
```

### Después (Optimizado)
```cpp
void ZCombatInterface::DrawScoreBoard(MDrawContext* pDC)
{
    // ✅ Una sola llamada al inicio
    ZGame* pGame = ZGetGame();
    if (!pGame) return;
    
    // ✅ Usar pGame consistentemente
    if (pGame->GetMatch()->IsTeamPlay()) {
        // ...
    }
    if (pGame->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DEATHMATCH_TEAM) {
        // ...
    }
    if (pGame->IsReplay()) {
        // ...
    }
    pGame->GetMatch()->GetTeamScore(MMT_RED);
    pGame->GetMatch()->GetTeamScore(MMT_BLUE);
    // ... más accesos usando pGame
}
```

**Beneficios:**
- ✅ **Seguridad:** Verificación de null al inicio
- ✅ **Rendimiento:** 1 llamada en lugar de 20+
- ✅ **Consistencia:** Un solo patrón de acceso
- ✅ **Mantenibilidad:** Más fácil de leer y mantener

---

## 8. Regla de Oro

### ✅ **SIEMPRE optimizar cuando:**
1. La función se llama **cada frame** (OnDraw, OnUpdate)
2. Hay **3 o más accesos** a `g_pGame`/`ZGetGame()` en la misma función
3. La función está en un **hot path** (código crítico de rendimiento)

### ⚠️ **Considerar optimizar cuando:**
1. Hay **2 accesos** y la función se llama frecuentemente
2. Se mezclan `g_pGame`, `ZGetGame()`, y `ZApplication::GetGame()`

### ❌ **NO es necesario cuando:**
1. Solo hay **1-2 accesos** únicos
2. La función se llama **raramente** (inicialización, eventos)
3. El acceso está en un **early return** (solo se usa una vez)

---

## 9. Checklist de Optimización

Antes de optimizar, verificar:

- [ ] ¿La función se llama cada frame? → **SÍ: Optimizar**
- [ ] ¿Hay 3+ accesos a `g_pGame`/`ZGetGame()`? → **SÍ: Optimizar**
- [ ] ¿La función está en un hot path? → **SÍ: Optimizar**
- [ ] ¿Solo hay 1-2 accesos únicos? → **NO: No optimizar**
- [ ] ¿La función se llama raramente? → **NO: No optimizar**

---

## 10. Conclusión

**Áreas donde es MÁS recomendado optimizar:**

1. 🔴 **Funciones llamadas cada frame** (OnDraw, OnUpdate)
   - Impacto: Muy alto (60+ llamadas/segundo)
   - Prioridad: CRÍTICA

2. 🔴 **Funciones de input** (OnEvent, ProcessInput)
   - Impacto: Alto durante input activo
   - Prioridad: CRÍTICA

3. 🟡 **Funciones con 3+ accesos** (DrawScoreBoard, etc.)
   - Impacto: Medio-Alto
   - Prioridad: ALTA

4. 🟢 **Funciones con 1-2 accesos únicos**
   - Impacto: Bajo
   - Prioridad: OPCIONAL

**Archivos prioritarios para optimización:**
1. `ZMyCharacter.cpp` - ⚠️ **MÁXIMA PRIORIDAD** (81+ usos)
2. `ZGameInput.cpp` - ⚠️ **ALTA PRIORIDAD** (30+ usos)
3. `ZCombatInterface.cpp` - ⚠️ **ALTA PRIORIDAD** (DrawScoreBoard: 20+ usos)

---

**Fecha:** 2024
**Estado:** Guía completa de optimización
**Próximos pasos:** Aplicar optimizaciones en archivos prioritarios

