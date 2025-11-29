#include "stdafx.h"
#include "ZTask_MoveToPos.h"
#include "ZGame.h"
#include "ZObjectManager.h"
#include "ZActor.h"

ZTask_MoveToPos::ZTask_MoveToPos(ZActor* pParent, rvector& vTarPos, bool bChained)
	: ZTask(pParent), m_TargetPos(vTarPos), m_bRotated(false), m_bChained(bChained)
{
	m_TargetPos.z = 0.0f;
}

ZTask_MoveToPos::~ZTask_MoveToPos()
{
}

void ZTask_MoveToPos::OnStart()
{
}

ZTaskResult ZTask_MoveToPos::OnRun(float fDelta)
{
	// CORRECCIÓN: No ejecutar movimiento si el NPC está en el aire
	if (!m_pParent->IsOnLand())
	{
		// Esperar a que llegue al suelo antes de continuar
		return ZTR_RUNNING;
	}

	rvector p1 = m_TargetPos;
	rvector p2 = m_pParent->GetPosition();
	p1.z = p2.z = 0.0f;
	float diff = Magnitude(p1 - p2);

	// MEJORA: Aumentar umbral de distancia para evitar que se queden parados
	// Considerar el radio de colisión del NPC para un umbral más realista
	float fThreshold = max(20.0f, m_pParent->GetCollRadius() * 1.5f);
	if (diff <= fThreshold)
	{
		return ZTR_COMPLETED;
	}

	rvector dir = m_TargetPos - m_pParent->GetPosition();
	Normalize(dir);

	// MEJORA: Detección de colisiones con otros NPCs
	// Si hay un NPC bloqueando el camino, intentar esquivar
	if (ZGetObjectManager())
	{
		float myRadius = m_pParent->GetCollRadius();
		
		for (ZObjectManager::iterator itor = ZGetObjectManager()->begin(); 
			 itor != ZGetObjectManager()->end(); ++itor)
		{
			ZObject* pObject = itor->second;
			if (!pObject || pObject == m_pParent || pObject->IsDead())
				continue;

			// Solo verificar NPCs (ZActor)
			ZActor* pOtherActor = MDynamicCast(ZActor, pObject);
			if (!pOtherActor)
				continue;

			rvector otherPos = pOtherActor->GetPosition();
			rvector toOther = otherPos - m_pParent->GetPosition();
			toOther.z = 0.0f;
			float distToOther = Magnitude(toOther);
			
			// Si está muy cerca, calcular dirección de evasión
			float combinedRadius = myRadius + pOtherActor->GetCollRadius();
			if (distToOther < combinedRadius * 2.0f && distToOther > 0.0f)
			{
				// Calcular dirección perpendicular para esquivar
				rvector perpendicular = rvector(-toOther.y, toOther.x, 0.0f);
				Normalize(perpendicular);
				
				// Usar la dirección que esté más alineada con el objetivo
				float dot1 = DotProduct(perpendicular, dir);
				rvector evadeDir = (dot1 > 0) ? perpendicular : -perpendicular;
				
				// Mezclar dirección original con evasión
				dir = dir * 0.7f + evadeDir * 0.3f;
				Normalize(dir);
				break;
			}
		}
	}

	if (!m_bRotated)
	{
		rmatrix mat;
		rvector vBodyDir = m_pParent->GetDirection();
		float fAngle = GetAngleOfVectors(dir, vBodyDir);
		float fRotAngle = m_pParent->GetNPCInfo()->fRotateSpeed * (fDelta / 1.0f);

		if (fAngle > 0.0f) fRotAngle = -fRotAngle;
		if (fabs(fRotAngle) > fabs(fAngle))
		{
			fRotAngle = -fAngle;
			m_bRotated = true;
		}
		mat = RGetRotZ(ToDegree(fRotAngle));

		m_pParent->RotateTo(vBodyDir * mat);
	}
	else
	{
		m_pParent->RunTo(dir);
	}

	return ZTR_RUNNING;
}

void ZTask_MoveToPos::OnComplete()
{
	m_pParent->Stop(!m_bChained);
}

bool ZTask_MoveToPos::OnCancel()
{
	m_pParent->Stop(false);
	return true;
}