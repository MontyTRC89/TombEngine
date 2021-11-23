#include "./CameraMatrixBuffer.hlsli"
#include "./FogBuffer.hlsli"

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
	float Fog : FOG;
};

Texture2D Texture;
SamplerState Sampler;

PixelShaderInput VS(VertexShaderInput input)
{
	PixelShaderInput output;

	float4 worldPosition;

	if (isBillboard) {
		worldPosition = mul(float4(input.Position, 1.0f), billboardMatrix);
		output.Position = mul(mul(float4(input.Position, 1.0f), billboardMatrix), ViewProjection);
	} else {
		worldPosition = float4(input.Position, 1.0f);
		output.Position = mul(float4(input.Position, 1.0f), ViewProjection);
	}
	
	output.Normal = input.Normal;
	output.Color = input.Color * color;
	output.UV = input.UV;

	float4 d = length(CamPositionWS - worldPosition);
	if (FogMaxDistance == 0)
		output.Fog = 1;
	else
		output.Fog = clamp((d - FogMinDistance * 1024) / (FogMaxDistance * 1024 - FogMinDistance * 1024), 0, 1);

	return output;
}

float4 PS(PixelShaderInput input) : SV_TARGET
{
	float4 output = Texture.Sample(Sampler, input.UV) * input.Color;

	if (FogMaxDistance != 0)
		output.xyz = lerp(output.xyz, FogColor, input.Fog);

	return output;
}