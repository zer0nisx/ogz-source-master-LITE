#include "stdafx.h"
#include "MMatchRuleWeaponDrop.h"
#include "MMatchServer.h"
#include "MMatchObject.h"
#include "MMatchStage.h"
#include "MMatchWorldItem.h"
#include "MSharedCommandTable.h"

// Tiempo de vida de los items dropeados (en milisegundos) - 5 minutos
#define WEAPON_DROP_LIFETIME (5 * 60 * 1000)

// WorldItem ID para armas dropeadas (debe estar configurado en worlditem.xml)
// Usando ID 202 para armas dropeadas (siguiendo la convención: 201=portal, 202=weapon drop)
#define WEAPON_DROP_WORLDITEM_ID 202

MMatchRuleWeaponDrop::MMatchRuleWeaponDrop(MMatchStage* pStage) : MMatchRule(pStage)
{
}

MMatchRuleWeaponDrop::~MMatchRuleWeaponDrop()
{
}

void MMatchRuleWeaponDrop::OnBegin()
{
	m_OriginalItemsMap.clear();
	m_WeaponBoxMap.clear();
}

void MMatchRuleWeaponDrop::OnEnd()
{
	// Restaurar items originales y limpiar items temporales de todos los jugadores
	MMatchStage* pStage = GetStage();
	if (pStage)
	{
		for (auto i = pStage->GetObjBegin(); i != pStage->GetObjEnd(); ++i)
		{
			MMatchObject* pObj = i->second;
			if (pObj && pObj->GetEnterBattle())
			{
				CleanupTemporaryItems(pObj);
				RestoreOriginalItems(pObj);
			}
		}
	}
	
	m_OriginalItemsMap.clear();
	m_TemporaryItemsMap.clear();
	m_WeaponBoxMap.clear();
}

void MMatchRuleWeaponDrop::OnRoundBegin()
{
	MMatchRule::OnRoundBegin();
}

void MMatchRuleWeaponDrop::OnRoundEnd()
{
	MMatchRule::OnRoundEnd();
}

void MMatchRuleWeaponDrop::OnRoundTimeOut()
{
	MMatchRule::OnRoundTimeOut();
}

bool MMatchRuleWeaponDrop::RoundCount()
{
	// Modo sin rounds, solo un round infinito
	return false;
}

bool MMatchRuleWeaponDrop::OnCheckRoundFinish()
{
	// En este modo, el round nunca termina automáticamente
	// Solo termina cuando el administrador lo decide
	return false;
}

void MMatchRuleWeaponDrop::SaveOriginalItems(MMatchObject* pObj)
{
	if (!pObj || !pObj->GetCharInfo()) return;

	MMatchCharInfo* pCharInfo = pObj->GetCharInfo();
	OriginalItems items;

	// Guardar Melee
	MMatchItem* pMelee = pCharInfo->m_EquipedItem.GetItem(MMCIP_MELEE);
	items.nMeleeID = (pMelee && pMelee->GetDesc()) ? pMelee->GetDesc()->m_nID : 0;

	// Guardar Primary
	MMatchItem* pPrimary = pCharInfo->m_EquipedItem.GetItem(MMCIP_PRIMARY);
	items.nPrimaryID = (pPrimary && pPrimary->GetDesc()) ? pPrimary->GetDesc()->m_nID : 0;

	// Guardar Secondary
	MMatchItem* pSecondary = pCharInfo->m_EquipedItem.GetItem(MMCIP_SECONDARY);
	items.nSecondaryID = (pSecondary && pSecondary->GetDesc()) ? pSecondary->GetDesc()->m_nID : 0;

	m_OriginalItemsMap[pObj->GetUID()] = items;
}

void MMatchRuleWeaponDrop::RestoreOriginalItems(MMatchObject* pObj)
{
	if (!pObj || !pObj->GetCharInfo()) return;

	auto it = m_OriginalItemsMap.find(pObj->GetUID());
	if (it == m_OriginalItemsMap.end()) return;

	MMatchCharInfo* pCharInfo = pObj->GetCharInfo();
	const OriginalItems& items = it->second;

	// Función helper para buscar item por ItemID
	auto FindItemByID = [&](u32 nItemID) -> MMatchItem* {
		if (nItemID == 0) return nullptr;
		for (auto& itemPair : pCharInfo->m_ItemList)
		{
			MMatchItem* pItem = itemPair.second;
			if (pItem && pItem->GetDesc() && pItem->GetDesc()->m_nID == nItemID)
			{
				return pItem;
			}
		}
		return nullptr;
	};

	// Restaurar Melee
	if (items.nMeleeID > 0)
	{
		MMatchItem* pItem = FindItemByID(items.nMeleeID);
		if (pItem)
		{
			pCharInfo->m_EquipedItem.SetItem(MMCIP_MELEE, pItem);
		}
	}

	// Restaurar Primary
	if (items.nPrimaryID > 0)
	{
		MMatchItem* pItem = FindItemByID(items.nPrimaryID);
		if (pItem)
		{
			pCharInfo->m_EquipedItem.SetItem(MMCIP_PRIMARY, pItem);
		}
	}

	// Restaurar Secondary
	if (items.nSecondaryID > 0)
	{
		MMatchItem* pItem = FindItemByID(items.nSecondaryID);
		if (pItem)
		{
			pCharInfo->m_EquipedItem.SetItem(MMCIP_SECONDARY, pItem);
		}
	}

	// Notificar al cliente sobre el cambio de items (equipar armas originales)
	if (items.nMeleeID > 0)
	{
		MMatchItem* pItem = FindItemByID(items.nMeleeID);
		if (pItem)
			GetStage()->NotifyEquipItem(pObj->GetUID(), pItem->GetUID(), MMCIP_MELEE);
	}
	if (items.nPrimaryID > 0)
	{
		MMatchItem* pItem = FindItemByID(items.nPrimaryID);
		if (pItem)
			GetStage()->NotifyEquipItem(pObj->GetUID(), pItem->GetUID(), MMCIP_PRIMARY);
	}
	if (items.nSecondaryID > 0)
	{
		MMatchItem* pItem = FindItemByID(items.nSecondaryID);
		if (pItem)
			GetStage()->NotifyEquipItem(pObj->GetUID(), pItem->GetUID(), MMCIP_SECONDARY);
	}
}

void MMatchRuleWeaponDrop::DropWeapons(MMatchObject* pVictim)
{
	if (!pVictim || !pVictim->GetCharInfo() || !GetStage()) return;

	MMatchCharInfo* pCharInfo = pVictim->GetCharInfo();
	v3 pos = pVictim->GetPosition();

	// Recopilar todas las armas que se van a dropear
	std::vector<DroppedWeapon> weapons;
	
	// Melee
	MMatchItem* pMelee = pCharInfo->m_EquipedItem.GetItem(MMCIP_MELEE);
	if (pMelee && pMelee->GetDesc() && pMelee->GetDesc()->m_nID > 0)
	{
		DroppedWeapon weapon;
		weapon.nItemID = pMelee->GetDesc()->m_nID;
		weapon.nSlot = MMCIP_MELEE;
		weapons.push_back(weapon);
		pCharInfo->m_EquipedItem.SetItem(MMCIP_MELEE, nullptr);
	}

	// Primary
	MMatchItem* pPrimary = pCharInfo->m_EquipedItem.GetItem(MMCIP_PRIMARY);
	if (pPrimary && pPrimary->GetDesc() && pPrimary->GetDesc()->m_nID > 0)
	{
		DroppedWeapon weapon;
		weapon.nItemID = pPrimary->GetDesc()->m_nID;
		weapon.nSlot = MMCIP_PRIMARY;
		weapons.push_back(weapon);
		pCharInfo->m_EquipedItem.SetItem(MMCIP_PRIMARY, nullptr);
	}

	// Secondary
	MMatchItem* pSecondary = pCharInfo->m_EquipedItem.GetItem(MMCIP_SECONDARY);
	if (pSecondary && pSecondary->GetDesc() && pSecondary->GetDesc()->m_nID > 0)
	{
		DroppedWeapon weapon;
		weapon.nItemID = pSecondary->GetDesc()->m_nID;
		weapon.nSlot = MMCIP_SECONDARY;
		weapons.push_back(weapon);
		pCharInfo->m_EquipedItem.SetItem(MMCIP_SECONDARY, nullptr);
	}

	// Si no hay armas para dropear, no hacer nada
	if (weapons.empty()) return;

	// Spawnear un solo WorldItem (weapon box) con todas las armas
	// Usar ExtraValues[0] = -1 para indicar que es un weapon box
	int nWorldItemExtraValues[WORLDITEM_EXTRAVALUE_NUM] = {0};
	nWorldItemExtraValues[0] = -1; // Marcador especial para weapon box
	nWorldItemExtraValues[1] = 0;  // No usado
	
	short nWorldItemUID = GetStage()->SpawnServerSideWorldItem(pVictim, WEAPON_DROP_WORLDITEM_ID, 
		pos.x, pos.y, pos.z, WEAPON_DROP_LIFETIME, nWorldItemExtraValues);
	
	// Guardar las armas asociadas con el UID del WorldItem
	if (nWorldItemUID > 0)
	{
		m_WeaponBoxMap[nWorldItemUID] = weapons;
	}

	// Notificar al cliente sobre el cambio de items (quitar armas que se dropearon)
	for (const auto& weapon : weapons)
	{
		GetStage()->NotifyTakeoffItem(pVictim->GetUID(), static_cast<MMatchCharItemParts>(weapon.nSlot));
	}
}

void MMatchRuleWeaponDrop::OnObtainWorldItem(MMatchObject* pObj, int nItemID, int* pnExtraValues, short nWorldItemUID)
{
	if (!pObj || !pObj->GetCharInfo() || !pnExtraValues) return;

	// Solo procesar si es un WorldItem de arma dropeada
	if (nItemID != WEAPON_DROP_WORLDITEM_ID) return;

	// Si ExtraValues[0] == -1, es un weapon box (múltiples armas)
	if (pnExtraValues[0] == -1)
	{
		// Es un weapon box - buscar las armas por UID
		auto it = m_WeaponBoxMap.find(nWorldItemUID);
		if (it != m_WeaponBoxMap.end() && !it->second.empty())
		{
			// Enviar comando al cliente con la lista de armas disponibles
			MCommand* pCmd = MMatchServer::GetInstance()->CreateCommand(MC_MATCH_WEAPON_DROP_SHOW_SELECTION, MUID(0,0));
			if (pCmd)
			{
				pCmd->AddParameter(new MCmdParamInt(nWorldItemUID)); // UID del weapon box
				pCmd->AddParameter(new MCmdParamInt(static_cast<int>(it->second.size()))); // Número de armas
				
				// Añadir cada arma como parámetro
				for (size_t i = 0; i < it->second.size(); ++i)
				{
					pCmd->AddParameter(new MCmdParamUInt(it->second[i].nItemID));
					pCmd->AddParameter(new MCmdParamInt(it->second[i].nSlot));
				}
				
				MMatchServer::GetInstance()->RouteToListener(pObj, pCmd);
			}
			
			return; // No equipar automáticamente, esperar selección del jugador
		}
	}
	else
	{
		// Comportamiento antiguo: arma individual (retrocompatibilidad)
		// ExtraValues[0] = ItemID del arma
		// ExtraValues[1] = Slot del arma (MMCIP_MELEE, MMCIP_PRIMARY, MMCIP_SECONDARY)
		u32 nWeaponItemID = static_cast<u32>(pnExtraValues[0]);
		int nSlot = pnExtraValues[1];

		// Validar que el slot sea válido
		if (nSlot != MMCIP_MELEE && nSlot != MMCIP_PRIMARY && nSlot != MMCIP_SECONDARY)
			return;

		MMatchCharInfo* pCharInfo = pObj->GetCharInfo();
		MMatchCharItemParts parts = static_cast<MMatchCharItemParts>(nSlot);

		// Buscar el item en el inventario del jugador
		MMatchItem* pItem = nullptr;
		for (auto& itemPair : pCharInfo->m_ItemList)
		{
			MMatchItem* pCheckItem = itemPair.second;
			if (pCheckItem && pCheckItem->GetDesc() && pCheckItem->GetDesc()->m_nID == nWeaponItemID)
			{
				pItem = pCheckItem;
				break;
			}
		}

		// Si el jugador no tiene el item, crear un item temporal (CIID = 0, no se guarda en DB)
		if (!pItem)
		{
			MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nWeaponItemID);
			if (!pItemDesc) return;

			// Crear item temporal (CIID = 0 significa que no está en la base de datos)
			MUID uidNew = MMatchItemMap::UseUID();
			if (pCharInfo->m_ItemList.CreateItem(uidNew, 0, nWeaponItemID, false, RENT_MINUTE_PERIOD_UNLIMITED))
			{
				pItem = pCharInfo->m_ItemList.GetItem(uidNew);
				
				// Rastrear el item temporal para limpiarlo después
				m_TemporaryItemsMap[pObj->GetUID()].push_back(uidNew);
			}
		}

		if (!pItem) return;

		// Equipar el arma
		pCharInfo->m_EquipedItem.SetItem(parts, pItem);

		// Notificar al cliente
		GetStage()->NotifyEquipItem(pObj->GetUID(), pItem->GetUID(), parts);
	}
}

void MMatchRuleWeaponDrop::OnRequestSelectWeapon(MMatchObject* pObj, short nWorldItemUID, int nWeaponIndex)
{
	if (!pObj || !pObj->GetCharInfo()) return;

	// Buscar el weapon box
	auto it = m_WeaponBoxMap.find(nWorldItemUID);
	if (it == m_WeaponBoxMap.end() || nWeaponIndex < 0 || nWeaponIndex >= static_cast<int>(it->second.size()))
		return;

	// Obtener el arma seleccionada
	const DroppedWeapon& weapon = it->second[nWeaponIndex];
	
	MMatchCharInfo* pCharInfo = pObj->GetCharInfo();
	MMatchCharItemParts parts = static_cast<MMatchCharItemParts>(weapon.nSlot);

	// Buscar el item en el inventario del jugador
	MMatchItem* pItem = nullptr;
	for (auto& itemPair : pCharInfo->m_ItemList)
	{
		MMatchItem* pCheckItem = itemPair.second;
		if (pCheckItem && pCheckItem->GetDesc() && pCheckItem->GetDesc()->m_nID == weapon.nItemID)
		{
			pItem = pCheckItem;
			break;
		}
	}

	// Si el jugador no tiene el item, crear un item temporal
	if (!pItem)
	{
		MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(weapon.nItemID);
		if (!pItemDesc) return;

		MUID uidNew = MMatchItemMap::UseUID();
		if (pCharInfo->m_ItemList.CreateItem(uidNew, 0, weapon.nItemID, false, RENT_MINUTE_PERIOD_UNLIMITED))
		{
			pItem = pCharInfo->m_ItemList.GetItem(uidNew);
			m_TemporaryItemsMap[pObj->GetUID()].push_back(uidNew);
		}
	}

	if (!pItem) return;

	// Equipar el arma seleccionada
	pCharInfo->m_EquipedItem.SetItem(parts, pItem);

	// Remover el weapon box del mapa (ya se usó)
	m_WeaponBoxMap.erase(it);

	// Notificar al cliente
	GetStage()->NotifyEquipItem(pObj->GetUID(), pItem->GetUID(), parts);
}

void MMatchRuleWeaponDrop::CleanupTemporaryItems(MMatchObject* pObj)
{
	if (!pObj || !pObj->GetCharInfo()) return;

	auto it = m_TemporaryItemsMap.find(pObj->GetUID());
	if (it == m_TemporaryItemsMap.end()) return;

	MMatchCharInfo* pCharInfo = pObj->GetCharInfo();
	
	// Remover todos los items temporales del inventario
	for (MUID uidItem : it->second)
	{
		// Verificar si el item está equipado y desequiparlo primero
		MMatchItem* pTempItem = pCharInfo->m_ItemList.GetItem(uidItem);
		if (pTempItem)
		{
			MMatchCharItemParts parts;
			if (pCharInfo->m_EquipedItem.IsEquipedItem(pTempItem, parts))
			{
				pCharInfo->m_EquipedItem.SetItem(parts, nullptr);
			}
		}
		
		// Remover el item del inventario (RemoveItem requiere referencia no-const)
		pCharInfo->m_ItemList.RemoveItem(uidItem);
	}

	// Limpiar la lista de items temporales
	m_TemporaryItemsMap.erase(it);
}

void MMatchRuleWeaponDrop::OnGameKill(const MUID& uidAttacker, const MUID& uidVictim)
{
	MMatchObject* pVictim = MMatchServer::GetInstance()->GetObject(uidVictim);
	if (!pVictim) return;

	// Dropear las armas del jugador muerto
	DropWeapons(pVictim);
}

void MMatchRuleWeaponDrop::OnEnterBattle(MUID& uidChar)
{
	MMatchObject* pObj = MMatchServer::GetInstance()->GetObject(uidChar);
	if (!pObj) return;

	// Guardar items originales cuando entra a la batalla
	SaveOriginalItems(pObj);
}

void MMatchRuleWeaponDrop::OnLeaveBattle(MUID& uidChar)
{
	MMatchObject* pObj = MMatchServer::GetInstance()->GetObject(uidChar);
	if (!pObj) return;

	// Limpiar items temporales primero
	CleanupTemporaryItems(pObj);
	
	// Restaurar items originales cuando sale de la batalla
	RestoreOriginalItems(pObj);
	
	// Remover del mapa de items originales
	m_OriginalItemsMap.erase(uidChar);
}

void MMatchRuleWeaponDrop::OnCommand(MCommand* pCommand)
{
	if (!pCommand) return;

	switch (pCommand->GetID())
	{
	case MC_MATCH_REQUEST_SELECT_WEAPON:
		{
			MUID uidPlayer = pCommand->GetSenderUID();
			short nWorldItemUID = 0;
			int nWeaponIndex = 0;

			pCommand->GetParameter(&nWorldItemUID, 1, MPT_INT);
			pCommand->GetParameter(&nWeaponIndex, 2, MPT_INT);

			MMatchObject* pObj = MMatchServer::GetInstance()->GetObject(uidPlayer);
			if (pObj)
			{
				OnRequestSelectWeapon(pObj, nWorldItemUID, nWeaponIndex);
			}
		}
		break;
	}
}
