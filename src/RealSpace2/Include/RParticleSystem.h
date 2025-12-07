#pragma once

#include "RTypes.h"
#include "RBaseTexture.h"
#include <list>
#include <memory>  // Para std::unique_ptr, std::make_unique

// Forward declarations
namespace RealSpace2 {
	class RParticleEmitterManager;
	class RParticleCollisionManager;
	class BulletCollision;
}

_USING_NAMESPACE_REALSPACE2

#define DISCARD_COUNT	2048
#define FLUSH_COUNT		512

_NAMESPACE_REALSPACE2_BEGIN

struct RParticle {
	rvector position;
	rvector velocity;
	rvector accel;
	float	ftime;

	virtual bool Update(float fTimeElapsed);
};

struct POINTVERTEX
{
	rvector v;
	D3DCOLOR color;

	static const u32 FVF;
};

// C++14: Usar unique_ptr para RAII automático
class RParticles {
protected:
	rvector mInitialPos;
	
	// C++14: Contenedor con unique_ptr en lugar de raw pointers
	std::list<std::unique_ptr<RParticle>> m_particles;

public:
	RParticles();
	virtual ~RParticles();

	bool Create(const char *szTextureName,float fSize);
	void Destroy();
	void Clear();

	virtual bool Draw();
	virtual bool Update(float fTime, RealSpace2::RParticleCollisionManager* pCollisionManager = nullptr);

	// Métodos de compatibilidad con std::list
	size_t size() const { return m_particles.size(); }
	bool empty() const { return m_particles.empty(); }
	
	// Agregar partícula (toma ownership)
	void push_back(std::unique_ptr<RParticle> particle) {
		m_particles.push_back(std::move(particle));
	}
	
	// Agregar partícula desde raw pointer (toma ownership)
	// DEPRECATED: Usar push_back(std::unique_ptr<RParticle>) en su lugar
	void push_back(RParticle* particle) {
		m_particles.push_back(std::unique_ptr<RParticle>(particle));
	}
	
	// Iteradores
	using iterator = std::list<std::unique_ptr<RParticle>>::iterator;
	using const_iterator = std::list<std::unique_ptr<RParticle>>::const_iterator;
	
	iterator begin() { return m_particles.begin(); }
	iterator end() { return m_particles.end(); }
	const_iterator begin() const { return m_particles.begin(); }
	const_iterator end() const { return m_particles.end(); }

protected:
	float	m_fSize;

	RBaseTexture* m_Texture;

};


// C++14: Usar unique_ptr para RAII automático
class RParticleSystem {
private:
	// C++14: Contenedor con unique_ptr en lugar de raw pointers
	std::list<std::unique_ptr<RParticles>> m_particles;
	
	// NUEVO: Gestores de emisores y colisiones
	std::unique_ptr<RealSpace2::RParticleEmitterManager> m_pEmitterManager;
	std::unique_ptr<RealSpace2::RParticleCollisionManager> m_pCollisionManager;

public:
	RParticleSystem();
	virtual ~RParticleSystem();

	void Destroy();

	bool Draw();
	bool Update(float fTime);

	// Agregar partículas (retorna raw pointer para compatibilidad)
	RParticles *AddParticles(const char *szTextureName,float fSize);
	
	// NUEVO: Métodos para emisores y colisiones
	bool LoadEmittersFromXML(const char* filename);
	bool LoadCollisionsFromXML(const char* filename);
	void SetMapCollision(RealSpace2::BulletCollision* pCollision);
	
	RealSpace2::RParticleEmitterManager* GetEmitterManager() { return m_pEmitterManager.get(); }
	RealSpace2::RParticleCollisionManager* GetCollisionManager() { return m_pCollisionManager.get(); }
	
	// Métodos de compatibilidad con std::list
	size_t size() const { return m_particles.size(); }
	bool empty() const { return m_particles.empty(); }
	
	// Iteradores
	using iterator = std::list<std::unique_ptr<RParticles>>::iterator;
	using const_iterator = std::list<std::unique_ptr<RParticles>>::const_iterator;
	
	iterator begin() { return m_particles.begin(); }
	iterator end() { return m_particles.end(); }
	const_iterator begin() const { return m_particles.begin(); }
	const_iterator end() const { return m_particles.end(); }

	static void BeginState();
	static void EndState();

	static bool Restore();
	static bool Invalidate();

	static LPDIRECT3DVERTEXBUFFER9 m_pVB;
	static DWORD m_dwBase;
};


_NAMESPACE_REALSPACE2_END
