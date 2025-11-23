# Factores que Afectan al Shader Skin

## 📋 Resumen Ejecutivo

El shader skin está afectado por múltiples factores que determinan el resultado visual final. Este documento detalla cada componente y su influencia.

---

## 🎨 1. MATERIALES (Propiedades del Objeto)

### MaterialDiffuse (c13) - **CRÍTICO** ✅ USADO
- **Origen**: `RShaderMgr::setMtrl()` o `SetCharacterMtrl_ON()`
- **Valores por defecto**:
  - RGB: `(0.5, 0.5, 0.5)` cuando se usa `setMtrl(RMtrl*)`
  - RGB: Color del material cuando se usa `setMtrl(color_r32&)`
  - Alpha: `fVisAlpha_` (visibilidad/transparencia)
- **Efecto**: Modula el color final multiplicando la iluminación
- **Fórmula**: `oDiffuse *= MaterialDiffuse`
- **Impacto**: ⭐⭐⭐⭐⭐ (Muy Alto) - Controla el color base del objeto

### MaterialAmbient (c12) - **IMPORTANTE** ✅ USADO
- **Origen**: `RShaderMgr::setMtrl()`
- **Valores por defecto**:
  - RGB: `(0.35, 0.35, 0.35)` cuando se usa `setMtrl(RMtrl*)`
  - RGB: `Color * 0.2` cuando se usa `setMtrl(color_r32&)`
- **Efecto**: Controla cuánta iluminación ambiente afecta al objeto
- **Fórmula**: `oDiffuse += (Light0Ambient + Light1Ambient + GlobalAmbient) * MaterialAmbient`
- **Impacto**: ⭐⭐⭐⭐ (Alto) - Afecta el brillo base en áreas sin luz directa

### MaterialSpecular (c14) - ❌ NO USADO
- **Estado**: Se envía al shader pero no se calcula
- **Impacto**: ⭐ (Ninguno) - No afecta el resultado visual

### MaterialPower (c15) - ❌ NO USADO
- **Estado**: Se envía al shader pero no se usa
- **Impacto**: ⭐ (Ninguno) - No afecta el resultado visual

---

## 💡 2. ILUMINACIÓN

### GlobalAmbient (c16) - **IMPORTANTE** ✅ USADO
- **Origen**: `RShaderMgr::setAmbient(u32 value_)`
- **Formato**: Color RGB (32-bit)
- **Efecto**: Iluminación ambiente global que afecta a todos los objetos
- **Fórmula**: Se suma a la iluminación ambiente total
- **Impacto**: ⭐⭐⭐⭐ (Alto) - Afecta el brillo general de la escena

### Light0 y Light1 - **CRÍTICO** ✅ USADO
Cada luz tiene múltiples propiedades:

#### Light Position (c17, c22) - **CRÍTICO**
- **Origen**: `RShaderMgr::setLight(int, D3DLIGHT9*)`
- **Efecto**: Posición de la luz en espacio de mundo
- **Impacto**: ⭐⭐⭐⭐⭐ (Muy Alto) - Determina dirección y distancia de iluminación

#### Light Diffuse (c19, c24) - **CRÍTICO**
- **Origen**: `D3DLIGHT9.Diffuse`
- **Efecto**: Color e intensidad de la luz difusa
- **Fórmula**: `LightDiffuse * diffuseFactor`
- **Impacto**: ⭐⭐⭐⭐⭐ (Muy Alto) - Color principal de la iluminación

#### Light Ambient (c18, c23) - **IMPORTANTE**
- **Origen**: `D3DLIGHT9.Ambient`
- **Efecto**: Iluminación ambiente de cada luz
- **Fórmula**: Se suma a la iluminación ambiente total
- **Impacto**: ⭐⭐⭐ (Medio) - Afecta áreas no directamente iluminadas

#### Light Attenuation (c27, c28) - **CRÍTICO**
- **Origen**: `D3DLIGHT9.Attenuation0/1/2`
- **Efecto**: Controla cómo la luz se atenúa con la distancia
- **Fórmula**: `1.0 / (Attenuation0 + Attenuation1 * dist + Attenuation2 * dist²)`
- **Valores por defecto**: `(0.1, 0.1, 0.1)`
- **Impacto**: ⭐⭐⭐⭐⭐ (Muy Alto) - Controla el alcance y suavidad de la luz

#### Light Specular (c20, c25) - ❌ NO USADO
- **Estado**: Se envía pero no se calcula
- **Impacto**: ⭐ (Ninguno)

#### Light Range (c21, c26) - ❌ NO USADO
- **Estado**: Se envía pero no se valida en el shader
- **Impacto**: ⭐ (Ninguno) - No se hace culling de luces fuera de rango

#### Estado de Luces
- **Origen**: `RShaderMgr::LightEnable(int, bool)`
- **Efecto**: Activa/desactiva cada luz
- **Impacto**: ⭐⭐⭐⭐⭐ (Muy Alto) - Si está desactivada, no ilumina

---

## 🌫️ 3. FOG (Niebla)

### FogStart y FogEnd (Constants.y, Constants.z) - **FUNCIONAL** ✅ USADO
- **Origen**: BSP → `ZWorld::Create()` → `RSetFog()` → `RGetFogNear()/Far()`
- **Efecto**: Controla la distancia donde empieza y termina el fog
- **Fórmula**: `fogFactor = (FogEnd - dist) / (FogEnd - FogStart)`
- **Impacto**: ⭐⭐⭐⭐ (Alto) - Afecta la visibilidad de objetos lejanos

### FogEnabled (Constants.w) - **FUNCIONAL** ✅ USADO
- **Origen**: `RGetFog()` verifica si el fog está activo
- **Efecto**: Si es 0, no hay fog (retorna 1.0)
- **Impacto**: ⭐⭐⭐⭐ (Alto) - Activa/desactiva el fog

### CameraPosition (c11) - **FUNCIONAL** ✅ USADO
- **Origen**: `RCameraPosition` (variable global)
- **Efecto**: Usado para calcular distancia al vértice para fog
- **Fórmula**: `distToCamera = length(TransformedPos - CameraPosition)`
- **Impacto**: ⭐⭐⭐⭐ (Alto) - Necesario para calcular fog correctamente

---

## 🎭 4. TRANSFORMACIONES Y ANIMACIÓN

### World Matrix (c3-c5) - **CRÍTICO** ✅ USADO
- **Origen**: `RMeshNode::RenderNodeVS()` - Transformación del objeto
- **Efecto**: Transforma vértices de espacio local a espacio de mundo
- **Impacto**: ⭐⭐⭐⭐⭐ (Muy Alto) - Posición y orientación del objeto

### ViewProjection Matrix (c6-c9) - **CRÍTICO** ✅ USADO
- **Origen**: `view * proj` desde transformaciones de DirectX
- **Efecto**: Transforma vértices a espacio de pantalla
- **Impacto**: ⭐⭐⭐⭐⭐ (Muy Alto) - Proyección final a pantalla

### AnimationMatrices (c29+) - **CRÍTICO** ✅ USADO
- **Origen**: `RMeshNode::RenderNodeVS()` - Matrices de huesos animados
- **Cantidad**: Hasta 1000 matrices (hasta 333 huesos)
- **Efecto**: Skinning de vértices y normales usando hasta 3 huesos por vértice
- **Fórmula**: `Weight.x * Matrix[Indices.x] + Weight.y * Matrix[Indices.y] + (1-Weight.x-Weight.y) * Matrix[Indices.z]`
- **Impacto**: ⭐⭐⭐⭐⭐ (Muy Alto) - Animación de personajes

### Identity Matrix (c0-c2) - **USADO** ✅
- **Origen**: Matriz identidad
- **Efecto**: Usado en cálculos internos
- **Impacto**: ⭐⭐ (Bajo) - Solo para cálculos

---

## 🎯 5. VERTEX DATA (Datos del Vértice)

### Position (POSITION) - **CRÍTICO** ✅
- **Efecto**: Posición original del vértice
- **Impacto**: ⭐⭐⭐⭐⭐ (Muy Alto) - Base de todas las transformaciones

### BlendWeight (BLENDWEIGHT) - **CRÍTICO** ✅
- **Efecto**: Pesos para skinning (hasta 3 huesos)
- **Impacto**: ⭐⭐⭐⭐⭐ (Muy Alto) - Controla la mezcla de transformaciones

### BlendIndices (BLENDINDICES) - **CRÍTICO** ✅
- **Efecto**: Índices de los huesos que afectan al vértice
- **Impacto**: ⭐⭐⭐⭐⭐ (Muy Alto) - Selecciona qué matrices usar

### Normal (NORMAL) - **CRÍTICO** ✅
- **Efecto**: Normal original del vértice
- **Impacto**: ⭐⭐⭐⭐⭐ (Muy Alto) - Esencial para cálculo de iluminación
- **Nota**: Se normaliza después del skinning (corrección crítica)

### TexCoord0 (TEXCOORD0) - **USADO** ✅
- **Efecto**: Coordenadas UV para texturas
- **Impacto**: ⭐⭐⭐ (Medio) - Se pasa al pixel shader

---

## 🔧 6. CONFIGURACIÓN DEL SHADER

### mbUsingShader - **CRÍTICO** ✅
- **Origen**: `RShaderMgr::SetEnable()/SetDisable()`
- **Efecto**: Si es false, el shader no se usa
- **Impacto**: ⭐⭐⭐⭐⭐ (Muy Alto) - Activa/desactiva todo el sistema

### fVisAlpha (Visibilidad Alpha) - **IMPORTANTE** ✅
- **Origen**: `SetCharacterMtrl_ON(..., float vis_alpha)`
- **Efecto**: Controla la transparencia/visibilidad del objeto
- **Fórmula**: `MaterialDiffuse.a = fVisAlpha`
- **Impacto**: ⭐⭐⭐⭐ (Alto) - Controla la opacidad

---

## 📊 7. ORDEN DE CÁLCULO (Pipeline)

```
1. Skinning de Posición
   └─> TransformedPos (espacio de mundo)

2. Skinning de Normal
   └─> TransformedNormal (normalizada)

3. Transformación a Pantalla
   └─> oPos (espacio de pantalla)

4. Cálculo de Iluminación Difusa
   ├─> Light0: GetLightDiffuse(...)
   └─> Light1: GetLightDiffuse(...)
   └─> oDiffuse = Light0 + Light1

5. Aplicación de Material
   └─> oDiffuse *= MaterialDiffuse

6. Iluminación Ambiente
   └─> oDiffuse += (Light0Ambient + Light1Ambient + GlobalAmbient) * MaterialAmbient

7. Cálculo de Fog
   └─> oFog = fogFactor (basado en distancia a cámara)
```

---

## 🎨 8. FACTORES VISUALES FINALES

### Color Final (oDiffuse)
```
oDiffuse = (
    (Light0Diffuse * NdotL0 * Attenuation0) +
    (Light1Diffuse * NdotL1 * Attenuation1)
) * MaterialDiffuse +
(Light0Ambient + Light1Ambient + GlobalAmbient) * MaterialAmbient
```

### Fog Final (oFog)
```
oFog = lerp(1.0, saturate((FogEnd - dist) / (FogEnd - FogStart)), fogEnabled)
```

---

## ⚠️ 9. FACTORES NO UTILIZADOS (Pero Enviados)

- **MaterialSpecular** (c14): Enviado pero no usado
- **MaterialPower** (c15): Enviado pero no usado
- **Light Specular** (c20, c25): Enviado pero no usado
- **Light Range** (c21, c26): Enviado pero no validado

---

## 🔍 10. DEPENDENCIAS EXTERNAS

### Desde BSP (Mapa)
- FogStart, FogEnd, FogColor

### Desde Animación
- AnimationMatrices (huesos)

### Desde Cámara
- CameraPosition (para fog)
- View, Projection matrices

### Desde Objeto
- World matrix
- Material properties
- Vertex data (posición, normal, weights, indices)

### Desde Configuración Global
- GlobalAmbient
- Light configuration

---

## 📈 RESUMEN DE IMPACTO

| Factor | Impacto | Estado |
|--------|---------|--------|
| MaterialDiffuse | ⭐⭐⭐⭐⭐ | ✅ USADO |
| MaterialAmbient | ⭐⭐⭐⭐ | ✅ USADO |
| Light Position | ⭐⭐⭐⭐⭐ | ✅ USADO |
| Light Diffuse | ⭐⭐⭐⭐⭐ | ✅ USADO |
| Light Attenuation | ⭐⭐⭐⭐⭐ | ✅ USADO |
| GlobalAmbient | ⭐⭐⭐⭐ | ✅ USADO |
| Fog | ⭐⭐⭐⭐ | ✅ USADO |
| AnimationMatrices | ⭐⭐⭐⭐⭐ | ✅ USADO |
| Normal (normalizada) | ⭐⭐⭐⭐⭐ | ✅ USADO |
| MaterialSpecular | ⭐ | ❌ NO USADO |
| Light Specular | ⭐ | ❌ NO USADO |
| Light Range | ⭐ | ❌ NO USADO |

---

## 💡 RECOMENDACIONES

1. **MaterialSpecular y Light Specular**: Considerar implementar iluminación especular para mejor calidad visual
2. **Light Range**: Validar rango de luces para mejor rendimiento
3. **Normalización de Normales**: ✅ Ya implementada (crítica para iluminación correcta)
4. **Fog**: ✅ Ya funcional desde BSP

