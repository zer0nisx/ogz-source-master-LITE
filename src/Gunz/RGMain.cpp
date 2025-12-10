#include "stdafx.h"
#include "RGMain.h"
#include "ZRule.h"
#include "ZInput.h"
#include <cstdint>

static optional<RGMain> g_RGMain;

RGMain& GetRGMain() { return g_RGMain.value(); }
void CreateRGMain() { g_RGMain.emplace(); }
void DestroyRGMain() { g_RGMain.reset(); }
bool IsRGMainAlive() { return g_RGMain.has_value(); }

RGMain::RGMain() = default;
RGMain::~RGMain() = default;

std::pair<bool, uint32_t> RGMain::GetPlayerSwordColor(const MUID& UID)
{
	auto it = SwordColors.find(UID);

	if (it == SwordColors.end())
		return{ false, 0 };

	return{ true, it->second };
}

void RGMain::SetSwordColor(const MUID& UID, uint32_t Color)
{
	SwordColors[UID] = Color;

	auto Char = ZGetCharacterManager()->Find(UID);

	if (!Char)
		return;

	Char->m_pVMesh->SetCustomColor(Color & 0x80FFFFFF, Color & 0x0FFFFFFF);
}
