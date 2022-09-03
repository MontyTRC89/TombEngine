#define MAX_SPRITES 128

struct InstancedSprite
{
	float4x4 World;
	float4 Color;
	float IsBillboard;
	float3 SpritePadding;
};

cbuffer InstancedSpriteBuffer : register(b13)
{
	InstancedSprite Sprites[MAX_SPRITES];

	//float4x4 World[MAX_SPRITES];
	//float4 Color[MAX_SPRITES];
	//float IsBillboard[MAX_SPRITES];
};