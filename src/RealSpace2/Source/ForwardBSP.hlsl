float4x4 WorldViewProj : WORLDVIEWPROJECTION : register(c0);
float4x4 WorldView : register(c4);
float3 CameraPosition : register(c8);

// Material properties
float4 MaterialAmbient : register(c9);
float4 MaterialDiffuse : register(c10);
float4 MaterialSpecular : register(c11);
float SpecLevel : register(c12);  // Shininess / 255.0

// Light properties (simple directional light for now)
float3 LightDirection : register(c13);
float4 LightDiffuse : register(c14);
float4 LightAmbient : register(c15);
float4 LightSpecular : register(c16);

// Textures
sampler2D diffuseTexture   : register(s0);
sampler2D normalTexture    : register(s1);
sampler2D specularTexture  : register(s2);
sampler2D opacityTexture   : register(s3);

//---------------------------------------
//	Vertex Shader
//---------------------------------------

struct VS_OUTPUT
{
	float4 Position   : POSITION;
	float3 Normal	  : TEXCOORD1;
	float2 Texture    : TEXCOORD2;
	float3 Tangent    : TEXCOORD3;
	float3 Binormal   : TEXCOORD4;
	float3 ViewDir    : TEXCOORD5;
};

VS_OUTPUT vs_main(
	float4 Position	: POSITION,
	float3 Normal   : NORMAL,
	float2 Texture  : TEXCOORD0,
	float4 Tangent  : TANGENT
	)
{
	VS_OUTPUT Out;
	Out.Position = mul(Position, WorldViewProj);
	Out.Texture = Texture;

	// Transform normal, tangent, and binormal to view space
	Out.Normal = normalize(mul(Normal, (float3x3) WorldView));
	Out.Tangent = normalize(mul(Tangent.xyz, (float3x3) WorldView));
	Out.Binormal = normalize(cross(Out.Tangent, Out.Normal) * Tangent.w);

	// Calculate view direction in world space
	float3 worldPos = mul(Position, (float4x3)WorldView);
	Out.ViewDir = normalize(CameraPosition - worldPos);

	return Out;
}

//---------------------------------------
//	Pixel Shader
//---------------------------------------

	float4 ps_main(in VS_OUTPUT In) : COLOR0
{
	// Sample textures - usar valores por defecto si no hay textura
	float4 diffuseColor = tex2D(diffuseTexture, In.Texture);
	// Si la textura no existe o está vacía, usar color blanco
	if (diffuseColor.a == 0.0 && dot(diffuseColor.rgb, diffuseColor.rgb) < 0.001)
		diffuseColor = float4(1, 1, 1, 1);
	
	// Sample normal map (si existe, usa normal map; si no, usa normal del vértice)
	float3 normal;
	float4 normalMapSample = tex2D(normalTexture, In.Texture);
	float3 normalMap = normalMapSample.xyz;
	
	// Si el normal map no existe (alpha == 0) o es el valor por defecto, usar normal del vértice
	if (normalMapSample.w == 0.0 || 
	    (normalMap.z > 0.99 && normalMap.x < 0.51 && normalMap.x > 0.49 && 
	     normalMap.y < 0.51 && normalMap.y > 0.49))
	{
		// Normal map por defecto o no existe, usar normal del vértice
		normal = normalize(In.Normal);
	}
	else
	{
		// Unpack normal from normal map (0-1 to -1 to 1)
		float3 normalMapUnpacked = normalize(2.0 * normalMap - 1.0);
		
		// Transform normal from tangent space to view space
		normal = normalize(
			normalMapUnpacked.z * In.Normal +
			normalMapUnpacked.x * In.Tangent +
			normalMapUnpacked.y * In.Binormal
		);
	}

	// Calculate lighting
	float3 lightDir = normalize(-LightDirection);  // Light direction is from light to surface
	float NdotL = max(dot(normal, lightDir), 0.0);
	
	// Ambient lighting - asegurar que siempre haya iluminación base
	float4 ambient = LightAmbient * MaterialAmbient;
	
	// Diffuse lighting
	float4 diffuse = LightDiffuse * NdotL;
	
	// Specular lighting (Blinn-Phong) - solo si hay specular map y SpecLevel > 0
	float4 specular = float4(0, 0, 0, 0);
	if (SpecLevel > 0.0 && NdotL > 0.0)
	{
		// Sample specular map
		float4 specularSample = tex2D(specularTexture, In.Texture);
		
		// Verificar si realmente hay specular map (no es la textura dummy negra)
		// La textura dummy negra tiene valores (0,0,0,0)
		bool hasSpecularMap = (specularSample.w > 0.01 || dot(specularSample.rgb, specularSample.rgb) > 0.001);
		
		if (hasSpecularMap)
		{
			// Usar el alpha del specular map como intensidad, o el promedio RGB si alpha es 0
			float specularIntensity = specularSample.w;
			if (specularIntensity < 0.01)
				specularIntensity = (specularSample.r + specularSample.g + specularSample.b) / 3.0;
			
			// Calculate half vector for Blinn-Phong
			float3 halfVector = normalize(lightDir + In.ViewDir);
			float NdotH = max(dot(normal, halfVector), 0.0);
			
			// Calculate specular term
			float specularPower = SpecLevel * 255.0;  // Convert back to shininess value
			float specularTerm = pow(NdotH, specularPower) * specularIntensity;
			
			specular = LightSpecular * specularTerm * MaterialSpecular;
		}
		// Si no hay specular map, no aplicar specular (ya está en 0)
	}
	
	// Combine all lighting components
	float4 lighting = ambient + diffuse + specular;
	
	// Aplicar MaterialDiffuse a la iluminación
	lighting.rgb *= MaterialDiffuse.rgb;
	
	// Multiplicar por la textura diffuse
	// Asegurar que la iluminación nunca sea completamente negra (mínimo 15% de ambient)
	// Esto ayuda a que los mapas no se vean completamente oscuros
	float4 finalColor;
	finalColor.rgb = max(lighting.rgb, ambient.rgb * 0.15) * diffuseColor.rgb;
	finalColor.a = diffuseColor.a;
	
	// Apply opacity texture if available
	float4 opacitySample = tex2D(opacityTexture, In.Texture);
	// Si la textura de opacidad existe (alpha > 0 o tiene color), usarla
	if (opacitySample.w > 0.01 || dot(opacitySample.rgb, opacitySample.rgb) > 0.001)
		finalColor.a *= opacitySample.w;
	// Si no, mantener el alpha de la textura diffuse
	
	// Asegurar que el alpha nunca sea 0 para materiales opacos (para evitar transparencia no deseada)
	// Solo aplicar esto si no hay textura de opacidad o si la opacidad es alta
	if (opacitySample.w < 0.01 && dot(opacitySample.rgb, opacitySample.rgb) < 0.001)
	{
		// No hay textura de opacidad, asegurar que sea opaco
		if (finalColor.a < 0.01)
			finalColor.a = 1.0;
	}
	
	return finalColor;
}

