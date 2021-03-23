#include "CameraMatrixBuffer.hlsli"
#include "./VertexInput.hlsli"
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
struct AnimatedFrameUV
{
	float2 topLeft;
	float2 topRight;
	float2 bottomRight;
	float2 bottomLeft;
};
cbuffer AnimatedBuffer : register(b6) {
	AnimatedFrameUV AnimFrames[32];
	uint numAnimFrames;
	
}

struct PixelShaderInput
{
	float4 Position: SV_POSITION;
	float3 WorldPosition: POSITION0;
	float3 Normal: NORMAL;
	float2 UV: TEXCOORD;
	float4 Color: COLOR;
	float4 LightPosition: POSITION1;
	float3x3 TBN : TBN;
};
Texture2D NormalTexture : register(t3);
Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

Texture2D CausticsTexture : register(t1);

Texture2D ShadowMap : register(t2);
SamplerComparisonState ShadowMapSampler : register(s1);


float hash(float3 n)
{
	float x = n.x;
	float y = n.y;
	float z = n.z;
	return float((frac(sin(x)) * 7385.6093) + (frac(cos(y)) * 1934.9663) - (frac(sin(z)) * 8349.2791));
}

PixelShaderInput VS(VertexShaderInput input)
{
	PixelShaderInput output;
	float4 screenPos = mul(float4(input.Position, 1.0f), ViewProjection);
	float2 clipPos = screenPos.xy / screenPos.w;
	if (CameraUnderwater != water) {
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
	if (water) {
		static const float PI = 3.14159265f;
		int offset = input.Hash;
		float wibble = sin((((Frame + offset) % 64) / 64.0)* PI)*0.5f+0.5f;
		wibble = lerp(0.1f, 1.0f, wibble);
		output.Color *= wibble;
	}
#ifdef ANIMATED
	int frame = (Frame / 2) % numAnimFrames;
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
	output.LightPosition = mul(float4(input.Position, 1.0f), LightViewProjection);
	float3x3 TBN = float3x3(input.Tangent, input.Bitangent, input.Normal);
	output.TBN = TBN;
	return output;
}

float2 texOffset(int u, int v) {
	return float2(u * 1.0f / SHADOW_MAP_SIZE, v * 1.0f / SHADOW_MAP_SIZE);
}

float4 PS(PixelShaderInput input) : SV_TARGET
{
	float4 output = Texture.Sample(Sampler, input.UV);
	if (AlphaTest && output.w < 0.01f) {
		discard;
	}
	float3 Normal = NormalTexture.Sample(Sampler,input.UV).rgb;
	//Normal = float3(0.5, 0.5, 1);
	Normal = Normal * 2 - 1;
	Normal = normalize(mul(Normal,input.TBN));
	//Normal = input.Normal;
	
	float3 lighting = input.Color.xyz;
	bool doLights = true;

	if (CastShadows)
	{
		// Transform clip space coords to texture space coords (-1:1 to 0:1)
		input.LightPosition.xyz /= input.LightPosition.w;

		if (input.LightPosition.x >= -1.0f && input.LightPosition.x <= 1.0f &&
			input.LightPosition.y >= -1.0f && input.LightPosition.y <= 1.0f &&
			input.LightPosition.z >= 0.0f && input.LightPosition.z <= 1.0f)
		{
			input.LightPosition.x = input.LightPosition.x / 2 + 0.5;
			input.LightPosition.y = input.LightPosition.y / -2 + 0.5;

			//PCF sampling for shadow map
			float sum = 0;
			float x, y;
			//perform PCF filtering on a 4 x 4 texel neighborhood
			for (y = -1.5; y <= 1.5; y += 1.0) {
				for (x = -1.5; x <= 1.5; x += 1.0) {
					sum += ShadowMap.SampleCmpLevelZero(ShadowMapSampler, input.LightPosition.xy + texOffset(x, y), input.LightPosition.z);
				}
			}

			float shadowFactor = sum / 16.0;
			lighting = lerp(lighting, min(AmbientColor,lighting), 1-saturate(shadowFactor));
		}
	}

	if (doLights)
	{
		for (uint i = 0; i < NumLights; i++)
		{
			float3 lightPos = Lights[i].Position.xyz;
			float3 color = Lights[i].Color.xyz;
			float radius = Lights[i].Out;
			float intensity = Lights[i].Intensity;

			float3 lightVec = (lightPos - input.WorldPosition);
			float distance = length(lightVec);
			lightVec = normalize(lightVec);
			if (distance > radius)
				continue;

			float d = saturate(dot(Normal,-lightVec ));
			if (d < 0)
				continue;
			
			float attenuation = pow(((radius - distance) / radius), 2);

			lighting += color * intensity * attenuation * d;
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

		lighting += float4((xaxis * blending.x + yaxis * blending.y + zaxis * blending.z).xyz, 0.0f) * attenuation * 2.0f;
	}
	
	output.xyz = output.xyz * lighting;

	return output;
}

