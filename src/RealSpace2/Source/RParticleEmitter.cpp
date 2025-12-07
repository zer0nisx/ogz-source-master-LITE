#include "stdafx.h"
#include "RParticleEmitter.h"
#include "RealSpace2.h"
#include "RMath.h"
#include "MMath.h"
#include "MZFileSystem.h"
#include "MDebug.h"
#include "rapidxml.hpp"
#include <algorithm>

_USING_NAMESPACE_REALSPACE2
_NAMESPACE_REALSPACE2_BEGIN

RParticleEmitter::RParticleEmitter()
	: m_pParticles(nullptr)
	, m_fSpawnAccumulator(0.0f)
	, m_fLastUpdateTime(0.0f)
{
}

RParticleEmitter::~RParticleEmitter()
{
	Destroy();
}

bool RParticleEmitter::Create(const RParticleEmitterConfig& config, RParticleSystem* pSystem)
{
	if (!pSystem)
		return false;
	
	m_config = config;
	
	// Crear grupo de partículas para este emisor
	m_pParticles = pSystem->AddParticles(m_config.texture.c_str(), m_config.particleSize);
	if (!m_pParticles)
		return false;
	
	m_fSpawnAccumulator = 0.0f;
	m_fLastUpdateTime = 0.0f;
	
	return true;
}

void RParticleEmitter::Destroy()
{
	// El RParticleSystem se encarga de destruir los grupos de partículas
	m_pParticles = nullptr;
}

rvector RParticleEmitter::GetRandomSpawnPosition()
{
	rvector pos = m_config.position;
	
	switch (m_config.type) {
		case RParticleEmitterType::Point:
			// Ya está en posición
			break;
			
		case RParticleEmitterType::Box:
			pos.x += RandomNumber(-m_config.size.x * 0.5f, m_config.size.x * 0.5f);
			pos.y += RandomNumber(-m_config.size.y * 0.5f, m_config.size.y * 0.5f);
			pos.z += RandomNumber(-m_config.size.z * 0.5f, m_config.size.z * 0.5f);
			break;
			
		case RParticleEmitterType::Sphere:
			{
				// Distribución uniforme en esfera
				float theta = RandomNumber(0.0f, 2.0f * PI_FLOAT);
				float phi = acosf(RandomNumber(-1.0f, 1.0f));
				float r = RandomNumber(0.0f, m_config.radius);
				
				pos.x += r * sinf(phi) * cosf(theta);
				pos.y += r * sinf(phi) * sinf(theta);
				pos.z += r * cosf(phi);
			}
			break;
			
		case RParticleEmitterType::Line:
			{
				float t = RandomNumber(0.0f, 1.0f);
				pos += m_config.direction * t;
			}
			break;
			
		case RParticleEmitterType::Cone:
			{
				// Emitir desde la base del cono
				float angle = RandomNumber(0.0f, 2.0f * PI_FLOAT);
				float distance = RandomNumber(0.0f, m_config.radius);
				rvector baseOffset = rvector(
					cosf(angle) * distance,
					sinf(angle) * distance,
					0
				);
				pos += baseOffset;
			}
			break;
	}
	
	return pos;
}

rvector RParticleEmitter::GetRandomVelocity()
{
	rvector vel;
	vel.x = RandomNumber(m_config.velocityMin.x, m_config.velocityMax.x);
	vel.y = RandomNumber(m_config.velocityMin.y, m_config.velocityMax.y);
	vel.z = RandomNumber(m_config.velocityMin.z, m_config.velocityMax.z);
	
	// Para cono, ajustar dirección según el cono
	if (m_config.type == RParticleEmitterType::Cone) {
		// Proyectar velocidad en la dirección del cono
		rvector coneDir = Normalized(m_config.direction);
		float speed = Magnitude(vel);
		vel = coneDir * speed;
		
		// Agregar variación angular
		float angle = RandomNumber(0.0f, m_config.radius / m_config.height); // Aproximación
		// Crear vector perpendicular usando cross product con un vector arbitrario
		rvector arbitrary = (fabs(coneDir.z) < 0.9f) ? rvector(0, 0, 1) : rvector(1, 0, 0);
		rvector perp = Normalized(CrossProduct(coneDir, arbitrary));
		vel += perp * angle * speed * 0.1f;
	}
	
	return vel;
}

void RParticleEmitter::Update(float fTime)
{
	if (!m_config.enabled || !m_pParticles)
		return;
	
	float deltaTime = fTime - m_fLastUpdateTime;
	if (deltaTime <= 0.0f)
		return;
	
	m_fLastUpdateTime = fTime;
	
	// Calcular cuántas partículas spawnear
	float spawnRate = m_config.spawnRate * (1.0f + RandomNumber(-m_config.spawnJitter, m_config.spawnJitter));
	m_fSpawnAccumulator += spawnRate * deltaTime;
	
	int numToSpawn = static_cast<int>(m_fSpawnAccumulator);
	m_fSpawnAccumulator -= static_cast<float>(numToSpawn);
	
	// Limitar número de partículas por frame para evitar spikes
	if (numToSpawn > 50)
		numToSpawn = 50;
	
	// Spawnear partículas
	for (int i = 0; i < numToSpawn; i++) {
		auto particle = std::make_unique<RParticle>();
		particle->position = GetRandomSpawnPosition();
		particle->velocity = GetRandomVelocity();
		particle->accel = m_config.acceleration;
		particle->ftime = 0.0f;
		
		m_pParticles->push_back(std::move(particle));
	}
}

// ============================================================================
// RParticleEmitterManager
// ============================================================================

RParticleEmitterManager::RParticleEmitterManager(RParticleSystem* pSystem)
	: m_pParticleSystem(pSystem)
{
}

RParticleEmitterManager::~RParticleEmitterManager()
{
	Clear();
}

void RParticleEmitterManager::Clear()
{
	m_emitters.clear();
}

void RParticleEmitterManager::Update(float fTime)
{
	for (auto& emitter : m_emitters) {
		emitter->Update(fTime);
	}
}

RParticleEmitter* RParticleEmitterManager::GetEmitter(const char* name)
{
	if (!name)
		return nullptr;
	
	for (auto& emitter : m_emitters) {
		if (emitter->GetConfig().name == name) {
			return emitter.get();
		}
	}
	
	return nullptr;
}

void RParticleEmitterManager::SetEmitterEnabled(const char* name, bool enabled)
{
	RParticleEmitter* emitter = GetEmitter(name);
	if (emitter) {
		emitter->SetEnabled(enabled);
	}
}

bool RParticleEmitterManager::LoadFromXML(const char* filename)
{
	MZFile file;
	if (!file.Open(filename, g_pFileSystem)) {
		MLog("RParticleEmitterManager::LoadFromXML -- Failed to open file: %s\n", filename);
		return false;
	}
	
	int fileLength = file.GetLength();
	if (fileLength <= 0) {
		MLog("RParticleEmitterManager::LoadFromXML -- Invalid file length: %s\n", filename);
		return false;
	}
	
	std::string xmlData;
	xmlData.resize(fileLength);
	file.Read(&xmlData[0], fileLength);
	
	rapidxml::xml_document<> doc;
	try {
		doc.parse<rapidxml::parse_no_data_nodes>(&xmlData[0], xmlData.size());
	}
	catch (rapidxml::parse_error& e) {
		MLog("RParticleEmitterManager::LoadFromXML -- Parse error: %s at %s\n", e.what(), e.where<char>());
		return false;
	}
	
	auto* xml = doc.first_node("XML");
	if (!xml) {
		MLog("RParticleEmitterManager::LoadFromXML -- No XML node found\n");
		return false;
	}
	
	auto* emitterListNode = xml->first_node("EMITTERLIST");
	if (!emitterListNode) {
		MLog("RParticleEmitterManager::LoadFromXML -- No EMITTERLIST node found\n");
		return false;
	}
	
	return LoadFromXMLNode(*emitterListNode);
}

bool RParticleEmitterManager::LoadFromXMLNode(rapidxml::xml_node<char>& node)
{
	for (auto* emitterNode = node.first_node("EMITTER");
		emitterNode;
		emitterNode = emitterNode->next_sibling("EMITTER")) {
		
		RParticleEmitterConfig config;
		
		// Leer atributos
		auto* nameAttr = emitterNode->first_attribute("name");
		if (nameAttr) config.name = nameAttr->value();
		
		auto* typeAttr = emitterNode->first_attribute("type");
		if (typeAttr) {
			std::string typeStr = typeAttr->value();
			if (typeStr == "point") config.type = RParticleEmitterType::Point;
			else if (typeStr == "box") config.type = RParticleEmitterType::Box;
			else if (typeStr == "sphere") config.type = RParticleEmitterType::Sphere;
			else if (typeStr == "line") config.type = RParticleEmitterType::Line;
			else if (typeStr == "cone") config.type = RParticleEmitterType::Cone;
		}
		
		auto* enabledAttr = emitterNode->first_attribute("enabled");
		if (enabledAttr) config.enabled = (strcmp(enabledAttr->value(), "true") == 0);
		
		// Leer POSITION
		auto* posNode = emitterNode->first_node("POSITION");
		if (posNode) {
			auto* xAttr = posNode->first_attribute("x");
			auto* yAttr = posNode->first_attribute("y");
			auto* zAttr = posNode->first_attribute("z");
			if (xAttr && yAttr && zAttr) {
				config.position = rvector(
					static_cast<float>(atof(xAttr->value())),
					static_cast<float>(atof(yAttr->value())),
					static_cast<float>(atof(zAttr->value()))
				);
			}
		}
		
		// Leer SIZE (para Box)
		auto* sizeNode = emitterNode->first_node("SIZE");
		if (sizeNode) {
			auto* xAttr = sizeNode->first_attribute("x");
			auto* yAttr = sizeNode->first_attribute("y");
			auto* zAttr = sizeNode->first_attribute("z");
			if (xAttr && yAttr && zAttr) {
				config.size = rvector(
					static_cast<float>(atof(xAttr->value())),
					static_cast<float>(atof(yAttr->value())),
					static_cast<float>(atof(zAttr->value()))
				);
			}
		}
		
		// Leer RADIUS (para Sphere/Cone)
		auto* radiusNode = emitterNode->first_node("RADIUS");
		if (radiusNode) {
			config.radius = static_cast<float>(atof(radiusNode->value()));
		}
		
		// Leer HEIGHT (para Cone)
		auto* heightNode = emitterNode->first_node("HEIGHT");
		if (heightNode) {
			config.height = static_cast<float>(atof(heightNode->value()));
		}
		
		// Leer DIRECTION (para Cone/Line)
		auto* dirNode = emitterNode->first_node("DIRECTION");
		if (dirNode) {
			auto* xAttr = dirNode->first_attribute("x");
			auto* yAttr = dirNode->first_attribute("y");
			auto* zAttr = dirNode->first_attribute("z");
			if (xAttr && yAttr && zAttr) {
				config.direction = rvector(
					static_cast<float>(atof(xAttr->value())),
					static_cast<float>(atof(yAttr->value())),
					static_cast<float>(atof(zAttr->value()))
				);
			}
		}
		
		// Leer PARTICLES
		auto* particlesNode = emitterNode->first_node("PARTICLES");
		if (particlesNode) {
			auto* textureNode = particlesNode->first_node("TEXTURE");
			if (textureNode) config.texture = textureNode->value();
			
			auto* sizeNode = particlesNode->first_node("SIZE");
			if (sizeNode) config.particleSize = static_cast<float>(atof(sizeNode->value()));
			
			auto* spawnRateNode = particlesNode->first_node("SPAWN_RATE");
			if (spawnRateNode) config.spawnRate = atoi(spawnRateNode->value());
			
			// Leer VELOCITY_MIN y VELOCITY_MAX
			auto* velMinNode = particlesNode->first_node("VELOCITY_MIN");
			if (velMinNode) {
				auto* xAttr = velMinNode->first_attribute("x");
				auto* yAttr = velMinNode->first_attribute("y");
				auto* zAttr = velMinNode->first_attribute("z");
				if (xAttr && yAttr && zAttr) {
					config.velocityMin = rvector(
						static_cast<float>(atof(xAttr->value())),
						static_cast<float>(atof(yAttr->value())),
						static_cast<float>(atof(zAttr->value()))
					);
				}
			}
			
			auto* velMaxNode = particlesNode->first_node("VELOCITY_MAX");
			if (velMaxNode) {
				auto* xAttr = velMaxNode->first_attribute("x");
				auto* yAttr = velMaxNode->first_attribute("y");
				auto* zAttr = velMaxNode->first_attribute("z");
				if (xAttr && yAttr && zAttr) {
					config.velocityMax = rvector(
						static_cast<float>(atof(xAttr->value())),
						static_cast<float>(atof(yAttr->value())),
						static_cast<float>(atof(zAttr->value()))
					);
				}
			}
			
			// Leer ACCELERATION
			auto* accelNode = particlesNode->first_node("ACCELERATION");
			if (accelNode) {
				auto* xAttr = accelNode->first_attribute("x");
				auto* yAttr = accelNode->first_attribute("y");
				auto* zAttr = accelNode->first_attribute("z");
				if (xAttr && yAttr && zAttr) {
					config.acceleration = rvector(
						static_cast<float>(atof(xAttr->value())),
						static_cast<float>(atof(yAttr->value())),
						static_cast<float>(atof(zAttr->value()))
					);
				}
			}
			
			// Leer LIFETIME
			auto* lifetimeNode = particlesNode->first_node("LIFETIME");
			if (lifetimeNode) config.lifetime = static_cast<float>(atof(lifetimeNode->value()));
		}
		
		// Crear emisor
		auto emitter = std::make_unique<RParticleEmitter>();
		if (emitter->Create(config, m_pParticleSystem)) {
			m_emitters.push_back(std::move(emitter));
			MLog("RParticleEmitterManager -- Loaded emitter: %s\n", config.name.c_str());
		} else {
			MLog("RParticleEmitterManager -- Failed to create emitter: %s\n", config.name.c_str());
		}
	}
	
	return true;
}

_NAMESPACE_REALSPACE2_END

