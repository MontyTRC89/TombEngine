#include "./CameraMatrixBuffer.hlsli"

struct RendererLight {
	float4 Position;
	float4 Color;
	float4 Direction;
	float Intensity;
	float In;
	float Out;
	float Range;
};

cbuffer LightsBuffer : register(b1)
{
	RendererLight Lights[48];
	int NumLights;
	float3 Padding;
};

cbuffer MiscBuffer : register(b3)
{
	int AlphaTest;
	int Caustics;
};

cbuffer CShadowLightBuffer : register(b4)
{
	RendererLight Light;
	float4x4 LightViewProjection;
	int CastShadows;
	float3 Padding2;
};

cbuffer RoomBuffer : register(b5)
{
	float4 AmbientColor;
	int water;
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
	float3 WorldPosition: POSITION0;
	float3 Normal: NORMAL;
	float2 UV: TEXCOORD;
	float4 Color: COLOR;
	float4 LightPosition: POSITION1;
};

Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

Texture2D CausticsTexture : register(t1);

Texture2D ShadowMap : register(t2);
SamplerState ShadowMapSampler : register(s1);
PixelShaderInput VS(VertexShaderInput input)
{
	PixelShaderInput output;
	float4 screenPos = mul(float4(input.Position, 1.0f), ViewProjection);
	float2 clipPos = screenPos.xy / screenPos.w;
	if (cameraUnderwater != water) {
		static const float PI = 3.14159265f;
		float factor = (Frame + clipPos.x*320);
		float xOffset = (sin(factor * PI/20.0f)) * (screenPos.z/1024)*5;
		float yOffset = (cos(factor*PI/20.0f))*(screenPos.z/1024)*5;
		screenPos.x += xOffset;
		screenPos.y += yOffset;
	}
	output.Position = screenPos;
	output.Normal = input.Normal;
	output.Color = input.Color;
	output.UV = input.UV;
	output.WorldPosition = input.Position.xyz;
	output.LightPosition = mul(float4(input.Position, 1.0f), LightViewProjection);

	return output;
}

float getShadowFactor(Texture2D shadowMap, SamplerState shadowMapSampler, float2 coords, float realDepth) {
	const float texelSize = 1.0f / 1024;
	float shadowFactor = 0.0f;
	//doing 9 samples
	for (float x = -1; x <= 1.0f; x++) {
		for (float y = -1; y <= 1.0f; y++) {
			float shadowMapDepth = ShadowMap.SampleLevel(ShadowMapSampler, coords + (float2(x, y)*texelSize),0).r;
			shadowFactor += shadowMapDepth < realDepth ? 1.0f : 0.0f;
		}
	}
	return shadowFactor / 9.0f;
}

[earlydepthstencil]
float4 PS(PixelShaderInput input) : SV_TARGET
{
	float4 output = Texture.Sample(Sampler, input.UV);
	if (AlphaTest)
		clip(output.w - 0.5f);
	float3 colorMul = min(input.Color.xyz, 1.0f) * 2.0f;

	float3 lighting = colorMul.xyz;
	bool doLights = true;

	if (CastShadows)
	{
		// Transform clip space coords to texture space coords (-1:1 to 0:1)
		input.LightPosition.xyz /= input.LightPosition.w;

		if (input.LightPosition.x >= -1.0f && input.LightPosition.x <= 1.0f &&
			input.LightPosition.y >= -1.0f && input.LightPosition.y <= 1.0f &&
			input.LightPosition.z >= 0.0f && input.LightPosition.z <= 1.0f)
		{
			float2 coords = float2(input.LightPosition.x / 2.0f + 0.5f, -input.LightPosition.y / 2.0f + 0.5f);

			// Sample shadow map - point sampler
			float shadowMapDepth = ShadowMap.Sample(ShadowMapSampler, coords).r;

			float realDepth = input.LightPosition.z;

			// If clip space z value greater than shadow map value then pixel is in shadow
			float shadow = getShadowFactor(ShadowMap, ShadowMapSampler, coords, realDepth);
			lighting = lerp(lighting, min(AmbientColor,lighting), saturate(shadow));
		}
	}

	if (doLights)
	{
		for (int i = 0; i < NumLights; i++)
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

			lighting += color * intensity * attenuation;
		}
	}

	if (Caustics)
	{
		float3 position = input.WorldPosition.xyz;
		float3 normal = input.Normal.xyz;

		float fracX = position.x - floor(position.x / 2048.0f) * 2048.0f;
		float fracY = position.y - floor(position.y / 2048.0f) * 2048.0f;
		float fracZ = position.z - floor(position.z / 2048.0f) * 2048.0f;

		float attenuation = saturate(dot(float3(0.0f, -1.0f, 0.0f), normal));

		float3 blending = abs(normal);
		blending = normalize(max(blending, 0.00001f));
		float b = (blending.x + blending.y + blending.z);
		blending /= float3(b, b, b);

		float3 p = float3(fracX, fracY, fracZ) / 2048.0f;
		float3 xaxis = CausticsTexture.Sample(Sampler, p.yz).xyz; 
		float3 yaxis = CausticsTexture.Sample(Sampler, p.xz).xyz;  
		float3 zaxis = CausticsTexture.Sample(Sampler, p.xy).xyz;  

		lighting += float4((xaxis * blending.x + yaxis * blending.y + zaxis * blending.z).xyz, 0.0f) * attenuation * 2.0f;
	}
	
	output.xyz = output.xyz * lighting;
	output.w = 1.0f;

	return output;
}

