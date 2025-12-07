# Problema: Los Chests No Entregan Items de la Drop Table

## Análisis del Problema

Cuando recoges un chest (cofre) durante una quest, actualmente **NO está entregando ningún objeto de la tabla de drop**. 

### Flujo Actual

1. **NPC muere** → `CheckRewards()` en `MMatchRuleBaseQuest.cpp:172-219`
   - Hace roll de la drop table
   - Spawnea un chest (ID 51, `QUEST_WORLDITEM_ITEMBOX_ID`) con el item en `ExtraValues`
   - El chest tiene:
     - `nWorldItemExtraValues[0]` = ItemID del drop
     - `nWorldItemExtraValues[1]` = RentPeriodHour

2. **Jugador recoge el chest** → `OnObtainWorldItem()` en `MMatchRuleQuest.cpp:945-962`
   ```cpp
   void MMatchRuleQuest::OnObtainWorldItem(MMatchObject* pObj, int nItemID, int* pnExtraValues)
   {
       if (0 == pObj) return;
       if (m_nCombatState != MQUEST_COMBAT_PLAY) return;
       
       int nQuestItemID = pnExtraValues[0];  // ⚠️ Aquí está el problema
       int nRentPeriodHour = pnExtraValues[1];
       
       if (m_pQuestLevel->OnItemObtained(pObj, (u32)nQuestItemID))
       {
           if (IsQuestItemID(nQuestItemID))
               RouteObtainQuestItem((u32)nQuestItemID);
           else
               RouteObtainZItem((u32)nQuestItemID);
       }
   }
   ```

### El Problema

El código actual **solo procesa el item que está en los ExtraValues del chest**. Esto funciona cuando:
- Un NPC dropea un item específico que ya está determinado
- El chest ya tiene el item asignado en los ExtraValues

**PERO** el problema es que:
- Si los `pnExtraValues[0]` están en 0 o vacíos, no se hace nada
- Si el chest se spawnea sin items en los ExtraValues, no hay drop
- **No hay lógica para hacer roll de una drop table cuando abres un chest**

### Posibles Causas

1. **Los chests se spawnean sin ExtraValues**: Los chests en el mapa pueden no tener items asignados
2. **Los ExtraValues están en 0**: El chest existe pero no tiene item configurado
3. **Falta lógica de drop table en chests**: Los chests estáticos del mapa no tienen drop table asociada

## Soluciones Posibles

### Opción 1: Agregar Drop Table a Chests del Mapa

Si los chests son objetos estáticos del mapa, necesitas:

1. **Configurar drop table en el XML del mapa** para cada chest
2. **Modificar `OnObtainWorldItem()`** para hacer roll cuando se recoge un chest sin item:
   ```cpp
   void MMatchRuleQuest::OnObtainWorldItem(MMatchObject* pObj, int nItemID, int* pnExtraValues)
   {
       if (0 == pObj) return;
       if (m_nCombatState != MQUEST_COMBAT_PLAY) return;
       
       int nQuestItemID = pnExtraValues[0];
       int nRentPeriodHour = pnExtraValues[1];
       
       // ⭐ NUEVO: Si el chest no tiene item, hacer roll de drop table
       if (nItemID == QUEST_WORLDITEM_ITEMBOX_ID && nQuestItemID == 0)
       {
           // Buscar drop table configurada para este chest
           int nDropTableID = GetChestDropTableID(/* algún identificador del chest */);
           if (nDropTableID > 0)
           {
               MQuestDropItem dropItem;
               int nQL = m_pQuestLevel->GetStaticInfo()->nQL;
               if (MMatchServer::GetInstance()->GetQuest()->GetDropTable()->Roll(dropItem, nDropTableID, nQL))
               {
                   // Spawnear el item o darlo directamente
                   CheckRewards(pObj->GetUID(), &dropItem, pObj->GetPosition());
               }
           }
           return;
       }
       
       // Código existente...
       if (m_pQuestLevel->OnItemObtained(pObj, (u32)nQuestItemID))
       {
           if (IsQuestItemID(nQuestItemID))
               RouteObtainQuestItem((u32)nQuestItemID);
           else
               RouteObtainZItem((u32)nQuestItemID);
       }
   }
   ```

### Opción 2: Verificar por qué los ExtraValues están vacíos

Si los chests deberían tener items pero no los tienen:

1. **Revisar cómo se spawnean los chests** en el mapa
2. **Verificar que los chests del NPC drop tengan ExtraValues** configurados
3. **Agregar logs** para ver qué contiene `pnExtraValues` cuando se recoge

### Opción 3: Asignar Drop Table Directa al Chest

Si quieres que cada chest tenga su propia drop table:

1. **Agregar campo en WorldItemDesc** para drop table ID
2. **Configurar en worlditem.xml**:
   ```xml
   <WORLDITEM id="51" name="itembox">
       <TYPE>quest</TYPE>
       <DROPTABLE>1</DROPTABLE>  <!-- ID de drop table -->
       ...
   </WORLDITEM>
   ```
3. **Hacer roll cuando se recoge el chest**

## Recomendación

Primero, **verifica qué está pasando**:

1. Agrega logs en `OnObtainWorldItem()` para ver:
   - Qué `nItemID` tiene el chest
   - Qué valores tienen `pnExtraValues[0]` y `pnExtraValues[1]`
   - Si el chest es de tipo QUEST (ID 51)

2. Revisa cómo se spawnean los chests:
   - ¿Son estáticos del mapa?
   - ¿Vienen de NPC drops?
   - ¿Tienen configuración de drop table?

3. Una vez identificado el problema específico, implementa la solución correspondiente.

## Archivos a Revisar

- `MatchServer/MMatchRuleQuest.cpp:945-962` - `OnObtainWorldItem()`
- `MatchServer/MMatchRuleBaseQuest.cpp:172-219` - `CheckRewards()` (NPC drops)
- `Gunz/XML/worlditem.xml` - Configuración de world items
- `Gunz/XML/droptable.xml` - Tablas de drop
- `MatchServer/MMatchWorldItem.cpp:304-332` - `Obtain()` (cuando se recoge)

## Código de Debug Sugerido

Agrega esto temporalmente para ver qué está pasando:

```cpp
void MMatchRuleQuest::OnObtainWorldItem(MMatchObject* pObj, int nItemID, int* pnExtraValues)
{
    if (0 == pObj) return;
    if (m_nCombatState != MQUEST_COMBAT_PLAY) return;
    
    // ⭐ DEBUG: Log para ver qué está pasando
    mlog("OnObtainWorldItem - ItemID: %d, ExtraValues[0]: %d, ExtraValues[1]: %d\n", 
         nItemID, pnExtraValues ? pnExtraValues[0] : -1, pnExtraValues ? pnExtraValues[1] : -1);
    
    int nQuestItemID = pnExtraValues ? pnExtraValues[0] : 0;
    int nRentPeriodHour = pnExtraValues ? pnExtraValues[1] : 0;
    
    if (nQuestItemID == 0) {
        mlog("⚠️ CHEST SIN ITEM! ItemID del chest: %d\n", nItemID);
        // Aquí necesitas agregar lógica para hacer roll de drop table
        return;
    }
    
    if (m_pQuestLevel->OnItemObtained(pObj, (u32)nQuestItemID))
    {
        if (IsQuestItemID(nQuestItemID))
            RouteObtainQuestItem((u32)nQuestItemID);
        else
            RouteObtainZItem((u32)nQuestItemID);
    }
}
```

