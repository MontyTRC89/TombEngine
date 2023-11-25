#include "./VertexInput.hlsli"
#include "./CameraMatrixBuffer.hlsli"

cbuffer PostProcessBuffer : register(b7)
{
    float CinematicBarsHeight;
    float ScreenFadeFactor;
    int ViewportWidth;
    int ViewportHeight;
    float4 SSAOKernel[64];
};

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

Texture2D SSAOTexture : register(t9);
SamplerState SSAOSampler : register(s9);

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

float PS(PixelShaderInput input) : SV_Target
{
    float4 output;

    float2 noiseScale = float2(ViewportWidth / 4.0f, ViewportHeight / 4.0f);

    float3 position = ReconstructPositionFromDepth(input.UV);
    float3 normal = DecodeNormal(NormalsTexture.Sample(NormalsSampler, input.UV).xyz);
    float3 randomVec = NoiseTexture.Sample(NoiseSampler, input.UV * noiseScale).xyz;

    float3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    float3 bitangent = cross(normal, tangent);
    float3x3 TBN = float3x3(tangent, bitangent, normal);

    float occlusion = 0.0f;
    int kernelSize = 64;
    float radius = 128.0f;
    float bias = 8.0f;

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

float PSBlur(PixelShaderInput input) : SV_Target
{
    float2 texelSize = 1.0f / float2(ViewportWidth, ViewportHeight);
    float result = 0.0f;

    for (int x = -2; x < 2; x++)
    {
        for (int y = -2; y < 2; y++)
        {
            float2 offset = float2(x, y) * texelSize;
            result += SSAOTexture.Sample(SSAOSampler, input.UV + offset).x;
        }
    }
    
    return (result / (4.0 * 4.0));
}
