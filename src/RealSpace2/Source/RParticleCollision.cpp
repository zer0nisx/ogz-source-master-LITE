#include "stdafx.h"
#include "RParticleCollision.h"
#include "RParticleSystem.h"
#include "BulletCollision.h"
#include "RMath.h"
#include "MZFileSystem.h"
#include "MDebug.h"
#include "rapidxml.hpp"
#include <algorithm>
#include <cmath>

_USING_NAMESPACE_REALSPACE2
_NAMESPACE_REALSPACE2_BEGIN

#define LIFETIME 500.f  // Debe coincidir con RParticles::Update

RParticleCollisionManager::RParticleCollisionManager()
	: m_pMapCollision(nullptr)
	, m_bAutoMapCollision(true)  // Por defecto, detectar colisiones automáticamente con el mapa
	, m_fMapBounce(0.3f)         // Rebote por defecto
	, m_fMapFriction(0.5f)       // Fricción por defecto
	, m_bKillOnMapCollision(false) // Por defecto, no eliminar partículas
{
}

RParticleCollisionManager::~RParticleCollisionManager()
{
	Clear();
}

void RParticleCollisionManager::Clear()
{
	m_zones.clear();
	m_pMapCollision = nullptr;
}

void RParticleCollisionManager::AddZone(const RParticleCollisionZone& zone)
{
	m_zones.push_back(zone);
}

void RParticleCollisionManager::RemoveZone(const char* name)
{
	if (!name)
		return;
	
	m_zones.remove_if([name](const RParticleCollisionZone& zone) {
		return zone.name == name;
	});
}

void RParticleCollisionManager::SetZoneEnabled(const char* name, bool enabled)
{
	if (!name)
		return;
	
	for (auto& zone : m_zones) {
		if (zone.name == name) {
			zone.enabled = enabled;
			return;
		}
	}
}

rvector RParticleCollisionManager::CalculateBoxNormal(const rvector& hitPos, const rvector& min, const rvector& max)
{
	// Calcular la normal basada en la cara más cercana
	rvector center = (min + max) * 0.5f;
	rvector diff = hitPos - center;
	rvector size = max - min;
	
	// Encontrar la cara más cercana
	float minDist = fabs(diff.x / (size.x * 0.5f));
	int face = 0; // 0 = X, 1 = Y, 2 = Z
	
	float distY = fabs(diff.y / (size.y * 0.5f));
	if (distY < minDist) {
		minDist = distY;
		face = 1;
	}
	
	float distZ = fabs(diff.z / (size.z * 0.5f));
	if (distZ < minDist) {
		face = 2;
	}
	
	// Retornar normal según la cara
	rvector normal(0, 0, 0);
	if (face == 0) {
		normal.x = (diff.x > 0) ? 1.0f : -1.0f;
	} else if (face == 1) {
		normal.y = (diff.y > 0) ? 1.0f : -1.0f;
	} else {
		normal.z = (diff.z > 0) ? 1.0f : -1.0f;
	}
	
	return normal;
}

bool RParticleCollisionManager::CheckCollision(RParticle* particle, float deltaTime,
                                               rvector* outHitPos, rvector* outNormal)
{
	if (!particle || deltaTime <= 0.0f)
		return false;
	
	rvector startPos = particle->position;
	rvector endPos = startPos + particle->velocity * deltaTime;
	
	// Verificar colisiones con zonas personalizadas
	for (auto& zone : m_zones) {
		if (!zone.enabled)
			continue;
		
		rvector hitPos, normal;
		bool hit = false;
		
		switch (zone.type) {
			case RParticleCollisionType::Plane:
				{
					// Ray-Plane intersection
					float denom = DotProduct(particle->velocity, zone.normal);
					if (fabs(denom) > 0.0001f) {
						rvector toPlane = zone.position - startPos;
						float t = DotProduct(toPlane, zone.normal) / denom;
						
						if (t >= 0.0f && t <= 1.0f) {
							hitPos = startPos + particle->velocity * deltaTime * t;
							normal = zone.normal;
							hit = true;
						}
					}
				}
				break;
				
			case RParticleCollisionType::Box:
				{
					// Ray-Box intersection (AABB)
					rvector min = zone.position - zone.size * 0.5f;
					rvector max = zone.position + zone.size * 0.5f;
					
					rvector invDir;
					invDir.x = (fabs(particle->velocity.x) > 0.0001f) ? 1.0f / particle->velocity.x : 1e10f;
					invDir.y = (fabs(particle->velocity.y) > 0.0001f) ? 1.0f / particle->velocity.y : 1e10f;
					invDir.z = (fabs(particle->velocity.z) > 0.0001f) ? 1.0f / particle->velocity.z : 1e10f;
					
					float tmin = 0.0f, tmax = deltaTime;
					
					for (int i = 0; i < 3; i++) {
						float t1 = (min[i] - startPos[i]) * invDir[i];
						float t2 = (max[i] - startPos[i]) * invDir[i];
						
						if (t1 > t2) std::swap(t1, t2);
						tmin = std::max(tmin, t1);
						tmax = std::min(tmax, t2);
						
						if (tmin > tmax) break;
					}
					
					if (tmin <= tmax && tmin >= 0.0f && tmin <= deltaTime) {
						hitPos = startPos + particle->velocity * tmin;
						normal = CalculateBoxNormal(hitPos, min, max);
						hit = true;
					}
				}
				break;
				
			case RParticleCollisionType::Sphere:
				{
					// Ray-Sphere intersection
					rvector toCenter = zone.position - startPos;
					rvector velNorm = Normalized(particle->velocity);
					float proj = DotProduct(toCenter, velNorm);
					
					if (proj > 0.0f && proj <= Magnitude(particle->velocity) * deltaTime) {
						rvector closestPoint = startPos + velNorm * proj;
						float distSq = MagnitudeSq(closestPoint - zone.position);
						
						if (distSq <= zone.radius * zone.radius) {
							hitPos = closestPoint;
							normal = Normalized(closestPoint - zone.position);
							hit = true;
						}
					}
				}
				break;
		}
		
		if (hit) {
			if (outHitPos) *outHitPos = hitPos;
			if (outNormal) *outNormal = normal;
			
			// Aplicar rebote o eliminar partícula
			if (zone.killOnCollision) {
				particle->ftime = LIFETIME + 1.0f;  // Marcar para eliminación
			} else {
				// Calcular velocidad de rebote
				rvector reflected = particle->velocity - 2.0f * DotProduct(particle->velocity, normal) * normal;
				particle->velocity = reflected * zone.bounce;
				
				// Aplicar fricción
				rvector tangent = particle->velocity - DotProduct(particle->velocity, normal) * normal;
				particle->velocity = normal * DotProduct(particle->velocity, normal) + 
				                    tangent * (1.0f - zone.friction);
				
				particle->position = hitPos;
			}
			
			return true;
		}
	}
	
	// Verificar colisión automática con el mapa (si está habilitada y disponible)
	if (m_bAutoMapCollision && m_pMapCollision) {
		return CheckMapCollision(particle, deltaTime, outHitPos, outNormal);
	}
	
	return false;
}

bool RParticleCollisionManager::CheckMapCollision(RParticle* particle, float deltaTime,
                                                  rvector* outHitPos, rvector* outNormal)
{
	if (!m_pMapCollision || !particle || deltaTime <= 0.0f)
		return false;
	
	rvector startPos = particle->position;
	rvector endPos = startPos + particle->velocity * deltaTime;
	
	// Usar BulletCollision::Pick() para ray casting
	rvector hitPos, normal;
	if (m_pMapCollision->Pick(startPos, endPos, &hitPos, &normal)) {
		if (outHitPos) *outHitPos = hitPos;
		if (outNormal) *outNormal = normal;
		
		// Aplicar rebote o eliminar partícula según configuración
		if (m_bKillOnMapCollision) {
			particle->ftime = LIFETIME + 1.0f;  // Marcar para eliminación
		} else {
			// Calcular velocidad de rebote
			rvector reflected = particle->velocity - 2.0f * DotProduct(particle->velocity, normal) * normal;
			particle->velocity = reflected * m_fMapBounce;
			
			// Aplicar fricción
			rvector tangent = particle->velocity - DotProduct(particle->velocity, normal) * normal;
			particle->velocity = normal * DotProduct(particle->velocity, normal) + 
			                    tangent * (1.0f - m_fMapFriction);
			
			particle->position = hitPos;
		}
		
		return true;
	}
	
	return false;
}

bool RParticleCollisionManager::LoadFromXML(const char* filename)
{
	MZFile file;
	if (!file.Open(filename, g_pFileSystem)) {
		MLog("RParticleCollisionManager::LoadFromXML -- Failed to open file: %s\n", filename);
		return false;
	}
	
	int fileLength = file.GetLength();
	if (fileLength <= 0) {
		MLog("RParticleCollisionManager::LoadFromXML -- Invalid file length: %s\n", filename);
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
		MLog("RParticleCollisionManager::LoadFromXML -- Parse error: %s at %s\n", e.what(), e.where<char>());
		return false;
	}
	
	auto* xml = doc.first_node("XML");
	if (!xml) {
		MLog("RParticleCollisionManager::LoadFromXML -- No XML node found\n");
		return false;
	}
	
	auto* collisionListNode = xml->first_node("COLLISIONLIST");
	if (!collisionListNode) {
		MLog("RParticleCollisionManager::LoadFromXML -- No COLLISIONLIST node found\n");
		return false;
	}
	
	return LoadFromXMLNode(*collisionListNode);
}

bool RParticleCollisionManager::LoadFromXMLNode(rapidxml::xml_node<char>& node)
{
	// Leer configuración global de colisión con el mapa
	auto* autoMapNode = node.first_node("AUTO_MAP_COLLISION");
	if (autoMapNode) {
		m_bAutoMapCollision = (strcmp(autoMapNode->value(), "true") == 0);
		
		// Leer configuración de rebote y fricción del mapa
		auto* bounceNode = autoMapNode->first_node("BOUNCE");
		if (bounceNode) {
			m_fMapBounce = static_cast<float>(atof(bounceNode->value()));
		}
		
		auto* frictionNode = autoMapNode->first_node("FRICTION");
		if (frictionNode) {
			m_fMapFriction = static_cast<float>(atof(frictionNode->value()));
		}
		
		auto* killNode = autoMapNode->first_node("KILL_ON_COLLISION");
		if (killNode) {
			m_bKillOnMapCollision = (strcmp(killNode->value(), "true") == 0);
		}
	}
	
	// Leer zonas de colisión personalizadas
	for (auto* zoneNode = node.first_node("COLLISION_ZONE");
		zoneNode;
		zoneNode = zoneNode->next_sibling("COLLISION_ZONE")) {
		
		RParticleCollisionZone zone;
		
		// Leer atributos
		auto* nameAttr = zoneNode->first_attribute("name");
		if (nameAttr) zone.name = nameAttr->value();
		
		auto* enabledAttr = zoneNode->first_attribute("enabled");
		if (enabledAttr) zone.enabled = (strcmp(enabledAttr->value(), "true") == 0);
		
		// Leer TYPE
		auto* typeNode = zoneNode->first_node("TYPE");
		if (typeNode) {
			std::string typeStr = typeNode->value();
			if (typeStr == "plane") zone.type = RParticleCollisionType::Plane;
			else if (typeStr == "box") zone.type = RParticleCollisionType::Box;
			else if (typeStr == "sphere") zone.type = RParticleCollisionType::Sphere;
		}
		
		// Leer POSITION
		auto* posNode = zoneNode->first_node("POSITION");
		if (posNode) {
			auto* xAttr = posNode->first_attribute("x");
			auto* yAttr = posNode->first_attribute("y");
			auto* zAttr = posNode->first_attribute("z");
			if (xAttr && yAttr && zAttr) {
				zone.position = rvector(
					static_cast<float>(atof(xAttr->value())),
					static_cast<float>(atof(yAttr->value())),
					static_cast<float>(atof(zAttr->value()))
				);
			}
		}
		
		// Leer NORMAL (para Plane)
		auto* normalNode = zoneNode->first_node("NORMAL");
		if (normalNode) {
			auto* xAttr = normalNode->first_attribute("x");
			auto* yAttr = normalNode->first_attribute("y");
			auto* zAttr = normalNode->first_attribute("z");
			if (xAttr && yAttr && zAttr) {
				zone.normal = rvector(
					static_cast<float>(atof(xAttr->value())),
					static_cast<float>(atof(yAttr->value())),
					static_cast<float>(atof(zAttr->value()))
				);
				Normalize(zone.normal);
			}
		}
		
		// Leer SIZE (para Box)
		auto* sizeNode = zoneNode->first_node("SIZE");
		if (sizeNode) {
			auto* xAttr = sizeNode->first_attribute("x");
			auto* yAttr = sizeNode->first_attribute("y");
			auto* zAttr = sizeNode->first_attribute("z");
			if (xAttr && yAttr && zAttr) {
				zone.size = rvector(
					static_cast<float>(atof(xAttr->value())),
					static_cast<float>(atof(yAttr->value())),
					static_cast<float>(atof(zAttr->value()))
				);
			}
		}
		
		// Leer RADIUS (para Sphere)
		auto* radiusNode = zoneNode->first_node("RADIUS");
		if (radiusNode) {
			zone.radius = static_cast<float>(atof(radiusNode->value()));
		}
		
		// Leer BOUNCE
		auto* bounceNode = zoneNode->first_node("BOUNCE");
		if (bounceNode) {
			zone.bounce = static_cast<float>(atof(bounceNode->value()));
		}
		
		// Leer FRICTION
		auto* frictionNode = zoneNode->first_node("FRICTION");
		if (frictionNode) {
			zone.friction = static_cast<float>(atof(frictionNode->value()));
		}
		
		// Leer KILL_ON_COLLISION
		auto* killNode = zoneNode->first_node("KILL_ON_COLLISION");
		if (killNode) {
			zone.killOnCollision = (strcmp(killNode->value(), "true") == 0);
		}
		
		AddZone(zone);
		MLog("RParticleCollisionManager -- Loaded collision zone: %s\n", zone.name.c_str());
	}
	
	return true;
}

_NAMESPACE_REALSPACE2_END

