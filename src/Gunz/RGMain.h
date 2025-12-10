#pragma once

// Sort this stuff sometime maybe '__'

#include <mutex>
#include <queue>
#include <memory>
#include "Config.h"
#include "VoiceChat.h"
#include "Tasks.h"
#include "Hitboxes.h"
#include "ZReplay.h"
#include "NewChat.h"
#include "optional.h"
#include <DxErr.h>

inline bool DXErr(HRESULT hr, const char* CallingFunction, const char* DXFunction)
{
	if (SUCCEEDED(hr))
		return false;

	MLog("In %s, %s failed -- error code: %s, description: %s\n",
		CallingFunction, DXFunction, DXGetErrorString(hr), DXGetErrorDescription(hr));

	return true;
}

// Returns true if DirectX expression failed, and false otherwise. Additionally, if it failed,
// it logs the error.
#define DXERR(expr) DXErr(expr, __func__, #expr)

class ZChatCmdManager;
class MEvent;
class ZIDLResource;

HRESULT GenerateTexture(IDirect3DDevice9 *pD3Ddev, IDirect3DTexture9 **ppD3Dtex, DWORD colour32);
void LoadRGCommands(ZChatCmdManager &CmdManager);
std::pair<bool, std::vector<unsigned char>> ReadMZFile(const char *szPath);
std::pair<bool, std::vector<unsigned char>> ReadZFile(const char *szPath);
void Invoke(std::function<void()> fn);
enum class PlayerFoundStatus
{
	FoundOne,
	NotFound,
	TooManyFound,
};
std::pair<PlayerFoundStatus, ZCharacter*> FindSinglePlayer(const char* NameSubstring);

struct ReplayInfo
{
	ReplayVersion Version;
	REPLAY_STAGE_SETTING_NODE StageSetting;
	std::string VersionString;
	time_t Timestamp = 0;
	bool Dead = true;

	struct PlayerInfo
	{
		char Name[MATCHOBJECT_NAME_LENGTH];
		int Kills = 0;
		int Deaths = 0;
	};

	std::unordered_map<MUID, PlayerInfo> PlayerInfos;
};

class RGMain
{
public:
	RGMain();
	RGMain(const RGMain&) = delete;
	~RGMain();


	
	void SetSwordColor(const MUID& UID, uint32_t Hue);



	std::pair<bool, uint32_t> GetPlayerSwordColor(const MUID& UID);




private:
	friend void LoadRGCommands(ZChatCmdManager& CmdManager);



	std::unordered_map<MUID, uint32_t> SwordColors;


};

 RGMain& GetRGMain();
 void CreateRGMain();
 void DestroyRGMain();
 bool IsRGMainAlive();
 inline Chat& GetNewChat() { return GetRGMain().GetChat(); }

inline ZMyCharacter* MyChar()
{
	return ZGetGame()->m_pMyCharacter;
}

