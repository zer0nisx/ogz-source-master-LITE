#pragma once

#include "GlobalTypes.h"
#include "ZPrerequisites.h"
#include <vector>
#include <string>

enum {
	zshop_item_filter_all = 0,
	zshop_item_filter_head,
	zshop_item_filter_chest,
	zshop_item_filter_hands,
	zshop_item_filter_legs,
	zshop_item_filter_feet,
	zshop_item_filter_melee,
	zshop_item_filter_range,
	zshop_item_filter_custom,
	zshop_item_filter_extra,
	zshop_item_filter_quest,
};

// Tipos de ordenamiento
enum ShopSortType {
	SHOP_SORT_NONE = 0,
	SHOP_SORT_NAME_ASC,
	SHOP_SORT_NAME_DESC,
	SHOP_SORT_PRICE_ASC,
	SHOP_SORT_PRICE_DESC,
	SHOP_SORT_LEVEL_ASC,
	SHOP_SORT_LEVEL_DESC,
};

class ZShop
{
protected:
	int m_nPage;
	bool m_bCreated;
	std::vector<u32> m_ItemVector;
	
	// Mejora #3: Búsqueda de items
	std::string m_SearchText;
	
	// Mejora #4: Ordenamiento
	int m_SortType;
	
	// Mejora #2: Filtro "Puedo Comprar"
	bool m_bFilterAffordable;
	
	// Mejora #5: Cache de items filtrados
	std::vector<u32> m_FilteredItemVector;
	bool m_bFilterCacheValid;

public:
	int m_ListFilter;

public:
	ZShop();
	virtual ~ZShop();
	bool Create(bool bForceRefresh = false);
	void Destroy();
	void Clear();
	void Serialize();

	bool CheckAddType(int type);
	
	// Mejora #3: Búsqueda
	void SetSearchText(const char* szText);
	void ClearSearch();
	bool MatchesSearch(u32 nItemID) const;
	
	// Mejora #4: Ordenamiento
	void SetSortType(int nSortType);
	void SortItems();
	
	// Mejora #2: Filtro "Puedo Comprar"
	void SetFilterAffordable(bool bEnable);
	bool CanAffordItem(u32 nItemID) const;
	
	// Mejora #5: Cache
	void InvalidateFilterCache();
	void UpdateFilterCache();

	int GetItemCount() { return (int)m_ItemVector.size(); }
	void SetItemsAll(u32* nItemList, int nItemCount);
	int GetPage() { return m_nPage; }
	u32 GetItemID(int nIndex);
	static ZShop* GetInstance();
};

inline ZShop* ZGetShop() { return ZShop::GetInstance(); }