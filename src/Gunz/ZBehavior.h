#ifndef _ZBEHAVIOR_H
#define _ZBEHAVIOR_H


#include "ZStateMachine.h"

/// AI ϴ
enum ZBEHAVIOR_STATE
{
	ZBEHAVIOR_STATE_IDLE			=0,				///< ִ´.
	ZBEHAVIOR_STATE_PATROL,							///<
	ZBEHAVIOR_STATE_ATTACK,							///<
	ZBEHAVIOR_STATE_RETREAT,						///<
	ZBEHAVIOR_STATE_SCRIPT,							///<ũƮ
	ZBEHAVIOR_STATE_END
};

enum ZBEHAVIOR_INPUT 
{
	ZBEHAVIOR_INPUT_NONE = 0,
	ZBEHAVIOR_INPUT_ATTACKED,						///< Recibe daño
	ZBEHAVIOR_INPUT_TARGET_FOUND,					///< Encuentra objetivo
	ZBEHAVIOR_INPUT_TARGET_LOST,					///< Pierde objetivo
	ZBEHAVIOR_INPUT_TARGET_IN_RANGE,				///< Objetivo en rango de ataque
	ZBEHAVIOR_INPUT_TARGET_OUT_RANGE,				///< Objetivo fuera de rango
	ZBEHAVIOR_INPUT_LOW_HEALTH,						///< Salud baja (retreat)
	ZBEHAVIOR_INPUT_PATH_BLOCKED,					///< Camino bloqueado

	ZBEHAVIOR_INPUT_END
};

// SUMMER-SOURCE: Tipo de ofensiva del NPC
enum ZOFFENSETYPE
{
	ZOFFENSETYPE_MELEE		= 1,					// 근접 공격타입
	ZOFFENSETYPE_RANGE		= 2,					// 원거리 공격타입
};

class ZBrain;

/// Behavior State ߻ Ŭ
class ZBehaviorState : public ZState
{
protected:
	ZBrain*		m_pBrain;

	virtual void OnEnter() {}
	virtual void OnExit() {}
	virtual void OnRun(float fDelta) {}
public:
	ZBehaviorState(ZBrain* pBrain, int nStateID) : ZState(nStateID), m_pBrain(pBrain) { }
	virtual ~ZBehaviorState() { }
	void Run(float fDelta)	{ OnRun(fDelta); }
	void Enter()			{ OnEnter(); }
	void Exit()				{ OnExit(); }
};



class ZBehavior
{
private:
	ZStateMachine		m_FSM;
	ZBehaviorState*		m_pCurrState;
	ZBrain*				m_pBrain;
	// SUMMER-SOURCE: Tipo de ofensiva y comportamiento friendly
	ZOFFENSETYPE		m_nOffecseType;
	bool				m_bFriendly;
	void ChangeBehavior(ZBEHAVIOR_STATE nState);
public:
	ZBehavior();
	virtual ~ZBehavior();
	void Init(ZBrain* pBrain);
	void Run(float fDelta);
	bool Input(ZBEHAVIOR_INPUT nInput);
	void ForceState(ZBEHAVIOR_STATE nState);
	int GetCurrStateID() const { return m_FSM.GetCurrStateID(); }  // Para verificar estado actual
	// SUMMER-SOURCE: Métodos para obtener y establecer tipo de ofensiva y comportamiento friendly
	ZOFFENSETYPE GetOffenseType()						{ return m_nOffecseType;	}
	void SetOffenseType( ZOFFENSETYPE nType)			{ m_nOffecseType = nType;	}
	bool IsFriendly()									{ return m_bFriendly;		}
	void SetFriendly( bool bFriendly)					{ m_bFriendly = bFriendly;	}
};




#endif
