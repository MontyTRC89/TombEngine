cbuffer CameraMatrixBuffer : register(b0)
{
	float4x4 View;
	float4x4 Projection;
};

cbuffer StaticMatrixBuffer : register(b1)
{
	float4x4 World;
	float4 StaticPosition;
	float4 Color;
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
	float2 UV: TEXCOORD;
	float4 Color: COLOR;
};

Texture2D Texture;
SamplerState Sampler;

PixelShaderInput VS(VertexShaderInput input)
{
	PixelShaderInput output;

	output.Position = mul(mul(mul(float4(input.Position, 1.0f), World), View), Projection);
	output.Normal = input.Normal;
	output.Color = input.Color;
	output.UV = input.UV;

	return output;
}

float4 PS(PixelShaderInput input) : SV_TARGET
{
	float4 output = Texture.Sample(Sampler, input.UV);
	clip(output.w - 0.5f);
	output.xyz = output.xyz * Color;
	output.w = 1.0f;

	return output;
}