#include <SimpleMath.h>

namespace TEN::Renderer::ConstantBuffers
{
	using namespace DirectX::SimpleMath;

	struct alignas(16) CPostProcessBuffer
	{
		float CinematicBarsHeight;
		float ScreenFadeFactor;
		int ViewportWidth;
		int ViewportHeight;
		//--
		float EffectStrength;
		Vector3 Tint;
		//--
		Vector4 SSAOKernel[64];
	};
}