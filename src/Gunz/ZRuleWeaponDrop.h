#pragma once
#include "ZRule.h"
#include "MWidget.h"
#include <vector>

class ZMatch;
class MFrame;
class MListBox;
class MButton;
class MLabel;

struct WeaponSelectionItem {
	u32 nItemID;
	int nSlot;
};

class ZRuleWeaponDrop : public ZRule, public MListener
{
private:
	short m_nCurrentWeaponBoxUID;
	std::vector<WeaponSelectionItem> m_WeaponList;
	bool m_bShowSelectionMenu;
	
	// UI elements
	MFrame* m_pSelectionFrame;
	MListBox* m_pWeaponListBox;
	MButton* m_pSelectButton;
	MButton* m_pCancelButton;
	MLabel* m_pTitleLabel;

	void CreateSelectionDialog();
	void DestroySelectionDialog();
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage) override;

public:
	ZRuleWeaponDrop(ZMatch* pMatch);
	virtual ~ZRuleWeaponDrop();
	
	virtual bool OnCommand(MCommand* pCommand) override;
	
	void ShowWeaponSelectionMenu(short nWorldItemUID, const std::vector<WeaponSelectionItem>& weapons);
	void SelectWeapon(int nIndex);
	void HideWeaponSelectionMenu();
	bool IsSelectionMenuActive() const { return m_bShowSelectionMenu; }
};

