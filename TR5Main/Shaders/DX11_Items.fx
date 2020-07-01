#define LT_SUN		0
#define LT_POINT	1
#define LT_SPOT		2
#define LT_SHADOW	3

struct RendererLight {
	float4 Position;
	float4 Color;
	float4 Direction;
	float Intensity;
	float In;
	float Out;
	float Range;
};

#include "./CameraMatrixBuffer.hlsli"

cbuffer ItemBuffer : register(b1) 
{
	float4x4 World;
	float4x4 Bones[32];
	float4 ItemPosition;
	float4 AmbientLight;
};

cbuffer LightsBuffer : register(b2)
{
	RendererLight Lights[48];
	int NumLights;
	float3 CameraPosition;
};

cbuffer MiscBuffer : register(b3)
{
	int AlphaTest;
	int Caustics;
};

struct VertexShaderInput
{
	float3 Position: POSITION;
	float3 Normal: NORMAL;
	float2 UV: TEXCOORD;
	float4 Color: COLOR;
	float Bone : BLENDINDICES;
};

struct PixelShaderInput
{
	float4 Position: SV_POSITION;
	float3 Normal: NORMAL;
	float3 WorldPosition : POSITION;
	float2 UV: TEXCOORD;
	float4 Color: COLOR;
	float3 ReflectionVector : TEXCOORD1;
};

Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);
TextureCube Reflection : register (t1);

PixelShaderInput VS(VertexShaderInput input)
{
	PixelShaderInput output;

	float4x4 world = mul(Bones[input.Bone], World);

	output.Position = mul(mul(float4(input.Position, 1.0f), world), ViewProjection);
	output.Normal = (mul(float4(input.Normal, 0.0f), world).xyz);
	output.Color = input.Color;
	output.UV = input.UV;
	output.WorldPosition = (mul(float4(input.Position, 1.0f), world));
	output.ReflectionVector = reflect(normalize(CamPositionWS - output.WorldPosition.xyz), normalize(output.Normal));

	return output;
}

float4 PS(PixelShaderInput input) : SV_TARGET
{
	float4 output = Texture.Sample(Sampler, input.UV);
	if (AlphaTest)
		clip(output.w - 0.5f);

	float3 lighting = AmbientLight.xyz * 2.0f;
	float4 reflectionColor = Reflection.Sample(Sampler,input.ReflectionVector.xyz);
	for (int i = 0; i < NumLights; i++)
	{
		int lightType = Lights[i].Position.w;

		if (lightType == LT_POINT || lightType == LT_SHADOW)
		{
			float3 lightPos = Lights[i].Position.xyz;
			float3 color = Lights[i].Color.xyz;
			float radius = Lights[i].Out;
			float intensity = Lights[i].Intensity;

			float3 lightVec = (lightPos - input.WorldPosition);
			float distance = length(lightVec);

			if (distance > radius)
				continue;

			lightVec = normalize(lightVec);
			float attenuation = (radius - distance) / radius;

			float d = dot(input.Normal, lightVec);
			if (d < 0)
				continue;

			float3 h = normalize(normalize(CameraPosition - input.WorldPosition) + lightVec);
			float s = pow(saturate(dot(h, input.Normal)), 0.5f);

			lighting += color * d * intensity * attenuation; // *0.6f + s * 0.5f;
		}
		else if (lightType == LT_SUN)
		{
			float3 color = Lights[i].Color.xyz;
			float3 direction = Lights[i].Direction.xyz;
			float intensity = Lights[i].Intensity;
			
			direction = normalize(direction);

			float d = dot(input.Normal, direction);
			if (d < 0)
				continue;

			float3 h = normalize(normalize(CameraPosition - input.WorldPosition) + direction);
			float s = pow(saturate(dot(h, input.Normal)), 0.5f);
			
			lighting += color * d * intensity; // *0.6f + s * 0.5f;
		}
		else if (lightType == LT_SPOT)
		{
			float3 lightPos = Lights[i].Position.xyz;
			float3 color = Lights[i].Color.xyz;
			float3 direction = Lights[i].Direction.xyz;
			float range = Lights[i].Range;
			float intensity = Lights[i].Intensity;
			float inAngle = Lights[i].In;
			float outAngle = Lights[i].Out;
			
			float3 lightVec = (lightPos - input.WorldPosition);
			float distance = length(lightVec);

			if (distance > range)
				continue;

			lightVec = normalize(lightVec);
			float inCone = dot(lightVec, direction); 
			if (inCone < outAngle)
				continue;			

			float attenuation = (range - distance) / range * (inCone - outAngle) / (1.0f - outAngle);

			float d = dot(input.Normal, lightVec);
			if (d < 0)
				continue;

			float3 h = normalize(normalize(CameraPosition - input.WorldPosition) + lightVec);
			float s = pow(saturate(dot(h, input.Normal)), 0.5f);

			lighting += color * d * intensity * attenuation; // *0.6f + s * 0.5f;
		}
	}

	output.xyz *= lighting.xyz;
	output += reflectionColor;
	return output;
}