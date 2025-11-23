#include "stdafx.h"
#include "ZObject.h"
#include "ZMyCharacter.h"
#include "ZNetCharacter.h"
#include "RCollisionDetection.h"
#include "ZModule_HPAP.h"
#include "ZGame.h"

MImplementRTTI(ZObject, ZModuleContainer);

ZObject::ZObject() : m_Position(0, 0, 0), m_Direction(1, 0, 0), m_bInitialized(false), m_UID(MUID(0, 0)),
m_fSpawnTime(0.0f), m_fDeadTime(0.0f), m_pVMesh(NULL), m_bVisible(false),
m_bIsNPC(false)
{
	m_Collision.bCollideable = true;
	m_Collision.fRadius = 0.0f;
	m_Collision.fHeight = 0.0f;

	m_pModule_Movable = AddModule<ZModule_Movable>();
	assert(m_pModule_Movable != nullptr && "Failed to create ZModule_Movable in ZObject constructor");
	if (m_pModule_Movable) {
		m_pModule_Movable->Active = true;
	}
}

void ZObject::OnDraw()
{
}
void ZObject::OnUpdate(float fDelta)
{
}

void ZObject::Draw()
{
	OnDraw();
}

void ZObject::Update(float fDelta)
{
	OnUpdate(fDelta);
}

bool ZObject::Pick(int x, int y, RPickInfo* pInfo)
{
	if (m_pVMesh)
		return m_pVMesh->Pick(x, y, pInfo);

	return false;
}

bool ZObject::Pick(int x, int y, rvector* v, float* f)
{
	RPickInfo info;
	bool hr = Pick(x, y, &info);
	*v = info.vOut;
	*f = info.t;
	return hr;
}

bool ZObject::Pick(const rvector& pos, const rvector& dir, RPickInfo* pInfo)
{
	if (m_pVMesh)
		return m_pVMesh->Pick(pos, dir, pInfo);

	return false;
}

bool ZObject::GetHistory(rvector* pos, rvector* direction, float fTime, rvector* cameradir)
{
	if (!m_pVMesh)
		return false;

	if (pos)
		*pos = GetPosition();
	if (direction)
		*direction = m_Direction;
	if (cameradir)
		*cameradir = m_Direction;

	return true;
}

void ZObject::SetDirection(const rvector& dir)
{
	m_Direction = dir;
}

void ZObject::SetSpawnTime(float fTime)
{
	m_fSpawnTime = fTime;
}

void ZObject::SetDeadTime(float fTime)
{
	m_fDeadTime = fTime;
}

//////////////////////////////////////////////////////////////////////
bool IsPlayerObject(ZObject* pObject)
{
	return MDynamicCast(ZCharacter, pObject) != nullptr;
}

void ZObject::Tremble(float fValue, DWORD nMaxTime, DWORD nReturnMaxTime)
{
	if (m_pVMesh)
	{
		RFrameTime* ft = &m_pVMesh->m_FrameTime;
		if (ft && !ft->m_bActive)
			ft->Start(fValue, nMaxTime, nReturnMaxTime);// ���� , �ִ�ð� , ���ͽð�...
	}
}

void ZObject::OnDamaged(ZObject* pAttacker, rvector srcPos, ZDAMAGETYPE damageType, MMatchWeaponType weaponType, float fDamage, float fPiercingRatio, int nMeleeType)
{
	ZModule_HPAP* pModule = (ZModule_HPAP*)GetModule(ZMID_HPAP);
	if (!pModule) return;

	pModule->OnDamage(pAttacker ? pAttacker->GetUID() : MUID(0, 0), fDamage, fPiercingRatio);
}

void ZObject::OnDamagedSkill(ZObject* pAttacker, rvector srcPos, ZDAMAGETYPE damageType, MMatchWeaponType weaponType, float fDamage, float fPiercingRatio, int nMeleeType)
{
	ZModule_HPAP* pModule = (ZModule_HPAP*)GetModule(ZMID_HPAP);
	if (!pModule) return;

	pModule->OnDamage(pAttacker ? pAttacker->GetUID() : MUID(0, 0), fDamage, fPiercingRatio);
}

void ZObject::OnSimpleDamaged(ZObject* pAttacker, float fDamage, float fPiercingRatio)
{
	ZModule_HPAP* pModule = (ZModule_HPAP*)GetModule(ZMID_HPAP);
	if (!pModule) return;

	pModule->OnDamage(pAttacker ? pAttacker->GetUID() : MUID(0, 0), fDamage, fPiercingRatio);
}

// HP/AP�� ȸ���Ѵ�
void ZObject::OnHealing(ZObject* pOwner, int nHP, int nAP)
{
	// ����
	ZModule_HPAP* pModule = (ZModule_HPAP*)GetModule(ZMID_HPAP);
	if (!pModule) return;

	pModule->SetHP(min(pModule->GetHP() + nHP, pModule->GetMaxHP()));
	pModule->SetAP(min(pModule->GetAP() + nAP, pModule->GetMaxAP()));

	// TODO: ����Ʈ �߰�. �ӽ÷� �޾���
	ZGetEffectManager()->AddHealEffect(GetPosition(), this);
}

bool ZObject::ColTest(const rvector& p1, const rvector& p2, float radius, float fTime)
{
	rvector p, d;
	if (GetHistory(&p, &d, fTime))
	{
		rvector a1 = p + rvector(0, 0, (min(m_Collision.fHeight, m_Collision.fRadius) / 2.0f));
		rvector a2 = p + rvector(0, 0, m_Collision.fHeight - (min(m_Collision.fHeight, m_Collision.fRadius) / 2.0f));

		rvector ap, cp;
		float dist = GetDistanceBetweenLineSegment(p1, p2, a1, a2, &ap, &cp);

		if (dist < (radius + m_Collision.fRadius)) return true;
	}

	return false;
}