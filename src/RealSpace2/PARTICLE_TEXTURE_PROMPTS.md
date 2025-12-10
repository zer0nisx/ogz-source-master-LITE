# Prompts para Generar Texturas de Partículas con IA

## Estilo General: Anime Neon

**Base Style**: Anime art style with neon cyberpunk aesthetic, glowing particles, vibrant colors, high contrast, smooth gradients, translucent effects, 2D sprite texture suitable for point sprites in 3D game engine.

---

## 📐 Especificaciones Técnicas Comunes

- **Formato**: PNG con canal Alpha (transparencia)
- **Tamaño recomendado**: 64x64, 128x128, o 256x256 píxeles
- **UV Mapping**: Textura cuadrada, centro del UV (0.5, 0.5) es el centro de la partícula
- **Alpha Channel**: Gradiente desde centro (opaco) hacia bordes (transparente)
- **Color Space**: RGB con Alpha
- **Background**: Transparente (alpha = 0)

---

## 🌊 EFECTOS DE AGUA

### 1. **NIEVE - Partícula Individual**
```
PROMPT:
"Anime neon style snowflake particle, single snowflake crystal, glowing white and cyan edges, 
translucent center with soft glow, hexagonal or star shape, delicate ice crystal structure, 
neon blue-white gradient, outer edges fade to transparent, high contrast, 
centered on transparent background, 64x64 pixels, PNG with alpha channel, 
suitable for point sprite particle system, soft edges, no hard borders"

ESPECIFICACIONES UV:
- Centro (0.5, 0.5): Opaco blanco/cyan brillante
- Radio 0.3-0.4: Gradiente a semi-transparente
- Radio 0.5+: Completamente transparente
- Forma: Hexágono o estrella de 6 puntas
- Brillo: Efecto neon en los bordes

COLORES:
- Centro: RGB(255, 255, 255) - Blanco puro
- Medio: RGB(200, 240, 255) - Cyan claro
- Bordes: RGB(150, 220, 255) - Cyan neon
- Alpha: 255 (centro) → 128 (medio) → 0 (bordes)
```

### 2. **LLUVIA - Gota de Agua**
```
PROMPT:
"Anime neon style raindrop particle, single water droplet, teardrop shape, 
glowing cyan and blue edges, translucent center, neon blue-white gradient, 
smooth rounded teardrop form, outer edges fade to transparent, 
high contrast, centered on transparent background, 32x64 pixels (vertical), 
PNG with alpha channel, suitable for point sprite particle system, 
soft glow effect, no hard borders"

ESPECIFICACIONES UV:
- Centro (0.5, 0.3): Opaco cyan brillante
- Cuerpo: Gradiente vertical a transparente
- Punta inferior: Más opaco
- Forma: Lágrima vertical
- Brillo: Efecto neon en el borde superior

COLORES:
- Centro: RGB(150, 220, 255) - Cyan neon
- Medio: RGB(100, 180, 255) - Azul cyan
- Bordes: RGB(50, 150, 255) - Azul brillante
- Alpha: 255 (centro) → 100 (medio) → 0 (bordes)
```

### 3. **BURBUJA - Burbuja de Agua**
```
PROMPT:
"Anime neon style water bubble particle, perfect circle bubble, 
translucent sphere with neon cyan rim, glowing edges, 
subtle rainbow refraction effect on edges, soft gradient from center to edges, 
outer rim glows cyan-blue, centered on transparent background, 
64x64 pixels, PNG with alpha channel, suitable for point sprite particle system, 
soft edges, bubble highlight on top-left, no hard borders"

ESPECIFICACIONES UV:
- Centro (0.5, 0.5): Semi-transparente (alpha ~100)
- Radio 0.4: Borde brillante cyan
- Radio 0.5: Completamente transparente
- Highlight: Pequeño punto brillante en (0.3, 0.3)
- Forma: Círculo perfecto

COLORES:
- Centro: RGB(200, 240, 255) con alpha 100
- Borde: RGB(100, 200, 255) - Cyan neon brillante
- Highlight: RGB(255, 255, 255) - Blanco puro
- Alpha: 100 (centro) → 255 (borde) → 0 (exterior)
```

### 4. **SALPICADURA DE AGUA**
```
PROMPT:
"Anime neon style water splash particle, burst of water droplets, 
multiple small droplets radiating outward, glowing cyan and blue, 
translucent water effect, neon blue-white gradient, 
explosive splash pattern, outer edges fade to transparent, 
high contrast, centered on transparent background, 128x128 pixels, 
PNG with alpha channel, suitable for point sprite particle system, 
dynamic motion blur effect, no hard borders"

ESPECIFICACIONES UV:
- Centro (0.5, 0.5): Opaco blanco/cyan
- Múltiples gotas: Radiando desde el centro
- Forma: Explosión estelar
- Brillo: Efecto neon en cada gota

COLORES:
- Centro: RGB(255, 255, 255) - Blanco
- Gotas: RGB(150, 220, 255) - Cyan neon
- Alpha: 255 (centro) → 0 (bordes)
```

---

## 💨 EFECTOS DE HUMO

### 5. **HUMO AMBIENTAL (smoke01)**
```
PROMPT:
"Anime neon style smoke particle, soft wispy cloud, 
translucent gray-white with neon purple and blue accents, 
irregular organic cloud shape, glowing edges, 
soft gradient from dense center to transparent edges, 
outer edges fade to transparent, high contrast, 
centered on transparent background, 128x128 pixels, 
PNG with alpha channel, suitable for point sprite particle system, 
soft fluffy edges, no hard borders, ethereal glow effect"

ESPECIFICACIONES UV:
- Centro (0.5, 0.5): Semi-opaco gris (alpha ~200)
- Radio 0.3: Denso
- Radio 0.5: Completamente transparente
- Forma: Nube orgánica irregular
- Brillo: Acentos púrpura/azul neon en bordes

COLORES:
- Centro: RGB(180, 180, 200) con alpha 200
- Medio: RGB(150, 150, 220) - Gris-azul
- Bordes: RGB(200, 150, 255) - Púrpura neon
- Alpha: 200 (centro) → 100 (medio) → 0 (bordes)
```

### 6. **HUMO DE CAÑÓN (muzzle_smoke01)**
```
PROMPT:
"Anime neon style gun muzzle smoke particle, dense smoke cloud, 
translucent gray-black with neon orange and yellow fire accents, 
thick billowing smoke, glowing orange edges from fire, 
dense center fading to transparent edges, 
explosive smoke burst pattern, outer edges fade to transparent, 
high contrast, centered on transparent background, 128x128 pixels, 
PNG with alpha channel, suitable for point sprite particle system, 
dynamic motion, no hard borders, fire glow effect"

ESPECIFICACIONES UV:
- Centro (0.5, 0.5): Opaco gris oscuro (alpha ~255)
- Radio 0.2: Muy denso con acentos naranja
- Radio 0.5: Transparente
- Forma: Nube densa con "dedos" de humo
- Brillo: Naranja/amarillo neon en el centro (fuego)

COLORES:
- Centro: RGB(80, 80, 90) con alpha 255
- Fuego: RGB(255, 150, 50) - Naranja neon
- Humo: RGB(120, 120, 140) - Gris
- Bordes: RGB(200, 200, 220) - Gris claro
- Alpha: 255 (centro) → 150 (medio) → 0 (bordes)
```

### 7. **HUMO DE COHETE (smoke_rocket)**
```
PROMPT:
"Anime neon style rocket smoke trail particle, long trailing smoke cloud, 
translucent gray-white with neon blue and cyan trail, 
elongated cloud shape, glowing cyan edges, 
soft gradient from dense center to transparent edges, 
trail effect with motion blur, outer edges fade to transparent, 
high contrast, centered on transparent background, 128x256 pixels (vertical), 
PNG with alpha channel, suitable for point sprite particle system, 
dynamic trailing effect, no hard borders, neon trail glow"

ESPECIFICACIONES UV:
- Centro (0.5, 0.2): Opaco gris (parte superior)
- Cuerpo: Gradiente vertical a transparente
- Forma: Nube alargada vertical
- Brillo: Cyan neon en los bordes

COLORES:
- Superior: RGB(200, 200, 220) - Gris claro
- Medio: RGB(150, 200, 255) - Cyan
- Inferior: RGB(100, 180, 255) - Azul cyan
- Alpha: 255 (superior) → 100 (medio) → 0 (inferior)
```

### 8. **HUMO DE METEORO (ef_methor_smoke)**
```
PROMPT:
"Anime neon style meteor smoke trail particle, massive smoke cloud, 
translucent dark gray with neon red and orange fire accents, 
large billowing cloud, glowing red-orange fire in center, 
dense dark smoke fading to transparent edges, 
explosive smoke pattern, outer edges fade to transparent, 
high contrast, centered on transparent background, 256x256 pixels, 
PNG with alpha channel, suitable for point sprite particle system, 
epic scale, no hard borders, intense fire glow effect"

ESPECIFICACIONES UV:
- Centro (0.5, 0.5): Opaco gris muy oscuro (alpha ~255)
- Radio 0.3: Fuego rojo-naranja brillante
- Radio 0.5: Transparente
- Forma: Nube masiva con fuego central
- Brillo: Rojo/naranja neon intenso

COLORES:
- Fuego centro: RGB(255, 100, 50) - Rojo-naranja neon
- Humo denso: RGB(60, 60, 70) - Gris muy oscuro
- Humo medio: RGB(100, 100, 120) - Gris
- Bordes: RGB(150, 150, 180) - Gris claro
- Alpha: 255 (centro) → 200 (medio) → 0 (bordes)
```

---

## 🔥 EFECTOS DE FUEGO/CHISPAS

### 9. **CHISPA (gz_sfx_tracer)**
```
PROMPT:
"Anime neon style spark particle, bright glowing spark, 
intense white-yellow-orange gradient, elongated teardrop shape, 
glowing neon edges, bright center fading to transparent tail, 
high contrast, centered on transparent background, 32x64 pixels (vertical), 
PNG with alpha channel, suitable for point sprite particle system, 
intense glow, no hard borders, electric spark effect"

ESPECIFICACIONES UV:
- Centro (0.5, 0.2): Opaco blanco/amarillo brillante
- Cuerpo: Gradiente a naranja
- Cola: Transparente
- Forma: Lágrima vertical con cola
- Brillo: Efecto neon intenso

COLORES:
- Centro: RGB(255, 255, 200) - Amarillo-blanco
- Medio: RGB(255, 200, 100) - Naranja
- Cola: RGB(255, 150, 50) - Naranja oscuro
- Alpha: 255 (centro) → 150 (medio) → 0 (cola)
```

### 10. **FUEGO DE RIFLE (gz_sfx_mf01)**
```
PROMPT:
"Anime neon style rifle muzzle flash particle, bright fire burst, 
intense white-yellow-orange-red gradient, explosive flame shape, 
glowing neon edges, bright center with flame tongues, 
dense fire center fading to transparent edges, 
explosive burst pattern, outer edges fade to transparent, 
high contrast, centered on transparent background, 128x128 pixels, 
PNG with alpha channel, suitable for point sprite particle system, 
intense fire glow, no hard borders, dynamic flame effect"

ESPECIFICACIONES UV:
- Centro (0.5, 0.5): Opaco blanco/amarillo
- Radio 0.3: Fuego naranja-rojo
- Radio 0.5: Transparente
- Forma: Explosión de fuego con lenguas
- Brillo: Amarillo/naranja neon intenso

COLORES:
- Centro: RGB(255, 255, 200) - Amarillo-blanco
- Fuego: RGB(255, 150, 50) - Naranja
- Bordes: RGB(255, 100, 0) - Rojo-naranja
- Alpha: 255 (centro) → 150 (medio) → 0 (bordes)
```

### 11. **FUEGO DE PISTOLA (gz_sfx_mf21)**
```
PROMPT:
"Anime neon style pistol muzzle flash particle, smaller fire burst, 
intense white-yellow-orange gradient, compact flame shape, 
glowing neon edges, bright center with smaller flame tongues, 
dense fire center fading to transparent edges, 
compact burst pattern, outer edges fade to transparent, 
high contrast, centered on transparent background, 96x96 pixels, 
PNG with alpha channel, suitable for point sprite particle system, 
intense fire glow, no hard borders, compact flame effect"

ESPECIFICACIONES UV:
- Similar a rifle pero más pequeño
- Centro más compacto
- Mismo esquema de colores pero escala reducida

COLORES:
- Mismo esquema que rifle pero más intenso en el centro
```

### 12. **FUEGO DE ESCOPETA (gz_sfx_mf41)**
```
PROMPT:
"Anime neon style shotgun muzzle flash particle, massive fire burst, 
intense white-yellow-orange-red gradient, large explosive flame shape, 
glowing neon edges, very bright center with large flame tongues, 
dense fire center fading to transparent edges, 
massive explosive burst pattern, outer edges fade to transparent, 
high contrast, centered on transparent background, 192x192 pixels, 
PNG with alpha channel, suitable for point sprite particle system, 
very intense fire glow, no hard borders, massive flame effect"

ESPECIFICACIONES UV:
- Similar a rifle pero más grande
- Centro más masivo
- Lenguas de fuego más grandes

COLORES:
- Mismo esquema que rifle pero escala aumentada
```

---

## 🩸 EFECTOS DE SANGRE

### 13. **SANGRE (blood01)**
```
PROMPT:
"Anime neon style blood particle, bright red blood droplet, 
intense red-crimson gradient with neon pink accents, 
teardrop or splatter shape, glowing pink edges, 
dense red center fading to transparent edges, 
high contrast, centered on transparent background, 64x64 pixels, 
PNG with alpha channel, suitable for point sprite particle system, 
vibrant neon effect, no hard borders, liquid blood effect"

ESPECIFICACIONES UV:
- Centro (0.5, 0.5): Opaco rojo intenso (alpha ~255)
- Radio 0.4: Gradiente a rosa
- Radio 0.5: Transparente
- Forma: Gota o salpicadura
- Brillo: Rosa neon en los bordes

COLORES:
- Centro: RGB(200, 0, 0) - Rojo intenso
- Medio: RGB(255, 50, 50) - Rojo brillante
- Bordes: RGB(255, 100, 150) - Rosa neon
- Alpha: 255 (centro) → 150 (medio) → 0 (bordes)
```

### 14. **SALPICADURA DE SANGRE (blood02-05)**
```
PROMPT:
"Anime neon style blood splatter particle, multiple blood droplets, 
intense red-crimson gradient with neon pink accents, 
explosive splatter pattern, glowing pink edges on each droplet, 
dense red center fading to transparent edges, 
multiple droplets radiating outward, outer edges fade to transparent, 
high contrast, centered on transparent background, 128x128 pixels, 
PNG with alpha channel, suitable for point sprite particle system, 
vibrant neon effect, no hard borders, dynamic splatter effect"

ESPECIFICACIONES UV:
- Centro (0.5, 0.5): Opaco rojo
- Múltiples gotas: Radiando desde el centro
- Forma: Explosión de salpicadura
- Brillo: Rosa neon en cada gota

COLORES:
- Similar a blood01 pero con múltiples gotas
```

---

## ✨ EFECTOS ESPECIALES

### 15. **ANILLO DE EFECTO (gd_effect_001)**
```
PROMPT:
"Anime neon style energy ring particle, glowing circular ring, 
bright cyan-blue-purple gradient, perfect circle with neon glow, 
translucent center, bright glowing rim, 
soft gradient from rim to transparent center and edges, 
high contrast, centered on transparent background, 128x128 pixels, 
PNG with alpha channel, suitable for point sprite particle system, 
intense neon glow, no hard borders, magical energy ring effect"

ESPECIFICACIONES UV:
- Centro (0.5, 0.5): Transparente
- Radio 0.35-0.45: Anillo brillante cyan
- Interior/Exterior: Transparente
- Forma: Anillo perfecto
- Brillo: Cyan/púrpura neon intenso

COLORES:
- Anillo: RGB(100, 200, 255) - Cyan neon
- Gradiente: RGB(150, 100, 255) - Púrpura
- Alpha: 0 (centro) → 255 (anillo) → 0 (exterior)
```

### 16. **EFECTO DE DASH (gz_effect_dash01)**
```
PROMPT:
"Anime neon style dash speed line particle, motion blur effect, 
bright cyan-blue-white gradient, elongated horizontal streaks, 
glowing neon edges, bright center with speed lines, 
dense center fading to transparent edges, 
horizontal motion blur pattern, outer edges fade to transparent, 
high contrast, centered on transparent background, 256x128 pixels (horizontal), 
PNG with alpha channel, suitable for point sprite particle system, 
intense neon glow, no hard borders, speed effect"

ESPECIFICACIONES UV:
- Centro (0.5, 0.5): Opaco cyan
- Forma: Líneas horizontales alargadas
- Brillo: Cyan/blanco neon

COLORES:
- Centro: RGB(200, 240, 255) - Cyan claro
- Líneas: RGB(100, 200, 255) - Cyan neon
- Alpha: 255 (centro) → 0 (bordes)
```

### 17. **HUELLA/PISADA (ef_gz_footstep)**
```
PROMPT:
"Anime neon style footprint particle, glowing footprint mark, 
bright cyan-blue gradient, footprint shape, 
glowing neon edges, translucent center, 
soft gradient from edges to transparent center, 
high contrast, centered on transparent background, 128x128 pixels, 
PNG with alpha channel, suitable for point sprite particle system, 
soft neon glow, no hard borders, magical footprint effect"

ESPECIFICACIONES UV:
- Forma: Huella de pie
- Bordes: Brillante cyan
- Centro: Semi-transparente
- Brillo: Cyan neon en el contorno

COLORES:
- Contorno: RGB(100, 200, 255) - Cyan neon
- Centro: RGB(150, 220, 255) con alpha 100
- Alpha: 255 (contorno) → 100 (centro) → 0 (exterior)
```

### 18. **MISIL MÁGICO (ef_magicmissile)**
```
PROMPT:
"Anime neon style magic missile particle, glowing energy orb, 
bright purple-pink-blue gradient, perfect sphere with neon glow, 
translucent center, bright glowing rim, 
soft gradient from rim to transparent center, 
magical energy effect, high contrast, centered on transparent background, 
128x128 pixels, PNG with alpha channel, suitable for point sprite particle system, 
intense neon glow, no hard borders, magical energy orb effect"

ESPECIFICACIONES UV:
- Centro (0.5, 0.5): Semi-transparente púrpura
- Radio 0.4: Borde brillante púrpura/rosa
- Radio 0.5: Transparente
- Forma: Esfera perfecta
- Brillo: Púrpura/rosa/azul neon

COLORES:
- Centro: RGB(200, 100, 255) con alpha 150 - Púrpura
- Borde: RGB(255, 100, 200) - Rosa neon
- Exterior: RGB(100, 150, 255) - Azul neon
- Alpha: 150 (centro) → 255 (borde) → 0 (exterior)
```

### 19. **EFECTO GENÉRICO (gz_effect004)**
```
PROMPT:
"Anime neon style generic energy particle, glowing energy burst, 
bright cyan-purple-pink gradient, organic cloud-like shape, 
glowing neon edges, translucent center, 
soft gradient from center to transparent edges, 
magical energy effect, high contrast, centered on transparent background, 
128x128 pixels, PNG with alpha channel, suitable for point sprite particle system, 
intense neon glow, no hard borders, versatile energy effect"

ESPECIFICACIONES UV:
- Centro (0.5, 0.5): Semi-transparente
- Forma: Nube orgánica
- Brillo: Múltiples colores neon

COLORES:
- Centro: RGB(200, 150, 255) - Púrpura
- Medio: RGB(150, 200, 255) - Cyan
- Bordes: RGB(255, 100, 200) - Rosa
- Alpha: 200 (centro) → 100 (medio) → 0 (bordes)
```

### 20. **MARCA DE BALA (bulletmark01)**
```
PROMPT:
"Anime neon style bullet impact mark, glowing impact crater, 
bright orange-red-yellow gradient, circular impact mark, 
glowing neon edges, translucent center with impact pattern, 
soft gradient from center to transparent edges, 
high contrast, centered on transparent background, 64x64 pixels, 
PNG with alpha channel, suitable for point sprite particle system, 
intense neon glow, no hard borders, impact mark effect"

ESPECIFICACIONES UV:
- Centro (0.5, 0.5): Opaco naranja
- Forma: Círculo con patrón de impacto
- Brillo: Naranja/rojo neon

COLORES:
- Centro: RGB(255, 150, 50) - Naranja
- Medio: RGB(255, 100, 0) - Rojo-naranja
- Bordes: RGB(255, 200, 100) - Amarillo-naranja
- Alpha: 255 (centro) → 150 (medio) → 0 (bordes)
```

### 21. **EFECTO ANIMADO (gd_effect_019-021)**
```
PROMPT:
"Anime neon style animated energy particle, glowing energy burst, 
bright multi-color gradient (cyan-purple-pink-yellow), 
organic cloud-like shape with motion, glowing neon edges, 
translucent center, soft gradient from center to transparent edges, 
magical animated energy effect, high contrast, centered on transparent background, 
128x128 pixels, PNG with alpha channel, suitable for point sprite particle system, 
intense neon glow, no hard borders, animated energy effect"

ESPECIFICACIONES UV:
- Similar a efecto genérico pero con más colores
- Forma más dinámica
- Múltiples colores neon mezclados

COLORES:
- Múltiples colores neon mezclados
- Cyan, púrpura, rosa, amarillo
```

---

## 🎨 **Guía de Colores Neon Anime**

### **Paleta de Colores Base**

| Color | RGB | Uso |
|-------|-----|-----|
| **Cyan Neon** | RGB(100, 200, 255) | Agua, hielo, energía fría |
| **Púrpura Neon** | RGB(200, 100, 255) | Magia, energía mística |
| **Rosa Neon** | RGB(255, 100, 200) | Magia, efectos especiales |
| **Naranja Neon** | RGB(255, 150, 50) | Fuego, explosiones |
| **Rojo Neon** | RGB(255, 50, 50) | Fuego, sangre |
| **Amarillo Neon** | RGB(255, 255, 100) | Fuego, energía |
| **Blanco Brillante** | RGB(255, 255, 255) | Centros de efectos |

---

## 📏 **Especificaciones UV Detalladas**

### **Patrón General de Alpha (Transparencia)**

```
Radio 0.0-0.2: Alpha 255 (Completamente opaco)
Radio 0.2-0.3: Alpha 200-255 (Muy opaco)
Radio 0.3-0.4: Alpha 100-200 (Semi-transparente)
Radio 0.4-0.5: Alpha 50-100 (Muy transparente)
Radio 0.5+:     Alpha 0 (Completamente transparente)
```

### **Patrón de Brillo Neon**

```
- Centro: Color base sólido
- Radio 0.3-0.4: Borde brillante (color neon +50% brillo)
- Radio 0.4-0.5: Gradiente a transparente
- Efecto: Glow suave de 2-3 píxeles alrededor del borde
```

---

## 🖼️ **Instrucciones para IA de Imagen**

### **Prompt Base (Agregar al inicio de cada prompt)**

```
"Anime art style, neon cyberpunk aesthetic, 2D sprite texture, 
transparent background, PNG format, high resolution, 
smooth gradients, glowing edges, vibrant colors, 
suitable for video game particle system, point sprite texture"
```

### **Parámetros Recomendados**

- **Aspect Ratio**: 1:1 (cuadrado) para la mayoría, 1:2 o 2:1 para gotas/líneas
- **Resolution**: 256x256, 128x128, o 64x64 píxeles
- **Style**: Anime, Neon, Cyberpunk
- **Background**: Transparente
- **Format**: PNG con Alpha Channel

### **Ejemplo Completo de Prompt**

```
"Anime neon style snowflake particle, single snowflake crystal, 
glowing white and cyan edges, translucent center with soft glow, 
hexagonal or star shape, delicate ice crystal structure, 
neon blue-white gradient, outer edges fade to transparent, 
high contrast, centered on transparent background, 64x64 pixels, 
PNG with alpha channel, suitable for point sprite particle system, 
soft edges, no hard borders, anime art style, neon cyberpunk aesthetic, 
2D sprite texture, transparent background, high resolution, 
smooth gradients, glowing edges, vibrant colors"
```

---

## ✅ **Checklist para Cada Textura**

- [ ] Formato PNG con Alpha
- [ ] Fondo transparente
- [ ] Gradiente desde centro opaco a bordes transparentes
- [ ] Efecto neon en los bordes
- [ ] Colores vibrantes y de alto contraste
- [ ] Sin bordes duros (soft edges)
- [ ] Tamaño apropiado (64x64, 128x128, o 256x256)
- [ ] Centro en (0.5, 0.5) del UV
- [ ] Estilo anime neon consistente

---

## 🎯 **Resumen por Categoría**

### **Agua (4 texturas)**
1. Nieve - Hexágono/estrella, blanco-cyan
2. Lluvia - Lágrima vertical, cyan
3. Burbuja - Círculo, cyan con highlight
4. Salpicadura - Explosión, cyan

### **Humo (8 texturas)**
5. Humo ambiental (4 variantes) - Nube, gris con acentos púrpura
6. Humo de cañón (4 variantes) - Nube densa, gris con fuego naranja
7. Humo de cohete - Nube alargada vertical, cyan
8. Humo de meteoro - Nube masiva, gris oscuro con fuego rojo

### **Fuego (18 texturas)**
9. Chispa - Lágrima, amarillo-naranja
10-17. Fuego de rifle (8 variantes) - Explosión, amarillo-naranja-rojo
18-23. Fuego de pistola (6 variantes) - Explosión pequeña, amarillo-naranja
24-27. Fuego de escopeta (4 variantes) - Explosión grande, amarillo-naranja-rojo

### **Sangre (5 texturas)**
28-32. Sangre (5 variantes) - Gota/salpicadura, rojo-rosa neon

### **Especiales (10 texturas)**
33-34. Anillo (2 variantes) - Círculo, cyan-púrpura
35-36. Dash (2 variantes) - Líneas horizontales, cyan
37. Huella - Forma de pie, cyan
38. Misil mágico - Esfera, púrpura-rosa-azul
39. Efecto genérico - Nube, multi-color
40-41. Marca de bala (2 variantes) - Impacto, naranja-rojo
42-44. Efecto animado (3 variantes) - Nube, multi-color

---

**Total: 45 texturas diferentes** 🎨

¡Usa estos prompts para generar todas las texturas que necesites! ✨





