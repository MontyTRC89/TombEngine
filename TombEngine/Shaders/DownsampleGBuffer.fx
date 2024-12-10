#include "./CBCamera.hlsli"

Texture2D DepthTarget : register(t0);
SamplerState DepthTargetSampler : register(s0);

Texture2D NormalTarget : register(t1);
SamplerState NormalTargetSampler : register(s1);

struct PixelShaderInput
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD0;
    float4 Color : COLOR0;
    float4 PositionCopy : TEXCOORD1;
};

struct PixelShaderOutput
{
    float4 Normals : SV_TARGET0;
    float Depth : SV_TARGET1;
};

#define KERNEL_SIZE 5
#define SPATIAL_SIGMA 2.0  // Spatial sigma for spatial weight (affects spatial distance influence)
#define DEPTH_SIGMA 0.2    // Depth sigma for normal/ depth weight (affects depth difference influence)

float GaussianWeight(float2 delta, float sigma)
{
    return exp(-(delta.x * delta.x + delta.y * delta.y) / (2.0 * sigma * sigma));
}

// Function to compute the bilateral weight based on depth (and normal similarity)
float BilateralWeight(float3 normal1, float3 normal2, float depth1, float depth2, float depthSigma)
{
    // Calculate the spatial weight (based only on the position of the pixels)
    float spatialWeight = GaussianWeight(float2(0.0f, 0.0f), SPATIAL_SIGMA);

    // Compute the depth difference (the closer the depth, the more influence the pixel has)
    float depthDifference = abs(depth1 - depth2);
    float depthWeight = exp(-(depthDifference * depthDifference) / (2.0 * depthSigma * depthSigma));

    // The bilateral weight is a combination of the spatial and depth weights
    return spatialWeight * depthWeight;
}

// Downsampling function for normals and depth using bilateral filtering
PixelShaderOutput PS(PixelShaderInput input)
{
    // Calculate the offset for sampling neighbors (assuming a 5x5 kernel)
    float2 texelSize = InvViewSize;

    // Initialize accumulators for the normal and depth
    float3 accumulatedNormal = float3(0.0f, 0.0f, 0.0f);
    float accumulatedDepth = 0.0f;
    float weightSum = 0.0f;

    // Get the current pixel's depth value for comparison
    float centerDepth = DepthTarget.Sample(DepthTargetSampler, input.UV);

    // Loop through the neighboring pixels in a 5x5 kernel (bilateral filter)
    for (int dy = -KERNEL_SIZE / 2; dy <= KERNEL_SIZE / 2; ++dy)
    {
        for (int dx = -KERNEL_SIZE / 2; dx <= KERNEL_SIZE / 2; ++dx)
        {
            // Calculate the texture coordinates for the neighboring pixel
            float2 neighborCoord = input.UV + float2(dx, dy) * texelSize;

            // Make sure we are within bounds of the texture
            if (neighborCoord.x >= 0.0f && neighborCoord.x <= 1.0f && neighborCoord.y >= 0.0f && neighborCoord.y <= 1.0f)
            {
                // Sample the normal and depth from the neighboring pixel
                float3 neighborNormal = NormalTarget.Sample(NormalTargetSampler, neighborCoord);
                float neighborDepth = DepthTarget.Sample(DepthTargetSampler, neighborCoord);

                // Compute the bilateral weight based on spatial distance and depth difference
                float weight = BilateralWeight(neighborNormal, neighborNormal, centerDepth, neighborDepth, DEPTH_SIGMA);

                // Accumulate the weighted normal and depth values
                accumulatedNormal += neighborNormal * weight;
                accumulatedDepth += neighborDepth * weight;
                weightSum += weight;
            }
        }
    }

    // Average the accumulated values and return them
    if (weightSum > 0.0f)
    {
        accumulatedNormal /= weightSum;
        accumulatedDepth /= weightSum;
    }

    PixelShaderOutput output;
    
    // Store the downsampled normal and depth in the output textures
    output.Normals = float4(accumulatedNormal, 1.0f);
    output.Depth = accumulatedDepth;

    return output;
}