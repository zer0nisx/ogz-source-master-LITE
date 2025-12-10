#ifndef _ZBEHAVIOR_STUCK_H
#define _ZBEHAVIOR_STUCK_H

#include "ZBehavior.h"

// MEJORA: Estado STUCK para manejar NPCs atascados
class ZBehavior_Stuck : public ZBehaviorState
{
protected:
	virtual void OnEnter();
	virtual void OnExit();
	virtual void OnRun(float fDelta);

private:
	DWORD m_dwStuckStartTime;
	int m_nEscapeAttempts;

public:
	ZBehavior_Stuck(ZBrain* pBrain);
	virtual ~ZBehavior_Stuck();
};

#endif



