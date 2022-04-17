#include "./CameraMatrixBuffer.hlsli"
#include "./VertexInput.hlsli"

struct PixelShaderInput
{
	float4 Position: SV_POSITION;
	float3 Normal: NORMAL;
	float2 UV: TEXCOORD;
	float4 Color: COLOR;
};

PixelShaderInput VS(VertexShaderInput input)
{
	PixelShaderInput output;

	output.Position = mul(float4(input.Position, 1.0f), ViewProjection); 
	output.Normal = input.Normal;
	output.Color = input.Color;
	output.UV = input.UV;

	return output;
}

float4 PS(PixelShaderInput input) : SV_TARGET
{
	/*float4 output = Texture.Sample(Sampler, input.UV);
	clip(output.w - 0.5f);
	float3 colorMul = min(input.Color.xyz, 1.0f) * 2.0f;
	output.xyz = output.xyz * colorMul.xyz;
	output.w = 1.0f;*/

	float3 colorMul = min(input.Color.xyz, 1.0f) * 2.0f;

	return float4(colorMul, 1.0f);
}