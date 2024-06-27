
namespace TEN::Renderer::ConstantBuffers
{
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
