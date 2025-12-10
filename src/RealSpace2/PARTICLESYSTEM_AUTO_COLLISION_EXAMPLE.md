# Sistema de Colisiones Automáticas con el Mapa

## Resumen

El sistema de partículas ahora **detecta automáticamente colisiones con todo el mapa** usando el `BulletCollision` del BSP object, sin necesidad de definir zonas de colisión manualmente.

---

## Características

### ✅ **Detección Automática Activada por Defecto**

Cuando se carga un mapa, el sistema automáticamente:
1. Conecta el `BulletCollision` del mapa al sistema de partículas
2. Habilita la detección automática de colisiones
3. Las partículas colisionan con toda la geometría del mapa

### ✅ **Configuración Flexible**

Puedes configurar:
- **Rebote**: Factor de rebote al colisionar con el mapa (0.0 = sin rebote, 1.0 = rebote perfecto)
- **Fricción**: Factor de fricción (0.0 = sin fricción, 1.0 = fricción máxima)
- **Eliminación**: Si las partículas se eliminan al colisionar o rebotan

---

## Ejemplo de XML

### **Opción 1: Usar Detección Automática (Recomendado)**

```xml
<?xml version="1.0" encoding="UTF-8" ?>
<XML id="PARTICLESYSTEM">
    <EMITTERLIST>
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
    </EMITTERLIST>
    
    <COLLISIONLIST>
        <!-- Configuración de colisión automática con el mapa -->
        <AUTO_MAP_COLLISION>
            <!-- Habilitar detección automática (por defecto: true) -->
            true
            <!-- Factor de rebote al colisionar con el mapa (0.0 - 1.0) -->
            <BOUNCE>0.2</BOUNCE>
            <!-- Factor de fricción (0.0 - 1.0) -->
            <FRICTION>0.8</FRICTION>
            <!-- Eliminar partículas al colisionar (true/false) -->
            <KILL_ON_COLLISION>true</KILL_ON_COLLISION>
        </AUTO_MAP_COLLISION>
        
        <!-- Opcional: Zonas de colisión personalizadas adicionales -->
        <!-- Estas se verifican ANTES de la colisión con el mapa -->
        <COLLISION_ZONE name="water_surface" enabled="true">
            <TYPE>plane</TYPE>
            <POSITION x="0" y="0" z="0" />
            <NORMAL x="0" y="0" z="1" />
            <BOUNCE>0.0</BOUNCE>
            <FRICTION>1.0</FRICTION>
            <KILL_ON_COLLISION>true</KILL_ON_COLLISION>
        </COLLISION_ZONE>
    </COLLISIONLIST>
</XML>
```

### **Opción 2: Solo Detección Automática (Sin Zonas Personalizadas)**

```xml
<?xml version="1.0" encoding="UTF-8" ?>
<XML id="PARTICLESYSTEM">
    <EMITTERLIST>
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
    </EMITTERLIST>
    
    <COLLISIONLIST>
        <!-- Solo detección automática con el mapa -->
        <!-- Si no especificas AUTO_MAP_COLLISION, se usa la configuración por defecto -->
        <!-- Por defecto: bounce=0.3, friction=0.5, killOnCollision=false -->
    </COLLISIONLIST>
</XML>
```

### **Opción 3: Deshabilitar Detección Automática**

```xml
<?xml version="1.0" encoding="UTF-8" ?>
<XML id="PARTICLESYSTEM">
    <COLLISIONLIST>
        <!-- Deshabilitar detección automática -->
        <AUTO_MAP_COLLISION>false</AUTO_MAP_COLLISION>
        
        <!-- Usar solo zonas personalizadas -->
        <COLLISION_ZONE name="ground" enabled="true">
            <TYPE>plane</TYPE>
            <POSITION x="0" y="0" z="-1000" />
            <NORMAL x="0" y="0" z="1" />
            <BOUNCE>0.0</BOUNCE>
            <FRICTION>1.0</FRICTION>
            <KILL_ON_COLLISION>true</KILL_ON_COLLISION>
        </COLLISION_ZONE>
    </COLLISIONLIST>
</XML>
```

---

## Configuración por Defecto

Si no especificas `AUTO_MAP_COLLISION` en el XML, el sistema usa estos valores por defecto:

- **Detección automática**: `true` (habilitada)
- **Rebote**: `0.3` (rebote moderado)
- **Fricción**: `0.5` (fricción media)
- **Eliminar partículas**: `false` (las partículas rebotan)

---

## Orden de Verificación de Colisiones

El sistema verifica colisiones en este orden:

1. **Zonas personalizadas** (si están definidas y habilitadas)
2. **Colisión automática con el mapa** (si está habilitada)

Si una partícula colisiona con una zona personalizada, **no se verifica** la colisión con el mapa (para evitar doble procesamiento).

---

## Uso en Código

### **Habilitar/Deshabilitar Detección Automática**

```cpp
// Habilitar detección automática
RGetParticleSystem()->GetCollisionManager()->SetAutoMapCollision(true);

// Deshabilitar detección automática
RGetParticleSystem()->GetCollisionManager()->SetAutoMapCollision(false);
```

### **Configurar Rebote y Fricción**

```cpp
// Configurar rebote (0.0 = sin rebote, 1.0 = rebote perfecto)
RGetParticleSystem()->GetCollisionManager()->SetMapBounce(0.2f);

// Configurar fricción (0.0 = sin fricción, 1.0 = fricción máxima)
RGetParticleSystem()->GetCollisionManager()->SetMapFriction(0.8f);

// Eliminar partículas al colisionar con el mapa
RGetParticleSystem()->GetCollisionManager()->SetKillOnMapCollision(true);
```

---

## Ventajas

### ✅ **Sin Configuración Manual**

- No necesitas definir zonas de colisión para cada superficie del mapa
- El sistema detecta automáticamente todas las colisiones con la geometría del mapa

### ✅ **Rendimiento Optimizado**

- Usa `BulletCollision` que ya está construido para el mapa
- Ray casting eficiente con el BSP tree

### ✅ **Flexible**

- Puedes combinar detección automática con zonas personalizadas
- Puedes deshabilitar la detección automática si prefieres control manual

---

## Ejemplos de Uso

### **Nieve que se Desvanece al Tocar el Suelo**

```xml
<AUTO_MAP_COLLISION>
    true
    <BOUNCE>0.0</BOUNCE>
    <FRICTION>1.0</FRICTION>
    <KILL_ON_COLLISION>true</KILL_ON_COLLISION>
</AUTO_MAP_COLLISION>
```

### **Chispas que Rebotan en las Paredes**

```xml
<AUTO_MAP_COLLISION>
    true
    <BOUNCE>0.5</BOUNCE>
    <FRICTION>0.3</FRICTION>
    <KILL_ON_COLLISION>false</KILL_ON_COLLISION>
</AUTO_MAP_COLLISION>
```

### **Lluvia que se Desvanece al Tocar el Suelo**

```xml
<AUTO_MAP_COLLISION>
    true
    <BOUNCE>0.0</BOUNCE>
    <FRICTION>1.0</FRICTION>
    <KILL_ON_COLLISION>true</KILL_ON_COLLISION>
</AUTO_MAP_COLLISION>
```

---

## Notas Técnicas

- La detección automática usa `BulletCollision::Pick()` para ray casting
- Solo funciona si el mapa tiene `BulletCollision` construido (requiere `_WIN32`)
- Las partículas se verifican contra toda la geometría del mapa, no solo superficies específicas
- El rendimiento depende del número de partículas y la complejidad del mapa

---

## Conclusión

Con la detección automática de colisiones, puedes crear efectos de partículas realistas sin necesidad de configurar manualmente cada superficie del mapa. El sistema se encarga de todo automáticamente. 🎉





