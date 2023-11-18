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

		// Copy render target to post process render target.
		float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		_context->ClearRenderTargetView(_postProcessRenderTarget[0].RenderTargetView.Get(), clearColor);
		_context->OMSetRenderTargets(1, _postProcessRenderTarget[0].RenderTargetView.GetAddressOf(), nullptr);
		_postProcess->SetEffect(BasicPostProcess::Copy);
		_postProcess->SetSourceTexture(_renderTarget.ShaderResourceView.Get());
		_postProcess->Process(_context.Get());

		int currentRenderTarget = 0;
		int destinationRenderTarget = 1;

		// Apply color scheme
		if (_postProcessColorScheme != PostProcessColorScheme::Normal)
		{
			_context->ClearRenderTargetView(_postProcessRenderTarget[destinationRenderTarget].RenderTargetView.Get(), clearColor);
			_context->OMSetRenderTargets(1, _postProcessRenderTarget[destinationRenderTarget].RenderTargetView.GetAddressOf(), nullptr);
			_postProcess->SetEffect(_postProcessColorScheme == PostProcessColorScheme::Sepia ? BasicPostProcess::Sepia : BasicPostProcess::Monochrome);
			_postProcess->SetSourceTexture(_postProcessRenderTarget[currentRenderTarget].ShaderResourceView.Get());
			_postProcess->Process(_context.Get());

			destinationRenderTarget = 0;
			currentRenderTarget = 1;
		}

		// Do the final pass
		_context->VSSetShader(_vsFinalPass.Get(), nullptr, 0);
		_context->PSSetShader(_psFinalPass.Get(), nullptr, 0);

		SetCullMode(CullMode::CounterClockwise);
		SetBlendMode(BlendMode::Opaque);

		_context->ClearRenderTargetView(renderTarget->RenderTargetView.Get(), Colors::Black);
		_context->ClearDepthStencilView(renderTarget->DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		_context->OMSetRenderTargets(1, renderTarget->RenderTargetView.GetAddressOf(), renderTarget->DepthStencilView.Get());
		_context->RSSetViewports(1, &view.Viewport);
		ResetScissor();

		Vertex vertices[4];

		vertices[0].Position.x = -1.0f;
		vertices[0].Position.y = 1.0f;
		vertices[0].Position.z = 0.0f;
		vertices[0].UV.x = 0.0f;
		vertices[0].UV.y = 0.0f;
		vertices[0].Color = Vector4::One;

		vertices[1].Position.x = 1.0f;
		vertices[1].Position.y = 1.0f;
		vertices[1].Position.z = 0.0f;
		vertices[1].UV.x = 1.0f;
		vertices[1].UV.y = 0.0f;
		vertices[1].Color = Vector4::One;

		vertices[2].Position.x = 1.0f;
		vertices[2].Position.y = -1.0f;
		vertices[2].Position.z = 0.0f;
		vertices[2].UV.x = 1.0f;
		vertices[2].UV.y = 1.0f;
		vertices[2].Color = Vector4::One;

		vertices[3].Position.x = -1.0f;
		vertices[3].Position.y = -1.0f;
		vertices[3].Position.z = 0.0f;
		vertices[3].UV.x = 0.0f;
		vertices[3].UV.y = 1.0f;
		vertices[3].Color = Vector4::One;

		_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		_context->IASetInputLayout(_inputLayout.Get());

		_stPostProcessBuffer.ViewportWidth = _screenWidth;
		_stPostProcessBuffer.ViewportHeight = _screenHeight;
		_stPostProcessBuffer.ScreenFadeFactor = ScreenFadeCurrent;
		_stPostProcessBuffer.CinematicBarsHeight = Smoothstep(CinematicBarsHeight) * SPOTCAM_CINEMATIC_BARS_HEIGHT;
		_cbPostProcessBuffer.updateData(_stPostProcessBuffer, _context.Get());
		BindConstantBufferPS(ConstantBufferRegister::PostProcess, _cbPostProcessBuffer.get());

		BindTexture(TextureRegister::ColorMap, &_postProcessRenderTarget[currentRenderTarget], SamplerStateRegister::AnisotropicClamp);

		_primitiveBatch->Begin();
		_primitiveBatch->DrawQuad(vertices[0], vertices[1], vertices[2], vertices[3]);
		_primitiveBatch->End();
	}
}
