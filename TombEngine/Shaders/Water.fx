#include "./CBCamera.hlsli"
#include "./CBPostProcess.hlsli"
#include "./Blending.hlsli"
#include "./Math.hlsli"
#include "./VertexInput.hlsli"

cbuffer WaterConstantBuffer : register(b2)
{
    float4x4 WaterReflectionView;
    //--
    float3 LightPosition;
    float KSpecular;
	//--
    float3 LightColor;
    float Shininess;
	//--
    float MoveFactor;
    float WaveStrength;
    int WaterLevel;
};

struct SSRVertexShaderInput
{
    float3 Position : POSITION0;
    float2 UV : TEXCOORD0;
    float4 Color : COLOR0;
};

struct SSRPixelShaderInput
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD;
    float4 Color : COLOR;
};

struct WaterPixelShaderInput
{
    float4 Position : SV_POSITION;
    float3 WorldPosition : POSITION0;
    float2 UV : TEXCOORD0;
    float4 Color : COLOR;
    float4 PositionCopy : TEXCOORD1;
    float4 ReflectedPosition : TEXCOORD2;
};

struct WaterPixelShaderOutput
{
    float4 Color : SV_TARGET0;
};

SSRPixelShaderInput VSSSR(SSRVertexShaderInput input)
{
    SSRPixelShaderInput output;

    output.Position = float4(input.Position, 1.0f);
    output.Color = input.Color;
    
   

    return output;
}

WaterPixelShaderInput VSWater(VertexShaderInput input)
{
    WaterPixelShaderInput output;

    output.Position = mul(float4(input.Position, 1.0f), ViewProjection);
    output.Color = input.Color;
    //output.UV = input.UV;
    output.PositionCopy = output.Position;
    output.WorldPosition = input.Position;
    
    float4x4 waterMatrix = mul(WaterReflectionView, Projection);
    output.ReflectedPosition = mul(float4(input.Position, 1.0f), waterMatrix);
    
    float2 waterUvs[4] =
    {
        float2(0.0f, 0.0f),
        float2(1.0f, 0.0f),
        float2(1.0f, 1.0f),
        float2(0.0f, 1.0f)
    };
    
    output.UV = waterUvs[input.PolyIndex];
    
    return output;
}

WaterPixelShaderInput VSWaterReflections(VertexShaderInput input)
{
    WaterPixelShaderInput output;

    output.Position = mul(float4(input.Position, 1.0f), ViewProjection);
    output.Color = input.Color;
    output.UV = input.UV;
    output.PositionCopy = output.Position;
    output.WorldPosition = input.Position;
    
    float4x4 waterMatrix = mul(WaterReflectionView, Projection);
    output.ReflectedPosition = mul(float4(input.Position, 1.0f), waterMatrix);
    
    return output;
}

Texture2D ColorTexture : register(t0);
SamplerState ColorSampler : register(s0);

Texture2D DepthTexture : register(t6);
SamplerState DepthSampler : register(s6);

Texture2D<uint> HashTexture : register(t11);

RWTexture2D<uint> HashBufferUAV : register(u1);

Texture2D WaterNormalMapTexture : register(t12);
SamplerState WaterNormalMapSampler : register(s12);

Texture2D WaterRefractionTexture : register(t13);
SamplerState WaterRefractionSampler : register(s13);

Texture2D WaterReflectionTexture : register(t14);
SamplerState WaterReflectionSampler : register(s14);

Texture2D WaterDistortionMapTexture : register(t15);
SamplerState WaterDistortionMapSampler : register(s15);

// Screen Space Reflections
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

float2 Project(float3 worldPos)
{
    float4 clipPos = mul(float4(worldPos, 1.0f), ViewProjection);
    float2 ndc = clipPos.xy / clipPos.w;
    ndc = ndc * 0.5f + 0.5f;
    ndc.y = 1.0f - ndc.y;
    
    return ndc;

}

float2 DitheredUV(float2 uv, float2 screenSize)
{
    float2 noise = frac(sin(dot(uv, float2(12.9898, 78.233))) * 43758.5453);
    float2 jitter = (noise - 0.5) / screenSize; // Small subpixel jitter
    return uv + jitter;
}

float4 PSSSRProjectHash(SSRPixelShaderInput input) : SV_Target0
{
    float3 positionWS = Unproject(input.UV);
    if (positionWS.y > WaterLevel)
    {
        discard;
    }
    
    float3 reflectedPositionWS = float3(positionWS.x, 2 * WaterLevel - positionWS.y, positionWS.z);
    float2 reflectedPositionUV = Project(reflectedPositionWS);
 
    float heightStretch = (positionWS.y - WaterLevel);
    float angleStretch = saturate(-CamDirectionWS.y);
    float screenStretch = saturate(abs(reflectedPositionUV.x * 2 - 1) - 0.3f);
     
    //reflectedPositionUV.x *= 1 + heightStretch * angleStretch * screenStretch * 0.15f;
 
     float2 viewSize = float2(ViewportWidth, ViewportHeight);
 //   reflectedPositionUV = DitheredUV(reflectedPositionUV, viewSize);
   
    uint2 sourcePositionInPixels = input.UV * uint2(viewSize);
    uint2 reflectedPositionInPixels = reflectedPositionUV * uint2(viewSize);
    
    uint hash = (sourcePositionInPixels.y << 16) | sourcePositionInPixels.x;
    InterlockedMax(HashBufferUAV[reflectedPositionInPixels], hash);
    
    return 0;
}


float4 PSSSRResolveHash(SSRPixelShaderInput input) : SV_Target0
{
    float2 viewSize = float2(ViewportWidth, ViewportHeight);
    
    uint hash = HashTexture.Load(int3(input.UV * uint2(viewSize), 0));

    uint x = hash & 0xFFFF;
    uint y = hash >> 16;
    
    float2 uv = float2(x, y) * 2;
    uv.x /= ViewSize.x;
    uv.y /= ViewSize.xy;
    
    uv = DitheredUV(uv, ViewSize);

    float3 wp = Unproject(input.UV);
    
    if (hash != 0)
    {
        float4 color = ColorTexture.Sample(ColorSampler, uv); 
        //ColorTexture.Load(int3(x * 8, y * 8, 0));
        return color;
    }
    else
    {
        return float4(0, 0, 0, 0);
    }
}

float ComputeReflectionFade(float3 normalWS, float3 viewDirWS)
{
    float fresnelFactor = saturate(dot(normalWS, viewDirWS)); // Dot product for angle effect
    return pow(fresnelFactor, 2.0); // Adjust exponent for smooth fading
}

bool RayPlaneIntersection(float3 rayOrigin, float3 rayDir, float height, out float3 hitPoint)
{
    // Check if ray is parallel to the plane (D.y == 0)
    if (abs(rayDir.y) < 1e-6)
        return false;

    // Compute intersection distance
    float t = (height - rayOrigin.y) / rayDir.y;

    // If t is negative, intersection is behind the camera
    if (t < 0)
        return false;

    // Compute intersection point
    hitPoint = rayOrigin + t * rayDir;
    return true;
}

WaterPixelShaderOutput PSWater(WaterPixelShaderInput input)
{
    WaterPixelShaderOutput output;
    
    // Refraction
    float4 refractionTexCoord;
    refractionTexCoord = input.PositionCopy;

    // Normalized device coordinates
    refractionTexCoord.xy /= refractionTexCoord.w;

    // Adjust offset
    refractionTexCoord.x = 0.5f * refractionTexCoord.x + 0.5f;
    refractionTexCoord.y = -0.5f * refractionTexCoord.y + 0.5f;
    //frac(input.WorldPosition.xyz / 16384.0f).xz;
    // Reflection
    float4 reflectionTexCoord;
    reflectionTexCoord = input.ReflectedPosition;

    // Normalized device coordinates
    reflectionTexCoord.xy /= reflectionTexCoord.w;

    // Adjust offset
    reflectionTexCoord.x = 0.5f * reflectionTexCoord.x + 0.5f;
    reflectionTexCoord.y = -0.5f * reflectionTexCoord.y + 0.5f;
    
    float2 oldRefractionUV = refractionTexCoord.xy;
    
    // Distor refraction UVs
    float2 temp = frac(input.WorldPosition.xyz / 8192.0f).xz;
    
    float2 distortion = (WaterDistortionMapTexture.Sample(WaterDistortionMapSampler, temp).xy 
                    * 2.0 - 1.0) * WaveStrength;

    
    float2 distortedTexCoords = WaterDistortionMapTexture.Sample(WaterDistortionMapSampler,
        float2(temp.x + Frame / 1200.0f, temp.y)) * 0.001;
    distortedTexCoords = temp + float2(distortedTexCoords.x, distortedTexCoords.y + Frame / 1200.0f);
    float2 totalDistortion = (WaterDistortionMapTexture.Sample(WaterDistortionMapSampler, distortedTexCoords).rg * 2.0 - 1.0) * WaveStrength;
    
    refractionTexCoord.xy += totalDistortion;
    reflectionTexCoord.xy += totalDistortion;
    
    float3 worldPosition = Unproject(refractionTexCoord.xy);
    if (worldPosition.y < WaterLevel || refractionTexCoord.x < 0.1f || refractionTexCoord.x > 0.9f || 
        refractionTexCoord.y < 0.1f || refractionTexCoord.y > 0.9f)
    {
        refractionTexCoord.xy = oldRefractionUV;
    }
    
    float3 refractedColor = WaterRefractionTexture.Sample(WaterRefractionSampler, refractionTexCoord.xy);
    float3 reflectedColor = WaterReflectionTexture.Sample(WaterReflectionSampler, reflectionTexCoord.xy);
    
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
    float fresnel = pow(dot(viewDirection, float3(0.0f, -1.0f, 0.0f)), 1.5f);
     
    float3 groundPosition = Unproject(refractionTexCoord);
    float distance = abs(WaterLevel - groundPosition.y);
    float t = smoothstep(0.0, 2048.0, distance); // Graduale aumento fino a 1024
    float extinction = 0.4 * t; // A 1024m è 50% tinta, 50% colore vero
    float3 underwaterColor = lerp(refractedColor, float3(0.0, 0.5, 0.7), extinction);
    //ComputeReflectionFade(float3(0, -1, 0), CamDirectionWS);
    
    // Final calculation
    output.Color = lerp(float4(underwaterColor, 1.0f), float4(reflectedColor, 1.0f), fresnel) + float4(specularLight, 0.0);

    return output;
}

WaterPixelShaderOutput PSWaterReflections(WaterPixelShaderInput input)
{
    WaterPixelShaderOutput output;
    
    output.Color = ColorTexture.Sample(ColorSampler, input.UV);

    if (input.WorldPosition.y >= WaterLevel)
    {
        discard;
    }
    
    DoAlphaTest(output.Color);

    output.Color.xyz *= input.Color.xyz;

    return output;

}