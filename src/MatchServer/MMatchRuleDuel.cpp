#include "stdafx.h"
#include "MMatchRuleDuel.h"
#include "MMatchTransDataType.h"
#include "MBlobArray.h"
#include "MMatchServer.h"

#include <algorithm>
//////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////
// MMatchRuleDuel	   ///////////////////////////////////////////////////////////
MMatchRuleDuel::MMatchRuleDuel(MMatchStage* pStage) : MMatchRule(pStage)
{

}

void MMatchRuleDuel::OnBegin()
{
	uidChampion = MUID(0, 0);
	uidChallenger = MUID(0, 0);

	MMatchStage* pStage = GetStage();

	WaitQueue.clear();	// ��� ť�� ����

	if (pStage != NULL)
	{
		for(auto itor=pStage->GetObjBegin(); itor!=pStage->GetObjEnd(); itor++)
			WaitQueue.push_back(itor->first);			// �÷��̾�� �׳� ���� ��� ť�� �ִ´�.

//		SpawnPlayers();
	}

	nVictory = 0;

	return;
}

void MMatchRuleDuel::OnEnd()
{
}


void MMatchRuleDuel::OnRoundBegin()
{
	isRoundEnd = false;
	isTimeover = true;

	SpawnPlayers();
	SendQueueInfo(true);
	// �ֱ׷��� ���� ������ �ؾ� �� ���� �����Ȱ� ���� -_- �׿�������. �������� �����ȵ����� ���̻��� ó���� �ʿ������.
	// �̰� �� �Ŀ� �����Ǵ°Ÿ� ���ҵ�;
	for (list<MUID>::iterator i = WaitQueue.begin(); i!=WaitQueue.end();  ++i)
		MMatchServer::GetInstance()->OnDuelSetObserver(*i);							

}


void MMatchRuleDuel::OnRoundEnd()
{
	if (isTimeover)
	{	
		WaitQueue.push_back(uidChampion);
		WaitQueue.push_back(uidChallenger);
		uidChampion = uidChallenger = MUID(0, 0);
		nVictory = 0;
	}
	else
	{
		if (isChangeChampion || uidChampion == MUID(0, 0))				// è�ǿ��� �ٲ��� �ϸ� �ϴ� è�ǿ°� �����ڸ� ����
		{
			MUID uidTemp;
			uidTemp = uidChampion;
			uidChampion = uidChallenger;
			uidChallenger = uidTemp;
		}

		if (uidChallenger != MUID(0, 0))
		{
			WaitQueue.push_back(uidChallenger);	// �����ڴ� ť�� �� �ڷ� �о�ְ�
			uidChallenger = MUID(0, 0);			// �������� id�� ��ȿȭ
		}
	}

//	SpawnPlayers();
	LogInfo();
}

bool MMatchRuleDuel::RoundCount() 
{
	if (m_pStage == NULL) return false;

	int nTotalRound = m_pStage->GetStageSetting()->GetRoundMax();
	m_nRoundCount++;

	MMatchStage* pStage = GetStage();
	for (auto i=pStage->GetObjBegin(); i!=pStage->GetObjEnd(); i++)
	{
		MMatchObject* pObj = i->second;
		if (pObj->GetEnterBattle() == false) continue;

		if (pObj->GetAllRoundKillCount() >= (unsigned int)pStage->GetStageSetting()->GetRoundMax())
		{
			return false;
		}
	}

	return true;
}

bool MMatchRuleDuel::OnCheckRoundFinish()
{
	if (!isRoundEnd)
		return false;
	else
	{
		isRoundEnd = false;
		isTimeover = false;
		return true;	
	}
}

void MMatchRuleDuel::OnRoundTimeOut()
{
	SetRoundArg(MMATCH_ROUNDRESULT_DRAW);
}


void MMatchRuleDuel::OnGameKill(const MUID& uidAttacker, const MUID& uidVictim)
{
	isRoundEnd = true;

	MUID chanID = MMatchServer::GetInstance()->GetChannelMap()->Find(m_pStage->GetOwnerChannel())->GetUID();

	if (uidVictim == uidChallenger)		// è�ǿ��� ������� è�ǿ� ����
	{
		isChangeChampion = false;
		nVictory++;

		if (m_pStage == NULL) return;
		if (m_pStage->IsPrivate()) return;		// ��й��̸� ��� �н�

		if (nVictory % 10 != 0) return;			// ���¼��� 10�� ����϶���

		MMatchObject* pChamp;
		pChamp = m_pStage->GetObj(uidChampion);
		if (pChamp == NULL) return;


		MMatchServer::GetInstance()->BroadCastDuelRenewVictories(
			chanID,
			pChamp->GetName(), 
			MMatchServer::GetInstance()->GetChannelMap()->Find(m_pStage->GetOwnerChannel())->GetName(), 
			m_pStage->GetIndex()+1,
			nVictory
			);
	}
	else
	{
		isChangeChampion = true;

		int nowVictory = nVictory;

		nVictory = 1;

		if (nowVictory < 10) return;				// 10���� �̻��� ������������
		if (m_pStage == NULL) return;
		if (m_pStage->IsPrivate()) return;		// ��й��̸� ��� �н�
	
		MMatchObject* pChamp, *pChallenger;
		pChamp = m_pStage->GetObj(uidChampion);
		if (pChamp == NULL) return;
		pChallenger = m_pStage->GetObj(uidChallenger);
		if (pChallenger == NULL) return;

		if (strcmp(m_pStage->GetPassword(), "") != 0) return;

		MMatchServer::GetInstance()->BroadCastDuelInterruptVictories(
			chanID,
			pChamp->GetName(),
			pChallenger->GetName(),
			nowVictory
			);
	}


	LogInfo();

}

void MMatchRuleDuel::OnEnterBattle(MUID& uidChar)
{
	MMatchObject* pChar;
	pChar = m_pStage->GetObj(uidChar);
	if (pChar->GetTeam() == MMT_SPECTATOR)
		return;

	if ((uidChar != uidChampion) && (uidChar != uidChallenger) && (find(WaitQueue.begin(), WaitQueue.end(), uidChar) == WaitQueue.end()))
	{
		WaitQueue.push_back(uidChar);
		SpawnPlayers();
	}
	SendQueueInfo();
	LogInfo();
}

void MMatchRuleDuel::OnLeaveBattle(MUID& uidChar)
{
	if (uidChar == uidChampion)
	{
		isChangeChampion = true;
		isRoundEnd = true;
		uidChampion = MUID(0, 0);
		nVictory = 0;
	}
	else if (uidChar == uidChallenger)
	{
		isChangeChampion = false;
		isRoundEnd = true;
		uidChallenger = MUID(0, 0);
	}
	else
	{
		WaitQueue.remove(uidChar);
		SendQueueInfo();
		LogInfo();
	}
}

void MMatchRuleDuel::OnTeam(const MUID &uidPlayer, enum MMatchTeam nTeam)
{
	if (nTeam != MMT_SPECTATOR)
		return;

	auto it = std::find(WaitQueue.begin(), WaitQueue.end(), uidPlayer);

	if (it == WaitQueue.end())
		return;

	WaitQueue.remove(uidPlayer);
}

void MMatchRuleDuel::SpawnPlayers()
{
	if (uidChampion == MUID(0, 0))
	{
		if (!WaitQueue.empty())
		{
			uidChampion = WaitQueue.front();
			WaitQueue.pop_front();
		}
	}
	if (uidChallenger == MUID(0, 0))
	{
		if (!WaitQueue.empty())
		{
			uidChallenger = WaitQueue.front();
			WaitQueue.pop_front();
		}
	}

	for (MUID UID : {uidChampion, uidChallenger})
	{
		auto Object = MGetMatchServer()->GetObject(UID);
		if (!Object || !(Object->GetPlayerFlags() & MTD_PlayerFlags_Bot)) continue;
		Object->SetAlive(true);
	}
}

bool MMatchRuleDuel::OnCheckEnableBattleCondition()
{
	if (uidChampion == MUID(0, 0) || uidChallenger == MUID(0, 0))
	{
		if (WaitQueue.empty())
			return false;
		else
			isRoundEnd = true;
	}

	return true;
}

void MMatchRuleDuel::LogInfo()
{
	if (m_pStage == NULL) return;
	MMatchObject* pObj;
	char buf[250];
	sprintf_safe(buf, "Logging Que--------------------\n");
	DMLog(buf);

	pObj = m_pStage->GetObj(uidChampion);
	if (pObj != NULL)
	{
		sprintf_safe(buf, "Champion name : %s \n", pObj->GetName());
		DMLog(buf);
	}

	pObj = m_pStage->GetObj(uidChallenger);
	if (pObj != NULL)
	{
		sprintf_safe(buf, "Challenger name : %s \n", pObj->GetName());
		DMLog(buf);
	}

	int x = 0;
	for (list<MUID>::iterator i = WaitQueue.begin(); i!=WaitQueue.end();  ++i)
	{
		pObj = m_pStage->GetObj(*i);
		if (pObj != NULL)
		{
			sprintf_safe(buf, "Wait Queue #%d : %s \n", x, pObj->GetName());
			DMLog(buf);		
			x++;
		}
	}
}

void MMatchRuleDuel::SendQueueInfo(bool isRoundEnd)
{
	if (m_pStage == NULL) return;
	MTD_DuelQueueInfo QInfo;
	QInfo.m_uidChampion = uidChampion;
	QInfo.m_uidChallenger = uidChallenger;
	QInfo.m_nQueueLength = static_cast<char>(WaitQueue.size());
	QInfo.m_nVictory = nVictory;
	QInfo.m_bIsRoundEnd = isRoundEnd;

	int i=0;
	for (list<MUID>::iterator iter = WaitQueue.begin(); iter != WaitQueue.end(); ++iter, ++i)
		QInfo.m_WaitQueue[i] = *iter;

	MMatchServer::GetInstance()->OnDuelQueueInfo(m_pStage->GetUID(), QInfo);
}