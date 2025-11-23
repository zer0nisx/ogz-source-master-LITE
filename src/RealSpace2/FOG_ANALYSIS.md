# Análisis del Fog en el Shader Skin

## Estado Actual

### ❌ PROBLEMA: El fog NO está funcional

1. **Engine configura fog del hardware:**
   - `RSetFog()` configura `D3DRS_FOGENABLE`, `D3DRS_FOGSTART`, `D3DRS_FOGEND`, `D3DRS_FOGCOLOR`
   - Modo: `D3DFOG_LINEAR` (fog basado en tabla)

2. **Shader tiene fog hardcodeado:**
   - `oFog = 1.0f` (sin fog)
   - El shader compilado muestra: `mov oFog, c10.x` (donde c10.x = 1.0f)

3. **Problema fundamental:**
   - Cuando usas un **vertex shader personalizado**, el fog automático del hardware **NO funciona**
   - DirectX 9 requiere calcular el fog manualmente en el shader
   - No se envían constantes de fog (FogNear, FogFar) al shader

## Solución Requerida

### 1. Agregar constantes de fog al shader
- `FogStart` (distancia donde empieza el fog)
- `FogEnd` (distancia donde termina el fog)
- `FogRange` (FogEnd - FogStart, para optimización)

### 2. Calcular distancia desde cámara
- Usar `CameraPosition` (ya disponible en c11)
- Calcular distancia: `length(TransformedPos - CameraPosition)`
- O usar la coordenada Z en espacio de vista (más eficiente)

### 3. Calcular factor de fog
- Fórmula: `saturate((FogEnd - dist) / (FogEnd - FogStart))`
- Valor 1.0 = sin fog (cerca)
- Valor 0.0 = fog completo (lejos)

### 4. Enviar constantes desde C++
- Agregar función en `RShaderMgr` para actualizar fog
- Llamar desde `RShaderMgr::Update()` o desde `RMeshNode::RenderNodeVS()`

## Implementación

### Opción 1: Usar distancia en espacio de mundo (más preciso)
- Calcular distancia desde `CameraPosition` a `TransformedPos`
- Requiere `sqrt()` o `length()`

### Opción 2: Usar coordenada Z en espacio de vista (más eficiente)
- Usar `oPos.z` después de transformar a espacio de vista
- Más eficiente pero menos preciso para fog basado en distancia real

### Recomendación: Opción 1 (distancia real)
- Más preciso y consistente con el fog del hardware
- El costo de `length()` es aceptable en vs_1_1

