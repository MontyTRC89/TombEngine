#include "framework.h"
#include "Renderer/Renderer.h"

using namespace TEN::Renderer::Graphics;

namespace TEN::Renderer
{
	void Renderer::ApplySMAA(RenderTarget2D* renderTarget, RenderView& view)
	{
		// Copy render target to SMAA scene target.
		float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		_context->ClearRenderTargetView(_SMAASceneRenderTarget.RenderTargetView.Get(), clearColor);
		_context->ClearDepthStencilView(_SMAASceneRenderTarget.DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		_context->OMSetRenderTargets(1, _SMAASceneRenderTarget.RenderTargetView.GetAddressOf(), _SMAASceneRenderTarget.DepthStencilView.Get());
		_postProcess->SetEffect(BasicPostProcess::Copy);
		_postProcess->SetSourceTexture(renderTarget->ShaderResourceView.Get());
		_postProcess->Process(_context.Get());

		// 1) Edge detection using color method (also depth and luma available).
		_context->ClearRenderTargetView(_SMAAEdgesRenderTarget.RenderTargetView.Get(), clearColor);
		_context->ClearRenderTargetView(_SMAABlendRenderTarget.RenderTargetView.Get(), clearColor);

		_context->RSSetState(_cullCounterClockwiseRasterizerState.Get());
		_context->OMSetRenderTargets(1, _SMAAEdgesRenderTarget.RenderTargetView.GetAddressOf(), nullptr);
		_context->RSSetViewports(1, &view.Viewport);
		ResetScissor();
		SetBlendMode(BlendMode::Opaque);

		_context->VSSetShader(_SMAAEdgeDetectionVS.Get(), nullptr, 0);
		_context->PSSetShader(_SMAAColorEdgeDetectionPS.Get(), nullptr, 0);
		 
		_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		_context->IASetInputLayout(_SMAATriangleInputLayout.Get());

		_stSMAABuffer.BlendFactor = 1.0f;
		_cbSMAABuffer.updateData(_stSMAABuffer, _context.Get());
		BindConstantBufferPS(static_cast<ConstantBufferRegister>(13), _cbSMAABuffer.get());

		BindRenderTargetAsTexture(static_cast<TextureRegister>(0), &_SMAASceneRenderTarget, SamplerStateRegister::LinearClamp);
		BindRenderTargetAsTexture(static_cast<TextureRegister>(1), &_SMAASceneSRGBRenderTarget, SamplerStateRegister::LinearClamp);
		BindRenderTargetAsTexture(static_cast<TextureRegister>(5), &_SMAAEdgesRenderTarget, SamplerStateRegister::LinearClamp);
		BindRenderTargetAsTexture(static_cast<TextureRegister>(6), &_SMAABlendRenderTarget, SamplerStateRegister::LinearClamp);
		BindTexture(static_cast<TextureRegister>(7), &_SMAAAreaTexture, SamplerStateRegister::LinearClamp);
		BindTexture(static_cast<TextureRegister>(8), &_SMAASearchTexture, SamplerStateRegister::LinearClamp);

		SMAAVertex vertices[3];

		vertices[0].Position = Vector3(-1.0f, -1.0f, 1.0f);
		vertices[1].Position = Vector3(-1.0f, 3.0f, 1.0f);
		vertices[2].Position = Vector3(3.0f, -1.0f, 1.0f);

		vertices[0].UV = Vector2(0.0f, 1.0f);
		vertices[1].UV = Vector2(0.0f, -1.0f);
		vertices[2].UV = Vector2(2.0f, 1.0f);

		_SMAAprimitiveBatch->Begin();
		_SMAAprimitiveBatch->DrawTriangle(vertices[0], vertices[1], vertices[2]);
		_SMAAprimitiveBatch->End();

		// 2) Blend weights calculation.
		_context->OMSetRenderTargets(1, _SMAABlendRenderTarget.RenderTargetView.GetAddressOf(), nullptr);

		_context->VSSetShader(_SMAABlendingWeightCalculationVS.Get(), nullptr, 0);
		_context->PSSetShader(_SMAABlendingWeightCalculationPS.Get(), nullptr, 0);

		_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		_context->IASetInputLayout(_SMAATriangleInputLayout.Get());

		_stSMAABuffer.SubsampleIndices = Vector4::Zero;
		_cbSMAABuffer.updateData(_stSMAABuffer, _context.Get());

		BindRenderTargetAsTexture(static_cast<TextureRegister>(0), &_SMAASceneRenderTarget, SamplerStateRegister::LinearClamp);
		BindRenderTargetAsTexture(static_cast<TextureRegister>(1), &_SMAASceneSRGBRenderTarget, SamplerStateRegister::LinearClamp);
		BindRenderTargetAsTexture(static_cast<TextureRegister>(5), &_SMAAEdgesRenderTarget, SamplerStateRegister::LinearClamp);
		BindRenderTargetAsTexture(static_cast<TextureRegister>(6), &_SMAABlendRenderTarget, SamplerStateRegister::LinearClamp);
		BindTexture(static_cast<TextureRegister>(7), &_SMAAAreaTexture, SamplerStateRegister::LinearClamp);
		BindTexture(static_cast<TextureRegister>(8), &_SMAASearchTexture, SamplerStateRegister::LinearClamp);

		_SMAAprimitiveBatch->Begin();
		_SMAAprimitiveBatch->DrawTriangle(vertices[0], vertices[1], vertices[2]);
		_SMAAprimitiveBatch->End();

		// 3) Neighborhood blending.
		_context->OMSetRenderTargets(1, renderTarget->RenderTargetView.GetAddressOf(), nullptr);

		_context->VSSetShader(_SMAANeighborhoodBlendingVS.Get(), nullptr, 0);
		_context->PSSetShader(_SMAANeighborhoodBlendingPS.Get(), nullptr, 0);

		_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		_context->IASetInputLayout(_SMAATriangleInputLayout.Get());

		BindRenderTargetAsTexture(static_cast<TextureRegister>(0), &_SMAASceneRenderTarget, SamplerStateRegister::LinearClamp);
		BindRenderTargetAsTexture(static_cast<TextureRegister>(1), &_SMAASceneSRGBRenderTarget, SamplerStateRegister::LinearClamp);
		BindRenderTargetAsTexture(static_cast<TextureRegister>(5), &_SMAAEdgesRenderTarget, SamplerStateRegister::LinearClamp);
		BindRenderTargetAsTexture(static_cast<TextureRegister>(6), &_SMAABlendRenderTarget, SamplerStateRegister::LinearClamp);
		BindTexture(static_cast<TextureRegister>(7), &_SMAAAreaTexture, SamplerStateRegister::LinearClamp);
		BindTexture(static_cast<TextureRegister>(8), &_SMAASearchTexture, SamplerStateRegister::LinearClamp);

		_SMAAprimitiveBatch->Begin();
		_SMAAprimitiveBatch->DrawTriangle(vertices[0], vertices[1], vertices[2]);
		_SMAAprimitiveBatch->End();

		_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		_context->IASetInputLayout(_inputLayout.Get());
	}

	void Renderer::ApplyFXAA(RenderTarget2D* renderTarget, RenderView& view)
	{
		SetBlendMode(BlendMode::Opaque);
		SetCullMode(CullMode::CounterClockwise);
		SetDepthState(DepthState::Write);
		_context->RSSetViewports(1, &view.Viewport);
		ResetScissor();

		// Copy render target to temp render target.
		float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		_context->ClearRenderTargetView(_tempRenderTarget.RenderTargetView.Get(), clearColor);
		_context->ClearDepthStencilView(_tempRenderTarget.DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		_context->OMSetRenderTargets(1, _tempRenderTarget.RenderTargetView.GetAddressOf(), _tempRenderTarget.DepthStencilView.Get());
		_postProcess->SetEffect(BasicPostProcess::Copy);
		_postProcess->SetSourceTexture(renderTarget->ShaderResourceView.Get());
		_postProcess->Process(_context.Get());

		// Apply FXAA
		_context->ClearRenderTargetView(renderTarget->RenderTargetView.Get(), Colors::Black);
		_context->ClearDepthStencilView(renderTarget->DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		_context->OMSetRenderTargets(1, renderTarget->RenderTargetView.GetAddressOf(), renderTarget->DepthStencilView.Get());

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

		_context->VSSetShader(_vsFXAA.Get(), nullptr, 0);
		_context->PSSetShader(_psFXAA.Get(), nullptr, 0);

		_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		_context->IASetInputLayout(_inputLayout.Get());

		_stPostProcessBuffer.ViewportWidth = _screenWidth;
		_stPostProcessBuffer.ViewportHeight = _screenHeight;
		_cbPostProcessBuffer.updateData(_stPostProcessBuffer, _context.Get());
		
		BindTexture(TextureRegister::ColorMap, &_tempRenderTarget, SamplerStateRegister::AnisotropicClamp);

		_primitiveBatch->Begin();
		_primitiveBatch->DrawQuad(vertices[0], vertices[1], vertices[2], vertices[3]);
		_primitiveBatch->End();
	}
}