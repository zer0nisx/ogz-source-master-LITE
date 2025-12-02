float4x3 Identity : register(c0);
float4x3 World : register(c3);
float4x4 ViewProjection : register(c6);
float4 Constants : register(c10);  // x=LightFlags (0=none, 1=Light0, 2=Light1, 3=both), y=FogStart, z=FogEnd, w=1.0/(FogEnd-FogStart)
float3 CameraPosition : register(c11);
float4 MaterialAmbient : register(c12);
float4 MaterialDiffuse : register(c13);
float4 MaterialSpecular : register(c14);
float4 MaterialPower : register(c15);
float4 GlobalAmbient : register(c16);
float3 Light0Position : register(c17);
float4 Light0Ambient : register(c18);
float4 Light0Diffuse : register(c19);
float4 Light0Specular : register(c20);
float4 Light0Range : register(c21);
float3 Light1Position : register(c22);
float4 Light1Ambient : register(c23);
float4 Light1Diffuse : register(c24);
float4 Light1Specular : register(c25);
float4 Light1Range : register(c26);
float4 Light0Attenuation : register(c27);
float4 Light1Attenuation : register(c28);
float4 AnimationMatrices[1000] : register(c29);

float3x3 Get3x3(int Index)
{
	float3x3 ret;
	ret._m00_m10_m20 = (float3)AnimationMatrices[Index];
	ret._m01_m11_m21 = (float3)AnimationMatrices[Index + 1];
	ret._m02_m12_m22 = (float3)AnimationMatrices[Index + 2];
	return ret;
}

float4x3 Get4x3(int Index)
{
	float4x3 ret;
	ret._m00_m10_m20_m30 = AnimationMatrices[Index];
	ret._m01_m11_m21_m31 = AnimationMatrices[Index + 1];
	ret._m02_m12_m22_m32 = AnimationMatrices[Index + 2];
	return ret;
}

// Calcula la iluminación difusa de una luz puntual con atenuación (versión original)
float4 GetLightDiffuse(float3 VertexPosition, float3 VertexNormal, 
	float3 LightPosition, float4 LightDiffuse, float4 Attenuation, float LightRange)
{
	// Vector desde vértice a luz
	float3 lightDir = LightPosition - VertexPosition;
	
	// Distancia al cuadrado y distancia inversa
	float distSq = dot(lightDir, lightDir);
	
	// VALIDACIÓN DE RANGO: Si la luz tiene rango definido y estamos fuera de rango, retornar 0
	// Esto permite que la luz se renderice correctamente hasta que salga de rango
	if (LightRange > 0.0f)
	{
		float lightRangeSq = LightRange * LightRange;
		if (distSq > lightRangeSq)
			return float4(0, 0, 0, 0);
	}
	
	// CORRECCIÓN: Protección contra distancias muy pequeñas (evitar división por cero)
	// Si la distancia es muy pequeña, usar un valor mínimo para evitar problemas numéricos
	const float MIN_DIST_SQ = 0.0001f;  // 0.01 unidades mínimo (1cm)
	distSq = max(distSq, MIN_DIST_SQ);
	
	float invDist = rsqrt(distSq);
	
	// Dirección normalizada de la luz
	float3 normalizedLightDir = lightDir * invDist;
	
	// Cálculo de atenuación: 1 / (Attenuation0 + Attenuation1 * dist + Attenuation2 * dist^2)
	// Usando dst() para optimización: dst(lsq, l) = (1, lsq, lsq, l)
	float attenuationFactor = 1.0f / dot(dst(distSq, invDist).xyz, Attenuation.xyz);
	
	// CORRECCIÓN: Limitar atenuación para evitar valores extremos cuando estás muy cerca
	// Esto previene que la luz desaparezca o se sature cuando estás muy cerca
	attenuationFactor = min(attenuationFactor, 100.0f);  // Limitar a máximo 100x
	
	// Factor de iluminación difusa (dot product normalizado)
	float NdotL = dot(VertexNormal, normalizedLightDir);
	float diffuseFactor = max(NdotL, 0.0f) * attenuationFactor;
	
	// Retornar color difuso modulado
	return LightDiffuse * diffuseFactor;
}

// OPTIMIZACIÓN: Versión optimizada que reutiliza cálculos pre-hechos
float4 GetLightDiffuseOptimized(float3 VertexPosition, float3 VertexNormal,
	float3 lightDir, float distSq, float3 normalizedLightDir, float NdotL,
	float4 LightDiffuse, float4 Attenuation, float LightRange)
{
	// VALIDACIÓN DE RANGO: Si la luz tiene rango definido y estamos fuera de rango, retornar 0
	if (LightRange > 0.0f)
	{
		float lightRangeSq = LightRange * LightRange;
		if (distSq > lightRangeSq)
			return float4(0, 0, 0, 0);
	}
	
	// CORRECCIÓN: Protección contra distancias muy pequeñas
	const float MIN_DIST_SQ = 0.0001f;
	float safeDistSq = max(distSq, MIN_DIST_SQ);
	float invDist = rsqrt(safeDistSq);
	
	// Cálculo de atenuación
	float attenuationFactor = 1.0f / dot(dst(safeDistSq, invDist).xyz, Attenuation.xyz);
	attenuationFactor = min(attenuationFactor, 100.0f);  // Limitar a máximo 100x
	
	// Factor de iluminación difusa (NdotL ya calculado)
	float diffuseFactor = NdotL * attenuationFactor;
	
	// Retornar color difuso modulado
	return LightDiffuse * diffuseFactor;
}

// Calcula la iluminación especular de una luz puntual usando modelo Blinn-Phong (versión original)
float4 GetLightSpecular(float3 VertexPosition, float3 VertexNormal, float3 ViewDir,
	float3 LightPosition, float4 LightSpecular, float4 Attenuation,
	float4 MaterialSpecular, float MaterialPower, float NdotL, float LightRange)
{
	// Solo calcular si hay contribución difusa (NdotL > 0) y MaterialPower > 0
	if (NdotL <= 0.0f || MaterialPower <= 0.0f)
		return float4(0, 0, 0, 0);
	
	// Vector desde vértice a luz
	float3 lightDir = LightPosition - VertexPosition;
	float distSq = dot(lightDir, lightDir);
	
	// VALIDACIÓN DE RANGO: Si la luz tiene rango definido y estamos fuera de rango, retornar 0
	if (LightRange > 0.0f)
	{
		float lightRangeSq = LightRange * LightRange;
		if (distSq > lightRangeSq)
			return float4(0, 0, 0, 0);
	}
	
	// CORRECCIÓN: Protección contra distancias muy pequeñas (evitar división por cero)
	const float MIN_DIST_SQ = 0.0001f;  // 0.01 unidades mínimo (1cm)
	distSq = max(distSq, MIN_DIST_SQ);
	
	float invDist = rsqrt(distSq);
	float3 normalizedLightDir = lightDir * invDist;
	
	// Cálculo de atenuación
	float attenuationFactor = 1.0f / dot(dst(distSq, invDist).xyz, Attenuation.xyz);
	
	// CORRECCIÓN: Limitar atenuación para evitar valores extremos cuando estás muy cerca
	attenuationFactor = min(attenuationFactor, 100.0f);  // Limitar a máximo 100x
	
	// Calcular vector halfway (Blinn-Phong es más eficiente que Phong)
	float3 halfway = normalize(normalizedLightDir + ViewDir);
	float NdotH = dot(VertexNormal, halfway);
	
	// Calcular factor especular: pow(NdotH, MaterialPower)
	float specularFactor = pow(max(NdotH, 0.0f), MaterialPower) * attenuationFactor;
	
	// Color especular = LightSpecular * MaterialSpecular * specularFactor
	return LightSpecular * MaterialSpecular * specularFactor;
}

// OPTIMIZACIÓN: Versión optimizada que reutiliza cálculos pre-hechos
float4 GetLightSpecularOptimized(float3 VertexNormal, float3 ViewDir,
	float3 lightDir, float distSq, float3 normalizedLightDir, float NdotL,
	float4 LightSpecular, float4 Attenuation,
	float4 MaterialSpecular, float MaterialPower, float LightRange)
{
	// Solo calcular si hay contribución difusa (NdotL > 0) y MaterialPower > 0
	if (NdotL <= 0.0f || MaterialPower <= 0.0f)
		return float4(0, 0, 0, 0);
	
	// VALIDACIÓN DE RANGO
	if (LightRange > 0.0f)
	{
		float lightRangeSq = LightRange * LightRange;
		if (distSq > lightRangeSq)
			return float4(0, 0, 0, 0);
	}
	
	// CORRECCIÓN: Protección contra distancias muy pequeñas
	const float MIN_DIST_SQ = 0.0001f;
	float safeDistSq = max(distSq, MIN_DIST_SQ);
	float invDist = rsqrt(safeDistSq);
	
	// Cálculo de atenuación
	float attenuationFactor = 1.0f / dot(dst(safeDistSq, invDist).xyz, Attenuation.xyz);
	attenuationFactor = min(attenuationFactor, 100.0f);
	
	// Calcular vector halfway (Blinn-Phong)
	float3 halfway = normalize(normalizedLightDir + ViewDir);
	float NdotH = dot(VertexNormal, halfway);
	
	// Calcular factor especular
	float specularFactor = pow(max(NdotH, 0.0f), MaterialPower) * attenuationFactor;
	
	// Color especular
	return LightSpecular * MaterialSpecular * specularFactor;
}

void main(float4 Pos            : POSITION,
          float2 Weight         : BLENDWEIGHT,
          float3 Indices        : BLENDINDICES,
          float3 Normal         : NORMAL,
          float2 T0             : TEXCOORD0,
      out float4 oPos           : POSITION,
      out float2 oT0            : TEXCOORD0,
      out float4 oDiffuse       : COLOR0,
      out float  oFog           : FOG)
{
	// Skinning de posición: transformar vértice usando hasta 3 huesos
	float3 TransformedPos =
		mul(mul(Pos, Get4x3(Indices.x)), Weight.x) +
		mul(mul(Pos, Get4x3(Indices.y)), Weight.y) +
		mul(mul(Pos, Get4x3(Indices.z)), 1.0f - (Weight.x + Weight.y));

	// Transformar a espacio de mundo y luego a espacio de pantalla
	TransformedPos = mul(float4(TransformedPos, 1.0f), World);
	oPos = mul(float4(TransformedPos, 1.0f), ViewProjection);

	// Skinning de normal: transformar normal usando las mismas matrices de huesos
	float3 TransformedNormal = 
		mul(mul(Normal, Get3x3(Indices.x)), Weight.x) +
		mul(mul(Normal, Get3x3(Indices.y)), Weight.y) +
		mul(mul(Normal, Get3x3(Indices.z)), 1.0f - (Weight.x + Weight.y));
	
	// CORRECCIÓN CRÍTICA: Normalizar la normal después del skinning
	// Esto es esencial para una iluminación correcta, ya que las transformaciones
	// de skinning pueden hacer que la normal no esté normalizada
	TransformedNormal = normalize(TransformedNormal);

	// OPTIMIZACIÓN: Early Exit - Solo calcular iluminación si la luz está habilitada
	// Constants.x contiene flags: 0.0=none, 1.0=Light0, 2.0=Light1, 3.0=both
	// Esto ahorra ~10-15 instrucciones por vértice cuando una luz está desactivada
	float light0Enabled = step(0.5f, Constants.x);  // >= 0.5 = Light0 activa
	float light1Enabled = step(1.5f, Constants.x);  // >= 1.5 = Light1 activa
	
	// Calcular vector de vista (necesario para especular y rim lighting)
	float3 viewDir = normalize(CameraPosition - TransformedPos);
	
	// Calcular iluminación difusa solo para luces habilitadas
	oDiffuse = float4(0, 0, 0, 0);
	
	// Variables para reutilización de cálculos
	float3 light0Dir = float3(0, 0, 0);
	float distSq0 = 0.0f;
	float3 normalizedLight0Dir = float3(0, 0, 0);
	float NdotL0 = 0.0f;
	float3 light1Dir = float3(0, 0, 0);
	float distSq1 = 0.0f;
	float3 normalizedLight1Dir = float3(0, 0, 0);
	float NdotL1 = 0.0f;
	
	// MEJORA: Calcular valores para Light0 una vez y reutilizar
	if (light0Enabled > 0.0f)
	{
		light0Dir = Light0Position - TransformedPos;
		distSq0 = dot(light0Dir, light0Dir);
		
		// MEJORA: Early Exit mejorado - verificar rango ANTES de calcular resto
		float lightRangeSq0 = Light0Range.x * Light0Range.x;
		if (Light0Range.x <= 0.0f || distSq0 <= lightRangeSq0)
		{
			// Calcular valores solo si está en rango
			const float MIN_DIST_SQ = 0.0001f;
			float safeDistSq0 = max(distSq0, MIN_DIST_SQ);
			float invDist0 = rsqrt(safeDistSq0);
			normalizedLight0Dir = light0Dir * invDist0;
			NdotL0 = max(dot(TransformedNormal, normalizedLight0Dir), 0.0f);
			
			// Usar función optimizada que reutiliza cálculos
			oDiffuse += GetLightDiffuseOptimized(TransformedPos, TransformedNormal,
				light0Dir, distSq0, normalizedLight0Dir, NdotL0,
				Light0Diffuse, Light0Attenuation, Light0Range.x);
		}
	}
	
	// MEJORA: Calcular valores para Light1 una vez y reutilizar
	if (light1Enabled > 0.0f)
	{
		light1Dir = Light1Position - TransformedPos;
		distSq1 = dot(light1Dir, light1Dir);
		
		// MEJORA: Early Exit mejorado - verificar rango ANTES de calcular resto
		float lightRangeSq1 = Light1Range.x * Light1Range.x;
		if (Light1Range.x <= 0.0f || distSq1 <= lightRangeSq1)
		{
			// Calcular valores solo si está en rango
			const float MIN_DIST_SQ = 0.0001f;
			float safeDistSq1 = max(distSq1, MIN_DIST_SQ);
			float invDist1 = rsqrt(safeDistSq1);
			normalizedLight1Dir = light1Dir * invDist1;
			NdotL1 = max(dot(TransformedNormal, normalizedLight1Dir), 0.0f);
			
			// Usar función optimizada que reutiliza cálculos
			oDiffuse += GetLightDiffuseOptimized(TransformedPos, TransformedNormal,
				light1Dir, distSq1, normalizedLight1Dir, NdotL1,
				Light1Diffuse, Light1Attenuation, Light1Range.x);
		}
	}
	
	// Calcular iluminación especular si MaterialPower > 0
	// MEJORA: Reutilizar cálculos ya hechos para lightDir, distSq, etc.
	if (MaterialPower.x > 0.0f)
	{
		// OPTIMIZACIÓN: Solo calcular especular para luces habilitadas y en rango
		if (light0Enabled > 0.0f && (Light0Range.x <= 0.0f || distSq0 <= (Light0Range.x * Light0Range.x)))
		{
			if (NdotL0 > 0.0f)
			{
				// Usar función optimizada que reutiliza cálculos
				oDiffuse += GetLightSpecularOptimized(TransformedNormal, viewDir,
					light0Dir, distSq0, normalizedLight0Dir, NdotL0,
					Light0Specular, Light0Attenuation,
					MaterialSpecular, MaterialPower.x, Light0Range.x);
			}
		}
		
		if (light1Enabled > 0.0f && (Light1Range.x <= 0.0f || distSq1 <= (Light1Range.x * Light1Range.x)))
		{
			if (NdotL1 > 0.0f)
			{
				// Usar función optimizada que reutiliza cálculos
				oDiffuse += GetLightSpecularOptimized(TransformedNormal, viewDir,
					light1Dir, distSq1, normalizedLight1Dir, NdotL1,
					Light1Specular, Light1Attenuation,
					MaterialSpecular, MaterialPower.x, Light1Range.x);
			}
		}
	}
	
	// MEJORA: Rim Lighting - efecto de borde brillante
	// Calcular factor de rim basado en ángulo entre normal y vista
	float rimFactor = 1.0 - dot(TransformedNormal, viewDir);
	rimFactor = pow(max(rimFactor, 0.0), 2.0);  // Exponente 2 para rim suave
	float rimIntensity = 0.3;  // Intensidad del rim (ajustable)
	float4 rimColor = float4(1, 1, 1, 1) * rimFactor * rimIntensity;
	oDiffuse += rimColor;
	
	// Aplicar color difuso del material
	oDiffuse *= MaterialDiffuse;
	
	// Agregar iluminación ambiente (global + luces)
	oDiffuse += (Light0Ambient + Light1Ambient + GlobalAmbient) * MaterialAmbient;
	
	// Preservar alpha del material
	oDiffuse.w = MaterialDiffuse.w;

	// Pasar coordenadas de textura
	oT0 = T0;
	
	// MEJORA: Calcular fog basado en distancia desde cámara
	// Soporta fog lineal y exponencial
	// Constants.y = FogStart, Constants.z = FogEnd, Constants.w = 1.0/(FogEnd-FogStart) o densidad
	// Si Constants.w es 0.0, no hay fog configurado (retornar 1.0)
	float3 cameraToVertex = TransformedPos - CameraPosition;
	float distToCamera = length(cameraToVertex);
	
	float fogFactor = 1.0f;
	float fogEnabled = step(0.0001f, Constants.w);
	
	if (fogEnabled > 0.0f)
	{
		// MEJORA: Soporte para fog exponencial (más realista)
		// Si FogStart == 0, usar fog exponencial
		// Si FogStart > 0, usar fog lineal
		float useExponentialFog = step(0.0001f, Constants.y);  // 0 si FogStart == 0, 1 si > 0
		
		// Fog lineal: fogFactor = (FogEnd - dist) / (FogEnd - FogStart)
		float linearFogFactor = (Constants.z - distToCamera) * Constants.w;
		linearFogFactor = saturate(linearFogFactor);
		
		// Fog exponencial: fogFactor = exp(-dist * densidad)
		float fogDensity = Constants.w;  // En modo exponencial, Constants.w es la densidad
		float exponentialFogFactor = exp(-distToCamera * fogDensity);
		exponentialFogFactor = saturate(exponentialFogFactor);
		
		// Seleccionar según modo
		fogFactor = lerp(exponentialFogFactor, linearFogFactor, useExponentialFog);
	}
	
	oFog = lerp(1.0f, fogFactor, fogEnabled);
}
