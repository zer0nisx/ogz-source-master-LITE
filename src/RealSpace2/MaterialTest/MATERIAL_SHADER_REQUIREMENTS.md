# Requisitos del Shader para Materiales Iluminados

## 📋 Resumen

Este documento describe los componentes necesarios en un shader para renderizar materiales iluminados correctamente en RealSpace2.

---

## 🎯 Componentes Esenciales del Shader

### 1. **Transformaciones de Matriz**

#### Registros Requeridos:
- `World` (c3): Matriz de transformación del mundo (float4x3)
- `ViewProjection` (c6): Matriz de vista-proyección (float4x4)

#### Uso:
```hlsl
// Transformar posición a espacio de mundo
float3 WorldPos = mul(float4(Pos.xyz, 1.0f), World);

// Transformar a espacio de pantalla
oPos = mul(float4(WorldPos, 1.0f), ViewProjection);

// Transformar normal a espacio de mundo
float3 WorldNormal = normalize(mul(Normal, (float3x3)World));
```

**Por qué es necesario:**
- Las luces y la cámara están en espacio de mundo
- Las normales deben estar en espacio de mundo para cálculos de iluminación correctos
- La posición del vértice en espacio de mundo es necesaria para calcular distancias a las luces

---

### 2. **Propiedades del Material**

#### Registros Requeridos:
- `MaterialAmbient` (c12): Color ambiente del material (float4)
- `MaterialDiffuse` (c13): Color difuso del material (float4)
- `MaterialSpecular` (c14): Color especular del material (float4)
- `MaterialPower` (c15): Exponente de brillo especular (float4, usar .x)

#### Uso:
```hlsl
// Aplicar color difuso del material
oDiffuse *= MaterialDiffuse;

// Agregar iluminación ambiente
oDiffuse += GlobalAmbient * MaterialAmbient;

// Calcular especular
float specular = pow(max(NdotH, 0.0f), MaterialPower.x);
oDiffuse += LightSpecular * MaterialSpecular * specular;
```

**Por qué es necesario:**
- **Ambient**: Color base del material cuando no hay luz directa
- **Diffuse**: Color principal del material bajo iluminación difusa
- **Specular**: Color del reflejo especular (típicamente blanco)
- **Power**: Controla qué tan "brillante" o "mate" es el material (valores altos = más brillante)

---

### 3. **Iluminación: Luces Puntuales**

#### Registros Requeridos (por cada luz):
- `LightXPosition` (c17, c22): Posición de la luz en espacio de mundo (float3)
- `LightXAmbient` (c18, c23): Color ambiente de la luz (float4)
- `LightXDiffuse` (c19, c24): Color difuso de la luz (float4)
- `LightXSpecular` (c20, c25): Color especular de la luz (float4)
- `LightXRange` (c21, c26): Rango máximo de la luz (float4, usar .x)
- `LightXAttenuation` (c27, c28): Coeficientes de atenuación (float4: x=const, y=linear, z=quadratic)

#### Cálculo de Iluminación Difusa:
```hlsl
// Vector desde vértice a luz
float3 lightDir = LightPosition - WorldPos;

// Distancia y normalización
float distSq = dot(lightDir, lightDir);
float invDist = rsqrt(distSq);
float3 normalizedLightDir = lightDir * invDist;

// Atenuación: 1 / (Attenuation0 + Attenuation1 * dist + Attenuation2 * dist^2)
float attenuation = 1.0f / dot(dst(distSq, invDist).xyz, Attenuation.xyz);

// Factor difuso (dot product normal-luz)
float NdotL = max(dot(WorldNormal, normalizedLightDir), 0.0f);

// Color difuso final
float4 diffuse = LightDiffuse * NdotL * attenuation;
```

**Por qué es necesario:**
- **Position**: Para calcular la dirección y distancia de la luz
- **Ambient/Diffuse/Specular**: Colores de la luz en cada componente
- **Range**: Para culling de luces fuera de rango (optimización)
- **Attenuation**: Simula cómo la luz se debilita con la distancia

---

### 4. **Iluminación Especular (Blinn-Phong)**

#### Componentes Necesarios:
- `CameraPosition` (c11): Posición de la cámara en espacio de mundo (float3)
- `MaterialSpecular` (c14): Color especular del material
- `MaterialPower` (c15): Exponente de brillo
- `LightSpecular` (c20, c25): Color especular de la luz

#### Cálculo:
```hlsl
// Vector de vista (desde vértice a cámara)
float3 viewDir = normalize(CameraPosition - WorldPos);

// Vector halfway (Blinn-Phong es más eficiente que Phong)
float3 halfway = normalize(normalizedLightDir + viewDir);

// Factor especular
float NdotH = dot(WorldNormal, halfway);
float specular = pow(max(NdotH, 0.0f), MaterialPower.x) * attenuation;

// Color especular final
float4 specularColor = LightSpecular * MaterialSpecular * specular;
```

**Por qué es necesario:**
- **CameraPosition**: Para calcular el vector de vista
- **MaterialSpecular + Power**: Define qué tan brillante y de qué color es el reflejo
- **LightSpecular**: Color del reflejo especular de la luz

**Diferencia Blinn-Phong vs Phong:**
- **Phong**: Usa el ángulo entre el reflejo de la luz y el vector de vista
- **Blinn-Phong**: Usa el vector "halfway" (más eficiente y visualmente similar)

---

### 5. **Ambiente Global**

#### Registro Requerido:
- `GlobalAmbient` (c16): Iluminación ambiente global (float4)

#### Uso:
```hlsl
// Agregar iluminación ambiente global
oDiffuse += GlobalAmbient * MaterialAmbient;
```

**Por qué es necesario:**
- Simula la iluminación indirecta (luz rebotada)
- Asegura que los objetos nunca estén completamente negros
- Proporciona iluminación base cuando no hay luces directas

---

### 6. **Optimizaciones: Early Exit**

#### Registro Requerido:
- `Constants.x` (c10): Flags de luces activas (0=none, 1=Light0, 2=Light1, 3=both)

#### Uso:
```hlsl
// Determinar qué luces están habilitadas
float light0Enabled = step(0.5f, Constants.x);
float light1Enabled = step(1.5f, Constants.x);

// Solo calcular iluminación para luces activas
if (light0Enabled > 0.0f)
{
    // Calcular iluminación de Light0
}
```

**Por qué es necesario:**
- Reduce el número de instrucciones del shader
- Mejora el rendimiento cuando hay menos luces activas
- Evita cálculos innecesarios

---

### 7. **Validación de Rango de Luces**

#### Uso:
```hlsl
// Validar rango antes de calcular iluminación
if (LightRange.x > 0.0f)
{
    float lightRangeSq = LightRange.x * LightRange.x;
    if (distSq > lightRangeSq)
    {
        // Fuera de rango, solo agregar ambiente
        return LightAmbient * MaterialAmbient;
    }
}
```

**Por qué es necesario:**
- Optimización: evita cálculos de luces muy lejanas
- Control artístico: permite definir el alcance de cada luz
- Rendimiento: reduce cálculos innecesarios

---

### 8. **Estabilidad Numérica**

#### Protecciones Necesarias:
```hlsl
// Protección contra distancias muy pequeñas
const float MIN_DIST_SQ = 0.0001f;
distSq = max(distSq, MIN_DIST_SQ);

// Limitar atenuación extrema
attenuation = min(attenuation, 100.0f);
```

**Por qué es necesario:**
- Evita división por cero cuando el vértice está exactamente en la posición de la luz
- Previene valores de atenuación extremos que causan artefactos visuales
- Asegura que la iluminación sea estable y predecible

---

### 9. **Fog (Opcional pero Recomendado)**

#### Registros Requeridos:
- `Constants.y` (c10): FogStart
- `Constants.z` (c10): FogEnd
- `Constants.w` (c10): 1.0 / (FogEnd - FogStart)
- `CameraPosition` (c11): Para calcular distancia

#### Cálculo:
```hlsl
float3 cameraToVertex = WorldPos - CameraPosition;
float distToCamera = length(cameraToVertex);
float fogFactor = (Constants.z - distToCamera) * Constants.w;
fogFactor = saturate(fogFactor);
oFog = lerp(1.0f, fogFactor, step(0.0001f, Constants.w));
```

**Por qué es necesario:**
- Mejora la percepción de profundidad
- Oculta objetos distantes de manera suave
- Integración con el sistema de fog del motor

---

## 📊 Resumen de Registros

| Registro | Tipo | Contenido | Uso |
|----------|------|-----------|-----|
| c3 | float4x3 | World | Transformación de mundo |
| c6 | float4x4 | ViewProjection | Transformación vista-proyección |
| c10 | float4 | Constants | Flags de luces, fog |
| c11 | float3 | CameraPosition | Posición de cámara |
| c12 | float4 | MaterialAmbient | Color ambiente del material |
| c13 | float4 | MaterialDiffuse | Color difuso del material |
| c14 | float4 | MaterialSpecular | Color especular del material |
| c15 | float4 | MaterialPower | Exponente de brillo especular |
| c16 | float4 | GlobalAmbient | Iluminación ambiente global |
| c17-c28 | Varios | Light0/Light1 | Propiedades de las luces |

---

## 🔧 Flujo de Cálculo Completo

1. **Transformar vértice y normal a espacio de mundo**
2. **Para cada luz activa:**
   - Calcular vector y distancia a la luz
   - Validar rango (si aplica)
   - Calcular atenuación
   - Calcular iluminación difusa (NdotL)
   - Calcular iluminación especular (si MaterialPower > 0)
   - Agregar iluminación ambiente de la luz
3. **Agregar iluminación ambiente global**
4. **Aplicar color difuso del material**
5. **Calcular fog (opcional)**
6. **Retornar color final**

---

## ✅ Checklist para Shader de Material Iluminado

- [ ] Transformaciones de matriz (World, ViewProjection)
- [ ] Propiedades del material (Ambient, Diffuse, Specular, Power)
- [ ] Posición de cámara (para especular y fog)
- [ ] Al menos una luz puntual con:
  - [ ] Posición
  - [ ] Colores (Ambient, Diffuse, Specular)
  - [ ] Rango
  - [ ] Atenuación
- [ ] Iluminación ambiente global
- [ ] Normalización de normales después de transformación
- [ ] Protecciones numéricas (MIN_DIST_SQ, límite de atenuación)
- [ ] Validación de rango de luces
- [ ] Early exit para luces desactivadas (optimización)
- [ ] Cálculo de fog (opcional pero recomendado)

---

## 🎨 Ejemplo de Materiales

### Material Mate (No Especular)
```cpp
MaterialSpecular = (0, 0, 0, 0)
MaterialPower = 0
```

### Material Brillante (Metal)
```cpp
MaterialSpecular = (1, 1, 1, 1)
MaterialPower = 32.0
```

### Material Plástico
```cpp
MaterialSpecular = (0.8, 0.8, 0.8, 1)
MaterialPower = 16.0
```

### Material Goma
```cpp
MaterialSpecular = (0.2, 0.2, 0.2, 1)
MaterialPower = 4.0
```

---

## 📝 Notas Importantes

1. **Normalización de Normales**: CRÍTICO normalizar las normales después de transformarlas a espacio de mundo
2. **Atenuación**: Usar `dst()` para optimización en DirectX 9
3. **Especular**: Solo calcular si `NdotL > 0` y `MaterialPower > 0`
4. **Rango**: Validar antes de calcular atenuación para mejor rendimiento
5. **Estabilidad**: Siempre proteger contra distancias muy pequeñas

---

## 🔗 Referencias

- Shader de referencia: `src/RealSpace2/Source/skin.hlsl`
- Shader de prueba: `src/RealSpace2/MaterialTest/Shaders/MaterialTest.hlsl`
- Gestor de shaders: `src/RealSpace2/Source/RShaderMgr.cpp`

