#pragma once

#include "RTypes.h"
#include <string>
#include <list>

// Forward declarations
struct RParticle;

// Forward declaration for rapidxml
namespace rapidxml {
	template<class Ch> class xml_node;
}

_NAMESPACE_REALSPACE2_BEGIN

// Forward declarations
class BulletCollision;
struct RParticle;

enum class RParticleCollisionType {
	Plane,   // Plano infinito
	Box,     // Caja
	Sphere   // Esfera
};

struct RParticleCollisionZone {
	std::string name;
	RParticleCollisionType type;
	bool enabled;
	
	rvector position;
	rvector normal;      // Para Plane
	rvector size;        // Para Box
	float radius;        // Para Sphere
	
	float bounce;        // Factor de rebote (0.0 = sin rebote, 1.0 = rebote perfecto)
	float friction;      // Factor de fricción (0.0 = sin fricción, 1.0 = fricción máxima)
	
	// Opciones avanzadas
	bool killOnCollision;  // Si true, elimina la partícula en lugar de rebotar
	
	RParticleCollisionZone()
		: type(RParticleCollisionType::Plane)
		, enabled(true)
		, position(0, 0, 0)
		, normal(0, 0, 1)
		, size(1, 1, 1)
		, radius(1.0f)
		, bounce(0.3f)
		, friction(0.5f)
		, killOnCollision(false)
	{
	}
};

class RParticleCollisionManager {
private:
	std::list<RParticleCollisionZone> m_zones;
	BulletCollision* m_pMapCollision;  // Colisión del mapa (opcional)
	bool m_bAutoMapCollision;  // Detección automática de colisiones con el mapa
	float m_fMapBounce;        // Factor de rebote para colisiones con el mapa
	float m_fMapFriction;      // Factor de fricción para colisiones con el mapa
	bool m_bKillOnMapCollision; // Eliminar partículas al colisionar con el mapa
	
	// Helper functions
	rvector CalculateBoxNormal(const rvector& hitPos, const rvector& min, const rvector& max);
	
public:
	RParticleCollisionManager();
	~RParticleCollisionManager();
	
	bool LoadFromXML(const char* filename);
	bool LoadFromXMLNode(rapidxml::xml_node<char>& node);
	
	// Verificar colisión de partícula
	bool CheckCollision(RParticle* particle, float deltaTime, 
	                   rvector* outHitPos, rvector* outNormal);
	
	// Verificar colisión con el mapa (usando BulletCollision)
	bool CheckMapCollision(RParticle* particle, float deltaTime,
	                      rvector* outHitPos, rvector* outNormal);
	
	void SetMapCollision(BulletCollision* pCollision) { m_pMapCollision = pCollision; }
	BulletCollision* GetMapCollision() const { return m_pMapCollision; }
	
	// Configuración de colisión automática con el mapa
	void SetAutoMapCollision(bool enabled) { m_bAutoMapCollision = enabled; }
	bool GetAutoMapCollision() const { return m_bAutoMapCollision; }
	
	void SetMapBounce(float bounce) { m_fMapBounce = bounce; }
	float GetMapBounce() const { return m_fMapBounce; }
	
	void SetMapFriction(float friction) { m_fMapFriction = friction; }
	float GetMapFriction() const { return m_fMapFriction; }
	
	void SetKillOnMapCollision(bool kill) { m_bKillOnMapCollision = kill; }
	bool GetKillOnMapCollision() const { return m_bKillOnMapCollision; }
	
	void Clear();
	void AddZone(const RParticleCollisionZone& zone);
	void RemoveZone(const char* name);
	void SetZoneEnabled(const char* name, bool enabled);
	
	size_t GetZoneCount() const { return m_zones.size(); }
};

_NAMESPACE_REALSPACE2_END

