struct VOut
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

VOut VShader(float3 position : POSITION, float3 normal : NORMAL, float2 uv : TEXCOORD, float4 color : COLOR, float bone : BLENDINDICES)
{
	VOut output;

	output.position = float4(position.xyz, 1);
	output.color = color;

	return output;
}


float4 PShader(float4 position : SV_POSITION, float4 color : COLOR) : SV_TARGET
{
	return color;
}
