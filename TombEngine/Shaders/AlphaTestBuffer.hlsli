#define ALPHA_TEST_NONE 0
#define ALPHA_TEST_GREATER_THAN 1
#define ALPHA_TEST_LESS_THAN 2

cbuffer AlphaTestBuffer : register(b12)
{
	int AlphaTest;
	float AlphaThreshold;
};

void DoAlphaTest(float4 inputColor)
{
	if (AlphaTest == ALPHA_TEST_GREATER_THAN && inputColor.w < AlphaThreshold)
	{
		discard;
	}
	else if (AlphaTest == ALPHA_TEST_LESS_THAN && inputColor.w > AlphaThreshold)
	{
		discard;
	}
	else
	{
		return;
	}
}