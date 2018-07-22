struct VertexShaderInput
{
    float3 Position : POSITION0;
    float4 Color : COLOR0;
};

struct VertexShaderOutput
{
    float4 Position : POSITION0;
    float4 Color : TEXCOORD0; // this is new
};

VertexShaderOutput VertexShaderFunction(VertexShaderInput input)
{
    VertexShaderOutput output;

    output.Position = float4(input.Position, 1);
	output.Color = input.Color;
	
    return output;
}

float4 PixelShaderFunction(VertexShaderOutput input) : COLOR0
{
    return (input.Color);
}
 
technique Textured
{
    pass Pass1
    {
        VertexShader = compile vs_3_0 VertexShaderFunction();
        PixelShader = compile ps_3_0 PixelShaderFunction();
    }
}
