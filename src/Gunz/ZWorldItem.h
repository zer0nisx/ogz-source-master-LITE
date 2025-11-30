#pragma once

#include "RTypes.h"
#include "RMesh.h"
#include "RVisualMeshMgr.h"
#include "list"
#include "map"
#include "MMatchWorldItemDesc.h"

#define _WORLD_ITEM_

using namespace std;
using namespace RealSpace2;

//enum ZWORLD_ITEM_SPAWN_TYPE
enum ZWORLD_ITEM_SPAWN_FLAG
{
	WORLD_ITEM_TIME_ONCE			= 0x1,		// �ѹ� �����ǰ� ������ �� ( DEFAULT)
	WORLD_ITEM_TIME_REGULAR			= 0x2,		// ���� �� ���� �ð��� ������ ������ 
	WORLD_ITEM_STAND_ALINE			= 0x4,		// ������ ������� �ʰ� Ŭ���̾�Ʈ�� ó��
};

enum ZWORLD_ITEM_STATE
{
	WORLD_ITEM_INVALIDATE = 0,
	WORLD_ITEM_VALIDATE,
	WORLD_ITEM_WAITING,
	WORLD_ITEM_CANDIDATE,	// Ŭ���̾�Ʈ������ ������ ��, ������ ������ �ֱ� ��ٸ��� �ִ� ����	
	NUM_WORLD_ITEM_STATE,
};

enum ZWORLD_ITEM_EFFECT
{
	WORLD_ITEM_EFFECT_CREATE = 0,
	WORLD_ITEM_EFFECT_IDLE,
	WORLD_ITEM_EFFECT_NUM,
	WORLD_ITEM_EFFECT_REMOVE,
};

#define MAX_NAME_LENGTH 256
class ZCharacter;

//////////////////////////////////////////////////////////////////////////
class ZWorldItem
{
protected:
	short					m_nID;							// �ν��Ͻ� ID
	short					m_nItemID;						// ������ ID
	char					m_Name[MAX_NAME_LENGTH];		// ������ �̸�
	char					m_modelName[MAX_NAME_LENGTH];	// �� �̸�
	MMATCH_WORLD_ITEM_TYPE	m_Type;							// �������� ����
	ZWORLD_ITEM_STATE		m_State;						// �������� ����
	MTD_WorldItemSubType	m_SubType;
	rvector					m_Position;						// �������� ���� ��ġ
	rvector					m_Dir;
	rvector					m_Up;
	unsigned int			m_nSpawnTypeFlags;
	float					m_fAmount;											
public:
	RVisualMesh*			m_pVMesh;
	unsigned int			m_dwStartTime;
	unsigned int			m_dwToggleBackupTime;
	bool					m_bToggle;
	bool					m_bisDraw;
public:
	// ������ ����
	void Initialize( int nID, short nItemID, MTD_WorldItemSubType SubType, ZWORLD_ITEM_STATE state, unsigned int nSpawnTypeFlags,	// ���� ����
		const rvector& position, float fAmount		);

	virtual bool ApplyWorldItem( ZCharacter* pCharacter );					// ������

	void CreateVisualMesh();

public:
	void SetPostion( const rvector& p )						{ m_Position = p; };
	void SetDir( const rvector& p )						{ m_Dir = p; };
	void SetUp( const rvector& p )						{ m_Up = p; };
	void SetState( ZWORLD_ITEM_STATE state )		{ m_State	= state; };
	void SetType( MMATCH_WORLD_ITEM_TYPE type )	{ m_Type = type; };
	void SetName( char* szName )							{ strcpy_safe(m_Name, szName );	};
	void SetModelName( char* szName )						{ strcpy_safe(m_modelName, szName );	};
	
	MTD_WorldItemSubType GetSubType()						{ return m_SubType; }

	rvector GetPosition() const									{ return m_Position; };
	rvector GetDir() const									{ return m_Dir; };
	rvector GetUp() const									{ return m_Up; };

	MMATCH_WORLD_ITEM_TYPE GetType() const	{ return m_Type;	};
	ZWORLD_ITEM_STATE GetState() const				{ return m_State; };
	int GetID() const										{ return m_nID;	};	
	short GetItemID() const											{ return m_nItemID; }
	const char* GetName() const								{ return m_Name; };
	const char* GetModelName() const						{ return m_modelName; };
	unsigned int GetSpawnTypeFlags() const					{ return m_nSpawnTypeFlags; };
	
public:
	ZWorldItem();
	~ZWorldItem();
};

//////////////////////////////////////////////////////////////////////////
typedef list<ZWorldItem* > WaitingList;

typedef map< int, ZWorldItem* >		WorldItemList;
typedef WorldItemList::iterator		WIL_Iterator;

typedef map<string, RealSpace2::RVisualMesh* >  WorldItemVMeshMap;
typedef WorldItemVMeshMap::iterator	WIVMM_iterator;

//////////////////////////////////////////////////////////////////////////
class ZWorldItemDrawer
{
protected:
	WorldItemVMeshMap mVMeshList;

protected:
	RVisualMesh* AddMesh( const char* pName );

public:
	void Clear();

	void DrawWorldItem( ZWorldItem* pWorldItem, bool Rotate = false );
	void DrawEffect( ZWORLD_ITEM_EFFECT effect, const rvector& pos );

public:
	~ZWorldItemDrawer();	
};

//////////////////////////////////////////////////////////////////////////
class ZWorldItemManager
{
protected:
	WorldItemList			mItemList;
	ZWorldItemDrawer	mDrawer;
	static ZWorldItemManager msInstance;

	int						m_nStandAloneIDGen;
	int						m_nUpdateCounter;		// Contador para throttling de update()
	int GenStandAlondID();
protected:
	bool ApplyWorldItem( WIL_Iterator& iter, ZCharacter* pCharacter );	// pCharacter���� pWorldItem�� �����Ű��
	void DeleteWorldItem( WIL_Iterator& iter, bool bDrawRemoveEffect );
	bool SpawnWorldItem( WIL_Iterator& iter );
	void OnOptainWorldItem(ZWorldItem* pItem);
public:
	void update();																			// ĳ���Ϳ��� �浹 üũ, ������ �����ð� üũ
	ZWorldItem *AddWorldItem( int nID, short nItemID,MTD_WorldItemSubType nItemSubType, const rvector& pos );	// �� �ε��� ������ �߰��Ҷ� ȣ���ϴ� �Լ�
			
	bool DeleteWorldItem( int nID, bool bDrawRemoveEffect=false );

	void Clear();											// ������ ����Ʈ, ������ ����Ʈ... // ���ӿ��� ���ö� ȣ��
	void Reset(bool bDrawRemoveEffect=false);				// �������� ���� �����Ҷ� ȣ��
	int GetLinkedWorldItemID(MMatchItemDesc* pItemDesc);		// �����۰� ����� ��������� ID ��ȯ

	bool ApplyWorldItem( int nID, ZCharacter* pCharacter );			// pCharacter���� pWorldItem�� �����Ű��
	
	
	void AddQuestPortal(rvector& pos);		// ����Ʈ���� ��Ż ������ ȣ��


	static ZWorldItemManager*	GetInstance()	{ return &msInstance; }
	

public:
	void Draw();
	void Draw(int mode,float height,bool bWaterMap);

public:
	ZWorldItemManager();
	~ZWorldItemManager() {};
};

ZWorldItemManager* ZGetWorldItemManager();


// worlditem.xml�� �ִ� Ư���� id
#define WORLDITEM_PORTAL_ID			201				// ����Ʈ���� ����ϴ� ��Ż
