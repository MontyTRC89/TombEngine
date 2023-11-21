#include "./VertexInput.hlsli"
#include "./CameraMatrixBuffer.hlsli"

struct PixelShaderInput
{
    float4 Position: SV_POSITION;
    float2 UV: TEXCOORD;
};

Texture2D PositionsAndDepthTexture : register(t0);
SamplerState PositionsAndDepthSampler : register(s0);

Texture2D NormalsTexture : register(t1);
SamplerState NormalsSampler : register(s1);

Texture2D NoiseTexture : register(t2);
SamplerState NoiseSampler : register(s2);

Texture2D PositionsTexture : register(t3);
SamplerState PositionsSampler : register(s3);

cbuffer PostProcessBuffer : register(b7)
{
    float CinematicBarsHeight;
    float ScreenFadeFactor;
    int ViewportWidth;
    int ViewportHeight;
    float4 SSAOKernel[64];
};

PixelShaderInput VS(VertexShaderInput input)
{
    PixelShaderInput output;

    output.Position = float4(input.Position, 1.0f);
    output.UV = input.UV;

    return output;
}

float3 DecodeNormal(float3 n)
{
    return (n * 2.0f - 1.0f);
}

float4 view_pos_from_depth(in float2 coordinate,uniform float4x4 inverse_projection, in float depth) {
    float x = coordinate.x * 2 - 1 ;
    float y = coordinate.y * 2 - 1;
    y*= -1;
    float4 clip_pos = float4(x,y,depth,1.0f);
    float4 view_pos = mul(inverse_projection, clip_pos);
    view_pos.xyz /= view_pos.w;
    return view_pos;
}

float4 PS(PixelShaderInput input) : SV_Target
{
    float4 output;

    float2 noiseScale = float2(ViewSize.x / 4.0f, ViewSize.y / 4.0f);

    float4 gbuffer = NormalsTexture.Sample(NormalsSampler, input.UV);
    float depth = gbuffer.w;

    //float3 fragPos = PositionsAndDepthTexture.Sample(PositionsAndDepthSampler, input.UV).xyz;
    float3 fragPos = view_pos_from_depth(input.UV, InverseProjection, depth);
    float3 normal = gbuffer.xyz;// (NormalsTexture.Sample(NormalsSampler, input.UV).xyz);
    float3 randomVec = NoiseTexture.Sample(NoiseSampler, input.UV * noiseScale).xyz;

    float3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    float3 bitangent = cross(normal, tangent);
    float3x3 TBN = float3x3(tangent, bitangent, normal);

    float occlusion = 0.0f;
    int kernelSize = 64;
    float radius = 0.05f; // 0.5f;
    float bias = 0.000025f;

    for (int i = 0; i < kernelSize; ++i)
    {
        float3 samplePos = mul(SSAOKernel[i], TBN);
        samplePos = fragPos + samplePos * radius;

        float4 offset = float4(samplePos, 1.0);
        offset = mul(offset, Projection);    // from view to clip-space
        offset.xyz /= offset.w;               // perspective divide
        offset.xyz = offset.xyz * 0.5f + 0.5f; // transform to range 0.0 - 1.0  
        offset.y = 1.0f - offset.y;

        float sampleDepth = PositionsAndDepthTexture.Sample(PositionsAndDepthSampler, offset.xy).w;
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));

        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
    }

    occlusion = 1.0 - (occlusion / kernelSize);

    return occlusion;
}
