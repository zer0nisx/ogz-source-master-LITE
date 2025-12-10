# Guía de Referencia de Texturas para Efectos de Partículas

## Resumen

Este documento lista **TODAS las texturas disponibles** y qué efecto usar para cada una.

---

## 📋 Lista Completa de Texturas

### 🌊 **EFECTOS DE AGUA**

| Textura | Efecto Recomendado | Tamaño Partícula | Spawn Rate |
|---------|-------------------|------------------|------------|
| `sfx/water_splash.bmp` | Nieve, Lluvia, Burbujas, Polvo, Hojas | 5-25 | 50-1000 |
| `SFX/gd_effect_006.tga` | Salpicaduras de agua | 20 | 30 |

**Uso**:
- **Nieve**: Partículas grandes (25.0), caída lenta
- **Lluvia**: Partículas pequeñas (5.0), caída rápida
- **Burbujas**: Partículas medianas (10.0), suben (velocidad Z positiva)
- **Polvo**: Partículas medianas (15.0), movimiento horizontal
- **Hojas**: Partículas pequeñas (12.0), caída lenta

---

### 💨 **EFECTOS DE HUMO**

| Textura | Efecto Recomendado | Tamaño Partícula | Spawn Rate |
|---------|-------------------|------------------|------------|
| `SFX/smoke01.tga` | Humo ambiental, Cenizas, Neblina | 20-40 | 15-150 |
| `SFX/smoke02.tga` | Humo ambiental (variante) | 35 | 15 |
| `SFX/smoke03.tga` | Humo ambiental (variante) | 32 | 18 |
| `SFX/smoke04.tga` | Humo ambiental (variante) | 28 | 22 |
| `SFX/muzzle_smoke01.tga` | Humo de cañón | 25 | 50 |
| `SFX/muzzle_smoke02.tga` | Humo de cañón (variante) | 27 | 45 |
| `SFX/muzzle_smoke03.tga` | Humo de cañón (variante) | 23 | 55 |
| `SFX/muzzle_smoke04.tga` | Humo de cañón (variante) | 29 | 40 |
| `SFX/muzzle_smoke41.tga` | Humo de escopeta | 30 | 60 |
| `SFX/muzzle_smoke42.tga` | Humo de escopeta (variante) | 32 | 55 |
| `SFX/smoke_rocket.tga` | Humo de cohete | 40 | 80 |
| `SFX/ef_methor_smoke.tga` | Humo de meteoro | 50 | 100 |

**Uso**:
- **Humo ambiental**: Emisión desde punto, movimiento vertical lento
- **Humo de cañón**: Emisión desde punto, movimiento rápido, vida corta
- **Cenizas**: Box emitter, caída lenta
- **Neblina**: Box emitter grande, movimiento muy lento

---

### 🔥 **EFECTOS DE FUEGO/CHISPAS**

| Textura | Efecto Recomendado | Tamaño Partícula | Spawn Rate |
|---------|-------------------|------------------|------------|
| `SFX/gz_sfx_tracer.bmp` | Chispas, Partículas mágicas | 6-8 | 60-100 |
| `SFX/gz_sfx_mf01.bmp` | Fuego de rifle | 15 | 120 |
| `SFX/gz_sfx_mf02.bmp` | Fuego de rifle (variante) | 16 | 115 |
| `SFX/gz_sfx_mf03.bmp` | Fuego de rifle (variante) | 14 | 125 |
| `SFX/gz_sfx_mf04.bmp` | Fuego de rifle (variante) | 17 | 110 |
| `SFX/gz_sfx_mf11.bmp` | Fuego de rifle (alt) | 15.5 | 118 |
| `SFX/gz_sfx_mf12.bmp` | Fuego de rifle (alt) | 16.5 | 113 |
| `SFX/gz_sfx_mf13.bmp` | Fuego de rifle (alt) | 14.5 | 123 |
| `SFX/gz_sfx_mf14.bmp` | Fuego de rifle (alt) | 17.5 | 108 |
| `SFX/gz_sfx_mf21.bmp` | Fuego de pistola | 12 | 100 |
| `SFX/gz_sfx_mf22.bmp` | Fuego de pistola (variante) | 13 | 95 |
| `SFX/gz_sfx_mf23.bmp` | Fuego de pistola (variante) | 11 | 105 |
| `SFX/gz_sfx_mf31.bmp` | Fuego de pistola (alt) | 12.5 | 98 |
| `SFX/gz_sfx_mf32.bmp` | Fuego de pistola (alt) | 13.5 | 93 |
| `SFX/gz_sfx_mf33.bmp` | Fuego de pistola (alt) | 11.5 | 103 |
| `SFX/gz_sfx_mf41.bmp` | Fuego de escopeta | 18 | 80 |
| `SFX/gz_sfx_mf42.bmp` | Fuego de escopeta (variante) | 19 | 75 |
| `SFX/gz_sfx_mf43.bmp` | Fuego de escopeta (variante) | 17 | 85 |
| `SFX/gz_sfx_mf44.bmp` | Fuego de escopeta (variante) | 20 | 70 |

**Uso**:
- **Chispas**: Partículas pequeñas, movimiento rápido, vida corta
- **Fuego de rifle**: Partículas medianas, movimiento rápido hacia adelante
- **Fuego de pistola**: Partículas más pequeñas, movimiento moderado
- **Fuego de escopeta**: Partículas grandes, movimiento rápido, dispersión amplia

---

### 🩸 **EFECTOS DE SANGRE**

| Textura | Efecto Recomendado | Tamaño Partícula | Spawn Rate |
|---------|-------------------|------------------|------------|
| `SFX/blood01.tga` | Sangre (spray) | 10 | 200 |
| `SFX/blood02.tga` | Sangre (spray variante) | 11 | 190 |
| `SFX/blood03.tga` | Sangre (spray variante) | 9 | 210 |
| `SFX/blood04.tga` | Sangre (spray variante) | 12 | 180 |
| `SFX/blood05.tga` | Sangre (spray variante) | 10.5 | 195 |

**Uso**:
- **Sangre**: Emisión desde punto (herida), movimiento rápido en todas direcciones
- **Gravedad fuerte**: Aceleración Z negativa alta (-30)
- **Vida corta**: 300 segundos máximo

---

### ✨ **EFECTOS ESPECIALES**

| Textura | Efecto Recomendado | Tamaño Partícula | Spawn Rate |
|---------|-------------------|------------------|------------|
| `SFX/gd_effect_001.tga` | Anillo de efecto | 20 | 40 |
| `SFX/gd_effect_002.tga` | Anillo de efecto (variante) | 22 | 38 |
| `SFX/gz_effect_dash01.tga` | Efecto de dash | 25 | 60 |
| `SFX/gz_effect_dash02.tga` | Efecto de dash (variante) | 27 | 55 |
| `SFX/ef_gz_footstep.tga` | Huella/pisada | 15 | 10 |
| `SFX/ef_sw.bmp` | Efecto de mundo/item | 12 | 30 |
| `SFX/gz_effect004.tga` | Efecto genérico | 18 | 50 |
| `SFX/gz_sfx_shotgun_bulletmark01.tga` | Marca de bala | 8 | 5 |
| `SFX/gz_sfx_shotgun_bulletmark02.tga` | Marca de bala (variante) | 9 | 5 |
| `SFX/gd_effect_019.bmp` | Efecto animado | 20 | 40 |
| `SFX/gd_effect_020.bmp` | Efecto animado (variante) | 22 | 38 |
| `SFX/gd_effect_021.bmp` | Efecto animado (variante) | 24 | 35 |
| `SFX/ef_magicmissile.bmp` | Misil mágico | 15 | 30 |

**Uso**:
- **Anillos**: Sphere emitter, expansión desde centro
- **Dash**: Point emitter, movimiento rápido horizontal
- **Huellas**: Point emitter, sin movimiento (quedan en el suelo)
- **Marcas de bala**: Point emitter, sin movimiento, vida larga
- **Efectos animados**: Point emitter, movimiento moderado

---

## 🎯 **Guía Rápida por Tipo de Efecto**

### **Efectos Ambientales (Box Emitter)**
- **Nieve**: `sfx/water_splash.bmp`, Size: 25, Spawn: 400
- **Lluvia**: `sfx/water_splash.bmp`, Size: 5, Spawn: 1000
- **Polvo**: `sfx/water_splash.bmp`, Size: 15, Spawn: 200
- **Cenizas**: `SFX/smoke01.tga`, Size: 20, Spawn: 150
- **Neblina**: `SFX/smoke01.tga`, Size: 40, Spawn: 30

### **Efectos desde Punto (Point Emitter)**
- **Humo**: `SFX/smoke01-04.tga`, Size: 30, Spawn: 15-22
- **Chispas**: `SFX/gz_sfx_tracer.bmp`, Size: 8, Spawn: 100
- **Fuego**: `SFX/gz_sfx_mf*.bmp`, Size: 12-20, Spawn: 70-125
- **Sangre**: `SFX/blood01-05.tga`, Size: 9-12, Spawn: 180-210

### **Efectos Esféricos (Sphere Emitter)**
- **Anillos**: `SFX/gd_effect_001-002.tga`, Size: 20-22, Spawn: 38-40
- **Partículas mágicas**: `SFX/gz_sfx_tracer.bmp`, Size: 6, Spawn: 60
- **Explosión**: `SFX/smoke01.tga`, Size: 25, Spawn: 200

---

## 📝 **Ejemplo de Configuración**

```xml
<!-- Ejemplo: Humo ambiental -->
<EMITTER name="smoke" type="point" enabled="true">
    <POSITION x="4188" y="-3079" z="-200" />
    <PARTICLES>
        <TEXTURE>SFX/smoke01.tga</TEXTURE>  <!-- ← TEXTURA AQUÍ -->
        <SIZE>30.0</SIZE>
        <SPAWN_RATE>20</SPAWN_RATE>
        <VELOCITY_MIN x="-10" y="-10" z="20" />
        <VELOCITY_MAX x="10" y="10" z="50" />
        <ACCELERATION x="0" y="0" z="-2" />
        <LIFETIME>800.0</LIFETIME>
    </PARTICLES>
</EMITTER>
```

---

## ⚠️ **Notas Importantes**

1. **Rutas de archivo**:
   - Algunas texturas usan `sfx/` (minúsculas)
   - Otras usan `SFX/` (mayúsculas)
   - **Usa exactamente la ruta que aparece en la tabla**

2. **Extensiones**:
   - `.bmp` - Bitmap
   - `.tga` - Targa

3. **Texturas no encontradas**:
   - Si una textura no existe, el efecto no se cargará
   - Verifica que el archivo exista en `SFX/` o `sfx/`

4. **Rendimiento**:
   - Spawn rate alto (>500) puede afectar FPS
   - Múltiples emisores activos simultáneamente reducen rendimiento

---

## 🎨 **Combinaciones Recomendadas**

### **Ambiente de Combate**
```xml
- muzzle_smoke_01 (habilitado)
- sparks_tracer (habilitado)
- rifle_fire_01 (habilitado)
```

### **Ambiente Mágico**
```xml
- magic_particles (habilitado)
- animated_effect_019 (habilitado)
- magic_missile (habilitado)
```

### **Ambiente de Nieve**
```xml
- snow_main (habilitado)
- chimney_smoke (habilitado, en posiciones de casas)
```

### **Ambiente de Lluvia**
```xml
- rain_main (habilitado)
- water_bubbles (habilitado, si hay agua)
```

---

## 📊 **Total de Efectos Disponibles**

- **60 efectos** configurados
- **25+ texturas** diferentes
- **5 tipos de emisores** (Point, Box, Sphere, Line, Cone)

---

¡Usa esta guía para crear los efectos que necesites! 🎉





