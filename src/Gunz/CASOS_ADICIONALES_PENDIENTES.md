# Casos Adicionales Pendientes de Optimización

Este documento lista todos los casos adicionales encontrados que aún requieren optimización para reducir llamadas redundantes a accesores.

## Resumen

Se encontraron **20+ casos adicionales** en **8 archivos** que podrían beneficiarse de las mismas optimizaciones aplicadas anteriormente.

---

## 1. ZGameInterface.cpp

### Caso 1.1: PostStageState() - Línea 4074

**Problema**: Múltiples llamadas a `ZGetGameClient()`.

```cpp
static void PostStageState(bool Shop)
{
    if (ZGetGameInterface()->GetState() == GUNZ_STAGE)
        ZPostStageState(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(),
            Shop ? MOSS_SHOP : MOSS_EQUIPMENT);
}
```

**Solución sugerida**:
```cpp
static void PostStageState(bool Shop)
{
    if (ZGetGameInterface()->GetState() == GUNZ_STAGE)
    {
        // Optimización: Guardar ZGetGameClient() en variable local
        ZGameClient* pGameClient = ZGetGameClient();
        ZPostStageState(pGameClient->GetPlayerUID(), pGameClient->GetStageUID(),
            Shop ? MOSS_SHOP : MOSS_EQUIPMENT);
    }
}
```

---

### Caso 1.2: HideShopOrEquipmentDialog() - Línea 4148

**Problema**: Múltiples llamadas a `ZGetGameClient()`.

```cpp
static void HideShopOrEquipmentDialog(bool Shop)
{
    bool InStage = ZGetGameInterface()->GetState() == GUNZ_STAGE;
    
    if (InStage)
        ZPostStageState(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), MOSS_NONREADY);
    // ...
}
```

**Solución sugerida**:
```cpp
static void HideShopOrEquipmentDialog(bool Shop)
{
    bool InStage = ZGetGameInterface()->GetState() == GUNZ_STAGE;
    
    if (InStage)
    {
        // Optimización: Guardar ZGetGameClient() en variable local
        ZGameClient* pGameClient = ZGetGameClient();
        ZPostStageState(pGameClient->GetPlayerUID(), pGameClient->GetStageUID(), MOSS_NONREADY);
    }
    // ...
}
```

---

### Caso 1.3: LeaveBattle() - Línea 4627

**Problema**: Múltiples llamadas a `ZGetGameClient()`.

```cpp
ZPostStageLeaveBattle(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID());
if (m_bLeaveStageReserved) {
    ZPostStageLeave(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID());
    SetState(GUNZ_LOBBY);
}
```

**Solución sugerida**:
```cpp
// Optimización: Guardar ZGetGameClient() en variable local
ZGameClient* pGameClient = ZGetGameClient();
ZPostStageLeaveBattle(pGameClient->GetPlayerUID(), pGameClient->GetStageUID());
if (m_bLeaveStageReserved) {
    ZPostStageLeave(pGameClient->GetPlayerUID(), pGameClient->GetStageUID());
    SetState(GUNZ_LOBBY);
}
```

---

### Caso 1.4: OnUpdateLobbyInterface() - Línea 4774

**Problema**: Múltiples llamadas a `ZGetGameClient()`.

```cpp
bool bClanServer = ((ZGetGameClient()->GetServerMode() == MSM_CLAN) || (ZGetGameClient()->GetServerMode() == MSM_TEST));
```

**Solución sugerida**:
```cpp
// Optimización: Guardar ZGetGameClient() en variable local
ZGameClient* pGameClient = ZGetGameClient();
bool bClanServer = ((pGameClient->GetServerMode() == MSM_CLAN) || (pGameClient->GetServerMode() == MSM_TEST));
```

---

### Caso 1.5: OnUpdateLobbyInterface() - Línea 2138

**Problema**: Múltiples llamadas a `ZGetGameClient()`.

```cpp
pLabel = (MLabel*)pRes->FindWidget("Lobby_ChannelName");
sprintf_safe(buf, "%s > %s > %s", ZGetGameClient()->GetServerName(), ZMsg(MSG_WORD_LOBBY), ZGetGameClient()->GetChannelName());
```

**Solución sugerida**:
```cpp
pLabel = (MLabel*)pRes->FindWidget("Lobby_ChannelName");
// Optimización: Guardar ZGetGameClient() en variable local
ZGameClient* pGameClient = ZGetGameClient();
sprintf_safe(buf, "%s > %s > %s", pGameClient->GetServerName(), ZMsg(MSG_WORD_LOBBY), pGameClient->GetChannelName());
```

---

## 2. ZCombatInterface.cpp

### Caso 2.1: DrawScoreBoard() - Línea 1437

**Problema**: Múltiples llamadas a `ZGetGameClient()`.

```cpp
else
{
    sprintf_safe(szText, "(%03d) %s", ZGetGameClient()->GetStageNumber(), ZGetGameClient()->GetStageName());
}
```

**Solución sugerida**:
```cpp
else
{
    // Optimización: Guardar ZGetGameClient() en variable local
    ZGameClient* pGameClient = ZGetGameClient();
    sprintf_safe(szText, "(%03d) %s", pGameClient->GetStageNumber(), pGameClient->GetStageName());
}
```

---

## 3. ZStageInterface.cpp

### Caso 3.1: OnUpdateStageInterface() - Línea 248

**Problema**: Múltiples llamadas a `ZGetGameClient()`.

```cpp
if ( pLabel != 0)
{
    char szStr[ 256];
    sprintf_safe( szStr, "%s > %s > %03d:%s",
        ZGetGameClient()->GetServerName(), ZMsg( MSG_WORD_STAGE),
        ZGetGameClient()->GetStageNumber(), ZGetGameClient()->GetStageName());
    pLabel->SetText( szStr);
}
```

**Solución sugerida**:
```cpp
if ( pLabel != 0)
{
    // Optimización: Guardar ZGetGameClient() en variable local (ya existe pGameClient más arriba, reutilizar)
    char szStr[ 256];
    sprintf_safe( szStr, "%s > %s > %03d:%s",
        pGameClient->GetServerName(), ZMsg( MSG_WORD_STAGE),
        pGameClient->GetStageNumber(), pGameClient->GetStageName());
    pLabel->SetText( szStr);
}
```

**Nota**: Este caso ya tiene `pGameClient` definido más arriba en el método (línea 194), solo necesita reutilizarse.

---

## 4. ZGameInterface_OnCommand.cpp

### Caso 4.1: OnCommand() - Línea 171

**Problema**: Múltiples llamadas a `ZGetGameClient()`.

```cpp
// if playing a clanwar type server decide if go to clan channel upon login
if ( ( (ZGetGameClient()->GetServerMode() != MSM_CLAN) && (ZGetGameClient()->GetServerMode() != MSM_TEST) ) || (!ZGetMyInfo()->IsClanJoined()) )
{
    ZPostRequestRecommendChannel();
}
```

**Solución sugerida**:
```cpp
// if playing a clanwar type server decide if go to clan channel upon login
// Optimización: Guardar ZGetGameClient() en variable local
ZGameClient* pGameClient = ZGetGameClient();
if ( ( (pGameClient->GetServerMode() != MSM_CLAN) && (pGameClient->GetServerMode() != MSM_TEST) ) || (!ZGetMyInfo()->IsClanJoined()) )
{
    ZPostRequestRecommendChannel();
}
```

---

## 5. ZChat.cpp

### Caso 5.1: ProcessInput() - Línea 131

**Problema**: Múltiples llamadas a `ZGetGameClient()`.

```cpp
// ׽ Ǻ - test̰ launchdevelop 쿡 ׽
if ((ZIsLaunchDevelop()) && 
    ((ZGetGameClient()->GetServerMode() == MSM_TEST) || (!ZGetGameClient()->IsConnected())) )
{
    nCmdInputFlag |= ZChatCmdManager::CIF_TESTER;
}
```

**Solución sugerida**:
```cpp
// Optimización: Guardar ZGetGameClient() en variable local
ZGameClient* pGameClient = ZGetGameClient();
if ((ZIsLaunchDevelop()) && 
    ((pGameClient->GetServerMode() == MSM_TEST) || (!pGameClient->IsConnected())) )
{
    nCmdInputFlag |= ZChatCmdManager::CIF_TESTER;
}
```

---

### Caso 5.2: ProcessInput() - Línea 201

**Problema**: Múltiples llamadas a `ZGetGameClient()`.

```cpp
case GUNZ_LOBBY:
{
    ZPostChannelChat(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetChannelUID(), szMsg);
}
break;
```

**Solución sugerida**:
```cpp
case GUNZ_LOBBY:
{
    // Optimización: Guardar ZGetGameClient() en variable local
    ZGameClient* pGameClient = ZGetGameClient();
    ZPostChannelChat(pGameClient->GetPlayerUID(), pGameClient->GetChannelUID(), szMsg);
}
break;
```

---

### Caso 5.3: ProcessInput() - Línea 206

**Problema**: Múltiples llamadas a `ZGetGameClient()`.

```cpp
case GUNZ_STAGE:
{
    ZPostStageChat(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), szMsg);
}
break;
```

**Solución sugerida**:
```cpp
case GUNZ_STAGE:
{
    // Optimización: Guardar ZGetGameClient() en variable local
    ZGameClient* pGameClient = ZGetGameClient();
    ZPostStageChat(pGameClient->GetPlayerUID(), pGameClient->GetStageUID(), szMsg);
}
break;
```

---

## 6. ZGame.cpp

### Caso 6.1: Create() - Línea 491

**Problema**: Llamada a `ZGetGameClient()` cuando ya existe `pGameClient` definido arriba.

```cpp
m_pMyCharacter = (ZMyCharacter*)m_CharacterManager.Add(ZGetGameClient()->GetPlayerUID(), rvector(0.0f, 0.0f, 0.0f), true);
```

**Solución sugerida**:
```cpp
// Reutilizar pGameClient ya definido arriba en el método
m_pMyCharacter = (ZMyCharacter*)m_CharacterManager.Add(pGameClient->GetPlayerUID(), rvector(0.0f, 0.0f, 0.0f), true);
```

---

## 7. ZCharacter.cpp

### Caso 7.1: OnShot() - Línea 597

**Problema**: Múltiples llamadas a `ZGetGame()` en el mismo método.

```cpp
RBSPPICKINFO bpi;
if (ZGetGame()->GetWorld()->GetBsp()->Pick(nPos, nDir, &bpi))
{
    // ...
}

// Más abajo en el mismo método (línea 609)
ZGame* pGame = ZGetGame();
if (pGame && pGame->m_pMyCharacter && m_UID == pGame->m_pMyCharacter->m_UID) {
    // ...
}
```

**Solución sugerida**:
```cpp
// Optimización: Guardar ZGetGame() en variable local al inicio del método
ZGame* pGame = ZGetGame();
if (!pGame) return;

RBSPPICKINFO bpi;
if (pGame->GetWorld()->GetBsp()->Pick(nPos, nDir, &bpi))
{
    // ...
}

// Más abajo, reutilizar pGame
if (pGame->m_pMyCharacter && m_UID == pGame->m_pMyCharacter->m_UID) {
    // ...
}
```

---

### Caso 7.2: Draw() - Línea 678

**Problema**: Llamada a `ZGetGame()` que podría optimizarse.

```cpp
if (!ZGetGame()->GetWorld()->GetBsp()->IsVisible(bb)) return;
```

**Solución sugerida**:
```cpp
// Optimización: Guardar ZGetGame() en variable local si se usa múltiples veces
// Si solo se usa una vez, mantener como está (overhead mínimo)
ZGame* pGame = ZGetGame();
if (!pGame || !pGame->GetWorld()->GetBsp()->IsVisible(bb)) return;
```

**Nota**: Este caso es menos crítico si solo se llama una vez en el método.

---

## 8. ZWeapon.cpp

### Caso 8.1: ZWeaponRocket::Create() - Línea 90

**Problema**: Ya está optimizado parcialmente, pero se podría mejorar.

```cpp
ZGame* pGame = ZGetGame();
if (!pGame) return;

float currentTime = pGame->GetTime();
m_fStartTime = currentTime;
m_fLastAddTime = currentTime;
```

**Estado**: ✅ Ya optimizado correctamente.

---

### Caso 8.2: ZWeaponRocket::Update() - Línea 126

**Problema**: Ya está optimizado parcialmente.

```cpp
ZGame* pGame = ZGetGame();
if (!pGame) return false;
```

**Estado**: ✅ Ya optimizado correctamente.

---

## 9. ZInterfaceListener.cpp

### Caso 9.1: Múltiples listeners con llamadas repetidas

**Problema**: Varios listeners tienen múltiples llamadas a `ZGetGameClient()`.

**Ejemplos encontrados**:
- `ZGetStageTeamRedListener` - Línea 796
- `ZGetStageTeamBlueListener` - Línea 804
- `ZGetStageReadyListener` - Línea 820
- `ZGetStageObserverListener` - Línea 830, 838, 840, 844
- `ZGetLobbyListener` - Línea 859
- `ZGetForcedEntryListener` - Línea 1302
- Y más...

**Solución sugerida**: Para cada listener, guardar `ZGetGameClient()` al inicio si se usa múltiples veces.

---

## Estadísticas de Casos Pendientes

| Archivo | Casos Encontrados | Prioridad |
|---------|-------------------|-----------|
| `ZGameInterface.cpp` | 5 | Alta |
| `ZChat.cpp` | 3 | Alta |
| `ZInterfaceListener.cpp` | 10+ | Media |
| `ZCharacter.cpp` | 2 | Media |
| `ZCombatInterface.cpp` | 1 | Media |
| `ZStageInterface.cpp` | 1 | Baja (ya tiene variable) |
| `ZGameInterface_OnCommand.cpp` | 1 | Media |
| `ZGame.cpp` | 1 | Baja (ya tiene variable) |

**Total**: **24+ casos pendientes**

---

## Priorización

### Prioridad Alta
- `ZGameInterface.cpp` - Métodos llamados frecuentemente
- `ZChat.cpp` - Métodos llamados durante cada mensaje de chat

### Prioridad Media
- `ZInterfaceListener.cpp` - Listeners llamados en eventos de UI
- `ZCharacter.cpp` - Métodos de renderizado
- `ZGameInterface_OnCommand.cpp` - Procesamiento de comandos

### Prioridad Baja
- Casos donde ya existe una variable local que puede reutilizarse
- Casos con una sola llamada (overhead mínimo)

---

## Patrón de Optimización Recomendado

Para cada caso, seguir este patrón:

1. **Identificar** todas las llamadas al mismo accesor en el método
2. **Guardar** el resultado en una variable local al inicio
3. **Reemplazar** todas las llamadas con la variable
4. **Agregar comentario** explicando la optimización

**Ejemplo**:
```cpp
// Antes
void SomeMethod() {
    if (ZGetGameClient()->Condition1()) {
        DoSomething(ZGetGameClient()->GetValue1());
        ZGetGameClient()->DoAction();
    }
    ZPostSomething(ZGetGameClient()->GetValue2());
}

// Después
void SomeMethod() {
    // Optimización: Guardar ZGetGameClient() en variable local para evitar múltiples llamadas
    ZGameClient* pGameClient = ZGetGameClient();
    if (pGameClient->Condition1()) {
        DoSomething(pGameClient->GetValue1());
        pGameClient->DoAction();
    }
    ZPostSomething(pGameClient->GetValue2());
}
```

---

**Fecha de identificación**: Generado automáticamente  
**Total de casos pendientes**: 24+  
**Archivos afectados**: 8

