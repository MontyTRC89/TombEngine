#include "framework.h"
#include "Renderer/Renderer.h"
#include "Game/spotcam.h"

namespace TEN::Renderer
{
	void Renderer::DrawPostprocess(RenderTarget2D* renderTarget, RenderView& view)
	{
		SetBlendMode(BlendMode::Opaque);
		SetCullMode(CullMode::CounterClockwise);
		SetDepthState(DepthState::Write);
		_context->RSSetViewports(1, &view.Viewport);
		ResetScissor();

		_stPostProcessBuffer.ViewportWidth = _screenWidth;
		_stPostProcessBuffer.ViewportHeight = _screenHeight;
		_stPostProcessBuffer.ScreenFadeFactor = ScreenFadeCurrent;
		_stPostProcessBuffer.CinematicBarsHeight = Smoothstep(CinematicBarsHeight) * SPOTCAM_CINEMATIC_BARS_HEIGHT;
		_stPostProcessBuffer.EffectStrength = _postProcessColorEffectStrength;
		_cbPostProcessBuffer.UpdateData(_stPostProcessBuffer, _context.Get());

		// Common vertex shader to all fullscreen effects
		_context->VSSetShader(_vsPostProcess.Get(), nullptr, 0);

		// We draw a fullscreen triangle
		_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		_context->IASetInputLayout(_fullscreenTriangleInputLayout.Get());

		UINT stride = sizeof(PostProcessVertex);
		UINT offset = 0;

		_context->IASetVertexBuffers(0, 1, _fullscreenTriangleVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);

		// Copy render target to post process render target.
		float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		_context->ClearRenderTargetView(_postProcessRenderTarget[0].RenderTargetView.Get(), clearColor);
		_context->OMSetRenderTargets(1, _postProcessRenderTarget[0].RenderTargetView.GetAddressOf(), nullptr);

		_context->PSSetShader(_psPostProcessCopy.Get(), nullptr, 0);
		BindRenderTargetAsTexture(TextureRegister::ColorMap, &_renderTarget, SamplerStateRegister::PointWrap);
		DrawTriangles(3, 0);

		// Let's do ping-pong between the two post-process render targets
		int currentRenderTarget = 0;
		int destinationRenderTarget = 1;

		// Apply color scheme
		if (_postProcessColorEffect != PostProcessColorEffect::Normal)
		{
			_context->ClearRenderTargetView(_postProcessRenderTarget[destinationRenderTarget].RenderTargetView.Get(), clearColor);
			_context->OMSetRenderTargets(1, _postProcessRenderTarget[destinationRenderTarget].RenderTargetView.GetAddressOf(), nullptr);
			
			switch (_postProcessColorEffect)
			{
			case PostProcessColorEffect::Sepia:
				_context->PSSetShader(_psPostProcessSepia.Get(), nullptr, 0);
				break;

			case PostProcessColorEffect::Monochrome:
				_context->PSSetShader(_psPostProcessMonochrome.Get(), nullptr, 0);
				break;
				 
			default:
				return;

			}

			BindRenderTargetAsTexture(TextureRegister::ColorMap, &_postProcessRenderTarget[currentRenderTarget], SamplerStateRegister::PointWrap);
			DrawTriangles(3, 0);

			destinationRenderTarget = 0;
			currentRenderTarget = 1;
		}  

		// Do the final pass
		_context->PSSetShader(_psPostProcessFinalPass.Get(), nullptr, 0);

		_context->ClearRenderTargetView(renderTarget->RenderTargetView.Get(), Colors::Black);
		_context->OMSetRenderTargets(1, renderTarget->RenderTargetView.GetAddressOf(), nullptr);

		BindTexture(TextureRegister::ColorMap, &_postProcessRenderTarget[currentRenderTarget], SamplerStateRegister::PointWrap);

		DrawTriangles(3, 0);
	}

	void Renderer::SetPostProcessColorEffect(PostProcessColorEffect colorScheme, float strength)
	{
		_postProcessColorEffect = colorScheme;
		_postProcessColorEffectStrength = strength;
	}
}
