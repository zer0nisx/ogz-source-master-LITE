#ifndef _ZACTORANIMCONTROLLER_H
#define _ZACTORANIMCONTROLLER_H

#include "ZStateMachine.h"

//! ���ϸ��̼� ��Ʈ
enum ZA_ANIM_STATE {

	ZA_ANIM_NONE = 0,
	ZA_ANIM_IDLE,				// ����
	ZA_ANIM_WALK,				// �ȱ�
	ZA_ANIM_RUN,				// �ٱ�
	ZA_ANIM_ATTACK_MELEE,		// ���� ����
	ZA_ANIM_ATTACK_RANGE,		// ���Ÿ� ����
	ZA_ANIM_RANGE_DAMAGED,		// ���Ÿ� �ǰ�
	ZA_ANIM_MELEE_DAMAGED1,		// ���Ÿ� �ǰ�
	ZA_ANIM_MELEE_DAMAGED2,		// ���Ÿ� �ǰ�
	ZA_ANIM_LIGHTNING_DAMAGED,	// ����Ʈ�� ������
	ZC_ANIM_DAMAGED_DOWN,		// �ٿ�
	ZC_ANIM_STAND,				// �Ͼ��

	ZA_ANIM_BLAST,				// ���󰡱�
	ZA_ANIM_BLAST_FALL,			// ��������
	ZA_ANIM_BLAST_DROP,			// ���� �ε�����
	ZA_ANIM_DIE,

	ZA_ANIM_BLAST_DAGGER,		// �ܰ���⿡ ���󰡴�
	ZA_ANIM_BLAST_DAGGER_DROP,	// �ܰ���⿡ �Ѿ�����

	ZA_ANIM_SPECIAL1,			// ��ų�̳� ����
	ZA_ANIM_SPECIAL2,			// ��ų�̳� ����
	ZA_ANIM_SPECIAL3,
	ZA_ANIM_SPECIAL4,

	ZA_ANIM_END
};

enum ZA_ANIM_INPUT {
	// actor input
	ZA_INPUT_NONE = 0,
	ZA_INPUT_WALK,				// a
	ZA_INPUT_RUN,				// b
	ZA_INPUT_ROTATE,			// ������ �ٲٷ� �Ѵ�.
	ZA_INPUT_WALK_DONE,
	ZA_INPUT_ATTACK_MELEE,		// c
	ZA_INPUT_ATTACK_RANGE,		// d
	ZA_INPUT_RISE,				// j

	// event
	ZA_EVENT_DETECT_ENEMY,		// b
	ZA_EVENT_RANGE_DAMAGED,		// d
	ZA_EVENT_MELEE_DAMAGED1,	// d
	ZA_EVENT_MELEE_DAMAGED2,	// d
	ZA_EVENT_LIGHTNING_DAMAGED,	// d
	ZA_EVENT_BLAST,				// e
	ZA_EVENT_BLAST_DAGGER,		// f
	ZA_EVENT_FALL,				// g
	ZA_EVENT_REACH_GROUND,		// i
	ZA_EVENT_REACH_GROUND_DAGGER,// i
	ZA_EVENT_REACH_PEAK,		
	ZA_EVENT_DEATH,				// l
	ZA_EVENT_SPECIAL1,
	ZA_EVENT_SPECIAL2,
	ZA_EVENT_SPECIAL3,
	ZA_EVENT_SPECIAL4,
	
	// SUMMER-SOURCE: Eventos para sistema de neglect y standup
	ZA_EVENT_NEGLECT1,			// NPC inactivo tipo 1
	ZA_EVENT_NEGLECT2,			// NPC inactivo tipo 2
	ZA_EVENT_STANDUP,			// Levantarse después de caer

	// anim event
	ZA_ANIM_DONE,				// k

	ZA_INPUT_END
};

class ZActor;

class ZActorAnimation
{
protected:
	//! ���ϸ��̼� ����
	static struct ANIMATION_INFO {		// ZCharacter�� ZANIMATIONINFO���� ������
		int		nID;
		char*	szName;
		bool	bEnableCancel;
		bool	bLoop;
		bool	bMove;
	} m_AnimationTable[ZA_ANIM_END];

	ZActor*					m_pBody;
	ZStateMachine			m_AniFSM;				///< �ִϸ��̼� ���±��
	ZA_ANIM_STATE			m_nCurrState;			///< ���� �ִϸ��̼� ����

	void InitAnimationStates();
public:
	ZActorAnimation();
	virtual ~ZActorAnimation();
	void Init(ZActor* pBody);
	void Set(ZA_ANIM_STATE nAnim, bool bReset=false);
	bool Input(ZA_ANIM_INPUT nInput);
	void ForceAniState(int nAnimState);				///< �ִϸ��̼� ���¸� ������ �ٲ۴�. 

	ZA_ANIM_STATE GetCurrState() { return m_nCurrState; }
	static bool IsAttackAnimation(ZA_ANIM_STATE nAnimState);
	static bool IsSkippableDamagedAnimation(ZA_ANIM_STATE nAnimState);
};












#endif