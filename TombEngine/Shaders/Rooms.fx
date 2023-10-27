#include "./CameraMatrixBuffer.hlsli"
#include "./VertexInput.hlsli"
#include "./VertexEffects.hlsli"
#include "./Blending.hlsli"
#include "./Math.hlsli"
#include "./AnimatedTextures.hlsli"
#include "./Shadows.hlsli"
#include "./ShaderLight.hlsli"

cbuffer RoomBuffer : register(b5)
{
	float2 CausticsStartUV;
	float2 CausticsScale;
	float4 AmbientColor;
	ShaderLight RoomLights[MAX_LIGHTS_PER_ROOM];
	int NumRoomLights;
	int Water;
	int Caustics;
	int Padding;
};

struct PixelShaderInput
{
	float4 Position: SV_POSITION;
	float3 WorldPosition: POSITION0;
	float3 Normal: NORMAL;
	float2 UV: TEXCOORD0;
	float4 Color: COLOR;
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
	
	output.WorldPosition = input.Position.xyz;
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
			
			float attenuation = pow(((radius - distance) / radius), 2);

			lighting += color * attenuation * d;
		}
	}

	if (Caustics)
	{
		float3 position = input.WorldPosition.xyz;
		
		float fracX = position.x - floor(position.x / 2048.0f) * 2048.0f;
		float fracY = position.y - floor(position.y / 2048.0f) * 2048.0f;
		float fracZ = position.z - floor(position.z / 2048.0f) * 2048.0f;

		float attenuation = saturate(dot(float3(0.0f, -1.0f, 0.0f), normal));

		float3 blending = abs(normal);
		blending = normalize(max(blending, 0.00001f));
		float b = (blending.x + blending.y + blending.z);
		blending /= float3(b, b, b);

		float3 p = float3(fracX, fracY, fracZ) / 2048.0f;
		float3 xaxis = CausticsTexture.Sample(CausticsTextureSampler, CausticsStartUV + float2(p.y * CausticsScale.x, p.z * CausticsScale.y)).xyz;
		float3 yaxis = CausticsTexture.Sample(CausticsTextureSampler, CausticsStartUV + float2(p.x * CausticsScale.x, p.z * CausticsScale.y)).xyz;
		float3 zaxis = CausticsTexture.Sample(CausticsTextureSampler, CausticsStartUV + float2(p.x * CausticsScale.x, p.y * CausticsScale.y)).xyz;

		lighting += float3((xaxis * blending.x + yaxis * blending.y + zaxis * blending.z).xyz) * attenuation * 2.0f;
	}

	output.Depth = output.Color.w > 0.0f ?
		float4(input.PositionCopy.z / input.PositionCopy.w, 0.0f, 0.0f, 1.0f) :
		float4(0.0f, 0.0f, 0.0f, 0.0f);

	lighting -= float3(input.FogBulbs.w, input.FogBulbs.w, input.FogBulbs.w);
	output.Color.xyz = output.Color.xyz * lighting;
	output.Color.xyz = saturate(output.Color.xyz);

	output.Color = DoFogBulbsForPixel(output.Color, float4(input.FogBulbs.xyz, 1.0f));
	output.Color = DoDistanceFogForPixel(output.Color, FogColor, input.DistanceFog);

	return output;
}
