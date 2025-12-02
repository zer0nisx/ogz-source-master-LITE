#pragma once
#include "MMatchRule.h"
#include <map>

/// Modo de juego donde las armas se dropean al morir
class MMatchRuleWeaponDrop : public MMatchRule {
private:
	// Estructura para guardar items originales de cada jugador
	struct OriginalItems {
		u32 nMeleeID;
		u32 nPrimaryID;
		u32 nSecondaryID;
	};

	// Mapa para guardar items originales: MUID -> OriginalItems
	std::map<MUID, OriginalItems> m_OriginalItemsMap;
	
	// Mapa para rastrear items temporales creados: MUID -> vector de MUIDs de items temporales
	std::map<MUID, std::vector<MUID>> m_TemporaryItemsMap;
	
	// Estructura para guardar información de un arma en un weapon box
	struct DroppedWeapon {
		u32 nItemID;
		int nSlot; // MMCIP_MELEE, MMCIP_PRIMARY, MMCIP_SECONDARY
	};
	
	// Mapa para guardar armas en cada weapon box: WorldItem UID -> vector de DroppedWeapon
	std::map<short, std::vector<DroppedWeapon>> m_WeaponBoxMap;

	// Guardar items originales del jugador
	void SaveOriginalItems(MMatchObject* pObj);
	
	// Restaurar items originales del jugador
	void RestoreOriginalItems(MMatchObject* pObj);
	
	// Dropear armas del jugador muerto
	void DropWeapons(MMatchObject* pVictim);
	
	// Limpiar items temporales del jugador
	void CleanupTemporaryItems(MMatchObject* pObj);

protected:
	virtual void OnBegin() override;
	virtual void OnEnd() override;
	virtual void OnRoundBegin() override;
	virtual void OnRoundEnd() override;
	virtual void OnRoundTimeOut() override;
	virtual bool OnCheckRoundFinish() override;
	virtual bool RoundCount() override;
	virtual void OnGameKill(const MUID& uidAttacker, const MUID& uidVictim) override;
	virtual void OnCommand(class MCommand* pCommand) override;

public:
	MMatchRuleWeaponDrop(MMatchStage* pStage);
	virtual ~MMatchRuleWeaponDrop();
	
	virtual void OnEnterBattle(MUID& uidChar) override;
	virtual void OnLeaveBattle(MUID& uidChar) override;
	virtual void OnObtainWorldItem(MMatchObject* pObj, int nItemID, int* pnExtraValues, short nWorldItemUID = 0) override;
	virtual void OnRequestSelectWeapon(MMatchObject* pObj, short nWorldItemUID, int nWeaponIndex) override;
	virtual MMATCH_GAMETYPE GetGameType() override { return MMATCH_GAMETYPE_WEAPON_DROP; }
};

