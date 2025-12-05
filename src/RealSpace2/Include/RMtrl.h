#pragma once

#include <list>
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <atomic>
#include <shared_mutex>  // Para std::shared_timed_mutex
#include "RBaseTexture.h"
#include "RTypes.h"
#include <d3d9.h>

_USING_NAMESPACE_REALSPACE2

class RMtrl
{
public:
	RMtrl();
	~RMtrl();
	RMtrl(const RMtrl& rhs) = delete;

	void CheckAniTexture();

	LPDIRECT3DTEXTURE9 GetTexture();

	void Restore(LPDIRECT3DDEVICE9 dev,char* path=NULL);

	void	SetTColor(u32 color);
	u32		GetTColor();

	RBaseTexture*  m_pTexture;

	RBaseTexture*  m_pToonTexture;

	D3DTEXTUREFILTERTYPE m_FilterType;
	D3DTEXTUREFILTERTYPE m_ToonFilterType;

	u32					m_AlphaRefValue;
	D3DTEXTUREOP		m_TextureBlendMode;
	D3DTEXTUREOP		m_ToonTextureBlendMode;

	color_r32 m_ambient;
	color_r32 m_diffuse;
	color_r32 m_specular;

	float	m_power;

	char	m_mtrl_name[255];

	char	m_name[255];
	char	m_opa_name[255];

	char	m_name_ani_tex[255];
	char	m_name_ani_tex_ext[20];

	int		m_id;
	int		m_u_id;
	int		m_mtrl_id;	  
	int		m_sub_mtrl_id;

	int		m_sub_mtrl_num;

	bool	m_bDiffuseMap;
	bool	m_bTwoSided;
	bool	m_bAlphaMap;
	bool	m_bAlphaTestMap;
	bool	m_bAdditive;

	int		m_nAlphaTestValue;

	bool	m_bUse;

	// Variables de textura animada (thread-safe para acceso concurrente)
	std::atomic<bool>	m_bAniTex;
	std::atomic<int>	m_nAniTexCnt;
	std::atomic<int>	m_nAniTexSpeed;
	std::atomic<int>	m_nAniTexGap;
	std::atomic<u64>	m_backup_time;  // CRÍTICO: leído/escrito desde múltiples threads

	bool	m_bObjectMtrl;// effect ,interface , object

	u32		m_dwTFactorColor;

	RBaseTexture** m_pAniTexture;
};

#define MAX_MTRL_NODE 100

class RMtrlMgr final
{
private:
	std::list<std::unique_ptr<RMtrl>> m_materials;
	bool m_bObjectMtrl{};// effect ,interface , object
	int m_id_last{};
	
	// Mutex para operaciones read-heavy (múltiples lectores, un escritor)
	mutable std::shared_timed_mutex m_mutex;

public:
	RMtrlMgr();
	RMtrlMgr(const RMtrlMgr&) = delete;
	~RMtrlMgr() = default;  // unique_ptr destruye automáticamente

	// Crear nuevo material
	int		Add(char* name, int u_id = -1);
	
	// Agregar material existente (toma ownership)
	int		Add(std::unique_ptr<RMtrl> tex);
	
	// Compatibilidad: acepta raw pointer (toma ownership)
	// DEPRECATED: Usar Add(std::unique_ptr<RMtrl>) en su lugar
	int		Add(RMtrl* tex);

	void	Del(int id);
	void	Del(RMtrl* tex);

	int		LoadList(char* name);
	int		SaveList(char* name);

	void	DelAll();
	void	Restore(LPDIRECT3DDEVICE9 dev, char* path = NULL);

	void	CheckUsed(bool b);
	void	ClearUsedCheck();
	void	ClearUsedMtrl();

	void	SetObjectTexture(bool bObject) { m_bObjectMtrl = bObject; }

	RMtrl*	Get_s(int mtrl_id, int sub_id) const;

	LPDIRECT3DTEXTURE9 Get(int id) const;
	LPDIRECT3DTEXTURE9 Get(int id, int sub_id) const;
	LPDIRECT3DTEXTURE9 GetUser(int id) const;
	LPDIRECT3DTEXTURE9 Get(char* name) const;

	RMtrl*  GetMtrl(char* name) const;
	RMtrl*  GetToolMtrl(char* name) const;

	int		GetNum() const;
	size_t	size() const { return m_materials.size(); }
};
