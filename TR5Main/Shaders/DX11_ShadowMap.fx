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
	float2 UV: TEXCOORD;
	float Depth: COLOR;
};

Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

PixelShaderInput VS(VertexShaderInput input)
{
	PixelShaderInput output;

	float4x4 world = mul(Bones[input.Bone], World);

	output.Position = mul(mul(mul(float4(input.Position, 1.0f), world), View), Projection);
	output.UV = input.UV;
	output.Depth = output.Position.z / output.Position.w;

	return output;
}

float4 PS(PixelShaderInput input) : SV_TARGET
{
	float4 output = Texture.Sample(Sampler, input.UV);
	if (AlphaTest)
		clip(output.w - 0.5f);
	return output;

	//return float4(input.Depth, 0.0f, 0.0f, 1.0f);
}