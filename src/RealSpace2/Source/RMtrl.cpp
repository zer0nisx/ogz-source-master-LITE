#include "stdafx.h"
#include "RMtrl.h"
#include <cstring>
#include <cstdio>
#include "RMtrl.h"
#include "assert.h"
#include <tchar.h>
#include <shared_mutex>  // Para std::shared_lock
#include "RBaseTexture.h"
#include "MDebug.h"

_USING_NAMESPACE_REALSPACE2

RMtrl::RMtrl()
{
	m_name[0] = 0;
	m_opa_name[0] = 0;

	m_id = -1;
	m_u_id = -1;
	m_mtrl_id = -1;
	m_sub_mtrl_id = -1;

	m_bDiffuseMap = false;
	m_bAlphaMap = false;
	m_bTwoSided = false;
	m_bAdditive = false;
	m_bAlphaTestMap = false;

	m_nAlphaTestValue = -1;

	m_ambient = m_diffuse = m_specular = color_r32{ 0xffffffff };

	m_power = 1.0f;

	m_bUse = true;
	m_pTexture = NULL;
	m_pAniTexture = NULL;

	m_pToonTexture = NULL;
	m_FilterType = D3DTEXF_LINEAR;
	m_ToonFilterType = D3DTEXF_POINT;
	m_AlphaRefValue = 0x05;
	m_TextureBlendMode = D3DTOP_BLENDTEXTUREALPHA;
	m_ToonTextureBlendMode = D3DTOP_MODULATE2X;

	m_bAniTex = false;
	m_nAniTexCnt = 0;
	m_nAniTexSpeed = 0;
	m_nAniTexGap = 0;
	m_backup_time = 0;  // atomic se inicializa así

	m_name_ani_tex[0] = 0;
	m_name_ani_tex_ext[0] = 0;
	m_bObjectMtrl = false;

	m_dwTFactorColor = D3DCOLOR_COLORVALUE(0.0f, 1.0f, 0.0f, 0.0f);
}

RMtrl::~RMtrl()
{
	bool bAniTex = m_bAniTex.load(std::memory_order_acquire);
	if (bAniTex) {
		if (m_pAniTexture) {
			int nAniTexCnt = m_nAniTexCnt.load(std::memory_order_acquire);
			for (int i = 0; i < nAniTexCnt; i++) {
				RDestroyBaseTexture(m_pAniTexture[i]);
			}
			delete[] m_pAniTexture;
		}
	}
	else {
		RDestroyBaseTexture(m_pTexture);
		m_pTexture = NULL;
	}
}

LPDIRECT3DTEXTURE9 RMtrl::GetTexture() {
	// Cargar flag de animación atómicamente
	bool bAniTex = m_bAniTex.load(std::memory_order_acquire);
	
	if (bAniTex) {
		auto this_time = GetGlobalTimeMS();

		// Leer m_backup_time atómicamente
		u64 backup_time = m_backup_time.load(std::memory_order_acquire);
		auto gap = (this_time - backup_time);

		// Cargar parámetros de animación atómicamente
		int nAniTexSpeed = m_nAniTexSpeed.load(std::memory_order_acquire);
		int nAniTexGap = m_nAniTexGap.load(std::memory_order_acquire);
		int nAniTexCnt = m_nAniTexCnt.load(std::memory_order_acquire);

		if (gap > (u64)nAniTexSpeed) {
			gap %= nAniTexSpeed;
			// Actualizar m_backup_time atómicamente
			// Usar store con release semantics
			m_backup_time.store(this_time, std::memory_order_release);
		}

		int pos = int(gap / nAniTexGap);

		if ((pos < 0) || (pos > nAniTexCnt - 1))
			pos = 0;

		if (m_pAniTexture && m_pAniTexture[pos]) {
			return m_pAniTexture[pos]->GetTexture();
		}

		return NULL;
	}
	else {
		if (!m_pTexture) return NULL;
		return m_pTexture->GetTexture();
	}
}

void RMtrl::SetTColor(u32 color)
{
	m_dwTFactorColor = color;
}

u32 RMtrl::GetTColor()
{
	return m_dwTFactorColor;
}

void RMtrl::CheckAniTexture()
{
	if (m_name[0]) {
		char drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
		_splitpath_s(m_name, drive, dir, fname, ext);

		if (_strnicmp(fname, "txa", 3) == 0) {
			char texname[256];

			int imax = 0;
			int ispeed = 0;

			// ex> txa_10_100_test01.tga
			// test02.tga

			auto ret = sscanf_s(fname, "txa %d %d %s", &imax, &ispeed, texname, sizeof(texname));
			if (ret != 3)
				return;

			m_pAniTexture = new RBaseTexture * [imax];

			for (int i = 0; i < imax; i++) {
				m_pAniTexture[i] = NULL;
			}

			int n = (int)strlen(texname);

			if (dir[0]) {
				strcpy_safe(m_name_ani_tex, dir);
				strncat_safe(m_name_ani_tex, texname, n - 2);
				int pos = strlen(dir) + n - 2;
				m_name_ani_tex[pos] = NULL;
			}
			else {
				strncpy_safe(m_name_ani_tex, texname, n - 2);
				m_name_ani_tex[n - 2] = NULL;
			}

			strcpy_safe(m_name_ani_tex_ext, ext);

			// Escribir parámetros de animación atómicamente con release semantics
			// Esto garantiza que todos los writes anteriores sean visibles antes de que
			// otros threads lean estos valores
			m_nAniTexSpeed.store(ispeed, std::memory_order_release);
			m_nAniTexCnt.store(imax, std::memory_order_release);
			m_nAniTexGap.store(ispeed / imax, std::memory_order_release);
			
			// m_bAniTex debe ser el último (release semantics garantiza visibilidad)
			m_bAniTex.store(true, std::memory_order_release);
		}
	}
}

void RMtrl::Restore(LPDIRECT3DDEVICE9 dev, char* path)
{
	if (m_name[0] == 0) return;

	char name[256];
	char name2[256];

	auto level = RTextureType::Etc;

	if (m_bObjectMtrl) {
		level = RTextureType::Object;
	}

	bool map_path = false;

	char drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
	_splitpath_s(m_name, drive, dir, fname, ext);

	if (dir[0])
		map_path = true;

	if (!map_path && path && path[0]) {
		bool bAniTex = m_bAniTex.load(std::memory_order_acquire);
		if (bAniTex) {
			strcpy_safe(name, path);
			strcat_safe(name, m_name);

			m_pAniTexture[0] = RCreateBaseTextureMg(name, level);

			if (m_pAniTexture[0] == NULL) {
				m_pAniTexture[0] = RCreateBaseTextureMg(m_name, level);
			}

			strcpy_safe(name2, path);
			strcat_safe(name2, m_name_ani_tex);

			int nAniTexCnt = m_nAniTexCnt.load(std::memory_order_acquire);
			for (int i = 1; i < nAniTexCnt; i++) {
				sprintf_safe(name, "%s%02d%s", name2, i, m_name_ani_tex_ext);
				m_pAniTexture[i] = RCreateBaseTextureMg(name, level);

				if (m_pAniTexture[i] == NULL) {
					sprintf_safe(name, "%s%2d.%s", m_name_ani_tex, i, m_name_ani_tex_ext);
					m_pAniTexture[i] = RCreateBaseTextureMg(name, level);
					if (m_pAniTexture[i] == NULL)
						int a = 0;
				}
			}
		}
		else {
			strcpy_safe(name, path);
			strcat_safe(name, m_name);

			m_pTexture = NULL;
			m_pTexture = RCreateBaseTextureMg(name, level);

			if (!m_pTexture)
				m_pTexture = RCreateBaseTextureMg(m_name, level);
		}
	}
	else {
		bool bAniTex = m_bAniTex.load(std::memory_order_acquire);
		if (bAniTex) {
			m_pAniTexture[0] = RCreateBaseTextureMg(m_name, level);

			int nAniTexCnt = m_nAniTexCnt.load(std::memory_order_acquire);
			for (int i = 1; i < nAniTexCnt; i++) {
				sprintf_safe(name, "%s%02d%s", m_name_ani_tex, i, m_name_ani_tex_ext);
				m_pAniTexture[i] = RCreateBaseTextureMg(name, level);
			}
		}
		else {
			m_pTexture = RCreateBaseTextureMg(m_name, level);
		}
	}
}

RMtrlMgr::RMtrlMgr()
{
	// unique_ptr maneja la destrucción automáticamente
}

int RMtrlMgr::Add(char* name, int u_id)
{
	// Write lock: solo un thread puede escribir
	std::unique_lock<std::shared_timed_mutex> lock(m_mutex);
	
	auto node = std::make_unique<RMtrl>();

	node->m_id = m_id_last;
	node->m_u_id = u_id;
	node->m_bObjectMtrl = m_bObjectMtrl;

	strcpy_safe(node->m_name, name);

	sprintf_safe(node->m_mtrl_name, "%s%d", name, m_id_last);

	m_materials.push_back(std::move(node));
	m_id_last++;
	return m_id_last - 1;
}

int RMtrlMgr::Add(std::unique_ptr<RMtrl> tex)
{
	// Write lock: solo un thread puede escribir
	std::unique_lock<std::shared_timed_mutex> lock(m_mutex);
	
	tex->m_id = m_id_last;
	tex->m_bObjectMtrl = m_bObjectMtrl;

	sprintf_safe(tex->m_mtrl_name, "%s%d", tex->m_name, tex->m_id);

	m_materials.push_back(std::move(tex));
	m_id_last++;
	return m_id_last - 1;
}

int RMtrlMgr::Add(RMtrl* tex)
{
	// Compatibilidad: tomar ownership del raw pointer
	return Add(std::unique_ptr<RMtrl>(tex));
}

int	RMtrlMgr::LoadList(char* fname)
{
	return 1;
}

int	RMtrlMgr::SaveList(char* name)
{
	return 1;
}

void RMtrlMgr::Del(RMtrl* tex)
{
	// Write lock: solo un thread puede escribir
	std::unique_lock<std::shared_timed_mutex> lock(m_mutex);
	
	m_materials.remove_if([tex](const std::unique_ptr<RMtrl>& mtrl) {
		return mtrl.get() == tex;
		});
}

void RMtrlMgr::Del(int id)
{
	// Write lock: solo un thread puede escribir
	std::unique_lock<std::shared_timed_mutex> lock(m_mutex);
	
	m_materials.remove_if([id](const std::unique_ptr<RMtrl>& mtrl) {
		return mtrl->m_id == id;
		});
}

void RMtrlMgr::DelAll()
{
	// Write lock: solo un thread puede escribir
	std::unique_lock<std::shared_timed_mutex> lock(m_mutex);
	
	m_materials.clear();
	m_id_last = 0;
}

void RMtrlMgr::Restore(LPDIRECT3DDEVICE9 dev, char* path)
{
	// Read lock: múltiples threads pueden leer simultáneamente
	std::shared_lock<std::shared_timed_mutex> lock(m_mutex);
	
	for (auto& mtrl : m_materials) {
		if (mtrl) {
			mtrl->Restore(dev, path);
		}
	}
}

void RMtrlMgr::CheckUsed(bool b)
{
	// Write lock: modificamos m_bUse de los materiales
	std::unique_lock<std::shared_timed_mutex> lock(m_mutex);
	
	for (auto& mtrl : m_materials) {
		if (mtrl) {
			mtrl->m_bUse = b;
		}
	}
}

void RMtrlMgr::ClearUsedCheck()
{
	// Write lock: modificamos m_bUse de los materiales
	std::unique_lock<std::shared_timed_mutex> lock(m_mutex);
	
	for (auto& mtrl : m_materials) {
		if (mtrl) {
			mtrl->m_bUse = false;
		}
	}
}

void RMtrlMgr::ClearUsedMtrl()
{
	// Write lock: modificamos la lista (remove_if)
	std::unique_lock<std::shared_timed_mutex> lock(m_mutex);
	
	m_materials.remove_if([](const std::unique_ptr<RMtrl>& mtrl) {
		return !mtrl->m_bUse;
		});
}

RMtrl* RMtrlMgr::Get_s(int mtrl_id, int sub_id) const
{
	// Read lock: múltiples threads pueden leer simultáneamente
	std::shared_lock<std::shared_timed_mutex> lock(m_mutex);
	
	for (auto& mtrl : m_materials) {
		if (mtrl && mtrl->m_mtrl_id == mtrl_id && mtrl->m_sub_mtrl_id == sub_id) {
			return mtrl.get();
		}
	}
	return nullptr;
}

LPDIRECT3DTEXTURE9 RMtrlMgr::Get(int id) const
{
	// Read lock: múltiples threads pueden leer simultáneamente
	std::shared_lock<std::shared_timed_mutex> lock(m_mutex);
	
	for (auto& mtrl : m_materials) {
		if (mtrl && mtrl->m_id == id) {
			return mtrl->GetTexture();
		}
	}
	return nullptr;
}

LPDIRECT3DTEXTURE9 RMtrlMgr::Get(int id, int sub_id) const
{
	// Read lock: múltiples threads pueden leer simultáneamente
	std::shared_lock<std::shared_timed_mutex> lock(m_mutex);
	
	for (auto& mtrl : m_materials) {
		if (mtrl && mtrl->m_mtrl_id == id && mtrl->m_sub_mtrl_id == sub_id) {
			return mtrl->GetTexture();
		}
	}
	return nullptr;
}

LPDIRECT3DTEXTURE9 RMtrlMgr::GetUser(int u_id) const
{
	// Read lock: múltiples threads pueden leer simultáneamente
	std::shared_lock<std::shared_timed_mutex> lock(m_mutex);
	
	for (auto& mtrl : m_materials) {
		if (mtrl && mtrl->m_u_id == u_id) {
			return mtrl->GetTexture();
		}
	}
	return nullptr;
}

LPDIRECT3DTEXTURE9 RMtrlMgr::Get(char* name) const
{
	// Read lock: múltiples threads pueden leer simultáneamente
	std::shared_lock<std::shared_timed_mutex> lock(m_mutex);
	
	for (auto& mtrl : m_materials) {
		if (mtrl && !strcmp(mtrl->m_name, name)) {
			return mtrl->GetTexture();
		}
	}
	return nullptr;
}

RMtrl* RMtrlMgr::GetMtrl(char* name) const
{
	// Read lock: múltiples threads pueden leer simultáneamente
	std::shared_lock<std::shared_timed_mutex> lock(m_mutex);
	
	for (auto& mtrl : m_materials) {
		if (mtrl && !strcmp(mtrl->m_name, name)) {
			return mtrl.get();
		}
	}
	return nullptr;
}

RMtrl* RMtrlMgr::GetToolMtrl(char* name) const
{
	// Read lock: múltiples threads pueden leer simultáneamente
	std::shared_lock<std::shared_timed_mutex> lock(m_mutex);
	
	for (auto& mtrl : m_materials) {
		if (mtrl && !strcmp(mtrl->m_mtrl_name, name)) {
			return mtrl.get();
		}
	}
	return nullptr;
}

int RMtrlMgr::GetNum() const
{
	// Read lock: múltiples threads pueden leer simultáneamente
	std::shared_lock<std::shared_timed_mutex> lock(m_mutex);
	
	return static_cast<int>(m_materials.size());
}