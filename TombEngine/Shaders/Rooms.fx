#include "./CBCamera.hlsli"
#include "./VertexInput.hlsli"
#include "./VertexEffects.hlsli"
#include "./Blending.hlsli"
#include "./Math.hlsli"
#include "./AnimatedTextures.hlsli"
#include "./Shadows.hlsli"
#include "./ShaderLight.hlsli"

cbuffer RoomBuffer : register(b5)
{
	int Water;
	int Caustics;
	int NumRoomLights;
	int Padding;
	float2 CausticsStartUV;
	float2 CausticsScale;
	float4 AmbientColor;
	ShaderLight RoomLights[MAX_LIGHTS_PER_ROOM];
};

struct PixelShaderInput
{
	float4 Position: SV_POSITION;
	float3 WorldPosition: POSITION0;
	float3 Normal: NORMAL;
	float2 UV: TEXCOORD0;
	float4 Color: COLOR;
	float Sheen : SHEEN;
	float4 PositionCopy : TEXCOORD1;
	float4 FogBulbs : TEXCOORD2;
	float DistanceFog : FOG;
	float3 Tangent: TANGENT;
	float3 Binormal: BINORMAL;
};

Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

Texture2D NormalTexture : register(t1);
SamplerState NormalTextureSampler : register(s1);

Texture2D CausticsTexture : register(t2);
SamplerState CausticsTextureSampler : register(s2);

Texture2D SSAOTexture : register(t9);
SamplerState SSAOSampler : register(s9);

struct PixelShaderOutput
{
	float4 Color: SV_TARGET0;
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
		float xOffset = (sin(factor * PI / 20.0f)) * (screenPos.z / 1024) * 4;
		float yOffset = (cos(factor * PI / 20.0f)) * (screenPos.z / 1024) * 4;
		screenPos.x += xOffset * weight;
		screenPos.y += yOffset * weight;
	}
	
	output.Position = screenPos;
	output.Normal = input.Normal;
	output.Color = float4(col, input.Color.w);
	output.PositionCopy = screenPos;

#ifdef ANIMATED

	if (Type == 0)
		output.UV = GetFrame(input.PolyIndex, input.AnimationFrameOffset);
	else
		output.UV = input.UV; // TODO: true UVRotate in future?
#else
	output.UV = input.UV;
#endif
	
	output.WorldPosition = pos;
	output.Tangent = input.Tangent;
	output.Binormal = input.Binormal;

	output.FogBulbs = DoFogBulbsForVertex(output.WorldPosition);
	output.DistanceFog = DoDistanceFogForVertex(output.WorldPosition);

	return output;
}

float3 UnpackNormalMap(float4 n)
{
	n = n * 2.0f - 1.0f;
	n.z = saturate(1.0f - dot(n.xy, n.xy));
	return n.xyz;
}

float3 PackNormal(float3 n)
{
	n = (n + 1.0f) * 0.5f;
	n.z = 0;
	return n.xyz;
}

PixelShaderOutput PS(PixelShaderInput input)
{
	PixelShaderOutput output;

	output.Color = Texture.Sample(Sampler, input.UV);

	DoAlphaTest(output.Color);

	float3x3 TBN = float3x3(input.Tangent, input.Binormal, input.Normal);
	float3 normal = UnpackNormalMap(NormalTexture.Sample(NormalTextureSampler, input.UV));
	normal = normalize(mul(normal, TBN));

	float3 lighting = input.Color.xyz;
	bool doLights = true;

	float occlusion = 1.0f;
	if (AmbientOcclusion == 1)
	{
		float2 samplePosition;
		samplePosition = input.PositionCopy.xy / input.PositionCopy.w;               // perspective divide
		samplePosition = samplePosition * 0.5f + 0.5f; // transform to range 0.0 - 1.0  
		samplePosition.y = 1.0f - samplePosition.y;
		occlusion = pow(SSAOTexture.Sample(SSAOSampler, samplePosition).x, AmbientOcclusionExponent);
	}

	if (CastShadows)
	{
        if (Light.Type == LT_POINT)
        {
            DoPointLightShadow(input.WorldPosition, lighting);
        }
        else if (Light.Type == LT_SPOT)
        {
            DoSpotLightShadow(input.WorldPosition, lighting);
        }
	}

    DoBlobShadows(input.WorldPosition, lighting);

	if (doLights)
	{
		for (int i = 0; i < NumRoomLights; i++)
		{
			int lightType = RoomLights[i].Type;
			float3 lightPos = RoomLights[i].Position.xyz;
			float3 color = RoomLights[i].Color.xyz;
			float radius = RoomLights[i].Out;

			float3 lightVec = (lightPos - input.WorldPosition);
			float distance = length(lightVec);
			if (distance > radius)
				continue;

			lightVec = normalize(lightVec);
			float d = saturate(dot(normal, lightVec ));
			if (d < 0)
				continue;

			if (lightType == LT_POINT)
			{
				lighting += DoPointLight(input.WorldPosition, normal, RoomLights[i]);
			}
			else if (lightType == LT_SPOT)
			{
				lighting += DoSpotLight(input.WorldPosition, normal, RoomLights[i]);
			}
		}
	}

	if (Caustics)
	{
		float attenuation = saturate(dot(float3(0.0f, -1.0f, 0.0f), normal));

		float3 blending = abs(normal);
		blending = normalize(max(blending, 0.00001f));
		float b = (blending.x + blending.y + blending.z);
		blending /= float3(b, b, b);

		float3 p = frac(input.WorldPosition.xyz / 2048.0f); 
		
		float3 xaxis = CausticsTexture.SampleLevel(CausticsTextureSampler, float2(p.z, p.y), 0).xyz;
		float3 yaxis = CausticsTexture.SampleLevel(CausticsTextureSampler, float2(p.z, p.x), 0).xyz;
		float3 zaxis = CausticsTexture.SampleLevel(CausticsTextureSampler, float2(p.y, p.x), 0).xyz;

		float3 xc = xaxis * blending.x;
		float3 yc = yaxis * blending.y;
		float3 zc = zaxis * blending.z;

		float3 caustics = xc + yc + zc;

		lighting += (caustics * attenuation * 2.0f);
	}

	lighting -= float3(input.FogBulbs.w, input.FogBulbs.w, input.FogBulbs.w);
	output.Color.xyz = output.Color.xyz * lighting * occlusion;
	output.Color.xyz = saturate(output.Color.xyz);

	output.Color = DoFogBulbsForPixel(output.Color, float4(input.FogBulbs.xyz, 1.0f));
	output.Color = DoDistanceFogForPixel(output.Color, FogColor, input.DistanceFog);

	return output;
}
