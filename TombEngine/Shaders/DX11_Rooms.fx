#include "./CameraMatrixBuffer.hlsli"
#include "./VertexInput.hlsli"
#include "./VertexEffects.hlsli"
#include "./Math.hlsli"
#include "./ShaderLight.hlsli"
#include "./AlphaTestBuffer.hlsli"

#define SHADOW_INTENSITY (0.55f)
#define INV_SHADOW_INTENSITY (1.0f - SHADOW_INTENSITY)

struct Sphere
{
    float3 position;
    float radius;
};

cbuffer MiscBuffer : register(b3)
{
	int Caustics;
};

cbuffer ShadowLightBuffer : register(b4)
{
	ShaderLight Light;
	float4x4 LightViewProjections[6];
	int CastShadows;
    int NumSpheres;
    int2 padding;
    Sphere Spheres[16];
};

cbuffer RoomBuffer : register(b5)
{
	float4 AmbientColor;
	uint Water;
};

struct AnimatedFrameUV
{
	float2 topLeft;
	float2 topRight;
	float2 bottomRight;
	float2 bottomLeft;
};

cbuffer AnimatedBuffer : register(b6) {
	AnimatedFrameUV AnimFrames[128];
	uint numAnimFrames;
    uint fps;
}

struct PixelShaderInput
{
	float4 Position: SV_POSITION;
	float3 WorldPosition: POSITION0;
	float3 Normal: NORMAL;
	float2 UV: TEXCOORD0;
	float4 Color: COLOR;
	float3x3 TBN : TBN;
	float Fog : FOG;
	float4 PositionCopy : TEXCOORD1;
};

Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

Texture2D NormalTexture : register(t1);

Texture2D CausticsTexture : register(t2);

Texture2DArray ShadowMap : register(t3);
SamplerComparisonState ShadowMapSampler : register(s3);

struct PixelShaderOutput
{
	float4 Color: SV_TARGET0;
	float4 Depth: SV_TARGET1;
};

PixelShaderInput VS(VertexShaderInput input)
{
	PixelShaderInput output;

	// Setting effect weight on TE side prevents portal vertices from moving.
	// Here we just read weight and decide if we should apply refraction or movement effect.
	float weight = input.Effects.z;

	// Calculate vertex effects
	float wibble = Wibble(input.Effects.xyz, input.Hash);
	float3 pos = Move(input.Position, input.Effects.xyz * weight, wibble);
	float3 col = Glow(input.Color.xyz, input.Effects.xyz, wibble);

	// Refraction
	float4 screenPos = mul(float4(pos, 1.0f), ViewProjection);
	float2 clipPos = screenPos.xy / screenPos.w;

	if (CameraUnderwater != Water)
	{
		float factor = (Frame + clipPos.x * 320);
		float xOffset = (sin(factor * PI / 20.0f)) * (screenPos.z/1024) * 4;
		float yOffset = (cos(factor * PI / 20.0f)) * (screenPos.z/1024) * 4;
		screenPos.x += xOffset * weight;
		screenPos.y += yOffset * weight;
	}
	
	output.Position = screenPos;
	output.Normal = input.Normal;
	output.Color = float4(col, input.Color.w);
	output.PositionCopy = screenPos;

#ifdef ANIMATED
	float speed = fps / 30.0f;
	int frame = (int)(Frame * speed + input.AnimationFrameOffset) % numAnimFrames;
	switch (input.PolyIndex) {
	case 0:
		output.UV = AnimFrames[frame].topLeft;
		break;
	case 1:
		output.UV = AnimFrames[frame].topRight;
		break;
	case 2:
		output.UV = AnimFrames[frame].bottomRight;
		break;
	case 3:
		output.UV = AnimFrames[frame].bottomLeft;
		break;
	}
#else
	output.UV = input.UV;
#endif
	
	output.WorldPosition = input.Position.xyz;

    float3x3 TBN = float3x3(input.Tangent, cross(input.Normal,input.Tangent), input.Normal);
	output.TBN = TBN;

	// Apply distance fog
	float d = length(CamPositionWS.xyz - output.WorldPosition);
	if (FogMaxDistance == 0)
		output.Fog = 1;
	else
		output.Fog = clamp((d - FogMinDistance * 1024) / (FogMaxDistance * 1024 - FogMinDistance * 1024), 0, 1);

	return output;
}

float2 texOffset(int u, int v) {
	return float2(u * 1.0f / SHADOW_MAP_SIZE, v * 1.0f / SHADOW_MAP_SIZE);
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

void doPointLightShadow(float3 worldPos, inout float3 lighting)
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
			//perform PCF filtering on a 4 x 4 texel neighborhood
			// what about borders of cubemap?
            for (y = -1.5; y <= 1.5; y += 1.0)
            {
                for (x = -1.5; x <= 1.5; x += 1.0)
                {
                    sum += ShadowMap.SampleCmpLevelZero(ShadowMapSampler, float3(lightClipSpace.xy + texOffset(x, y), i), lightClipSpace.z);
                }
            }

            shadowFactor = sum / 16.0;

        }
    }
    float distanceFactor = saturate(((distance(worldPos, Light.Position) ) / (Light.Out)));
    lighting *= saturate((shadowFactor + SHADOW_INTENSITY) + (pow(distanceFactor, 4) * INV_SHADOW_INTENSITY));
}


void doBlobShadows(float3 worldPos, inout float3 lighting)
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
        shadowFactor -= factor*shadowFactor;

    }
    shadowFactor = saturate(shadowFactor);
    lighting *= saturate((shadowFactor+SHADOW_INTENSITY));
}

void doSpotLightShadow(float3 worldPos,inout float3 lighting)
{
    float4 lightClipSpace = mul(float4(worldPos, 1.0f), LightViewProjections[0]);
    lightClipSpace.xyz /= lightClipSpace.w;
    float shadowFactor = 1.0f;
    if (lightClipSpace.x >= -1.0f && lightClipSpace.x <= 1.0f &&
			lightClipSpace.y >= -1.0f && lightClipSpace.y <= 1.0f &&
			lightClipSpace.z >= 0.0f && lightClipSpace.z <= 1.0f)
    {
        lightClipSpace.x = lightClipSpace.x / 2 + 0.5;
        lightClipSpace.y = lightClipSpace.y / -2 + 0.5;
        float sum = 0;
        float x, y;
			//perform PCF filtering on a 4 x 4 texel neighborhood
        for (y = -1.5; y <= 1.5; y += 1.0)
        {
            for (x = -1.5; x <= 1.5; x += 1.0)
            {
                sum += ShadowMap.SampleCmpLevelZero(ShadowMapSampler, float3(lightClipSpace.xy + texOffset(x, y),0), lightClipSpace.z);
            }
        }

        shadowFactor = sum / 16.0;
    }
    float distanceFactor = saturate(((distance(worldPos, Light.Position)) / (Light.Out)));
	//Fade out at the borders of the sampled texture
    float angleFactor = min(max(sin(lightClipSpace.x * PI)*1.2, 0), 1);
    lighting *= saturate((shadowFactor + SHADOW_INTENSITY) + (pow(distanceFactor, 4) * (1 - angleFactor) * INV_SHADOW_INTENSITY));
}

PixelShaderOutput PS(PixelShaderInput input)
{
	PixelShaderOutput output;

	output.Color = Texture.Sample(Sampler, input.UV);
	
	DoAlphaTest(output.Color);

	float3 Normal = NormalTexture.Sample(Sampler,input.UV).rgb;
	Normal = Normal * 2 - 1;
	Normal = normalize(mul(Normal, input.TBN));

	float3 lighting = input.Color.xyz;
	bool doLights = true;

	if (CastShadows)
	{
        if (Light.Type == LT_POINT)
        {
            doPointLightShadow(input.WorldPosition,lighting);

        }
        else if (Light.Type == LT_SPOT)
        {
            doSpotLightShadow(input.WorldPosition,lighting);

        }
	}
	
    doBlobShadows(input.WorldPosition, lighting);

	if (doLights)
	{
		for (int i = 0; i < NumLights; i++)
		{
			float3 lightPos = Lights[i].Position.xyz;
			float3 color = Lights[i].Color.xyz;
			float radius = Lights[i].Out;

			float3 lightVec = (lightPos - input.WorldPosition);
			float distance = length(lightVec);
			if (distance > radius)
				continue;

			lightVec = normalize(lightVec);
			float d = saturate(dot(Normal, -lightVec ));
			if (d < 0)
				continue;
			
			float attenuation = pow(((radius - distance) / radius), 2);

			lighting += color * attenuation * d;
		}
	}

	if (Caustics)
	{
		float3 position = input.WorldPosition.xyz;
		float3 normal = Normal;

		float fracX = position.x - floor(position.x / 2048.0f) * 2048.0f;
		float fracY = position.y - floor(position.y / 2048.0f) * 2048.0f;
		float fracZ = position.z - floor(position.z / 2048.0f) * 2048.0f;

		float attenuation = saturate(dot(float3(0.0f, 1.0f, 0.0f), normal));

		float3 blending = abs(normal);
		blending = normalize(max(blending, 0.00001f));
		float b = (blending.x + blending.y + blending.z);
		blending /= float3(b, b, b);

		float3 p = float3(fracX, fracY, fracZ) / 2048.0f;
		float3 xaxis = CausticsTexture.Sample(Sampler, p.yz).xyz; 
		float3 yaxis = CausticsTexture.Sample(Sampler, p.xz).xyz;  
		float3 zaxis = CausticsTexture.Sample(Sampler, p.xy).xyz;  

		lighting += float3((xaxis * blending.x + yaxis * blending.y + zaxis * blending.z).xyz) * attenuation * 2.0f;
	}
	
	output.Color.xyz = saturate(output.Color.xyz * lighting);

	output.Depth = output.Color.w > 0.0f ?
		float4(input.PositionCopy.z / input.PositionCopy.w, 0.0f, 0.0f, 1.0f) :
		float4(0.0f, 0.0f, 0.0f, 0.0f);

	if (FogMaxDistance != 0)
		output.Color.xyz = lerp(output.Color.xyz, FogColor.xyz, input.Fog);

	return output;
}
