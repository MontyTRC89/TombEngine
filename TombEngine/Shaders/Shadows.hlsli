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

void DoPointLightShadow(float3 worldPos, inout float3 lighting)
{
    float shadowFactor = 1.0f;
    for (int i = 0; i < 6; i++)
    {
        float3 dir = normalize(worldPos - Light.Position);
        int face = GetCubeFaceIndex(dir);
        //debug coloring
        /*
        switch (face)
        {
            case 0:
                lighting += float3(0.2, 0, 0);
                break;
            case 1:
                lighting += float3(0.1, 0, 0);

                break;
            case 2:
                lighting += float3(0, 0.2, 0);

                break;
            case 3:
                lighting += float3(0, 0.1, 0);

                break;
            case 4:
                lighting += float3(0, 0, 0.2);

                break;
            default:
                lighting += float3(0, 0, 0.1);

            break;
        }
        */
        float2 uv = GetCubeUVFromDir(face, dir);
        float4 lightClipSpace = mul(float4(worldPos, 1.0f), LightViewProjections[i]);
        lightClipSpace.xyz /= lightClipSpace.w;
        if (lightClipSpace.x >= -1.0f && lightClipSpace.x <= 1.0f &&
            lightClipSpace.y >= -1.0f && lightClipSpace.y <= 1.0f &&
            lightClipSpace.z >= 0.0f && lightClipSpace.z <= 1.0f)
        {
            lightClipSpace.x = lightClipSpace.x / 2 + 0.5;
            lightClipSpace.y = lightClipSpace.y / -2 + 0.5;

            float sum = 0;
            float x, y;

            // Perform PCF filtering on a 4 x 4 texel neighborhood
            // what about borders of cubemap?
            for (y = -1.5; y <= 1.5; y += 1.0)
            {
                for (x = -1.5; x <= 1.5; x += 1.0)
                {
                    sum += ShadowMap.SampleCmpLevelZero(ShadowMapSampler, float3(lightClipSpace.xy + TexOffset(x, y), i), lightClipSpace.z);
                }
            }

            shadowFactor = sum / 16.0;
        }
    }
    float distanceFactor = saturate(((distance(worldPos, Light.Position)) / (Light.Out)));
    lighting *= saturate((shadowFactor + SHADOW_INTENSITY) + (pow(distanceFactor, 4) * INV_SHADOW_INTENSITY));
}


void DoBlobShadows(float3 worldPos, inout float3 lighting)
{
    float shadowFactor = 1.0f;
    for (int i = 0; i < NumSpheres; i++)
    {
        Sphere s = Spheres[i];
        float dist = distance(worldPos, s.position);
        if (dist > s.radius)
            continue;
        float radiusFactor = dist / s.radius;
        float factor = 1 - (saturate(radiusFactor));
        shadowFactor -= factor * shadowFactor;

    }
    shadowFactor = saturate(shadowFactor);
    lighting *= saturate((shadowFactor + SHADOW_INTENSITY));
}

void DoSpotLightShadow(float3 worldPos, inout float3 lighting)
{
    float4 lightClipSpace = mul(float4(worldPos, 1.0f), LightViewProjections[0]);
    lightClipSpace.xyz /= lightClipSpace.w;
    float2 shadowUV = lightClipSpace.xy * 0.5f + 0.5f;
    shadowUV.y = (1 - shadowUV.y);
    float shadowFactor = 1.0f;
    if (lightClipSpace.x >= -1.0f && lightClipSpace.x <= 1.0f &&
        lightClipSpace.y >= -1.0f && lightClipSpace.y <= 1.0f &&
        lightClipSpace.z >= 0.0f && lightClipSpace.z <= 1.0f)
    {
        float sum = 0;
        float x, y;
        //perform PCF filtering on a 4 x 4 texel neighborhood
        
        for (y = -1.5; y <= 1.5; y += 1.0)
        {
            for (x = -1.5; x <= 1.5; x += 1.0)
            {
                sum += ShadowMap.SampleCmpLevelZero(ShadowMapSampler, float3(shadowUV.xy + TexOffset(x, y), 0), lightClipSpace.z);
            }
        }
        
        shadowFactor = sum / 16.0;
        
    }
    // Fade out towards the borders of the sampled texture
    // for that we simply compare the distance between the shadow texture UV coordinate we sampled and the center (0.5,0.5) of the shadow texture
    // use pow to "boost" the shadow intensity towards the center
    float angleFactor = saturate(pow(distance(shadowUV.xy, 0.5f) * 2,2.2f));
    
    lighting *= saturate((shadowFactor + SHADOW_INTENSITY) + ((angleFactor) * INV_SHADOW_INTENSITY));
}
