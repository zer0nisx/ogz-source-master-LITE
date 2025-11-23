#include "stdafx.h"
#include "MMatchServer.h"
#include "MSharedCommandTable.h"
#include "MErrorTable.h"
#include "MBlobArray.h"
#include "MObject.h"
#include "MMatchObject.h"
#include "MMatchItem.h"
#include "MAgentObject.h"
#include "MMatchNotify.h"
#include "MMatchObjCache.h"
#include "MMatchStage.h"
#include "MMatchTransDataType.h"
#include "MMatchFormula.h"
#include "MMatchConfig.h"
#include "MCommandCommunicator.h"
#include "MMatchShop.h"
#include "MMatchTransDataType.h"
#include "MDebug.h"	
#include "MMatchAuth.h"
#include "MMatchStatus.h"
#include "MAsyncDBJob.h"
#include "MAsyncDBJob_BringAccountItem.h"
#include "MMatchUtil.h"

bool MMatchServer::InsertCharItem(const MUID& uidPlayer, const u32 nItemID, bool bRentItem, int nRentPeriodHour)
{
	MMatchObject* pObject = GetObject(uidPlayer);
	if (!IsEnabledObject(pObject)) return false;

	MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
	if (pItemDesc == NULL) return false;


	// ��� ������ �߰�
	u32 nNewCIID = 0;
	if (!GetDBMgr()->InsertCharItem(pObject->GetCharInfo()->m_nCID, nItemID, bRentItem, nRentPeriodHour, &nNewCIID))
	{
		return false;
	}

	// ������Ʈ�� ������ �߰�
	int nRentMinutePeriodRemainder = nRentPeriodHour * 60;
	MUID uidNew = MMatchItemMap::UseUID();
	pObject->GetCharInfo()->m_ItemList.CreateItem(uidNew, nNewCIID, nItemID, bRentItem, nRentMinutePeriodRemainder);

	return true;
}

bool MMatchServer::BuyItem(MMatchObject* pObject, unsigned int nItemID, bool bRentItem, int nRentPeriodHour)
{
	MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
	if (pItemDesc == NULL) return false;


	int nPrice = pItemDesc->m_nBountyPrice;

	if (pObject->GetCharInfo() == NULL) return false;

	// ���� �ִ� ������ ������ �ѵ��� �Ѿ����� ����
	if (pObject->GetCharInfo()->m_ItemList.GetCount() >= MAX_ITEM_COUNT)
	{
		MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_BUY_ITEM, MUID(0,0));
		pNew->AddParameter(new MCmdParamInt(MERR_TOO_MANY_ITEM));
		RouteToListener(pObject, pNew);

		return false;
	}


	// �������� �� �� �ִ� �ڰ��� �Ǵ��� ����
	if (pObject->GetCharInfo()->m_nBP < nPrice)
	{
		MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_BUY_ITEM, MUID(0,0));
		pNew->AddParameter(new MCmdParamInt(MERR_TOO_EXPENSIVE_BOUNTY));
		RouteToListener(pObject, pNew);

		return false;
	}

	// ĳ���� ���� ĳ�� ������Ʈ�� ���� ���ش�.
	UpdateCharDBCachingData(pObject);	


	u32 nNewCIID = 0;
	if (!GetDBMgr()->BuyBountyItem(pObject->GetCharInfo()->m_nCID, pItemDesc->m_nID, nPrice, &nNewCIID))
	{
		MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_BUY_ITEM, MUID(0,0));
		pNew->AddParameter(new MCmdParamInt(MERR_CANNOT_BUY_ITEM));
		RouteToListener(pObject, pNew);

		return false;
	}


	// ������Ʈ�� �ٿ�Ƽ ��´�.
	pObject->GetCharInfo()->m_nBP -= nPrice;

	// ������Ʈ�� ������ �߰�
	MUID uidNew = MMatchItemMap::UseUID();
	pObject->GetCharInfo()->m_ItemList.CreateItem(uidNew, nNewCIID, pItemDesc->m_nID);


	MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_BUY_ITEM, MUID(0,0));
	pNew->AddParameter(new MCmdParamInt(MOK));
	RouteToListener(pObject, pNew);

	return true;
}


void MMatchServer::OnRequestBuyItem(const MUID& uidPlayer, const u32 nItemID)
{
	ResponseBuyItem(uidPlayer, nItemID);
}
bool MMatchServer::ResponseBuyItem(const MUID& uidPlayer, const u32 nItemID)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return false;

	// ������ �Ȱ� �ִ� ��ǰ���� Ȯ���Ѵ�.
	if (MGetMatchShop()->IsSellItem(nItemID) == false) return false;

	BuyItem(pObj, nItemID);

	return true;
}

void MMatchServer::OnRequestSellItem(const MUID& uidPlayer, const MUID& uidItem)
{
	ResponseSellItem(uidPlayer, uidItem);
}
bool MMatchServer::ResponseSellItem(const MUID& uidPlayer, const MUID& uidItem)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return false;
	if (pObj->GetCharInfo() == NULL) return false;

	int nPrice = 0;

	MUID uidCharItem = uidItem;
	MMatchItem* pItem = pObj->GetCharInfo()->m_ItemList.GetItem(uidCharItem);
	if ((pItem == NULL) || (pItem->GetDesc() == NULL))
	{
		// �������� �������� ������
		MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_SELL_ITEM, MUID(0,0));
		pNew->AddParameter(new MCmdParamInt(MERR_CANNOT_SELL_NONE_ITEM));
		RouteToListener(pObj, pNew);

		return false;
	}

	// ����ϰ� ������ �� �� ����.
	if (pItem->IsEquiped())
	{
		MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_SELL_ITEM, MUID(0,0));
		pNew->AddParameter(new MCmdParamInt(MERR_CANNOT_SELL_EQUIPED_ITEM));
		RouteToListener(pObj, pNew);

		return false;
	}

	// ĳ���������̸� �� �� ����.
	if(pItem->GetDesc()->IsCashItem())
	{
		MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_SELL_ITEM, MUID(0,0));
		pNew->AddParameter(new MCmdParamInt(MERR_CANNOT_SELL_CASHITEM));
		RouteToListener(pObj, pNew);

		return false;
	}


	// ĳ���� ���� ĳ�� ������Ʈ�� ���� ���ش�.
	UpdateCharDBCachingData(pObj);	
	
	nPrice = pItem->GetDesc()->GetBountyValue();	
	unsigned int nCID = pObj->GetCharInfo()->m_nCID;
	unsigned int nSelItemID = pItem->GetDesc()->m_nID;
	unsigned int nCIID = pItem->GetCIID();
	int nCharBP = pObj->GetCharInfo()->m_nBP + nPrice;


	if (!GetDBMgr()->SellBountyItem(nCID, nSelItemID, nCIID, nPrice, nCharBP))
	{
		MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_SELL_ITEM, MUID(0,0));
		pNew->AddParameter(new MCmdParamInt(MERR_CANNOT_SELL_ITEM));
		RouteToListener(pObj, pNew);

		return false;
	}

	// ������Ʈ�� �ٿ�Ƽ �����ش�.
	pObj->GetCharInfo()->m_nBP += nPrice;

	// ������Ʈ���� ������ ����
	pObj->GetCharInfo()->m_ItemList.RemoveItem(uidCharItem);


/*
	// ��� �ٿ�Ƽ �����ش�
	if (!GetDBMgr()->UpdateCharBP(pObj->GetCharInfo()->m_nCID, nPrice))
	{
		MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_SELL_ITEM, MUID(0,0));
		pNew->AddParameter(new MCmdParamInt(MERR_CANNOT_SELL_ITEM));
		RouteToListener(pObj, pNew);

		return false;
	}
	// ������Ʈ�� �ٿ�Ƽ �����ش�.
	pObj->GetCharInfo()->m_nBP += nPrice;

	u32 nSelItemID = pItem->GetDesc()->m_nID;
	if (RemoveCharItem(pObj, uidCharItem) == true)
	{
		// RemoveCharItem �Լ� ���Ŀ� pItem�� ����ϸ� �ȵȴ�.
		pItem = NULL;
		GetDBMgr()->InsertItemPurchaseLogByBounty(nSelItemID, pObj->GetCharInfo()->m_nCID,
			nPrice, pObj->GetCharInfo()->m_nBP, MMatchDBMgr::IPT_SELL);
	}
	else
	{
		MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_SELL_ITEM, MUID(0,0));
		pNew->AddParameter(new MCmdParamInt(MERR_CANNOT_SELL_ITEM));
		RouteToListener(pObj, pNew);

		return false;
	}
*/

	MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_SELL_ITEM, MUID(0,0));
	pNew->AddParameter(new MCmdParamInt(MOK));
	RouteToListener(pObj, pNew);


	ResponseCharacterItemList(uidPlayer);	// ���� �ٲ� ������ ����Ʈ�� �ٽ� �ѷ��ش�.

	return true;
}

// ���� ���� ������Ʈ���� �������� ����
bool MMatchServer::RemoveCharItem(MMatchObject* pObject, MUID& uidItem)
{
	MMatchItem* pItem = pObject->GetCharInfo()->m_ItemList.GetItem(uidItem);
	if (!pItem) return false;

	// ��񿡼� ������ ����
	if (!GetDBMgr()->DeleteCharItem(pObject->GetCharInfo()->m_nCID, pItem->GetCIID()))
	{
		return false;
	}

	// ���� ������̸� ��ü
	MMatchCharItemParts nCheckParts = MMCIP_END;
	if (pObject->GetCharInfo()->m_EquipedItem.IsEquipedItem(pItem, nCheckParts))
	{
		pObject->GetCharInfo()->m_EquipedItem.Remove(nCheckParts);
	}

	// ������Ʈ���� ������ ����
	pObject->GetCharInfo()->m_ItemList.RemoveItem(uidItem);

	return true;
}

void MMatchServer::OnRequestShopItemList(const MUID& uidPlayer, const int nFirstItemIndex, const int nItemCount)
{
	ResponseShopItemList(uidPlayer, nFirstItemIndex, nItemCount);
}

void MMatchServer::ResponseShopItemList(const MUID& uidPlayer, const int nFirstItemIndex, const int nItemCount)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;

	int nRealItemCount = 0;
	if ((nItemCount <= 0) || (nItemCount > MGetMatchShop()->GetCount())) nRealItemCount = MGetMatchShop()->GetCount();
	else nRealItemCount = nItemCount;


	MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_SHOP_ITEMLIST, MUID(0,0));
	void* pItemArray = MMakeBlobArray(sizeof(u32), nRealItemCount);
	int nIndex=0;

	for (int i = nFirstItemIndex; i < nFirstItemIndex+nRealItemCount; i++)
	{
		u32* pnItemID = (u32*)MGetBlobArrayElement(pItemArray, nIndex++);
		ShopItemNode* pSINode = MGetMatchShop()->GetSellItem(i);

		if (pSINode != NULL)
		{
			*pnItemID = pSINode->nItemID;
		}
		else
		{
			*pnItemID = 0;
		}

	}

	pNew->AddParameter(new MCommandParameterBlob(pItemArray, MGetBlobArraySize(pItemArray)));
	MEraseBlobArray(pItemArray);

	RouteToListener(pObj, pNew);	

}

void MMatchServer::OnRequestCharacterItemList(const MUID& uidPlayer)
{
	ResponseCharacterItemList(uidPlayer);
}

void MMatchServer::ResponseCharacterItemList(const MUID& uidPlayer)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if ((pObj == NULL) || (pObj->GetCharInfo() == NULL)) 
	{
		mlog("ResponseCharacterItemList > pObj or pObj->GetCharInfo() IS NULL\n");
		return;
	}

	// ������ ��� �＼���� ���߾����� ��񿡼� ������ ������ �����´�
	if (!pObj->GetCharInfo()->m_ItemList.IsDoneDbAccess())
	{
		if (!GetDBMgr()->GetCharItemInfo(*pObj->GetCharInfo()))
		{
			mlog("DB Query(ResponseCharacterItemList > GetCharItemInfo) Failed\n");
			return;
		}
	}

	if( MSM_TEST == MGetServerConfig()->GetServerMode() ) 
	{
		if( !pObj->GetCharInfo()->m_QuestItemList.IsDoneDbAccess() )
		{
			if( !GetDBMgr()->GetCharQuestItemInfo(pObj->GetCharInfo()) )
			{
				mlog( "MMatchServer::ResponseCharacterItemList - ��� ���� ������ ������ ����. ���� ���� �Ұ�.\n" );
				return;
			}
		}
	}

	MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_CHARACTER_ITEMLIST, MUID(0,0));

	// �ٿ�Ƽ ����
	pNew->AddParameter(new MCommandParameterInt(pObj->GetCharInfo()->m_nBP));

	// ����� ������ ����
	int nRealEquipedItemCount = 0;
	int nIndex = 0;
	void* pEquipItemArray = MMakeBlobArray(sizeof(MUID), MMCIP_END);
	for (int i = 0; i < MMCIP_END; i++)
	{
		MUID* pUID = (MUID*)MGetBlobArrayElement(pEquipItemArray, nIndex++);

		if (!pObj->GetCharInfo()->m_EquipedItem.IsEmpty(MMatchCharItemParts(i)))
		{
			*pUID = pObj->GetCharInfo()->m_EquipedItem.GetItem(MMatchCharItemParts(i))->GetUID();
			nRealEquipedItemCount++;
		}
		else
		{
			*pUID = MUID(0,0);
		}
	}

	pNew->AddParameter(new MCommandParameterBlob(pEquipItemArray, MGetBlobArraySize(pEquipItemArray)));
	MEraseBlobArray(pEquipItemArray);


	// ���� �ִ� ������ ����Ʈ ����
	int nItemCount = pObj->GetCharInfo()->m_ItemList.GetCount();

	void* pItemArray = MMakeBlobArray(sizeof(MTD_ItemNode), nItemCount);
	MMatchItemMap* pItemList = &pObj->GetCharInfo()->m_ItemList;

	nIndex = 0;
	for (MMatchItemMap::iterator itor = pItemList->begin(); itor != pItemList->end(); ++itor)
	{
		MMatchItem* pItem = (*itor).second;

		MTD_ItemNode* pItemNode = (MTD_ItemNode*)MGetBlobArrayElement(pItemArray, nIndex++);

		auto nPassTime = MGetTimeDistance(static_cast<unsigned long int>(pItem->GetRentItemRegTime()), static_cast<unsigned long int>(GetTickTime()));
		int nPassMinuteTime = static_cast<int>(nPassTime / (1000 * 60));

		int nRentMinutePeriodRemainder = RENT_MINUTE_PERIOD_UNLIMITED;
		if (pItem->IsRentItem())
		{
			nRentMinutePeriodRemainder = pItem->GetRentMinutePeriodRemainder() - nPassMinuteTime;
		}

		Make_MTDItemNode(pItemNode, pItem->GetUID(), pItem->GetDescID(), nRentMinutePeriodRemainder);
	}


	pNew->AddParameter(new MCommandParameterBlob(pItemArray, MGetBlobArraySize(pItemArray)));
	MEraseBlobArray(pItemArray);

	RouteToListener(pObj, pNew);	
}

void MMatchServer::OnRequestAccountItemList(const MUID& uidPlayer)
{
	ResponseAccountItemList(uidPlayer);
}
void MMatchServer::ResponseAccountItemList(const MUID& uidPlayer)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if ((pObj == NULL) || (pObj->GetCharInfo() == NULL)) 
	{
		mlog("ResponseAccountItemList > pObj or pObj->GetCharInfo() IS NULL\n");
		return;
	}

#define MAX_ACCOUNT_ITEM		1000		// �ְ� 1000���� �����Ѵ�.
	MAccountItemNode accountItems[MAX_ACCOUNT_ITEM];
	int nItemCount = 0;

#define MAX_EXPIRED_ACCOUNT_ITEM	100
	MAccountItemNode ExpiredItemList[100];
	int nExpiredItemCount = 0;

	// ��񿡼� AccountItem�� �����´�
	if (!GetDBMgr()->GetAccountItemInfo(pObj->GetAccountInfo()->m_nAID, accountItems, &nItemCount, MAX_ACCOUNT_ITEM,
										 ExpiredItemList, &nExpiredItemCount, MAX_EXPIRED_ACCOUNT_ITEM))
	{
		mlog("DB Query(ResponseAccountItemList > GetAccountItemInfo) Failed\n");
		return;
	}

	// ���⼭ �߾������� �Ⱓ���� �������� �ִ��� üũ�Ѵ�.
	if (nExpiredItemCount > 0)
	{
		vector<u32> vecExpiredItemIDList;

		for (int i = 0; i < nExpiredItemCount; i++)
		{
			// ��񿡼� �Ⱓ����� AccountItem�� �����.
			if (GetDBMgr()->DeleteExpiredAccountItem(ExpiredItemList[i].nAIID))
			{
				vecExpiredItemIDList.push_back(ExpiredItemList[i].nItemID);
			}
			else
			{
				mlog("DB Query(ResponseAccountItemList > DeleteExpiredAccountItem) Failed\n");
			}
		}
		
		if (!vecExpiredItemIDList.empty())
		{
			ResponseExpiredItemIDList(pObj, vecExpiredItemIDList);
		}
	}



	if (nItemCount > 0)
	{
		MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_ACCOUNT_ITEMLIST, MUID(0,0));

		// ���� �ִ� ������ ����Ʈ ����
		void* pItemArray = MMakeBlobArray(sizeof(MTD_AccountItemNode), nItemCount);
		

		for (int i = 0; i < nItemCount; i++)
		{
			MTD_AccountItemNode* pItemNode = (MTD_AccountItemNode*)MGetBlobArrayElement(pItemArray, i);

			Make_MTDAccountItemNode(pItemNode, 
									accountItems[i].nAIID, 
									accountItems[i].nItemID, 
									accountItems[i].nRentMinutePeriodRemainder);
		}

		pNew->AddParameter(new MCommandParameterBlob(pItemArray, MGetBlobArraySize(pItemArray)));
		MEraseBlobArray(pItemArray);

		RouteToListener(pObj, pNew);	
	}
}

void MMatchServer::OnRequestEquipItem(const MUID& uidPlayer, const MUID& uidItem, const i32 nEquipmentSlot)
{
	if (nEquipmentSlot >= MMCIP_END) return;
	MMatchCharItemParts parts = MMatchCharItemParts(nEquipmentSlot);

	ResponseEquipItem(uidPlayer, uidItem, parts);
}

void MMatchServer::ResponseEquipItem(const MUID& uidPlayer, const MUID& uidItem, const MMatchCharItemParts parts)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;

	MMatchCharInfo* pCharInfo = pObj->GetCharInfo();
	if (pCharInfo == NULL) return;

	MUID uidRealItem = uidItem;
	MMatchItem* pItem = pCharInfo->m_ItemList.GetItem(uidRealItem);

	auto Respond = [&](int nResult) {
		MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_EQUIP_ITEM, MUID(0, 0));
		pNew->AddParameter(new MCommandParameterInt(nResult));
		RouteToListener(pObj, pNew);
	};

	if ((pItem == NULL) || (!IsSuitableItemSlot(pItem->GetDesc()->m_nSlot, parts)))
	{
		Respond(MERR_CANNOT_EQUIP_ITEM);
		return;
	}

	{
		auto nResult = ValidateEquipItem(pObj, pItem, parts);
		if (nResult != MOK)
		{
			Respond(nResult);
			return;
		}
	}

	u32 nItemCIID = 0;
	u32 nItemID = 0;

	nItemCIID = pItem->GetCIID();
	nItemID = pItem->GetDesc()->m_nID;

	if (GetDBMgr()->UpdateEquipedItem(pCharInfo->m_nCID, parts, nItemCIID, nItemID))
	{
		pCharInfo->m_EquipedItem.SetItem(parts, pItem);
	}
	else
	{
		Respond(MERR_CANNOT_EQUIP_ITEM);
		return;
	}

#ifdef UPDATE_STAGE_EQUIP_LOOK
	ResponseCharacterItemList(uidPlayer);

	if (FindStage(pObj->GetStageUID()))
	{
		MCommand* pEquipInfo = CreateCommand(MC_MATCH_ROUTE_UPDATE_STAGE_EQUIP_LOOK, MUID(0, 0));
		pEquipInfo->AddParameter(new MCmdParamUID(uidPlayer));
		pEquipInfo->AddParameter(new MCmdParamInt(parts));
		pEquipInfo->AddParameter(new MCmdParamInt(pItem->GetDescID()));
		RouteToStage(pObj->GetStageUID(), pEquipInfo);
	}
#else
	Respond(MOK);
#endif
}

void MMatchServer::OnRequestTakeoffItem(const MUID& uidPlayer, const u32 nEquipmentSlot)
{
	if (nEquipmentSlot >= MMCIP_END) return;
	MMatchCharItemParts parts = MMatchCharItemParts(nEquipmentSlot);

	ResponseTakeoffItem(uidPlayer, parts);
}



void MMatchServer::ResponseTakeoffItem(const MUID& uidPlayer, const MMatchCharItemParts parts)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;

	MMatchCharInfo* pCharInfo = pObj->GetCharInfo();
	if (pCharInfo == NULL) return;
	if (pCharInfo->m_EquipedItem.IsEmpty(parts)) return;


	
	MMatchItem* pItem = pCharInfo->m_EquipedItem.GetItem(parts);
	MMatchItemDesc* pItemDesc = pItem->GetDesc();
	if (pItemDesc == NULL) return;

	auto Respond = [&](int nResult) {
		MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_TAKEOFF_ITEM, MUID(0, 0));
		pNew->AddParameter(new MCommandParameterInt(nResult));
		RouteToListener(pObj, pNew);
	};

	int nWeight=0, nMaxWeight=0;
	pCharInfo->m_EquipedItem.GetTotalWeight(&nWeight, &nMaxWeight);
	nMaxWeight = pCharInfo->m_nMaxWeight + nMaxWeight - pItemDesc->m_nMaxWT;
	nWeight -= pItemDesc->m_nWeight;

	if (nWeight > nMaxWeight)
	{
		Respond(MERR_CANNOT_TAKEOFF_ITEM_BY_WEIGHT);
		return;
	}

	pCharInfo->m_EquipedItem.Remove(parts);

	if (!GetDBMgr()->UpdateEquipedItem(pCharInfo->m_nCID, parts, 0, 0))
	{
		Respond(MERR_CANNOT_TAKEOFF_ITEM);
		return;
	}

#ifdef UPDATE_STAGE_EQUIP_LOOK
	ResponseCharacterItemList(uidPlayer);

	if (FindStage(pObj->GetStageUID()))
	{
		MCommand* pEquipInfo = CreateCommand(MC_MATCH_ROUTE_UPDATE_STAGE_EQUIP_LOOK, MUID(0, 0));
		pEquipInfo->AddParameter(new MCmdParamUID(uidPlayer));
		pEquipInfo->AddParameter(new MCmdParamInt(parts));
		pEquipInfo->AddParameter(new MCmdParamInt(0));
		RouteToStage(pObj->GetStageUID(), pEquipInfo);
	}
#else
	Respond(MOK);
#endif
}


void MMatchServer::OnRequestBringAccountItem(const MUID& uidPlayer, const int nAIID)
{
	ResponseBringAccountItem(uidPlayer, nAIID);
}


void MMatchServer::ResponseBringAccountItem(const MUID& uidPlayer, const int nAIID)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if ((pObj == NULL) || (pObj->GetCharInfo() == NULL)) return;

	MMatchCharInfo* pCharInfo = pObj->GetCharInfo();

	// Async DB
	MAsyncDBJob_BringAccountItem* pJob = new MAsyncDBJob_BringAccountItem(pObj->GetUID());
	pJob->Input(pObj->GetAccountInfo()->m_nAID, pObj->GetCharInfo()->m_nCID, nAIID);
	PostAsyncJob(pJob);
}


void MMatchServer::OnRequestBringBackAccountItem(const MUID& uidPlayer, const MUID& uidItem)
{
	ResponseBringBackAccountItem(uidPlayer, uidItem);
}

void MMatchServer::ResponseBringBackAccountItem(const MUID& uidPlayer, const MUID& uidItem)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if ((pObj == NULL) || (pObj->GetCharInfo() == NULL)) return;

	MMatchCharInfo* pCharInfo = pObj->GetCharInfo();

	int nRet = MERR_UNKNOWN;

	MUID uidCharItem = uidItem;
	MMatchItem* pItem = pObj->GetCharInfo()->m_ItemList.GetItem(uidCharItem);
	if ((pItem == NULL) || (pItem->GetDesc() == NULL))
	{
		// �������� �������� ������
		MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_BRING_BACK_ACCOUNTITEM, MUID(0,0));
		pNew->AddParameter(new MCmdParamInt(MERR_BRING_BACK_ACCOUNTITEM));
		RouteToListener(pObj, pNew);

		return;
	}

	// ����ϰ� ������ �ű� �� ����
	if (pItem->IsEquiped())
	{
		MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_BRING_BACK_ACCOUNTITEM, MUID(0,0));
		pNew->AddParameter(new MCmdParamInt(MERR_BRING_BACK_ACCOUNTITEM));
		RouteToListener(pObj, pNew);

		return;
	}

	// ĳ���������� �ƴϸ� �ű� �� ����
	if(!pItem->GetDesc()->IsCashItem())
	{
		MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_BRING_BACK_ACCOUNTITEM, MUID(0,0));
		pNew->AddParameter(new MCmdParamInt(MERR_BRING_BACK_ACCOUNTITEM_FOR_CASHITEM));
		RouteToListener(pObj, pNew);

		return;
	}

	// ��񿡼� �߾��������� �Ű��ش�.
	if (!GetDBMgr()->BringBackAccountItem(pObj->GetAccountInfo()->m_nAID, 
										  pObj->GetCharInfo()->m_nCID, 
										  pItem->GetCIID()))
	{
		mlog("DB Query(ResponseBringBackAccountItem > BringBackAccountItem) Failed(ciid=%u)\n", pItem->GetCIID());

		MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_BRING_BACK_ACCOUNTITEM, MUID(0,0));
		pNew->AddParameter(new MCmdParamInt(MERR_BRING_BACK_ACCOUNTITEM));
		RouteToListener(pObj, pNew);

		return;
	}


	// ������Ʈ���� ������ ����
	pObj->GetCharInfo()->m_ItemList.RemoveItem(uidCharItem);

	MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_BRING_BACK_ACCOUNTITEM, MUID(0,0));
	pNew->AddParameter(new MCmdParamInt(MOK));
	RouteToListener(pObj, pNew);


	ResponseCharacterItemList(uidPlayer);	// ���� �ٲ� ������ ����Ʈ�� �ٽ� �ѷ��ش�.
}
