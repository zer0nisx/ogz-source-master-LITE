# Mejoras Funcionales para la Tienda - Propuestas

Este documento propone mejoras funcionales para la tienda desde la perspectiva del usuario final.

## Resumen Ejecutivo

**Funcionalidades actuales**:
- ✅ Filtros básicos por tipo (head, chest, hands, legs, feet, melee, range, custom, extra, quest)
- ✅ Compra y venta de items
- ✅ Vista previa del personaje con items
- ✅ Pestañas (Tienda, Inventario, Cash Shop)
- ✅ Descripción de items

**Funcionalidades propuestas**: 15 mejoras organizadas por prioridad

---

## Prioridad Alta (Impacto Alto, Implementación Media)

### 1. 🔍 Búsqueda de Items por Nombre

**Estado actual**: Los botones `EquipmentSearch` y `ShopSearch` existen pero muestran "NOT_SUPPORT"

**Propuesta**:
```cpp
// Agregar a ZShop.h
std::string m_SearchText;  // Texto de búsqueda actual

// Agregar métodos
void SetSearchText(const char* szText);
void ClearSearch();
bool MatchesSearch(u32 nItemID) const;
```

**Funcionalidad**:
- Campo de búsqueda en la UI
- Búsqueda en tiempo real mientras se escribe
- Búsqueda por nombre de item (case-insensitive)
- Búsqueda parcial (substring matching)
- Botón para limpiar búsqueda

**Beneficio**: Los usuarios pueden encontrar items rápidamente sin navegar por categorías

**Complejidad**: Media (requiere UI y lógica de filtrado)

---

### 2. 📊 Ordenamiento de Items

**Propuesta**:
```cpp
// Agregar a ZShop.h
enum ShopSortType {
    SHOP_SORT_NONE = 0,
    SHOP_SORT_NAME_ASC,
    SHOP_SORT_NAME_DESC,
    SHOP_SORT_PRICE_ASC,
    SHOP_SORT_PRICE_DESC,
    SHOP_SORT_LEVEL_ASC,
    SHOP_SORT_LEVEL_DESC,
    SHOP_SORT_NEWEST,  // Items más recientes primero
};

int m_SortType;  // Tipo de ordenamiento actual

void SetSortType(ShopSortType nType);
void SortItems();
```

**Funcionalidad**:
- ComboBox o botones para seleccionar tipo de ordenamiento
- Ordenar por:
  - Nombre (A-Z, Z-A)
  - Precio (menor a mayor, mayor a menor)
  - Nivel requerido (bajo a alto, alto a bajo)
  - Fecha de adición (más nuevos primero)

**Beneficio**: Los usuarios pueden organizar items según sus necesidades

**Complejidad**: Media (requiere implementar algoritmos de ordenamiento)

---

### 3. 💰 Filtro "Puedo Comprar"

**Propuesta**:
```cpp
// Agregar a ZShop.h
bool m_bFilterAffordable;  // Solo mostrar items que puedo comprar

bool CanAffordItem(u32 nItemID) const;
```

**Funcionalidad**:
- Checkbox "Solo items que puedo comprar"
- Filtra items basado en BP disponible del jugador
- Actualiza automáticamente cuando cambia el BP

**Beneficio**: Los usuarios ven solo items que pueden comprar, evitando frustración

**Complejidad**: Baja (solo verificar precio vs BP)

---

### 4. 📈 Comparación de Stats de Items

**Propuesta**:
```cpp
// Agregar a ZShop.h
u32 m_nCompareItemID1;  // Primer item para comparar
u32 m_nCompareItemID2;  // Segundo item para comparar

void SetCompareItem(int nIndex, u32 nItemID);
void ClearCompare();
void ShowComparison();
```

**Funcionalidad**:
- Click derecho en item → "Comparar"
- Seleccionar dos items para comparar
- Mostrar tabla comparativa:
  - Precio
  - Nivel requerido
  - Stats (damage, defense, weight, etc.)
  - Diferencias resaltadas

**Beneficio**: Los usuarios pueden tomar decisiones informadas antes de comprar

**Complejidad**: Media-Alta (requiere UI de comparación y extracción de stats)

---

## Prioridad Media (Impacto Medio, Implementación Variable)

### 5. ⭐ Sistema de Favoritos/Wishlist

**Propuesta**:
```cpp
// Agregar a ZShop.h
std::vector<u32> m_FavoriteItems;  // Lista de items favoritos

void AddFavorite(u32 nItemID);
void RemoveFavorite(u32 nItemID);
bool IsFavorite(u32 nItemID) const;
void LoadFavorites();  // Desde archivo/config
void SaveFavorites();  // A archivo/config
```

**Funcionalidad**:
- Botón "⭐ Agregar a favoritos" en cada item
- Pestaña "Favoritos" en la tienda
- Notificación cuando un item favorito está en oferta (futuro)
- Persistencia entre sesiones

**Beneficio**: Los usuarios pueden guardar items para comprar después

**Complejidad**: Media (requiere persistencia de datos)

---

### 6. 📄 Paginación de Items

**Estado actual**: `m_nPage` existe pero no se usa

**Propuesta**:
```cpp
// Usar m_nPage existente
void SetPage(int nPage);
int GetTotalPages() const;
void UpdatePaginationUI();
```

**Funcionalidad**:
- Mostrar X items por página (ej: 20, 50, 100)
- Botones "Anterior" / "Siguiente"
- Indicador "Página X de Y"
- Navegación rápida a primera/última página

**Beneficio**: Mejor rendimiento con muchas items, mejor UX

**Complejidad**: Media (requiere UI de paginación)

---

### 7. 🔍 Filtros Avanzados

**Propuesta**:
```cpp
// Agregar a ZShop.h
struct ShopAdvancedFilter {
    int nMinPrice;
    int nMaxPrice;
    int nMinLevel;
    int nMaxLevel;
    bool bOnlyNew;  // Solo items nuevos
    bool bOnlyOwned;  // Solo items que ya tengo
    bool bOnlyNotOwned;  // Solo items que NO tengo
};

ShopAdvancedFilter m_AdvancedFilter;

bool MatchesAdvancedFilter(u32 nItemID) const;
```

**Funcionalidad**:
- Panel de filtros avanzados (colapsable)
- Filtro por rango de precio
- Filtro por rango de nivel requerido
- Filtro "Solo items nuevos" (agregados recientemente)
- Filtro "Solo items que ya tengo" / "Solo items que NO tengo"
- Combinar múltiples filtros

**Beneficio**: Búsqueda muy específica de items

**Complejidad**: Media-Alta (requiere UI compleja)

---

### 8. 📊 Vista de Estadísticas al Equipar

**Propuesta**:
```cpp
// Agregar a ZCharacterView o crear nueva clase
void ShowStatsComparison(u32 nCurrentItemID, u32 nNewItemID);
void UpdateStatsDisplay();
```

**Funcionalidad**:
- Al hacer hover sobre item en la tienda, mostrar:
  - Stats actuales del personaje
  - Stats si equipa este item
  - Diferencias (verde para mejoras, rojo para empeoramientos)
- Panel lateral con comparación de stats

**Beneficio**: Los usuarios ven el impacto antes de comprar

**Complejidad**: Media-Alta (requiere cálculo de stats y UI)

---

### 9. 🛒 Carrito de Compras

**Propuesta**:
```cpp
// Agregar a ZShop.h
struct CartItem {
    u32 nItemID;
    int nQuantity;
    int nPrice;
};

std::vector<CartItem> m_Cart;

void AddToCart(u32 nItemID, int nQuantity = 1);
void RemoveFromCart(u32 nItemID);
void ClearCart();
int GetCartTotal() const;
void BuyCart();  // Comprar todos los items del carrito
```

**Funcionalidad**:
- Botón "Agregar al carrito" en cada item
- Icono de carrito con contador de items
- Panel de carrito con lista de items
- Total del carrito
- Botón "Comprar todo"
- Validación de BP suficiente para todo el carrito

**Beneficio**: Compra múltiple sin tener que comprar uno por uno

**Complejidad**: Media (requiere UI de carrito)

---

### 10. 📜 Historial de Compras

**Propuesta**:
```cpp
// Agregar a ZShop.h
struct PurchaseHistory {
    u32 nItemID;
    u32 dwPurchaseDate;
    int nPrice;
    bool bSold;  // Si fue vendido después
};

std::vector<PurchaseHistory> m_PurchaseHistory;

void AddToHistory(u32 nItemID, int nPrice);
void LoadHistory();  // Desde servidor o archivo local
void ShowHistory();
```

**Funcionalidad**:
- Pestaña "Historial" en la tienda
- Lista de compras recientes (últimos 30 días)
- Mostrar: Item, fecha, precio, si fue vendido
- Click en item del historial → mostrar en tienda

**Beneficio**: Los usuarios pueden ver qué compraron y cuándo

**Complejidad**: Media (requiere persistencia y UI)

---

## Prioridad Baja (Mejoras de UX)

### 11. 🎨 Vista Grid vs Lista

**Propuesta**:
```cpp
// Agregar a ZShop.h
enum ShopViewMode {
    SHOP_VIEW_LIST = 0,
    SHOP_VIEW_GRID,
};

int m_ViewMode;

void SetViewMode(ShopViewMode nMode);
void ToggleViewMode();
```

**Funcionalidad**:
- Botón para cambiar entre vista de lista y vista de grid
- Vista grid: iconos grandes, menos información
- Vista lista: iconos pequeños, más información
- Guardar preferencia del usuario

**Beneficio**: Flexibilidad según preferencia del usuario

**Complejidad**: Media (requiere dos layouts diferentes)

---

### 12. 🔔 Notificaciones de Items Nuevos

**Propuesta**:
```cpp
// Agregar a ZShop.h
std::set<u32> m_NewItems;  // Items agregados recientemente
u32 m_dwLastShopUpdate;  // Última vez que se actualizó la tienda

void MarkAsNew(u32 nItemID);
bool IsNewItem(u32 nItemID) const;
void ClearNewItems();
```

**Funcionalidad**:
- Badge "NUEVO" en items agregados recientemente
- Notificación cuando hay items nuevos
- Filtro "Solo items nuevos"
- Los items dejan de ser "nuevos" después de X días

**Beneficio**: Los usuarios descubren nuevos items fácilmente

**Complejidad**: Baja (solo marcar items como nuevos)

---

### 13. 🔗 Items Similares / Recomendados

**Propuesta**:
```cpp
// Agregar a ZShop.h
std::vector<u32> GetSimilarItems(u32 nItemID) const;
std::vector<u32> GetRecommendedItems() const;
```

**Funcionalidad**:
- Al seleccionar un item, mostrar "Items similares"
- Basado en:
  - Mismo tipo/slot
  - Precio similar
  - Stats similares
- Sección "Recomendados para ti" basado en nivel y items actuales

**Beneficio**: Descubrimiento de items relacionados

**Complejidad**: Media (requiere algoritmo de similitud)

---

### 14. 📱 Atajos de Teclado

**Propuesta**:
```cpp
// Agregar a ZGameInterface
void OnShopKeyPress(int nKey);
```

**Funcionalidad**:
- `Ctrl+F`: Abrir búsqueda
- `Ctrl+B`: Comprar item seleccionado
- `Ctrl+S`: Vender item seleccionado
- `Esc`: Cerrar tienda
- `Tab`: Cambiar entre pestañas
- `F`: Agregar a favoritos
- `C`: Comparar items

**Beneficio**: Navegación más rápida para usuarios avanzados

**Complejidad**: Baja (solo manejo de teclado)

---

### 15. 💾 Guardar Filtros y Preferencias

**Propuesta**:
```cpp
// Agregar a ZShop.h
void SavePreferences();  // Guardar a archivo/config
void LoadPreferences();  // Cargar desde archivo/config
```

**Funcionalidad**:
- Guardar filtros activos
- Guardar tipo de ordenamiento
- Guardar modo de vista (grid/lista)
- Guardar preferencias de búsqueda
- Restaurar al abrir la tienda

**Beneficio**: Los usuarios no tienen que reconfigurar cada vez

**Complejidad**: Baja (solo persistencia de configuración)

---

## Implementación Sugerida (Orden de Prioridad)

### Fase 1 - Funcionalidades Básicas (Alto Impacto)
1. ✅ **Búsqueda de Items** (#1)
2. ✅ **Ordenamiento** (#2)
3. ✅ **Filtro "Puedo Comprar"** (#3)

### Fase 2 - Funcionalidades Avanzadas
4. ✅ **Comparación de Stats** (#4)
5. ✅ **Sistema de Favoritos** (#5)
6. ✅ **Paginación** (#6)

### Fase 3 - Mejoras de UX
7. ✅ **Filtros Avanzados** (#7)
8. ✅ **Vista Grid/Lista** (#11)
9. ✅ **Atajos de Teclado** (#14)

### Fase 4 - Funcionalidades Adicionales
10. ✅ **Carrito de Compras** (#9)
11. ✅ **Historial de Compras** (#10)
12. ✅ **Items Similares** (#13)

---

## Consideraciones Técnicas

### Cambios Necesarios en el Servidor

Algunas funcionalidades pueden requerir cambios en el servidor:
- **Historial de Compras**: Requiere almacenamiento en BD
- **Items Nuevos**: Requiere tracking de fecha de adición
- **Items Similares**: Puede calcularse en cliente o servidor

### Cambios Necesarios en la UI (IDL)

- Agregar widgets para búsqueda
- Agregar widgets para ordenamiento
- Agregar widgets para filtros avanzados
- Agregar widgets para comparación
- Agregar widgets para carrito

### Persistencia de Datos

- **Favoritos**: Guardar en archivo local o servidor
- **Preferencias**: Guardar en archivo de configuración
- **Historial**: Requiere soporte del servidor

---

## Ejemplo de Implementación: Búsqueda de Items

### 1. Agregar a ZShop.h
```cpp
class ZShop {
    // ... código existente ...
    
    // Búsqueda
    std::string m_SearchText;
    void SetSearchText(const char* szText);
    void ClearSearch();
    bool MatchesSearch(u32 nItemID) const;
};
```

### 2. Implementar en ZShop.cpp
```cpp
void ZShop::SetSearchText(const char* szText)
{
    if (szText)
        m_SearchText = szText;
    else
        m_SearchText.clear();
    
    // Convertir a minúsculas para búsqueda case-insensitive
    std::transform(m_SearchText.begin(), m_SearchText.end(), 
                   m_SearchText.begin(), ::tolower);
    
    Serialize();  // Re-serializar con nuevo filtro
}

bool ZShop::MatchesSearch(u32 nItemID) const
{
    if (m_SearchText.empty())
        return true;
    
    MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
    if (!pItemDesc)
        return false;
    
    // Búsqueda case-insensitive
    std::string itemName = pItemDesc->m_szName;
    std::transform(itemName.begin(), itemName.end(), 
                   itemName.begin(), ::tolower);
    
    return itemName.find(m_SearchText) != std::string::npos;
}

void ZShop::Serialize()
{
    // ... código existente ...
    
    for (int i = 0; i < GetItemCount(); i++) {
        // ... código existente de filtrado por tipo ...
        
        // Agregar filtro de búsqueda
        if (!MatchesSearch(m_ItemVector[i]))
            continue;
        
        // ... resto del código ...
    }
}
```

### 3. Agregar Listener para el campo de búsqueda
```cpp
// En ZInterfaceListener.cpp
BEGIN_IMPLEMENT_LISTENER(ZGetShopSearchListener, MINT_EDIT_MSG)
    MEdit* pEdit = (MEdit*)pWidget;
    if (pEdit) {
        const char* szText = pEdit->GetText();
        ZGetShop()->SetSearchText(szText);
    }
END_IMPLEMENT_LISTENER()
```

---

## Métricas de Éxito

### Antes de Implementar
- Tiempo promedio para encontrar un item: ~30-60 segundos
- Número de clicks para comprar un item: ~5-7
- Tasa de abandono de tienda: Desconocida

### Después de Implementar (Objetivos)
- Tiempo promedio para encontrar un item: ~5-10 segundos (con búsqueda)
- Número de clicks para comprar un item: ~2-3 (con atajos)
- Tasa de abandono de tienda: Reducción del 50%

---

## Recomendación Final

**Empezar con**:
1. Búsqueda de Items (#1) - Alto impacto, implementación media
2. Ordenamiento (#2) - Alto impacto, implementación media
3. Filtro "Puedo Comprar" (#3) - Alto impacto, implementación baja

Estas tres funcionalidades mejoran significativamente la experiencia del usuario sin requerir cambios complejos en el servidor.

---

**Fecha**: Generado automáticamente  
**Propuestas**: 15  
**Prioridad Alta**: 4  
**Prioridad Media**: 6  
**Prioridad Baja**: 5

