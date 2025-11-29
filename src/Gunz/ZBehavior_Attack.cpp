#include "stdafx.h"
#include "ZBehavior_Attack.h"
#include "ZBrain.h"
#include "ZObject.h"

void ZBehavior_Attack::OnEnter()
{
	// Cuando entra en estado ATTACK, asegurar que tenemos un objetivo válido
	if (m_pBrain && m_pBrain->GetBody())
	{
		ZObject* pTarget = m_pBrain->GetTarget();
		if (!pTarget)
		{
			// Si no hay objetivo, volver a IDLE
			// Esto se maneja desde ZBrain::Think()
		}
	}
}

void ZBehavior_Attack::OnExit()
{
	// Cuando sale del estado ATTACK, limpiar cualquier estado de ataque
	if (m_pBrain && m_pBrain->GetBody())
	{
		// Opcional: detener animaciones de ataque, etc.
	}
}

void ZBehavior_Attack::OnRun(float fDelta)
{
	// MEJORA: En estado ATTACK, verificar continuamente que el objetivo sigue siendo válido
	if (!m_pBrain || !m_pBrain->GetBody()) return;

	ZObject* pTarget = m_pBrain->GetTarget();
	if (!pTarget || pTarget->IsDead())
	{
		// Objetivo perdido o muerto - esto se maneja desde ZBrain::Think()
		// pero podemos agregar lógica adicional aquí
		return;
	}

	// Verificar si el objetivo está fuera de rango
	// Esta verificación se hace en ZBrain, pero podemos agregar lógica específica del estado aquí
}

ZBehavior_Attack::ZBehavior_Attack(ZBrain* pBrain) : ZBehaviorState(pBrain, ZBEHAVIOR_STATE_ATTACK)
{

}

ZBehavior_Attack::~ZBehavior_Attack()
{

}
