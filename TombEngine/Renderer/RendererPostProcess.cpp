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
		_stPostProcessBuffer.EffectStrength = _postProcessStrength;
		_stPostProcessBuffer.Tint = _postProcessTint;
		_cbPostProcessBuffer.UpdateData(_stPostProcessBuffer, _context.Get());

		// Common vertex shader to all fullscreen effects.
		_context->VSSetShader(_vsPostProcess.Get(), nullptr, 0);

		// Draw fullscreen triangle.
		_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		_context->IASetInputLayout(_fullscreenTriangleInputLayout.Get());

		unsigned int stride = sizeof(PostProcessVertex);
		unsigned int offset = 0;

		_context->IASetVertexBuffers(0, 1, _fullscreenTriangleVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);

		// Copy render target to post process render target.
		float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		_context->ClearRenderTargetView(_postProcessRenderTarget[0].RenderTargetView.Get(), clearColor);
		_context->OMSetRenderTargets(1, _postProcessRenderTarget[0].RenderTargetView.GetAddressOf(), nullptr);

		_context->PSSetShader(_psPostProcessCopy.Get(), nullptr, 0);
		BindRenderTargetAsTexture(TextureRegister::ColorMap, &_renderTarget, SamplerStateRegister::PointWrap);
		DrawTriangles(3, 0);

		// Ping-pong between two post-process render targets.
		int currentRenderTarget = 0;
		int destRenderTarget = 1;

		// Apply color scheme.
		if (_postProcessMode != PostProcessMode::None && _postProcessStrength > EPSILON)
		{
			_context->ClearRenderTargetView(_postProcessRenderTarget[destRenderTarget].RenderTargetView.Get(), clearColor);
			_context->OMSetRenderTargets(1, _postProcessRenderTarget[destRenderTarget].RenderTargetView.GetAddressOf(), nullptr);

			switch (_postProcessMode)
			{
			case PostProcessMode::Monochrome:
				_context->PSSetShader(_psPostProcessMonochrome.Get(), nullptr, 0);
				break;

			case PostProcessMode::Negative:
				_context->PSSetShader(_psPostProcessNegative.Get(), nullptr, 0);
				break;

			case PostProcessMode::Exclusion:
				_context->PSSetShader(_psPostProcessExclusion.Get(), nullptr, 0);
				break;

			default:
				return;
			}

			BindRenderTargetAsTexture(TextureRegister::ColorMap, &_postProcessRenderTarget[currentRenderTarget], SamplerStateRegister::PointWrap);
			DrawTriangles(3, 0);

			destRenderTarget = (destRenderTarget == 1) ? 0 : 1;
			currentRenderTarget = (currentRenderTarget == 1) ? 0 : 1;
		}

		// Lens flares.
		if (!view.LensFlaresToDraw.empty())
		{
			_context->ClearRenderTargetView(_postProcessRenderTarget[destRenderTarget].RenderTargetView.Get(), clearColor);
			_context->OMSetRenderTargets(1, _postProcessRenderTarget[destRenderTarget].RenderTargetView.GetAddressOf(), nullptr);

			_context->PSSetShader(_psPostProcessLensFlare.Get(), nullptr, 0);

			for (int i = 0; i < view.LensFlaresToDraw.size(); i++)
			{
				_stPostProcessBuffer.LensFlares[i].Position = view.LensFlaresToDraw[i].Position;
				_stPostProcessBuffer.LensFlares[i].Color = view.LensFlaresToDraw[i].Color.ToVector3();
			}
			_stPostProcessBuffer.NumLensFlares = (int)view.LensFlaresToDraw.size();
			_cbPostProcessBuffer.UpdateData(_stPostProcessBuffer, _context.Get());

			BindRenderTargetAsTexture(TextureRegister::ColorMap, &_postProcessRenderTarget[currentRenderTarget], SamplerStateRegister::PointWrap);
			DrawTriangles(3, 0);

			destRenderTarget = (destRenderTarget) == 1 ? 0 : 1;
			currentRenderTarget = (currentRenderTarget == 1) ? 0 : 1;
		}

		// Do final pass.
		_context->PSSetShader(_psPostProcessFinalPass.Get(), nullptr, 0);

		_context->ClearRenderTargetView(renderTarget->RenderTargetView.Get(), Colors::Black);
		_context->OMSetRenderTargets(1, renderTarget->RenderTargetView.GetAddressOf(), nullptr);

		BindTexture(TextureRegister::ColorMap, &_postProcessRenderTarget[currentRenderTarget], SamplerStateRegister::PointWrap);

		DrawTriangles(3, 0);
	}

	PostProcessMode Renderer::GetPostProcessMode()
	{
		return _postProcessMode;
	}

	float Renderer::GetPostProcessStrength()
	{
		return _postProcessStrength;
	}

	Vector3 Renderer::GetPostProcessTint()
	{
		return _postProcessTint;
	}

	void Renderer::SetPostProcessMode(PostProcessMode mode)
	{
		_postProcessMode = mode;
	}

	void Renderer::SetPostProcessStrength(float strength)
	{
		_postProcessStrength = strength;
	}

	void Renderer::SetPostProcessTint(Vector3 tint)
	{
		_postProcessTint = tint;
	}

	void Renderer::CopyRenderTarget(RenderTarget2D* source, RenderTarget2D* dest, RenderView& view)
	{
		SetBlendMode(BlendMode::Opaque, true);
		SetCullMode(CullMode::CounterClockwise, true);
		SetDepthState(DepthState::Write, true);
		_context->RSSetViewports(1, &view.Viewport);
		ResetScissor();

		// Common vertex shader to all fullscreen effects
		_context->VSSetShader(_vsPostProcess.Get(), nullptr, 0);

		// We draw a fullscreen triangle
		_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		_context->IASetInputLayout(_fullscreenTriangleInputLayout.Get());

		UINT stride = sizeof(PostProcessVertex);
		UINT offset = 0;

		_context->IASetVertexBuffers(0, 1, _fullscreenTriangleVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);

		float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		_context->ClearRenderTargetView(dest->RenderTargetView.Get(), clearColor);
		_context->OMSetRenderTargets(1, dest->RenderTargetView.GetAddressOf(), nullptr);

		_context->PSSetShader(_psPostProcessCopy.Get(), nullptr, 0);
		BindRenderTargetAsTexture(TextureRegister::ColorMap, source, SamplerStateRegister::PointWrap);
		DrawTriangles(3, 0);
	}
}
