# Comparación de Modelos de Iluminación Especular

## Modelos Disponibles

### 1. **Phong** (Modelo Original)
- **Fórmula**: `R = reflect(-L, N)`, `specular = pow(max(dot(V, R), 0), power)`
- **Cálculo**: Requiere calcular el vector de reflexión `R`
- **Ventajas**: 
  - Modelo clásico y bien entendido
  - Buen control del tamaño del highlight
- **Desventajas**:
  - Más costoso (requiere `reflect()`)
  - Puede producir artefactos en ángulos extremos
- **Costo**: ~3-4 instrucciones más que Blinn-Phong

### 2. **Blinn-Phong** (Modelo Actual - Recomendado)
- **Fórmula**: `H = normalize(L + V)`, `specular = pow(max(dot(N, H), 0), power)`
- **Cálculo**: Usa vector "halfway" (mitad entre luz y vista)
- **Ventajas**:
  - Más eficiente que Phong (no requiere `reflect()`)
  - Más suave y natural
  - Estándar en la industria
  - Mejor comportamiento en ángulos extremos
- **Desventajas**:
  - Ligeramente diferente visualmente que Phong
- **Costo**: Más eficiente que Phong

### 3. **Cook-Torrance** (Físicamente Basado - PBR)
- **Fórmula**: Complejo, incluye términos de Fresnel, distribución normal, geometría
- **Cálculo**: Muy costoso, requiere múltiples cálculos
- **Ventajas**:
  - Físicamente preciso
  - Mejor para materiales realistas
  - Estándar en gráficos modernos (PBR)
- **Desventajas**:
  - Muy costoso para vertex shader
  - Requiere más parámetros (roughness, metallic, etc.)
- **Costo**: ~100+ instrucciones (no viable para vertex shader)

### 4. **Ward** (Modelo Anisotrópico)
- **Fórmula**: Similar a Blinn-Phong pero con términos anisotrópicos
- **Cálculo**: Más complejo que Blinn-Phong
- **Ventajas**:
  - Soporta highlights anisotrópicos (brillo direccional)
  - Bueno para materiales como tela, cabello, metal pulido
- **Desventajas**:
  - Más costoso
  - Requiere parámetros adicionales
- **Costo**: ~20-30 instrucciones más que Blinn-Phong

### 5. **Schlick** (Aproximación de Fresnel)
- **Fórmula**: `F = F0 + (1 - F0) * pow(1 - dot(H, V), 5)`
- **Cálculo**: Aproximación rápida de Fresnel
- **Ventajas**:
  - Muy eficiente
  - Bueno para combinar con otros modelos
- **Desventajas**:
  - No es un modelo completo, solo término de Fresnel
- **Costo**: ~5-10 instrucciones

## Comparación Visual

| Modelo | Suavidad | Realismo | Eficiencia | Uso Típico |
|--------|----------|----------|------------|------------|
| **Phong** | Media | Medio | Media | Clásico, control preciso |
| **Blinn-Phong** | Alta | Medio-Alto | Alta | Estándar moderno |
| **Cook-Torrance** | Variable | Muy Alto | Muy Baja | PBR, gráficos modernos |
| **Ward** | Variable | Alto | Media | Materiales anisotrópicos |
| **Schlick** | N/A | Medio | Muy Alta | Término de Fresnel |

## Comparación de Código

### Phong (Original)
```hlsl
float3 R = reflect(-normalizedLightDir, VertexNormal);
float RdotV = dot(R, ViewDir);
float specularFactor = pow(max(RdotV, 0.0f), MaterialPower) * attenuationFactor;
```

### Blinn-Phong (Actual - Implementado)
```hlsl
float3 H = normalize(normalizedLightDir + ViewDir);
float NdotH = dot(VertexNormal, H);
float specularFactor = pow(max(NdotH, 0.0f), MaterialPower) * attenuationFactor;
```

### Diferencia Clave
- **Phong**: Calcula el vector de reflexión `R` y compara con la vista `V`
- **Blinn-Phong**: Calcula el vector halfway `H` y compara con la normal `N`

## Recomendación para Vertex Shader

**Blinn-Phong es la mejor opción** porque:
1. ✅ Más eficiente que Phong
2. ✅ Visualmente similar o mejor
3. ✅ Estándar en la industria
4. ✅ Mejor comportamiento numérico
5. ✅ Adecuado para vertex shader (no muy costoso)

## ¿Cuándo Usar Cada Uno?

- **Blinn-Phong**: ✅ **Recomendado para vertex shader** (actual)
- **Phong**: Si necesitas compatibilidad exacta con código legacy
- **Cook-Torrance**: Solo para pixel shader PBR moderno
- **Ward**: Si necesitas highlights anisotrópicos (cabello, tela)
- **Schlick**: Como término adicional para mejorar realismo

## Implementación Alternativa: Phong

Si quisieras cambiar a Phong, el código sería:

```hlsl
// Phong (alternativa)
float3 R = reflect(-normalizedLightDir, VertexNormal);
float RdotV = dot(R, ViewDir);
float specularFactor = pow(max(RdotV, 0.0f), MaterialPower) * attenuationFactor;
```

**Nota**: `reflect()` requiere más instrucciones que `normalize(L + V)`, por lo que Blinn-Phong es más eficiente.

