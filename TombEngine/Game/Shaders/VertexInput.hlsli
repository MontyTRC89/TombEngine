
struct VertexShaderInput 
{
	float3 Position: POSITION0;
	float3 Normal: NORMAL0;
	float2 UV: TEXCOORD0;
	float4 Color: COLOR0;
	float3 Tangent: TANGENT0;
	float3 Binormal: BINORMAL0;
	unsigned int AnimationFrameOffset: ANIMATIONFRAMEOFFSET;
	float4 Effects: EFFECTS;
	float Bone: BLENDINDICES;
	unsigned int PolyIndex : POLYINDEX;
	unsigned int Index: DRAWINDEX;
	int Hash : HASH;
};
