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

cbuffer LightsBuffer : register(b1)
{
	RendererLight Lights[48];
	int NumLights;
	float3 Padding;
};

cbuffer MiscBuffer : register(b3)
{
	int AlphaTest;
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
	float3 WorldPosition: POSITION;
	float3 Normal: NORMAL;
	float2 UV: TEXCOORD;
	float4 Color: COLOR;
};

Texture2D Texture;
SamplerState Sampler;

PixelShaderInput VS(VertexShaderInput input)
{
	PixelShaderInput output;

	output.Position = mul(mul(float4(input.Position, 1.0f), View), Projection); 
	output.Normal = input.Normal;
	output.Color = input.Color;
	output.UV = input.UV;
	output.WorldPosition = input.Position.xyz;

	return output;
}

[earlydepthstencil]
float4 PS(PixelShaderInput input) : SV_TARGET
{
	float4 output = Texture.Sample(Sampler, input.UV);
	if (AlphaTest)
		clip(output.w - 0.5f);
	float3 colorMul = min(input.Color.xyz, 1.0f) * 2.0f;

	float3 lighting = colorMul.xyz;

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
	
	output.xyz = output.xyz * lighting;
	output.w = 1.0f;

	return output;
}