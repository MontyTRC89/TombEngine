#include "framework.h"
#include "Renderer/Renderer11.h"

#include "Game/camera.h"
#include "Game/control/control.h"
#include "Game/spotcam.h"
#include "Game/effects/DisplaySprite.h"
#include "Game/effects/weather.h"
#include "Game/Lara/lara.h"
#include "Game/Setup.h"
#include "Math/Math.h"
#include "Objects/game_object_ids.h"
#include "Objects/Utils/object_helper.h"
#include "Specific/trutils.h"

using namespace TEN::Effects::DisplaySprite;
using namespace TEN::Effects::Environment;
using namespace TEN::Math;

TEN::Renderer::RendererHudBar* g_AirBar;
TEN::Renderer::RendererHudBar* g_ExposureBar;
TEN::Renderer::RendererHudBar* g_HealthBar;
TEN::Renderer::RendererHudBar* g_StaminaBar;
TEN::Renderer::RendererHudBar* g_LoadingBar;

namespace TEN::Renderer
{
	void Renderer11::InitializeGameBars()
	{
		constexpr auto AIR_BAR_POS		= Vector2(630.0f, 30.0f);
		constexpr auto EXPOSURE_BAR_POS = Vector2(630.0f, 70.0f);
		constexpr auto HEALTH_BAR_POS	= Vector2(20.0f, 30.0f);
		constexpr auto STAMINA_BAR_POS	= Vector2(630.0f, 50.0f);
		constexpr auto LOADING_BAR_POS	= Vector2(325.0f, 550.0f);

		static const auto AIR_BAR_COLORS = std::array<Vector4, RendererHudBar::COLOR_COUNT>
		{
			// Top
			Vector4(0.0f, 0.0f, 0.35f, 1.0f),
			Vector4(0.0f, 0.18f, 0.38f, 1.0f),

			// Center
			Vector4(0.0f, 0.15f, 0.6f, 1.0f),

			// Bottom
			Vector4(0.0f, 0.0f, 0.35f, 1.0f),
			Vector4(0.0f, 0.18f, 0.38f, 1.0f)
		};
		
		static const auto EXPOSURE_BAR_COLORS = std::array<Vector4, RendererHudBar::COLOR_COUNT>
		{
			// Top
			Vector4(0.18f, 0.3f, 0.72f, 1.0f),
			Vector4(0.18f, 0.3f, 0.72f, 1.0f),

			// Center
			Vector4(0.18f, 0.3f, 0.72f, 1.0f),

			// Bottom
			Vector4(0.18f, 0.3f, 0.72f, 1.0f),
			Vector4(0.18f, 0.3f, 0.72f, 1.0f)
		};

		static const auto HEALTH_BAR_COLORS = std::array<Vector4, RendererHudBar::COLOR_COUNT>
		{
			// Top
			Vector4(0.32f, 0.0f, 0.0f, 1.0f),
			Vector4(0.0f, 0.32f, 0.0f, 1.0f),

			// Center
			Vector4(0.3f, 0.32f, 0.0f, 1.0f),

			// Bottom
			Vector4(0.32f, 0.0f, 0.0f, 1.0f),
			Vector4(0.0f, 0.32f, 0.0f, 1.0f)
		};
		
		static const auto STAMINA_BAR_COLORS = std::array<Vector4, RendererHudBar::COLOR_COUNT>
		{
			// Top
			Vector4(0.3f, 0.02f, 0.0f, 1.0f),
			Vector4(0.55f, 0.45f, 0.02f, 1.0f),

			// Center
			Vector4(0.95f, 0.45f, 0.09f, 1.0f),

			// Bottom
			Vector4(0.3f, 0.02f, 0.0f, 1.0f),
			Vector4(0.55f, 0.45f, 0.02f, 1.0f)
		};

		static const auto LOADING_BAR_COLORS = std::array<Vector4, RendererHudBar::COLOR_COUNT>
		{
			// Top
			Vector4(0.0f, 0.0f, 0.35f, 1.0f),
			Vector4(0.0f, 0.18f, 0.38f, 1.0f),

			// Center
			Vector4(0.0f, 0.15f, 0.6f, 1.0f),

			// Bottom
			Vector4(0.0f, 0.0f, 0.35f, 1.0f),
			Vector4(0.0f, 0.18f, 0.38f, 1.0f)
		};

		g_AirBar = new RendererHudBar(m_device.Get(), AIR_BAR_POS, RendererHudBar::SIZE_DEFAULT, 1, AIR_BAR_COLORS);
		g_ExposureBar = new RendererHudBar(m_device.Get(), EXPOSURE_BAR_POS, RendererHudBar::SIZE_DEFAULT, 1, EXPOSURE_BAR_COLORS);
		g_HealthBar = new RendererHudBar(m_device.Get(), HEALTH_BAR_POS, RendererHudBar::SIZE_DEFAULT, 1, HEALTH_BAR_COLORS);
		g_StaminaBar = new RendererHudBar(m_device.Get(), STAMINA_BAR_POS, RendererHudBar::SIZE_DEFAULT, 1, STAMINA_BAR_COLORS);
		g_LoadingBar = new RendererHudBar(m_device.Get(), LOADING_BAR_POS, RendererHudBar::SIZE_DEFAULT, 1, LOADING_BAR_COLORS);
	}

	void Renderer11::DrawBar(float percent, const RendererHudBar& bar, GAME_OBJECT_ID textureSlot, int frame, bool isPoisoned)
	{
		if (!CheckIfSlotExists(ID_BAR_BORDER_GRAPHIC, "Bar rendering"))
			return;

		unsigned int strides = sizeof(RendererVertex);
		unsigned int offset = 0;
	
		m_context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.0f, 0xFF);
		
		m_context->IASetInputLayout(m_inputLayout.Get());
		m_context->IASetVertexBuffers(0, 1, bar.VertexBufferBorder.Buffer.GetAddressOf(), &strides, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetIndexBuffer(bar.IndexBufferBorder.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		
		m_context->VSSetShader(m_vsHUD.Get(), nullptr, 0);
		m_context->PSSetShader(m_psHUDTexture.Get(), nullptr, 0);

		SetBlendMode(BLENDMODE_OPAQUE);
		SetDepthState(DEPTH_STATE_NONE);
		SetCullMode(CULL_MODE_NONE);

		BindConstantBufferVS(CB_HUD, m_cbHUD.get());

		RendererSprite* borderSprite = &m_sprites[Objects[ID_BAR_BORDER_GRAPHIC].meshIndex];
		m_stHUDBar.BarStartUV = borderSprite->UV[0];
		m_stHUDBar.BarScale = Vector2(borderSprite->Width / (float)borderSprite->Texture->Width, borderSprite->Height / (float)borderSprite->Texture->Height);
		m_cbHUDBar.updateData(m_stHUDBar, m_context.Get());
		BindConstantBufferVS(CB_HUD_BAR, m_cbHUDBar.get());
		BindConstantBufferPS(CB_HUD_BAR, m_cbHUDBar.get());
		 
		BindTexture(TEXTURE_HUD, borderSprite->Texture, SAMPLER_LINEAR_CLAMP);

		DrawIndexedTriangles(56, 0, 0);

		m_context->PSSetShaderResources(0, 1, m_sprites[Objects[textureSlot].meshIndex].Texture->ShaderResourceView.GetAddressOf());

		m_context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.0f, 0xFF);
		
		m_context->IASetInputLayout(m_inputLayout.Get());
		m_context->IASetVertexBuffers(0, 1, bar.InnerVertexBuffer.Buffer.GetAddressOf(), &strides, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetIndexBuffer(bar.InnerIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		
		m_context->VSSetShader(m_vsHUD.Get(), nullptr, 0);
		m_context->PSSetShader(m_psHUDBarColor.Get(), nullptr, 0);

		m_stHUDBar.Percent = percent;
		m_stHUDBar.Poisoned = isPoisoned;
		m_stHUDBar.Frame = frame;	
		RendererSprite* innerSprite = &m_sprites[Objects[textureSlot].meshIndex];
		m_stHUDBar.BarStartUV = innerSprite->UV[0];
		m_stHUDBar.BarScale = Vector2(innerSprite->Width / (float)innerSprite->Texture->Width, innerSprite->Height / (float)innerSprite->Texture->Height);
		m_cbHUDBar.updateData(m_stHUDBar, m_context.Get());

		BindConstantBufferVS(CB_HUD_BAR, m_cbHUDBar.get());
		BindConstantBufferPS(CB_HUD_BAR, m_cbHUDBar.get());
		 
		BindTexture(TEXTURE_HUD, innerSprite->Texture, SAMPLER_LINEAR_CLAMP);

		DrawIndexedTriangles(12, 0, 0);
	}

	void Renderer11::DrawLoadingBar(float percentage)
	{
		unsigned int strides = sizeof(RendererVertex);
		unsigned int offset = 0;
		
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
		BindTexture(TEXTURE_HUD, &m_loadingBarBorder, SAMPLER_LINEAR_CLAMP);

		m_stHUDBar.BarStartUV = Vector2::Zero;
		m_stHUDBar.BarScale = Vector2::One;
		m_cbHUDBar.updateData(m_stHUDBar, m_context.Get());
		BindConstantBufferVS(CB_HUD_BAR, m_cbHUDBar.get());
		BindConstantBufferPS(CB_HUD_BAR, m_cbHUDBar.get());

		DrawIndexedTriangles(56, 0, 0);

		m_context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.0f, 0xFF);
		
		m_context->IASetInputLayout(m_inputLayout.Get());
		m_context->IASetVertexBuffers(0, 1, g_LoadingBar->InnerVertexBuffer.Buffer.GetAddressOf(), &strides, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetIndexBuffer(g_LoadingBar->InnerIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	
		m_context->VSSetShader(m_vsHUD.Get(), nullptr, 0);
		m_context->PSSetShader(m_psHUDBarColor.Get(), nullptr, 0);
		
		m_stHUDBar.Percent = percentage / 100.0f;
		m_stHUDBar.Poisoned = false;
		m_stHUDBar.Frame = 0;
		m_cbHUDBar.updateData(m_stHUDBar, m_context.Get());
		BindConstantBufferVS(CB_HUD_BAR, m_cbHUDBar.get());
		BindConstantBufferPS(CB_HUD_BAR, m_cbHUDBar.get());

		BindTexture(TEXTURE_HUD, &m_loadingBarInner, SAMPLER_LINEAR_CLAMP);

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

		if (!Lara.Control.Look.OpticRange && !SpotcamOverlay)
			return;

		SetBlendMode(BLENDMODE_ALPHABLEND);

		if (Lara.Control.Look.OpticRange != 0 && !Lara.Control.Look.IsUsingLasersight)
		{
			DrawFullScreenSprite(&m_sprites[Objects[ID_BINOCULAR_GRAPHIC].meshIndex], Vector3::One, false);
		}
		else if (Lara.Control.Look.OpticRange != 0 && Lara.Control.Look.IsUsingLasersight)
		{
			DrawFullScreenSprite(&m_sprites[Objects[ID_LASER_SIGHT_GRAPHIC].meshIndex], Vector3::One);

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
		m_context->RSSetViewports(1, &view.Viewport);
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

		m_stPostProcessBuffer.FXAA = g_Configuration.AntialiasingMode == AntialiasingMode::Low ? 1 : 0;
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

	void Renderer11::DrawDisplaySprites(RenderView& renderView)
	{
		constexpr auto VERTEX_COUNT = 4;

		if (renderView.DisplaySpritesToDraw.empty())
			return;

		m_context->VSSetShader(m_vsFullScreenQuad.Get(), nullptr, 0);
		m_context->PSSetShader(m_psFullScreenQuad.Get(), nullptr, 0);

		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetInputLayout(m_inputLayout.Get());

		Texture2D* texture2DPtr = nullptr;
		for (const auto& spriteToDraw : renderView.DisplaySpritesToDraw)
		{
			if (texture2DPtr == nullptr)
			{
				m_primitiveBatch->Begin();

				BindTexture(TEXTURE_COLOR_MAP, spriteToDraw.SpritePtr->Texture, SAMPLER_ANISOTROPIC_CLAMP);
				SetBlendMode(spriteToDraw.BlendMode);
			}
			else if (texture2DPtr != spriteToDraw.SpritePtr->Texture || lastBlendMode != spriteToDraw.BlendMode)
			{
				m_primitiveBatch->End();
				m_primitiveBatch->Begin();

				BindTexture(TEXTURE_COLOR_MAP, spriteToDraw.SpritePtr->Texture, SAMPLER_ANISOTROPIC_CLAMP);
				SetBlendMode(spriteToDraw.BlendMode);
			}

			// Calculate vertex base.
			auto vertices = std::array<Vector2, VERTEX_COUNT>
			{
				spriteToDraw.Size / 2,
				Vector2(-spriteToDraw.Size.x, spriteToDraw.Size.y) / 2,
				-spriteToDraw.Size / 2,
				Vector2(spriteToDraw.Size.x, -spriteToDraw.Size.y) / 2
			};

			// Transform vertices.
			// NOTE: Must rotate 180 degrees to account for +Y being down.
			auto rotMatrix = Matrix::CreateRotationZ(TO_RAD(spriteToDraw.Orientation + ANGLE(180.0f)));
			for (auto& vertex : vertices)
			{
				// Rotate.
				vertex = Vector2::Transform(vertex, rotMatrix);

				// Apply aspect correction.
				vertex *= spriteToDraw.AspectCorrection;

				// Offset to position and convert to NDC.
				vertex += spriteToDraw.Position;
				vertex = TEN::Utils::Convert2DPositionToNDC(vertex);
			}

			// Define renderer vertices.
			auto rVertices = std::array<RendererVertex, VERTEX_COUNT>{};
			for (int i = 0; i < rVertices.size(); i++)
			{
				rVertices[i].Position = Vector3(vertices[i]);
				rVertices[i].UV = spriteToDraw.SpritePtr->UV[i];
				rVertices[i].Color = Vector4(spriteToDraw.Color.x, spriteToDraw.Color.y, spriteToDraw.Color.z, spriteToDraw.Color.w);
			}
			
			m_primitiveBatch->DrawQuad(rVertices[0], rVertices[1], rVertices[2], rVertices[3]);

			texture2DPtr = spriteToDraw.SpritePtr->Texture;
		}
		
		m_primitiveBatch->End();
	}

	void Renderer11::DrawFullScreenQuad(ID3D11ShaderResourceView* texture, Vector3 color, bool fit)
	{
		constexpr auto VERTEX_COUNT = 4;
		constexpr auto UV_RANGE		= std::pair<Vector2, Vector2>(Vector2(0.0f), Vector2(1.0f));

		auto uvStart = Vector2::Zero;
		auto uvEnd	 = Vector2::One;

		if (fit)
		{
			ID3D11Texture2D* texture2DPtr;
			texture->GetResource(reinterpret_cast<ID3D11Resource**>(&texture2DPtr));

			auto desc = D3D11_TEXTURE2D_DESC();
			texture2DPtr->GetDesc(&desc);

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

	void Renderer11::DrawFullScreenSprite(RendererSprite* sprite, DirectX::SimpleMath::Vector3 color, bool fit)
	{
		Vector2 uvStart = { 0.0f, 0.0f };
		Vector2 uvEnd = { 1.0f, 1.0f };

		ID3D11ShaderResourceView* texture = sprite->Texture->ShaderResourceView.Get();

		if (fit)
		{
			float screenAspect = float(m_screenWidth) / float(m_screenHeight);
			float imageAspect = float(sprite->Width) / float(sprite->Height);

			if (screenAspect > imageAspect)
			{
				float diff = (screenAspect - imageAspect) / screenAspect / 2;
				uvStart.y += diff;
				uvEnd.y -= diff;
			}
			else
			{
				float diff = (imageAspect - screenAspect) / imageAspect / 2;
				uvStart.x += diff;
				uvEnd.x -= diff;
			}
		}

		auto scale = Vector2(sprite->Width / (float)sprite->Texture->Width, sprite->Height / (float)sprite->Texture->Height);
		uvStart.x = uvStart.x * scale.x + sprite->UV[0].x;
		uvStart.y = uvStart.y * scale.y + sprite->UV[0].y;
		uvEnd.x = uvEnd.x * scale.x + sprite->UV[0].x;
		uvEnd.y = uvEnd.y * scale.y + sprite->UV[0].y;

		RendererVertex vertices[4];

		vertices[0].Position.x = -1.0f;
		vertices[0].Position.y = 1.0f;
		vertices[0].Position.z = 0.0f;
		vertices[0].UV.x = uvStart.x;
		vertices[0].UV.y = uvStart.y;
		vertices[0].Color = Vector4(color.x, color.y, color.z, 1.0f);

		vertices[1].Position.x = 1.0f;
		vertices[1].Position.y = 1.0f;
		vertices[1].Position.z = 0.0f;
		vertices[1].UV.x = uvEnd.x;
		vertices[1].UV.y = uvStart.y;
		vertices[1].Color = Vector4(color.x, color.y, color.z, 1.0f);

		vertices[2].Position.x = 1.0f;
		vertices[2].Position.y = -1.0f;
		vertices[2].Position.z = 0.0f;
		vertices[2].UV.x = uvEnd.x;
		vertices[2].UV.y = uvEnd.y;
		vertices[2].Color = Vector4(color.x, color.y, color.z, 1.0f);

		vertices[3].Position.x = -1.0f;
		vertices[3].Position.y = -1.0f;
		vertices[3].Position.z = 0.0f;
		vertices[3].UV.x = uvStart.x;
		vertices[3].UV.y = uvEnd.y;
		vertices[3].Color = Vector4(color.x, color.y, color.z, 1.0f);

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

	void Renderer11::AddDisplaySprite(const RendererSprite& sprite, const Vector2& pos2D, short orient, const Vector2& size, const Vector4& color,
									  int priority, BLEND_MODES blendMode, const Vector2& aspectCorrection, RenderView& renderView)
	{
		auto spriteToDraw = RendererDisplaySpriteToDraw{};

		spriteToDraw.SpritePtr = &sprite;
		spriteToDraw.Position = pos2D;
		spriteToDraw.Orientation = orient;
		spriteToDraw.Size = size;
		spriteToDraw.Color = color;
		spriteToDraw.Priority = priority;
		spriteToDraw.BlendMode = blendMode;
		spriteToDraw.AspectCorrection = aspectCorrection;

		renderView.DisplaySpritesToDraw.push_back(spriteToDraw);
	}

	void Renderer11::CollectDisplaySprites(RenderView& renderView)
	{
		constexpr auto DISPLAY_SPACE_ASPECT = SCREEN_SPACE_RES.x / SCREEN_SPACE_RES.y;

		// Calculate screen aspect ratio.
		auto screenRes = GetScreenResolution().ToVector2();
		float screenResAspect = screenRes.x / screenRes.y;

		// Calculate aspect ratio correction base.
		auto aspectCorrectionBase = screenResAspect / DISPLAY_SPACE_ASPECT;

		for (const auto& displaySprite : DisplaySprites)
		{
			const auto& sprite = m_sprites[Objects[displaySprite.ObjectID].meshIndex + displaySprite.SpriteID];

			// Calculate sprite aspect ratio.
			float spriteAspect = (float)sprite.Width / (float)sprite.Height;

			auto halfSize = Vector2::Zero;
			auto aspectCorrection = Vector2::One;

			// Calculate size and aspect correction.
			switch (displaySprite.ScaleMode)
			{
			default:
			case DisplaySpriteScaleMode::Fit:
				if (screenResAspect >= spriteAspect)
				{
					halfSize = (Vector2(SCREEN_SPACE_RES.y) * displaySprite.Scale) / 2;
					halfSize.x *= (spriteAspect >= 1.0f) ? spriteAspect : (1.0f / spriteAspect);
					
					aspectCorrection.x = 1.0f / aspectCorrectionBase;
				}
				else
				{
					halfSize = (Vector2(SCREEN_SPACE_RES.x) * displaySprite.Scale) / 2;
					halfSize.y *= (spriteAspect >= 1.0f) ? (1.0f / spriteAspect) : spriteAspect;

					aspectCorrection.y = aspectCorrectionBase;
				}
				break;

			case DisplaySpriteScaleMode::Fill:
				if (screenResAspect >= spriteAspect)
				{
					halfSize = (Vector2(SCREEN_SPACE_RES.x) * displaySprite.Scale) / 2;
					halfSize.y *= (spriteAspect >= 1.0f) ? (1.0f / spriteAspect) : spriteAspect;

					aspectCorrection.y = aspectCorrectionBase;
				}
				else
				{
					halfSize = (Vector2(SCREEN_SPACE_RES.y) * displaySprite.Scale) / 2;
					halfSize.x *= (spriteAspect >= 1.0f) ? spriteAspect : (1.0f / spriteAspect);

					aspectCorrection.x = 1.0f / aspectCorrectionBase;
				}
				break;

			case DisplaySpriteScaleMode::Stretch:
				if (screenResAspect >= 1.0f)
				{
					halfSize = (SCREEN_SPACE_RES.x * displaySprite.Scale) / 2;
					halfSize.y *= (screenResAspect >= 1.0f) ? (1.0f / screenResAspect) : screenResAspect;

					aspectCorrection.y = aspectCorrectionBase;
				}
				else
				{
					halfSize = (Vector2(SCREEN_SPACE_RES.y) * displaySprite.Scale) / 2;
					halfSize.x *= (screenResAspect >= 1.0f) ? (1.0f / screenResAspect) : screenResAspect;

					aspectCorrection.x = 1.0f / aspectCorrectionBase;
				}
				break;
			}

			// Calculate position offset.
			auto offset = Vector2::Zero;
			switch (displaySprite.AlignMode)
			{
			default:
			case DisplaySpriteAlignMode::Center:
				break;

			case DisplaySpriteAlignMode::CenterTop:
				offset = Vector2(0.0f, halfSize.y);
				break;

			case DisplaySpriteAlignMode::CenterBottom:
				offset = Vector2(0.0f, -halfSize.y);
				break;

			case DisplaySpriteAlignMode::CenterLeft:
				offset = Vector2(halfSize.x, 0.0f);
				break;
				
			case DisplaySpriteAlignMode::CenterRight:
				offset = Vector2(-halfSize.x, 0.0f);
				break;

			case DisplaySpriteAlignMode::TopLeft:
				offset = Vector2(halfSize.x, halfSize.y);
				break;

			case DisplaySpriteAlignMode::TopRight:
				offset = Vector2(-halfSize.x, halfSize.y);
				break;

			case DisplaySpriteAlignMode::BottomLeft:
				offset = Vector2(halfSize.x, -halfSize.y);
				break;

			case DisplaySpriteAlignMode::BottomRight:
				offset = Vector2(-halfSize.x, -halfSize.y);
				break;
			}
			offset *= aspectCorrection;

			AddDisplaySprite(
				sprite,
				displaySprite.Position + offset,
				displaySprite.Orientation,
				halfSize * 2,
				displaySprite.Color,
				displaySprite.Priority,
				displaySprite.BlendMode,
				aspectCorrection,
				renderView);
		}

		std::sort(
			renderView.DisplaySpritesToDraw.begin(), renderView.DisplaySpritesToDraw.end(),
			[](const RendererDisplaySpriteToDraw& spriteToDraw0, const RendererDisplaySpriteToDraw& spriteToDraw1)
			{
				// Same priority; sort by blend mode.
				if (spriteToDraw0.Priority == spriteToDraw1.Priority)
					return (spriteToDraw0.BlendMode < spriteToDraw1.BlendMode);

				// Sort by priority.
				return (spriteToDraw0.Priority < spriteToDraw1.Priority);
			});
	}
}
