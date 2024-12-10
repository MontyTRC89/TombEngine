#include "./VertexInput.hlsli"
#include "./CBCamera.hlsli"
#include "./CBPostProcess.hlsli"

#define BLUR_KERNEL_SIZE 5
#define BLUR_SIGMA 3.0
#define BLUR_BSIGMA 0.3

#define UPSCALE_KERNEL_SIZE 5
#define UPSCALE_SPATIAL_SIGMA 2.0  
#define UPSCALE_DEPTH_SIGMA 0.3  

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

RWTexture2D<float4> UpscaledSSAOTexture : register(u0);

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
    float3 encodedNormal = NormalsTexture.Sample(NormalsSampler, input.UV).xyz;

    // Let's avoid SSAO on the skybox and on surfaces with no normals
    if (length(encodedNormal) <= 0.0001f)
	{
		return float4(1.0f, 1.0f, 1.0f, 1.0f);
	}

    float3 normal = DecodeNormal(encodedNormal);
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

float normpdf(float x, float sigma)
{
    return 0.39894 * exp(-0.5 * x * x / (sigma * sigma)) / sigma;
}

float PSBlur(PixelShaderInput input) : SV_Target
{
    float2 texelSize = 1.0f / float2(ViewportWidth, ViewportHeight);
    float result = 0.0f;

    const int kernelSize = (BLUR_KERNEL_SIZE - 1) / 2;
    float kernel[BLUR_KERNEL_SIZE];
    float bZ = 0.0;

    // Create the 1-D kernel
    for (int j = 0; j <= kernelSize; j++)
    {
        kernel[kernelSize + j] = kernel[kernelSize - j] = normpdf(float(j), BLUR_SIGMA);
    }

    float color;
    float baseColor = SSAOTexture.Sample(SSAOSampler, input.UV).x;
    float gfactor;
    float bfactor;
    float bZnorm = 1.0 / normpdf(0.0, BLUR_BSIGMA);

    // Read out the texels
    for (int i = -kernelSize; i <= kernelSize; i++)
    {
        for (int j = -kernelSize; j <= kernelSize; j++)
        {
            // Color at pixel in the neighborhood
            float2 offset = float2(i, j) * texelSize;
            color = SSAOTexture.Sample(SSAOSampler, input.UV + offset).x;

            // Compute both the gaussian smoothed and bilateral
            gfactor = kernel[kernelSize + j] * kernel[kernelSize + i];
            bfactor = normpdf(color - baseColor, BLUR_BSIGMA) * bZnorm * gfactor;
            bZ += bfactor;

            result += bfactor * color;
        }
    }

    return (result / bZ);
}

// Function to compute the Gaussian weight based on spatial distance
float GaussianWeight(float2 delta, float sigma)
{
    return exp(-(delta.x * delta.x + delta.y * delta.y) / (2.0 * sigma * sigma));
}

// Function to compute the bilateral weight based on depth and normal similarity
float BilateralWeight(float3 normal1, float3 normal2, float depth1, float depth2, float depthSigma)
{
    // Calculate the spatial weight (based only on the position of the pixels)
    float spatialWeight = GaussianWeight(float2(0.0f, 0.0f), UPSCALE_SPATIAL_SIGMA);

    // Compute the depth difference (the closer the depth, the more influence the pixel has)
    float depthDifference = abs(depth1 - depth2);
    float depthWeight = exp(-(depthDifference * depthDifference) / (2.0 * depthSigma * depthSigma));

    // The bilateral weight is a combination of the spatial and depth weights
    return spatialWeight * depthWeight;
}

// Upscaling function for SSAO using bilateral filtering to reduce edge bleeding
float4 PSUpscale(PixelShaderInput input) : SV_Target
{
    // Calculate the offset for sampling neighbors (assuming a 5x5 kernel)
    float2 texelSize = float2(1.0f / (ViewSize.x / 2.0f), 1.0f / (ViewSize.y / 2.0f));

    // Initialize accumulators for the SSAO value
    float accumulatedSSAO = 0.0f;
    float weightSum = 0.0f;

    // Get the current pixel's normal and depth for comparison
    float3 centerNormal = NormalsTexture.Sample(NormalsSampler, input.UV);
    float centerDepth = DepthTexture.Sample(DepthSampler, input.UV);

    // Loop through the neighboring pixels in a 5x5 kernel (bilateral filter)
    for (int dy = -UPSCALE_KERNEL_SIZE / 2; dy <= UPSCALE_KERNEL_SIZE / 2; ++dy)
    {
        for (int dx = -UPSCALE_KERNEL_SIZE / 2; dx <= UPSCALE_KERNEL_SIZE / 2; ++dx)
        {
            // Calculate the texture coordinates for the neighboring pixel
            float2 neighborCoord = input.UV + float2(dx, dy) * texelSize;

            // Make sure we are within bounds of the texture
            if (neighborCoord.x >= 0.0f && neighborCoord.x <= 1.0f && neighborCoord.y >= 0.0f && neighborCoord.y <= 1.0f)
            {
                // Sample the SSAO, normal, and depth from the neighboring pixel
                float neighborSSAO = SSAOTexture.Sample(SSAOSampler, neighborCoord);
                float3 neighborNormal = NormalsTexture.Sample(NormalsSampler, neighborCoord);
                float neighborDepth = DepthTexture.Sample(DepthSampler, neighborCoord);

                // Compute the bilateral weight based on depth and normal similarity
                float weight = BilateralWeight(centerNormal, neighborNormal, centerDepth, neighborDepth, UPSCALE_DEPTH_SIGMA);

                // Accumulate the weighted SSAO values
                accumulatedSSAO += neighborSSAO * weight;
                weightSum += weight;
            }
        }
    }

    // Normalize the accumulated SSAO value
    if (weightSum > 0.0f)
    {
        accumulatedSSAO /= weightSum;
    }

    return float4(accumulatedSSAO, 0, 0, 1);
}