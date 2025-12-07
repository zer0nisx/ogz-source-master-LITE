# Guía de Efectos de Partículas Disponibles

## Resumen

Este documento describe los efectos de partículas disponibles basados en los recursos encontrados en `SFX/`.

**Posición de referencia del personaje**: `4188, -3079, -429`

---

## Recursos de Texturas Disponibles

### Texturas de Efectos
- `sfx/water_splash.bmp` - Salpicaduras de agua (útil para nieve, lluvia, burbujas)
- `SFX/smoke01.tga` - Humo (hasta smoke04.tga)
- `SFX/blood01.tga` - Sangre (hasta blood05.tga)
- `SFX/gz_sfx_tracer.bmp` - Rastro de bala (útil para chispas, partículas mágicas)
- `SFX/muzzle_smoke01.tga` - Humo de cañón
- `SFX/gz_sfx_mf*.bmp` - Efectos de fuego de armas

---

## Efectos Implementados

### 1. **Nieve** (`snow_main`)
- **Tipo**: Box emitter
- **Uso**: Mapas de nieve
- **Textura**: `sfx/water_splash.bmp`
- **Características**: Partículas grandes que caen lentamente
- **Habilitado por defecto**: ✅

### 2. **Lluvia** (`rain_main`)
- **Tipo**: Box emitter
- **Uso**: Ambientes lluviosos
- **Textura**: `sfx/water_splash.bmp`
- **Características**: Partículas pequeñas que caen rápido
- **Habilitado por defecto**: ❌

### 3. **Humo Ambiental** (`ambient_smoke_1`)
- **Tipo**: Point emitter
- **Uso**: Fogatas, incendios, ambientes industriales
- **Textura**: `SFX/smoke01.tga`
- **Posición**: Cerca del personaje (4188, -3079, -200)
- **Habilitado por defecto**: ❌

### 4. **Chispas** (`sparks_1`)
- **Tipo**: Point emitter
- **Uso**: Fuegos, forjas, explosiones
- **Texture**: `SFX/gz_sfx_tracer.bmp`
- **Características**: Partículas pequeñas con movimiento rápido
- **Habilitado por defecto**: ❌

### 5. **Polvo/Arena** (`dust_wind`)
- **Tipo**: Box emitter
- **Uso**: Ambientes desérticos, viento
- **Textura**: `sfx/water_splash.bmp`
- **Características**: Partículas medianas con movimiento horizontal
- **Habilitado por defecto**: ❌

### 6. **Cenizas** (`ash_fall`)
- **Tipo**: Box emitter
- **Uso**: Post-explosión, ambientes destruidos
- **Textura**: `SFX/smoke01.tga`
- **Características**: Partículas que caen lentamente
- **Habilitado por defecto**: ❌

### 7. **Burbujas** (`water_bubbles`)
- **Tipo**: Box emitter
- **Uso**: Áreas acuáticas, lagos, ríos
- **Textura**: `sfx/water_splash.bmp`
- **Características**: Partículas que suben (velocidad Z positiva)
- **Habilitado por defecto**: ❌

### 8. **Hojas Cayendo** (`falling_leaves`)
- **Tipo**: Box emitter
- **Uso**: Ambientes naturales, otoño
- **Textura**: `sfx/water_splash.bmp`
- **Características**: Partículas medianas con caída lenta
- **Habilitado por defecto**: ❌

### 9. **Neblina Baja** (`ground_fog`)
- **Tipo**: Box emitter
- **Uso**: Efectos atmosféricos, ambientes misteriosos
- **Textura**: `SFX/smoke01.tga`
- **Características**: Partículas grandes con movimiento lento
- **Habilitado por defecto**: ❌

### 10. **Partículas Mágicas** (`magic_particles`)
- **Tipo**: Sphere emitter
- **Uso**: Efectos mágicos, energía, ambientes fantásticos
- **Textura**: `SFX/gz_sfx_tracer.bmp`
- **Posición**: Esfera alrededor del personaje
- **Habilitado por defecto**: ❌

### 11. **Humo de Chimenea** (`chimney_smoke`)
- **Tipo**: Point emitter
- **Uso**: Casas, edificios, ambientes urbanos
- **Textura**: `SFX/smoke01.tga`
- **Características**: Emisión vertical constante
- **Habilitado por defecto**: ❌

### 12. **Polvo de Explosión** (`explosion_dust`)
- **Tipo**: Sphere emitter
- **Uso**: Impactos, explosiones, efectos de combate
- **Textura**: `SFX/smoke01.tga`
- **Posición**: En la posición del personaje
- **Habilitado por defecto**: ❌

---

## Cómo Usar

### **Habilitar un Efecto**

Edita el archivo `[MapName]_particles.xml` y cambia `enabled="false"` a `enabled="true"`:

```xml
<EMITTER name="rain_main" type="box" enabled="true">
    <!-- ... -->
</EMITTER>
```

### **Ajustar Posición**

Para efectos point/sphere, ajusta la posición según el mapa:

```xml
<POSITION x="4188" y="-3079" z="-200" />
```

### **Ajustar Parámetros**

- **SPAWN_RATE**: Partículas por segundo (más = más denso)
- **SIZE**: Tamaño de las partículas
- **VELOCITY_MIN/MAX**: Rango de velocidad
- **ACCELERATION**: Aceleración (gravedad, viento, etc.)
- **LIFETIME**: Tiempo de vida en segundos

---

## Combinaciones Recomendadas

### **Mapa de Nieve**
```xml
- snow_main (habilitado)
- ambient_smoke_1 (opcional, para chimeneas)
```

### **Mapa Lluvioso**
```xml
- rain_main (habilitado)
- water_bubbles (si hay agua)
```

### **Ambiente Desértico**
```xml
- dust_wind (habilitado)
- ambient_smoke_1 (para efectos de calor)
```

### **Ambiente Mágico/Fantástico**
```xml
- magic_particles (habilitado)
- ground_fog (opcional)
```

### **Ambiente Urbano**
```xml
- chimney_smoke (habilitado, en posiciones de edificios)
- ambient_smoke_1 (para fogatas)
```

---

## Notas de Rendimiento

- **SPAWN_RATE alto** (>500) puede afectar el rendimiento
- **Múltiples emisores activos** simultáneamente pueden reducir FPS
- **Efectos grandes** (box emitters grandes) generan más partículas
- **LIFETIME largo** mantiene más partículas en memoria

### **Optimización**

Para mejor rendimiento:
1. Usa `SPAWN_RATE` más bajo
2. Reduce el `SIZE` de los emisores box
3. Usa `LIFETIME` más corto
4. Deshabilita efectos que no se ven

---

## Ejemplo de Configuración por Mapa

### **Snow_Town_particles.xml**
```xml
<?xml version="1.0" encoding="UTF-8" ?>
<XML id="PARTICLESYSTEM">
    <EMITTERLIST>
        <EMITTER name="snow_main" type="box" enabled="true">
            <!-- Configuración de nieve -->
        </EMITTER>
    </EMITTERLIST>
    
    <COLLISIONLIST>
        <AUTO_MAP_COLLISION>
            true
            <BOUNCE>0.0</BOUNCE>
            <FRICTION>1.0</FRICTION>
            <KILL_ON_COLLISION>true</KILL_ON_COLLISION>
        </AUTO_MAP_COLLISION>
    </COLLISIONLIST>
</XML>
```

---

## Próximos Pasos

1. **Copiar** `PARTICLE_EFFECTS_EXAMPLES.xml` a `maps/[MapName]/[MapName]_particles.xml`
2. **Habilitar** los efectos deseados (`enabled="true"`)
3. **Ajustar** posiciones según el mapa
4. **Probar** y ajustar parámetros según necesidad

---

## Recursos Adicionales

Si necesitas más texturas, puedes:
- Usar otras texturas de `SFX/` disponibles
- Crear nuevas texturas para efectos personalizados
- Reutilizar texturas existentes con diferentes parámetros

---

¡Disfruta creando efectos atmosféricos increíbles! 🎉

