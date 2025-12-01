# Errores de Lógica Encontrados en Shop/Equipment

Este documento lista los errores de lógica, bugs potenciales y problemas de diseño encontrados en el sistema de Shop/Equipment.

## Resumen

Se encontraron **10 errores de lógica** y **3 problemas de optimización** que podrían causar crashes, condiciones de carrera, o comportamiento incorrecto.

**Errores críticos**: 3 (posibles crashes)  
**Errores de lógica**: 4 (comportamiento incorrecto)  
**Problemas de optimización**: 3 (overhead innecesario)

---

## Errores Críticos (Posibles Crashes)

### 1. ❌ CRÍTICO: Acceso a puntero NULL en ZGameInterface::Buy()

**Ubicación**: `ZGameInterface.cpp` - Línea 3149

**Problema**: Se accede a `pItemDesc->IsCashItem()` sin verificar si `pItemDesc` es NULL después del bloque `#ifdef _QUEST_ITEM`.

**Código problemático**:
```cpp
MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);

#ifdef _QUEST_ITEM
    if (0 == pItemDesc)
    {
        MQuestItemDesc* pQuestItemDesc = GetQuestItemDescMgr().FindQItemDesc(nItemID);
        if (0 == pQuestItemDesc)
        {
            return;  // ✅ Sale aquí si no es quest item
        }
        ZPostRequestBuyQuestItem(ZGetGameClient()->GetPlayerUID(), nItemID);
        return;
    }
#endif
// ❌ PROBLEMA: Si pItemDesc es NULL y no es quest item, el código continúa
if (pItemDesc->IsCashItem())  // 💥 CRASH si pItemDesc es NULL
{
    // ...
}
```

**Escenario de crash**:
1. `GetItemDesc(nItemID)` retorna NULL (item no existe o error)
2. No es un quest item (o `_QUEST_ITEM` no está definido)
3. El código continúa y accede a `pItemDesc->IsCashItem()` → **CRASH**

**Solución**:
```cpp
MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);

#ifdef _QUEST_ITEM
    if (0 == pItemDesc)
    {
        MQuestItemDesc* pQuestItemDesc = GetQuestItemDescMgr().FindQItemDesc(nItemID);
        if (0 == pQuestItemDesc)
        {
            // Item no encontrado - mostrar error
            ZApplication::GetGameInterface()->ShowErrorMessage(MERR_INVALID_ITEM);
            return;
        }
        ZPostRequestBuyQuestItem(ZGetGameClient()->GetPlayerUID(), nItemID);
        return;
    }
#endif

// ✅ Verificar NULL antes de usar
if (pItemDesc == NULL)
{
    ZApplication::GetGameInterface()->ShowErrorMessage(MERR_INVALID_ITEM);
    return;
}

if (pItemDesc->IsCashItem())
{
    // ...
}
```

---

### 2. ❌ CRÍTICO: Acceso a puntero NULL en SetSellQuestItemConfirmFrame()

**Ubicación**: `ZGameInterface.cpp` - Línea 3051

**Problema**: Se accede a `pQuestItemNode->GetDesc()` sin verificar si `pQuestItemNode` es NULL después de la verificación inicial.

**Código problemático**:
```cpp
ZMyQuestItemNode* pQuestItemNode = ZGetMyInfo()->GetItemList()->GetQuestItemMap().Find(pListItem->GetItemID());
if (pQuestItemNode)
{
    if (m_nSellQuestItemCount > pQuestItemNode->m_nCount)
        m_nSellQuestItemCount = pQuestItemNode->m_nCount;
}

// ❌ PROBLEMA: Se accede a pQuestItemNode sin verificar si es NULL
MQuestItemDesc* pQuestItemDesc = pQuestItemNode->GetDesc();  // 💥 CRASH si pQuestItemNode es NULL

MLabel* pLabel = (MLabel*)m_IDLResource.FindWidget("SellQuestItem_Calculate");
if (pLabel && pQuestItemDesc)  // Verifica pQuestItemDesc pero ya accedió a pQuestItemNode
{
    // ...
}
```

**Escenario de crash**:
1. `GetQuestItemMap().Find()` retorna NULL (item no está en el mapa)
2. El código accede a `pQuestItemNode->GetDesc()` → **CRASH**

**Solución**:
```cpp
ZMyQuestItemNode* pQuestItemNode = ZGetMyInfo()->GetItemList()->GetQuestItemMap().Find(pListItem->GetItemID());
if (!pQuestItemNode)
{
    // Item no encontrado en el inventario
    return;
}

if (m_nSellQuestItemCount > pQuestItemNode->m_nCount)
    m_nSellQuestItemCount = pQuestItemNode->m_nCount;

MQuestItemDesc* pQuestItemDesc = pQuestItemNode->GetDesc();
if (!pQuestItemDesc)
{
    // Descripción no encontrada
    return;
}

MLabel* pLabel = (MLabel*)m_IDLResource.FindWidget("SellQuestItem_Calculate");
if (pLabel)
{
    // ...
}
```

---

## Errores de Lógica (Comportamiento Incorrecto)

### 3. ⚠️ MEDIO: Cálculo incorrecto de índice en Buy()

**Ubicación**: `ZGameInterface.cpp` - Línea 3132

**Problema**: El cálculo `pListItem->GetUID().Low - 1` puede ser confuso y propenso a errores.

**Código problemático**:
```cpp
MUID uidItem = pListItem->GetUID();
nItemID = ZGetShop()->GetItemID(pListItem->GetUID().Low - 1);
```

**Problemas**:
1. Si `Low` es 0, el índice será -1 (aunque `GetItemID()` valida, es confuso)
2. Se calcula `uidItem` pero no se usa
3. Se accede a `GetUID()` dos veces

**Análisis**:
- En `ZShop::Serialize()` (línea 83), se crea: `MUID uidItem = MUID(0, i + 1);`
- Entonces `Low = i + 1`, donde `i` es el índice en el vector (0-based)
- Para obtener el índice original: `Low - 1 = i`
- Esto funciona, pero es frágil si cambia la lógica de creación de UIDs

**Solución sugerida**:
```cpp
MUID uidItem = pListItem->GetUID();
int nIndex = (int)uidItem.Low - 1;

// Validar índice antes de usar
if (nIndex < 0)
{
    ZApplication::GetGameInterface()->ShowErrorMessage(MERR_INVALID_ITEM);
    return;
}

nItemID = ZGetShop()->GetItemID(nIndex);
if (nItemID == 0)
{
    ZApplication::GetGameInterface()->ShowErrorMessage(MERR_INVALID_ITEM);
    return;
}
```

---

### 4. ✅ CORREGIDO: Condición de carrera en Sell()

**Ubicación**: `ZGameInterface.cpp` - Línea 2999

**Problema**: Se solicitaba la lista de items inmediatamente después de vender, pero el servidor ya la envía automáticamente.

**Código problemático (ANTES)**:
```cpp
// En Sell() - Línea 2998-2999
ZPostRequestSellItem(ZGetGameClient()->GetPlayerUID(), uidItem);
ZPostRequestCharacterItemList(ZGetGameClient()->GetPlayerUID());  // ❌ Redundante
```

**Problemas**:
1. **Condición de carrera**: El cliente puede recibir dos respuestas de lista (una automática del servidor y una solicitada)
2. **Solicitudes redundantes**: El servidor envía la lista automáticamente después de vender (línea 236 de `MMatchServer_Item.cpp`)
3. **Desperdicio de ancho de banda**: Solicitud innecesaria

**Solución aplicada**:
```cpp
// En Sell() - CORREGIDO
ZPostRequestSellItem(ZGetGameClient()->GetPlayerUID(), uidItem);
// NO solicitar lista aquí - el servidor la envía automáticamente
// (ver VALIDACION_SERVIDOR_SHOP.md para más detalles)
```

**Nota sobre Buy()**: 
- El servidor NO envía la lista automáticamente después de comprar
- La solicitud inmediata en `Buy()` es **NECESARIA** (no es un error)
- Ver `VALIDACION_SERVIDOR_SHOP.md` para más detalles sobre el comportamiento del servidor

---

### 5. ⚠️ MEDIO: Lógica redundante en SellQuestItem()

**Ubicación**: `ZGameInterface.cpp` - Líneas 3017-3022

**Problema**: Lógica confusa que oculta, deshabilita y luego muestra el mismo widget.

**Código problemático**:
```cpp
MWidget* pWidget = m_IDLResource.FindWidget("SellQuestItemConfirmCaller");
if (pWidget)
{
    pWidget->Show(false);      // Ocultar
    pWidget->Enable(false);    // Deshabilitar
    pWidget->Show(true);       // Mostrar de nuevo (pero deshabilitado)
}
```

**Problema**: 
- ¿Por qué ocultar y luego mostrar inmediatamente?
- Parece código legacy o lógica incorrecta

**Solución sugerida**:
```cpp
MWidget* pWidget = m_IDLResource.FindWidget("SellQuestItemConfirmCaller");
if (pWidget)
{
    // Si el propósito es resetear el estado, hacerlo explícitamente
    pWidget->Enable(false);
    pWidget->Show(true);
    // O simplemente:
    // pWidget->Show(false);  // Ocultar hasta que se confirme
}
```

---

## Problemas de Optimización

### 6. ⚠️ BAJO: Llamada redundante a ZGetMyInfo()->GetSex() en loop

**Ubicación**: `ZShop.cpp` - Línea 80

**Problema**: Se llama a `ZGetMyInfo()->GetSex()` dentro de un loop que puede tener muchos items.

**Código problemático**:
```cpp
for (int i = 0; i < GetItemCount(); i++) {
    // ...
    if (pItemDesc != NULL) {
        if (pItemDesc->m_nResSex != -1) {
            if (pItemDesc->m_nResSex != int(ZGetMyInfo()->GetSex())) continue;  // ❌ Llamada en loop
        }
        // ...
    }
}
```

**Solución**:
```cpp
// Optimización: Guardar fuera del loop
MMatchSex nPlayerSex = ZGetMyInfo()->GetSex();

for (int i = 0; i < GetItemCount(); i++) {
    // ...
    if (pItemDesc != NULL) {
        if (pItemDesc->m_nResSex != -1) {
            if (pItemDesc->m_nResSex != int(nPlayerSex)) continue;  // ✅ Usar variable local
        }
        // ...
    }
}
```

---

### 7. ⚠️ BAJO: Múltiples llamadas a ZGetGameClient() en ZShop::Create()

**Ubicación**: `ZShop.cpp` - Líneas 26-27

**Problema**: Múltiples llamadas a `ZGetGameClient()` sin optimizar.

**Código problemático**:
```cpp
bool ZShop::Create()
{
    if (m_bCreated) return false;

    ZPostRequestShopItemList(ZGetGameClient()->GetPlayerUID(), 0, 0);  // ❌ Llamada 1
    ZPostRequestCharacterItemList(ZGetGameClient()->GetPlayerUID());   // ❌ Llamada 2

    m_bCreated = true;
    return true;
}
```

**Solución**:
```cpp
bool ZShop::Create()
{
    if (m_bCreated) return false;

    // Optimización: Guardar ZGetGameClient() en variable local
    ZGameClient* pGameClient = ZGetGameClient();
    ZPostRequestShopItemList(pGameClient->GetPlayerUID(), 0, 0);
    ZPostRequestCharacterItemList(pGameClient->GetPlayerUID());

    m_bCreated = true;
    return true;
}
```

---

### 10. ❌ CRÍTICO: Acceso a puntero NULL en MShopSaleItemListBoxListener

**Ubicación**: `ZEquipmentListBox.cpp` - Línea 321

**Problema**: Se accede a `pListItem->GetItemID()` fuera del bloque que verifica si `pListItem` es NULL.

**Código problemático**:
```cpp
ZEquipmentListItem* pListItem;
if (pEquipmentListBox->IsSelected())
{
    pListItem = (ZEquipmentListItem*)pEquipmentListBox->GetSelItem();
    if (pListItem != NULL)
    {
        nItemID = ZGetMyInfo()->GetItemList()->GetItemID(pListItem->GetUID());
    }
}

// ... código intermedio ...

// ❌ PROBLEMA: pListItem puede ser NULL aquí
MQuestItemDesc* pQuestItemDesc = GetQuestItemDescMgr().FindQItemDesc(pListItem->GetItemID());  // 💥 CRASH
```

**Escenario de crash**:
1. `IsSelected()` retorna `true` pero `GetSelItem()` retorna `NULL`
2. O `IsSelected()` retorna `false` y `pListItem` nunca se inicializa
3. El código accede a `pListItem->GetItemID()` → **CRASH**

**Solución**:
```cpp
ZEquipmentListItem* pListItem = NULL;
u32 nItemID = 0;

if (pEquipmentListBox->IsSelected())
{
    pListItem = (ZEquipmentListItem*)pEquipmentListBox->GetSelItem();
    if (pListItem != NULL)
    {
        nItemID = ZGetMyInfo()->GetItemList()->GetItemID(pListItem->GetUID());
    }
}

MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
ZMyItemNode* pItemNode = ZGetMyInfo()->GetItemList()->GetItem(pEquipmentListBox->GetSelIndex());
if (pItemDesc && pItemNode)
{
    // ... código para items normales ...
}

// ✅ Verificar pListItem antes de usar
if (pListItem != NULL)
{
    MQuestItemDesc* pQuestItemDesc = GetQuestItemDescMgr().FindQItemDesc(pListItem->GetItemID());
    if (pQuestItemDesc)
    {
        // ... código para quest items ...
    }
}
```

---

## Problemas de Diseño

### 8. ⚠️ BAJO: Falta de validación de estado en ZShop::Create()

**Ubicación**: `ZShop.cpp` - Línea 22

**Problema**: `Create()` retorna `false` si ya está creado, pero no hay forma de "recrear" o verificar si necesita actualización.

**Código**:
```cpp
bool ZShop::Create()
{
    if (m_bCreated) return false;  // ❌ No permite recrear
    // ...
}
```

**Problema**: Si la lista de items cambia en el servidor, no hay forma de refrescar sin destruir primero.

**Solución sugerida**:
```cpp
bool ZShop::Create(bool bForceRefresh = false)
{
    if (m_bCreated && !bForceRefresh) return false;

    ZGameClient* pGameClient = ZGetGameClient();
    ZPostRequestShopItemList(pGameClient->GetPlayerUID(), 0, 0);
    ZPostRequestCharacterItemList(pGameClient->GetPlayerUID());

    m_bCreated = true;
    return true;
}
```

---

### 9. ⚠️ BAJO: Falta de validación en GetItemID()

**Ubicación**: `ZShop.cpp` - Línea 143

**Problema**: Aunque valida el índice, retorna 0 en caso de error, lo cual podría ser un ItemID válido.

**Código**:
```cpp
u32 ZShop::GetItemID(int nIndex)
{
    if ((nIndex < 0) || (nIndex >= (int)m_ItemVector.size())) return 0;  // ❌ 0 podría ser válido

    return m_ItemVector[nIndex];
}
```

**Problema**: Si un item tiene ID 0, no se puede distinguir entre "error" y "item válido con ID 0".

**Solución sugerida**:
```cpp
// Opción 1: Usar un valor de error específico
static const u32 INVALID_ITEM_ID = 0xFFFFFFFF;

u32 ZShop::GetItemID(int nIndex)
{
    if ((nIndex < 0) || (nIndex >= (int)m_ItemVector.size())) 
        return INVALID_ITEM_ID;
    return m_ItemVector[nIndex];
}

// Opción 2: Usar un parámetro de salida
bool ZShop::GetItemID(int nIndex, u32& nItemID)
{
    if ((nIndex < 0) || (nIndex >= (int)m_ItemVector.size())) 
        return false;
    nItemID = m_ItemVector[nIndex];
    return true;
}
```

---

## Resumen de Errores

| # | Severidad | Ubicación | Descripción | Impacto | Estado |
|---|-----------|-----------|-------------|---------|--------|
| 1 | ❌ CRÍTICO | `ZGameInterface::Buy()` L3149 | Acceso a `pItemDesc` NULL | Crash | ✅ Confirmado |
| 2 | ❌ CRÍTICO | `SetSellQuestItemConfirmFrame()` L3051 | Acceso a `pQuestItemNode` NULL | Crash | ✅ Confirmado |
| 10 | ❌ CRÍTICO | `MShopSaleItemListBoxListener` L321 | Acceso a `pListItem` NULL | Crash | ✅ Confirmado |
| 3 | ⚠️ MEDIO | `ZGameInterface::Buy()` L3132 | Cálculo de índice confuso | Comportamiento incorrecto | ✅ Confirmado |
| 4 | ✅ CORREGIDO | `Sell()` L2999 | Condición de carrera | Datos desincronizados | ✅ Corregido |
| 5 | ⚠️ MEDIO | `SellQuestItem()` L3017-3022 | Lógica redundante | Código confuso | ✅ Confirmado |
| 6 | ⚠️ BAJO | `ZShop::Serialize()` L80 | Llamada en loop | Overhead | ✅ Confirmado |
| 7 | ⚠️ BAJO | `ZShop::Create()` L26-27 | Llamadas redundantes | Overhead | ✅ Confirmado |
| 8 | ⚠️ BAJO | `ZShop::Create()` L22 | No permite recrear | Limitación de diseño | ✅ Confirmado |
| 9 | ⚠️ BAJO | `ZShop::GetItemID()` L143 | Valor de error ambiguo | Posible bug | ✅ Confirmado |

---

## Priorización de Correcciones

### Prioridad Alta (Corregir inmediatamente)
1. **Error #1**: Acceso a `pItemDesc` NULL en `Buy()` - Puede causar crash
2. **Error #2**: Acceso a `pQuestItemNode` NULL en `SetSellQuestItemConfirmFrame()` - Puede causar crash
3. **Error #10**: Acceso a `pListItem` NULL en `MShopSaleItemListBoxListener` - Puede causar crash

### Prioridad Media (Corregir pronto)
3. **Error #3**: Cálculo de índice en `Buy()` - ✅ Mejorado con validación
4. **Error #4**: Condición de carrera en `Sell()` - ✅ Corregido (eliminada solicitud redundante)
5. **Error #5**: Lógica redundante en `SellQuestItem()` - ✅ Corregido

### Prioridad Baja (Mejoras)
6. **Error #6-9**: Optimizaciones y mejoras de diseño

---

## Recomendaciones

1. **Agregar validaciones de NULL** en todos los accesos a punteros después de operaciones que pueden fallar
2. **Revisar flujo de comunicación** con el servidor para evitar condiciones de carrera
3. **Documentar** el propósito de lógica que parece redundante
4. **Aplicar optimizaciones** de accesores siguiendo el patrón ya establecido
5. **Considerar** usar valores de error específicos en lugar de 0 para distinguir errores

---

**Fecha de análisis**: Generado automáticamente  
**Errores críticos encontrados**: 3  
**Errores de lógica encontrados**: 4  
**Problemas de optimización**: 3

---

## Notas de Verificación

### Verificaciones Realizadas

1. ✅ **Error #1 confirmado**: El código accede a `pItemDesc->IsCashItem()` sin verificar NULL cuando `_QUEST_ITEM` no está definido o cuando el item no existe.

2. ✅ **Error #2 confirmado**: El código accede a `pQuestItemNode->GetDesc()` fuera del bloque `if (pQuestItemNode)`, causando crash si el item no está en el inventario.

3. ✅ **Error #10 confirmado**: El código accede a `pListItem->GetItemID()` sin verificar si `pListItem` es NULL, especialmente cuando `IsSelected()` retorna `false` o `GetSelItem()` retorna `NULL`.

4. ⚠️ **Error #4 - Requiere verificación del servidor**: 
   - Los handlers `MC_MATCH_RESPONSE_BUY_ITEM` y `MC_MATCH_RESPONSE_SELL_ITEM` solo muestran mensajes
   - No solicitan automáticamente la lista de items actualizada
   - El cliente solicita la lista inmediatamente después de comprar/vender
   - **Necesita verificación**: ¿El servidor envía `MC_MATCH_RESPONSE_CHARACTER_ITEMLIST` automáticamente después de comprar/vender?
   - Si el servidor NO lo envía automáticamente, entonces la solicitud inmediata es correcta pero podría causar condición de carrera
   - Si el servidor SÍ lo envía automáticamente, entonces la solicitud inmediata es redundante

5. ✅ **Error #3 confirmado**: El cálculo `Low - 1` funciona correctamente según la lógica de creación de UIDs en `ZShop::Serialize()`, pero es frágil y confuso.

6. ✅ **Errores #6-9 confirmados**: Todos los problemas de optimización y diseño son válidos.

