#include "./Blending.hlsli"
#include "./Math.hlsli"
#include "./ShaderLight.hlsli"

#define SHADOW_INTENSITY (0.55f)
#define INV_SHADOW_INTENSITY (1.0f - SHADOW_INTENSITY)

struct Sphere
{
    float3 position;
    float radius;
};

cbuffer ShadowLightBuffer : register(b4)
{
    ShaderLight Light;
    float4x4 LightViewProjections[6];
    int CastShadows;
    int NumSpheres;
    int ShadowMapSize;
    int padding;
    Sphere Spheres[16];
};

Texture2DArray ShadowMap : register(t3);
SamplerComparisonState ShadowMapSampler : register(s3);

float2 TexOffset(int u, int v) 
{
    return float2(u * 1.0f / ShadowMapSize, v * 1.0f / ShadowMapSize);
}

//https://gist.github.com/JuanDiegoMontoya/d8788148dcb9780848ce8bf50f89b7bb
int GetCubeFaceIndex(float3 dir)
{
    float x = abs(dir.x);
    float y = abs(dir.y);
    float z = abs(dir.z);
    if (x > y && x > z)
        return 0 + (dir.x > 0 ? 0 : 1);
    else if (y > z)
        return 2 + (dir.y > 0 ? 0 : 1);
    return 4 + (dir.z > 0 ? 0 : 1);
}

float2 GetCubeUVFromDir(int faceIndex, float3 dir)
{
    float2 uv;
    switch (faceIndex)
    {
    case 0:
        uv = float2(-dir.z, dir.y);
        break; // +X
    case 1:
        uv = float2(dir.z, dir.y);
        break; // -X
    case 2:
        uv = float2(dir.x, dir.z);
        break; // +Y
    case 3:
        uv = float2(dir.x, -dir.z);
        break; // -Y
    case 4:
        uv = float2(dir.x, dir.y);
        break; // +Z
    default:
        uv = float2(-dir.x, dir.y);
        break; // -Z
    }
    return uv * .5 + .5;
}

float3 DoBlobShadows(float3 worldPos, float3 lighting)
{
    float shadowFactor = 1.0f;

    for (int i = 0; i < NumSpheres; i++)
    {
        Sphere s = Spheres[i];
        float dist = distance(worldPos, s.position);
        float insideSphere = saturate(1.0f - step(s.radius, dist)); // Eliminates branching
        float radiusFactor = dist / s.radius;
        float factor = (1.0f - saturate(radiusFactor)) * insideSphere;
        shadowFactor -= factor * shadowFactor;
    }

    shadowFactor = saturate(shadowFactor);
    return lighting * saturate(shadowFactor + SHADOW_INTENSITY);
}

float3 DoShadow(float3 worldPos, float3 normal, float3 lighting, float bias)
{
    if (!CastShadows)
        return lighting;

    if (BlendMode != BLENDMODE_OPAQUE && BlendMode != BLENDMODE_ALPHATEST && BlendMode != BLENDMODE_ALPHABLEND)	
        return lighting;

    float shadowFactor = 1.0f;

    float3 dir = normalize(Light.Position - worldPos);
    float ndot = dot(normal, dir);
    float facingFactor = saturate((ndot - bias) / (1.0f - bias + EPSILON));

    [unroll]
    for (int i = 0; i < 6; i++)
    {
        float4 lightClipSpace = mul(float4(worldPos, 1.0f), LightViewProjections[i]);
        lightClipSpace.xyz /= lightClipSpace.w;

        float insideLightBounds =
            step(-1.0f, lightClipSpace.x) * step(lightClipSpace.x, 1.0f) *
            step(-1.0f, lightClipSpace.y) * step(lightClipSpace.y, 1.0f) *
            step( 0.0f, lightClipSpace.z) * step(lightClipSpace.z, 1.0f);

        if (insideLightBounds > 0.0f)
        {
            lightClipSpace.x = lightClipSpace.x / 2 + 0.5;
            lightClipSpace.y = lightClipSpace.y / -2 + 0.5;

            float sum = 0;
            float x, y;

            // Perform PCF filtering on a 4 x 4 texel neighborhood.
            [unroll]
            for (y = -1.5; y <= 1.5; y += 1.0)
            {
                [unroll]
                for (x = -1.5; x <= 1.5; x += 1.0)
                {
                    sum += ShadowMap.SampleCmpLevelZero(ShadowMapSampler, float3(lightClipSpace.xy + TexOffset(x, y), i), lightClipSpace.z);
                }
            }

            shadowFactor = lerp(shadowFactor, sum / 16.0, facingFactor * insideLightBounds);
        }
    }

    float isPoint = step(0.5f, Light.Type == LT_POINT); // 1.0 if LT_POINT, 0.0 otherwise
    float isSpot  = step(0.5f, Light.Type == LT_SPOT);  // 1.0 if LT_SPOT,  0.0 otherwise
    float isOther = 1.0 - (isPoint + isSpot); // 1.0 if neither LT_POINT nor LT_SPOT

    float pointFactor  = saturate(distance(worldPos, Light.Position) / Light.Out);
    float spotFactor   = 1.0f - Luma(DoSpotLight(worldPos, normal, Light));

    float3 pointShadow = lighting * saturate((shadowFactor + SHADOW_INTENSITY) + (pow(pointFactor, 4) * INV_SHADOW_INTENSITY));
    float3 spotShadow  = lighting * saturate((shadowFactor + SHADOW_INTENSITY) + (pow(spotFactor,  4) * INV_SHADOW_INTENSITY));

    return pointShadow * isPoint + spotShadow * isSpot + lighting * isOther;
}