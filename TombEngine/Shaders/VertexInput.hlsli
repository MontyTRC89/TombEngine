struct VertexShaderInput {
	float3 Position: POSITION0;
	float3 Normal: NORMAL0;
	float2 UV: TEXCOORD0;
	float4 Color: COLOR0;
	float3 Tangent: TANGENT0;
	uint AnimationFrameOffset: ANIMATIONFRAMEOFFSET;
	float4 Effects: EFFECTS;
	float Bone: BLENDINDICES;
	uint PolyIndex : POLYINDEX;
	uint Index: DRAWINDEX;
	int Hash : HASH;
};