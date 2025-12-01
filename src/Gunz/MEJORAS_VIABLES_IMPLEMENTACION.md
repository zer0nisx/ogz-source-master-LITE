# Mejoras Viables para la Tienda - Análisis de Implementación

## Resumen Ejecutivo

Basado en el análisis del código actual (`ZShop.cpp`, `ZShop.h`, `ZEquipmentListBox.cpp`), estas son las mejoras **viables y prácticas** que se pueden implementar **sin requerir cambios en el servidor o modificaciones masivas de UI**.

---

## ✅ Mejoras Viables (Ordenadas por Facilidad)

### 1. 🔍 **Búsqueda de Items por Nombre** ⭐⭐⭐ (FÁCIL)

**Estado actual**: 
- Los botones `EquipmentSearch` y `ShopSearchFrameCaller` existen pero muestran "NOT_SUPPORT"
- La infraestructura de filtrado ya existe (`CheckAddType()`, `Serialize()`)

**Implementación**:
- Agregar campo `std::string m_SearchText` a `ZShop`
- Agregar método `MatchesSearch(u32 nItemID)` que compare nombres
- Modificar `Serialize()` para filtrar por búsqueda además de tipo
- Agregar listener para campo de búsqueda (MEdit widget)

**Código necesario**:
```cpp
// ZShop.h
std::string m_SearchText;

// ZShop.cpp
bool ZShop::MatchesSearch(u32 nItemID) const {
    if (m_SearchText.empty()) return true;
    
    MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
    if (!pItemDesc) return false;
    
    std::string itemName = pItemDesc->m_szName;
    std::string searchLower = m_SearchText;
    // Convertir a minúsculas y comparar
    return itemName.find(searchLower) != std::string::npos;
}

// En Serialize(), agregar antes de CheckAddType():
if (!MatchesSearch(m_ItemVector[i])) continue;
```

**Complejidad**: ⭐⭐ (Baja)
**Tiempo estimado**: 1-2 horas
**Requiere UI**: Sí (campo MEdit, pero puede reutilizar widget existente)

---

### 2. 📊 **Ordenamiento de Items** ⭐⭐ (FÁCIL)

**Estado actual**:
- Los items se muestran en el orden que vienen del servidor
- No hay lógica de ordenamiento

**Implementación**:
- Agregar enum `ShopSortType` y campo `m_SortType` a `ZShop`
- Crear vector temporal con items + metadata (precio, nivel, nombre)
- Ordenar vector antes de serializar
- Agregar ComboBox o botones para seleccionar tipo de ordenamiento

**Código necesario**:
```cpp
// ZShop.h
enum ShopSortType {
    SHOP_SORT_NONE = 0,
    SHOP_SORT_NAME_ASC,
    SHOP_SORT_NAME_DESC,
    SHOP_SORT_PRICE_ASC,
    SHOP_SORT_PRICE_DESC,
    SHOP_SORT_LEVEL_ASC,
    SHOP_SORT_LEVEL_DESC,
};
int m_SortType;

// ZShop.cpp
struct SortableItem {
    u32 nItemID;
    MMatchItemDesc* pItemDesc;
    // ... metadata para ordenar
};

void ZShop::SortItems() {
    if (m_SortType == SHOP_SORT_NONE) return;
    
    std::vector<SortableItem> sortable;
    // Llenar vector con items y metadata
    // Ordenar según m_SortType
    // Actualizar m_ItemVector con orden
}
```

**Complejidad**: ⭐⭐⭐ (Media)
**Tiempo estimado**: 2-3 horas
**Requiere UI**: Sí (ComboBox similar a filtros)

---

### 3. 💰 **Filtro "Puedo Comprar"** ⭐ (MUY FÁCIL)

**Estado actual**:
- Ya se verifica BP en `ZGameInterface::Buy()`, pero no se filtra en la lista

**Implementación**:
- Agregar campo `bool m_bFilterAffordable` a `ZShop`
- Agregar método `CanAffordItem(u32 nItemID)` que compare precio vs BP
- Modificar `Serialize()` para filtrar items no asequibles

**Código necesario**:
```cpp
// ZShop.h
bool m_bFilterAffordable;

// ZShop.cpp
bool ZShop::CanAffordItem(u32 nItemID) const {
    MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
    if (!pItemDesc) return false;
    
    int nBP = ZGetMyInfo()->GetBP();
    return nBP >= pItemDesc->m_nBountyPrice;
}

// En Serialize(), agregar:
if (m_bFilterAffordable && !CanAffordItem(m_ItemVector[i])) continue;
```

**Complejidad**: ⭐ (Muy Baja)
**Tiempo estimado**: 30 minutos - 1 hora
**Requiere UI**: Sí (checkbox simple)

---

### 4. 🎯 **Optimización de CheckAddType()** ⭐ (MUY FÁCIL)

**Estado actual**:
- `CheckAddType()` tiene múltiples `if-else` anidados
- Se puede optimizar con switch o lookup table

**Implementación**:
- Convertir a `switch` statement
- O usar array de funciones/valores para lookup más rápido

**Código necesario**:
```cpp
// ZShop.cpp - Versión optimizada
bool ZShop::CheckAddType(int type) {
    if (m_ListFilter == zshop_item_filter_all) return true;
    
    switch (m_ListFilter) {
        case zshop_item_filter_head:  return type == MMIST_HEAD;
        case zshop_item_filter_chest: return type == MMIST_CHEST;
        case zshop_item_filter_hands:  return type == MMIST_HANDS;
        case zshop_item_filter_legs:   return type == MMIST_LEGS;
        case zshop_item_filter_feet:  return type == MMIST_FEET;
        case zshop_item_filter_melee: return type == MMIST_MELEE;
        case zshop_item_filter_range: return type == MMIST_RANGE;
        case zshop_item_filter_custom: return type == MMIST_CUSTOM;
        case zshop_item_filter_extra: return (type == MMIST_EXTRA) || (type == MMIST_FINGER);
        case zshop_item_filter_quest: return true; // Se maneja en Serialize()
        default: return false;
    }
}
```

**Complejidad**: ⭐ (Muy Baja)
**Tiempo estimado**: 15 minutos
**Requiere UI**: No

---

### 5. 📄 **Paginación de Items** ⭐⭐⭐ (MEDIA)

**Estado actual**:
- `m_nPage` existe pero no se usa
- Todos los items se muestran a la vez

**Implementación**:
- Usar `m_nPage` existente
- Calcular items por página (ej: 20, 50, 100)
- Modificar `Serialize()` para mostrar solo items de la página actual
- Agregar botones "Anterior" / "Siguiente"
- Agregar label "Página X de Y"

**Código necesario**:
```cpp
// ZShop.h
static const int ITEMS_PER_PAGE = 50; // Configurable
int GetTotalPages() const { return (GetItemCount() + ITEMS_PER_PAGE - 1) / ITEMS_PER_PAGE; }
void SetPage(int nPage);

// ZShop.cpp
void ZShop::Serialize() {
    // ... código existente ...
    
    int nStartIndex = m_nPage * ITEMS_PER_PAGE;
    int nEndIndex = min(nStartIndex + ITEMS_PER_PAGE, GetItemCount());
    
    for (int i = nStartIndex; i < nEndIndex; i++) {
        // ... código de agregar items ...
    }
}
```

**Complejidad**: ⭐⭐⭐ (Media)
**Tiempo estimado**: 2-3 horas
**Requiere UI**: Sí (botones y label)

---

### 6. 🔄 **Cache de Items Filtrados** ⭐⭐ (FÁCIL)

**Estado actual**:
- `Serialize()` recalcula todo cada vez que cambia el filtro
- Se puede optimizar cacheando resultados

**Implementación**:
- Agregar vector `m_FilteredItemVector` que se actualiza solo cuando cambian filtros
- Invalidar cache cuando cambia filtro, búsqueda, o se reciben nuevos items

**Código necesario**:
```cpp
// ZShop.h
std::vector<u32> m_FilteredItemVector;
bool m_bFilterCacheValid;

void InvalidateFilterCache();
void UpdateFilterCache();

// ZShop.cpp
void ZShop::UpdateFilterCache() {
    if (m_bFilterCacheValid) return;
    
    m_FilteredItemVector.clear();
    for (int i = 0; i < GetItemCount(); i++) {
        // Aplicar todos los filtros
        if (MatchesSearch(m_ItemVector[i]) && 
            CanAffordItem(m_ItemVector[i]) && 
            CheckAddType(...)) {
            m_FilteredItemVector.push_back(m_ItemVector[i]);
        }
    }
    m_bFilterCacheValid = true;
}
```

**Complejidad**: ⭐⭐ (Baja)
**Tiempo estimado**: 1-2 horas
**Requiere UI**: No

---

## ❌ Mejoras NO Viables (Por Ahora)

### 1. **Comparación de Stats**
- Requiere extraer stats de items (damage, defense, etc.)
- Requiere UI compleja de comparación
- **Complejidad**: ⭐⭐⭐⭐⭐ (Muy Alta)

### 2. **Sistema de Favoritos**
- Requiere persistencia (archivo o servidor)
- Requiere UI adicional
- **Complejidad**: ⭐⭐⭐⭐ (Alta)

### 3. **Carrito de Compras**
- Requiere UI compleja
- Requiere validación de múltiples items
- **Complejidad**: ⭐⭐⭐⭐ (Alta)

### 4. **Historial de Compras**
- Requiere soporte del servidor
- Requiere almacenamiento en BD
- **Complejidad**: ⭐⭐⭐⭐⭐ (Muy Alta)

---

## 🎯 Plan de Implementación Recomendado

### Fase 1 - Mejoras Rápidas (1-2 días)
1. ✅ **Optimización de CheckAddType()** (15 min)
2. ✅ **Filtro "Puedo Comprar"** (1 hora)
3. ✅ **Cache de Items Filtrados** (1-2 horas)

### Fase 2 - Funcionalidades Básicas (2-3 días)
4. ✅ **Búsqueda de Items** (1-2 horas)
5. ✅ **Ordenamiento de Items** (2-3 horas)

### Fase 3 - Mejoras de UX (Opcional)
6. ✅ **Paginación de Items** (2-3 horas)

---

## 📝 Notas de Implementación

### Consideraciones Técnicas

1. **Thread Safety**: 
   - `ZShop` es un singleton, asegurar que `Serialize()` no se llame desde múltiples threads

2. **Performance**:
   - Con muchos items (1000+), la búsqueda puede ser lenta
   - Considerar usar índices o estructuras de datos más eficientes

3. **UI**:
   - Los widgets necesarios pueden ya existir en los archivos IDL
   - Verificar `Shop.xml` o archivos de UI relacionados

4. **Compatibilidad**:
   - Asegurar que las mejoras no rompan código existente
   - Mantener compatibilidad con `_QUEST_ITEM` y otros defines

---

## 🔧 Código Base para Empezar

### Estructura Actual de ZShop

```cpp
class ZShop {
protected:
    int m_nPage;                    // ✅ Existe pero no se usa
    bool m_bCreated;
    std::vector<u32> m_ItemVector;  // Lista de IDs de items
    
public:
    int m_ListFilter;               // Filtro actual
    
    void Serialize();                // Serializa items a UI
    bool CheckAddType(int type);     // Verifica si item pasa filtro
};
```

### Puntos de Extensión

1. **`Serialize()`**: Lugar ideal para agregar filtros adicionales
2. **`CheckAddType()`**: Puede extenderse o crear nuevos métodos de filtrado
3. **`m_ItemVector`**: Puede ordenarse antes de serializar

---

## ✅ Conclusión

Las mejoras más viables son:
1. **Búsqueda** (fácil, alto impacto)
2. **Ordenamiento** (fácil, alto impacto)
3. **Filtro "Puedo Comprar"** (muy fácil, buen impacto)
4. **Optimizaciones** (muy fácil, bajo impacto pero importante)

**Total estimado**: 6-10 horas de desarrollo para implementar las 3 mejoras principales.

