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

float4 PS(PixelShaderInput input) : SV_TARGET
{
	float4 output = Texture.Sample(Sampler, input.UV);
	clip(output.w - 0.5f);

	output.xyz *= AmbientLight.xyz * 1.0f;
	
	return output;
}