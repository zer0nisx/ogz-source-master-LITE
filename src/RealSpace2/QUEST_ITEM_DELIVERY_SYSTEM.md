# Sistema de Entrega de Objetos de Quest (ZItems)

## Resumen

Este documento explica cómo el juego entrega objetos de quest, específicamente los **ZItems** (objetos normales del juego entregados como recompensa de quest) y los **Quest Items** (objetos especiales de quest).

## Flujo Principal: Entrega de Recompensas al Completar Quest

### 0. Preparación de Recompensas: `MakeRewardList()`

**ANTES** de distribuir las recompensas, el sistema debe construir la lista de qué items darle a cada jugador. Esto se hace en `MakeRewardList()`:

```cpp
void MMatchRuleQuest::MakeRewardList()
{
    // Limpiar listas de recompensas de todos los jugadores
    for (MQuestPlayerManager::iterator itor = m_PlayerManager.begin(); 
         itor != m_PlayerManager.end(); ++itor)
    {
        MQuestPlayerInfo* pPlayerInfo = (*itor).second;
        pPlayerInfo->RewardQuestItemMap.Clear();
        pPlayerInfo->RewardZItemList.clear();  // ⭐ Limpiar ZItems
        a_vecPlayerInfos.push_back(pPlayerInfo);
    }
    
    // Iterar sobre todos los items obtenidos durante la quest
    for (itObtainQItem = m_pQuestLevel->GetDynamicInfo()->ItemMap.begin(); 
         itObtainQItem != endObtainQItem; ++itObtainQItem)
    {
        pObtainQItem = itObtainQItem->second;
        
        if (!pObtainQItem->bObtained) continue;  // Solo items obtenidos
        
        if (pObtainQItem->IsQuestItem())
        {
            // Asignar Quest Item a un jugador aleatorio
            nPos = RandomNumber(0, nLimitRandNum);
            // ... agregar a RewardQuestItemMap ...
        }
        else
        {
            // ⭐ PROCESAR ZITEM ⭐
            RewardZItemInfo iteminfo;
            iteminfo.nItemID = pObtainQItem->nItemID;
            iteminfo.nRentPeriodHour = pObtainQItem->nRentPeriodHour;
            
            // Buscar un jugador válido (hasta 5 intentos)
            int nLoopCounter = 0;
            const int MAX_LOOP_COUNT = 5;
            
            while (nLoopCounter < MAX_LOOP_COUNT)
            {
                nLoopCounter++;
                nPos = RandomNumber(0, nLimitRandNum);
                
                if ((nPos < nPlayerCount) && (nPos < (int)a_vecPlayerInfos.size()))
                {
                    MQuestPlayerInfo* pPlayerInfo = a_vecPlayerInfos[nPos];
                    MQuestRewardZItemList* pRewardZItemList = &pPlayerInfo->RewardZItemList;
                    
                    if (IsEnabledObject(pPlayerInfo->pObject))
                    {
                        // Verificar si el item es equipable para este jugador
                        if (IsEquipableItem(iteminfo.nItemID, MAX_LEVEL, 
                                           pPlayerInfo->pObject->GetCharInfo()->m_nSex))
                        {
                            // ⭐ AGREGAR ZITEM A LA LISTA DE RECOMPENSAS ⭐
                            pRewardZItemList->push_back(iteminfo);
                            break;
                        }
                    }
                }
            }
        }
    }
}
```

**Puntos importantes:**
- Los items vienen de `m_pQuestLevel->GetDynamicInfo()->ItemMap` (items obtenidos durante la quest)
- Solo se consideran items con `bObtained = true`
- Los ZItems se asignan aleatoriamente a un jugador que pueda equiparlos
- Se intenta hasta 5 veces encontrar un jugador válido
- Los items se agregan a `RewardZItemList` de cada jugador

### 1. Distribución de Recompensas (Servidor)

Cuando se completa una quest, el servidor llama a `DistributeReward()` en `MatchServer/MMatchRuleQuest.cpp`:

```cpp
void MMatchRuleQuest::DistributeReward()
{
    if (!m_pQuestLevel) return;
    if (MSM_TEST != MGetServerConfig()->GetServerMode()) return;
    
    // ... obtener XP y BP del scenario ...
    
    // ⭐ PASO 1: Construir lista de recompensas ⭐
    MakeRewardList();
    
    // ⭐ PASO 2: Distribuir a cada jugador ⭐
    for (MQuestPlayerManager::iterator itor = m_PlayerManager.begin(); 
         itor != m_PlayerManager.end(); ++itor)
    {
        MQuestPlayerInfo* pPlayerInfo = (*itor).second;
        
        DistributeXPnBP(pPlayerInfo, nRewardXP, nRewardBP, nScenarioQL);
        
        pPlayer = MMatchServer::GetInstance()->GetObject((*itor).first);
        if (!IsEnabledObject(pPlayer)) continue;
        
        // Distribuir Quest Items
        void* pSimpleQuestItemBlob = NULL;
        if (!DistributeQItem(pPlayerInfo, &pSimpleQuestItemBlob)) continue;
        
        // Distribuir ZItems
        void* pSimpleZItemBlob = NULL;
        if (!DistributeZItem(pPlayerInfo, &pSimpleZItemBlob)) continue;
        
        pPlayer->GetCharInfo()->GetDBQuestCachingData().IncreasePlayCount();
        
        // Enviar recompensas al cliente
        RouteRewardCommandToStage(pPlayer, (*itor).second->nXP, 
                                   (*itor).second->nBP, 
                                   pSimpleQuestItemBlob, pSimpleZItemBlob);
        
        MEraseBlobArray(pSimpleQuestItemBlob);
    }
}
```

### 2. Distribución de ZItems (`DistributeZItem`)

Función clave: `MatchServer/MMatchRuleQuest.cpp:877-916`

```cpp
bool MMatchRuleQuest::DistributeZItem(MQuestPlayerInfo* pPlayerInfo, 
                                      void** ppoutQuestRewardZItemBlob)
{
    MMatchObject* pPlayer = pPlayerInfo->pObject;
    if (!IsEnabledObject(pPlayer)) return false;
    
    // Obtener lista de ZItems de recompensa
    MQuestRewardZItemList* pObtainZItemList = &pPlayerInfo->RewardZItemList;
    
    // Crear blob para enviar al cliente
    void* pSimpleZItemBlob = MMakeBlobArray(sizeof(MTD_QuestZItemNode), 
                                            (int)(pObtainZItemList->size()));
    
    int nBlobIndex = 0;
    for (MQuestRewardZItemList::iterator itor = pObtainZItemList->begin(); 
         itor != pObtainZItemList->end(); ++itor)
    {
        RewardZItemInfo iteminfo = (*itor);
        MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(iteminfo.nItemID);
        if (pItemDesc == NULL) continue;
        
        // Verificar si el item es equipable para este personaje
        if (!IsEquipableItem(iteminfo.nItemID, MAX_LEVEL, 
                             pPlayer->GetCharInfo()->m_nSex))
            continue;
        
        // ⭐ AQUÍ SE INSERTA EL ITEM EN LA BASE DE DATOS Y EN EL INVENTARIO ⭐
        MMatchServer::GetInstance()->InsertCharItem(pPlayer->GetUID(), 
                                                     iteminfo.nItemID, 
                                                     true,  // Es rent item
                                                     iteminfo.nRentPeriodHour);
        
        // Preparar nodo para el blob que se enviará al cliente
        MTD_QuestZItemNode* pZItemNode = 
            (MTD_QuestZItemNode*)(MGetBlobArrayElement(pSimpleZItemBlob, nBlobIndex++));
        pZItemNode->m_nItemID = iteminfo.nItemID;
        pZItemNode->m_nRentPeriodHour = iteminfo.nRentPeriodHour;
    }
    
    *ppoutQuestRewardZItemBlob = pSimpleZItemBlob;
    return true;
}
```

**Puntos importantes:**
- Los ZItems se obtienen de `pPlayerInfo->RewardZItemList`
- Se insertan en la base de datos con `InsertCharItem()`
- Se crea un blob con estructura `MTD_QuestZItemNode` para enviar al cliente
- Solo se entregan items equipables para el personaje

### 3. Inserción en Base de Datos (`InsertCharItem`)

Función: `MatchServer/MMatchServer_Item.cpp:26-48`

```cpp
bool MMatchServer::InsertCharItem(const MUID& uidPlayer, const u32 nItemID, 
                                  bool bRentItem, int nRentPeriodHour)
{
    MMatchObject* pObject = GetObject(uidPlayer);
    if (!IsEnabledObject(pObject)) return false;
    
    MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
    if (pItemDesc == NULL) return false;
    
    // Insertar en base de datos
    u32 nNewCIID = 0;
    if (!GetDBMgr()->InsertCharItem(pObject->GetCharInfo()->m_nCID, 
                                     nItemID, bRentItem, nRentPeriodHour, 
                                     &nNewCIID))
        return false;
    
    // Agregar al inventario del objeto en memoria
    int nRentMinutePeriodRemainder = nRentPeriodHour * 60;
    MUID uidNew = MMatchItemMap::UseUID();
    pObject->GetCharInfo()->m_ItemList.CreateItem(uidNew, nNewCIID, nItemID, 
                                                   bRentItem, 
                                                   nRentMinutePeriodRemainder);
    
    return true;
}
```

### 4. Envío al Cliente (`RouteRewardCommandToStage`)

Función: `MatchServer/MMatchRuleQuest.cpp:918-933`

```cpp
void MMatchRuleQuest::RouteRewardCommandToStage(MMatchObject* pPlayer, 
                                                const int nRewardXP, 
                                                const int nRewardBP, 
                                                void* pSimpleQuestItemBlob, 
                                                void* pSimpleZItemBlob)
{
    if (!IsEnabledObject(pPlayer) || (0 == pSimpleQuestItemBlob))
        return;
    
    // Crear comando de recompensa
    MCommand* pNewCmd = MMatchServer::GetInstance()->CreateCommand(
        MC_MATCH_USER_REWARD_QUEST, MUID(0, 0));
    
    // Agregar parámetros: XP, BP, Quest Items blob, ZItems blob
    pNewCmd->AddParameter(new MCmdParamInt(nRewardXP));
    pNewCmd->AddParameter(new MCmdParamInt(nRewardBP));
    pNewCmd->AddParameter(new MCommandParameterBlob(pSimpleQuestItemBlob, 
                                                     MGetBlobArraySize(pSimpleQuestItemBlob)));
    pNewCmd->AddParameter(new MCommandParameterBlob(pSimpleZItemBlob, 
                                                     MGetBlobArraySize(pSimpleZItemBlob)));
    
    // Enviar al cliente
    MMatchServer::GetInstance()->RouteToListener(pPlayer, pNewCmd);
}
```

**Comando:** `MC_MATCH_USER_REWARD_QUEST` (definido en `MSharedCommandTable.h:552`)

### 5. Procesamiento en el Cliente (`OnRewardQuest`)

Función: `Gunz/ZQuest.cpp:886-901`

```cpp
bool ZQuest::OnRewardQuest(MCommand* pCmd)
{
    if (0 == pCmd) return false;
    
    int nXP, nBP;
    pCmd->GetParameter(&nXP, 0, MPT_INT);
    pCmd->GetParameter(&nBP, 1, MPT_INT);
    MCommandParameter* pParam1 = pCmd->GetParameter(2);  // Quest Items blob
    MCommandParameter* pParam2 = pCmd->GetParameter(3);  // ZItems blob
    
    // Procesar y mostrar las recompensas
    GetMyObtainQuestItemList(nXP, nBP, 
                             pParam1->GetPointer(), 
                             pParam2->GetPointer());
    
    return true;
}
```

### 6. Procesamiento de ZItems en el Cliente (`GetMyObtainQuestItemList`)

Función: `Gunz/ZQuest.cpp:997-1010`

```cpp
void ZQuest::GetMyObtainQuestItemList(int nRewardXP, int nRewardBP,
                                      void* pMyObtainQuestItemListBlob, 
                                      void* pMyObtainZItemListBlob)
{
    // ... procesamiento de Quest Items ...
    
    // ⭐ PROCESAR ZITEMS ⭐
    int nZItemCount = MGetBlobArrayCount(pMyObtainZItemListBlob);
    for (int i = 0; i < nZItemCount; i++)
    {
        MTD_QuestZItemNode* pZItemNode = 
            (MTD_QuestZItemNode*)(MGetBlobArrayElement(pMyObtainZItemListBlob, i));
        
        // Mostrar en la lista de recompensas
        if (pListBox)
        {
            MMatchItemDesc* pItemDesc = 
                MGetMatchItemDescMgr()->GetItemDesc(pZItemNode->m_nItemID);
            
            char szMsg[128];
            ZTransMsg(szMsg, MSG_GAME_GET_QUEST_ITEM2, 2, 
                      pItemDesc->m_szName, "1");
            pListBox->Add(new ObtainItemListBoxItem(
                GetItemIconBitmap(pItemDesc, true), szMsg));
        }
    }
}
```

## Sistema de Tracking de Items Durante la Quest

### Registro de Items Obtenidos

Cuando un item se crea durante la quest (por ejemplo, cuando un NPC lo dropea), se registra en el sistema:

**`MQuestLevel::OnItemCreated()`** - `MatchServer/MQuestLevel.cpp:398-406`
```cpp
void MQuestLevel::OnItemCreated(u32 nItemID, int nRentPeriodHour)
{
    MQuestLevelItem* pNewItem = new MQuestLevelItem();
    pNewItem->nItemID = nItemID;
    pNewItem->nRentPeriodHour = nRentPeriodHour;
    pNewItem->bObtained = false;  // Aún no obtenido
    
    // Agregar al mapa de items del nivel
    m_DynamicInfo.ItemMap.insert(make_pair(nItemID, pNewItem));
}
```

**`MQuestLevel::OnItemObtained()`** - `MatchServer/MQuestLevel.cpp:408-426`
```cpp
bool MQuestLevel::OnItemObtained(MMatchObject* pPlayer, u32 nItemID)
{
    if (0 == pPlayer) return false;
    
    // Buscar el item en el mapa
    for (MQuestLevelItemMap::iterator itor = m_DynamicInfo.ItemMap.lower_bound(nItemID);
         itor != m_DynamicInfo.ItemMap.upper_bound(nItemID); ++itor)
    {
        MQuestLevelItem* pQuestItem = (*itor).second;
        if (!pQuestItem->bObtained)
        {
            pQuestItem->bObtained = true;  // ⭐ Marcar como obtenido
            return true;
        }
    }
    
    return false;
}
```

**Estructura `MQuestLevelItem`** - `MatchServer/MQuestLevel.h:66-75`
```cpp
struct MQuestLevelItem
{
    u32     nItemID;            // ID del item
    int     nRentPeriodHour;    // Período de renta (para ZItems)
    bool    bObtained;          // Si el jugador lo obtuvo
    int     nMonsetBibleIndex;
    
    bool IsQuestItem() { return IsQuestItemID(nItemID); }
};
```

### Sistema de Logging de Recompensas

Antes de distribuir los items, el sistema registra qué items se van a entregar:

**`AddRewardZItemInfo()`** - `MatchServer/MMatchQuestGameLog.cpp:81-97`
```cpp
bool MMatchQuestGameLogInfoManager::AddRewardZItemInfo(const MUID& uidPlayer, 
                                                       MQuestRewardZItemList* pObtainZItemList)
{
    MQuestPlayerLogInfo* pQuestPlayerLogInfo = Find(uidPlayer);
    if ((0 == pQuestPlayerLogInfo) || (0 == pObtainZItemList)) return false;
    
    // Registrar cada ZItem obtenido
    for(MQuestRewardZItemList::iterator itor = pObtainZItemList->begin(); 
        itor != pObtainZItemList->end(); ++itor)
    {
        RewardZItemInfo iteminfo = (*itor);
        MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(iteminfo.nItemID);
        if (pItemDesc == NULL) continue;
        
        // Agregar a la lista de items únicos obtenidos
        pQuestPlayerLogInfo->AddUniqueItem(iteminfo.nItemID, 1);
    }
    
    return true;
}
```

## Flujo Secundario: Obtención Durante el Juego

### Cuando el Jugador Recoge un World Item

Cuando un jugador recoge un objeto del mundo durante una quest:

1. **Servidor:** `OnObtainWorldItem()` en `MatchServer/MMatchRuleQuest.cpp:945-962`
   ```cpp
   void MMatchRuleQuest::OnObtainWorldItem(MMatchObject* pObj, int nItemID, 
                                           int* pnExtraValues)
   {
       int nQuestItemID = pnExtraValues[0];
       int nRentPeriodHour = pnExtraValues[1];
       
       if (m_pQuestLevel->OnItemObtained(pObj, (u32)nQuestItemID))
       {
           if (IsQuestItemID(nQuestItemID))
               RouteObtainQuestItem((u32)nQuestItemID);
           else
               RouteObtainZItem((u32)nQuestItemID);  // ⭐ Para ZItems
       }
   }
   ```

2. **Notificación al Cliente:** `RouteObtainZItem()` en `MatchServer/MMatchRuleQuest.cpp:80-84`
   ```cpp
   void MMatchRuleQuest::RouteObtainZItem(u32 nItemID)
   {
       MCommand* pCmd = MMatchServer::GetInstance()->CreateCommand(
           MC_QUEST_OBTAIN_ZITEM, MUID(0, 0));
       pCmd->AddParameter(new MCmdParamUInt(nItemID));
       MMatchServer::GetInstance()->RouteToStage(GetStage()->GetUID(), pCmd);
   }
   ```

3. **Cliente:** `OnObtainZItem()` en `Gunz/ZQuest.cpp:777-795`
   ```cpp
   bool ZQuest::OnObtainZItem(MCommand* pCommand)
   {
       u32 nItemID;
       pCommand->GetParameter(&nItemID, 0, MPT_UINT);
       
       m_GameInfo.IncreaseObtainQuestItem();
       
       // Mostrar mensaje de obtención
       MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
       if (pItemDesc)
       {
           char szMsg[128];
           ZTransMsg(szMsg, MSG_GAME_GET_QUEST_ITEM, 1, pItemDesc->m_szName);
           ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM_GAME), szMsg);
       }
       
       return true;
   }
   ```

## Estructuras de Datos

### `MTD_QuestZItemNode`
Definición: `CSCommon/Include/MMatchTransDataType.h:353-357`

```cpp
struct MTD_QuestZItemNode
{
    unsigned int    m_nItemID;          // ID del item
    int             m_nRentPeriodHour;  // Período de renta en horas (0 = permanente)
};
```

### `RewardZItemInfo`
Definición: `MatchServer/MQuestPlayer.h:11-17`

```cpp
struct RewardZItemInfo
{
    int nItemID;
    int nRentPeriodHour;
};
```

## Comandos de Red

- **`MC_MATCH_USER_REWARD_QUEST`** (21006): Envía todas las recompensas al completar quest
  - Parámetro 0: XP (int)
  - Parámetro 1: BP (int)
  - Parámetro 2: Quest Items blob
  - Parámetro 3: ZItems blob

- **`MC_QUEST_OBTAIN_ZITEM`** (6011): Notifica cuando se obtiene un ZItem durante el juego
  - Parámetro 0: ItemID (uint)

## Archivos Clave

### Servidor
- `MatchServer/MMatchRuleQuest.cpp`: Lógica principal de distribución
- `MatchServer/MMatchServer_Item.cpp`: Inserción de items en BD
- `MatchServer/MMatchRuleBaseQuest.cpp`: Spawn de items en el mundo

### Cliente
- `Gunz/ZQuest.cpp`: Procesamiento de recompensas
- `Gunz/ZWorldItem.cpp`: Manejo de items del mundo

### Estructuras
- `CSCommon/Include/MMatchTransDataType.h`: Estructuras de datos
- `MatchServer/MQuestPlayer.h`: Información de recompensas

## Notas Importantes

1. **Los ZItems se insertan directamente en la base de datos** cuando se entregan como recompensa, no solo cuando el cliente los recibe.

2. **Los ZItems pueden ser rent items** con un período de alquiler especificado en horas.

3. **Solo se entregan items equipables** según el nivel y sexo del personaje.

4. **Los ZItems son diferentes de los Quest Items:**
   - Quest Items: Objetos especiales de quest (almacenados en `m_QuestItemList`)
   - ZItems: Objetos normales del juego (almacenados en `m_ItemList`)

5. **El sistema usa blobs** para enviar múltiples items eficientemente a través de la red.

6. **El sistema rastrea todos los items obtenidos** durante la quest en `ItemMap` y solo entrega los que fueron marcados como `bObtained = true`.

7. **Los items se asignan aleatoriamente** a los jugadores que puedan equiparlos, con hasta 5 intentos para encontrar un jugador válido.

## Flujo Completo Paso a Paso

### Al Completar una Quest:

1. **`DistributeReward()`** se llama
2. **`MakeRewardList()`** construye las listas de recompensas:
   - Itera sobre `m_pQuestLevel->GetDynamicInfo()->ItemMap`
   - Solo considera items con `bObtained = true`
   - Asigna ZItems aleatoriamente a jugadores válidos
   - Agrega a `RewardZItemList` de cada jugador
3. Para cada jugador:
   - **`DistributeZItem()`** procesa `RewardZItemList`:
     - Inserta cada item en la BD con `InsertCharItem()`
     - Crea blob para enviar al cliente
     - Registra en el log de quest
   - **`RouteRewardCommandToStage()`** envía comando al cliente
4. Cliente recibe y procesa:
   - **`OnRewardQuest()`** recibe el comando
   - **`GetMyObtainQuestItemList()`** muestra los items en la UI

### Durante la Quest:

1. NPC dropea un item → **`OnItemCreated()`** lo registra en `ItemMap`
2. Jugador recoge el item → **`OnItemObtained()`** marca `bObtained = true`
3. Si es ZItem → **`RouteObtainZItem()`** notifica al cliente
4. Cliente muestra mensaje de obtención

### Diferencias Clave:

- **Durante la quest:** Solo se notifica, NO se entrega el item aún
- **Al completar la quest:** Se distribuyen todos los items obtenidos según `MakeRewardList()`
- **Los ZItems se insertan en la BD** solo cuando se distribuyen como recompensa final

