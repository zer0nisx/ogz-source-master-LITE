#include "stdafx.h"

#include "ZShop.h"
#include "ZPost.h"
#include "ZGameClient.h"
#include "MUID.h"
#include "ZApplication.h"
#include "ZGameInterface.h"
#include "MListBox.h"
#include "MLabel.h"
#include "ZEquipmentListBox.h"
#include "ZNetRepository.h"
#include "ZMyInfo.h"

#include <algorithm>
#include <cctype>

// Corrección #5: Valor de error específico para GetItemID()
static const u32 INVALID_ITEM_ID = 0xFFFFFFFF;

ZShop::ZShop() : m_nPage(0), m_bCreated(false), m_SortType(SHOP_SORT_NONE),
m_bFilterAffordable(false), m_bFilterCacheValid(false)
{
	m_ListFilter = zshop_item_filter_all;
}

ZShop::~ZShop()
{
}
bool ZShop::Create(bool bForceRefresh)
{
	// Corrección #4: Permitir recrear con bForceRefresh
	if (m_bCreated && !bForceRefresh) return false;

	// Error #7: Optimizar - guardar ZGetGameClient() en variable local
	ZGameClient* pGameClient = ZGetGameClient();
	ZPostRequestShopItemList(pGameClient->GetPlayerUID(), 0, 0);
	ZPostRequestCharacterItemList(pGameClient->GetPlayerUID());

	m_bCreated = true;
	return true;
}

void ZShop::Destroy()
{
	if (!m_bCreated) return;

	m_bCreated = false;
}

ZShop* ZShop::GetInstance()
{
	static ZShop m_stShop;
	return &m_stShop;
}

// Mejora #1: Optimización de CheckAddType() - convertir a switch
bool ZShop::CheckAddType(int type)
{
	if (m_ListFilter == zshop_item_filter_all) return true;

	switch (m_ListFilter) {
	case zshop_item_filter_head:  return type == MMIST_HEAD;
	case zshop_item_filter_chest: return type == MMIST_CHEST;
	case zshop_item_filter_hands: return type == MMIST_HANDS;
	case zshop_item_filter_legs:  return type == MMIST_LEGS;
	case zshop_item_filter_feet:  return type == MMIST_FEET;
	case zshop_item_filter_melee: return type == MMIST_MELEE;
	case zshop_item_filter_range: return type == MMIST_RANGE;
	case zshop_item_filter_custom: return type == MMIST_CUSTOM;
	case zshop_item_filter_extra: return (type == MMIST_EXTRA) || (type == MMIST_FINGER);
	case zshop_item_filter_quest: return true; // Se maneja en Serialize()
	default: return false;
	}
}

void ZShop::Serialize()
{
	// Mejora #5: Actualizar cache de items filtrados
	UpdateFilterCache();

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MListBox* pListBox = (MListBox*)pResource->FindWidget("AllEquipmentList");

	if (pListBox) {
		pListBox->RemoveAll();

		// Error #6: Optimizar - guardar GetSex() fuera del loop
		MMatchSex nPlayerSex = ZGetMyInfo()->GetSex();

		// Usar m_FilteredItemVector en lugar de m_ItemVector
		for (size_t i = 0; i < m_FilteredItemVector.size(); i++) {
			u32 nItemID = m_FilteredItemVector[i];
			MMatchItemDesc* pItemDesc = NULL;
			pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);

			if (pItemDesc != NULL) {
				if (pItemDesc->m_nResSex != -1) {
					if (pItemDesc->m_nResSex != int(nPlayerSex)) continue;
				}

				MUID uidItem = MUID(0, (int)i + 1);

				// Aplicar filtro de tipo
				if (CheckAddType(pItemDesc->m_nSlot))
				{
					((ZEquipmentListBox*)(pListBox))->Add(uidItem, pItemDesc->m_nID,
						GetItemIconBitmap(pItemDesc, true),
						pItemDesc->m_szName,
						pItemDesc->m_nResLevel,
						pItemDesc->m_nBountyPrice);
				}
			}

#ifdef _QUEST_ITEM
			else
			{
				MQuestItemDesc* pQuestItemDesc = GetQuestItemDescMgr().FindQItemDesc(nItemID);
				if (0 == pQuestItemDesc)
					continue;

				MUID uidItem = MUID(0, (int)i + 1);
				char szPrice[128];
				sprintf_safe(szPrice, "%d", pQuestItemDesc->m_nPrice);

				if ((m_ListFilter == zshop_item_filter_all) || (m_ListFilter == zshop_item_filter_quest))
				{
					((ZEquipmentListBox*)(pListBox))->Add(uidItem,
						pQuestItemDesc->m_nItemID,
						ZApplication::GetGameInterface()->GetQuestItemIcon(pQuestItemDesc->m_nItemID, true),
						pQuestItemDesc->m_szQuestItemName,
						"-",
						szPrice);
				}
			}
#endif
		}
	}

#ifdef _DEBUG
	pListBox = (MListBox*)pResource->FindWidget("CashEquipmentList");
	if (pListBox)
	{
		pListBox->RemoveAll();

		MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(506001);
		if (pItemDesc)
		{
			MUID uidItem = MUID(0, 200);

			// Usar la variable local nPlayerSex si está disponible, sino obtenerla
			MMatchSex nPlayerSex = ZGetMyInfo()->GetSex();
			if ((pItemDesc->m_nResSex == -1) || (pItemDesc->m_nResSex == int(nPlayerSex)))
				((ZEquipmentListBox*)(pListBox))->Add(uidItem,
					pItemDesc->m_nID,
					GetItemIconBitmap(pItemDesc, true),
					pItemDesc->m_szName,
					pItemDesc->m_nResLevel,
					pItemDesc->m_nBountyPrice);
		}
	}
#endif
}

u32 ZShop::GetItemID(int nIndex)
{
	// Mejora #5: Si hay cache válido (filtros activos), usar m_FilteredItemVector
	// Si no hay filtros, usar m_ItemVector original
	if (m_bFilterCacheValid && !m_FilteredItemVector.empty())
	{
		// Usar el vector filtrado cuando hay filtros activos
		if ((nIndex < 0) || (nIndex >= (int)m_FilteredItemVector.size()))
			return INVALID_ITEM_ID;
		return m_FilteredItemVector[nIndex];
	}
	else
	{
		// Usar el vector original cuando no hay filtros
		if ((nIndex < 0) || (nIndex >= (int)m_ItemVector.size()))
			return INVALID_ITEM_ID;
		return m_ItemVector[nIndex];
	}
}

void ZShop::SetItemsAll(u32* nItemList, int nItemCount)
{
	Clear();
	for (int i = 0; i < nItemCount; i++)
	{
		m_ItemVector.push_back(nItemList[i]);
	}
	InvalidateFilterCache(); // Invalidar cache cuando se reciben nuevos items
}

void ZShop::Clear()
{
	m_ItemVector.clear();
	InvalidateFilterCache();
}

// Mejora #3: Búsqueda de items
void ZShop::SetSearchText(const char* szText)
{
	if (szText)
		m_SearchText = szText;
	else
		m_SearchText.clear();

	// Convertir a minúsculas para búsqueda case-insensitive
	std::transform(m_SearchText.begin(), m_SearchText.end(),
		m_SearchText.begin(), ::tolower);

	InvalidateFilterCache();
}

void ZShop::ClearSearch()
{
	m_SearchText.clear();
	InvalidateFilterCache();
}

bool ZShop::MatchesSearch(u32 nItemID) const
{
	if (m_SearchText.empty())
		return true;

	MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
	if (!pItemDesc)
	{
#ifdef _QUEST_ITEM
		MQuestItemDesc* pQuestItemDesc = GetQuestItemDescMgr().FindQItemDesc(nItemID);
		if (!pQuestItemDesc)
			return false;

		// Búsqueda case-insensitive en nombre de quest item
		std::string itemName = pQuestItemDesc->m_szQuestItemName;
		std::transform(itemName.begin(), itemName.end(), itemName.begin(), ::tolower);
		return itemName.find(m_SearchText) != std::string::npos;
#else
		return false;
#endif
	}

	// Búsqueda case-insensitive en nombre de item
	std::string itemName = pItemDesc->m_szName;
	std::transform(itemName.begin(), itemName.end(), itemName.begin(), ::tolower);
	return itemName.find(m_SearchText) != std::string::npos;
}

// Mejora #4: Ordenamiento de items
void ZShop::SetSortType(int nSortType)
{
	m_SortType = nSortType;
	InvalidateFilterCache();
}

void ZShop::SortItems()
{
	if (m_SortType == SHOP_SORT_NONE)
		return;

	// Estructura para ordenar items con metadata
	struct SortableItem {
		u32 nItemID;
		MMatchItemDesc* pItemDesc;
#ifdef _QUEST_ITEM
		MQuestItemDesc* pQuestItemDesc;
#endif
		std::string szName;
		int nPrice;
		int nLevel;

		SortableItem(u32 id) : nItemID(id), pItemDesc(NULL), nPrice(0), nLevel(0)
#ifdef _QUEST_ITEM
			, pQuestItemDesc(NULL)
#endif
		{
			pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(id);
			if (pItemDesc)
			{
				szName = pItemDesc->m_szName;
				nPrice = pItemDesc->m_nBountyPrice;
				nLevel = pItemDesc->m_nResLevel;
			}
#ifdef _QUEST_ITEM
			else
			{
				pQuestItemDesc = GetQuestItemDescMgr().FindQItemDesc(id);
				if (pQuestItemDesc)
				{
					szName = pQuestItemDesc->m_szQuestItemName;
					nPrice = pQuestItemDesc->m_nPrice;
					nLevel = 0; // Quest items no tienen nivel
				}
			}
#endif
		}
	};

	// Crear vector de items ordenables desde m_FilteredItemVector (ya filtrado)
	std::vector<SortableItem> sortableItems;
	for (size_t i = 0; i < m_FilteredItemVector.size(); i++)
	{
		sortableItems.push_back(SortableItem(m_FilteredItemVector[i]));
	}

	// Ordenar según tipo
	switch (m_SortType)
	{
	case SHOP_SORT_NAME_ASC:
		std::sort(sortableItems.begin(), sortableItems.end(),
			[](const SortableItem& a, const SortableItem& b) {
				return a.szName < b.szName;
			});
		break;

	case SHOP_SORT_NAME_DESC:
		std::sort(sortableItems.begin(), sortableItems.end(),
			[](const SortableItem& a, const SortableItem& b) {
				return a.szName > b.szName;
			});
		break;

	case SHOP_SORT_PRICE_ASC:
		std::sort(sortableItems.begin(), sortableItems.end(),
			[](const SortableItem& a, const SortableItem& b) {
				if (a.nPrice != b.nPrice) return a.nPrice < b.nPrice;
				return a.szName < b.szName; // Desempate por nombre
			});
		break;

	case SHOP_SORT_PRICE_DESC:
		std::sort(sortableItems.begin(), sortableItems.end(),
			[](const SortableItem& a, const SortableItem& b) {
				if (a.nPrice != b.nPrice) return a.nPrice > b.nPrice;
				return a.szName < b.szName; // Desempate por nombre
			});
		break;

	case SHOP_SORT_LEVEL_ASC:
		std::sort(sortableItems.begin(), sortableItems.end(),
			[](const SortableItem& a, const SortableItem& b) {
				if (a.nLevel != b.nLevel) return a.nLevel < b.nLevel;
				return a.szName < b.szName; // Desempate por nombre
			});
		break;

	case SHOP_SORT_LEVEL_DESC:
		std::sort(sortableItems.begin(), sortableItems.end(),
			[](const SortableItem& a, const SortableItem& b) {
				if (a.nLevel != b.nLevel) return a.nLevel > b.nLevel;
				return a.szName < b.szName; // Desempate por nombre
			});
		break;
	}

	// Actualizar m_FilteredItemVector con orden ordenado
	m_FilteredItemVector.clear();
	for (size_t i = 0; i < sortableItems.size(); i++)
	{
		m_FilteredItemVector.push_back(sortableItems[i].nItemID);
	}
}

// Mejora #2: Filtro "Puedo Comprar"
void ZShop::SetFilterAffordable(bool bEnable)
{
	m_bFilterAffordable = bEnable;
	InvalidateFilterCache();
}

bool ZShop::CanAffordItem(u32 nItemID) const
{
	MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
	if (pItemDesc)
	{
		int nBP = ZGetMyInfo()->GetBP();
		return nBP >= pItemDesc->m_nBountyPrice;
	}

#ifdef _QUEST_ITEM
	MQuestItemDesc* pQuestItemDesc = GetQuestItemDescMgr().FindQItemDesc(nItemID);
	if (pQuestItemDesc)
	{
		int nBP = ZGetMyInfo()->GetBP();
		return nBP >= pQuestItemDesc->m_nPrice;
	}
#endif

	return false;
}

// Mejora #5: Cache de items filtrados
void ZShop::InvalidateFilterCache()
{
	m_bFilterCacheValid = false;
	m_FilteredItemVector.clear();
}

void ZShop::UpdateFilterCache()
{
	if (m_bFilterCacheValid)
		return;

	m_FilteredItemVector.clear();

	// Aplicar todos los filtros
	for (size_t i = 0; i < m_ItemVector.size(); i++)
	{
		u32 nItemID = m_ItemVector[i];

		// Filtro de búsqueda
		if (!MatchesSearch(nItemID))
			continue;

		// Filtro "Puedo Comprar"
		if (m_bFilterAffordable && !CanAffordItem(nItemID))
			continue;

		// Filtro de tipo (se aplica en Serialize() para items normales y quest)
		m_FilteredItemVector.push_back(nItemID);
	}

	// Ordenar items filtrados si es necesario
	if (m_SortType != SHOP_SORT_NONE)
	{
		SortItems();
	}

	m_bFilterCacheValid = true;
}