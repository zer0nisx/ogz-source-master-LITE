# Análisis de Clases: Shop Logic y UI

Este documento describe todas las clases involucradas en la lógica y UI del sistema de tienda (Shop) y equipamiento (Equipment) en Gunz.

## Resumen Ejecutivo

El sistema de Shop/Equipment está compuesto por **15+ clases principales** organizadas en capas de:
- **Lógica de negocio**: Gestión de items, compra/venta
- **UI Components**: Listas, slots, vistas de personaje
- **Event Handlers**: Listeners para interacciones del usuario
- **Data Models**: Representación de items y equipamiento

---

## Arquitectura General

```
┌─────────────────────────────────────────────────────────┐
│              ZGameInterface (Orquestador)               │
│  - ShowShopOrEquipmentDialog()                          │
│  - Buy() / Sell() / Equip()                            │
└──────────────┬──────────────────────────────────────────┘
               │
       ┌───────┴────────┬──────────────────┐
       │                 │                  │
┌──────▼──────┐  ┌──────▼──────┐  ┌───────▼────────┐
│   ZShop     │  │ ZEquipment   │  │ ZMyItemList   │
│  (Lógica)   │  │  ListBox     │  │  (Inventario) │
└──────┬──────┘  └──────┬──────┘  └───────┬────────┘
       │                 │                  │
       └─────────────────┼──────────────────┘
                         │
              ┌──────────▼──────────┐
              │ ZEquipmentListItem │
              │   (Item UI)        │
              └────────────────────┘
```

---

## Clases Principales

### 1. ZShop (Lógica de Tienda)

**Archivo**: `ZShop.h`, `ZShop.cpp`

**Responsabilidad**: Gestión central de la lógica de la tienda.

**Miembros principales**:
```cpp
class ZShop {
protected:
    int m_nPage;                    // Página actual
    bool m_bCreated;                 // Estado de creación
    std::vector<u32> m_ItemVector;  // Lista de IDs de items disponibles
    
public:
    int m_ListFilter;               // Filtro actual (head, chest, etc.)
    
    // Métodos principales
    bool Create();                  // Inicializa y solicita lista de items
    void Destroy();                 // Limpia recursos
    void Serialize();               // Serializa items a UI
    void SetItemsAll(u32* nItemList, int nItemCount);
    bool CheckAddType(int type);    // Verifica si item pasa el filtro
    u32 GetItemID(int nIndex);
    static ZShop* GetInstance();    // Singleton
};
```

**Patrón**: Singleton (GetInstance())

**Filtros disponibles**:
- `zshop_item_filter_all` - Todos los items
- `zshop_item_filter_head` - Cascos
- `zshop_item_filter_chest` - Pecho
- `zshop_item_filter_hands` - Manos
- `zshop_item_filter_legs` - Piernas
- `zshop_item_filter_feet` - Pies
- `zshop_item_filter_melee` - Armas cuerpo a cuerpo
- `zshop_item_filter_range` - Armas a distancia
- `zshop_item_filter_custom` - Items personalizados
- `zshop_item_filter_extra` - Extras
- `zshop_item_filter_quest` - Items de quest

**Flujo de trabajo**:
1. `Create()` → Solicita lista de items al servidor
2. `SetItemsAll()` → Recibe lista del servidor
3. `Serialize()` → Pobla la UI con los items filtrados

---

### 2. ZEquipmentListItem (Item UI)

**Archivo**: `ZEquipmentListBox.h`

**Responsabilidad**: Representa un item individual en las listas de la UI.

**Jerarquía**: `MListItem` → `ZEquipmentListItem`

**Miembros principales**:
```cpp
class ZEquipmentListItem : public MListItem {
protected:
    MBitmap* m_pBitmap;      // Icono del item
    int m_nAIID;             // ID de AI (para items de AI)
    u32 m_nItemID;           // ID del item
    
public:
    MUID m_UID;              // UID único del item
    char m_szName[256];      // Nombre del item
    char m_szLevel[256];     // Nivel requerido
    char m_szPrice[256];     // Precio
    
    // Constructores
    ZEquipmentListItem(const MUID& uidItem, u32 nItemID, 
                      MBitmap* pBitmap, const char* szName, 
                      const char* szLevel, const char* szPrice);
    
    // Métodos virtuales de MListItem
    virtual const char* GetString(int i);  // Obtiene string por índice
    virtual MBitmap* GetBitmap(int i);     // Obtiene bitmap por índice
    virtual bool GetDragItem(...);         // Para drag & drop
    
    // Getters
    MUID& GetUID();
    int GetAIID();
    u32 GetItemID();
};
```

**Uso**:
- Representa items en listas de compra (`AllEquipmentList`)
- Representa items en inventario (`MyAllEquipmentList`)
- Representa items de cash shop (`CashEquipmentList`)

---

### 3. ZEquipmentListBox (Lista de Items)

**Archivo**: `ZEquipmentListBox.h`, `ZEquipmentListBox.cpp`

**Responsabilidad**: Contenedor de lista para mostrar items en la UI.

**Jerarquía**: `MListBox` → `ZEquipmentListBox`

**Miembros principales**:
```cpp
class ZEquipmentListBox : public MListBox {
protected:
    MWidget* m_pDescFrame;          // Frame de descripción
    ZItemMenu* m_pItemMenu;         // Menú contextual
    
public:
    ZEquipmentListBox(const char* szName, MWidget* pParent = NULL, 
                     MListener* pListener = NULL);
    
    // Métodos de agregado
    void Add(const MUID& uidItem, u32 nItemID, MBitmap* pIconBitmap,
            const char* szName, const char* szLevel, const char* szPrice);
    void Add(const MUID& uidItem, u32 nItemID, MBitmap* pIconBitmap,
            const char* szName, int nLevel, int nBountyPrice);
    void Add(const int nAIID, u32 nItemID, MBitmap* pIconBitmap,
            const char* szName, int nLevel);
    
    // Configuración
    void AttachMenu(ZItemMenu* pMenu);
    void SetDescriptionWidget(MWidget* pWidget);
    
    // Eventos
    virtual bool IsDropable(MWidget* pSender);  // Drag & drop
    virtual bool OnEvent(MEvent* pEvent, MListener* pListener);
    
    // Estado
    u32 m_dwLastMouseMove;
    int m_nLastItem;
};
```

**Widgets IDL asociados**:
- `"AllEquipmentList"` - Lista de items disponibles para compra
- `"MyAllEquipmentList"` - Lista de items del inventario del jugador
- `"CashEquipmentList"` - Lista de items de cash shop

**Funcionalidades**:
- Drag & Drop con `ZItemSlotView`
- Menú contextual con `ZItemMenu`
- Descripción de items al hacer hover
- Selección de items

---

### 4. ZItemSlotView (Slot de Equipamiento)

**Archivo**: `ZItemSlotView.h`, `ZItemSlotView.cpp`

**Responsabilidad**: Representa un slot donde se puede equipar un item.

**Jerarquía**: `MButton` → `ZItemSlotView`

**Miembros principales**:
```cpp
class ZItemSlotView : public MButton {
protected:
    MBitmap* m_pBackBitmap;          // Bitmap de fondo
    MMatchCharItemParts m_nParts;    // Parte del cuerpo (head, chest, etc.)
    bool m_bSelectBox;               // Mostrar caja de selección
    bool m_bDragAndDrop;             // Habilitar drag & drop
    bool m_bKindable;                // Si acepta items "kindable"
    
public:
    char m_szItemSlotPlace[128];     // Identificador del slot
    
    // Métodos
    MMatchCharItemParts GetParts();
    void SetParts(MMatchCharItemParts nParts);
    void SetBackBitmap(MBitmap* pBitmap);
    void SetIConBitmap(MBitmap* pBitmap);
    void EnableDragAndDrop(bool bEnable);
    void SetKindable(bool bKindable);
    
    // Eventos
    virtual bool IsDropable(MWidget* pSender);
    virtual bool OnDrop(MWidget* pSender, MBitmap* pBitmap, 
                      const char* szString, const char* szItemString);
    virtual bool OnEvent(MEvent* pEvent, MListener* pListener);
    
    // Validación
    bool IsEquipableItem(u32 nItemID, int nPlayerLevel, MMatchSex nPlayerSex);
};
```

**Uso**:
- Slots de equipamiento en la UI de Equipment
- Permite drag & drop desde `ZEquipmentListBox`
- Valida si un item puede ser equipado

---

### 5. ZCharacterView (Vista de Personaje)

**Archivo**: `ZCharacterView.h`, `ZCharacterView.cpp`

**Responsabilidad**: Muestra una vista 3D del personaje con el equipamiento actual.

**Jerarquía**: `ZMeshView` → `ZCharacterView`

**Miembros principales**:
```cpp
class ZCharacterView : public ZMeshView {
public:
    ZCharacterViewInfo m_Info;           // Info del personaje
    ZItemSlot m_ItemSlots[MMCIP_END];   // Slots de items
    ZMeshView* m_pItemMeshView[MMCIP_END]; // Vistas de mesh de items
    
protected:
    bool m_bVisibleEquipment;            // Mostrar equipamiento
    MMatchCharItemParts m_nVisualWeaponParts; // Parte de arma visible
    bool m_bAutoRotate;                  // Auto-rotación
    u32 m_dwTime;                        // Tiempo para animación
    
public:
    // Inicialización
    void InitCharParts(ZCharacterView* pCharView, 
                      MMatchCharItemParts nVisualWeaponParts = MMCIP_PRIMARY);
    void InitCharParts(MMatchSex nSex, unsigned int nHair, 
                      unsigned int nFace, u32* nEquipItemIDs,
                      MMatchCharItemParts nVisualWeaponParts = MMCIP_PRIMARY);
    
    // Configuración
    void SetParts(MMatchCharItemParts nParts, unsigned int nItemID);
    void ChangeVisualWeaponParts(MMatchCharItemParts nVisualWeaponParts);
    void SetVisibleEquipment(bool bVisible);
    void SetSelectMyCharacter();
    void EnableAutoRotate(bool bAutoRotate);
    
    // Eventos
    virtual bool IsDropable(MWidget* pSender);
    virtual bool OnDrop(MWidget* pSender, MBitmap* pBitmap, ...);
};
```

**Widgets IDL asociados**:
- `"EquipmentInformation"` - Vista del personaje en Equipment
- `"EquipmentInformationShop"` - Vista del personaje en Shop

**Funcionalidades**:
- Renderizado 3D del personaje
- Visualización de items equipados
- Drag & drop para equipar items
- Auto-rotación para preview

---

### 6. ZItemMenu (Menú Contextual)

**Archivo**: `ZItemMenu.h`, `ZItemMenu.cpp`

**Responsabilidad**: Menú contextual que aparece al hacer clic derecho en items.

**Jerarquía**: `MPopupMenu` → `ZItemMenu`

**Miembros principales**:
```cpp
class ZItemMenu : public MPopupMenu {
protected:
    char m_szItemName[128];    // Nombre del item objetivo
    MUID m_ItemUID;            // UID del item objetivo
    
public:
    void AddMenuItem(ZItemMenuItem* pMenuItem);
    void SetupMenu();
    virtual void Show(int x, int y, bool bVisible = true);
    
    const char* GetTargetName();
    void SetTargetName(const char* pszItemName);
    const MUID& GetTargetUID();
    void SetTargetUID(const MUID& uidTarget);
};

class ZItemMenuItem : public MMenuItem {
protected:
    ZCMD_ITEMMENU m_nCmdID;    // ID del comando
    
public:
    ZItemMenuItem(ZCMD_ITEMMENU nCmdID, const char* szName = NULL);
    ZCMD_ITEMMENU GetCmdID();
};
```

**Comandos disponibles**:
- `ZCMD_ITEMMENU_BRINGBACK_ACCOUNTITEM` - Traer item de cuenta

---

### 7. ZMyItemList (Inventario del Jugador)

**Archivo**: `ZMyItemList.h`, `ZMyItemList.cpp`

**Responsabilidad**: Gestiona el inventario del jugador (items poseídos).

**Miembros principales**:
```cpp
class ZMyItemNode : public MBaseItem {
protected:
    u32 m_nItemID;                    // ID del item
    MUID m_UID;                       // UID único
    u64 m_dwWhenReceivedClock;        // Cuándo fue recibido
    
public:
    void Create(MUID& uidItem, u32 nItemID, bool bIsRentItem = false,
               int nRentMinutePeriodRemainder = RENT_MINUTE_PERIOD_UNLIMITED);
    u32 GetItemID() const;
    MUID& GetUID();
};

class ZMyQuestItemNode {
public:
    u32 m_nItemID;
    int m_nCount;
    MQuestItemDesc* m_pDesc;
    
    void Increase(int nCount = 1);
    void Decrease(int nCount = 1);
    u32 GetItemID();
    int GetCount();
    MQuestItemDesc* GetDesc();
};

class ZMyItemList {
    // Gestión de items normales
    MITEMNODEMAP m_ItemNodeMap;        // Mapa de items por UID
    MACCOUNT_ITEMNODEMAP m_AccountItemNodeMap; // Items de cuenta
    
    // Gestión de items de quest
    ZMyQuestItemMap m_QuestItemMap;    // Mapa de items de quest
    
    // Items equipados
    u32 m_EquipedItemID[MMCIP_END];    // IDs de items equipados
    
    // Filtros
    int m_ListFilter;                  // Filtro actual para UI
    
public:
    // Operaciones con items
    ZMyItemNode* GetItem(int nIndex);
    ZMyItemNode* GetItem(MUID uidItem);
    u32 GetItemID(MUID uidItem);
    void AddItem(MUID uidItem, u32 nItemID, ...);
    void RemoveItem(MUID uidItem);
    
    // Equipamiento
    u32 GetEquipedItemID(MMatchCharItemParts nParts);
    void SetEquipedItem(MMatchCharItemParts nParts, MUID uidItem);
    
    // Serialización a UI
    void Serialize();
    
    // Quest items
    ZMyQuestItemNode* GetQuestItem(u32 nItemID);
    ZMyQuestItemMap& GetQuestItemMap();
};
```

**Funcionalidades**:
- Gestión de inventario del jugador
- Items equipados
- Items de quest (con contador)
- Serialización a UI para mostrar en listas

---

## Event Listeners (Manejadores de Eventos)

### 8. MShopSaleItemListBoxListener

**Responsabilidad**: Maneja eventos de la lista de items del inventario (venta).

**Eventos**:
- `MLB_ITEM_SEL` - Selección de item
- `MLB_ITEM_DBLCLK` - Doble clic en item

**Acciones**:
- Muestra descripción del item
- Habilita/deshabilita botón de venta
- Maneja items de quest

---

### 9. MShopPurchaseItemListBoxListener

**Responsabilidad**: Maneja eventos de la lista de items disponibles para compra.

**Eventos**:
- `MLB_ITEM_SEL` - Selección de item
- `MLB_ITEM_DBLCLK` - Doble clic en item

**Acciones**:
- Muestra descripción del item
- Actualiza vista del personaje con preview
- Habilita botón de compra

---

### 10. MCashShopItemListBoxListener

**Responsabilidad**: Maneja eventos de la lista de items de cash shop.

**Eventos**:
- `MLB_ITEM_SEL` - Selección de item
- `MLB_ITEM_DBLCLK` - Doble clic en item

**Acciones**:
- Muestra preview del item en el personaje
- Habilita botón de compra con cash

---

### 11. MEquipmentItemListBoxListener

**Responsabilidad**: Maneja eventos de la lista de items en la UI de Equipment.

**Eventos**:
- `MLB_ITEM_SEL` - Selección de item
- `MLB_ITEM_DBLCLK` - Doble clic para equipar

**Acciones**:
- Muestra descripción del item
- Permite equipar items

---

### 12. MShopAllEquipmentFilterListener

**Responsabilidad**: Maneja cambios en el filtro de categorías del shop.

**Eventos**:
- `MCMBBOX_CHANGED` - Cambio en el ComboBox de filtro

**Acciones**:
- Actualiza filtro en `ZShop`
- Re-serializa lista de items
- Actualiza filtro en `ZMyItemList`

---

### 13. MEquipAllEquipmentFilterListener

**Responsabilidad**: Maneja cambios en el filtro de categorías del equipment.

**Eventos**:
- `MCMBBOX_CHANGED` - Cambio en el ComboBox de filtro

**Acciones**:
- Actualiza filtro en `ZMyItemList`
- Re-serializa lista de items

---

## Funciones de Utilidad

### 14. GetItemIconBitmap()

**Archivo**: `ZEquipmentListBox.cpp`

**Responsabilidad**: Obtiene el bitmap del icono de un item según su tipo.

**Parámetros**:
- `MMatchItemDesc* pItemDesc` - Descripción del item
- `bool bSmallIcon` - Si usar icono pequeño

**Retorna**: `MBitmap*` - Bitmap del icono

**Lógica**:
- Determina el nombre del archivo según el tipo de item
- Maneja variantes (cash items, small icons)
- Retorna el bitmap desde `MBitmapManager`

---

### 15. ZGetIsCashItem()

**Archivo**: `ZEquipmentListBox.cpp`

**Responsabilidad**: Verifica si un item es un cash item.

**Parámetros**:
- `u32 nItemID` - ID del item

**Retorna**: `bool` - True si es cash item

---

## Funciones de Drag & Drop

### 16. ShopPurchaseItemListBoxOnDrop()

**Responsabilidad**: Maneja drop de items en la lista de compra (deshabilitado).

---

### 17. ShopSaleItemListBoxOnDrop()

**Responsabilidad**: Maneja drop de items en la lista de venta (deshabilitado).

---

### 18. CharacterEquipmentItemListBoxOnDrop()

**Responsabilidad**: Maneja drop de items desde inventario a slots de equipamiento.

**Acciones**:
- Valida que el origen sea un `ZItemSlotView`
- Envía comando al servidor para desequipar item
- Solicita actualización de lista de items

---

## Integración con ZGameInterface

### Métodos Principales

**ShowShopOrEquipmentDialog(bool Shop)**
- Muestra el diálogo de Shop o Equipment
- Inicializa la UI correspondiente
- Configura widgets y listeners

**Buy()**
- Procesa compra de item
- Valida selección
- Maneja items normales vs cash items
- Envía comando al servidor

**Sell()**
- Procesa venta de item
- Valida selección
- Maneja items normales vs quest items
- Envía comando al servidor

**Equip()**
- Equipa un item seleccionado
- Valida que el item sea equipable
- Envía comando al servidor

**SelectShopTab(int nTabIndex)**
- Cambia entre pestañas del shop:
  - Tab 0: AllEquipmentList (todos los items)
  - Tab 1: MyAllEquipmentList (inventario)
  - Tab 2: CashEquipmentList (cash shop)

**SelectEquipmentTab(int nTabIndex)**
- Cambia entre pestañas del equipment

---

## Flujo de Datos

### Compra de Item

```
Usuario selecciona item
    ↓
MShopPurchaseItemListBoxListener::OnCommand()
    ↓
ZGameInterface::Buy()
    ↓
ZPostRequestBuyItem() → Servidor
    ↓
Servidor responde
    ↓
ZGameInterface::OnResponseCharacterItemList()
    ↓
ZMyItemList actualizado
    ↓
UI actualizada
```

### Venta de Item

```
Usuario selecciona item del inventario
    ↓
MShopSaleItemListBoxListener::OnCommand()
    ↓
ZGameInterface::Sell()
    ↓
ZPostRequestSellItem() → Servidor
    ↓
Servidor responde
    ↓
ZGameInterface::OnResponseCharacterItemList()
    ↓
ZMyItemList actualizado
    ↓
UI actualizada
```

### Equipamiento de Item

```
Usuario hace drag & drop o doble clic
    ↓
ZItemSlotView::OnDrop() o MEquipmentItemListBoxListener
    ↓
ZPostRequestEquipItem() → Servidor
    ↓
Servidor responde
    ↓
ZGameInterface::OnResponseCharacterItemList()
    ↓
ZMyItemList actualizado
    ↓
ZCharacterView actualizado
    ↓
UI actualizada
```

---

## Widgets IDL Principales

### Shop UI
- `"Shop"` - Frame principal del shop
- `"AllEquipmentList"` - Lista de items disponibles
- `"MyAllEquipmentList"` - Lista de inventario
- `"CashEquipmentList"` - Lista de cash shop
- `"Shop_AllEquipmentFilter"` - ComboBox de filtro
- `"Shop_ItemDescription1/2/3"` - Áreas de descripción
- `"Shop_ItemIcon"` - Icono del item
- `"BuyConfirmCaller"` - Botón de compra
- `"BuyCashConfirmCaller"` - Botón de compra cash
- `"SellConfirmCaller"` - Botón de venta

### Equipment UI
- `"Equipment"` - Frame principal del equipment
- `"EquipmentInformation"` - Vista del personaje
- `"Equip_AllEquipmentFilter"` - ComboBox de filtro
- Slots de equipamiento (varios `ZItemSlotView`)

---

## Dependencias

### Clases Externas Utilizadas

- `ZGameInterface` - Orquestador principal
- `ZGameClient` - Comunicación con servidor
- `ZPost` - Funciones de envío de comandos
- `ZMyInfo` - Información del jugador
- `MMatchItemDesc` - Descripción de items
- `MGetMatchItemDescMgr()` - Manager de descripciones
- `ZIDLResource` - Recursos de UI
- `MBitmapManager` - Manager de bitmaps

---

## Patrones de Diseño Utilizados

1. **Singleton**: `ZShop::GetInstance()`
2. **Observer**: Listeners para eventos de UI
3. **Factory**: `GetItemIconBitmap()` crea bitmaps según tipo
4. **Strategy**: Filtros de items (`CheckAddType()`)
5. **MVC**: Separación entre modelo (`ZMyItemList`), vista (`ZEquipmentListBox`), controlador (`ZGameInterface`)

---

## Consideraciones de Rendimiento

1. **Serialización**: `ZShop::Serialize()` y `ZMyItemList::Serialize()` se llaman frecuentemente
2. **Filtrado**: `CheckAddType()` se ejecuta para cada item en cada serialización
3. **Bitmaps**: Los iconos se cargan desde `MBitmapManager` (cache)
4. **Vista 3D**: `ZCharacterView` renderiza en tiempo real

---

## Posibles Mejoras

1. **Cache de filtros**: Evitar re-filtrar si el filtro no cambió
2. **Lazy loading**: Cargar items por páginas
3. **Pool de bitmaps**: Reutilizar bitmaps de iconos
4. **Optimización de serialización**: Solo actualizar items visibles

---

**Fecha de análisis**: Generado automáticamente  
**Archivos analizados**: 15+  
**Clases identificadas**: 18+

