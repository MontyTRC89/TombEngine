#include "./CBCamera.hlsli"
#include "./CBPostProcess.hlsli"
#include "./CBItem.hlsli"
#include "./CBInstancedStatics.hlsli"
#include "./Blending.hlsli"
#include "./Math.hlsli"
#include "./VertexInput.hlsli"
#include "./PostProcessVertexInput.hlsli"

#define MAX_REFLECTION_TARGETS 8

cbuffer WaterConstantBuffer : register(b2)
{
    float4x4 WaterReflectionViews[8];
    //--
    float4 WaterLevels[8];
    //--
    float4x4 SkyWorldMatrices[8];
    //--
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
    int WaterPlaneIndex;
    //--
    float4 SkyColor;
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

    output.Position = mul(float4(input.Position, 1.0f), ViewProjection);
    output.Color = input.Color;
    output.UV = input.UV;
    output.PositionCopy = output.Position;
    output.WorldPosition = input.Position;
    output.ReflectedPosition = mul(float4(input.Position, 1.0f), mul(WaterReflectionView, Projection));
    
    return output;
}

float4 PSWater(WaterPixelShaderInput input) : SV_Target
{
    float4 output;
    
    // Refraction
    float4 refractionTexCoord;
    refractionTexCoord = input.PositionCopy;

    // Normalized device coordinates
    refractionTexCoord.xy /= refractionTexCoord.w;

    // Adjust offset
    refractionTexCoord.x = 0.5f * refractionTexCoord.x + 0.5f;
    refractionTexCoord.y = -0.5f * refractionTexCoord.y + 0.5f;

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
    
    float2 distortion = (WaterDistortionMapTexture.Sample(WaterDistortionMapSampler, temp).xy * 2.0 - 1.0) * WaveStrength;

    float2 distortedTexCoords = WaterDistortionMapTexture.Sample(WaterDistortionMapSampler,
        float2(temp.x + Frame / 1200.0f, temp.y)) * 0.001;
    distortedTexCoords = temp + float2(distortedTexCoords.x, distortedTexCoords.y + Frame / 1200.0f);
    float2 totalDistortion = (WaterDistortionMapTexture.Sample(WaterDistortionMapSampler, distortedTexCoords).rg * 2.0 - 1.0) * WaveStrength;
    
    refractionTexCoord.xy += totalDistortion;
    reflectionTexCoord.xy += totalDistortion;
    
    float3 worldPosition = Unproject(refractionTexCoord.xy);
    if (worldPosition.y < WaterLevels[WaterPlaneIndex].x || 
        refractionTexCoord.x < 0.01f || refractionTexCoord.x > 0.99f ||
        refractionTexCoord.y < 0.01f || refractionTexCoord.y > 0.99f)
    {
        refractionTexCoord.xy = oldRefractionUV;
    }
    
    float3 refractedColor = WaterRefractionTexture.Sample(WaterRefractionSampler, refractionTexCoord.xy);
    float3 reflectedColor = CameraUnderwater == 1 ? float4(0.0f, 0.0f, 0.0f, 0.0f) : WaterReflectionTexture.Sample(WaterReflectionSampler, float3(reflectionTexCoord.xy, WaterPlaneIndex));
    
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
    float distance = abs(WaterLevels[WaterPlaneIndex] - groundPosition.y);
    float t = smoothstep(0.0, 2048.0, distance); // Graduale aumento fino a 1024
    float extinction = 0.4 * t; // A 1024m è 50% tinta, 50% colore vero
    float3 underwaterColor = lerp(refractedColor, float3(0.0, 0.5, 0.7), extinction);
    //ComputeReflectionFade(float3(0, -1, 0), CamDirectionWS);
    
    if (CameraUnderwater == 1)
    {
        fresnel = 0.0f;
    }
    
    // Final calculation
    output = lerp(float4(underwaterColor, 1.0f), float4(reflectedColor, 1.0f), fresnel) + float4(specularLight, 0.0);

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

    if (input.WorldPosition.y >= WaterLevels[input.RTIndex].x)
    {
        discard;
    }
    
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