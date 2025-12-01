# Correcciones Adicionales Pendientes - Shop y Equipamiento

Este documento lista las correcciones adicionales que podemos aplicar para mejorar aún más el código.

## Resumen

**Correcciones pendientes**: 5
- **Prioridad Alta**: 1 (condición de carrera)
- **Prioridad Media**: 2 (optimizaciones)
- **Prioridad Baja**: 2 (mejoras de diseño)

---

## Correcciones Pendientes

### 1. ⚠️ MEDIO: Condición de carrera en CharacterEquipmentItemListBoxOnDrop()

**Ubicación**: `ZEquipmentListBox.cpp` - Línea 272

**Problema**: Se solicita la lista de items inmediatamente después de desequipar, pero el servidor puede enviarla automáticamente si `UPDATE_STAGE_EQUIP_LOOK` está definido.

**Código actual**:
```cpp
void CharacterEquipmentItemListBoxOnDrop(...)
{
    // ...
    ZPostRequestTakeoffItem(ZGetGameClient()->GetPlayerUID(), pItemSlotView->GetParts());
    ZPostRequestCharacterItemList(ZGetGameClient()->GetPlayerUID());  // ❌ Siempre solicita
}
```

**Problema**:
- Si `UPDATE_STAGE_EQUIP_LOOK` está definido, el servidor envía la lista automáticamente (línea 607 de `MMatchServer_Item.cpp`)
- La solicitud del cliente es redundante y puede causar condición de carrera
- Inconsistente con `ZGameInterface::Equip()` y `ZItemSlotView::OnDrop()` que ya manejan esto correctamente

**Solución**:
```cpp
void CharacterEquipmentItemListBoxOnDrop(...)
{
    // ...
    ZPostRequestTakeoffItem(ZGetGameClient()->GetPlayerUID(), pItemSlotView->GetParts());
    // The server sends this automatically if UPDATE_STAGE_EQUIP_LOOK is defined.
#ifndef UPDATE_STAGE_EQUIP_LOOK
    ZPostRequestCharacterItemList(ZGetGameClient()->GetPlayerUID());
#endif
}
```

**Consistencia**: Esto hace que el código sea consistente con:
- `ZGameInterface::Equip()` (línea 3302-3304)
- `ZItemSlotView::OnDrop()` (línea 306-308)

---

### 2. ⚠️ BAJO: Llamadas redundantes a ZGetMyInfo() en InitializeEquipmentInformation()

**Ubicación**: `ZGameInterface.cpp` - Líneas 4117-4118

**Problema**: Se llama a `ZGetMyInfo()` dos veces consecutivas.

**Código actual**:
```cpp
static void InitializeEquipmentInformation(ZIDLResource* pResource)
{
    // ...
    ZMyInfo* pmi = ZGetMyInfo();           // ❌ Llamada 1
    ZMyItemList* pil = ZGetMyInfo()->GetItemList();  // ❌ Llamada 2
    // ...
}
```

**Solución**:
```cpp
static void InitializeEquipmentInformation(ZIDLResource* pResource)
{
    // ...
    // Optimización: Guardar ZGetMyInfo() en variable local
    ZMyInfo* pmi = ZGetMyInfo();
    ZMyItemList* pil = pmi->GetItemList();  // ✅ Usar variable local
    // ...
}
```

---

### 3. ⚠️ BAJO: Optimización de ZGetGameClient() en CharacterEquipmentItemListBoxOnDrop()

**Ubicación**: `ZEquipmentListBox.cpp` - Líneas 271-272

**Problema**: Se llama a `ZGetGameClient()` dos veces.

**Código actual**:
```cpp
void CharacterEquipmentItemListBoxOnDrop(...)
{
    // ...
    ZPostRequestTakeoffItem(ZGetGameClient()->GetPlayerUID(), ...);  // ❌ Llamada 1
    ZPostRequestCharacterItemList(ZGetGameClient()->GetPlayerUID()); // ❌ Llamada 2
}
```

**Solución**:
```cpp
void CharacterEquipmentItemListBoxOnDrop(...)
{
    // ...
    // Optimización: Guardar ZGetGameClient() en variable local
    ZGameClient* pGameClient = ZGetGameClient();
    ZPostRequestTakeoffItem(pGameClient->GetPlayerUID(), pItemSlotView->GetParts());
#ifndef UPDATE_STAGE_EQUIP_LOOK
    ZPostRequestCharacterItemList(pGameClient->GetPlayerUID());
#endif
}
```

---

### 4. ⚠️ BAJO: Mejora de diseño en ZShop::Create()

**Ubicación**: `ZShop.cpp` - Línea 22

**Problema**: `Create()` no permite recrear sin destruir primero.

**Código actual**:
```cpp
bool ZShop::Create()
{
    if (m_bCreated) return false;  // ❌ No permite recrear
    // ...
}
```

**Solución sugerida**:
```cpp
bool ZShop::Create(bool bForceRefresh = false)
{
    if (m_bCreated && !bForceRefresh) return false;

    // Optimización: Guardar ZGetGameClient() en variable local
    ZGameClient* pGameClient = ZGetGameClient();
    ZPostRequestShopItemList(pGameClient->GetPlayerUID(), 0, 0);
    ZPostRequestCharacterItemList(pGameClient->GetPlayerUID());

    m_bCreated = true;
    return true;
}
```

**Beneficio**: Permite refrescar la lista de items sin necesidad de destruir y recrear.

---

### 5. ⚠️ BAJO: Mejora de validación en ZShop::GetItemID()

**Ubicación**: `ZShop.cpp` - Línea 143

**Problema**: Retorna 0 en caso de error, pero 0 podría ser un ItemID válido.

**Código actual**:
```cpp
u32 ZShop::GetItemID(int nIndex)
{
    if ((nIndex < 0) || (nIndex >= (int)m_ItemVector.size())) return 0;  // ❌ Ambiguo
    return m_ItemVector[nIndex];
}
```

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

// Y actualizar Buy() para verificar:
nItemID = ZGetShop()->GetItemID(nIndex);
if (nItemID == INVALID_ITEM_ID)  // En lugar de == 0
{
    ZApplication::GetGameInterface()->ShowErrorMessage(MERR_INVALID_ITEM);
    return;
}
```

**Nota**: Esta es una mejora de diseño que requiere actualizar el código que usa `GetItemID()`. Actualmente funciona porque validamos `nIndex` antes de llamar, pero sería más robusto.

---

## Priorización

### Prioridad Alta
1. **Corrección #1**: Condición de carrera en `CharacterEquipmentItemListBoxOnDrop()` - Hacer consistente con el resto del código

### Prioridad Media
2. **Corrección #2**: Optimizar `ZGetMyInfo()` en `InitializeEquipmentInformation()`
3. **Corrección #3**: Optimizar `ZGetGameClient()` en `CharacterEquipmentItemListBoxOnDrop()`

### Prioridad Baja
4. **Corrección #4**: Mejora de diseño en `ZShop::Create()` - Agregar parámetro `bForceRefresh`
5. **Corrección #5**: Mejora de validación en `ZShop::GetItemID()` - Usar valor de error específico

---

## Impacto Esperado

### Correcciones #1, #2, #3
- **Rendimiento**: Reducción de llamadas redundantes a funciones singleton
- **Consistencia**: Código más consistente y predecible
- **Robustez**: Eliminación de condiciones de carrera potenciales

### Correcciones #4, #5
- **Mantenibilidad**: Código más claro y fácil de mantener
- **Robustez**: Mejor manejo de casos límite

---

## Recomendación

Aplicar las correcciones en este orden:
1. ✅ **Corrección #1** (Prioridad Alta) - Elimina condición de carrera
2. ✅ **Corrección #2 y #3** (Prioridad Media) - Optimizaciones rápidas
3. ⚠️ **Corrección #4 y #5** (Prioridad Baja) - Mejoras de diseño (opcional, requiere más cambios)

---

**Fecha**: Generado automáticamente  
**Correcciones identificadas**: 5  
**Impacto total**: Bajo-Medio (optimizaciones y mejoras de diseño)

