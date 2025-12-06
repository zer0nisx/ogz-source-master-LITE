# Propuesta: Sistema de Emisores y Colisiones para RParticleSystem

## Resumen Ejecutivo

Esta propuesta describe cómo implementar:
1. **Sistema de Emisores de Partículas** (Point, Box, Sphere, Line, Cone)
2. **Sistema de Colisiones** para partículas con el mapa y objetos
3. **Configuración XML por Mapa** para emisores y zonas de colisión

---

## 1. Análisis del Sistema Actual

### 1.1 **Sistema XML de Mapas**

**Ubicación**: `RealSpace2/Source/RBspObject.cpp`

**Estructura actual**:
```cpp
bool RBspObject::LoadRS2Map(rapidxml::xml_node<>& aParent) {
    for (auto* node = aParent.first_node(); node; node = node->next_sibling()) {
        auto* szTagName = node->name();
        if (_stricmp(szTagName, RTOK_MATERIALLIST) == 0)
            Open_MaterialList(*node);
        else if (_stricmp(szTagName, RTOK_LIGHTLIST) == 0)
            Open_LightList(*node);
        else if (_stricmp(szTagName, RTOK_OBJECTLIST) == 0)
            Open_ObjectList(*node);
        else if (_stricmp(szTagName, RTOK_DUMMYLIST) == 0)
            Open_DummyList(*node);
        // ... más elementos
    }
}
```

**Conclusión**: ✅ Podemos agregar nuevos elementos XML (`PARTICLEEMITTERLIST`, `PARTICLECOLLISIONLIST`) sin modificar la estructura existente.

---

### 1.2 **Sistema de Colisiones del Mapa**

**Ubicación**: `RealSpace2/Source/BulletCollision.cpp`

**Características**:
- ✅ Usa Bullet Physics para colisiones
- ✅ `BulletCollision::Pick()` - Ray casting
- ✅ `BulletCollision::CheckCylinder()` - Colisión cilíndrica
- ✅ Se construye automáticamente al cargar el mapa

**Conclusión**: ✅ Podemos usar `RBspObject::Collision` para detectar colisiones de partículas.

---

### 1.3 **Estructura de Archivos de Mapas**

**Formato actual**:
```
maps/
  └─ [MapName]/
      ├─ [MapName].rs          (archivo binario del mapa)
      └─ [MapName].xml         (descripción XML del mapa)
```

**Ejemplo**: `maps/Snow_Town/Snow_Town.xml`

**Conclusión**: ✅ Podemos agregar elementos XML al archivo de descripción del mapa existente, o crear un archivo separado `[MapName]_particles.xml`.

---

## 2. Propuesta de Implementación

### 2.1 **Opción A: XML Integrado en el Mapa (Recomendado)**

**Ventajas**:
- ✅ Todo en un solo archivo
- ✅ Consistente con la estructura actual
- ✅ Fácil de mantener

**Desventajas**:
- ⚠️ Requiere modificar archivos XML existentes (o crear nuevos)

**Estructura XML propuesta**:
```xml
<?xml version="1.0" encoding="UTF-8" ?>
<XML>
    <!-- Elementos existentes -->
    <MATERIALLIST>...</MATERIALLIST>
    <LIGHTLIST>...</LIGHTLIST>
    <OBJECTLIST>...</OBJECTLIST>
    
    <!-- NUEVO: Sistema de Emisores de Partículas -->
    <PARTICLEEMITTERLIST>
        <EMITTER name="snow_emitter_1" type="box" enabled="true">
            <POSITION x="0" y="0" z="1500" />
            <SIZE x="16000" y="16000" z="100" />
            <PARTICLES>
                <TEXTURE>sfx/water_splash.bmp</TEXTURE>
                <SIZE>25.0</SIZE>
                <SPAWN_RATE>400</SPAWN_RATE>  <!-- partículas por segundo -->
                <VELOCITY_MIN x="-40" y="-40" z="-250" />
                <VELOCITY_MAX x="40" y="40" z="-150" />
                <ACCELERATION x="0" y="0" z="-5" />
                <LIFETIME>500.0</LIFETIME>
            </PARTICLES>
        </EMITTER>
        
        <EMITTER name="fire_emitter_1" type="point" enabled="true">
            <POSITION x="100" y="200" z="50" />
            <PARTICLES>
                <TEXTURE>sfx/fire_particle.bmp</TEXTURE>
                <SIZE>10.0</SIZE>
                <SPAWN_RATE>100</SPAWN_RATE>
                <VELOCITY_MIN x="-10" y="-10" z="20" />
                <VELOCITY_MAX x="10" y="10" z="50" />
                <ACCELERATION x="0" y="0" z="-2" />
                <LIFETIME>200.0</LIFETIME>
            </PARTICLES>
        </EMITTER>
        
        <EMITTER name="rain_emitter_1" type="box" enabled="true">
            <POSITION x="0" y="0" z="2000" />
            <SIZE x="20000" y="20000" z="0" />
            <PARTICLES>
                <TEXTURE>sfx/rain_drop.bmp</TEXTURE>
                <SIZE>5.0</SIZE>
                <SPAWN_RATE>1000</SPAWN_RATE>
                <VELOCITY_MIN x="-5" y="-5" z="-200" />
                <VELOCITY_MAX x="5" y="5" z="-150" />
                <ACCELERATION x="0" y="0" z="0" />
                <LIFETIME>300.0</LIFETIME>
            </PARTICLES>
        </EMITTER>
    </PARTICLEEMITTERLIST>
    
    <!-- NUEVO: Zonas de Colisión para Partículas -->
    <PARTICLECOLLISIONLIST>
        <COLLISION_ZONE name="ground_collision" enabled="true">
            <TYPE>plane</TYPE>  <!-- plane, box, sphere -->
            <POSITION x="0" y="0" z="-1000" />
            <NORMAL x="0" y="0" z="1" />  <!-- Para plane -->
            <SIZE x="20000" y="20000" z="0" />  <!-- Para box -->
            <RADIUS>1000</RADIUS>  <!-- Para sphere -->
            <BOUNCE>0.2</BOUNCE>  <!-- Factor de rebote (0.0 = sin rebote, 1.0 = rebote perfecto) -->
            <FRICTION>0.5</FRICTION>  <!-- Factor de fricción -->
        </COLLISION_ZONE>
        
        <COLLISION_ZONE name="water_surface" enabled="true">
            <TYPE>plane</TYPE>
            <POSITION x="0" y="0" z="0" />
            <NORMAL x="0" y="0" z="1" />
            <BOUNCE>0.0</BOUNCE>  <!-- Sin rebote, las partículas se "hunden" -->
            <FRICTION>1.0</FRICTION>
        </COLLISION_ZONE>
    </PARTICLECOLLISIONLIST>
</XML>
```

---

### 2.2 **Opción B: XML Separado por Mapa**

**Ventajas**:
- ✅ No modifica archivos XML existentes
- ✅ Fácil de agregar/remover sin tocar el mapa principal
- ✅ Puede ser opcional (si no existe, no se carga)

**Desventajas**:
- ⚠️ Archivo adicional por mapa

**Estructura de archivos**:
```
maps/
  └─ [MapName]/
      ├─ [MapName].rs
      ├─ [MapName].xml
      └─ [MapName]_particles.xml  <!-- NUEVO -->
```

**Estructura XML propuesta** (`[MapName]_particles.xml`):
```xml
<?xml version="1.0" encoding="UTF-8" ?>
<XML id="PARTICLESYSTEM">
    <EMITTERLIST>
        <!-- Misma estructura que Opción A -->
    </EMITTERLIST>
    
    <COLLISIONLIST>
        <!-- Misma estructura que Opción A -->
    </COLLISIONLIST>
</XML>
```

**Carga**:
```cpp
// En RBspObject::OpenDescription() o ZWorld::Create()
char szParticleFile[256];
sprintf_safe(szParticleFile, "maps/%s/%s_particles.xml", mapName, mapName);
if (MZFile::Exists(szParticleFile, g_pFileSystem)) {
    LoadParticleSystem(szParticleFile);
}
```

---

## 3. Implementación Técnica

### 3.1 **Sistema de Emisores**

#### **3.1.1 Clases Propuestas**

```cpp
// RealSpace2/Include/RParticleEmitter.h

namespace RealSpace2 {

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
    float spawnJitter;   // Variación aleatoria en spawn rate
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
    bool LoadFromXMLNode(rapidxml::xml_node<>& node);
    
    void Update(float fTime);
    void Clear();
    
    RParticleEmitter* GetEmitter(const char* name);
    void SetEmitterEnabled(const char* name, bool enabled);
};

} // namespace RealSpace2
```

---

#### **3.1.2 Implementación de Emisores**

```cpp
// RealSpace2/Source/RParticleEmitter.cpp

rvector RParticleEmitter::GetRandomSpawnPosition() {
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
                float theta = RandomNumber(0.0f, 2.0f * PI);
                float phi = acos(RandomNumber(-1.0f, 1.0f));
                float r = RandomNumber(0.0f, m_config.radius);
                
                pos.x += r * sin(phi) * cos(theta);
                pos.y += r * sin(phi) * sin(theta);
                pos.z += r * cos(phi);
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
                float angle = RandomNumber(0.0f, 2.0f * PI);
                float distance = RandomNumber(0.0f, m_config.radius);
                rvector baseOffset = rvector(
                    cos(angle) * distance,
                    sin(angle) * distance,
                    0
                );
                pos += baseOffset;
            }
            break;
    }
    
    return pos;
}

rvector RParticleEmitter::GetRandomVelocity() {
    rvector vel;
    vel.x = RandomNumber(m_config.velocityMin.x, m_config.velocityMax.x);
    vel.y = RandomNumber(m_config.velocityMin.y, m_config.velocityMax.y);
    vel.z = RandomNumber(m_config.velocityMin.z, m_config.velocityMax.z);
    
    // Para cono, ajustar dirección según el cono
    if (m_config.type == RParticleEmitterType::Cone) {
        // Proyectar velocidad en la dirección del cono
        rvector coneDir = Normalize(m_config.direction);
        float speed = Magnitude(vel);
        vel = coneDir * speed;
        
        // Agregar variación angular
        float angle = RandomNumber(0.0f, m_config.radius / m_config.height); // Aproximación
        rvector perp = GetPerpendicularVector(coneDir);
        vel += perp * angle * speed * 0.1f;
    }
    
    return vel;
}

void RParticleEmitter::Update(float fTime) {
    if (!m_config.enabled || !m_pParticles)
        return;
    
    float deltaTime = fTime - m_fLastUpdateTime;
    m_fLastUpdateTime = fTime;
    
    // Calcular cuántas partículas spawnear
    float spawnRate = m_config.spawnRate * (1.0f + RandomNumber(-m_config.spawnJitter, m_config.spawnJitter));
    m_fSpawnAccumulator += spawnRate * deltaTime;
    
    int numToSpawn = static_cast<int>(m_fSpawnAccumulator);
    m_fSpawnAccumulator -= numToSpawn;
    
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
```

---

### 3.2 **Sistema de Colisiones**

#### **3.2.1 Clases Propuestas**

```cpp
// RealSpace2/Include/RParticleCollision.h

namespace RealSpace2 {

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
};

class RParticleCollisionManager {
private:
    std::list<RParticleCollisionZone> m_zones;
    BulletCollision* m_pMapCollision;  // Colisión del mapa (opcional)
    
public:
    RParticleCollisionManager();
    ~RParticleCollisionManager();
    
    bool LoadFromXML(const char* filename);
    bool LoadFromXMLNode(rapidxml::xml_node<>& node);
    
    // Verificar colisión de partícula
    bool CheckCollision(RParticle* particle, float deltaTime, 
                       rvector* outHitPos, rvector* outNormal);
    
    // Verificar colisión con el mapa (usando BulletCollision)
    bool CheckMapCollision(RParticle* particle, float deltaTime,
                          rvector* outHitPos, rvector* outNormal);
    
    void SetMapCollision(BulletCollision* pCollision) { m_pMapCollision = pCollision; }
    
    void Clear();
    void AddZone(const RParticleCollisionZone& zone);
    void RemoveZone(const char* name);
    void SetZoneEnabled(const char* name, bool enabled);
};

} // namespace RealSpace2
```

---

#### **3.2.2 Implementación de Colisiones**

```cpp
// RealSpace2/Source/RParticleCollision.cpp

bool RParticleCollisionManager::CheckCollision(RParticle* particle, float deltaTime,
                                                rvector* outHitPos, rvector* outNormal) {
    if (!particle)
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
                    
                    float tmin = 0.0f, tmax = deltaTime;
                    rvector invDir = rvector(1.0f / particle->velocity.x, 
                                           1.0f / particle->velocity.y, 
                                           1.0f / particle->velocity.z);
                    
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
                        // Calcular normal (simplificado: usar la cara más cercana)
                        normal = CalculateBoxNormal(hitPos, min, max);
                        hit = true;
                    }
                }
                break;
                
            case RParticleCollisionType::Sphere:
                {
                    // Ray-Sphere intersection
                    rvector toCenter = zone.position - startPos;
                    float proj = DotProduct(toCenter, Normalize(particle->velocity));
                    
                    if (proj > 0.0f && proj <= Magnitude(particle->velocity) * deltaTime) {
                        rvector closestPoint = startPos + Normalize(particle->velocity) * proj;
                        float distSq = MagnitudeSq(closestPoint - zone.position);
                        
                        if (distSq <= zone.radius * zone.radius) {
                            hitPos = closestPoint;
                            normal = Normalize(closestPoint - zone.position);
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
    
    // Verificar colisión con el mapa (si está disponible)
    if (m_pMapCollision) {
        return CheckMapCollision(particle, deltaTime, outHitPos, outNormal);
    }
    
    return false;
}

bool RParticleCollisionManager::CheckMapCollision(RParticle* particle, float deltaTime,
                                                  rvector* outHitPos, rvector* outNormal) {
    if (!m_pMapCollision || !particle)
        return false;
    
    rvector startPos = particle->position;
    rvector endPos = startPos + particle->velocity * deltaTime;
    
    // Usar BulletCollision::Pick() para ray casting
    rvector hitPos, normal;
    if (m_pMapCollision->Pick(startPos, endPos, &hitPos, &normal)) {
        if (outHitPos) *outHitPos = hitPos;
        if (outNormal) *outNormal = normal;
        
        // Aplicar rebote básico
        float bounceFactor = 0.3f;  // Configurable
        rvector reflected = particle->velocity - 2.0f * DotProduct(particle->velocity, normal) * normal;
        particle->velocity = reflected * bounceFactor;
        
        particle->position = hitPos;
        
        return true;
    }
    
    return false;
}
```

---

### 3.3 **Integración con RParticleSystem**

```cpp
// Modificaciones a RParticleSystem

class RParticleSystem {
private:
    std::list<std::unique_ptr<RParticles>> m_particles;
    RParticleEmitterManager* m_pEmitterManager;
    RParticleCollisionManager* m_pCollisionManager;
    
public:
    RParticleSystem();
    ~RParticleSystem();
    
    // ... métodos existentes ...
    
    // NUEVOS métodos
    bool LoadEmittersFromXML(const char* filename);
    bool LoadCollisionsFromXML(const char* filename);
    
    void SetMapCollision(BulletCollision* pCollision);
    
    RParticleEmitterManager* GetEmitterManager() { return m_pEmitterManager; }
    RParticleCollisionManager* GetCollisionManager() { return m_pCollisionManager; }
};

// Modificaciones a RParticles::Update()

bool RParticles::Update(float fTime) {
    // Actualizar física de partículas
    for (auto& particle_ptr : m_particles) {
        RParticle* p = particle_ptr.get();
        
        // Verificar colisiones antes de actualizar
        rvector hitPos, normal;
        if (m_pCollisionManager && 
            m_pCollisionManager->CheckCollision(p, fTime, &hitPos, &normal)) {
            // Colisión manejada en CheckCollision()
        }
        
        // Actualizar física
        p->Update(fTime);
    }
    
    // Eliminar partículas muertas
    m_particles.remove_if([fTime](const std::unique_ptr<RParticle>& pp) {
        return (pp->ftime > LIFETIME) || (pp->Update(fTime) == false);
    });
    
    return true;
}
```

---

### 3.4 **Carga desde XML**

```cpp
// RealSpace2/Source/RParticleEmitter.cpp

bool RParticleEmitterManager::LoadFromXMLNode(rapidxml::xml_node<>& node) {
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
                config.velocityMin = rvector(
                    static_cast<float>(atof(velMinNode->first_attribute("x")->value())),
                    static_cast<float>(atof(velMinNode->first_attribute("y")->value())),
                    static_cast<float>(atof(velMinNode->first_attribute("z")->value()))
                );
            }
            
            auto* velMaxNode = particlesNode->first_node("VELOCITY_MAX");
            if (velMaxNode) {
                config.velocityMax = rvector(
                    static_cast<float>(atof(velMaxNode->first_attribute("x")->value())),
                    static_cast<float>(atof(velMaxNode->first_attribute("y")->value())),
                    static_cast<float>(atof(velMaxNode->first_attribute("z")->value()))
                );
            }
            
            // Leer ACCELERATION
            auto* accelNode = particlesNode->first_node("ACCELERATION");
            if (accelNode) {
                config.acceleration = rvector(
                    static_cast<float>(atof(accelNode->first_attribute("x")->value())),
                    static_cast<float>(atof(accelNode->first_attribute("y")->value())),
                    static_cast<float>(atof(accelNode->first_attribute("z")->value()))
                );
            }
            
            // Leer LIFETIME
            auto* lifetimeNode = particlesNode->first_node("LIFETIME");
            if (lifetimeNode) config.lifetime = static_cast<float>(atof(lifetimeNode->value()));
        }
        
        // Crear emisor
        auto emitter = std::make_unique<RParticleEmitter>();
        if (emitter->Create(config, m_pParticleSystem)) {
            m_emitters.push_back(std::move(emitter));
        }
    }
    
    return true;
}
```

---

## 4. Integración con el Sistema de Mapas

### 4.1 **Modificación a RBspObject**

```cpp
// RealSpace2/Source/RBspObject.cpp

bool RBspObject::LoadRS2Map(rapidxml::xml_node<>& aParent) {
    for (auto* node = aParent.first_node(); node; node = node->next_sibling()) {
        auto* szTagName = node->name();
        
        // ... elementos existentes ...
        
        // NUEVO: Cargar emisores de partículas
        else if (_stricmp(szTagName, "PARTICLEEMITTERLIST") == 0) {
            if (RGetParticleSystem()) {
                RGetParticleSystem()->GetEmitterManager()->LoadFromXMLNode(*node);
            }
        }
        
        // NUEVO: Cargar zonas de colisión
        else if (_stricmp(szTagName, "PARTICLECOLLISIONLIST") == 0) {
            if (RGetParticleSystem()) {
                RGetParticleSystem()->GetCollisionManager()->LoadFromXMLNode(*node);
                // Conectar colisión del mapa
                RGetParticleSystem()->SetMapCollision(Collision.get());
            }
        }
    }
    
    return true;
}
```

---

### 4.2 **Modificación a ZWorld**

```cpp
// Gunz/ZWorld.cpp

bool ZWorld::Create(ZLoadingProgress *pLoading) {
    // ... código existente ...
    
    // NUEVO: Cargar sistema de partículas del mapa
    if (RGetParticleSystem() && m_pBsp) {
        // Opción A: Cargar desde XML del mapa (si está integrado)
        // Ya se carga en RBspObject::LoadRS2Map()
        
        // Opción B: Cargar desde archivo separado
        char szParticleFile[256];
        sprintf_safe(szParticleFile, "maps/%s/%s_particles.xml", m_szName, m_szName);
        if (MZFile::Exists(szParticleFile, g_pFileSystem)) {
            RGetParticleSystem()->LoadEmittersFromXML(szParticleFile);
            RGetParticleSystem()->LoadCollisionsFromXML(szParticleFile);
            RGetParticleSystem()->SetMapCollision(m_pBsp->GetCollision());
        }
    }
    
    return true;
}
```

---

## 5. Ejemplo de Uso Completo

### 5.1 **XML de Mapa con Sistema de Partículas**

```xml
<?xml version="1.0" encoding="UTF-8" ?>
<XML>
    <!-- ... elementos existentes del mapa ... -->
    
    <!-- Sistema de Nieve con Emisor Box -->
    <PARTICLEEMITTERLIST>
        <EMITTER name="snow_emitter" type="box" enabled="true">
            <POSITION x="0" y="0" z="1500" />
            <SIZE x="16000" y="16000" z="100" />
            <PARTICLES>
                <TEXTURE>sfx/water_splash.bmp</TEXTURE>
                <SIZE>25.0</SIZE>
                <SPAWN_RATE>400</SPAWN_RATE>
                <VELOCITY_MIN x="-40" y="-40" z="-250" />
                <VELOCITY_MAX x="40" y="40" z="-150" />
                <ACCELERATION x="0" y="0" z="-5" />
                <LIFETIME>500.0</LIFETIME>
            </PARTICLES>
        </EMITTER>
    </PARTICLEEMITTERLIST>
    
    <!-- Colisión con el suelo -->
    <PARTICLECOLLISIONLIST>
        <COLLISION_ZONE name="ground" enabled="true">
            <TYPE>plane</TYPE>
            <POSITION x="0" y="0" z="-1000" />
            <NORMAL x="0" y="0" z="1" />
            <BOUNCE>0.0</BOUNCE>
            <FRICTION>1.0</FRICTION>
            <KILL_ON_COLLISION>true</KILL_ON_COLLISION>
        </COLLISION_ZONE>
    </PARTICLECOLLISIONLIST>
</XML>
```

---

### 5.2 **Código de Actualización**

```cpp
// En ZGame::Update() o loop principal

void ZGame::Update(float fElapsed) {
    // ... código existente ...
    
    // Actualizar sistema de partículas
    if (RGetParticleSystem()) {
        // Actualizar emisores (spawnean nuevas partículas)
        RGetParticleSystem()->GetEmitterManager()->Update(fElapsed);
        
        // Actualizar partículas (física + colisiones)
        RGetParticleSystem()->Update(fElapsed);
    }
}

void ZGame::Draw() {
    // ... código existente ...
    
    // Renderizar partículas
    if (RGetParticleSystem()) {
        RGetParticleSystem()->Draw();
    }
}
```

---

## 6. Recomendación Final

### ✅ **Opción Recomendada: XML Separado**

**Razones**:
1. ✅ **No modifica archivos existentes**: Los mapas actuales no se tocan
2. ✅ **Opcional**: Si no existe el XML de partículas, el sistema funciona normalmente
3. ✅ **Fácil de mantener**: Cada mapa puede tener su propio archivo de partículas
4. ✅ **Extensible**: Fácil agregar más configuraciones sin afectar el XML del mapa principal

**Estructura propuesta**:
```
maps/
  └─ Snow_Town/
      ├─ Snow_Town.rs
      ├─ Snow_Town.xml          (existente, no se modifica)
      └─ Snow_Town_particles.xml (NUEVO, opcional)
```

**Carga**:
- Se carga automáticamente en `ZWorld::Create()` si existe
- Si no existe, el sistema funciona como antes (sin emisores ni colisiones personalizadas)

---

## 7. Plan de Implementación

### **Fase 1: Sistema de Emisores**
1. Crear `RParticleEmitter.h` y `RParticleEmitter.cpp`
2. Crear `RParticleEmitterManager.h` y `RParticleEmitterManager.cpp`
3. Implementar tipos de emisores (Point, Box, Sphere, Line, Cone)
4. Integrar con `RParticleSystem`

### **Fase 2: Sistema de Colisiones**
1. Crear `RParticleCollision.h` y `RParticleCollision.cpp`
2. Implementar tipos de colisión (Plane, Box, Sphere)
3. Integrar con `BulletCollision` del mapa
4. Implementar rebote y fricción

### **Fase 3: Carga desde XML**
1. Implementar parser XML para emisores
2. Implementar parser XML para zonas de colisión
3. Integrar carga en `ZWorld::Create()`

### **Fase 4: Testing y Optimización**
1. Crear XML de ejemplo para `Snow_Town`
2. Probar emisores y colisiones
3. Optimizar rendimiento si es necesario

---

## 8. Consideraciones de Rendimiento

### **Optimizaciones Propuestas**:
1. **Spatial Partitioning**: Dividir zonas de colisión en octree/quadtree
2. **Culling**: No verificar colisiones para partículas fuera de vista
3. **Batching**: Procesar colisiones en batches
4. **Lazy Evaluation**: Solo verificar colisiones cuando la partícula está cerca de una zona

---

## 9. Conclusión

Esta propuesta permite:
- ✅ **Emisores configurables** por mapa (Point, Box, Sphere, Line, Cone)
- ✅ **Colisiones** con zonas personalizadas y con el mapa
- ✅ **Configuración XML** opcional y no invasiva
- ✅ **Extensibilidad** para futuras mejoras

**¿Seguimos con la implementación?**

