float4x3 Identity : register(c0);
float4x3 World : register(c3);
float4x4 ViewProjection : register(c6);
float4 Constants : register(c10);
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

// Calcula la iluminación difusa de una luz puntual con atenuación
float4 GetLightDiffuse(float3 VertexPosition, float3 VertexNormal, 
	float3 LightPosition, float4 LightDiffuse, float4 Attenuation)
{
	// Vector desde vértice a luz
	float3 lightDir = LightPosition - VertexPosition;
	
	// Distancia al cuadrado y distancia inversa
	float distSq = dot(lightDir, lightDir);
	float invDist = rsqrt(distSq);
	
	// Dirección normalizada de la luz
	float3 normalizedLightDir = lightDir * invDist;
	
	// Cálculo de atenuación: 1 / (Attenuation0 + Attenuation1 * dist + Attenuation2 * dist^2)
	// Usando dst() para optimización: dst(lsq, l) = (1, lsq, lsq, l)
	float attenuationFactor = 1.0f / dot(dst(distSq, invDist).xyz, Attenuation.xyz);
	
	// Factor de iluminación difusa (dot product normalizado)
	float NdotL = dot(VertexNormal, normalizedLightDir);
	float diffuseFactor = max(NdotL, 0.0f) * attenuationFactor;
	
	// Retornar color difuso modulado
	return LightDiffuse * diffuseFactor;
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

	// Calcular iluminación difusa de ambas luces
	oDiffuse = GetLightDiffuse(TransformedPos, TransformedNormal,
		Light0Position, Light0Diffuse, Light0Attenuation);
	oDiffuse += GetLightDiffuse(TransformedPos, TransformedNormal,
		Light1Position, Light1Diffuse, Light1Attenuation);
	
	// Aplicar color difuso del material
	oDiffuse *= MaterialDiffuse;
	
	// Agregar iluminación ambiente (global + luces)
	oDiffuse += (Light0Ambient + Light1Ambient + GlobalAmbient) * MaterialAmbient;
	
	// Preservar alpha del material
	oDiffuse.w = MaterialDiffuse.w;

	// Pasar coordenadas de textura
	oT0 = T0;
	
	// Fog hardcodeado a 1.0 (sin fog)
	// Nota: Si se necesita fog en el futuro, calcular basado en distancia a cámara
	oFog = 1.0f;
}