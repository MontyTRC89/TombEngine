#include "./VertexInput.hlsli"
#include "./CameraMatrixBuffer.hlsli"

struct PixelShaderInput
{
    float4 Position: SV_POSITION;
    float2 UV: TEXCOORD0;
};

Texture2D DepthTexture : register(t0);
SamplerState DepthSampler : register(s0);

Texture2D NormalsTexture : register(t1);
SamplerState NormalsSampler : register(s1);

Texture2D NoiseTexture : register(t2);
SamplerState NoiseSampler : register(s2);

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

float3 ReconstructPositionFromDepth(float2 uv)
{
    float x = uv.x * 2.0f - 1.0f;
    float y = (1.0f - uv.y) * 2.0f - 1.0f;
    float z = DepthTexture.Sample(DepthSampler, uv).x;

    float4 projectedPosition = float4(x, y, z, 1.0f);
    float4 position = mul(projectedPosition, InverseProjection);

    return position.xyz / position.w;
}

float4 PS(PixelShaderInput input) : SV_Target
{
    float4 output;

    float2 noiseScale = float2(ViewSize.x / 4.0f, ViewSize.y / 4.0f);

    float3 position = ReconstructPositionFromDepth(input.UV);
    float3 normal = (NormalsTexture.Sample(NormalsSampler, input.UV).xyz);
    float3 randomVec = NoiseTexture.Sample(NoiseSampler, input.UV * noiseScale).xyz;

    float3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    float3 bitangent = cross(normal, tangent);
    float3x3 TBN = float3x3(tangent, bitangent, normal);

    float occlusion = 0.0f;
    int kernelSize = 64;
    float radius = 64.0f;
    float bias = 4.0f;

    for (int i = 0; i < kernelSize; ++i)
    {
        float3 samplePos = mul(SSAOKernel[i], TBN);
        samplePos = position + samplePos * radius;

        float4 offset = float4(samplePos, 1.0);
        offset = mul(offset, Projection); 
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5f + 0.5f; 
        offset.y = 1.0f - offset.y;

        float sampleDepth = ReconstructPositionFromDepth(offset.xy).z;
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(position.z - sampleDepth));

        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
    }

    occlusion = 1.0 - (occlusion / kernelSize);

    return occlusion;
}
