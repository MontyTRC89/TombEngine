#define MAX_SPRITES 256

struct InstancedSprite
{
	float4x4 World;
	float4 UV[2];
	float4 Color;
	float IsBillboard;
	float IsSoftParticle;
};

cbuffer InstancedSpriteBuffer : register(b13)
{
	InstancedSprite Sprites[MAX_SPRITES];
};