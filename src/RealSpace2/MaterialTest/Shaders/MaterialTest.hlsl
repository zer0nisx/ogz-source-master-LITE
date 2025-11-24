// ============================================================================
// MaterialTest.hlsl - Shader de prueba para materiales iluminados
// ============================================================================
// Este shader demuestra los componentes necesarios para renderizar materiales
// iluminados con múltiples luces puntuales usando el modelo Blinn-Phong.
// ============================================================================

// ============================================================================
// REGISTROS DE CONSTANTES (Vertex Shader)
// ============================================================================
float4x3 World : register(c3);
float4x4 ViewProjection : register(c6);
float4 Constants : register(c10);  // x=LightFlags, y=FogStart, z=FogEnd, w=1.0/(FogEnd-FogStart)
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

// ============================================================================
// VERTEX SHADER
// ============================================================================
void main(
    float4 Pos            : POSITION,
    float3 Normal         : NORMAL,
    float2 T0             : TEXCOORD0,
    out float4 oPos       : POSITION,
    out float2 oT0        : TEXCOORD0,
    out float4 oDiffuse   : COLOR0,
    out float  oFog       : FOG)
{
    // Transformar posición a espacio de mundo
    float3 WorldPos = mul(float4(Pos.xyz, 1.0f), World);
    
    // Transformar a espacio de pantalla
    oPos = mul(float4(WorldPos, 1.0f), ViewProjection);
    
    // Transformar normal a espacio de mundo y normalizar
    float3 WorldNormal = normalize(mul(Normal, (float3x3)World));
    
    // Early Exit: Determinar qué luces están habilitadas
    float light0Enabled = step(0.5f, Constants.x);  // Light0 activa si Constants.x >= 0.5
    float light1Enabled = step(1.5f, Constants.x);  // Light1 activa si Constants.x >= 1.5
    
    // Inicializar color difuso
    oDiffuse = float4(0, 0, 0, 0);
    
    // Calcular iluminación difusa de ambas luces
    if (light0Enabled > 0.0f)
    {
        float3 light0Dir = Light0Position - WorldPos;
        float distSq0 = dot(light0Dir, light0Dir);
        
        // Validación de rango
        if (Light0Range.x > 0.0f && distSq0 > (Light0Range.x * Light0Range.x))
        {
            // Fuera de rango, solo agregar ambiente
            oDiffuse += Light0Ambient * MaterialAmbient;
        }
        else
        {
            // Dentro de rango, calcular iluminación completa
            const float MIN_DIST_SQ = 0.0001f;
            distSq0 = max(distSq0, MIN_DIST_SQ);
            float invDist0 = rsqrt(distSq0);
            float3 normalizedLight0Dir = light0Dir * invDist0;
            
            // Atenuación
            float attenuation0 = 1.0f / dot(dst(distSq0, invDist0).xyz, Light0Attenuation.xyz);
            attenuation0 = min(attenuation0, 100.0f);
            
            // Iluminación difusa
            float NdotL0 = max(dot(WorldNormal, normalizedLight0Dir), 0.0f);
            oDiffuse += Light0Diffuse * NdotL0 * attenuation0;
            oDiffuse += Light0Ambient * MaterialAmbient;
        }
    }
    
    if (light1Enabled > 0.0f)
    {
        float3 light1Dir = Light1Position - WorldPos;
        float distSq1 = dot(light1Dir, light1Dir);
        
        // Validación de rango
        if (Light1Range.x > 0.0f && distSq1 > (Light1Range.x * Light1Range.x))
        {
            // Fuera de rango, solo agregar ambiente
            oDiffuse += Light1Ambient * MaterialAmbient;
        }
        else
        {
            // Dentro de rango, calcular iluminación completa
            const float MIN_DIST_SQ = 0.0001f;
            distSq1 = max(distSq1, MIN_DIST_SQ);
            float invDist1 = rsqrt(distSq1);
            float3 normalizedLight1Dir = light1Dir * invDist1;
            
            // Atenuación
            float attenuation1 = 1.0f / dot(dst(distSq1, invDist1).xyz, Light1Attenuation.xyz);
            attenuation1 = min(attenuation1, 100.0f);
            
            // Iluminación difusa
            float NdotL1 = max(dot(WorldNormal, normalizedLight1Dir), 0.0f);
            oDiffuse += Light1Diffuse * NdotL1 * attenuation1;
            oDiffuse += Light1Ambient * MaterialAmbient;
        }
    }
    
    // Agregar ambiente global
    oDiffuse += GlobalAmbient * MaterialAmbient;
    
    // Calcular iluminación especular si MaterialPower > 0
    if (MaterialPower.x > 0.0f)
    {
        float3 viewDir = normalize(CameraPosition - WorldPos);
        
        if (light0Enabled > 0.0f)
        {
            float3 light0Dir = Light0Position - WorldPos;
            float distSq0 = dot(light0Dir, light0Dir);
            
            // Validar rango
            if (Light0Range.x <= 0.0f || distSq0 <= (Light0Range.x * Light0Range.x))
            {
                const float MIN_DIST_SQ = 0.0001f;
                distSq0 = max(distSq0, MIN_DIST_SQ);
                float invDist0 = rsqrt(distSq0);
                float3 normalizedLight0Dir = light0Dir * invDist0;
                float NdotL0 = max(dot(WorldNormal, normalizedLight0Dir), 0.0f);
                
                if (NdotL0 > 0.0f)
                {
                    float attenuation0 = 1.0f / dot(dst(distSq0, invDist0).xyz, Light0Attenuation.xyz);
                    attenuation0 = min(attenuation0, 100.0f);
                    
                    float3 halfway0 = normalize(normalizedLight0Dir + viewDir);
                    float NdotH0 = dot(WorldNormal, halfway0);
                    float specular0 = pow(max(NdotH0, 0.0f), MaterialPower.x) * attenuation0;
                    oDiffuse += Light0Specular * MaterialSpecular * specular0;
                }
            }
        }
        
        if (light1Enabled > 0.0f)
        {
            float3 light1Dir = Light1Position - WorldPos;
            float distSq1 = dot(light1Dir, light1Dir);
            
            // Validar rango
            if (Light1Range.x <= 0.0f || distSq1 <= (Light1Range.x * Light1Range.x))
            {
                const float MIN_DIST_SQ = 0.0001f;
                distSq1 = max(distSq1, MIN_DIST_SQ);
                float invDist1 = rsqrt(distSq1);
                float3 normalizedLight1Dir = light1Dir * invDist1;
                float NdotL1 = max(dot(WorldNormal, normalizedLight1Dir), 0.0f);
                
                if (NdotL1 > 0.0f)
                {
                    float attenuation1 = 1.0f / dot(dst(distSq1, invDist1).xyz, Light1Attenuation.xyz);
                    attenuation1 = min(attenuation1, 100.0f);
                    
                    float3 halfway1 = normalize(normalizedLight1Dir + viewDir);
                    float NdotH1 = dot(WorldNormal, halfway1);
                    float specular1 = pow(max(NdotH1, 0.0f), MaterialPower.x) * attenuation1;
                    oDiffuse += Light1Specular * MaterialSpecular * specular1;
                }
            }
        }
    }
    
    // Aplicar color difuso del material
    oDiffuse *= MaterialDiffuse;
    
    // Preservar alpha del material
    oDiffuse.w = MaterialDiffuse.w;
    
    // Pasar coordenadas de textura
    oT0 = T0;
    
    // Calcular fog
    float3 cameraToVertex = WorldPos - CameraPosition;
    float distToCamera = length(cameraToVertex);
    float fogFactor = (Constants.z - distToCamera) * Constants.w;
    fogFactor = saturate(fogFactor);
    float fogEnabled = step(0.0001f, Constants.w);
    oFog = lerp(1.0f, fogFactor, fogEnabled);
}

// ============================================================================
// NOTA: Este archivo contiene solo el Vertex Shader
// Para un Pixel Shader separado, crear MaterialTestPS.hlsl
// ============================================================================

