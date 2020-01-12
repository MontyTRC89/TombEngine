cbuffer HUDBuffer : register(b0)
{
	float4x4 ViewProjection;
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
	float4 Color: COLOR;
};


PixelShaderInput VS(VertexShaderInput input)
{
	PixelShaderInput output;
	output.Position = mul(float4(input.Position, 1.0f), ViewProjection); 
	output.Color = input.Color;
	output.UV = input.UV;
	return output;
}