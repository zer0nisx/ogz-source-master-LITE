#include "stdafx.h"
#include "ZBehavior.h"
#include "ZBrain.h"
#include "ZBehavior_Idle.h"
#include "ZBehavior_Attack.h"
#include "ZBehavior_Patrol.h"
#include "ZBehavior_Stuck.h"

ZBehavior::ZBehavior() : m_pCurrState(NULL), m_pBrain(NULL), m_nOffecseType(ZOFFENSETYPE_MELEE), m_bFriendly(false)
{
}

ZBehavior::~ZBehavior()
{
}

void ZBehavior::Init(ZBrain* pBrain)
{
	m_pBrain = pBrain;

	// Estado IDLE: Estado inicial, busca objetivos
	ZState* pStateIdle = new ZBehavior_Idle(pBrain);
	pStateIdle->AddTransition(ZBEHAVIOR_INPUT_ATTACKED, ZBEHAVIOR_STATE_ATTACK);
	pStateIdle->AddTransition(ZBEHAVIOR_INPUT_TARGET_FOUND, ZBEHAVIOR_STATE_ATTACK);
	pStateIdle->AddTransition(ZBEHAVIOR_INPUT_STUCK, ZBEHAVIOR_STATE_STUCK);  // MEJORA: Puede quedar stuck
	m_FSM.AddState(pStateIdle);

	// Estado ATTACK: Ataca al objetivo
	ZState* pStateAttack = new ZBehavior_Attack(pBrain);
	pStateAttack->AddTransition(ZBEHAVIOR_INPUT_TARGET_LOST, ZBEHAVIOR_STATE_IDLE);
	pStateAttack->AddTransition(ZBEHAVIOR_INPUT_TARGET_OUT_RANGE, ZBEHAVIOR_STATE_IDLE);
	pStateAttack->AddTransition(ZBEHAVIOR_INPUT_LOW_HEALTH, ZBEHAVIOR_STATE_RETREAT);
	pStateAttack->AddTransition(ZBEHAVIOR_INPUT_STUCK, ZBEHAVIOR_STATE_STUCK);  // MEJORA: Puede quedar stuck
	m_FSM.AddState(pStateAttack);

	// Estado PATROL: Patrulla el área (si existe)
	auto* pState = new ZBehavior_Patrol(pBrain);
	pState->AddTransition(ZBEHAVIOR_INPUT_TARGET_FOUND, ZBEHAVIOR_STATE_ATTACK);
	pState->AddTransition(ZBEHAVIOR_INPUT_ATTACKED, ZBEHAVIOR_STATE_ATTACK);
	m_FSM.AddState(pState);

	// MEJORA: Estado STUCK cuando está atascado
	ZState* pStateStuck = new ZBehavior_Stuck(pBrain);
	pStateStuck->AddTransition(ZBEHAVIOR_INPUT_UNSTUCK, ZBEHAVIOR_STATE_IDLE);
	pStateStuck->AddTransition(ZBEHAVIOR_INPUT_TARGET_FOUND, ZBEHAVIOR_STATE_ATTACK);
	pStateStuck->AddTransition(ZBEHAVIOR_INPUT_ATTACKED, ZBEHAVIOR_STATE_ATTACK);
	m_FSM.AddState(pStateStuck);

	m_nOffecseType = (ZOFFENSETYPE)pBrain->GetBody()->GetNPCInfo()->nOffenseType;
	m_bFriendly = pBrain->GetBody()->GetNPCInfo()->bFriendly;

	ForceState(ZBEHAVIOR_STATE_IDLE);
}

void ZBehavior::Run(float fDelta)
{
	if (m_pCurrState) m_pCurrState->Run(fDelta);
}

bool ZBehavior::Input(ZBEHAVIOR_INPUT nInput)
{
	int nextState = m_FSM.StateTransition(nInput);
	if (nextState == ZStateMachine::INVALID_STATE) return false;

	ZBEHAVIOR_STATE nNextBehaviorState = ZBEHAVIOR_STATE(nextState);
	ChangeBehavior(nNextBehaviorState);

	return true;
}

void ZBehavior::ForceState(ZBEHAVIOR_STATE nState)
{
	ChangeBehavior(nState);
}

void ZBehavior::ChangeBehavior(ZBEHAVIOR_STATE nState)
{
	if (m_pCurrState)
	{
		((ZBehaviorState*)m_pCurrState)->Exit();
	}

	m_FSM.SetState(nState);

	m_pCurrState = (ZBehaviorState*)m_FSM.GetCurrState();
	m_pCurrState->Enter();
}