struct VertexShaderInput
{
    float3 Position : POSITION0;
    float3 Normal : NORMAL0;
    float2 TextureCoordinate : TEXCOORD0; // this is new
    float4 Color : COLOR0; // this is new
	float Bone : BLENDINDICES0;
};

struct VertexShaderOutput
{
    float4 Position : POSITION0;
	float4 ScreenPosition : TEXCOORD3;
    float3 Normal : TEXCOORD0;
    float2 TextureCoordinate : TEXCOORD1; // this is new
    float4 Color : TEXCOORD2; // this is new
};

float4x4 World;
float4x4 View;
float4x4 Projection;

bool UseSkinning;
float4x4 Bones[48];

bool EnableVertexColors;
float FadeTimer;

texture ModelTexture;
sampler2D textureSampler = sampler_state {
    Texture = (ModelTexture);
    MinFilter = Linear;
    MagFilter = Linear;
    AddressU = Clamp;
    AddressV = Clamp;
};

bool CinematicMode;

VertexShaderOutput VertexShaderFunction(VertexShaderInput input)
{
    VertexShaderOutput output;
 
    float4 worldPosition;
	
	if (UseSkinning)
	{
		worldPosition = mul(float4(input.Position, 1), mul(Bones[input.Bone], World));
	}
	else
		worldPosition = mul(float4(input.Position, 1), World);
		
    float4 viewPosition = mul(worldPosition, View);
    output.Position = mul(viewPosition, Projection);

    output.Normal = input.Normal;
    output.TextureCoordinate = input.TextureCoordinate;
	output.Color = input.Color;
	output.ScreenPosition = output.Position;
	
    return output;
}

float4 PixelShaderFunction(VertexShaderOutput input) : COLOR0
{
	float2 screenPos = input.ScreenPosition.xy / input.ScreenPosition.w;

	if (CinematicMode && (screenPos.y <= -0.8f || screenPos.y >= 0.8f))
		return float4(0, 0, 0, 1);
		
    float4 textureColor = tex2D(textureSampler, input.TextureCoordinate);

	if (textureColor.a <= 0.5)
		return float4(0, 0, 0, 0);
	
	if (EnableVertexColors)
	{
		//float3 colorAdd = max(input.Color.xyz - 1.0f, 0.0f);
		float3 colorMul = min(input.Color.xyz, 1.0f) * 2.0f;
		textureColor.xyz = textureColor.xyz * colorMul; // + colorAdd;
    }
	
	if (FadeTimer < 30)
	{
		textureColor *= FadeTimer / 30.0f;
	}
	
    //textureColor.a = 1;
	
    return textureColor; 
}
 
technique Textured
{
    pass Pass1
    {
        VertexShader = compile vs_3_0 VertexShaderFunction();
        PixelShader = compile ps_3_0 PixelShaderFunction();
    }
}
