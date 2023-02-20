#include "framework.h"
#include "Renderer/Renderer11.h"

#include "Game/camera.h"
#include "Game/control/control.h"
#include "Game/spotcam.h"
#include "Game/effects/weather.h"
#include "Math/Math.h"
#include "Specific/setup.h"
#include "Specific/trutils.h"

using namespace TEN::Effects::Environment;
using namespace TEN::Math;

TEN::Renderer::RendererHUDBar* g_HealthBar;
TEN::Renderer::RendererHUDBar* g_AirBar;
TEN::Renderer::RendererHUDBar* g_DashBar;
TEN::Renderer::RendererHUDBar* g_LoadingBar;

namespace TEN::Renderer
{
	void Renderer11::InitialiseGameBars()
	{
		std::array<Vector4, 5> healthColors = 
		{
			//top
			Vector4(82 / 255.0f,0,0,1),
			Vector4(0,82 / 255.0f,0,1),
			//center
			Vector4(78 / 255.0f,81 / 255.0f,0,1),
			//bottom
			Vector4(82 / 255.0f,0,0,1),
			Vector4(0,82 / 255.0f,0,1),
		};

		std::array<Vector4, 5> airColors = 
		{
			//top
			Vector4(0 ,0,90 / 255.0f,1),
			Vector4(0 ,47 / 255.0f,96 / 255.0f,1),
			//center
			Vector4(0,39 / 255,155 / 255.0f,1),
			//bottom
			Vector4(0 ,0,90 / 255.0f,1),
			Vector4(0 ,47 / 255.0f,96 / 255.0f,1),
		};

		std::array<Vector4, 5> loadingColors = 
		{
			//top
			Vector4(0 ,0,90 / 255.0f,1),
			Vector4(0 ,47 / 255.0f,96 / 255.0f,1),
			//center
			Vector4(0,39 / 255,155 / 255.0f,1),
			//bottom
			Vector4(0 ,0,90 / 255.0f,1),
			Vector4(0 ,47 / 255.0f,96 / 255.0f,1),
		};

		std::array<Vector4, 5> dashColors =
		{
			//top
			Vector4(78 / 255.0f,4 / 255.0f,0,1),
			Vector4(136 / 255.0f,117 / 255.0f,5 / 255.0f,1),
			//center
			Vector4(245 / 255.0f,119 / 255,24 / 255.0f,1),
			//bottom
			Vector4(78 / 255.0f,4 / 255.0f,0,1),
			Vector4(136 / 255.0f,117 / 255.0f,5 / 255.0f,1),
		};

		g_HealthBar = new RendererHUDBar(m_device.Get(), 20, 32, 150, 8, 1, healthColors);
		g_AirBar = new RendererHUDBar(m_device.Get(), 630, 32, 150, 8, 1, airColors);
		g_DashBar = new RendererHUDBar(m_device.Get(), 630, 32 + 8 + 4, 150, 8, 1, dashColors);
		g_LoadingBar = new RendererHUDBar(m_device.Get(), 325, 550, 150, 8, 1, airColors);
	}

	void Renderer11::DrawBar(float percent, const RendererHUDBar* const bar, GAME_OBJECT_ID textureSlot, int frame, bool isPoisoned)
	{
		UINT strides = sizeof(RendererVertex);
		UINT offset = 0;
		float color[] = { 0,0,0,1.0f };
	
		m_context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.0f, 0xFF);
		
		m_context->IASetInputLayout(m_inputLayout.Get());
		m_context->IASetVertexBuffers(0, 1, bar->VertexBufferBorder.Buffer.GetAddressOf(), &strides, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetIndexBuffer(bar->IndexBufferBorder.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		
		m_context->VSSetShader(m_vsHUD.Get(), nullptr, 0);
		m_context->PSSetShader(m_psHUDTexture.Get(), nullptr, 0);

		SetBlendMode(BLENDMODE_OPAQUE);
		SetDepthState(DEPTH_STATE_NONE);
		SetCullMode(CULL_MODE_NONE);

		BindConstantBufferVS(CB_HUD, m_cbHUD.get());
		BindTexture(TEXTURE_HUD, m_sprites[Objects[ID_BAR_BORDER_GRAPHIC].meshIndex].Texture, SAMPLER_LINEAR_CLAMP);

		DrawIndexedTriangles(56, 0, 0);

		m_context->PSSetShaderResources(0, 1, m_sprites[Objects[textureSlot].meshIndex].Texture->ShaderResourceView.GetAddressOf());

		m_context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.0f, 0xFF);
		
		m_context->IASetInputLayout(m_inputLayout.Get());
		m_context->IASetVertexBuffers(0, 1, bar->InnerVertexBuffer.Buffer.GetAddressOf(), &strides, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetIndexBuffer(bar->InnerIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		
		m_context->VSSetShader(m_vsHUD.Get(), nullptr, 0);
		m_context->PSSetShader(m_psHUDBarColor.Get(), nullptr, 0);

		m_stHUDBar.Percent = percent;
		m_stHUDBar.Poisoned = isPoisoned;
		m_stHUDBar.Frame = frame;
		m_cbHUDBar.updateData(m_stHUDBar, m_context.Get());
		BindConstantBufferVS(CB_HUD_BAR, m_cbHUDBar.get());
		BindConstantBufferPS(CB_HUD_BAR, m_cbHUDBar.get());

		BindTexture(TEXTURE_HUD, m_sprites[Objects[textureSlot].meshIndex].Texture, SAMPLER_LINEAR_CLAMP);

		DrawIndexedTriangles(12, 0, 0);
	}

	void Renderer11::DrawLoadingBar(float percentage)
	{
		UINT strides = sizeof(RendererVertex);
		UINT offset = 0;
		float color[] = { 0,0,0,1.0f };
		
		m_context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.0f, 0xFF);
	
		m_context->IASetInputLayout(m_inputLayout.Get());
		m_context->IASetVertexBuffers(0, 1, g_LoadingBar->VertexBufferBorder.Buffer.GetAddressOf(), &strides, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetIndexBuffer(g_LoadingBar->IndexBufferBorder.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	
		m_context->VSSetShader(m_vsHUD.Get(), NULL, 0);
		m_context->PSSetShader(m_psHUDTexture.Get(), NULL, 0);

		SetBlendMode(BLENDMODE_OPAQUE);
		SetDepthState(DEPTH_STATE_NONE);
		SetCullMode(CULL_MODE_NONE);

		BindConstantBufferVS(CB_HUD, m_cbHUD.get());
		BindTexture(TEXTURE_HUD, &loadingBarBorder, SAMPLER_LINEAR_CLAMP);

		DrawIndexedTriangles(56, 0, 0);

		m_context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.0f, 0xFF);
		
		m_context->IASetInputLayout(m_inputLayout.Get());
		m_context->IASetVertexBuffers(0, 1, g_LoadingBar->InnerVertexBuffer.Buffer.GetAddressOf(), &strides, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetIndexBuffer(g_LoadingBar->InnerIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	
		m_context->VSSetShader(m_vsHUD.Get(), NULL, 0);
		m_context->PSSetShader(m_psHUDBarColor.Get(), NULL, 0);
		
		m_stHUDBar.Percent = percentage / 100.0f;
		m_stHUDBar.Poisoned = false;
		m_stHUDBar.Frame = 0;
		m_cbHUDBar.updateData(m_stHUDBar, m_context.Get());
		BindConstantBufferVS(CB_HUD_BAR, m_cbHUDBar.get());
		BindConstantBufferPS(CB_HUD_BAR, m_cbHUDBar.get());

		BindTexture(TEXTURE_HUD, &loadingBarInner, SAMPLER_LINEAR_CLAMP);

		DrawIndexedTriangles(12, 0, 0);
	}

	void Renderer11::AddLine2D(const Vector2& origin, const Vector2& target, const Color& color)
	{
		auto line = RendererLine2D{ origin, target, color };
		m_lines2DToDraw.push_back(line);
	}

	void Renderer11::DrawOverlays(RenderView& view)
	{
		auto flashColor = Weather.FlashColor();
		if (flashColor != Vector3::Zero)
		{
			SetBlendMode(BLENDMODE_ADDITIVE);
			DrawFullScreenQuad(m_whiteTexture.ShaderResourceView.Get(), flashColor);
		}

		if (CurrentLevel == 0)
			return;

		if (!BinocularRange && !SpotcamOverlay)
			return;

		SetBlendMode(BLENDMODE_ALPHABLEND);

		if (BinocularRange && !LaserSight)
		{
			DrawFullScreenQuad(m_sprites[Objects[ID_BINOCULAR_GRAPHIC].meshIndex].Texture->ShaderResourceView.Get(), Vector3::One, true);
		}
		else if (BinocularRange && LaserSight)
		{
			DrawFullScreenQuad(m_sprites[Objects[ID_LASER_SIGHT_GRAPHIC].meshIndex].Texture->ShaderResourceView.Get(), Vector3::One);

			SetBlendMode(BLENDMODE_OPAQUE);

			// Draw the aiming point
			RendererVertex vertices[4];

			vertices[0].Position.x = -4.0f / m_screenWidth;
			vertices[0].Position.y = 4.0f / m_screenHeight;
			vertices[0].Position.z = 0.0f;
			vertices[0].UV.x = 0.0f;
			vertices[0].UV.y = 0.0f;
			vertices[0].Color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);

			vertices[1].Position.x = 4.0f / m_screenWidth;
			vertices[1].Position.y = 4.0f / m_screenHeight;
			vertices[1].Position.z = 0.0f;
			vertices[1].UV.x = 1.0f;
			vertices[1].UV.y = 0.0f;
			vertices[1].Color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);

			vertices[2].Position.x = 4.0f / m_screenWidth;
			vertices[2].Position.y = -4.0f / m_screenHeight;
			vertices[2].Position.z = 0.0f;
			vertices[2].UV.x = 1.0f;
			vertices[2].UV.y = 1.0f;
			vertices[2].Color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);

			vertices[3].Position.x = -4.0f / m_screenWidth;
			vertices[3].Position.y = -4.0f / m_screenHeight;
			vertices[3].Position.z = 0.0f;
			vertices[3].UV.x = 0.0f;
			vertices[3].UV.y = 1.0f;
			vertices[3].Color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);

			m_context->VSSetShader(m_vsFullScreenQuad.Get(), nullptr, 0);
			m_context->PSSetShader(m_psFullScreenQuad.Get(), nullptr, 0);

			m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			m_context->IASetInputLayout(m_inputLayout.Get());

			m_primitiveBatch->Begin();
			m_primitiveBatch->DrawQuad(vertices[0], vertices[1], vertices[2], vertices[3]);
			m_primitiveBatch->End();
		}
		else
		{
			// TODO: Vignette goes here! -- Lwmte, 21.08.21
		}
	}

	void Renderer11::DrawPostprocess(ID3D11RenderTargetView* target, ID3D11DepthStencilView* depthTarget, RenderView& view)
	{
		SetBlendMode(BLENDMODE_OPAQUE);

		m_context->RSSetState(m_cullCounterClockwiseRasterizerState.Get());
		m_context->ClearRenderTargetView(target, Colors::Black);
		m_context->ClearDepthStencilView(depthTarget, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		m_context->OMSetRenderTargets(1, &target, depthTarget);
		m_context->RSSetViewports(1, &view.viewport);
		ResetScissor();

		RendererVertex vertices[4];

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

		m_context->VSSetShader(m_vsFinalPass.Get(), nullptr, 0);
		m_context->PSSetShader(m_psFinalPass.Get(), nullptr, 0);

		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetInputLayout(m_inputLayout.Get());

		m_stPostProcessBuffer.FXAA = g_Configuration.Antialiasing == AntialiasingMode::Low ? 1 : 0;
		m_stPostProcessBuffer.ViewportWidth = m_screenWidth;
		m_stPostProcessBuffer.ViewportHeight = m_screenHeight;
		m_stPostProcessBuffer.ScreenFadeFactor = ScreenFadeCurrent;
		m_stPostProcessBuffer.CinematicBarsHeight = Smoothstep(CinematicBarsHeight) * SPOTCAM_CINEMATIC_BARS_HEIGHT;
		m_cbPostProcessBuffer.updateData(m_stPostProcessBuffer, m_context.Get());
		BindConstantBufferPS(CB_POSTPROCESS, m_cbPostProcessBuffer.get());

		BindTexture(TEXTURE_COLOR_MAP, &m_renderTarget, SAMPLER_ANISOTROPIC_CLAMP);

		m_primitiveBatch->Begin();
		m_primitiveBatch->DrawQuad(vertices[0], vertices[1], vertices[2], vertices[3]);
		m_primitiveBatch->End();
	}

	void Renderer11::DrawFullScreenImage(ID3D11ShaderResourceView* texture, float fade, ID3D11RenderTargetView* target,
		ID3D11DepthStencilView* depthTarget)
	{
		// Reset GPU state
		SetBlendMode(BLENDMODE_OPAQUE);
		SetCullMode(CULL_MODE_NONE);

		m_context->OMSetRenderTargets(1, &target, depthTarget);
		m_context->RSSetViewports(1, &m_viewport);
		ResetScissor();

		DrawFullScreenQuad(texture, Vector3(fade), true);
	}

	void Renderer11::DrawSpriteInScreenSpace(unsigned int spriteID, const Vector2& pos, short orient2D, const Vector4& color, float scale)
	{
		constexpr auto VERTEX_COUNT			 = 4;
		constexpr auto UV_RANGE				 = std::pair<Vector2, Vector2>(Vector2(0.0f, 0.0f), Vector2(1.0f, 1.0f));
		constexpr auto VERTEX_POINTS_DEFAULT = std::array<Vector2, VERTEX_COUNT>
		{
			Vector2(-SQRT_2, -SQRT_2),
			Vector2( SQRT_2, -SQRT_2),
			Vector2( SQRT_2,  SQRT_2),
			Vector2(-SQRT_2,  SQRT_2)
		};

		// Calculate relative screen space vertex positions.
		auto rotMatrix = Matrix::CreateRotationZ(TO_RAD(orient2D));
		auto vertexPoints = std::array<Vector2, VERTEX_COUNT>
		{
			Vector2::Transform(VERTEX_POINTS_DEFAULT[0] * (scale / 2), rotMatrix),
			Vector2::Transform(VERTEX_POINTS_DEFAULT[1] * (scale / 2), rotMatrix),
			Vector2::Transform(VERTEX_POINTS_DEFAULT[2] * (scale / 2), rotMatrix),
			Vector2::Transform(VERTEX_POINTS_DEFAULT[3] * (scale / 2), rotMatrix)
		};

		// Adjust for aspect ratio and convert to NDC.
		for (auto& vertexPoint : vertexPoints)
		{
			vertexPoint = TEN::Utils::GetAspectCorrectScreenSpacePos(vertexPoint);
			vertexPoint += pos;
			vertexPoint = TEN::Utils::ConvertScreenSpacePosToNDC(vertexPoint);
		}

		g_Renderer.PrintDebugMessage("%.3f, %.3f", vertexPoints[0].x, vertexPoints[0].y);

		// Define renderer vertices.
		auto vertices = std::array<RendererVertex, VERTEX_COUNT>{};

		// Vertex 0
		vertices[0].Position = Vector3(vertexPoints[0]);
		vertices[0].UV = UV_RANGE.first;
		vertices[0].Color = color;

		// Vertex 1
		vertices[1].Position = Vector3(vertexPoints[1]);
		vertices[1].UV.x = UV_RANGE.second.x;
		vertices[1].UV.y = UV_RANGE.first.y;
		vertices[1].Color = color;

		// Vertex 2
		vertices[2].Position = Vector3(vertexPoints[2]);
		vertices[2].UV = UV_RANGE.second;
		vertices[2].Color = color;

		// Vertex 3
		vertices[3].Position = Vector3(vertexPoints[3]);
		vertices[3].UV.x = UV_RANGE.first.x;
		vertices[3].UV.y = UV_RANGE.second.y;
		vertices[3].Color = color;

		SetBlendMode(BLENDMODE_ALPHABLEND);

		m_context->VSSetShader(m_vsFullScreenQuad.Get(), nullptr, 0);
		m_context->PSSetShader(m_psFullScreenQuad.Get(), nullptr, 0);

		const auto& spritePtr = m_sprites[Objects[spriteID].meshIndex];
		auto* texturePtr = spritePtr.Texture->ShaderResourceView.Get();
		m_context->PSSetShaderResources(0, 1, &texturePtr);
		auto* sampler = m_states->AnisotropicClamp();
		m_context->PSSetSamplers(0, 1, &sampler);

		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetInputLayout(m_inputLayout.Get());

		m_primitiveBatch->Begin();
		m_primitiveBatch->DrawQuad(vertices[0], vertices[1], vertices[2], vertices[3]);
		m_primitiveBatch->End();
	}

	void Renderer11::DrawFullScreenQuad(ID3D11ShaderResourceView* texture, Vector3 color, bool fit)
	{
		constexpr auto VERTEX_COUNT = 4;

		auto uvStart = Vector2::Zero;
		auto uvEnd	 = Vector2::One;

		if (fit)
		{
			ID3D11Texture2D* texture2D;
			texture->GetResource(reinterpret_cast<ID3D11Resource**>(&texture2D));

			auto desc = D3D11_TEXTURE2D_DESC();
			texture2D->GetDesc(&desc);

			float screenAspect = float(m_screenWidth) / float(m_screenHeight);
			float imageAspect  = float(desc.Width) / float(desc.Height);

			if (screenAspect > imageAspect)
			{
				float diff = ((screenAspect - imageAspect) / screenAspect) / 2;
				uvStart.y += diff;
				uvEnd.y -= diff;
			}
			else
			{
				float diff = ((imageAspect - screenAspect) / imageAspect) / 2;
				uvStart.x += diff;
				uvEnd.x -= diff;
			}
		}

		auto vertices = std::array<RendererVertex, VERTEX_COUNT>{};
		auto colorVec4 = Vector4(color.x, color.y, color.z, 1.0f);

		vertices[0].Position = Vector3(-1.0f, 1.0f, 0.0f);
		vertices[0].UV.x = uvStart.x;
		vertices[0].UV.y = uvStart.y;
		vertices[0].Color = colorVec4;

		vertices[1].Position = Vector3(1.0f, 1.0f, 0.0f);
		vertices[1].UV.x = uvEnd.x;
		vertices[1].UV.y = uvStart.y;
		vertices[1].Color = colorVec4;

		vertices[2].Position = Vector3(1.0f, -1.0f, 0.0f);
		vertices[2].UV.x = uvEnd.x;
		vertices[2].UV.y = uvEnd.y;
		vertices[2].Color = colorVec4;

		vertices[3].Position = Vector3(-1.0f, -1.0f, 0.0f);
		vertices[3].UV.x = uvStart.x;
		vertices[3].UV.y = uvEnd.y;
		vertices[3].Color = colorVec4;

		m_context->VSSetShader(m_vsFullScreenQuad.Get(), nullptr, 0);
		m_context->PSSetShader(m_psFullScreenQuad.Get(), nullptr, 0);

		m_context->PSSetShaderResources(0, 1, &texture);
		auto* sampler = m_states->AnisotropicClamp();
		m_context->PSSetSamplers(0, 1, &sampler);

		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetInputLayout(m_inputLayout.Get());

		m_primitiveBatch->Begin();
		m_primitiveBatch->DrawQuad(vertices[0], vertices[1], vertices[2], vertices[3]);
		m_primitiveBatch->End();
	}
}
