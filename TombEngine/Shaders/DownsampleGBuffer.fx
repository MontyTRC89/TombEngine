#include "./CBCamera.hlsli"

Texture2D DepthTarget : register(t0);
SamplerState DepthTargetSampler : register(s0);

Texture2D NormalTarget : register(t1);
SamplerState NormalTargetSampler : register(s1);

RWTexture2D<float> OutDepth : register(u0); // Downsampled depth
RWTexture2D<float4> OutNormal : register(u1); // Downsampled normal (RGBA format)

float GetDepth(float2 uv)
{
    int3 texelCoord = int3(uv * ViewSize, 1);
    return DepthTarget.Load(texelCoord).r;
    //return DepthTarget.Sample(DepthTargetSampler, uv, 0).r;
}

float3 GetNormal(float2 uv)
{
    int3 texelCoord = int3(uv * ViewSize, 1);
    return normalize(NormalTarget.Load(texelCoord) * 2.0f - 1.0f);
    
    //return normalize(NormalTarget.Sample(NormalTargetSampler, uv, 0) * 2.0f - 1.0f); // Assuming normals are in range [-1, 1]
}

float DownsampleDepth(float2 uv)
{
    float sum = 0.0f;
    const int kernelSize = 3; // 3x3 kernel for downsampling
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            sum += GetDepth(uv + float2(x, y) / ViewSize);
        }
    }
    return sum / (kernelSize * kernelSize);
}

float3 DownsampleNormal(float2 uv)
{
    float3 normalSum = float3(0.0f, 0.0f, 0.0f);
    const int kernelSize = 3; // 3x3 kernel for downsampling
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            normalSum += GetNormal(uv + float2(x, y) / ViewSize);
        }
    }
    return normalize(normalSum / (kernelSize * kernelSize)); // Normalize to preserve directionality
}

[numthreads(16, 16, 1)] // Thread group size (adjust as needed)
void CS(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Calculate the current texel's UV coordinates for downsampling
    float2 uv = dispatchThreadID.xy / ViewSize;

    // Downsample the depth and normal buffers
    float depth = DownsampleDepth(uv);
    float3 normal = DownsampleNormal(uv);
    
    // Output the downsampled depth and normal
    OutDepth[dispatchThreadID.xy] = depth; // Only depth in the R channel
    OutNormal[dispatchThreadID.xy] = float4(normal * 0.5f + 0.5f, 1.0f); // Convert normal back from [-1, 1] to [0, 1] range
}