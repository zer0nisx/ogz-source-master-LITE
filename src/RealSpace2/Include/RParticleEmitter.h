#pragma once

#include "RTypes.h"
#include "RParticleSystem.h"
#include <string>
#include <list>
#include <memory>

// Forward declaration for rapidxml
namespace rapidxml {
	template<class Ch> class xml_node;
}

_NAMESPACE_REALSPACE2_BEGIN

// Forward declarations
class RParticleSystem;
class RParticles;

enum class RParticleEmitterType {
	Point,   // Emite desde un punto
	Box,     // Emite desde una caja
	Sphere,  // Emite desde una esfera
	Line,    // Emite desde una línea
	Cone     // Emite desde un cono
};

struct RParticleEmitterConfig {
	std::string name;
	RParticleEmitterType type;
	bool enabled;
	
	rvector position;
	rvector size;        // Para Box
	float radius;        // Para Sphere/Cone
	float height;        // Para Cone
	rvector direction;   // Para Cone/Line
	
	// Configuración de partículas
	std::string texture;
	float particleSize;
	int spawnRate;       // Partículas por segundo
	rvector velocityMin;
	rvector velocityMax;
	rvector acceleration;
	float lifetime;
	
	// Opciones avanzadas
	bool useWorldSpace;  // true = espacio mundo, false = espacio local
	float spawnJitter;   // Variación aleatoria en spawn rate (0.0 = sin variación)
	
	RParticleEmitterConfig() 
		: type(RParticleEmitterType::Point)
		, enabled(true)
		, position(0, 0, 0)
		, size(1, 1, 1)
		, radius(1.0f)
		, height(1.0f)
		, direction(0, 0, 1)
		, particleSize(10.0f)
		, spawnRate(100)
		, velocityMin(0, 0, 0)
		, velocityMax(0, 0, 0)
		, acceleration(0, 0, 0)
		, lifetime(500.0f)
		, useWorldSpace(true)
		, spawnJitter(0.1f)
	{
	}
};

class RParticleEmitter {
private:
	RParticleEmitterConfig m_config;
	RParticles* m_pParticles;
	float m_fSpawnAccumulator;  // Acumulador para spawn rate
	float m_fLastUpdateTime;
	
public:
	RParticleEmitter();
	~RParticleEmitter();
	
	bool Create(const RParticleEmitterConfig& config, RParticleSystem* pSystem);
	void Destroy();
	
	void Update(float fTime);
	void SetEnabled(bool enabled) { m_config.enabled = enabled; }
	bool IsEnabled() const { return m_config.enabled; }
	
	const RParticleEmitterConfig& GetConfig() const { return m_config; }
	RParticles* GetParticles() const { return m_pParticles; }
	
	// Generar posición aleatoria según tipo de emisor
	rvector GetRandomSpawnPosition();
	
	// Generar velocidad aleatoria
	rvector GetRandomVelocity();
};

class RParticleEmitterManager {
private:
	std::list<std::unique_ptr<RParticleEmitter>> m_emitters;
	RParticleSystem* m_pParticleSystem;
	
public:
	RParticleEmitterManager(RParticleSystem* pSystem);
	~RParticleEmitterManager();
	
	bool LoadFromXML(const char* filename);
	bool LoadFromXMLNode(rapidxml::xml_node<char>& node);
	
	void Update(float fTime);
	void Clear();
	
	RParticleEmitter* GetEmitter(const char* name);
	void SetEmitterEnabled(const char* name, bool enabled);
	
	size_t GetEmitterCount() const { return m_emitters.size(); }
};

_NAMESPACE_REALSPACE2_END

