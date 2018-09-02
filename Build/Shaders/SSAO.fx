struct VertexShaderInput
{
	float3 Position : POSITION0;
	float2 TextureCoordinate : TEXCOORD0;
};

struct VertexShaderOutput
{
	float4 Position : POSITION0;
};

sampler g_buffer_norm;
sampler g_buffer_pos;
sampler g_random;
float RandomSize = 64.0f;
float SampleRadius;
float Intensity;
float Scale;
float Bias;
int ScreenWidth;
int ScreenHeight;

float HalfPixelX;
float HalfPixelY;

float3 getPosition(in float2 uv)
{
	return tex2D(g_buffer_pos, uv).xyz;
}

float3 getNormal(in float2 uv)
{
	return normalize(tex2D(g_buffer_norm, uv).xyz * 2.0f - 1.0f);
}

float2 getRandom(in float2 uv)
{
	return normalize(tex2D(g_random, g_screen_size * uv / random_size).xy * 2.0f - 1.0f);
}

float doAmbientOcclusion(in float2 tcoord, in float2 uv, in float3 p, in float3 cnorm)
{
	float3 diff = getPosition(tcoord + uv) - p;
	const float3 v = normalize(diff);
	const float d = length(diff)*g_scale;
	return max(0.0, dot(cnorm, v) - g_bias)*(1.0 / (1.0 + d))*g_intensity;
}

VertexShaderOutput VertexShaderFunction(VertexShaderInput input)
{
	VertexShaderOutput output;

	output.Position = float4(input.Position, 1.0f);

	return output;
}

PixelShaderOutput PixelShaderFunction(VertexShaderOutput input)
{
	float4 o = (PS_OUTPUT)0;
	o.color.rgb = 1.0f;
	const float2 vec[4] = { float2(1,0),float2(-1,0), float2(0,1),float2(0,-1) };
	float3 p = getPosition(i.uv);
	float3 n = getNormal(i.uv);
	float2 rand = getRandom(i.uv);
	float ao = 0.0f;
	float rad = g_sample_rad / p.z;

	int iterations = 4;
	for (int j = 0; j < iterations; ++j)
	{
		float2 coord1 = reflect(vec[j], rand)*rad;
		float2 coord2 = float2(coord1.x*0.707 - coord1.y*0.707, coord1.x*0.707 + coord1.y*0.707);

		ao += doAmbientOcclusion(i.uv, coord1*0.25, p, n);
		ao += doAmbientOcclusion(i.uv, coord2*0.5, p, n);
		ao += doAmbientOcclusion(i.uv, coord1*0.75, p, n);
		ao += doAmbientOcclusion(i.uv, coord2, p, n);
	}

	ao /= (float)iterations*4.0;

	//**END**// 
	//Do stuff here with your occlusion value ao modulate ambient lighting, write it to a buffer for later 
	//use, etc. 
	return o;
}