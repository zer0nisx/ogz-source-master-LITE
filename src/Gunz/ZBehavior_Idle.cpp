#include "stdafx.h"
#include "ZBehavior_Idle.h"
#include "ZBrain.h"
#include "ZObject.h"

void ZBehavior_Idle::OnEnter()
{
	// Cuando entra en estado IDLE, detener cualquier acción agresiva
	if (m_pBrain && m_pBrain->GetBody())
	{
		// No hacer nada agresivo, solo esperar
	}
}

void ZBehavior_Idle::OnExit()
{
	// Limpiar cualquier estado temporal del IDLE
}

void ZBehavior_Idle::OnRun(float fDelta)
{
	// MEJORA: En estado IDLE, buscar objetivos cercanos
	if (!m_pBrain || !m_pBrain->GetBody()) return;

	ZObject* pTarget = m_pBrain->GetTarget();
	if (pTarget)
	{
		// Si encontramos un objetivo, cambiar a estado ATTACK
		// Esto se maneja automáticamente desde ZBrain::Think()
		// pero podemos agregar lógica adicional aquí si es necesario
	}
}

ZBehavior_Idle::ZBehavior_Idle(ZBrain* pBrain) : ZBehaviorState(pBrain, ZBEHAVIOR_STATE_IDLE)
{

}

ZBehavior_Idle::~ZBehavior_Idle()
{

}
