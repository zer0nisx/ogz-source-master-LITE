#include "stdafx.h"
#include "ZBehavior_Stuck.h"
#include "ZBrain.h"
#include "ZObject.h"

void ZBehavior_Stuck::OnEnter()
{
	// Cuando entra en estado STUCK, iniciar escape
	if (m_pBrain && m_pBrain->GetBody())
	{
		m_dwStuckStartTime = timeGetTime();
		m_nEscapeAttempts = 0;

		// El escape se maneja automáticamente desde OnBody_CollisionWall()
		// que llama a EscapeFromCorner() cuando detecta colisiones múltiples
	}
}

void ZBehavior_Stuck::OnExit()
{
	// Limpiar estado cuando sale de STUCK
	m_nEscapeAttempts = 0;
}

void ZBehavior_Stuck::OnRun(float fDelta)
{
	if (!m_pBrain || !m_pBrain->GetBody()) return;

	// Verificar si sigue stuck o si logró escapar
	ZObject* pTarget = m_pBrain->GetTarget();

	// Si encontramos un objetivo, intentar volver a la acción normal
	if (pTarget)
	{
		// Verificar si ya no está stuck (se movió significativamente)
		// Esto se maneja desde ZBrain::EscapeFromCorner()
	}

	// Si lleva mucho tiempo stuck (>5 segundos), forzar warp
	DWORD currTime = timeGetTime();
	if (currTime - m_dwStuckStartTime > 5000)
	{
		// Forzar warp después de 5 segundos
		// Esto se maneja desde ZBrain::EscapeFromStuckIn()
	}
}

ZBehavior_Stuck::ZBehavior_Stuck(ZBrain* pBrain) : ZBehaviorState(pBrain, ZBEHAVIOR_STATE_STUCK),
m_dwStuckStartTime(0), m_nEscapeAttempts(0)
{
}

ZBehavior_Stuck::~ZBehavior_Stuck()
{
}