#include "./CameraMatrixBuffer.hlsli"

cbuffer MiscBuffer : register(b3)
{
	int AlphaTest;
};

cbuffer SpriteBuffer: register(b4) {
	float4x4 billboardMatrix;
	float4 color;
	bool isBillboard;
}

#include "./VertexInput.hlsli"

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
	if (isBillboard) {
		output.Position = mul(mul(float4(input.Position, 1.0f), billboardMatrix), ViewProjection);
	} else {
		output.Position = mul(float4(input.Position, 1.0f), ViewProjection);
	}
	
	output.Normal = input.Normal;
	output.Color = input.Color * color;
	output.UV = input.UV;

	return output;
}

float4 PS(PixelShaderInput input) : SV_TARGET
{
	float4 output = Texture.Sample(Sampler, input.UV) * input.Color;
	return output;
}