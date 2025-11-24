#include "StdAfx.h"
#include "ZModule_Resistance.h"

ZModule_Resistance::ZModule_Resistance()
{
}

ZModule_Resistance::~ZModule_Resistance(void)
{
}

void ZModule_Resistance::OnAdd(void)
{
	_ASSERT(MIsDerivedFromClass(ZObject, m_pContainer));
}

void ZModule_Resistance::InitStatus()
{
	// Verificar que el contenedor sea válido
	if (!m_pContainer)
		return;

	m_nFR = 0;
	m_nCR = 0;
	m_nPR = 0;
	m_nLR = 0;

	ZObject* pThisObj = MStaticCast(ZObject, m_pContainer);
	if (!pThisObj)
		return;

	ZCharacterItem* pItems = pThisObj->GetItems();
	for (int i = 0; i < MMCIP_END; i++)
	{
		ZItem* pItem = pItems->GetItem((MMatchCharItemParts)i);
		if (pItem) {
			MMatchItemDesc* pDesc = pItem->GetDesc();
			if (pDesc) {
				m_nFR += pDesc->m_nFR;
				m_nCR += pDesc->m_nCR;
				m_nLR += pDesc->m_nLR;
				m_nPR += pDesc->m_nPR;
			}
		}
	}

	m_nFR = max(min(m_nFR, 100), 0);
	m_nCR = max(min(m_nCR, 100), 0);
	m_nPR = max(min(m_nPR, 100), 0);
	m_nLR = max(min(m_nLR, 100), 0);
}