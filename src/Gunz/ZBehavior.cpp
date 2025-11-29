#include "stdafx.h"
#include "ZBehavior.h"
#include "ZBrain.h"
#include "ZBehavior_Idle.h"
#include "ZBehavior_Attack.h"
#include "ZBehavior_Patrol.h"

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
	ZState* pState = new ZBehavior_Idle(pBrain);
	pState->AddTransition(ZBEHAVIOR_INPUT_ATTACKED, ZBEHAVIOR_STATE_ATTACK);
	pState->AddTransition(ZBEHAVIOR_INPUT_TARGET_FOUND, ZBEHAVIOR_STATE_ATTACK);
	m_FSM.AddState(pState);

	// Estado ATTACK: Ataca al objetivo
	pState = new ZBehavior_Attack(pBrain);
	pState->AddTransition(ZBEHAVIOR_INPUT_TARGET_LOST, ZBEHAVIOR_STATE_IDLE);
	pState->AddTransition(ZBEHAVIOR_INPUT_TARGET_OUT_RANGE, ZBEHAVIOR_STATE_IDLE);
	pState->AddTransition(ZBEHAVIOR_INPUT_LOW_HEALTH, ZBEHAVIOR_STATE_RETREAT);
	m_FSM.AddState(pState);

	// Estado PATROL: Patrulla el área (si existe)
	pState = new ZBehavior_Patrol(pBrain);
	pState->AddTransition(ZBEHAVIOR_INPUT_TARGET_FOUND, ZBEHAVIOR_STATE_ATTACK);
	pState->AddTransition(ZBEHAVIOR_INPUT_ATTACKED, ZBEHAVIOR_STATE_ATTACK);
	m_FSM.AddState(pState);

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