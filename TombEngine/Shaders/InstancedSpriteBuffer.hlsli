#define MAX_SPRITES 128

struct InstancedSprite
{
	float4x4 World;
	float4 Color;
	float IsBillboard;
	float IsSoftParticle;
	float2 SpritePadding;
};

cbuffer InstancedSpriteBuffer : register(b13)
{
	InstancedSprite Sprites[MAX_SPRITES];
};