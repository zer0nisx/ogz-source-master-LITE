#include "stdafx.h"
#include "ZBrain.h"
#include "ZActor.h"
#include "ZGame.h"
#include "ZObject.h"
#include "ZMyCharacter.h"
#include "ZRangeWeaponHitDice.h"
#include "ZModule_Skills.h"
#include "MMath.h"
#include "RNavigationMesh.h"
#include "RNavigationNode.h"
#include "RBspObject.h"
#include "ZPickInfo.h"

ZBrain* ZBrain::CreateBrain(MQUEST_NPC nNPCType)
{
	return new ZBrain();
}

float ZBrain::MakePathFindingUpdateTime(char nIntelligence)
{
	MQuestNPCGlobalAIValue* pGlobalAIValue = ZGetQuest()->GetNPCCatalogue()->GetGlobalAIValue();
	float fShakingRatio = pGlobalAIValue->m_fPathFinding_ShakingRatio;
	float fTime = pGlobalAIValue->m_fPathFindingUpdateTime[nIntelligence - 1];

	float fExtraValue = fTime * fShakingRatio;
	float fMinTime = fTime - fExtraValue;
	if (fMinTime < 0.0f) fMinTime = 0.0f;
	float fMaxTime = fTime + fExtraValue;

	return RandomNumber(fMinTime, fMaxTime);
}

float ZBrain::MakeAttackUpdateTime(char nAgility)
{
	MQuestNPCGlobalAIValue* pGlobalAIValue = ZGetQuest()->GetNPCCatalogue()->GetGlobalAIValue();
	float fShakingRatio = pGlobalAIValue->m_fAttack_ShakingRatio;
	float fTime = pGlobalAIValue->m_fAttackUpdateTime[nAgility - 1];

	float fExtraValue = fTime * fShakingRatio;
	float fMinTime = fTime - fExtraValue;
	if (fMinTime < 0.0f) fMinTime = 0.0f;
	float fMaxTime = fTime + fExtraValue;

	return RandomNumber(fMinTime, fMaxTime);
}

float ZBrain::MakeSpeed(float fSpeed)
{
	MQuestNPCGlobalAIValue* pGlobalAIValue = ZGetQuest()->GetNPCCatalogue()->GetGlobalAIValue();
	float fShakingRatio = pGlobalAIValue->m_fSpeed_ShakingRatio;

	float fExtraValue = fSpeed * fShakingRatio;
	float fMinSpeed = max((fSpeed - fExtraValue), 0.0f);
	float fMaxSpeed = fSpeed + fExtraValue;

	return RandomNumber(fMinSpeed, fMaxSpeed);
}

float ZBrain::MakeDefaultAttackCoolTime()
{
	if (!m_pBody->GetNPCInfo()) return 0.0f;

	float fShakingRatio = 0.3f;
	float fCoolTime = m_pBody->GetNPCInfo()->fAttackCoolTime;

	float fExtraValue = fCoolTime * fShakingRatio;
	float fMinCoolTime = max((fCoolTime - fExtraValue), 0.01f);
	float fMaxCoolTime = fCoolTime + fExtraValue;

	return RandomNumber(fMinCoolTime, fMaxCoolTime);
}

void ZBrain::MakeNeglectUpdateTime()
{
	m_dwNeglectTimer = timeGetTime() + 5500;
}

ZBrain::ZBrain() : m_pBody(NULL), m_uidTarget(MUID(0, 0))
{
	ResetStuckInState();
	ResetStuckInStateForWarp();
}

ZBrain::~ZBrain()
{
}

void ZBrain::Init(ZActor* pBody)
{
	m_pBody = pBody;
	m_Behavior.Init(this);

	if (m_pBody->GetNPCInfo())
	{
		float fDefaultPathFindingUpdateTime = ZBrain::MakePathFindingUpdateTime(m_pBody->GetNPCInfo()->nIntelligence);
		float fAttackUpdateTime = ZBrain::MakeAttackUpdateTime(m_pBody->GetNPCInfo()->nAgility);
		float fDefaultAttackUpdateTime = m_pBody->GetNPCInfo()->fAttackCoolTime;

		m_PathFindingTimer.Init(fDefaultPathFindingUpdateTime);
		m_AttackTimer.Init(fAttackUpdateTime);
		m_DefaultAttackTimer.Init(fDefaultAttackUpdateTime);
	}

	// SUMMER-SOURCE: Inicializar timers y distancias configurables
	m_dwNoSkillTimer = timeGetTime() + RandomNumber(1000, 5000);

	// Neglect timer
	MakeNeglectUpdateTime();

	// Set distance
	m_fDistForcedIn = DIST_FORCEDIN + RandomNumber(1.0f, (DIST_FORCEDIN * 0.6f)) - (DIST_FORCEDIN * 0.6f / 2.0f);
	m_fDistIn = DIST_IN + RandomNumber(1.0f, (DIST_IN * 0.6f)) - (DIST_IN * 0.6f / 2.0f);
	m_fDistOut = DIST_OUT + RandomNumber(1.0f, (DIST_OUT * 0.6f)) - (DIST_OUT * 0.6f / 2.0f);
}

#include "ZGame.h"

void ZBrain::Think(float fDelta)
{
	if (m_pBody->isThinkAble())
	{
		MUID prevTarget = m_uidTarget;

		// Buscar objetivo
		bool bFind = FindTarget();

		// Si encontró objetivo...
		if (bFind)
		{
			// Pathfinding
			ProcessBuildPath(fDelta);

			// Ataque
			ProcessAttack(fDelta);
		}

		// Si perdió objetivo...
		else if (prevTarget != MUID(0, 0))
		{
			m_pBody->Stop();
			m_pBody->m_TaskManager.Clear();

			MakeNeglectUpdateTime();

			m_pBody->OnNeglect(1);
		}
	}

	// Check neglect
	DWORD dwCurrTime = timeGetTime();
	if (!m_pBody->m_TaskManager.IsEmpty())
		MakeNeglectUpdateTime();
	else if (dwCurrTime > m_dwNeglectTimer)
	{
		m_pBody->OnNeglect(1);
		MakeNeglectUpdateTime();
	}
}

void ZBrain::ProcessAttack(float fDelta)
{
	bool bDefaultAttackEnabled = true;

	// Update time
	if (m_pBody->GetNPCInfo() && (m_pBody->GetNPCInfo()->fAttackCoolTime != 0.0f))
		bDefaultAttackEnabled = m_DefaultAttackTimer.Update(fDelta);

	if (!m_AttackTimer.Update(fDelta) && !bDefaultAttackEnabled)
		return;

	// Skip if friendly NPC
	if (m_Behavior.IsFriendly())
		return;

	// Check attackable status
	if (!m_pBody->IsAttackable())
		return;

	// Use default attack
	if (bDefaultAttackEnabled && m_pBody->CanAttackMelee(GetTarget()) && !ZGetGame()->CheckWall(m_pBody, GetTarget(), true))
	{
		float fNextCoolTime = MakeDefaultAttackCoolTime();
		m_DefaultAttackTimer.Init(fNextCoolTime);

		ZTask* pNew = ZTaskManager::CreateAttackMelee(m_pBody);
		m_pBody->m_TaskManager.PushFront(pNew);

		return;
	}

	// Check skill useable
	if ((m_pBody->GetNPCInfo()->nNPCAttackTypes & NPC_ATTACK_MAGIC) == NPC_ATTACK_NONE)
		return;

	ZTASK_ID nTaskID = m_pBody->m_TaskManager.GetCurrTaskID();
	if ((nTaskID == ZTID_SKILL) || (nTaskID == ZTID_ROTATE_TO_DIR))
		return;

	if (timeGetTime() < m_dwNoSkillTimer)
		return;

	// Get skill
	int nSkill;
	MUID uidTarget;
	rvector targetPosition;
	if (CheckSkillUsable(&nSkill, &uidTarget, &targetPosition))
	{
		// Use skill
		if (m_pBody->CanSee(GetTarget()))
		{
			m_pBody->m_TaskManager.Clear();

			ZTask* pNew = ZTaskManager::CreateSkill(m_pBody, nSkill, uidTarget, targetPosition);
			m_pBody->m_TaskManager.Push(pNew);
		}
	}
}

bool ZBrain::IsTaskBlockingSkill() const
{
	ZTASK_ID nTaskID = m_pBody->m_TaskManager.GetCurrTaskID();
	return (nTaskID != ZTID_NONE) &&
		(nTaskID != ZTID_SKILL) &&
		(nTaskID != ZTID_ROTATE_TO_DIR);
}

void ZBrain::UseSkill(int nSkill, MUID& uidTarget, rvector& vTargetPos)
{
	if (IsTaskBlockingSkill()) return;

#ifdef _DEBUG
	mlog("Use Skill(id=%d, skill=%d)\n", m_pBody->GetUID().Low, nSkill);
#endif

	rvector dir = vTargetPos - m_pBody->GetPosition();
	Normalize(dir);

	m_pBody->m_TaskManager.Clear();
	ZTask* pNew;
	pNew = ZTaskManager::CreateRotateToDir(m_pBody, dir);
	m_pBody->m_TaskManager.Push(pNew);
	pNew = ZTaskManager::CreateSkill(m_pBody, nSkill, uidTarget, vTargetPos);
	m_pBody->m_TaskManager.Push(pNew);
}

void ZBrain::DefaultAttack(MQUEST_NPC_ATTACK nNpcAttackType)
{
	switch (nNpcAttackType)
	{
	case NPC_ATTACK_MELEE:
	{
		ZTask* pNew = ZTaskManager::CreateAttackMelee(m_pBody);
		m_pBody->m_TaskManager.PushFront(pNew);
	}
	break;
	case NPC_ATTACK_RANGE:
	{
		ZObject* pTarget = GetTarget();
		if (pTarget)
		{
			ZRangeWeaponHitDice dice;
			dice.BuildSourcePosition(m_pBody->GetPosition());
			dice.BuildTargetPosition(pTarget->GetPosition());
			dice.BuildTargetBounds(pTarget->GetCollRadius(), pTarget->GetCollHeight());
			float fTargetSpeed = Magnitude(pTarget->GetVelocity());
			dice.BuildTargetSpeed(fTargetSpeed);
			dice.BuildGlobalFactor(m_pBody->GetHitRate());

			rvector shot_dir = dice.ReturnShotDir();

			ZTask* pNew = ZTaskManager::CreateAttackRange(m_pBody, shot_dir);
			m_pBody->m_TaskManager.PushFront(pNew);
		}
	}
	break;
	}
}

bool ZBrain::IsTaskBlockingPathFinding() const
{
	ZTASK_ID nTaskID = m_pBody->m_TaskManager.GetCurrTaskID();
	return (nTaskID == ZTID_ATTACK_MELEE) ||
		(nTaskID == ZTID_ATTACK_RANGE) ||
		(nTaskID == ZTID_ROTATE_TO_DIR) ||
		(nTaskID == ZTID_SKILL);
}

void ZBrain::ProcessBuildPath(float fDelta)
{
	if (!m_PathFindingTimer.Update(fDelta))
		return;

	ZTASK_ID nTaskID = m_pBody->m_TaskManager.GetCurrTaskID();
	if ((nTaskID == ZTID_ATTACK_MELEE) || (nTaskID == ZTID_ATTACK_RANGE) || (nTaskID == ZTID_ROTATE_TO_DIR) || (nTaskID == ZTID_SKILL))
		return;

	if (ZGetGame()->GetMatch() &&
		ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_QUEST)
	{
		if (EscapeFromStuckIn(m_WayPointList))
			return;
	}

	ZObject* pTarget = GetTarget();
	if (!pTarget)
	{
		m_pBody->m_TaskManager.Clear();
		m_pBody->Stop();

		return;
	}

	if ((m_Behavior.GetOffenseType() == ZOFFENSETYPE_RANGE) || m_Behavior.IsFriendly())
	{
		float dist = MagnitudeSq(pTarget->GetPosition() - m_pBody->GetPosition());

		bool bStop = false;

		if (m_Behavior.IsFriendly())
		{
			if (dist < m_fDistForcedIn)
				bStop = true;
		}
		else
		{
			if ((dist > DIST_FORCEDIN) && (dist < m_fDistIn))
			{
				dist = pTarget->GetPosition().z - m_pBody->GetPosition().z;

				if ((dist > -DIST_HEIGHT) && (dist < DIST_HEIGHT))
					bStop = true;
			}
		}

		if (bStop)
		{
			if (m_pBody->CanSee(pTarget) && m_pBody->CanAttackRange(pTarget))
			{
				m_pBody->Stop();
				m_pBody->m_TaskManager.Clear();

				return;
			}
		}
	}

	RNavigationMesh* pNavMesh = ZGetGame()->GetWorld()->GetBsp()->GetNavigationMesh();
	if (pNavMesh == NULL)
		return;

	rvector tarpos = pTarget->GetPosition();
	if (!pNavMesh->BuildNavigationPath(m_pBody->GetPosition(), tarpos))
		return;

	m_WayPointList.clear();
	for (list<rvector>::iterator itor = pNavMesh->GetWaypointList().begin(); itor != pNavMesh->GetWaypointList().end(); ++itor)
		m_WayPointList.push_back((*itor));

	AdjustWayPointWithBound(m_WayPointList, pNavMesh);

	PushWayPointsToTask();
}

void ZBrain::OnBody_AnimEnter(ZA_ANIM_STATE nAnimState)
{
}

void ZBrain::OnBody_AnimExit(ZA_ANIM_STATE nAnimState)
{
}

void ZBrain::OnBody_CollisionWall()
{
}

void ZBrain::OnBody_OnTaskFinished(ZTASK_ID nLastID)
{
	if ((nLastID == ZTID_MOVE_TO_POS) || (nLastID == ZTID_MOVE_TO_DIR) || (nLastID == ZTID_MOVE_TO_TARGET))
	{
		if (GetTarget())
		{
			m_PathFindingTimer.Force();
		}
	}
}

bool ZBrain::BuildPath(rvector& vTarPos)
{
	RNavigationMesh* pNavMesh = ZGetGame()->GetWorld()->GetBsp()->GetNavigationMesh();
	if (pNavMesh != NULL)
	{
		if (pNavMesh->BuildNavigationPath(m_pBody->GetPosition(), vTarPos))
		{
			m_WayPointList.clear();

			for (list<rvector>::iterator itor = pNavMesh->GetWaypointList().begin();
				itor != pNavMesh->GetWaypointList().end(); ++itor)
			{
				m_WayPointList.push_back((*itor));
			}

			return true;
		}
	}

	return false;
}

void OutputDebugVector(char* szText, rvector& v)
{
	char text[128];
	sprintf_safe(text, "[%s] %.3f %.3f %.3f\n", szText, v.x, v.y, v.z);
	OutputDebugString(text);
}

void ZBrain::DebugTest()
{
	if (!m_WayPointList.empty())
	{
		float diff = 0.0f;
		rvector tar = *m_WayPointList.begin();
		rvector dir = tar - m_pBody->GetPosition();
		Normalize(dir);

		list<rvector>::iterator itor = m_WayPointList.begin();
		list<rvector>::iterator itorNext = itor;
		itorNext++;

		RGetDevice()->SetTexture(0, NULL);
		RGetDevice()->SetTexture(1, NULL);
		RGetDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);
		RGetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
		RGetDevice()->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);

		rmatrix birdinitmat;
		GetIdentityMatrix(birdinitmat);
		RSetTransform(D3DTS_WORLD, birdinitmat);

		rvector v1, v2;
		v1 = m_pBody->GetPosition();
		v2 = (*m_WayPointList.begin());
		v1.z = 0.0f;
		v2.z = 0.0f;
		RDrawLine(v1, v2, 0xFFFFFF00);

		while (itorNext != m_WayPointList.end())
		{
			v1 = (*itor);
			v2 = (*itorNext);
			v1.z = 0.0f;
			v2.z = 0.0f;
			RDrawLine(v1, v2, 0xFFFFFF00);

			++itor;
			++itorNext;
		}
	}
	RGetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
}

void ZBrain::DrawDebugPath()
{
	// DEBUG: Dibujar ruta del NPC
	if (!m_pBody || m_WayPointList.empty())
		return;

	RGetDevice()->SetTexture(0, NULL);
	RGetDevice()->SetTexture(1, NULL);
	RGetDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);
	RGetDevice()->SetRenderState(D3DRS_ZENABLE, TRUE);
	RGetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	RGetDevice()->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);

	rmatrix identity;
	GetIdentityMatrix(identity);
	RSetTransform(D3DTS_WORLD, identity);

	// Color para la ruta: Amarillo semitransparente
	const DWORD ROUTE_COLOR = 0x80FFFF00;  // Amarillo con alpha
	const DWORD WAYPOINT_COLOR = 0xFF00FF00;  // Verde para waypoints
	const DWORD CURRENT_POS_COLOR = 0xFFFF0000;  // Rojo para posición actual

	rvector currentPos = m_pBody->GetPosition();
	
	// Dibujar línea desde posición actual hasta el primer waypoint
	if (!m_WayPointList.empty())
	{
		rvector firstWaypoint = m_WayPointList.front();
		RDrawLine(currentPos, firstWaypoint, ROUTE_COLOR);
	}

	// Dibujar líneas entre waypoints
	list<rvector>::iterator itor = m_WayPointList.begin();
	list<rvector>::iterator itorNext = itor;
	if (itorNext != m_WayPointList.end())
		itorNext++;

	while (itorNext != m_WayPointList.end())
	{
		rvector from = *itor;
		rvector to = *itorNext;
		
		// Dibujar línea entre waypoints
		RDrawLine(from, to, ROUTE_COLOR);
		
		// Dibujar esfera pequeña en el waypoint
		const float WAYPOINT_SIZE = 20.0f;
		rvector up = rvector(0, 0, WAYPOINT_SIZE);
		RDrawLine(to, to + up, WAYPOINT_COLOR);
		RDrawLine(to, to - up, WAYPOINT_COLOR);
		rvector right = rvector(WAYPOINT_SIZE, 0, 0);
		RDrawLine(to, to + right, WAYPOINT_COLOR);
		RDrawLine(to, to - right, WAYPOINT_COLOR);
		rvector forward = rvector(0, WAYPOINT_SIZE, 0);
		RDrawLine(to, to + forward, WAYPOINT_COLOR);
		RDrawLine(to, to - forward, WAYPOINT_COLOR);

		itor++;
		itorNext++;
	}

	// Dibujar indicador en posición actual
	const float CURRENT_POS_SIZE = 30.0f;
	rvector up = rvector(0, 0, CURRENT_POS_SIZE);
	RDrawLine(currentPos, currentPos + up, CURRENT_POS_COLOR);
	RDrawLine(currentPos, currentPos - up, CURRENT_POS_COLOR);
	rvector right = rvector(CURRENT_POS_SIZE, 0, 0);
	RDrawLine(currentPos, currentPos + right, CURRENT_POS_COLOR);
	RDrawLine(currentPos, currentPos - right, CURRENT_POS_COLOR);
	rvector forward = rvector(0, CURRENT_POS_SIZE, 0);
	RDrawLine(currentPos, currentPos + forward, CURRENT_POS_COLOR);
	RDrawLine(currentPos, currentPos - forward, CURRENT_POS_COLOR);

	RGetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
}

void ZBrain::PushPathTask()
{
	if (!m_WayPointList.empty())
	{
		m_pBody->m_TaskManager.Clear();

		int nTotal = (int)m_WayPointList.size();
		int cnt = 0;
		for (list<rvector>::iterator itor = m_WayPointList.begin(); itor != m_WayPointList.end(); ++itor)
		{
			bool bChained = !((nTotal - 1) == cnt);

			ZTask* pNew = ZTaskManager::CreateMoveToPos(m_pBody, (*itor), bChained);
			m_pBody->m_TaskManager.Push(pNew);

			++cnt;
		}
	}
}

ZObject* ZBrain::GetTarget()
{
#ifdef _DEBUG
	if ((ZApplication::GetInstance()->GetLaunchMode() == ZApplication::ZLAUNCH_MODE_STANDALONE_QUEST) ||
		(ZApplication::GetInstance()->GetLaunchMode() == ZApplication::ZLAUNCH_MODE_STANDALONE_AI))
	{
		return (ZObject*)g_pGame->m_pMyCharacter;
	}

#endif

	if (ZGetObjectManager())
	{
		ZObject* pObject = ZGetObjectManager()->GetObject(m_uidTarget);
		return pObject;
	}
	return NULL;
}

MQUEST_NPC_ATTACK ZBrain::CheckAttackable()
{
	ZObject* pTarget = GetTarget();
	if ((pTarget == NULL) || (pTarget->IsDead())) return NPC_ATTACK_NONE;

	if (m_pBody->GetNPCInfo()->nNPCAttackTypes & NPC_ATTACK_MELEE)
	{
		if (m_pBody->CanAttackMelee(pTarget)) return NPC_ATTACK_MELEE;
	}
	else if (m_pBody->GetNPCInfo()->nNPCAttackTypes & NPC_ATTACK_RANGE)
	{
		if (m_pBody->CanAttackRange(pTarget)) return NPC_ATTACK_RANGE;
	}
	return NPC_ATTACK_NONE;
}

bool ZBrain::CheckSkillUsable(int* pnSkill, MUID* puidTarget, rvector* pTargetPosition)
{
	// Get skill module
	ZModule_Skills* pmod = (ZModule_Skills*)m_pBody->GetModule(ZMID_SKILLS);
	if (!pmod)
		return false;

	// Set value
	if (puidTarget)
		(*puidTarget) = MUID(0, 0);

	if (pTargetPosition)
		(*pTargetPosition) = rvector(0.0f, 0.0f, 0.0f);

	// Check skills
	for (int i = 0; i < pmod->GetSkillCount(); i++)
	{
		ZSkill* pSkill = pmod->GetSkill(i);

		// Check cool time
		if (!pSkill->IsReady())
			continue;

		// Get skill description
		ZSkillDesc* pDesc = pmod->GetSkill(i)->GetDesc();

		// Si el skill tiene objetivo aliado...
		if (pDesc->IsAlliedTarget())
		{
			// Buscar el aliado más cercano que pueda usar el skill
			float fDist = DIST_OUT;
			ZObject* pAlliedTarget = NULL;

			for (ZObjectManager::iterator itor = ZGetObjectManager()->begin(); itor != ZGetObjectManager()->end(); ++itor)
			{
				ZObject* pObject = itor->second;

				// Si está muerto, saltar
				if (pObject->IsDead())
					continue;

				// Si es enemigo, saltar
				if (ZGetGame()->IsAttackable(m_pBody, pObject))
					continue;

				// Si es uno mismo, saltar
				if (pObject == m_pBody)
					continue;

				// Get distance
				float dist = MagnitudeSq(pObject->GetPosition() - m_pBody->GetPosition());
				if (pSkill->IsUsable(pObject) && (dist < fDist))
				{
					fDist = dist;
					pAlliedTarget = pObject;
				}
			}

			// Si no hay aliado, usar el skill en uno mismo
			if ((pAlliedTarget == NULL) && (pSkill->IsUsable(m_pBody)))
				pAlliedTarget = m_pBody;

			if (pAlliedTarget)
			{
				if (pnSkill)
					*pnSkill = i;

				if (puidTarget)
					*puidTarget = pAlliedTarget->GetUID();

				if (pTargetPosition)
					*pTargetPosition = pAlliedTarget->GetCenterPos();

				return true;
			}
		}

		// Si el skill tiene objetivo enemigo...
		else
		{
			ZObject* pTarget = GetTarget();
			if (pTarget == NULL)
				continue;

			// Check useable
			if (!pSkill->IsUsable(pTarget))
				continue;

			// Get pick info
			ZPICKINFO pickinfo;
			memset(&pickinfo, 0, sizeof(ZPICKINFO));

			// Check picking
			rvector pos, tarpos, dir;
			// Posición desde la mitad de la altura del NPC hacia el objetivo
			pos = m_pBody->GetPosition() + rvector(0, 0, m_pBody->GetCollHeight() * 0.5f * 0.8f);
			tarpos = pTarget->GetPosition() + rvector(0, 0, pTarget->GetCollHeight() * 0.5f * 0.8f);
			dir = tarpos - pos;
			Normalize(dir);

			const DWORD dwPickPassFlag = RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSROCKET | RM_FLAG_PASSBULLET;
			if (ZGetGame()->Pick(m_pBody, pos, dir, &pickinfo, dwPickPassFlag))
			{
				if (pickinfo.pObject)
				{
					if (pnSkill)
						*pnSkill = i;

					if (puidTarget)
						*puidTarget = pTarget->GetUID();

					if (pTargetPosition)
						*pTargetPosition = pTarget->GetCenterPos();

					return true;
				}
			}
		}
	}

	return false;
}

bool ZBrain::FindTarget()
{
	MUID uidTarget = MUID(0, 0);
	float fDist = FLT_MAX;

	for (ZCharacterManager::iterator itor = ZGetCharacterManager()->begin(); itor != ZGetCharacterManager()->end(); ++itor)
	{
		// Verificar si el personaje está muerto
		ZCharacter* pCharacter = (*itor).second;
		if (pCharacter->IsDead())
			continue;

		// Personajes exceptuados de targeting
		if (ZGetGame()->IsExceptedFromNpcTargetting(pCharacter))
			continue;

		// Calcular distancia
		float dist = MagnitudeSq(pCharacter->GetPosition() - m_pBody->GetPosition());

		// Si está más cerca, actualizar objetivo
		if (dist < fDist)
		{
			fDist = dist;
			uidTarget = pCharacter->GetUID();
		}
	}

	m_uidTarget = uidTarget;

	if (uidTarget == MUID(0, 0))
		return false;

	return true;
}

bool ZBrain::CheckEnableTargetting(ZCharacter* pCharacter)
{
	u32 nAttackTypes = m_pBody->GetNPCInfo()->nNPCAttackTypes;
	if (nAttackTypes == NPC_ATTACK_MELEE)
	{
		if ((pCharacter->GetStateLower() == ZC_STATE_LOWER_BIND) &&
			(IS_EQ(MagnitudeSq(pCharacter->GetVelocity()), 0.0f)) &&
			(pCharacter->GetDistToFloor() >= m_pBody->GetCollHeight()))
		{
			return false;
		}
	}
	return true;
}

bool ZBrain::EscapeFromStuckIn(std::list<rvector>& wayPointList)
{
	DWORD currTime = timeGetTime();
	if (currTime - m_dwExPositionTimeForWarp > 2000)
	{
		rvector diff = m_exPositionForWarp - m_pBody->GetPosition();

		ResetStuckInStateForWarp();

		if (MagnitudeSq(diff) < 100)
		{
			
			RNavigationMesh* pNavMesh = ZGetGame()->GetWorld()->GetBsp()->GetNavigationMesh();
			if (pNavMesh) {
				float angle = (rand() % (314 * 2)) * 0.01f;
				rmatrix matRot = RGetRotZRad(angle);

				rvector dir(200, 0, 0);
				dir = dir * matRot;
				rvector newpos = m_pBody->GetPosition() + dir;

				RNavigationNode* pNavNode = pNavMesh->FindClosestNode(newpos);
				if (pNavNode) {
					rvector warpPos = pNavNode->CenterVertex();
					// CORRECCIÓN: Verificar que la posición de warp esté en el suelo antes de mover
					// Ajustar la altura Z al suelo si es necesario
					RBspObject* pBsp = ZGetGame()->GetWorld()->GetBsp();
					if (pBsp)
					{
						rvector testPos = warpPos;
						testPos.z += 100.0f;
						RBSPPICKINFO pickInfo;
						if (pBsp->Pick(testPos, rvector(0, 0, -1), &pickInfo))
						{
							warpPos.z = pickInfo.PickPos.z + m_pBody->GetCollHeight() * 0.5f;
						}
					}
					m_pBody->SetPosition(warpPos);
		
					return false;
				}
			}
		}
	}

	if (currTime - m_dwExPositionTime > 1000)
	{
		rvector diff = m_exPosition - m_pBody->GetPosition();

		ResetStuckInState();

		if (MagnitudeSq(diff) < 100)
		{
			// CORRECCIÓN: Solo intentar escape si está en el suelo
			if (!m_pBody->IsOnLand())
				return false;

			wayPointList.clear();

			rvector dir = m_pBody->GetDirection();
			rmatrix matRot = RGetRotZRad((rand() % 314 - 157) * 0.01f);
			Normalize(dir);

			dir *= m_pBody->GetCollRadius() * 0.8f;
			rvector escapePos = m_pBody->GetPosition() + dir;
			
			// CORRECCIÓN: Asegurar que el waypoint de escape esté en el suelo
			RNavigationMesh* pNavMesh = ZGetGame()->GetWorld()->GetBsp()->GetNavigationMesh();
			if (pNavMesh)
			{
				RNavigationNode* pNode = pNavMesh->FindClosestNode(escapePos);
				if (pNode)
				{
					escapePos = pNode->CenterVertex();
				}
			}
			
			wayPointList.push_back(escapePos);

			PushWayPointsToTask();

			return true;
		}
	}

	return false;
}

void ZBrain::ResetStuckInState()
{
	rvector pos;
	if (m_pBody) pos = m_pBody->GetPosition();
	else	pos.x = pos.y = pos.z = 0;

	DWORD currTime = timeGetTime();
	m_dwExPositionTime = currTime;
	m_exPosition = pos;
}

void ZBrain::ResetStuckInStateForWarp()
{
	rvector pos;
	if (m_pBody) pos = m_pBody->GetPosition();
	else	pos.x = pos.y = pos.z = 0;

	DWORD currTime = timeGetTime();
	m_dwExPositionTimeForWarp = currTime;
	m_exPositionForWarp = pos;
}

void ZBrain::PushWayPointsToTask()
{
	if (m_WayPointList.empty())
		return;

	m_pBody->m_TaskManager.Clear();
	int nTotal = (int)m_WayPointList.size();
	int cnt = 0;
	for (std::list<rvector>::iterator itor = m_WayPointList.begin(); itor != m_WayPointList.end(); ++itor)
	{
		bool bChained = !((nTotal - 1) == cnt);
		ZTask* pNew = ZTaskManager::CreateMoveToPos(m_pBody, (*itor), bChained);
		m_pBody->m_TaskManager.Push(pNew);
		++cnt;
	}
}

void ZBrain::AdjustWayPointWithBound(std::list<rvector>& wayPointList, RNavigationMesh* pNavMesh)
{
	if (wayPointList.empty()) return;

	const rvector& targetpos = *(wayPointList.begin());
	rvector center = m_pBody->GetPosition();
	rvector dir = targetpos - center;
	dir.z = 0;
	Normalize(dir);

	dir *= m_pBody->GetCollRadius() * 0.8f;

	rvector side1 = center + rvector(-dir.y, dir.x, 0);
	rvector side2 = center + rvector(dir.y, -dir.x, 0);

	if (pNavMesh->BuildNavigationPath(side1, targetpos) && pNavMesh->GetWaypointList().size() > 1)
		m_WayPointList.push_front(side2);
	else if (pNavMesh->BuildNavigationPath(side2, targetpos) && pNavMesh->GetWaypointList().size() > 1)
		m_WayPointList.push_front(side1);
}