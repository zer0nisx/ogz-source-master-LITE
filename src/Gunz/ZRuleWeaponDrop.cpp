#include "stdafx.h"
#include "ZRuleWeaponDrop.h"
#include "ZMatch.h"
#include "ZGameInterface.h"
#include "ZPost.h"
#include "MSharedCommandTable.h"
#include "MMatchItem.h"
#include "ZChat.h"
#include "ZColorTable.h"
#include "MFrame.h"
#include "MListBox.h"
#include "MButton.h"
#include "MLabel.h"
#include "ZCombatInterface.h"
#include "ZEquipmentListBox.h"

ZRuleWeaponDrop::ZRuleWeaponDrop(ZMatch* pMatch) : ZRule(pMatch),
m_nCurrentWeaponBoxUID(0),
m_bShowSelectionMenu(false),
m_pSelectionFrame(nullptr),
m_pWeaponListBox(nullptr),
m_pSelectButton(nullptr),
m_pCancelButton(nullptr),
m_pTitleLabel(nullptr)
{
}

ZRuleWeaponDrop::~ZRuleWeaponDrop()
{
	DestroySelectionDialog();
}

bool ZRuleWeaponDrop::OnCommand(MCommand* pCommand)
{
	if (!pCommand) return false;

	switch (pCommand->GetID())
	{
	case MC_MATCH_WEAPON_DROP_SHOW_SELECTION:
	{
		short nWorldItemUID = 0;
		int nWeaponCount = 0;

		if (!pCommand->GetParameter(&nWorldItemUID, 0, MPT_INT)) return false;
		if (!pCommand->GetParameter(&nWeaponCount, 1, MPT_INT)) return false;

		std::vector<WeaponSelectionItem> weapons;
		for (int i = 0; i < nWeaponCount; ++i)
		{
			u32 nItemID = 0;
			int nSlot = 0;
			if (!pCommand->GetParameter(&nItemID, 2 + i * 2, MPT_UINT)) return false;
			if (!pCommand->GetParameter(&nSlot, 2 + i * 2 + 1, MPT_INT)) return false;
			weapons.push_back({ nItemID, nSlot });
		}

		ShowWeaponSelectionMenu(nWorldItemUID, weapons);
		return true;
	}
	default:
		return ZRule::OnCommand(pCommand);
	}
}

void ZRuleWeaponDrop::ShowWeaponSelectionMenu(short nWorldItemUID, const std::vector<WeaponSelectionItem>& weapons)
{
	// Validate input
	if (weapons.empty())
	{
		ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM_GAME), "No weapons available to select.");
		return;
	}
	
	m_nCurrentWeaponBoxUID = nWorldItemUID;
	m_WeaponList = weapons;
	m_bShowSelectionMenu = true;
	
	// Create and show visual dialog
	CreateSelectionDialog();
	
	// Also show in chat as backup
	ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM_GAME), "=== Weapon Selection ===");
	ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM_GAME), "Select a weapon from the dialog or press number key (1-9, 0)");
}

void ZRuleWeaponDrop::SelectWeapon(int nIndex)
{
	if (nIndex < 0 || nIndex >= (int)m_WeaponList.size())
		return;

	if (m_nCurrentWeaponBoxUID == 0)
		return;

	// Send selection to server
	ZPostRequestSelectWeapon(m_nCurrentWeaponBoxUID, nIndex);

	// Hide menu
	HideWeaponSelectionMenu();
}

void ZRuleWeaponDrop::HideWeaponSelectionMenu()
{
	m_bShowSelectionMenu = false;
	m_nCurrentWeaponBoxUID = 0;
	m_WeaponList.clear();
	DestroySelectionDialog();
}

void ZRuleWeaponDrop::CreateSelectionDialog()
{
	// Destroy existing dialog if any
	DestroySelectionDialog();
	
	// Validate weapon list
	if (m_WeaponList.empty())
	{
		return;
	}
	
	ZGameInterface* pGameInterface = ZGetGameInterface();
	if (!pGameInterface) return;
	
	ZCombatInterface* pCombatInterface = pGameInterface->GetCombatInterface();
	if (!pCombatInterface) return;
	
	// Get screen dimensions
	int nScreenWidth = MGetWorkspaceWidth();
	int nScreenHeight = MGetWorkspaceHeight();
	
	// Calculate dialog size and position (centered)
	int nDialogWidth = 400;
	int nDialogHeight = 300;
	int nDialogX = (nScreenWidth - nDialogWidth) / 2;
	int nDialogY = (nScreenHeight - nDialogHeight) / 2;
	
	// Create frame
	m_pSelectionFrame = new MFrame("WeaponSelectionFrame", pCombatInterface, this);
	if (!m_pSelectionFrame)
	{
		return; // Failed to create frame
	}
	m_pSelectionFrame->SetBounds(nDialogX, nDialogY, nDialogWidth, nDialogHeight);
	m_pSelectionFrame->SetText("Select Weapon");
	m_pSelectionFrame->Show(true);

	// Create title label
	m_pTitleLabel = new MLabel("WeaponSelectionTitle", m_pSelectionFrame, this);
	if (!m_pTitleLabel)
	{
		DestroySelectionDialog();
		return;
	}
	m_pTitleLabel->SetBounds(10, 30, nDialogWidth - 20, 25);
	m_pTitleLabel->SetText("Choose a weapon to pick up:");
	m_pTitleLabel->SetAlignment(MAM_LEFT);
	m_pTitleLabel->Show(true);
	
	// Create listbox
	m_pWeaponListBox = new MListBox("WeaponSelectionList", m_pSelectionFrame, this);
	if (!m_pWeaponListBox)
	{
		DestroySelectionDialog();
		return;
	}
	m_pWeaponListBox->SetBounds(10, 60, nDialogWidth - 20, nDialogHeight - 120);
	m_pWeaponListBox->SetItemHeight(30);
	m_pWeaponListBox->SetVisibleHeader(false);
	m_pWeaponListBox->AddField("Icon", 40);
	m_pWeaponListBox->AddField("Name", nDialogWidth - 80);
	m_pWeaponListBox->Show(true);

	// Populate listbox with weapons
	for (size_t i = 0; i < m_WeaponList.size(); ++i)
	{
		MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(m_WeaponList[i].nItemID);
		const char* szItemName = pItemDesc ? pItemDesc->m_szName : "Unknown Weapon";

		const char* szSlotName = "";
		switch (m_WeaponList[i].nSlot)
		{
		case MMCIP_MELEE: szSlotName = "Melee"; break;
		case MMCIP_PRIMARY: szSlotName = "Primary"; break;
		case MMCIP_SECONDARY: szSlotName = "Secondary"; break;
		}

		// Get item icon using the existing function
		MBitmap* pIcon = GetItemIconBitmap(pItemDesc, true);

		char szDisplayName[256];
		sprintf_safe(szDisplayName, "%s (%s)", szItemName, szSlotName);

		MDefaultListItem* pItem = new MDefaultListItem(pIcon, szDisplayName);
		m_pWeaponListBox->Add(pItem);
	}

	// Select first item by default
	if (m_pWeaponListBox->GetCount() > 0)
		m_pWeaponListBox->SetSelIndex(0);

	// Create Select button
	m_pSelectButton = new MButton("WeaponSelectionSelect", m_pSelectionFrame, this);
	if (!m_pSelectButton)
	{
		DestroySelectionDialog();
		return;
	}
	m_pSelectButton->SetBounds(nDialogWidth - 180, nDialogHeight - 50, 80, 30);
	m_pSelectButton->SetText("Select");
	m_pSelectButton->Show(true);
	
	// Create Cancel button
	m_pCancelButton = new MButton("WeaponSelectionCancel", m_pSelectionFrame, this);
	if (!m_pCancelButton)
	{
		DestroySelectionDialog();
		return;
	}
	m_pCancelButton->SetBounds(nDialogWidth - 90, nDialogHeight - 50, 80, 30);
	m_pCancelButton->SetText("Cancel");
	m_pCancelButton->Show(true);
}

void ZRuleWeaponDrop::DestroySelectionDialog()
{
	if (m_pSelectionFrame)
	{
		// Widgets will be destroyed automatically when parent is destroyed
		SAFE_DELETE(m_pSelectionFrame);
		m_pWeaponListBox = nullptr;
		m_pSelectButton = nullptr;
		m_pCancelButton = nullptr;
		m_pTitleLabel = nullptr;
	}
}

bool ZRuleWeaponDrop::OnCommand(MWidget* pWidget, const char* szMessage)
{
	if (!pWidget || !szMessage) return false;

	// Handle selection dialog commands
	if (m_pSelectionFrame && pWidget->GetParent() == m_pSelectionFrame)
	{
		if (pWidget == m_pSelectButton && strcmp(szMessage, "OK") == 0)
		{
			// Select button clicked
			if (m_pWeaponListBox)
			{
				int nSelIndex = m_pWeaponListBox->GetSelIndex();
				if (nSelIndex >= 0 && nSelIndex < (int)m_WeaponList.size())
				{
					SelectWeapon(nSelIndex);
					return true;
				}
			}
		}
		else if (pWidget == m_pCancelButton && strcmp(szMessage, "OK") == 0)
		{
			// Cancel button clicked
			HideWeaponSelectionMenu();
			return true;
		}
		else if (pWidget == m_pWeaponListBox && strcmp(szMessage, "OK") == 0)
		{
			// Double-click on list item
			if (m_pWeaponListBox)
			{
				int nSelIndex = m_pWeaponListBox->GetSelIndex();
				if (nSelIndex >= 0 && nSelIndex < (int)m_WeaponList.size())
				{
					SelectWeapon(nSelIndex);
					return true;
				}
			}
		}
	}

	return false;
}