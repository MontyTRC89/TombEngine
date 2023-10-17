#include "./CameraMatrixBuffer.hlsli"
#include "./Blending.hlsli"
#include "./VertexInput.hlsli"
#include "./Math.hlsli"
#include "./ShaderLight.hlsli"

#define MAX_SORTED_PIXELS 16

struct PixelShaderInput
{
	float4 Position: SV_POSITION;
	float2 UV: TEXCOORD1;
	float4 PositionCopy: TEXCOORD2;
};

struct FragmentAndLinkBuffer_STRUCT
{
	float4 PixelColor;
	uint PixelDepthAndBlendMode;
	uint NextNode;
};

StructuredBuffer<FragmentAndLinkBuffer_STRUCT> FLBufferSRV : register(t1);

ByteAddressBuffer StartOffsetBufferSRV : register(t2);

Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

PixelShaderInput VS(VertexShaderInput input)
{
	PixelShaderInput output;

	output.Position = float4(input.Position, 1.0f);
	output.PositionCopy = output.Position;
	output.UV = input.UV;

	return output;
}

float4 PS(PixelShaderInput input) : SV_TARGET
{
	int nNumPixels = 0;
	
	static FragmentAndLinkBuffer_STRUCT SortedPixels[MAX_SORTED_PIXELS];

	float4 vPos = (input.PositionCopy);
	vPos.xy /= vPos.w;
	float2 pos = 0.5f * (float2(vPos.x, vPos.y) + 1);
	int posX = (int)(pos.x * ViewSize.x);
	int posY = (int)(pos.y * ViewSize.y);

	uint uStartOffsetAddress = 4 * ((ViewSize.x * posY) + posX);

	// Fetch offset of first fragment for current pixel
	uint uOffset = StartOffsetBufferSRV.Load(uStartOffsetAddress);

	while (uOffset != 0xFFFFFFFF)
	{
		// Retrieve pixel at current offset
		FragmentAndLinkBuffer_STRUCT Element = FLBufferSRV[uOffset];
		// Copy pixel data into temp array
		SortedPixels[nNumPixels].PixelColor = Element.PixelColor;
		SortedPixels[nNumPixels].PixelDepthAndBlendMode = Element.PixelDepthAndBlendMode;

		nNumPixels++;

		// Retrieve next offset
		[flatten] uOffset = (nNumPixels >= MAX_SORTED_PIXELS) ?
			0xFFFFFFFF : Element.NextNode;
	}

	for (int j = 0; j < nNumPixels - 1; j++)
	{
		for (int i = 0; i < nNumPixels - 1; i++)
		{
			if ((SortedPixels[i].PixelDepthAndBlendMode & 0xFFFFFF) < (SortedPixels[i + 1].PixelDepthAndBlendMode & 0xFFFFFF))
			{
				FragmentAndLinkBuffer_STRUCT temp = SortedPixels[i];
				SortedPixels[i] = SortedPixels[i + 1];
				SortedPixels[i + 1] = temp;
			}
		}
	}

	float4 color = Texture.Sample(Sampler, input.UV);

	for (int i = 0; i < nNumPixels; i++)
	{
		uint blendMode = (SortedPixels[i].PixelDepthAndBlendMode & 0xFF000000) >> 24;

		switch (blendMode)
		{
		case BLENDMODE_ADDITIVE:
			color.xyz += SortedPixels[i].PixelColor.xyz;
			break;

		case BLENDMODE_ALPHABLEND:
			color.xyz = lerp(color.xyz, SortedPixels[i].PixelColor.xyz, SortedPixels[i].PixelColor.w);
			break;

		default:
			color = SortedPixels[i].PixelColor;
			break;

		}	
	}

	return color;
}
