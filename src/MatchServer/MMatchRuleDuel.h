#ifndef _MMATCHRULE_DUEL_H
#define _MMATCHRULE_DUEL_H


#include "MMatchRule.h"
#include <list>

using namespace std;

class MMatchRuleDuel : public MMatchRule {
private:
	list<MUID>		WaitQueue;										///< ��⿭ ť
	bool			isChangeChampion;								///< ���ڰ� �ٲ��°�
	bool			isRoundEnd;										///< ���� �����°�
	bool			isTimeover;										///< Ÿ�ӿ��� �ƴ°�
	int				nVictory;										///< ���¼�
protected:	
	virtual bool RoundCount();										///< ���� ī��Ʈ. ��� ���尡 ������ false�� ��ȯ�Ѵ�.

	virtual void OnBegin();											///< ��ü ���� ���۽� ȣ��
	virtual void OnEnd();											///< ��ü ���� ����� ȣ��
	virtual void OnRoundBegin();									///< ���� ������ �� ȣ��
	virtual void OnRoundEnd();										/// ���� ���� �� ȣ��
	virtual void OnRoundTimeOut();									///< ���尡 Ÿ�Ӿƿ����� ����� �� OnRoundEnd() ���̴�.

	virtual bool OnCheckRoundFinish();								///< ���尡 �������� üũ

	virtual void OnGameKill(const MUID& uidAttacker, const MUID& uidVictim);	///< ų������ �������� ų���� è�Ǿ��� ų���� üũ

	virtual bool OnCheckEnableBattleCondition();					///< ���� �������� üũ

	/// ������ �����Ҷ� ȣ��ȴ�.
	virtual void OnEnterBattle(MUID& uidChar);
	// ������ �������� ȣ��ȴ�.
	virtual void OnLeaveBattle(MUID& uidChar);	

	virtual void OnTeam(const MUID &uidPlayer, enum MMatchTeam nTeam) override;

	void		 SpawnPlayers();									///< �÷��̾���� ������Ų��.

	void		LogInfo();

	void		SendQueueInfo(bool isRoundEnd = false);				///< �÷��̾�鿡�� ť ������ ����

public:
	int				GetVictory() { return nVictory;	}					///< ���¼� ����
	MUID			uidChampion;									///< 1������ ���
	MUID			uidChallenger;									///< ������

	MMatchRuleDuel(MMatchStage* pStage);
	virtual ~MMatchRuleDuel() { }
	virtual MMATCH_GAMETYPE GetGameType() { return MMATCH_GAMETYPE_DUEL; }
};


#endif