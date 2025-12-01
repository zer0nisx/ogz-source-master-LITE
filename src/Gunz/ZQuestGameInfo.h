#ifndef _ZQUEST_GAMEINFO_H
#define _ZQUEST_GAMEINFO_H

#include "MBaseQuest.h"
#include <vector>
using namespace std;

// ����Ʈ�� ���� ����
class ZQuestGameInfo
{
private:
	bool							m_bInitialized;
	vector<MQUEST_NPC>				m_NPCInfoVector;
	vector<MQuestLevelSectorNode>	m_MapSectorVector;
	int								m_nQL;						// ���� �ó������� ����Ʈ ����
	float							m_fNPC_TC;
	int								m_nNPCCount;				// �� ���ʹ� ������ �� NPC��
	int								m_nNPCKilled;				// �� ���ʹ� ���� NPC ��
	int								m_nCurrSectorIndex;			// ���� �������� ���� �ε���
	int								m_nNumOfObtainQuestItem;	// ����Ʈ ���� ȹ���� ������ ����
	vector<MUID>					m_Bosses;					// ���� UID
	rvector							m_vPortalPos;				// ��Ż�� ��ġ
public:
	ZQuestGameInfo();
	~ZQuestGameInfo();
	void Init(MTD_QuestGameInfo* pMTDQuestGameInfo);
	void Final();
	void OnMovetoNewSector(int nSectorIndex);				// ���ο� ���ͷ� �̵�

	// interface func
	int GetNPCInfoCount();
	int GetMapSectorCount();
	MQUEST_NPC GetNPCInfo(int index);
	int GetMapSectorID(int index);
	int GetMapSectorLink(int index);
	bool IsInited();
	inline float GetNPC_TC();
	inline bool IsCurrSectorLastSector();			// ���� ���Ͱ� ������ �������� ����
	int GetCurrSectorIndex()				{ return m_nCurrSectorIndex; }
	int GetNPCCount( void)					{ return m_nNPCCount; }
	int GetNPCKilled( void)					{ return m_nNPCKilled; }
	void ClearNPCKilled( void)				{ m_nNPCKilled = 0; }
	void IncreaseNPCKilled( void)			{ m_nNPCKilled++; }
	void SetQuestLevel( int nQL)			{ m_nQL = nQL; }
	int GetQuestLevel( void)				{ return m_nQL; }
	int GetNumOfObtainQuestItem( void)		{ return m_nNumOfObtainQuestItem; }
	void IncreaseObtainQuestItem( void)		{ m_nNumOfObtainQuestItem++; }
	vector<MUID>& GetBosses(void)			{ return m_Bosses; }
	MUID GetBoss();
	void SetPortalPos(rvector& pos)			{ m_vPortalPos = pos; }
	rvector GetPortalPos()					{ return m_vPortalPos; }
};



/////////////////////////////////////////////////////////////////////////////////////
inline int ZQuestGameInfo::GetNPCInfoCount() 
{ 
	return (int)m_NPCInfoVector.size(); 
}
inline int ZQuestGameInfo::GetMapSectorCount() 
{ 
	return (int)m_MapSectorVector.size(); 
}
inline MQUEST_NPC ZQuestGameInfo::GetNPCInfo(int index) 
{ 
	if (index < 0 || index >= (int)m_NPCInfoVector.size()) {
		mlog("ZQuestGameInfo::GetNPCInfo - Invalid index %d (size: %d), returning 0\n", 
			index, (int)m_NPCInfoVector.size());
		return MQUEST_NPC(0);
	}
	return m_NPCInfoVector[index]; 
}
inline int ZQuestGameInfo::GetMapSectorID(int index) 
{ 
	if (index < 0 || index >= (int)m_MapSectorVector.size()) {
		mlog("ZQuestGameInfo::GetMapSectorID - Invalid index %d (size: %d), returning 0\n", 
			index, (int)m_MapSectorVector.size());
		return 0;
	}
	return m_MapSectorVector[index].nSectorID; 
}
inline int ZQuestGameInfo::GetMapSectorLink(int index)
{
	if (index < 0 || index >= (int)m_MapSectorVector.size()) {
		mlog("ZQuestGameInfo::GetMapSectorLink - Invalid index %d (size: %d), returning 0\n", 
			index, (int)m_MapSectorVector.size());
		return 0;
	}
	return m_MapSectorVector[index].nNextLinkIndex;
}

inline bool ZQuestGameInfo::IsInited()
{
	return m_bInitialized; 
}

inline float ZQuestGameInfo::GetNPC_TC()
{
	return m_fNPC_TC;
}

bool ZQuestGameInfo::IsCurrSectorLastSector()
{
	if (GetMapSectorCount() <= (m_nCurrSectorIndex+1)) return true;
	return false;
}

#endif