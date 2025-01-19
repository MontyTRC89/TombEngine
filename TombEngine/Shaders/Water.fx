#include "./CBCamera.hlsli"
#include "./Blending.hlsli"
#include "./Math.hlsli"

struct VertexShaderInput
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

SSRPixelShaderInput VSSSR(VertexShaderInput input)
{
    SSRPixelShaderInput output;

    output.Position = float4(input.Position, 1.0f);
    output.Color = input.Color;
    output.UV = input.UV;

    return output;
}

Texture2D ColorTexture : register(t0);
SamplerState ColorSampler : register(s0);

Texture2D DepthTexture : register(t6);
SamplerState DepthSampler : register(s6);

Texture2D<uint> HashTexture : register(t11);

RWTexture2D<uint> HashBufferUAV : register(u1);

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
    return ndc * 0.5f + 0.5f;
}

float4 PSSSRProjectHash(SSRPixelShaderInput input) : SV_Target0
{
    float3 positionWS = Unproject(input.UV);
    float3 reflectedPositionWS = float3(positionWS.x, 2 * WaterHeight - positionWS.y, positionWS.z);
    float2 reflectedPositionUV = Project(reflectedPositionWS);
 
    uint2 sourcePositionInPixels = input.UV * uint2(ViewSize);
    uint2 reflectedPositionInPixels = reflectedPositionUV * uint2(ViewSize);
 
    uint hash = (sourcePositionInPixels.y << 16) | sourcePositionInPixels.x;
    InterlockedMax(HashBufferUAV[reflectedPositionInPixels], hash);
    
    return 0;
}

float4 PSSSRResolveHash(SSRPixelShaderInput input) : SV_Target0
{
    uint hash = HashTexture.Load(int3(input.UV * uint2(ViewSize), 0));

    uint x = hash & 0xFFFF;
    uint y = hash >> 16;

    if (hash != 0)
    {
        float4 color = ColorTexture.Load(int3(x, y, 0));
        return color;
    }
    else
    {
        return float4(0, 0, 0, 0);
    }
}