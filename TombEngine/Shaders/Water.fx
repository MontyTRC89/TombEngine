#include "./CBCamera.hlsli"
#include "./CBPostProcess.hlsli"
#include "./CBItem.hlsli"
#include "./CBInstancedStatics.hlsli"
#include "./Blending.hlsli"
#include "./Math.hlsli"
#include "./VertexInput.hlsli"
#include "./PostProcessVertexInput.hlsli"
#include "./CBWater.hlsli"
#include "./VertexEffects.hlsli"

#define MAX_REFLECTION_TARGETS       8
#define BORDERS_BIAS                 0.008f

struct WaterPixelShaderInput
{
    float4 Position : SV_POSITION;
    float3 WorldPosition : POSITION0;
    float2 UV : TEXCOORD0;
    float4 Color : COLOR;
    float4 PositionCopy : TEXCOORD1;
    float4 ReflectedPosition : TEXCOORD2;
};

struct WaterReflectionsGeometryShaderInput
{
    float4 Position : SV_POSITION;
    float3 WorldPosition : WORLDPOSITION;
    float2 UV : TEXCOORD0;
    float4 Color : COLOR;
    float4 PositionCopy : TEXCOORD1;
    uint InstanceID : SV_InstanceID;
};

struct WaterReflectionsPixelShaderInput
{
    float4 Position : SV_POSITION;
    float3 WorldPosition : POSITION0;
    float2 UV : TEXCOORD0;
    float4 Color : COLOR;
    float4 PositionCopy : TEXCOORD1;
    uint InstanceID : SV_InstanceID;
    uint RTIndex : SV_RenderTargetArrayIndex;
};

Texture2D ColorTexture : register(t0);
SamplerState ColorSampler : register(s0);

Texture2D DepthTexture : register(t6);
SamplerState DepthSampler : register(s6);

Texture2D WaterNormalMapTexture : register(t12);
SamplerState WaterNormalMapSampler : register(s12);

Texture2D WaterRefractionTexture : register(t13);
SamplerState WaterRefractionSampler : register(s13);

Texture2DArray WaterReflectionTexture : register(t14);
SamplerState WaterReflectionSampler : register(s14);

Texture2D WaterDistortionMapTexture : register(t15);
SamplerState WaterDistortionMapSampler : register(s15);

Texture2D WaterFoamTexture : register(t11);
SamplerState WaterFoamSampler : register(s11);

// Utility functions
float3 Unproject(float2 uv)
{
    float x = uv.x * 2.0f - 1.0f;
    float y = (1.0f - uv.y) * 2.0f - 1.0f;
    float z = DepthTexture.Sample(DepthSampler, uv).x;

    float4 projectedPosition = float4(x, y, z, 1.0f);
    float4 position = mul(projectedPosition, InverseProjection);

    float4 viewPosition = position / position.w;
    
    return mul(viewPosition, InverseView).xyz;
}

// Main water surface shaders
WaterPixelShaderInput VSWater(VertexShaderInput input)
{
    WaterPixelShaderInput output;

    float weight = input.Effects.z;
    float wibble = Wibble(input.Effects.xyz, input.Hash);
    float3 pos = Move(input.Position, input.Effects.xyz * weight, wibble);
    
    output.Position = mul(float4(pos, 1.0f), ViewProjection);
    output.Color = input.Color;
    output.UV = input.UV;
    output.PositionCopy = output.Position;
    output.WorldPosition = pos;
    output.ReflectedPosition = mul(float4(pos, 1.0f), mul(WaterReflectionView, Projection));
    
    return output;
}

#ifdef CAMERA_UNDERWATER
float ComputeTotalInternalReflection(float3 viewDir)
{
    const float n_air = 1.0;
    const float n_water = 1.33;
    float sinThetaC = n_air / n_water;
    float cosThetaC = sqrt(1.0 - sinThetaC * sinThetaC);
    float cosThetaV = abs(viewDir.y);
    return smoothstep(cosThetaC + 0.1, cosThetaC - 0.1, cosThetaV);
}
#else
float FresnelSchlick(float3 viewDir)
{
    const float n_air = 1.0;
    const float n_water = 1.33;
    float F0 = pow((n_water - n_air) / (n_water + n_air), 2.0);
    float cosTheta = abs(viewDir.y);
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
#endif

static const float WAVELENGTH = 512.0; 
static const float WAVE_SPEED = 0.1;
static const float ATTENUATION = 0.01;

// Funzione aggiornata per onde in propagazione
float CircularWave(float2 uv, float2 origin, float amplitude, float normTime, float waveSize)
{
    // d è la distanza dal punto di origine
    float d = length(uv - origin);

    // Numero d'onda: k = 2*pi / wavelength
    float k = (2.0 * 3.14159) / WAVELENGTH;

    // Il fronte dell'onda si sposta nel tempo (l'onda si propaga)
    float phase = k * (d - WAVE_SPEED * (1.0 - normTime));

    // Decadimento dell'ampiezza: l'ampiezza diminuisce linearmente nel tempo
    float adjustedAmplitude = amplitude * (1.0 - normTime);

    // Equazione dell'onda con attenuazione
    float wave = adjustedAmplitude * cos(phase) * exp(-ATTENUATION * d);

    // Applichiamo una maschera per limitare l'effetto all'interno di waveSize
    wave *= saturate(1.0 - d / waveSize);

    return wave;
}


float4 PSWater(WaterPixelShaderInput input) : SV_Target
{
    float4 output;
    
    float waterSceneDepth = input.PositionCopy.z / input.PositionCopy.w;
    
    // Refraction
    float4 refractionUV = input.PositionCopy;
    refractionUV.xy /= refractionUV.w;
    refractionUV.x = 0.5f * refractionUV.x + 0.5f;
    refractionUV.y = -0.5f * refractionUV.y + 0.5f;

    // Reflection
    float4 reflectionUV = input.ReflectedPosition;
    reflectionUV.xy /= reflectionUV.w;
    reflectionUV.x = 0.5f * reflectionUV.x + 0.5f;
    reflectionUV.y = -0.5f * reflectionUV.y + 0.5f;
    
    // Store old refraction UV for using them in the case we are near the borders
    float2 oldRefractionUV = refractionUV.xy;
    
    // Distor refraction UV
    float time = Frame / 1200.0f;
    float2 mappedUV = frac(input.WorldPosition.xyz / 8192.0f).xz;
    float2 distortion = (WaterDistortionMapTexture.Sample(WaterDistortionMapSampler, mappedUV).xy * 2.0 - 1.0) * WaveStrength;
    float2 distortedTexCoords = WaterDistortionMapTexture.Sample(WaterDistortionMapSampler,
        float2(mappedUV.x + time, mappedUV.y)) * 0.001;
    distortedTexCoords = mappedUV + float2(distortedTexCoords.x, distortedTexCoords.y + time);
    float2 totalDistortion = (WaterDistortionMapTexture.Sample(WaterDistortionMapSampler, distortedTexCoords).rg * 2.0 - 1.0) * WaveStrength;
    
    refractionUV.xy += totalDistortion;
    refractionUV.xy += totalDistortion;
    
#ifdef NEW_RIPPLES
    float wavesDistortion = 0.0;

    for (int i = 0; i < RipplesCount; i++)
    {
        float2 origin = RipplesPosSize[i].xz;
        float amplitude = 256.0f;
        float normTime = RipplesParameters[i].y;

        float waveSize = 1024.0;

        wavesDistortion += CircularWave(input.WorldPosition.xz, origin, amplitude, normTime, waveSize);
    }
    
    // Distorsione delle UV in base alle onde
    refractionUV.xy += float2(wavesDistortion * RIPPLE_ATTENUATION, wavesDistortion * RIPPLE_ATTENUATION);
    reflectionUV.xy += float2(wavesDistortion * RIPPLE_ATTENUATION, wavesDistortion * RIPPLE_ATTENUATION);
 #endif
    
    // Reconstruct world position for avoiding sampling wrong refraction color
    float3 worldPosition = Unproject(refractionUV.xy);
    
#ifdef CAMERA_UNDERWATER
    if (worldPosition.y > WaterLevels[WaterPlaneIndex].x || 
#else
    if (worldPosition.y < WaterLevels[WaterPlaneIndex].x ||
#endif
        refractionUV.x < BORDERS_BIAS || refractionUV.x > (1.0f - BORDERS_BIAS) ||
        refractionUV.y < BORDERS_BIAS || refractionUV.y > (1.0f - BORDERS_BIAS))
    {
        refractionUV.xy = oldRefractionUV;
    }

    // Sample refraction and reflections colors
    float3 refractedColor = WaterRefractionTexture.Sample(WaterRefractionSampler, refractionUV.xy);
    float3 reflectedColor = WaterReflectionTexture.Sample(WaterReflectionSampler, float3(reflectionUV.xy, WaterPlaneIndex));
    
    // Specular highlights
    float3 lightDirection = normalize(input.WorldPosition.xyz - LightPosition);
    float3 halfVector = normalize(lightDirection + CamDirectionWS.xyz);
 
    float4 normalMapColor = WaterNormalMapTexture.Sample(WaterNormalMapSampler, distortedTexCoords);
    float3 normal = float3(normalMapColor.r * 2.0 - 1.0, normalMapColor.b, normalMapColor.g * 2.0 - 1.0);
    normal = normalize(normal);
    normal.y = normal.y;
    
    float NdotL = saturate(dot(normal, lightDirection));
    float NdotH = saturate(dot(normal, halfVector));
    float3 specularLight = sign(NdotL) * KSpecular * LightColor * pow(NdotH, Shininess);
    
    // Fresnel
    float3 viewDirection = normalize(CamPositionWS - input.WorldPosition.xyz);
#ifdef CAMERA_UNDERWATER
        float fresnel = ComputeTotalInternalReflection(viewDirection);
#else
        float fresnel = FresnelSchlick(viewDirection);
#endif

    // Final calculation
    output = lerp(float4(refractedColor, 1.0f), float4(reflectedColor, 1.0f), fresnel) + float4(specularLight, 0.0);

    // Extinction
#ifdef CAMERA_UNDERWATER
    float waterDepth = distance(CamPositionWS, input.WorldPosition);
    float3 extinction = exp(-waterDepth * AbsorptionCoefficient * WaterFogDensity * 0.1f);
    output.xyz = lerp(WaterFogColor, output.xyz, extinction);
#else
    float3 groundPosition = Unproject(refractionUV);
    float waterDepth = abs(WaterLevels[WaterPlaneIndex] - groundPosition.y);
    float3 extinction = exp(-waterDepth * AbsorptionCoefficient * WaterDepthScale);
    output.xyz = lerp(WaterFogColor, output.xyz, extinction);
#endif
    
#ifdef WATER_FOAM
    // Foam
    mappedUV = frac(input.WorldPosition.xyz / 1024.0f).xz;
    float foamThreshold = 128.0f;
    float foamFactor = saturate(1.0 - (waterDepth / foamThreshold));  
    float2 foamUV = float2(mappedUV.x + time, mappedUV.y);
    float foamColor = WaterFoamTexture.Sample(WaterFoamSampler, foamUV).x;
    float foamIntensity = saturate(foamFactor);
    float foamFinal = foamIntensity * 0.5f;
    output.xyz = lerp(output.xyz, float3(foamColor, foamColor, foamColor), foamFinal);
    
    // Water fade
    float sceneDepth = DepthTexture.Sample(DepthSampler, oldRefractionUV).x;
    sceneDepth = LinearizeDepth(sceneDepth, NearPlane, FarPlane);
    waterSceneDepth = LinearizeDepth(waterSceneDepth, NearPlane, FarPlane);
    if (waterSceneDepth - sceneDepth > 0.01f)
    {
        discard;
    }
    float fade = (sceneDepth - waterSceneDepth) * 100.0f;
    output.w = min(output.w, fade);
#endif
    
    return output;
}

// Reflection drawing shaders
WaterReflectionsGeometryShaderInput VSRoomsWaterReflections(VertexShaderInput input)
{
    WaterReflectionsGeometryShaderInput output;

    output.PositionCopy = output.Position = float4(0.0f, 0.0f, 0.0f, 0.0f);
    output.WorldPosition = input.Position;
    output.Color = input.Color;
    output.UV = input.UV;
    output.InstanceID = 0;
    
    return output;
}

WaterReflectionsGeometryShaderInput VSSkyWaterReflections(VertexShaderInput input)
{
    WaterReflectionsGeometryShaderInput output;
    
    output.PositionCopy = output.Position = float4(0.0f, 0.0f, 0.0f, 0.0f);
    output.WorldPosition = input.Position;
    output.Color = input.Color;
    output.UV = input.UV;
    output.InstanceID = 0;
    
    return output;
}

WaterReflectionsGeometryShaderInput VSItemsWaterReflections(VertexShaderInput input)
{
    WaterReflectionsGeometryShaderInput output;
    
    float4x4 world = mul(Bones[input.Bone], World);
    
    output.PositionCopy = output.Position = float4(0.0f, 0.0f, 0.0f, 0.0f);
    output.WorldPosition = mul(float4(input.Position, 1.0f), world);
    output.Color = input.Color;
    output.UV = input.UV;
    output.InstanceID = 0;
    
    return output;
}

WaterReflectionsGeometryShaderInput VSInstancedStaticsWaterReflections(VertexShaderInput input, uint InstanceID : SV_InstanceID)
{
    WaterReflectionsGeometryShaderInput output;
    
    output.PositionCopy = output.Position = float4(0.0f, 0.0f, 0.0f, 0.0f);
    output.WorldPosition = mul(float4(input.Position, 1.0f), StaticMeshes[InstanceID].World);
    output.Color = input.Color;
    output.UV = input.UV;
    output.InstanceID = InstanceID;
    
    return output;
}

WaterReflectionsGeometryShaderInput VSBlurWaterReflections(PostProcessVertexShaderInput input)
{
    WaterReflectionsGeometryShaderInput output;
    
    output.PositionCopy = output.Position = float4(input.Position, 1.0f);
    output.WorldPosition = float3(0.0f, 0.0f, 0.0f);
    output.Color = input.Color;
    output.UV = input.UV;
    output.InstanceID = 0;
    
    return output;
}

[maxvertexcount(12)]
void GSWaterReflections(triangle WaterReflectionsGeometryShaderInput input[3], inout TriangleStream<WaterReflectionsPixelShaderInput> outputStream)
{
    for (uint i = 0; i < MAX_REFLECTION_TARGETS; i++)
    {
        WaterReflectionsPixelShaderInput output;
        
        for (int j = 0; j < 3; j++)
        {
            output.Position = mul(mul(float4(input[j].WorldPosition, 1.0f), WaterReflectionViews[i]), Projection);
            output.PositionCopy = output.Position;
            output.UV = input[j].UV;
            output.Color = input[j].Color;
            output.WorldPosition = input[j].WorldPosition;
            output.RTIndex = i;
            output.InstanceID = input[j].InstanceID;
            outputStream.Append(output);
        }
        outputStream.RestartStrip();
    }
}

[maxvertexcount(12)]
void GSSkyWaterReflections(triangle WaterReflectionsGeometryShaderInput input[3], inout TriangleStream<WaterReflectionsPixelShaderInput> outputStream)
{
    for (uint i = 0; i < MAX_REFLECTION_TARGETS; i++)
    {
        WaterReflectionsPixelShaderInput output;
        
        for (int j = 0; j < 3; j++)
        {
            float4 worldPosition = mul(float4(input[j].WorldPosition, 1.0f), SkyWorldMatrices[i]);
            
            output.Position = mul(mul(worldPosition, WaterReflectionViews[i]), Projection);
            output.PositionCopy = output.Position;
            output.UV = input[j].UV;
            output.Color = input[j].Color * SkyColor;
            output.WorldPosition = worldPosition;
            output.RTIndex = i;
            output.InstanceID = input[j].InstanceID;
            outputStream.Append(output);
        }
        outputStream.RestartStrip();
    }
}

[maxvertexcount(12)]
void GSBlurWaterReflections(triangle WaterReflectionsGeometryShaderInput input[3], inout TriangleStream<WaterReflectionsPixelShaderInput> outputStream)
{
    for (uint i = 0; i < MAX_REFLECTION_TARGETS; i++)
    {
        WaterReflectionsPixelShaderInput output;
        
        for (int j = 0; j < 3; j++)
        {
            output.Position = input[j].Position;
            output.PositionCopy = output.Position;
            output.UV = input[j].UV;
            output.Color = input[j].Color;
            output.WorldPosition = input[j].WorldPosition;
            output.RTIndex = i;
            output.InstanceID = input[j].InstanceID;
            outputStream.Append(output);
        }
        outputStream.RestartStrip();
    }
}

float4 PSSkyWaterReflections(WaterReflectionsPixelShaderInput input) : SV_Target
{
    float4 output = ColorTexture.Sample(ColorSampler, input.UV);

    DoAlphaTest(output);

    output.xyz *= input.Color.xyz;

    return output;
}

float4 PSWaterReflections(WaterReflectionsPixelShaderInput input) : SV_Target
{
    float4 output = ColorTexture.Sample(ColorSampler, input.UV);

    float wl = WaterLevels[input.RTIndex].x;
    
#ifdef CAMERA_UNDERWATER
        if (input.WorldPosition.y <= wl)
        {
            discard;
        }
#else
        if (input.WorldPosition.y >= wl)
        {
            discard;
        }
#endif
    
    DoAlphaTest(output);

    output.xyz *= input.Color.xyz;

    return output;
}

float4 PSBlurWaterReflections(WaterReflectionsPixelShaderInput input) : SV_Target
{
    float3 color = float3(0, 0, 0);
    float weightSum = 0.0;
    int blurFactor = 3;
	
    for (int i = -blurFactor; i <= blurFactor; i++)
    {
        float weight = exp(-0.5 * (i / (float) blurFactor) * (i / (float) blurFactor)); // Gaussian weight
        float2 offset = float2(i * InvViewSize.x, 0.0);
        color += WaterReflectionTexture.Sample(WaterReflectionSampler, float3(input.UV + offset, input.RTIndex)).rgb * weight;
        weightSum += weight;
    }

    return float4(color / weightSum, 1.0);
}

/*
[numthreads(64, 1, 1)]
void CSWaves(uint id : SV_DispatchThreadID)
{
    if (id >= MAX_WAVES)
        return;

    WavesParticle particle = WavesParticlesBuffer[id];

    // Se la particella è "morta", sostituiamola con una nuova se c'è un oggetto in movimento
    if (particle.timeToLive <= 0.0f)
    {
        particle.position = newParticlePosition.xz;
        particle.amplitude = newParticleAmplitude;
        particle.timeToLive = 5.0f; // Le onde durano 5 secondi
    }
    else
    {
        // Altrimenti, facciamo decadere l'onda
        particle.timeToLive -= deltaTime;
        particle.amplitude *= 0.98; // Smorzamento
    }

    WavesParticlesBuffer[id] = particle;
}*/