struct RendererLight {
	float4 Position;
	float4 Color;
	float4 Direction;
	float Intensity;
	float In;
	float Out;
	float Range;
};

cbuffer CameraMatrixBuffer : register(b0)
{
	float4x4 View;
	float4x4 Projection;
};

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
	float3 Padding;
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
};

Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

PixelShaderInput VS(VertexShaderInput input)
{
	PixelShaderInput output;

	float4x4 world = mul(Bones[input.Bone], World);

	output.Position = mul(mul(mul(float4(input.Position, 1.0f), world), View), Projection);
	output.Normal = (mul(float4(input.Normal, 0.0f), world).xyz);
	output.Color = input.Color;
	output.UV = input.UV;
	output.WorldPosition = (mul(float4(input.Position, 1.0f), world));

	return output;
}

[earlydepthstencil]
float4 PS(PixelShaderInput input) : SV_TARGET
{
	float4 output = Texture.Sample(Sampler, input.UV);
	clip(output.w - 0.5f);

	float3 lighting = AmbientLight.xyz;

	for (int i = 0; i < NumLights; i++)
	{
		int lightType = Lights[i].Position.w;

		if (lightType == 1)
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

			lighting += color * d * intensity * attenuation;
		}
		else if (lightType == 0)
		{
			float3 color = Lights[i].Color.xyz;
			float3 direction = Lights[i].Direction.xyz;
			float intensity = Lights[i].Intensity;
			
			direction = normalize(direction);

			float d = dot(input.Normal, direction);
			if (d < 0)
				continue;

			lighting += color * d * intensity;
		}
	}

	output.xyz *= lighting.xyz;
	
	return output;
}