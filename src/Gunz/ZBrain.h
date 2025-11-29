#pragma once

#include "ZActorAnimation.h"
#include "ZTask.h"
#include "ZTaskManager.h"
#include "ZBehavior.h"
#include "ZTimer.h"

// SUMMER-SOURCE: Distancias configurables para comportamiento de NPCs
#define DIST_FORCEDIN		300000.0f
#define DIST_IN				1600000.0f
#define DIST_OUT			5000000.0f
#define DIST_HEIGHT			320.0f

class ZActor;

class ZBrain
{
	friend ZActor;
	friend ZActorAnimation;
private:
	virtual void OnBody_AnimEnter(ZA_ANIM_STATE nAnimState);
	virtual void OnBody_AnimExit(ZA_ANIM_STATE nAnimState);
	virtual void OnBody_CollisionWall();
	virtual void OnBody_OnTaskFinished(ZTASK_ID nLastID);

	ZUpdateTimer		m_PathFindingTimer;
	ZUpdateTimer		m_AttackTimer;
	ZUpdateTimer		m_DefaultAttackTimer;

	// SUMMER-SOURCE: Timers para sistema de neglect
	DWORD				m_dwNoSkillTimer;    // Timer para evitar uso inmediato de skills
	DWORD				m_dwNeglectTimer;    // Timer para NPCs inactivos

	// SUMMER-SOURCE: Distancias configurables con variación aleatoria
	float				m_fDistForcedIn;
	float				m_fDistIn;
	float				m_fDistOut;

protected:
	ZActor* m_pBody;
	ZBehavior			m_Behavior;
	MUID				m_uidTarget;

	// SUMMER-SOURCE: Variables para sistema anti-stuck
	rvector				m_exPosition;              // Posición anterior del NPC
	DWORD				m_dwExPositionTime;       // Tiempo de última posición
	rvector				m_exPositionForWarp;      // Posición para warp
	DWORD				m_dwExPositionTimeForWarp;// Tiempo para warp

	std::list<rvector>		m_WayPointList;
	bool BuildPath(rvector& vTarPos);
	void DrawDebugPath();
	void PushPathTask();

	MQUEST_NPC_ATTACK CheckAttackable();

	bool CheckSkillUsable(int* pnSkill, MUID* pTarget, rvector* pTargetPosition);

	bool FindTarget();
	void ProcessAttack(float fDelta);
	void ProcessBuildPath(float fDelta);
	void DefaultAttack(MQUEST_NPC_ATTACK nNpcAttackType);
	void UseSkill(int nSkill, MUID& uidTarget, rvector& vTargetPos);

	float MakePathFindingUpdateTime(char nIntelligence);
	float MakeAttackUpdateTime(char nAgility);
	float MakeDefaultAttackCoolTime();

	// SUMMER-SOURCE: Sistema de neglect
	void MakeNeglectUpdateTime();

	// SUMMER-SOURCE: Sistema anti-stuck
	void AdjustWayPointWithBound(std::list<rvector>& wayPointList, class RNavigationMesh* pNavMesh);
	bool EscapeFromStuckIn(std::list<rvector>& wayPointList);
	void ResetStuckInState();
	void ResetStuckInStateForWarp();
	void PushWayPointsToTask();  // Renombrado de PushPathTask para consistencia

	virtual bool CheckEnableTargetting(ZCharacter* pCharacter);

	// REFACTORIZACIÓN: Helper para eliminar código duplicado de verificación de tareas
	bool IsTaskBlockingPathFinding() const;
	bool IsTaskBlockingSkill() const;
public:
	ZBrain();
	virtual ~ZBrain();
	void Init(ZActor* pBody);
	void Think(float fDelta);
	ZActor* GetBody() { return m_pBody; }
	ZObject* GetTarget();
	void DebugTest();

	// SUMMER-SOURCE: Método para cambiar comportamiento cuando recibe daño
	void OnDamaged();

	static float MakeSpeed(float fSpeed);
	static ZBrain* CreateBrain(MQUEST_NPC nNPCType);
};
