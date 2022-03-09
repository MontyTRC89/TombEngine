#include "framework.h"
#include "Renderer/Renderer11.h"
#include "Specific/configuration.h"
#include "Game/savegame.h"
#include "Game/health.h"
#include "Game/camera.h"
#include "Game/animation.h"
#include "Game/gui.h"
#include "Game/Lara/lara.h"
#include "Objects/Generic/Object/rope.h"
#include "Game/effects/tomb4fx.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "Game/control/control.h"
#include "Objects/TR5/Emitter/tr5_rats_emitter.h"
#include "Objects/TR5/Emitter/tr5_bats_emitter.h"
#include "ConstantBuffers/CameraMatrixBuffer.h"
#include "Objects/TR4/Entity/tr4_beetle_swarm.h"
#include "RenderView/RenderView.h"
#include "Game/effects/hair.h"
#include "Game/effects/weather.h"
#include "Specific/winmain.h"
#include "Objects/Effects/tr4_locusts.h"
#include "Game/control/volume.h"
#include "Game/items.h"
#include "Objects/Generic/Object/rope.h"
#include <chrono>
#include <algorithm>
#include <execution>

using namespace TEN::Entities::Generic;

extern TEN::Renderer::RendererHUDBar* g_DashBar;
extern TEN::Renderer::RendererHUDBar* g_SFXVolumeBar;
extern TEN::Renderer::RendererHUDBar* g_MusicVolumeBar;
extern GUNSHELL_STRUCT Gunshells[MAX_GUNSHELL];

namespace TEN::Renderer
{
	using namespace TEN::Renderer;
	using namespace std::chrono;

	void Renderer11::drawPickup(short objectNum)
	{
		drawObjectOn2DPosition(700 + PickupX, 450, objectNum, 0, m_pickupRotation, 0, 0.5f); // TODO: + PickupY
		m_pickupRotation += 45 * 360 / 30;
	}

	void Renderer11::drawObjectOn2DPosition(short x, short y, short objectNum, short rotX, short rotY, short rotZ,
	                                        float scale1)
	{
		Matrix translation;
		Matrix rotation;
		Matrix world;
		Matrix view;
		Matrix projection;
		Matrix scale;

		UINT stride = sizeof(RendererVertex);
		UINT offset = 0;

		x *= (ScreenWidth / 800.0f);
		y *= (ScreenHeight / 600.0f);

		auto index = g_Gui.ConvertObjectToInventoryItem(objectNum);

		if (index != -1)
		{
			auto objme = &inventry_objects_list[index];
			y += objme->yoff;
			rotX += objme->xrot;
			rotY += objme->yrot;
			rotZ += objme->zrot;
		}

		view = Matrix::CreateLookAt(Vector3(0.0f, 0.0f, 2048.0f), Vector3(0.0f, 0.0f, 0.0f),
		                            Vector3(0.0f, -1.0f, 0.0f));
		projection = Matrix::CreateOrthographic(ScreenWidth, ScreenHeight, -1024.0f, 1024.0f);

		auto& moveableObj = m_moveableObjects[objectNum];
		if (!moveableObj)
			return;

		OBJECT_INFO* obj = &Objects[objectNum];

		if (obj->animIndex != -1)
		{
			ANIM_FRAME* frame[] = {&g_Level.Frames[g_Level.Anims[obj->animIndex].framePtr]};
			UpdateAnimation(NULL, *moveableObj, frame, 0, 0, 0xFFFFFFFF);
		}

		Vector3 pos = m_viewportToolkit.Unproject(Vector3(x, y, 1), projection, view, Matrix::Identity);

		// Clear just the Z-buffer so we can start drawing on top of the scene
		ID3D11DepthStencilView* dsv;
		m_context->OMGetRenderTargets(1, NULL, &dsv);
		m_context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		// Set vertex buffer
		m_context->IASetVertexBuffers(0, 1, m_moveablesVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetInputLayout(m_inputLayout.Get());
		m_context->IASetIndexBuffer(m_moveablesIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		// Set shaders
		m_context->VSSetShader(m_vsInventory.Get(), NULL, 0);
		m_context->PSSetShader(m_psInventory.Get(), NULL, 0);

		// Set matrices
		CCameraMatrixBuffer HudCamera;
		HudCamera.ViewProjection = view * projection;
		m_cbCameraMatrices.updateData(HudCamera, m_context.Get());
		m_context->VSSetConstantBuffers(0, 1, m_cbCameraMatrices.get());

		for (int n = 0; n < (*moveableObj).ObjectMeshes.size(); n++)
		{
			RendererMesh* mesh = (*moveableObj).ObjectMeshes[n];

			/*if (GLOBAL_invMode)
			{
			    InventoryObject* objme;

			    objme = &inventry_objects_list[g_Gui.ConvertObjectToInventoryItem(objectNum)];

			    if (!(objme->meshbits & (1 << n)))
			        continue;
			}*/

			// Finish the world matrix
			translation = Matrix::CreateTranslation(pos.x, pos.y, pos.z + 1024.0f);
			rotation = Matrix::CreateFromYawPitchRoll(TO_RAD(rotY), TO_RAD(rotX), TO_RAD(rotZ));
			scale = Matrix::CreateScale(scale1);

			world = scale * rotation;
			world = world * translation;

			if (obj->animIndex != -1)
				m_stItem.World = ((*moveableObj).AnimationTransforms[n] * world);
			else
				m_stItem.World = ((*moveableObj).BindPoseTransforms[n] * world);
			m_stItem.AmbientLight = Vector4(0.5f, 0.5f, 0.5f, 1.0f);
			m_cbItem.updateData(m_stItem, m_context.Get());
			m_context->VSSetConstantBuffers(1, 1, m_cbItem.get());
			m_context->PSSetConstantBuffers(1, 1, m_cbItem.get());

			for (auto& bucket : mesh->buckets)
			{
				if (bucket.NumVertices == 0)
					continue;

				setBlendMode(bucket.BlendMode);

				BindTexture(TextureRegister::MainTexture, &std::get<0>(m_moveablesTextures[bucket.Texture]),
				            SamplerStateType::AnisotropicClamp);
				BindTexture(TextureRegister::NormalMapTexture, &std::get<1>(m_moveablesTextures[bucket.Texture]),
				            SamplerStateType::None);

				m_stMisc.AlphaTest = (bucket.BlendMode != BLEND_MODES::BLENDMODE_OPAQUE);
				m_cbMisc.updateData(m_stMisc, m_context.Get());
				m_context->PSSetConstantBuffers(3, 1, m_cbMisc.get());

				m_context->DrawIndexed(bucket.NumIndices, bucket.StartIndex, 0);
			}
		}
	}

	void Renderer11::RenderShadowMap(RenderView& renderView)
	{
		if (shadowLight == nullptr)
			return;

		// Reset GPU state
		m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
		m_context->RSSetState(m_states->CullCounterClockwise());
		m_context->OMSetDepthStencilState(m_states->DepthDefault(), 0);

		// Bind and clear render target
		m_context->ClearRenderTargetView(m_shadowMap.RenderTargetView.Get(), Colors::White);
		m_context->ClearDepthStencilView(m_shadowMap.DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		                                 1.0f, 0);
		m_context->OMSetRenderTargets(1, m_shadowMap.RenderTargetView.GetAddressOf(),
		                              m_shadowMap.DepthStencilView.Get());

		m_context->RSSetViewports(1, &m_shadowMapViewport);

		//DrawLara(false, true);

		Vector3 lightPos = Vector3(shadowLight->Position.x, shadowLight->Position.y, shadowLight->Position.z);
		Vector3 itemPos = Vector3(LaraItem->Position.xPos, LaraItem->Position.yPos, LaraItem->Position.zPos);
		if (lightPos == itemPos)
			return;

		UINT stride = sizeof(RendererVertex);
		UINT offset = 0;

		// Set shaders
		m_context->VSSetShader(m_vsShadowMap.Get(), NULL, 0);
		m_context->PSSetShader(m_psShadowMap.Get(), NULL, 0);

		m_context->IASetVertexBuffers(0, 1, m_moveablesVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetInputLayout(m_inputLayout.Get());
		m_context->IASetIndexBuffer(m_moveablesIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		// Set texture
		BindTexture(TextureRegister::MainTexture, &std::get<0>(m_moveablesTextures[0]),
		            SamplerStateType::AnisotropicClamp);
		BindTexture(TextureRegister::NormalMapTexture, &std::get<1>(m_moveablesTextures[0]), SamplerStateType::None);

		// Set camera matrices
		Matrix view = Matrix::CreateLookAt(lightPos,
		                                   itemPos,
		                                   Vector3(0.0f, -1.0f, 0.0f));
		Matrix projection = Matrix::CreatePerspectiveFieldOfView(90.0f * RADIAN, 1.0f, 64.0f,
		                                                         (shadowLight->Type == LIGHT_TYPE_POINT
			                                                          ? shadowLight->Out
			                                                          : shadowLight->Range) * 1.2f);
		CCameraMatrixBuffer shadowProjection;
		shadowProjection.ViewProjection = view * projection;
		m_cbCameraMatrices.updateData(shadowProjection, m_context.Get());
		m_context->VSSetConstantBuffers(0, 1, m_cbCameraMatrices.get());

		m_stShadowMap.LightViewProjection = (view * projection);

		m_stMisc.AlphaTest = true;
		m_cbMisc.updateData(m_stMisc, m_context.Get());
		m_context->PSSetConstantBuffers(3, 1, m_cbMisc.get());

		RendererObject& laraObj = *m_moveableObjects[ID_LARA];
		RendererObject& laraSkin = *m_moveableObjects[ID_LARA_SKIN];
		RendererRoom& room = m_rooms[LaraItem->RoomNumber];

		m_stItem.World = m_LaraWorldMatrix;
		m_stItem.Position = Vector4(LaraItem->Position.xPos, LaraItem->Position.yPos, LaraItem->Position.zPos, 1.0f);
		m_stItem.AmbientLight = room.AmbientLight;
		memcpy(m_stItem.BonesMatrices, laraObj.AnimationTransforms.data(), sizeof(Matrix) * 32);
		m_cbItem.updateData(m_stItem, m_context.Get());
		m_context->VSSetConstantBuffers(1, 1, m_cbItem.get());
		m_context->PSSetConstantBuffers(1, 1, m_cbItem.get());

		for (int k = 0; k < laraSkin.ObjectMeshes.size(); k++)
		{
			RendererMesh* mesh = GetMesh(Lara.MeshPtrs[k]);

			for (auto& bucket : mesh->buckets)
			{
				if (bucket.NumVertices == 0 && bucket.BlendMode != 0)
					continue;

				// Draw vertices
				m_context->DrawIndexed(bucket.NumIndices, bucket.StartIndex, 0);
				m_numDrawCalls++;
			}
		}

		if (m_moveableObjects[ID_LARA_SKIN_JOINTS].has_value())
		{
			RendererObject& laraSkinJoints = *m_moveableObjects[ID_LARA_SKIN_JOINTS];

			for (int k = 0; k < laraSkinJoints.ObjectMeshes.size(); k++)
			{
				RendererMesh* mesh = laraSkinJoints.ObjectMeshes[k];

				for (auto& bucket : mesh->buckets)
				{
					if (bucket.NumVertices == 0 && bucket.BlendMode != 0)
						continue;

					// Draw vertices
					m_context->DrawIndexed(bucket.NumIndices, bucket.StartIndex, 0);
					m_numDrawCalls++;
				}
			}
		}

		for (int k = 0; k < laraSkin.ObjectMeshes.size(); k++)
		{
			RendererMesh* mesh = laraSkin.ObjectMeshes[k];

			for (auto& bucket : mesh->buckets)
			{
				if (bucket.NumVertices == 0 && bucket.BlendMode != 0)
					continue;

				// Draw vertices
				m_context->DrawIndexed(bucket.NumIndices, bucket.StartIndex, 0);
				m_numDrawCalls++;
			}
		}

		// Draw items

		RendererObject& hairsObj = *m_moveableObjects[ID_LARA_HAIR];

		// First matrix is Lara's head matrix, then all 6 hairs matrices. Bones are adjusted at load time for accounting this.
		m_stItem.World = Matrix::Identity;
		Matrix matrices[7];
		matrices[0] = laraObj.AnimationTransforms[LM_HEAD] * m_LaraWorldMatrix;
		for (int i = 0; i < hairsObj.BindPoseTransforms.size(); i++)
		{
			HAIR_STRUCT* hairs = &Hairs[0][i];
			Matrix world = Matrix::CreateFromYawPitchRoll(TO_RAD(hairs->pos.yRot), TO_RAD(hairs->pos.xRot), 0) *
				Matrix::CreateTranslation(hairs->pos.xPos, hairs->pos.yPos, hairs->pos.zPos);
			matrices[i + 1] = world;
		}
		memcpy(m_stItem.BonesMatrices, matrices, sizeof(Matrix) * 7);
		m_cbItem.updateData(m_stItem, m_context.Get());
		m_context->VSSetConstantBuffers(1, 1, m_cbItem.get());
		m_context->PSSetConstantBuffers(1, 1, m_cbItem.get());

		for (int k = 0; k < hairsObj.ObjectMeshes.size(); k++)
		{
			RendererMesh* mesh = hairsObj.ObjectMeshes[k];

			for (auto& bucket : mesh->buckets)
			{
				if (bucket.NumVertices == 0 && bucket.BlendMode != 0)
					continue;

				// Draw vertices
				m_context->DrawIndexed(bucket.NumIndices, bucket.StartIndex, 0);
				m_numDrawCalls++;
			}
		}
	}

	void Renderer11::renderTitleImage()
	{
		wchar_t introFileChars[255];

		std::mbstowcs(introFileChars, g_GameFlow->IntroImagePath.c_str(), 255);
		std::wstring titleStringFileName(introFileChars);
		Texture2D texture = Texture2D(m_device.Get(), titleStringFileName);

		float currentFade = 0;
		while (currentFade <= 1.0f)
		{
			DrawFullScreenImage(texture.ShaderResourceView.Get(), currentFade, m_backBufferRTV, m_depthStencilView);
			SyncRenderer();
			currentFade += FADE_FACTOR;
			m_swapChain->Present(0, 0);
		}

		for (int i = 0; i < 20; i++)
		{
			DrawFullScreenImage(texture.ShaderResourceView.Get(), 1.0f, m_backBufferRTV, m_depthStencilView);
			SyncRenderer();
			m_swapChain->Present(0, 0);
		}

		currentFade = 1.0f;
		while (currentFade >= 0.0f)
		{
			DrawFullScreenImage(texture.ShaderResourceView.Get(), currentFade, m_backBufferRTV, m_depthStencilView);
			SyncRenderer();
			currentFade -= FADE_FACTOR;
			m_swapChain->Present(0, 0);
		}
	}

	void Renderer11::DrawGunShells(RenderView& view)
	{
		RendererRoom& room = m_rooms[LaraItem->RoomNumber];
		RendererItem* item = &m_items[Lara.ItemNumber];

		m_stItem.AmbientLight = room.AmbientLight;
		memcpy(m_stItem.BonesMatrices, &Matrix::Identity, sizeof(Matrix));

		m_stLights.NumLights = item->LightsToDraw.size();
		for (int j = 0; j < item->LightsToDraw.size(); j++)
			memcpy(&m_stLights.Lights[j], item->LightsToDraw[j], sizeof(ShaderLight));
		m_cbLights.updateData(m_stLights, m_context.Get());
		m_context->PSSetConstantBuffers(2, 1, m_cbLights.get());

		m_stMisc.AlphaTest = true;
		m_cbMisc.updateData(m_stMisc, m_context.Get());
		m_context->PSSetConstantBuffers(3, 1, m_cbMisc.get());

		for (int i = 0; i < 24; i++)
		{
			GUNSHELL_STRUCT* gunshell = &Gunshells[i];

			if (gunshell->counter > 0)
			{
				OBJECT_INFO* obj = &Objects[gunshell->objectNumber];
				RendererObject& moveableObj = *m_moveableObjects[gunshell->objectNumber];

				Matrix translation = Matrix::CreateTranslation(gunshell->pos.xPos, gunshell->pos.yPos,
				                                               gunshell->pos.zPos);
				Matrix rotation = Matrix::CreateFromYawPitchRoll(TO_RAD(gunshell->pos.yRot), TO_RAD(gunshell->pos.xRot),
				                                                 TO_RAD(gunshell->pos.zRot));
				Matrix world = rotation * translation;

				m_stItem.World = world;
				m_cbItem.updateData(m_stItem, m_context.Get());
				m_context->VSSetConstantBuffers(1, 1, m_cbItem.get());

				RendererMesh* mesh = moveableObj.ObjectMeshes[0];

				for (auto& bucket : mesh->buckets)
				{
					if (bucket.NumVertices == 0 && bucket.BlendMode == BLENDMODE_OPAQUE)
						continue;

					// Draw vertices
					m_context->DrawIndexed(bucket.NumIndices, bucket.StartIndex, 0);
					m_numDrawCalls++;
				}
			}
		}
	}

	void Renderer11::RenderTitleMenu()
	{
		char stringBuffer[255];
		int y = 400;
		short lastY;
		RendererVideoAdapter* adapter = &m_adapters[g_Configuration.Adapter];
		RendererDisplayMode* mode = &adapter->DisplayModes[g_Gui.GetCurrentSettings().videoMode];
		__int64 title_option = g_Gui.GetSelectedOption();
		Menu title_menu = g_Gui.GetMenuToDisplay();

		switch (title_menu)
		{
		case Menu::Title:
		a:
			if (title_option == 0)
				drawString(400, y, g_GameFlow->GetString(STRING_NEW_GAME), PRINTSTRING_COLOR_WHITE,
				           PRINTSTRING_CENTER | PRINTSTRING_BLINK);
			else
				drawString(400, y, g_GameFlow->GetString(STRING_NEW_GAME), PRINTSTRING_COLOR_WHITE, PRINTSTRING_CENTER);

			y += 25;

			if (title_option == 1)
				drawString(400, y, g_GameFlow->GetString(STRING_LOAD_GAME), PRINTSTRING_COLOR_WHITE,
				           PRINTSTRING_CENTER | PRINTSTRING_BLINK);
			else
				drawString(400, y, g_GameFlow->GetString(STRING_LOAD_GAME), PRINTSTRING_COLOR_WHITE,
				           PRINTSTRING_CENTER);

			y += 25;

			if (title_option == 2)
				drawString(400, y, "Options", PRINTSTRING_COLOR_WHITE, PRINTSTRING_CENTER | PRINTSTRING_BLINK);
			else
				drawString(400, y, "Options", PRINTSTRING_COLOR_WHITE, PRINTSTRING_CENTER);

			y += 25;

			if (title_option == 3)
				drawString(400, y, g_GameFlow->GetString(STRING_EXIT_GAME), PRINTSTRING_COLOR_WHITE,
				           PRINTSTRING_CENTER | PRINTSTRING_BLINK);
			else
				drawString(400, y, g_GameFlow->GetString(STRING_EXIT_GAME), PRINTSTRING_COLOR_WHITE,
				           PRINTSTRING_CENTER);
			break;

		case Menu::SelectLevel:
			lastY = 50;

			drawString(400, 26, g_GameFlow->GetString(STRING_SELECT_LEVEL), PRINTSTRING_COLOR_WHITE,
			           PRINTSTRING_CENTER);

			for (int i = 1; i < g_GameFlow->GetNumLevels(); i++)
			{
				int i2 = i - 1;
				GameScriptLevel* levelScript = g_GameFlow->GetLevel(i);

				drawString(400, lastY, g_GameFlow->GetString(levelScript->NameStringKey.c_str()),
				           D3DCOLOR_ARGB(255, 255, 255, 255),
				           PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | (title_option == (i - 1)
					                                                       ? PRINTSTRING_BLINK
					                                                       : 0));

				lastY += 24;
			}
			break;

		case Menu::LoadGame:
			if (g_GameFlow->EnableLoadSave)
			{
				y = 44;
				LoadSavegameInfos();

				for (int n = 1; n < SAVEGAME_MAX + 1; n++)
				{
					if (!SavegameInfos[n - 1].Present)
						drawString(400, y, g_GameFlow->GetString(STRING_UNUSED), D3DCOLOR_ARGB(255, 255, 255, 255),
						           PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | (title_option == (n - 1)
							                                                       ? PRINTSTRING_BLINK
							                                                       : 0));
					else
					{
						sprintf(stringBuffer, "%05d", SavegameInfos[n - 1].Count);
						drawString(200, y, stringBuffer, D3DCOLOR_ARGB(255, 255, 255, 255),
						           PRINTSTRING_OUTLINE | (title_option == (n - 1)
							                                  ? PRINTSTRING_BLINK | PRINTSTRING_DONT_UPDATE_BLINK
							                                  : 0));

						drawString(250, y, (char*)SavegameInfos[n - 1].LevelName.c_str(),
						           D3DCOLOR_ARGB(255, 255, 255, 255),
						           PRINTSTRING_OUTLINE | (title_option == (n - 1)
							                                  ? PRINTSTRING_BLINK | PRINTSTRING_DONT_UPDATE_BLINK
							                                  : 0));

						sprintf(stringBuffer, g_GameFlow->GetString(STRING_SAVEGAME_TIMESTAMP),
						        SavegameInfos[n - 1].Days, SavegameInfos[n - 1].Hours, SavegameInfos[n - 1].Minutes,
						        SavegameInfos[n - 1].Seconds);
						drawString(475, y, stringBuffer, D3DCOLOR_ARGB(255, 255, 255, 255),
						           PRINTSTRING_OUTLINE | (title_option == (n - 1) ? PRINTSTRING_BLINK : 0));
					}

					y += 24;
				}
			}
			else
				goto a;
			break;

		case Menu::Options:
			y = 350;

			if (title_option == 0)
				drawString(400, y, g_GameFlow->GetString(STRING_DISPLAY), PRINTSTRING_COLOR_WHITE,
				           PRINTSTRING_CENTER | PRINTSTRING_BLINK);
			else
				drawString(400, y, g_GameFlow->GetString(STRING_DISPLAY), PRINTSTRING_COLOR_WHITE, PRINTSTRING_CENTER);

			y += 25;

			if (title_option == 1)
				drawString(400, y, g_GameFlow->GetString(STRING_CONTROLS), PRINTSTRING_COLOR_WHITE,
				           PRINTSTRING_CENTER | PRINTSTRING_BLINK);
			else
				drawString(400, y, g_GameFlow->GetString(STRING_CONTROLS), PRINTSTRING_COLOR_WHITE, PRINTSTRING_CENTER);

			y += 25;

			if (title_option == 2)
				drawString(400, y, g_GameFlow->GetString(STRING_SOUND), PRINTSTRING_COLOR_WHITE,
				           PRINTSTRING_CENTER | PRINTSTRING_BLINK);
			else
				drawString(400, y, g_GameFlow->GetString(STRING_SOUND), PRINTSTRING_COLOR_WHITE, PRINTSTRING_CENTER);

			break;

		case Menu::Display:
			y = 200;

			drawString(400, y, g_GameFlow->GetString(STRING_DISPLAY),
			           PRINTSTRING_COLOR_YELLOW, PRINTSTRING_OUTLINE | PRINTSTRING_CENTER);

			y += 25;

		// Screen resolution
			drawString(200, y, g_GameFlow->GetString(STRING_SCREEN_RESOLUTION),
			           PRINTSTRING_COLOR_ORANGE,
			           PRINTSTRING_DONT_UPDATE_BLINK | PRINTSTRING_OUTLINE | (
				           title_option == 0 ? PRINTSTRING_BLINK : 0));

			ZeroMemory(stringBuffer, 255);
			sprintf(stringBuffer, "%d x %d (%d Hz)", mode->Width, mode->Height, mode->RefreshRate);

			drawString(400, y, stringBuffer, PRINTSTRING_COLOR_WHITE,
			           PRINTSTRING_OUTLINE | (title_option == 0 ? PRINTSTRING_BLINK : 0));

			y += 25;

		// Windowed mode
			drawString(200, y, g_GameFlow->GetString(STRING_WINDOWED),
			           PRINTSTRING_COLOR_ORANGE,
			           PRINTSTRING_DONT_UPDATE_BLINK | PRINTSTRING_OUTLINE | (
				           title_option == 1 ? PRINTSTRING_BLINK : 0));
			drawString(400, y, g_GameFlow->GetString(g_Gui.GetCurrentSettings().conf.Windowed
				                                         ? STRING_ENABLED
				                                         : STRING_DISABLED),
			           PRINTSTRING_COLOR_WHITE,
			           PRINTSTRING_OUTLINE | (title_option == 1 ? PRINTSTRING_BLINK : 0));

			y += 25;

		// Enable dynamic shadows
			drawString(200, y, g_GameFlow->GetString(STRING_SHADOWS),
			           PRINTSTRING_COLOR_ORANGE,
			           PRINTSTRING_DONT_UPDATE_BLINK | PRINTSTRING_OUTLINE | (
				           title_option == 2 ? PRINTSTRING_BLINK : 0));
			drawString(400, y, g_GameFlow->GetString(g_Gui.GetCurrentSettings().conf.EnableShadows
				                                         ? STRING_ENABLED
				                                         : STRING_DISABLED),
			           PRINTSTRING_COLOR_WHITE,
			           PRINTSTRING_OUTLINE | (title_option == 2 ? PRINTSTRING_BLINK : 0));

			y += 25;

		// Enable caustics
			drawString(200, y, g_GameFlow->GetString(STRING_CAUSTICS),
			           PRINTSTRING_COLOR_ORANGE,
			           PRINTSTRING_DONT_UPDATE_BLINK | PRINTSTRING_OUTLINE | (
				           title_option == 3 ? PRINTSTRING_BLINK : 0));
			drawString(400, y, g_GameFlow->GetString(g_Gui.GetCurrentSettings().conf.EnableCaustics
				                                         ? STRING_ENABLED
				                                         : STRING_DISABLED),
			           PRINTSTRING_COLOR_WHITE,
			           PRINTSTRING_OUTLINE | (title_option == 3 ? PRINTSTRING_BLINK : 0));

			y += 25;

		// Enable volumetric fog
			drawString(200, y, g_GameFlow->GetString(STRING_VOLUMETRIC_FOG),
			           PRINTSTRING_COLOR_ORANGE,
			           PRINTSTRING_DONT_UPDATE_BLINK | PRINTSTRING_OUTLINE | (
				           title_option == 4 ? PRINTSTRING_BLINK : 0));
			drawString(400, y, g_GameFlow->GetString(g_Gui.GetCurrentSettings().conf.EnableVolumetricFog
				                                         ? STRING_ENABLED
				                                         : STRING_DISABLED),
			           PRINTSTRING_COLOR_WHITE,
			           PRINTSTRING_OUTLINE | (title_option == 4 ? PRINTSTRING_BLINK : 0));

			y += 25;

		// Apply
			drawString(400, y, g_GameFlow->GetString(STRING_APPLY),
			           PRINTSTRING_COLOR_ORANGE,
			           PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | (title_option == 5 ? PRINTSTRING_BLINK : 0));

			y += 25;

		//cancel
			drawString(400, y, g_GameFlow->GetString(STRING_CANCEL),
			           PRINTSTRING_COLOR_ORANGE,
			           PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | (title_option == 6 ? PRINTSTRING_BLINK : 0));
			break;

		case Menu::Controls:
			y = 40;

			drawString(400, y, g_GameFlow->GetString(STRING_CONTROLS),
			           PRINTSTRING_COLOR_YELLOW, PRINTSTRING_OUTLINE | PRINTSTRING_CENTER);

			y += 25;

			for (int k = 0; k < 16; k++)
			{
				drawString(200, y, g_GameFlow->GetString(controlmsgs[k]),
				           PRINTSTRING_COLOR_WHITE,
				           PRINTSTRING_OUTLINE | (title_option == k ? PRINTSTRING_BLINK : 0) |
				           (g_Gui.GetCurrentSettings().waitingForkey ? PRINTSTRING_DONT_UPDATE_BLINK : 0));

				if (g_Gui.GetCurrentSettings().waitingForkey && title_option == k)
				{
					drawString(400, y, g_GameFlow->GetString(STRING_WAITING_FOR_KEY),
					           PRINTSTRING_COLOR_YELLOW,
					           PRINTSTRING_OUTLINE | PRINTSTRING_BLINK);
				}
				else
				{
					drawString(400, y, (char*)g_KeyNames[KeyboardLayout[1][k]],
					           PRINTSTRING_COLOR_ORANGE,
					           PRINTSTRING_OUTLINE);
				}

				y += 25;
			}

		// Apply and cancel
			drawString(400, y, g_GameFlow->GetString(STRING_APPLY),
			           PRINTSTRING_COLOR_ORANGE,
			           PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | (title_option == 16 ? PRINTSTRING_BLINK : 0));

			y += 25;

			drawString(400, y, g_GameFlow->GetString(STRING_CANCEL),
			           PRINTSTRING_COLOR_ORANGE,
			           PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | (title_option == 17 ? PRINTSTRING_BLINK : 0));
			break;

		case Menu::Sound:
			y = 200;

			drawString(400, y, g_GameFlow->GetString(STRING_SOUND),
			           PRINTSTRING_COLOR_YELLOW, PRINTSTRING_OUTLINE | PRINTSTRING_CENTER);

			y += 25;

		// Enable sound special effects
			drawString(200, y, g_GameFlow->GetString(STRING_SPECIAL_SOUND_FX),
			           PRINTSTRING_COLOR_ORANGE,
			           PRINTSTRING_DONT_UPDATE_BLINK | PRINTSTRING_OUTLINE | (
				           title_option == 0 ? PRINTSTRING_BLINK : 0));
			drawString(400, y, g_GameFlow->GetString(g_Gui.GetCurrentSettings().conf.EnableAudioSpecialEffects
				                                         ? STRING_ENABLED
				                                         : STRING_DISABLED),
			           PRINTSTRING_COLOR_WHITE,
			           PRINTSTRING_OUTLINE | (title_option == 0 ? PRINTSTRING_BLINK : 0));

			y += 25;

		// Music volume
			drawString(200, y, g_GameFlow->GetString(STRING_MUSIC_VOLUME),
			           PRINTSTRING_COLOR_ORANGE,
			           PRINTSTRING_OUTLINE | (title_option == 1 ? PRINTSTRING_BLINK : 0));
			drawBar(g_Gui.GetCurrentSettings().conf.MusicVolume / 100.0f, g_MusicVolumeBar, ID_SFX_BAR_TEXTURE, 0,
			        false);

			y += 25;

		// Sound FX volume
			drawString(200, y, g_GameFlow->GetString(STRING_SFX_VOLUME),
			           PRINTSTRING_COLOR_ORANGE,
			           PRINTSTRING_OUTLINE | (title_option == 2 ? PRINTSTRING_BLINK : 0));
			drawBar(g_Gui.GetCurrentSettings().conf.SfxVolume / 100.0f, g_SFXVolumeBar, ID_SFX_BAR_TEXTURE, 0, false);
			y += 25;

		// Apply and cancel
			drawString(400, y, g_GameFlow->GetString(STRING_APPLY),
			           PRINTSTRING_COLOR_ORANGE,
			           PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | (title_option == 3 ? PRINTSTRING_BLINK : 0));

			y += 25;

			drawString(400, y, g_GameFlow->GetString(STRING_CANCEL),
			           PRINTSTRING_COLOR_ORANGE,
			           PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | (title_option == 4 ? PRINTSTRING_BLINK : 0));
			break;
		}
	}

	void Renderer11::RenderPauseMenu()
	{
		char stringBuffer[255];
		int y;
		RendererVideoAdapter* adapter = &m_adapters[g_Configuration.Adapter];
		RendererDisplayMode* mode = &adapter->DisplayModes[g_Gui.GetCurrentSettings().videoMode];
		Menu pause_menu = g_Gui.GetMenuToDisplay();
		__int64 pause_option = g_Gui.GetSelectedOption();

		switch (pause_menu)
		{
		case Menu::Pause:
			y = 275;

			if (pause_option == 0)
				drawString(400, y, "Statistics", PRINTSTRING_COLOR_WHITE, PRINTSTRING_CENTER | PRINTSTRING_BLINK);
			else
				drawString(400, y, "Statistics", PRINTSTRING_COLOR_WHITE, PRINTSTRING_CENTER);

			y += 25;

			if (pause_option == 1)
				drawString(400, y, "Options", PRINTSTRING_COLOR_WHITE, PRINTSTRING_CENTER | PRINTSTRING_BLINK);
			else
				drawString(400, y, "Options", PRINTSTRING_COLOR_WHITE, PRINTSTRING_CENTER);

			y += 25;

			if (pause_option == 2)
				drawString(400, y, g_GameFlow->GetString(STRING_EXIT_TO_TITLE), PRINTSTRING_COLOR_WHITE,
				           PRINTSTRING_CENTER | PRINTSTRING_BLINK);
			else
				drawString(400, y, g_GameFlow->GetString(STRING_EXIT_TO_TITLE), PRINTSTRING_COLOR_WHITE,
				           PRINTSTRING_CENTER);

			break;

		case Menu::Statistics:
			DrawStatistics();
			break;

		case Menu::Options:
			y = 275;

			if (pause_option == 0)
				drawString(400, y, g_GameFlow->GetString(STRING_DISPLAY), PRINTSTRING_COLOR_WHITE,
				           PRINTSTRING_CENTER | PRINTSTRING_BLINK);
			else
				drawString(400, y, g_GameFlow->GetString(STRING_DISPLAY), PRINTSTRING_COLOR_WHITE, PRINTSTRING_CENTER);

			y += 25;

			if (pause_option == 1)
				drawString(400, y, g_GameFlow->GetString(STRING_CONTROLS), PRINTSTRING_COLOR_WHITE,
				           PRINTSTRING_CENTER | PRINTSTRING_BLINK);
			else
				drawString(400, y, g_GameFlow->GetString(STRING_CONTROLS), PRINTSTRING_COLOR_WHITE, PRINTSTRING_CENTER);

			y += 25;

			if (pause_option == 2)
				drawString(400, y, g_GameFlow->GetString(STRING_SOUND), PRINTSTRING_COLOR_WHITE,
				           PRINTSTRING_CENTER | PRINTSTRING_BLINK);
			else
				drawString(400, y, g_GameFlow->GetString(STRING_SOUND), PRINTSTRING_COLOR_WHITE, PRINTSTRING_CENTER);

			break;

		case Menu::Display:
			y = 200;

			drawString(400, y, g_GameFlow->GetString(STRING_DISPLAY),
			           PRINTSTRING_COLOR_YELLOW, PRINTSTRING_OUTLINE | PRINTSTRING_CENTER);

			y += 25;

		// Screen resolution
			drawString(200, y, g_GameFlow->GetString(STRING_SCREEN_RESOLUTION),
			           PRINTSTRING_COLOR_ORANGE,
			           PRINTSTRING_DONT_UPDATE_BLINK | PRINTSTRING_OUTLINE | (
				           pause_option == 0 ? PRINTSTRING_BLINK : 0));

			ZeroMemory(stringBuffer, 255);
			sprintf(stringBuffer, "%d x %d (%d Hz)", mode->Width, mode->Height, mode->RefreshRate);

			drawString(400, y, stringBuffer, PRINTSTRING_COLOR_WHITE,
			           PRINTSTRING_OUTLINE | (pause_option == 0 ? PRINTSTRING_BLINK : 0));

			y += 25;

		// Windowed mode
			drawString(200, y, g_GameFlow->GetString(STRING_WINDOWED),
			           PRINTSTRING_COLOR_ORANGE,
			           PRINTSTRING_DONT_UPDATE_BLINK | PRINTSTRING_OUTLINE | (
				           pause_option == 1 ? PRINTSTRING_BLINK : 0));
			drawString(400, y, g_GameFlow->GetString(g_Gui.GetCurrentSettings().conf.Windowed
				                                         ? STRING_ENABLED
				                                         : STRING_DISABLED),
			           PRINTSTRING_COLOR_WHITE,
			           PRINTSTRING_OUTLINE | (pause_option == 1 ? PRINTSTRING_BLINK : 0));

			y += 25;

		// Enable dynamic shadows
			drawString(200, y, g_GameFlow->GetString(STRING_SHADOWS),
			           PRINTSTRING_COLOR_ORANGE,
			           PRINTSTRING_DONT_UPDATE_BLINK | PRINTSTRING_OUTLINE | (
				           pause_option == 2 ? PRINTSTRING_BLINK : 0));
			drawString(400, y, g_GameFlow->GetString(g_Gui.GetCurrentSettings().conf.EnableShadows
				                                         ? STRING_ENABLED
				                                         : STRING_DISABLED),
			           PRINTSTRING_COLOR_WHITE,
			           PRINTSTRING_OUTLINE | (pause_option == 2 ? PRINTSTRING_BLINK : 0));

			y += 25;

		// Enable caustics
			drawString(200, y, g_GameFlow->GetString(STRING_CAUSTICS),
			           PRINTSTRING_COLOR_ORANGE,
			           PRINTSTRING_DONT_UPDATE_BLINK | PRINTSTRING_OUTLINE | (
				           pause_option == 3 ? PRINTSTRING_BLINK : 0));
			drawString(400, y, g_GameFlow->GetString(g_Gui.GetCurrentSettings().conf.EnableCaustics
				                                         ? STRING_ENABLED
				                                         : STRING_DISABLED),
			           PRINTSTRING_COLOR_WHITE,
			           PRINTSTRING_OUTLINE | (pause_option == 3 ? PRINTSTRING_BLINK : 0));

			y += 25;

		// Enable volumetric fog
			drawString(200, y, g_GameFlow->GetString(STRING_VOLUMETRIC_FOG),
			           PRINTSTRING_COLOR_ORANGE,
			           PRINTSTRING_DONT_UPDATE_BLINK | PRINTSTRING_OUTLINE | (
				           pause_option == 4 ? PRINTSTRING_BLINK : 0));
			drawString(400, y, g_GameFlow->GetString(g_Gui.GetCurrentSettings().conf.EnableVolumetricFog
				                                         ? STRING_ENABLED
				                                         : STRING_DISABLED),
			           PRINTSTRING_COLOR_WHITE,
			           PRINTSTRING_OUTLINE | (pause_option == 4 ? PRINTSTRING_BLINK : 0));

			y += 25;

		// Apply
			drawString(400, y, g_GameFlow->GetString(STRING_APPLY),
			           PRINTSTRING_COLOR_ORANGE,
			           PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | (pause_option == 5 ? PRINTSTRING_BLINK : 0));

			y += 25;

		//cancel
			drawString(400, y, g_GameFlow->GetString(STRING_CANCEL),
			           PRINTSTRING_COLOR_ORANGE,
			           PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | (pause_option == 6 ? PRINTSTRING_BLINK : 0));
			break;

		case Menu::Controls:
			y = 40;

			drawString(400, y, g_GameFlow->GetString(STRING_CONTROLS),
			           PRINTSTRING_COLOR_YELLOW, PRINTSTRING_OUTLINE | PRINTSTRING_CENTER);

			y += 25;

			for (int k = 0; k < 16; k++)
			{
				drawString(200, y, g_GameFlow->GetString(controlmsgs[k]),
				           PRINTSTRING_COLOR_WHITE,
				           PRINTSTRING_OUTLINE | (pause_option == k ? PRINTSTRING_BLINK : 0) |
				           (g_Gui.GetCurrentSettings().waitingForkey ? PRINTSTRING_DONT_UPDATE_BLINK : 0));

				if (g_Gui.GetCurrentSettings().waitingForkey && pause_option == k)
				{
					drawString(400, y, g_GameFlow->GetString(STRING_WAITING_FOR_KEY),
					           PRINTSTRING_COLOR_YELLOW,
					           PRINTSTRING_OUTLINE | PRINTSTRING_BLINK);
				}
				else
				{
					drawString(400, y, (char*)g_KeyNames[KeyboardLayout[1][k]],
					           PRINTSTRING_COLOR_ORANGE,
					           PRINTSTRING_OUTLINE);
				}

				y += 25;
			}

		// Apply and cancel
			drawString(400, y, g_GameFlow->GetString(STRING_APPLY),
			           PRINTSTRING_COLOR_ORANGE,
			           PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | (pause_option == 16 ? PRINTSTRING_BLINK : 0));

			y += 25;

			drawString(400, y, g_GameFlow->GetString(STRING_CANCEL),
			           PRINTSTRING_COLOR_ORANGE,
			           PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | (pause_option == 17 ? PRINTSTRING_BLINK : 0));

			break;

		case Menu::Sound:
			y = 200;

			drawString(400, y, g_GameFlow->GetString(STRING_SOUND),
			           PRINTSTRING_COLOR_YELLOW, PRINTSTRING_OUTLINE | PRINTSTRING_CENTER);

			y += 25;

		// Enable sound special effects
			drawString(200, y, g_GameFlow->GetString(STRING_SPECIAL_SOUND_FX),
			           PRINTSTRING_COLOR_ORANGE,
			           PRINTSTRING_DONT_UPDATE_BLINK | PRINTSTRING_OUTLINE | (
				           pause_option == 0 ? PRINTSTRING_BLINK : 0));
			drawString(400, y, g_GameFlow->GetString(g_Gui.GetCurrentSettings().conf.EnableAudioSpecialEffects
				                                         ? STRING_ENABLED
				                                         : STRING_DISABLED),
			           PRINTSTRING_COLOR_WHITE,
			           PRINTSTRING_OUTLINE | (pause_option == 0 ? PRINTSTRING_BLINK : 0));

			y += 25;

		// Music volume
			drawString(200, y, g_GameFlow->GetString(STRING_MUSIC_VOLUME),
			           PRINTSTRING_COLOR_ORANGE,
			           PRINTSTRING_OUTLINE | (pause_option == 1 ? PRINTSTRING_BLINK : 0));
			drawBar(g_Gui.GetCurrentSettings().conf.MusicVolume / 100.0f, g_MusicVolumeBar, ID_SFX_BAR_TEXTURE, 0, 0);

			y += 25;

		// Sound FX volume
			drawString(200, y, g_GameFlow->GetString(STRING_SFX_VOLUME),
			           PRINTSTRING_COLOR_ORANGE,
			           PRINTSTRING_OUTLINE | (pause_option == 2 ? PRINTSTRING_BLINK : 0));
			drawBar(g_Gui.GetCurrentSettings().conf.SfxVolume / 100.0f, g_SFXVolumeBar, ID_SFX_BAR_TEXTURE, 0, 0);
			y += 25;

		// Apply and cancel
			drawString(400, y, g_GameFlow->GetString(STRING_APPLY),
			           PRINTSTRING_COLOR_ORANGE,
			           PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | (pause_option == 3 ? PRINTSTRING_BLINK : 0));

			y += 25;

			drawString(400, y, g_GameFlow->GetString(STRING_CANCEL),
			           PRINTSTRING_COLOR_ORANGE,
			           PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | (pause_option == 4 ? PRINTSTRING_BLINK : 0));
			break;
		}
		DrawLines2D();
		DrawAllStrings();
	}

	void Renderer11::RenderLoadSaveMenu()
	{
		if (!g_GameFlow->EnableLoadSave)
		{
			g_Gui.SetInventoryMode(InventoryMode::InGame);
			return;
		}

		short selection = g_Gui.GetLoadSaveSelection();
		char stringBuffer[255];
		long y = 70;
		LoadSavegameInfos();

		if (g_Gui.GetInventoryMode() == InventoryMode::Load)
			drawString(400, 40, "Load Game", D3DCOLOR_ARGB(255, 255, 140, 0), PRINTSTRING_CENTER | PRINTSTRING_OUTLINE);
		else
			drawString(400, 40, "Save Game", D3DCOLOR_ARGB(255, 255, 140, 0), PRINTSTRING_CENTER | PRINTSTRING_OUTLINE);

		for (int n = 1; n < SAVEGAME_MAX + 1; n++)
		{
			int n2 = n - 1;

			if (!SavegameInfos[n - 1].Present)
				drawString(400, y, g_GameFlow->GetString(STRING_UNUSED), D3DCOLOR_ARGB(255, 255, 255, 255),
				           PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | (selection == n2 ? PRINTSTRING_BLINK : 0));
			else
			{
				sprintf(stringBuffer, "%05d", SavegameInfos[n - 1].Count);
				drawString(200, y, stringBuffer, D3DCOLOR_ARGB(255, 255, 255, 255),
				           PRINTSTRING_OUTLINE | (selection == n2
					                                  ? PRINTSTRING_BLINK | PRINTSTRING_DONT_UPDATE_BLINK
					                                  : 0));

				drawString(250, y, (char*)SavegameInfos[n - 1].LevelName.c_str(), D3DCOLOR_ARGB(255, 255, 255, 255),
				           PRINTSTRING_OUTLINE | (selection == n2
					                                  ? PRINTSTRING_BLINK | PRINTSTRING_DONT_UPDATE_BLINK
					                                  : 0));

				sprintf(stringBuffer, g_GameFlow->GetString(STRING_SAVEGAME_TIMESTAMP), SavegameInfos[n - 1].Days,
				        SavegameInfos[n - 1].Hours, SavegameInfos[n - 1].Minutes, SavegameInfos[n - 1].Seconds);
				drawString(475, y, stringBuffer, D3DCOLOR_ARGB(255, 255, 255, 255),
				           PRINTSTRING_OUTLINE | (selection == n2 ? PRINTSTRING_BLINK : 0));
			}

			y += 30;
		}

		DrawAllStrings();
	}

	void Renderer11::RenderNewInventory()
	{
		g_Gui.DrawCurrentObjectList((int)RingTypes::Inventory);

		g_Gui.HandleInventoryMenu();

		if (g_Gui.GetRings((int)RingTypes::Ammo)->ringactive)
			g_Gui.DrawCurrentObjectList((int)RingTypes::Ammo);

		g_Gui.DrawAmmoSelector();
		g_Gui.FadeAmmoSelector();
		g_Gui.DrawCompass();
		DrawAllStrings();
	}

	void Renderer11::DrawStatistics()
	{
		unsigned short ypos;
		short Days, Hours, Min, Sec;
		char buffer[40];
		int seconds;
		GameScriptLevel* lvl = g_GameFlow->GetLevel(CurrentLevel);

		ypos = 150;
		drawString(400, ypos, STRING_STATISTICS, PRINTSTRING_COLOR_ORANGE, PRINTSTRING_CENTER);
		drawString(400, ypos + 2 * 25, g_GameFlow->GetString(lvl->NameStringKey.c_str()), PRINTSTRING_COLOR_WHITE,
		           PRINTSTRING_CENTER);
		drawString(200, ypos + 3 * 25, g_GameFlow->GetString(STRING_TIME_TAKEN), PRINTSTRING_COLOR_WHITE, 0);
		drawString(200, ypos + 4 * 25, g_GameFlow->GetString(STRING_DISTANCE_TRAVELLED), PRINTSTRING_COLOR_WHITE, 0);
		drawString(200, ypos + 5 * 25, g_GameFlow->GetString(STRING_USED_AMMOS), PRINTSTRING_COLOR_WHITE, 0);
		drawString(200, ypos + 6 * 25, g_GameFlow->GetString(STRING_USED_MEDIPACKS), PRINTSTRING_COLOR_WHITE, 0);
		drawString(200, ypos + 7 * 25, g_GameFlow->GetString(STRING_SECRETS_FOUND), PRINTSTRING_COLOR_WHITE, 0);

		seconds = GameTimer / 30;
		Days = seconds / (24 * 60 * 60);
		Hours = (seconds % (24 * 60 * 60)) / (60 * 60);
		Min = (seconds / 60) % 60;
		Sec = (seconds % 60);

		sprintf(buffer, "%02d:%02d:%02d", (Days * 24) + Hours, Min, Sec);
		drawString(500, ypos + 3 * 25, buffer, PRINTSTRING_COLOR_WHITE, 0);
		sprintf(buffer, "%dm", Statistics.Game.Distance / 419);
		drawString(500, ypos + 4 * 25, buffer, PRINTSTRING_COLOR_WHITE, 0);
		sprintf(buffer, "%d", Statistics.Game.AmmoUsed);
		drawString(500, ypos + 5 * 25, buffer, PRINTSTRING_COLOR_WHITE, 0);
		sprintf(buffer, "%d", Statistics.Game.HealthUsed);
		drawString(500, ypos + 6 * 25, buffer, PRINTSTRING_COLOR_WHITE, 0);
		sprintf(buffer, "%d / 36", Statistics.Game.Secrets);
		drawString(500, ypos + 7 * 25, buffer, PRINTSTRING_COLOR_WHITE, 0);

		DrawAllStrings();
	}

	void Renderer11::DrawExamines()
	{
		static short xrot = 0, yrot = 0, zrot = 0;
		static float scaler = 1.2f;
		float saved_scale;
		short inv_item = g_Gui.GetRings((int)RingTypes::Inventory)->current_object_list[g_Gui.GetRings(
			(int)RingTypes::Inventory)->curobjinlist].invitem;
		InventoryObject* obj = &inventry_objects_list[inv_item];

		if (TrInput & IN_LEFT)
			yrot += ANGLE(3);

		if (TrInput & IN_RIGHT)
			yrot -= ANGLE(3);

		if (TrInput & IN_FORWARD)
			xrot += ANGLE(3);

		if (TrInput & IN_BACK)
			xrot -= ANGLE(3);

		if (TrInput & IN_SPRINT)
			scaler += 0.03f;

        if (TrInput & IN_CROUCH)
            scaler -= 0.03f;

		if (scaler > 1.6f)
			scaler = 1.6f;

		if (scaler < 0.8f)
			scaler = 0.8f;

		saved_scale = obj->scale1;
		obj->scale1 = scaler;
		drawObjectOn2DPosition(400, 300, g_Gui.ConvertInventoryItemToObject(inv_item), xrot, yrot, zrot, obj->scale1);
		obj->scale1 = saved_scale;
	}

	void Renderer11::DrawDiary()
	{
		InventoryObject* obj = &inventry_objects_list[INV_OBJECT_OPEN_DIARY];
		short currentPage = Lara.Diary.currentPage;
		drawObjectOn2DPosition(400, 300, g_Gui.ConvertInventoryItemToObject(INV_OBJECT_OPEN_DIARY), obj->xrot,
		                       obj->yrot, obj->zrot, obj->scale1);

		for (int i = 0; i < MaxStringsPerPage; i++)
		{
			if (!Lara.Diary.Pages[Lara.Diary.currentPage].Strings[i].x && !Lara.Diary.Pages[Lara.Diary.currentPage].
				Strings[i].y && !Lara.Diary.Pages[Lara.Diary.currentPage].Strings[i].stringID)
				break;

			//drawString(Lara.Diary.Pages[currentPage].Strings[i].x, Lara.Diary.Pages[currentPage].Strings[i].y, g_GameFlow->GetString(Lara.Diary.Pages[currentPage].Strings[i].stringID), PRINTSTRING_COLOR_WHITE, 0);
		}

		DrawAllStrings();
	}

	void Renderer11::RenderInventoryScene(ID3D11RenderTargetView* target, ID3D11DepthStencilView* depthTarget,
	                                      ID3D11ShaderResourceView* background)
	{
		char stringBuffer[255];

		bool drawLogo = true;

		RECT guiRect;
		Vector4 guiColor = Vector4(0.0f, 0.0f, 0.25f, 0.5f);
		bool drawGuiRect = false;

		RECT rect;
		rect.left = 0;
		rect.top = 0;
		rect.right = ScreenWidth;
		rect.bottom = ScreenHeight;

		m_lines2DToDraw.clear();
		m_strings.clear();


		// Set basic render states
		m_context->OMSetDepthStencilState(m_states->DepthDefault(), 0);
		m_context->RSSetState(m_states->CullCounterClockwise());
		m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);

		// Bind and clear render target
		//m_context->ClearRenderTargetView(target, Colors::Black);
		//m_context->ClearDepthStencilView(depthTarget, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		m_context->OMSetRenderTargets(1, &target, depthTarget);
		m_context->RSSetViewports(1, &m_viewport);

		if (background != nullptr)
		{
			DrawFullScreenImage(background, 0.5f, target, depthTarget);
		}

		m_context->ClearDepthStencilView(depthTarget, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		UINT stride = sizeof(RendererVertex);
		UINT offset = 0;

		// Set vertex buffer
		m_context->IASetVertexBuffers(0, 1, m_moveablesVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetInputLayout(m_inputLayout.Get());
		m_context->IASetIndexBuffer(m_moveablesIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		// Set shaders
		m_context->VSSetShader(m_vsInventory.Get(), NULL, 0);
		m_context->PSSetShader(m_psInventory.Get(), NULL, 0);

		// Set texture
		BindTexture(TextureRegister::MainTexture, &std::get<0>(m_moveablesTextures[0]),
		            SamplerStateType::AnisotropicClamp);
		BindTexture(TextureRegister::NormalMapTexture, &std::get<1>(m_moveablesTextures[0]), SamplerStateType::None);

		if (CurrentLevel == 0)
		{
			Menu title_menu = g_Gui.GetMenuToDisplay();

			if (title_menu == Menu::Title || title_menu == Menu::Options)
				drawLogo = 1;
			else
				drawLogo = 0;

			if (drawLogo)
			{
				float factorX = (float)ScreenWidth / REFERENCE_RES_WIDTH;
				float factorY = (float)ScreenHeight / REFERENCE_RES_HEIGHT;

				RECT rect;
				rect.left = 250 * factorX;
				rect.right = 550 * factorX;
				rect.top = 50 * factorY;
				rect.bottom = 200 * factorY;

				m_spriteBatch->Begin(SpriteSortMode_BackToFront, m_states->Additive());
				m_spriteBatch->Draw(m_logo.ShaderResourceView.Get(), rect, Vector4::One);
				m_spriteBatch->End();
			}

			RenderTitleMenu();
			return;
		}

		InventoryMode mode = g_Gui.GetInventoryMode();

		if (mode == InventoryMode::Load || mode == InventoryMode::Save)
		{
			RenderLoadSaveMenu();
			return;
		}

		if (mode == InventoryMode::InGame)
		{
			RenderNewInventory();
			return;
		}

		if (mode == InventoryMode::Statistics)
		{
			DrawStatistics();
			return;
		}

		if (mode == InventoryMode::Examine)
		{
			DrawExamines();
			return;
		}

		if (mode == InventoryMode::Pause)
		{
			Menu pause_menu = g_Gui.GetMenuToDisplay();
			RenderPauseMenu();
			return;
		}

		if (mode == InventoryMode::Diary)
		{
			DrawDiary();
			return;
		}
	}

	void Renderer11::DrawFullScreenQuad(ID3D11ShaderResourceView* texture, DirectX::SimpleMath::Vector3 color,
	                                    bool cinematicBars)
	{
		RendererVertex vertices[4];

		if (!cinematicBars)
		{
			vertices[0].Position.x = -1.0f;
			vertices[0].Position.y = 1.0f;
			vertices[0].Position.z = 0.0f;
			vertices[0].UV.x = 0.0f;
			vertices[0].UV.y = 0.0f;
			vertices[0].Color = Vector4(color.x, color.y, color.z, 1.0f);

			vertices[1].Position.x = 1.0f;
			vertices[1].Position.y = 1.0f;
			vertices[1].Position.z = 0.0f;
			vertices[1].UV.x = 1.0f;
			vertices[1].UV.y = 0.0f;
			vertices[1].Color = Vector4(color.x, color.y, color.z, 1.0f);

			vertices[2].Position.x = 1.0f;
			vertices[2].Position.y = -1.0f;
			vertices[2].Position.z = 0.0f;
			vertices[2].UV.x = 1.0f;
			vertices[2].UV.y = 1.0f;
			vertices[2].Color = Vector4(color.x, color.y, color.z, 1.0f);

			vertices[3].Position.x = -1.0f;
			vertices[3].Position.y = -1.0f;
			vertices[3].Position.z = 0.0f;
			vertices[3].UV.x = 0.0f;
			vertices[3].UV.y = 1.0f;
			vertices[3].Color = Vector4(color.x, color.y, color.z, 1.0f);
		}
		else
		{
			float cinematicFactor = 0.12f;

			vertices[0].Position.x = -1.0f;
			vertices[0].Position.y = 1.0f - cinematicFactor * 2;
			vertices[0].Position.z = 0.0f;
			vertices[0].UV.x = 0.0f;
			vertices[0].UV.y = cinematicFactor;
			vertices[0].Color = Vector4(color.x, color.y, color.z, 1.0f);

			vertices[1].Position.x = 1.0f;
			vertices[1].Position.y = 1.0f - cinematicFactor * 2;
			vertices[1].Position.z = 0.0f;
			vertices[1].UV.x = 1.0f;
			vertices[1].UV.y = cinematicFactor;
			vertices[1].Color = Vector4(color.x, color.y, color.z, 1.0f);

			vertices[2].Position.x = 1.0f;
			vertices[2].Position.y = -(1.0f - cinematicFactor * 2);
			vertices[2].Position.z = 0.0f;
			vertices[2].UV.x = 1.0f;
			vertices[2].UV.y = 1.0f - cinematicFactor;
			vertices[2].Color = Vector4(color.x, color.y, color.z, 1.0f);

			vertices[3].Position.x = -1.0f;
			vertices[3].Position.y = -(1.0f - cinematicFactor * 2);
			vertices[3].Position.z = 0.0f;
			vertices[3].UV.x = 0.0f;
			vertices[3].UV.y = 1.0f - cinematicFactor;
			vertices[3].Color = Vector4(color.x, color.y, color.z, 1.0f);
		}

		m_context->VSSetShader(m_vsFullScreenQuad.Get(), NULL, 0);
		m_context->PSSetShader(m_psFullScreenQuad.Get(), NULL, 0);

		m_context->PSSetShaderResources(0, 1, &texture);
		ID3D11SamplerState* sampler = m_states->AnisotropicClamp();
		m_context->PSSetSamplers(0, 1, &sampler);

		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetInputLayout(m_inputLayout.Get());

		m_primitiveBatch->Begin();
		m_primitiveBatch->DrawQuad(vertices[0], vertices[1], vertices[2], vertices[3]);
		m_primitiveBatch->End();
	}

	void Renderer11::DrawRopes(RenderView& view)
	{
		for (int n = 0; n < Ropes.size(); n++)
		{
			ROPE_STRUCT* rope = &Ropes[n];

			if (rope->active)
			{
				Matrix world = Matrix::CreateTranslation(
					rope->position.x,
					rope->position.y,
					rope->position.z
				);

				Vector3 absolute[24];

				for (int n = 0; n < ROPE_SEGMENTS; n++)
				{
					PHD_VECTOR* s = &rope->meshSegment[n];
					Vector3 t;
					Vector3 output;

					t.x = s->x >> FP_SHIFT;
					t.y = s->y >> FP_SHIFT;
					t.z = s->z >> FP_SHIFT;

					output = Vector3::Transform(t, world);

					Vector3 absolutePosition = Vector3(output.x, output.y, output.z);
					absolute[n] = absolutePosition;
				}

				for (int n = 0; n < ROPE_SEGMENTS - 1; n++)
				{
					addLine3D(absolute[n], absolute[n + 1], Vector4::One);
				}
			}
		}
	}

	void Renderer11::DrawLines2D()
	{
		m_context->RSSetState(m_states->CullNone());
		m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
		m_context->OMSetDepthStencilState(m_states->DepthRead(), 0);

		m_context->VSSetShader(m_vsSolid.Get(), NULL, 0);
		m_context->PSSetShader(m_psSolid.Get(), NULL, 0);
		Matrix world = Matrix::CreateOrthographicOffCenter(0, ScreenWidth, ScreenHeight, 0, m_viewport.MinDepth,
		                                                   m_viewport.MaxDepth);

		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		m_context->IASetInputLayout(m_inputLayout.Get());

		m_primitiveBatch->Begin();

		for (int i = 0; i < m_lines2DToDraw.size(); i++)
		{
			RendererLine2D* line = &m_lines2DToDraw[i];

			RendererVertex v1;
			v1.Position.x = line->Vertices[0].x;
			v1.Position.y = line->Vertices[0].y;
			v1.Position.z = 1.0f;
			v1.Color.x = line->Color.x / 255.0f;
			v1.Color.y = line->Color.y / 255.0f;
			v1.Color.z = line->Color.z / 255.0f;
			v1.Color.w = line->Color.w / 255.0f;

			RendererVertex v2;
			v2.Position.x = line->Vertices[1].x;
			v2.Position.y = line->Vertices[1].y;
			v2.Position.z = 1.0f;
			v2.Color.x = line->Color.x / 255.0f;
			v2.Color.y = line->Color.y / 255.0f;
			v2.Color.z = line->Color.z / 255.0f;
			v2.Color.w = line->Color.w / 255.0f;

			v1.Position = Vector3::Transform(v1.Position, world);
			v2.Position = Vector3::Transform(v2.Position, world);

			v1.Position.z = 0.5f;
			v2.Position.z = 0.5f;

			m_primitiveBatch->DrawLine(v1, v2);
		}

		m_primitiveBatch->End();

		m_context->RSSetState(m_states->CullCounterClockwise());
		m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
		m_context->OMSetDepthStencilState(m_states->DepthDefault(), 0);
	}

	void Renderer11::DrawSpiders(RenderView& view)
	{
		/*XMMATRIX world;
		UINT cPasses = 1;

		if (Objects[ID_SPIDERS_EMITTER].loaded)
		{
		    OBJECT_INFO* obj = &Objects[ID_SPIDERS_EMITTER];
		    RendererObject* moveableObj = m_moveableObjects[ID_SPIDERS_EMITTER].get();
		    short* meshPtr = Meshes[Objects[ID_SPIDERS_EMITTER].meshIndex + ((Wibble >> 2) & 2)];
		    RendererMesh* mesh = m_meshPointersToMesh[meshPtr];
		    RendererBucket* bucket = mesh->GetBucket(bucketIndex);

		    if (bucket->NumVertices == 0)
		        return true;

		    setGpuStateForBucket(bucketIndex);

		    m_device->SetStreamSource(0, bucket->GetVertexBuffer(), 0, sizeof(RendererVertex));
		    m_device->SetIndices(bucket->GetIndexBuffer());

		    LPD3DXEFFECT effect;
		    if (pass == RENDERER_PASS_SHADOW_MAP)
		        effect = m_shaderDepth->GetEffect();
		    else if (pass == RENDERER_PASS_RECONSTRUCT_DEPTH)
		        effect = m_shaderReconstructZBuffer->GetEffect();
		    else if (pass == RENDERER_PASS_GBUFFER)
		        effect = m_shaderFillGBuffer->GetEffect();
		    else
		        effect = m_shaderTransparent->GetEffect();

		    effect->SetBool(effect->GetParameterByName(NULL, "UseSkinning"), false);
		    effect->SetInt(effect->GetParameterByName(NULL, "ModelType"), MODEL_TYPE_MOVEABLE);

		    if (bucketIndex == RENDERER_BUCKET_SOLID || bucketIndex == RENDERER_BUCKET_SOLID_DS)
		        effect->SetInt(effect->GetParameterByName(NULL, "BlendMode"), BLENDMODE_OPAQUE);
		    else
		        effect->SetInt(effect->GetParameterByName(NULL, "BlendMode"), BLENDMODE_ALPHATEST);

		    for (int i = 0; i < NUM_SPIDERS; i++)
		    {
		        SPIDER_STRUCT* spider = &Spiders[i];

		        if (spider->on)
		        {
		            XMMATRIXTranslation(&m_tempTranslation, spider->pos.xPos, spider->pos.yPos, spider->pos.zPos);
		            XMMATRIXRotationYawPitchRoll(&m_tempRotation, spider->pos.yRot, spider->pos.xRot, spider->pos.zRot);
		            XMMATRIXMultiply(&m_tempWorld, &m_tempRotation, &m_tempTranslation);
		            effect->SetMatrix(effect->GetParameterByName(NULL, "World"), &m_tempWorld);

		            effect->SetVector(effect->GetParameterByName(NULL, "AmbientLight"), &m_rooms[spider->roomNumber]->AmbientLight);

		            for (int iPass = 0; iPass < cPasses; iPass++)
		            {
		                effect->BeginPass(iPass);
		                effect->CommitChanges();

		                drawPrimitives(D3DPT_TRIANGLELIST, 0, 0, bucket->NumVertices, 0, bucket->Indices.size() / 3);

		                effect->EndPass();
		            }
		        }
		    }
		}*/
	}

	void Renderer11::DrawRats(RenderView& view)
	{
		UINT stride = sizeof(RendererVertex);
		UINT offset = 0;

		m_context->IASetVertexBuffers(0, 1, m_moveablesVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetInputLayout(m_inputLayout.Get());
		m_context->IASetIndexBuffer(m_moveablesIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		if (Objects[ID_RATS_EMITTER].loaded)
		{
			OBJECT_INFO* obj = &Objects[ID_RATS_EMITTER];
			RendererObject& moveableObj = *m_moveableObjects[ID_RATS_EMITTER];

			for (int m = 0; m < 32; m++)
				memcpy(&m_stItem.BonesMatrices[m], &Matrix::Identity, sizeof(Matrix));

			for (int i = 0; i < NUM_RATS; i++)
			{
				RAT_STRUCT* rat = &Rats[i];

				if (rat->on)
				{
					RendererMesh* mesh = GetMesh(Objects[ID_RATS_EMITTER].meshIndex + (rand() % 8));
					Matrix translation = Matrix::CreateTranslation(rat->pos.xPos, rat->pos.yPos, rat->pos.zPos);
					Matrix rotation = Matrix::CreateFromYawPitchRoll(TO_RAD(rat->pos.yRot), TO_RAD(rat->pos.xRot),
					                                                 TO_RAD(rat->pos.zRot));
					Matrix world = rotation * translation;

					m_stItem.World = world;
					m_stItem.Position = Vector4(rat->pos.xPos, rat->pos.yPos, rat->pos.zPos, 1.0f);
					m_stItem.AmbientLight = m_rooms[rat->roomNumber].AmbientLight;
					m_cbItem.updateData(m_stItem, m_context.Get());

					for (int b = 0; b < mesh->buckets.size(); b++)
					{
						RendererBucket* bucket = &mesh->buckets[b];

						if (bucket->Vertices.size() == 0)
							continue;

						m_context->DrawIndexed(bucket->Indices.size(), bucket->StartIndex, 0);
						m_numDrawCalls++;
					}
				}
			}
		}
	}

	void Renderer11::DrawBats(RenderView& view)
	{
		UINT stride = sizeof(RendererVertex);
		UINT offset = 0;

		m_context->IASetVertexBuffers(0, 1, m_moveablesVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetInputLayout(m_inputLayout.Get());
		m_context->IASetIndexBuffer(m_moveablesIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		if (Objects[ID_BATS_EMITTER].loaded)
		{
			OBJECT_INFO* obj = &Objects[ID_BATS_EMITTER];
			RendererObject& moveableObj = *m_moveableObjects[ID_BATS_EMITTER];
			RendererMesh* mesh = GetMesh(Objects[ID_BATS_EMITTER].meshIndex + (-GlobalCounter & 3));

			for (int m = 0; m < 32; m++)
				memcpy(&m_stItem.BonesMatrices[m], &Matrix::Identity, sizeof(Matrix));

			for (int b = 0; b < mesh->buckets.size(); b++)
			{
				RendererBucket* bucket = &mesh->buckets[b];

				if (bucket->Vertices.size() == 0)
					continue;

				for (int i = 0; i < NUM_BATS; i++)
				{
					BAT_STRUCT* bat = &Bats[i];

					if (bat->on)
					{
						Matrix translation = Matrix::CreateTranslation(bat->pos.xPos, bat->pos.yPos, bat->pos.zPos);
						Matrix rotation = Matrix::CreateFromYawPitchRoll(
							TO_RAD(bat->pos.yRot), TO_RAD(bat->pos.xRot), TO_RAD(bat->pos.zRot));
						Matrix world = rotation * translation;

						m_stItem.World = world;
						m_stItem.Position = Vector4(bat->pos.xPos, bat->pos.yPos, bat->pos.zPos, 1.0f);
						m_stItem.AmbientLight = m_rooms[bat->roomNumber].AmbientLight;
						m_cbItem.updateData(m_stItem, m_context.Get());

						m_context->DrawIndexed(bucket->Indices.size(), bucket->StartIndex, 0);
						m_numDrawCalls++;
					}
				}
			}
		}
	}

	void Renderer11::DrawScarabs(RenderView& view)
	{
		UINT stride = sizeof(RendererVertex);
		UINT offset = 0;

		m_context->IASetVertexBuffers(0, 1, m_moveablesVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetInputLayout(m_inputLayout.Get());
		m_context->IASetIndexBuffer(m_moveablesIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		if (Objects[ID_LITTLE_BEETLE].loaded)
		{
			OBJECT_INFO* obj = &Objects[ID_LITTLE_BEETLE];
			RendererObject& moveableObj = *m_moveableObjects[ID_LITTLE_BEETLE];

			for (int m = 0; m < 32; m++)
				memcpy(&m_stItem.BonesMatrices[m], &Matrix::Identity, sizeof(Matrix));

			for (int i = 0; i < TEN::Entities::TR4::NUM_BEETLES; i++)
			{
				BeetleInfo* beetle = &TEN::Entities::TR4::BeetleSwarm[i];

				if (beetle->On)
				{
					RendererMesh* mesh = GetMesh(Objects[ID_LITTLE_BEETLE].meshIndex + ((Wibble >> 2) % 2));
					Matrix translation =
						Matrix::CreateTranslation(beetle->Position.xPos, beetle->Position.yPos, beetle->Position.zPos);
					Matrix rotation = Matrix::CreateFromYawPitchRoll(TO_RAD(beetle->Position.yRot), TO_RAD(beetle->Position.xRot),
					                                                 TO_RAD(beetle->Position.zRot));
					Matrix world = rotation * translation;

					m_stItem.World = world;
					m_stItem.Position = Vector4(beetle->Position.xPos, beetle->Position.yPos, beetle->Position.zPos, 1.0f);
					m_stItem.AmbientLight = m_rooms[beetle->RoomNumber].AmbientLight;
					m_cbItem.updateData(m_stItem, m_context.Get());

					for (int b = 0; b < mesh->buckets.size(); b++)
					{
						RendererBucket* bucket = &mesh->buckets[b];

						if (bucket->Vertices.size() == 0)
							continue;

						m_context->DrawIndexed(bucket->Indices.size(), bucket->StartIndex, 0);
						m_numDrawCalls++;
					}
				}
			}
		}
	}

	void Renderer11::DrawLocusts(RenderView& view)
	{
		UINT stride = sizeof(RendererVertex);
		UINT offset = 0;

		m_context->IASetVertexBuffers(0, 1, m_moveablesVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetInputLayout(m_inputLayout.Get());
		m_context->IASetIndexBuffer(m_moveablesIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		if (Objects[ID_LOCUSTS].loaded)
		{
			OBJECT_INFO* obj = &Objects[ID_LOCUSTS];
			RendererObject& moveableObj = *m_moveableObjects[ID_LOCUSTS];

			for (int m = 0; m < 32; m++)
				memcpy(&m_stItem.BonesMatrices[m], &Matrix::Identity, sizeof(Matrix));

			for (int i = 0; i < TEN::Entities::TR4::MAX_LOCUSTS; i++)
			{
				LOCUST_INFO* locust = &TEN::Entities::TR4::Locusts[i];

				if (locust->on)
				{
					RendererMesh* mesh = GetMesh(Objects[ID_LOCUSTS].meshIndex + (-locust->counter & 3));
					Matrix translation =
						Matrix::CreateTranslation(locust->pos.xPos, locust->pos.yPos, locust->pos.zPos);
					Matrix rotation = Matrix::CreateFromYawPitchRoll(locust->pos.yRot, locust->pos.xRot,
					                                                 locust->pos.zRot);
					Matrix world = rotation * translation;

					m_stItem.World = world;
					m_stItem.Position = Vector4(locust->pos.xPos, locust->pos.yPos, locust->pos.zPos, 1.0f);
					m_stItem.AmbientLight = m_rooms[locust->roomNumber].AmbientLight;
					m_cbItem.updateData(m_stItem, m_context.Get());

					for (int b = 0; b < mesh->buckets.size(); b++)
					{
						RendererBucket* bucket = &mesh->buckets[b];

						if (bucket->Vertices.size() == 0)
							continue;

						m_context->DrawIndexed(bucket->Indices.size(), bucket->StartIndex, 0);
						m_numDrawCalls++;
					}
				}
			}
		}
	}

	void Renderer11::DrawLines3D(RenderView& view)
	{
		m_context->RSSetState(m_states->CullNone());
		m_context->OMSetBlendState(m_states->Additive(), NULL, 0xFFFFFFFF);
		m_context->OMSetDepthStencilState(m_states->DepthRead(), 0);

		m_context->VSSetShader(m_vsSolid.Get(), NULL, 0);
		m_context->PSSetShader(m_psSolid.Get(), NULL, 0);

		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		m_context->IASetInputLayout(m_inputLayout.Get());

		m_primitiveBatch->Begin();

		for (int i = 0; i < m_lines3DToDraw.size(); i++)
		{
			RendererLine3D* line = &m_lines3DToDraw[i];

			RendererVertex v1;
			v1.Position = line->start;
			v1.Color = line->color;

			RendererVertex v2;
			v2.Position = line->end;
			v2.Color = line->color;
			m_primitiveBatch->DrawLine(v1, v2);
		}

		m_primitiveBatch->End();

		m_context->RSSetState(m_states->CullCounterClockwise());
		m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
		m_context->OMSetDepthStencilState(m_states->DepthDefault(), 0);
	}

	void Renderer11::addLine3D(Vector3 start, Vector3 end, Vector4 color)
	{
		RendererLine3D line;

		line.start = start;
		line.end = end;
		line.color = color;

		m_lines3DToDraw.push_back(line);
	}

	void Renderer11::addSphere(Vector3 center, float radius, Vector4 color)
	{
		constexpr auto subdivisions = 10;
		constexpr auto steps = 6;
		constexpr auto step = PI / steps;

		std::array<Vector3, 3> prevPoint;

		for (int s = 0; s < steps; s++)
		{
			auto x = sin(step * (float)s) * radius;
			auto z = cos(step * (float)s) * radius;
			float currAngle = 0.0f;

			for (int i = 0; i < subdivisions; i++)
			{
				std::array<Vector3, 3> point =
				{
					center + Vector3(sin(currAngle) * abs(x), z, cos(currAngle) * abs(x)),
					center + Vector3(cos(currAngle) * abs(x), sin(currAngle) * abs(x), z),
					center + Vector3(z, sin(currAngle) * abs(x), cos(currAngle) * abs(x))
				};

				if (i > 0)
					for (int p = 0; p < 3; p++)
						addLine3D(prevPoint[p], point[p], color);

				prevPoint = point;
				currAngle += ((PI * 2) / (subdivisions - 1));
			}
		}
	}

	void Renderer11::addDebugSphere(Vector3 center, float radius, Vector4 color, RENDERER_DEBUG_PAGE page)
	{
#ifdef _DEBUG
		if (m_numDebugPage != page)
			return;

		addSphere(center, radius, color);
#endif _DEBUG
	}

	void Renderer11::addBox(Vector3* corners, Vector4 color)
	{
		for (int i = 0; i < 12; i++)
		{
			RendererLine3D line;

			switch (i)
			{
			case 0: line.start = corners[0];
				line.end = corners[1];
				break;
			case 1: line.start = corners[1];
				line.end = corners[2];
				break;
			case 2: line.start = corners[2];
				line.end = corners[3];
				break;
			case 3: line.start = corners[3];
				line.end = corners[0];
				break;


			case 4: line.start = corners[4];
				line.end = corners[5];
				break;
			case 5: line.start = corners[5];
				line.end = corners[6];
				break;
			case 6: line.start = corners[6];
				line.end = corners[7];
				break;
			case 7: line.start = corners[7];
				line.end = corners[4];
				break;


			case 8: line.start = corners[0];
				line.end = corners[4];
				break;
			case 9: line.start = corners[1];
				line.end = corners[5];
				break;
			case 10: line.start = corners[2];
				line.end = corners[6];
				break;
			case 11: line.start = corners[3];
				line.end = corners[7];
				break;
			}

			line.color = color;
			m_lines3DToDraw.push_back(line);
		}
	}

	void Renderer11::addBox(Vector3 min, Vector3 max, Vector4 color)
	{
		for (int i = 0; i < 12; i++)
		{
			RendererLine3D line;

			switch (i)
			{
			case 0: line.start = Vector3(min.x, min.y, min.z);
				line.end = Vector3(min.x, min.y, max.z);
				break;
			case 1: line.start = Vector3(min.x, min.y, max.z);
				line.end = Vector3(max.x, min.y, max.z);
				break;
			case 2: line.start = Vector3(max.x, min.y, max.z);
				line.end = Vector3(max.x, min.y, min.z);
				break;
			case 3: line.start = Vector3(max.x, min.y, min.z);
				line.end = Vector3(min.x, min.y, min.z);
				break;

			case 4: line.start = Vector3(min.x, max.y, min.z);
				line.end = Vector3(min.x, max.y, max.z);
				break;
			case 5: line.start = Vector3(min.x, max.y, max.z);
				line.end = Vector3(max.x, max.y, max.z);
				break;
			case 6: line.start = Vector3(max.x, max.y, max.z);
				line.end = Vector3(max.x, max.y, min.z);
				break;
			case 7: line.start = Vector3(max.x, max.y, min.z);
				line.end = Vector3(min.x, max.y, min.z);
				break;

			case 8: line.start = Vector3(min.x, min.y, min.z);
				line.end = Vector3(min.x, max.y, min.z);
				break;
			case 9: line.start = Vector3(min.x, min.y, max.z);
				line.end = Vector3(min.x, max.y, max.z);
				break;
			case 10: line.start = Vector3(max.x, min.y, max.z);
				line.end = Vector3(max.x, max.y, max.z);
				break;
			case 11: line.start = Vector3(max.x, min.y, min.z);
				line.end = Vector3(max.x, max.y, min.z);
				break;
			}

			line.color = color;
			m_lines3DToDraw.push_back(line);
		}
	}

	void Renderer11::addDebugBox(BoundingOrientedBox box, Vector4 color, RENDERER_DEBUG_PAGE page)
	{
#ifdef _DEBUG
		if (m_numDebugPage != page)
			return;

		Vector3 corners[8];
		box.GetCorners(corners);
		addBox(corners, color);
#endif _DEBUG
	}

	void Renderer11::addDebugBox(Vector3 min, Vector3 max, Vector4 color, RENDERER_DEBUG_PAGE page)
	{
#ifdef _DEBUG
		if (m_numDebugPage != page)
			return;
		addBox(min, max, color);
#endif _DEBUG
	}

	void Renderer11::renderLoadingScreen(std::wstring& fileName)
	{
		return;
		/*

		Texture2D texture = Texture2D(m_device, fileName);

		m_fadeStatus = RENDERER_FADE_STATUS::FADE_IN;
		m_fadeFactor = 0.0f;

		while (true) {
		    if (m_fadeStatus == RENDERER_FADE_STATUS::FADE_IN && m_fadeFactor < 1.0f)
		        m_fadeFactor += FADE_FACTOR;

		    if (m_fadeStatus == RENDERER_FADE_STATUS::FADE_OUT && m_fadeFactor > 0.0f)
		        m_fadeFactor -= FADE_FACTOR;

		    // Set basic render states
		    m_context->OMSetDepthStencilState(m_states->DepthDefault(), 0);
		    m_context->RSSetState(m_states->CullCounterClockwise());
		    m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);

		    // Clear screen
		    m_context->ClearRenderTargetView(m_backBufferRTV, Colors::Black);
		    m_context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		    // Bind the back buffer
		    m_context->OMSetRenderTargets(1, &m_backBufferRTV, m_depthStencilView);
		    m_context->RSSetViewports(1, &m_viewport);

		    // Draw the full screen background
		    DrawFullScreenQuad(texture.ShaderResourceView.GetAddressOf(), Vector3(m_fadeFactor, m_fadeFactor, m_fadeFactor), false);
		    m_context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		    m_swapChain->Present(0, 0);
		    m_context->ClearState();
		    if (m_fadeStatus == RENDERER_FADE_STATUS::FADE_IN && m_fadeFactor >= 1.0f) {
		        m_fadeStatus = RENDERER_FADE_STATUS::NO_FADE;
		        m_fadeFactor = 1.0f;
		    }

		    if (m_fadeStatus == RENDERER_FADE_STATUS::NO_FADE && m_progress == 100) {
		        m_fadeStatus = RENDERER_FADE_STATUS::FADE_OUT;
		        m_fadeFactor = 1.0f;
		    }

		    if (m_fadeStatus == RENDERER_FADE_STATUS::FADE_OUT && m_fadeFactor <= 0.0f) {
		        break;
		    }
		}
		*/
	}

	void Renderer11::addDynamicLight(int x, int y, int z, short falloff, byte r, byte g, byte b)
	{
		RendererLight dynamicLight;

		if (falloff >= 8)
		{
			dynamicLight.Color = Vector3(r / 255.0f, g / 255.0f, b / 255.0f) * 2.0f;
		}
		else
		{
			r = (r * falloff) >> 3;
			g = (g * falloff) >> 3;
			b = (b * falloff) >> 3;

			dynamicLight.Color = Vector3(r / 255.0f, g / 255.0f, b / 255.0f) * 2.0f;
		}

		dynamicLight.Position = Vector3(float(x), float(y), float(z));
		dynamicLight.Out = falloff * 256.0f;
		dynamicLight.Type = LIGHT_TYPES::LIGHT_TYPE_POINT;

		dynamicLights.push_back(dynamicLight);
	}

	void Renderer11::clearDynamicLights()
	{
		dynamicLights.clear();
	}

	void Renderer11::DrawFullScreenImage(ID3D11ShaderResourceView* texture, float fade, ID3D11RenderTargetView* target,
	                                     ID3D11DepthStencilView* depthTarget)
	{
		// Reset GPU state
		m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
		m_context->RSSetState(m_states->CullCounterClockwise());
		m_context->OMSetDepthStencilState(m_states->DepthDefault(), 0);
		m_context->OMSetRenderTargets(1, &target, depthTarget);
		m_context->RSSetViewports(1, &m_viewport);
		DrawFullScreenQuad(texture, Vector3(fade, fade, fade), false);
	}

	void Renderer11::renderInventory()
	{
		m_context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_STENCIL | D3D11_CLEAR_DEPTH, 1.0f, 0);
		m_context->ClearRenderTargetView(m_backBufferRTV, Colors::Black);
		RenderInventoryScene(m_backBufferRTV, m_depthStencilView, m_dumpScreenRenderTarget.ShaderResourceView.Get());
		m_swapChain->Present(0, 0);
	}

	void Renderer11::renderTitle()
	{
		m_context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_STENCIL | D3D11_CLEAR_DEPTH, 1.0f, 0);
		m_context->ClearRenderTargetView(m_backBufferRTV, Colors::Black);

		renderScene(m_backBufferRTV, m_depthStencilView, gameCamera);
		m_context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_STENCIL | D3D11_CLEAR_DEPTH, 1.0f, 0);

		RenderInventoryScene(m_backBufferRTV, m_depthStencilView, nullptr);
#if _DEBUG
		drawString(0, 0, commit.c_str(), D3DCOLOR_ARGB(255, 255, 255, 255), 0);
		DrawAllStrings();
#else
        DrawAllStrings();
#endif
		m_swapChain->Present(0, 0);
	}

	void Renderer11::clearScene()
	{
		resetAnimations();

		// Clear dynamic lights
		ClearFires();

		clearDynamicLights();
		ClearSceneItems();

		m_strings.clear();

		m_currentCausticsFrame++;
		m_currentCausticsFrame %= 32;

		calculateFrameRate();
	}

	void Renderer11::drawDebugInfo()
	{
		if (CurrentLevel != 0)
		{
			m_currentY = 60;

			ROOM_INFO* r = &g_Level.Rooms[LaraItem->RoomNumber];

			switch (m_numDebugPage)
			{
			case RENDERER_DEBUG_PAGE::NO_PAGE:
				break;

			case RENDERER_DEBUG_PAGE::RENDERER_STATS:
				PrintDebugMessage("Fps: %3.2f", m_fps);
				PrintDebugMessage("Update time: %d", m_timeUpdate);
				PrintDebugMessage("Frame time: %d", m_timeFrame);
				PrintDebugMessage("Total draw calls: %d", m_numDrawCalls);
				PrintDebugMessage("Transparent faces draw calls: %d", m_numTransparentDrawCalls);
				PrintDebugMessage("    For rooms: %d", m_numRoomsTransparentDrawCalls);
				PrintDebugMessage("    For moveables: %d", m_numMoveablesTransparentDrawCalls);
				PrintDebugMessage("    For statics: %d", m_numStaticsTransparentDrawCalls);
				PrintDebugMessage("    For sprites: %d", m_numSpritesTransparentDrawCalls);
				PrintDebugMessage("Biggest room's index buffer: %d", m_biggestRoomIndexBuffer);
				PrintDebugMessage("Total rooms transparent polygons: %d", numRoomsTransparentPolygons);
				break;

			case RENDERER_DEBUG_PAGE::DIMENSION_STATS:
				PrintDebugMessage("Lara.Location: %d %d", LaraItem->Location.roomNumber, LaraItem->Location.yNumber);
				PrintDebugMessage("Lara.RoomNumber: %d", LaraItem->RoomNumber);
				PrintDebugMessage("LaraItem.BoxNumber: %d",/* canJump: %d, canLongJump: %d, canMonkey: %d,*/
				                  LaraItem->BoxNumber);
				PrintDebugMessage("Lara.Pos: %d %d %d", LaraItem->Position.xPos, LaraItem->Position.yPos, LaraItem->Position.zPos);
				PrintDebugMessage("Lara.Rot: %d %d %d", LaraItem->Position.xRot, LaraItem->Position.yRot, LaraItem->Position.zRot);
				PrintDebugMessage("Room: %d %d %d %d", r->x, r->z, r->x + r->xSize * WALL_SIZE,
				                  r->z + r->zSize * WALL_SIZE);
				PrintDebugMessage("Room.y, minFloor, maxCeiling: %d %d %d ", r->y, r->minfloor, r->maxceiling);
				PrintDebugMessage("Camera.pos: %d %d %d", Camera.pos.x, Camera.pos.y, Camera.pos.z);
				PrintDebugMessage("Camera.target: %d %d %d", Camera.target.x, Camera.target.y, Camera.target.z);
				break;

			case RENDERER_DEBUG_PAGE::LARA_STATS:
				PrintDebugMessage("Lara.AnimNumber: %d", LaraItem->AnimNumber);
				PrintDebugMessage("Lara.FrameNumber: %d", LaraItem->FrameNumber);
				PrintDebugMessage("Lara.ActiveState: %d", LaraItem->ActiveState);
				PrintDebugMessage("Lara.RequiredState: %d", LaraItem->RequiredState);
				PrintDebugMessage("Lara.TargetState: %d", LaraItem->TargetState);
				PrintDebugMessage("Lara.Control.WeaponControl.WeaponItem: %d", Lara.Control.Weapon.WeaponItem);
				PrintDebugMessage("Lara.Control.WeaponControl.GunType: %d", Lara.Control.Weapon.GunType);
				PrintDebugMessage("Lara.Control.HandStatus: %d", Lara.Control.HandStatus);
				PrintDebugMessage("Lara.Velocity, VerticalVelocity: %d %d", LaraItem->Velocity, LaraItem->VerticalVelocity);
				PrintDebugMessage("Lara.Airborne: %d", LaraItem->Airborne);
				PrintDebugMessage("Lara.Control.CanClimbLadder: %d", Lara.Control.CanClimbLadder);
				PrintDebugMessage("Lara.WaterSurfaceDist: %d", Lara.WaterSurfaceDist);
				break;

			case RENDERER_DEBUG_PAGE::LOGIC_STATS:
				PrintDebugMessage("target HitPoints: %d", Lara.TargetEntity ? Lara.TargetEntity->HitPoints : NULL);
				PrintDebugMessage("CollidedVolume: %d", TEN::Control::Volumes::CurrentCollidedVolume);
				break;
			}
		}
	}

	void Renderer11::switchDebugPage(bool back)
	{
		auto index = (int)m_numDebugPage;

		if (back)
			--index;
		else
			++index;

		if (index < RENDERER_DEBUG_PAGE::NO_PAGE)
			index = 4;
		else if (index > RENDERER_DEBUG_PAGE::LOGIC_STATS)
			index = 0;

		m_numDebugPage = (RENDERER_DEBUG_PAGE)index;
	}

	void Renderer11::DrawTransparentFaces(RenderView& view)
	{
		std::for_each(std::execution::par_unseq,
		              view.roomsToDraw.begin(),
		              view.roomsToDraw.end(),
		              [](RendererRoom* room)
		              {
			              std::sort(
				              room->TransparentFacesToDraw.begin(),
				              room->TransparentFacesToDraw.end(),
				              [](RendererTransparentFace& a, RendererTransparentFace& b)
				              {
					              return (a.distance > b.distance);
				              }
			              );
		              }
		);

		for (int r = view.roomsToDraw.size() - 1; r >= 0; r--)
		{
			RendererRoom& room = *view.roomsToDraw[r];

			int size = room.TransparentFacesToDraw.size();
			int currentBlendMode = -1;
			int altas = -1;
			int lastType = 0;

			std::vector<RendererVertex> vertices;
			//vertices.reserve(50000);

			m_transparentFacesVertices.clear();
			m_transparentFacesIndices.clear();

			RendererTransparentFaceType oldType = RendererTransparentFaceType::TRANSPARENT_FACE_NONE;
			RendererTransparentFaceInfo* oldInfo = nullptr;
			bool outputPolygons = false;

			for (auto& face : room.TransparentFacesToDraw)
			{
				if (oldInfo != nullptr)
				{
					// Check if it's time to output polygons
					if (oldType != face.type)
					{
						outputPolygons = true;
					}
					else
					{
						// If same type, check additional conditions
						if (face.type == RendererTransparentFaceType::TRANSPARENT_FACE_ROOM &&
							(oldInfo->room != face.info.room
								|| oldInfo->texture != face.info.texture
								|| oldInfo->animated != face.info.animated
								|| oldInfo->blendMode != face.info.blendMode
								|| m_transparentFacesIndices.size() + (face.info.polygon->shape ? 3 : 6) >
								MAX_TRANSPARENT_VERTICES))
						{
							outputPolygons = true;
						}
						else if (face.type == RendererTransparentFaceType::TRANSPARENT_FACE_MOVEABLE &&
							(oldInfo->blendMode != face.info.blendMode
								|| oldInfo->item->ItemNumber != face.info.item->ItemNumber
								|| m_transparentFacesIndices.size() + (face.info.polygon->shape ? 3 : 6) >
								MAX_TRANSPARENT_VERTICES))
						{
							outputPolygons = true;
						}
						else if (face.type == RendererTransparentFaceType::TRANSPARENT_FACE_STATIC &&
							(oldInfo->blendMode != face.info.blendMode
								|| oldInfo->staticMesh != face.info.staticMesh
								|| m_transparentFacesIndices.size() + (face.info.polygon->shape ? 3 : 6) >
								MAX_TRANSPARENT_VERTICES))
						{
							outputPolygons = true;
						}
						else if (face.type == RendererTransparentFaceType::TRANSPARENT_FACE_SPRITE &&
							(oldInfo->blendMode != face.info.blendMode
								|| oldInfo->sprite->Type != face.info.sprite->Type
								|| oldInfo->sprite->color != face.info.sprite->color
								|| oldInfo->sprite->Sprite != face.info.sprite->Sprite
								|| m_transparentFacesIndices.size() + 6 > MAX_TRANSPARENT_VERTICES))
						{
							outputPolygons = true;
						}
					}
				}

				if (outputPolygons)
				{
					// Here we send polygons to the GPU for drawing and we clear the vertex buffer
					if (oldType == RendererTransparentFaceType::TRANSPARENT_FACE_ROOM)
					{
						DrawRoomsTransparent(oldInfo, view);
					}
					else if (oldType == RendererTransparentFaceType::TRANSPARENT_FACE_MOVEABLE)
					{
						DrawItemsTransparent(oldInfo, view);
					}
					else if (oldType == RendererTransparentFaceType::TRANSPARENT_FACE_STATIC)
					{
						DrawStaticsTransparent(oldInfo, view);
					}
					else if (oldType == RendererTransparentFaceType::TRANSPARENT_FACE_SPRITE)
					{
						DrawSpritesTransparent(oldInfo, view);
					}

					outputPolygons = false;
					m_transparentFacesVertices.clear();
					m_transparentFacesIndices.clear();
				}

				oldInfo = &face.info;
				oldType = face.type;

				// Accumulate vertices in the buffer
				if (face.type == RendererTransparentFaceType::TRANSPARENT_FACE_ROOM)
				{
					// For rooms, we already pass world coordinates, just copy vertices
					int numIndices = (face.info.polygon->shape == 0 ? 6 : 3);
					m_transparentFacesIndices.bulk_push_back(roomsIndices.data(), face.info.polygon->baseIndex,
					                                         numIndices);
				}
				else if (face.type == RendererTransparentFaceType::TRANSPARENT_FACE_MOVEABLE)
				{
					// For rooms, we already pass world coordinates, just copy vertices
					int numIndices = (face.info.polygon->shape == 0 ? 6 : 3);
					m_transparentFacesIndices.bulk_push_back(moveablesIndices.data(), face.info.polygon->baseIndex,
					                                         numIndices);
				}
				else if (face.type == RendererTransparentFaceType::TRANSPARENT_FACE_STATIC)
				{
					// For rooms, we already pass world coordinates, just copy vertices
					int numIndices = (face.info.polygon->shape == 0 ? 6 : 3);
					m_transparentFacesIndices.bulk_push_back(staticsIndices.data(), face.info.polygon->baseIndex,
					                                         numIndices);
				}
				else if (face.type == RendererTransparentFaceType::TRANSPARENT_FACE_SPRITE)
				{
					// For sprites, we need to compute the corners of the quad and multiply 
					// by the world matrix that can be an identity (for 3D sprites) or 
					// a billboard matrix. We transform vertices on the CPU because 
					// CPUs nowadays are fast enough and we save draw calls, in fact 
					// each sprite would require a different world matrix and then 
					// we would fall in the case 1 poly = 1 draw call (worst case scenario).
					// For the same reason, we store also color directly there and we simply 
					// pass a Vector4::One as color to the shader.

					RendererSpriteToDraw* spr = face.info.sprite;

					Vector3 p0t;
					Vector3 p1t;
					Vector3 p2t;
					Vector3 p3t;

					Vector2 uv0;
					Vector2 uv1;
					Vector2 uv2;
					Vector2 uv3;

					if (spr->Type == RENDERER_SPRITE_TYPE::SPRITE_TYPE_3D)
					{
						p0t = spr->vtx1;
						p1t = spr->vtx2;
						p2t = spr->vtx3;
						p3t = spr->vtx4;

						uv0 = spr->Sprite->UV[0];
						uv1 = spr->Sprite->UV[1];
						uv2 = spr->Sprite->UV[2];
						uv3 = spr->Sprite->UV[3];
					}
					else
					{
						p0t = Vector3(-0.5, -0.5, 0);
						p1t = Vector3(-0.5, 0.5, 0);
						p2t = Vector3(0.5, -0.5, 0);
						p3t = Vector3(0.5, 0.5, 0);

						uv0 = Vector2(0, 0);
						uv1 = Vector2(1, 0);
						uv2 = Vector2(0, 1);
						uv3 = Vector2(1, 1);
					}

					RendererVertex v0;
					v0.Position = Vector3::Transform(p0t, face.info.world);
					v0.UV = uv0;
					v0.Color = spr->color;

					RendererVertex v1;
					v1.Position = Vector3::Transform(p1t, face.info.world);
					v1.UV = uv1;
					v1.Color = spr->color;

					RendererVertex v2;
					v2.Position = Vector3::Transform(p2t, face.info.world);
					v2.UV = uv2;
					v2.Color = spr->color;

					RendererVertex v3;
					v3.Position = Vector3::Transform(p3t, face.info.world);
					v3.UV = uv3;
					v3.Color = spr->color;

					m_transparentFacesVertices.push_back(v0);
					m_transparentFacesVertices.push_back(v1);
					m_transparentFacesVertices.push_back(v3);
					m_transparentFacesVertices.push_back(v2);
					m_transparentFacesVertices.push_back(v3);
					m_transparentFacesVertices.push_back(v1);
				}
			}

			// Here we send polygons to the GPU for drawing and we clear the vertex buffer
			if (oldType == RendererTransparentFaceType::TRANSPARENT_FACE_ROOM)
			{
				DrawRoomsTransparent(oldInfo, view);
			}
			else if (oldType == RendererTransparentFaceType::TRANSPARENT_FACE_MOVEABLE)
			{
				DrawItemsTransparent(oldInfo, view);
			}
			else if (oldType == RendererTransparentFaceType::TRANSPARENT_FACE_STATIC)
			{
				DrawStaticsTransparent(oldInfo, view);
			}
			else if (oldType == RendererTransparentFaceType::TRANSPARENT_FACE_SPRITE)
			{
				DrawSpritesTransparent(oldInfo, view);
			}
		}

		m_context->RSSetState(m_states->CullCounterClockwise());
	}

	void Renderer11::DrawRoomsTransparent(RendererTransparentFaceInfo* info, RenderView& view)
	{
		UINT stride = sizeof(RendererVertex);
		UINT offset = 0;

		ROOM_INFO* nativeRoom = &g_Level.Rooms[info->room->RoomNumber];

		// Set vertex buffer
		//m_transparentFacesVertexBuffer.Update(m_context.Get(), m_transparentFacesVertices, 0, m_transparentFacesVertices.size());

		m_context->IASetVertexBuffers(0, 1, m_roomsVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetInputLayout(m_inputLayout.Get());

		// Set shaders
		if (!info->animated)
		{
			m_context->VSSetShader(m_vsRooms.Get(), nullptr, 0);
		}
		else
		{
			m_context->VSSetConstantBuffers(6, 1, m_cbAnimated.get());
			m_context->VSSetShader(m_vsRooms_Anim.Get(), nullptr, 0);
		}

		m_context->PSSetShader(m_psRooms.Get(), NULL, 0);

		// Set texture
		int nmeshes = -Objects[ID_CAUSTICS_TEXTURES].nmeshes;
		int meshIndex = Objects[ID_CAUSTICS_TEXTURES].meshIndex;
		int causticsFrame = nmeshes ? meshIndex + ((GlobalCounter) % nmeshes) : 0;
		BindTexture(TextureRegister::CausticsTexture, m_sprites[causticsFrame].Texture, SamplerStateType::None);
		BindTexture(TextureRegister::ShadowMapTexture, &m_shadowMap, SamplerStateType::ShadowMap);

		// Set shadow map data
		if (shadowLight != NULL)
		{
			memcpy(&m_stShadowMap.Light, shadowLight, sizeof(ShaderLight));
			m_stShadowMap.CastShadows = true;
			//m_stShadowMap.ViewProjectionInverse = ViewProjection.Invert();
		}
		else
		{
			m_stShadowMap.CastShadows = false;
		}
		m_cbShadowMap.updateData(m_stShadowMap, m_context.Get());
		m_context->VSSetConstantBuffers(4, 1, m_cbShadowMap.get());
		m_context->PSSetConstantBuffers(4, 1, m_cbShadowMap.get());

		m_stLights.NumLights = view.lightsToDraw.size();
		for (int j = 0; j < view.lightsToDraw.size(); j++)
			memcpy(&m_stLights.Lights[j], view.lightsToDraw[j], sizeof(ShaderLight));
		m_cbLights.updateData(m_stLights, m_context.Get());
		m_context->PSSetConstantBuffers(1, 1, m_cbLights.get());

		m_stMisc.Caustics = (nativeRoom->flags & ENV_FLAG_WATER);
		m_stMisc.AlphaTest = false;
		m_cbMisc.updateData(m_stMisc, m_context.Get());
		m_context->PSSetConstantBuffers(3, 1, m_cbMisc.get());

		m_stRoom.AmbientColor = info->room->AmbientLight;
		m_stRoom.Water = (nativeRoom->flags & ENV_FLAG_WATER) != 0 ? 1 : 0;
		m_cbRoom.updateData(m_stRoom, m_context.Get());

		m_context->VSSetConstantBuffers(5, 1, m_cbRoom.get());
		m_context->PSSetConstantBuffers(5, 1, m_cbRoom.get());

		// Draw geometry
		if (info->animated)
		{
			BindTexture(TextureRegister::MainTexture, &std::get<0>(m_animatedTextures[info->texture]),
			            SamplerStateType::AnisotropicClamp);
			BindTexture(TextureRegister::NormalMapTexture, &std::get<1>(m_animatedTextures[info->texture]),
			            SamplerStateType::None);

			RendererAnimatedTextureSet& set = m_animatedTextureSets[info->texture];
			m_stAnimated.NumFrames = set.NumTextures;
			for (unsigned char i = 0; i < set.NumTextures; i++)
			{
				auto& tex = set.Textures[i];
				m_stAnimated.Textures[i].topLeft = set.Textures[i].UV[0];
				m_stAnimated.Textures[i].topRight = set.Textures[i].UV[1];
				m_stAnimated.Textures[i].bottomRight = set.Textures[i].UV[2];
				m_stAnimated.Textures[i].bottomLeft = set.Textures[i].UV[3];
			}
			m_cbAnimated.updateData(m_stAnimated, m_context.Get());
		}
		else
		{
			BindTexture(TextureRegister::MainTexture, &std::get<0>(m_roomTextures[info->texture]),
			            SamplerStateType::AnisotropicClamp);
			BindTexture(TextureRegister::NormalMapTexture, &std::get<1>(m_roomTextures[info->texture]),
			            SamplerStateType::None);
		}

		setBlendMode(info->blendMode);

		m_biggestRoomIndexBuffer = std::fmaxf(m_biggestRoomIndexBuffer, m_transparentFacesIndices.size());

		int drawnVertices = 0;
		int size = m_transparentFacesIndices.size();

		while (drawnVertices < size)
		{
			int count = (drawnVertices + TRANSPARENT_BUCKET_SIZE > size
				             ? size - drawnVertices
				             : TRANSPARENT_BUCKET_SIZE);

			m_transparentFacesIndexBuffer.Update(m_context.Get(), m_transparentFacesIndices, drawnVertices, count);
			m_context->IASetIndexBuffer(m_transparentFacesIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

			m_context->DrawIndexed(count, 0, 0);

			m_numDrawCalls++;
			m_numTransparentDrawCalls++;
			m_numRoomsTransparentDrawCalls++;

			drawnVertices += TRANSPARENT_BUCKET_SIZE;
		}

		m_context->RSSetState(m_states->CullCounterClockwise());
	}

	void Renderer11::DrawStaticsTransparent(RendererTransparentFaceInfo* info, RenderView& view)
	{
		UINT stride = sizeof(RendererVertex);
		UINT offset = 0;

		m_context->IASetVertexBuffers(0, 1, m_staticsVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetInputLayout(m_inputLayout.Get());

		// Set shaders
		m_context->VSSetShader(m_vsStatics.Get(), NULL, 0);
		m_context->PSSetShader(m_psStatics.Get(), NULL, 0);

		// Set texture
		BindTexture(TextureRegister::MainTexture, &std::get<0>(m_staticsTextures[info->bucket->Texture]),
		            SamplerStateType::AnisotropicClamp);
		BindTexture(TextureRegister::NormalMapTexture, &std::get<1>(m_staticsTextures[info->bucket->Texture]),
		            SamplerStateType::None);

		m_stStatic.World = info->world;
		m_stStatic.Position = Vector4(info->position.x, info->position.y, info->position.z, 1.0f);
		m_stStatic.Color = info->color;
		m_cbStatic.updateData(m_stStatic, m_context.Get());
		m_context->VSSetConstantBuffers(1, 1, m_cbStatic.get());

		m_stMisc.AlphaTest = false;
		m_cbMisc.updateData(m_stMisc, m_context.Get());
		m_context->VSSetConstantBuffers(3, 1, m_cbMisc.get());

		setBlendMode(info->blendMode);

		int drawnVertices = 0;
		int size = m_transparentFacesIndices.size();

		while (drawnVertices < size)
		{
			int count = (drawnVertices + TRANSPARENT_BUCKET_SIZE > size
				             ? size - drawnVertices
				             : TRANSPARENT_BUCKET_SIZE);

			m_transparentFacesIndexBuffer.Update(m_context.Get(), m_transparentFacesIndices, drawnVertices, count);
			m_context->IASetIndexBuffer(m_transparentFacesIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

			m_context->DrawIndexed(count, 0, 0);

			m_numDrawCalls++;
			m_numTransparentDrawCalls++;
			m_numStaticsTransparentDrawCalls++;

			drawnVertices += TRANSPARENT_BUCKET_SIZE;
		}
	}

	void Renderer11::DoFadingAndCinematicBars(ID3D11RenderTargetView* target, ID3D11DepthStencilView* depthTarget,
	                                          RenderView& view)
	{
		setBlendMode(BLENDMODE_OPAQUE);
		m_context->RSSetState(m_states->CullCounterClockwise());

		m_context->ClearRenderTargetView(target, Colors::Black);
		m_context->ClearDepthStencilView(depthTarget, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		m_context->OMSetRenderTargets(1, &target, depthTarget);
		m_context->RSSetViewports(1, &view.viewport);

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

		m_context->VSSetShader(m_vsFinalPass.Get(), NULL, 0);
		m_context->PSSetShader(m_psFinalPass.Get(), NULL, 0);

		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetInputLayout(m_inputLayout.Get());

		m_stPostProcessBuffer.ViewportWidth = ScreenWidth;
		m_stPostProcessBuffer.ViewportHeight = ScreenHeight;
		m_stPostProcessBuffer.ScreenFadeFactor = ScreenFadeCurrent;
		m_stPostProcessBuffer.CinematicBarsHeight = CinematicBarsHeight;
		m_cbPostProcessBuffer.updateData(m_stPostProcessBuffer, m_context.Get());
		m_context->PSSetConstantBuffers(7, 1, m_cbPostProcessBuffer.get());

		BindTexture(TextureRegister::MainTexture, &m_renderTarget, SamplerStateType::AnisotropicClamp);

		m_primitiveBatch->Begin();
		m_primitiveBatch->DrawQuad(vertices[0], vertices[1], vertices[2], vertices[3]);
		m_primitiveBatch->End();
	}

	void Renderer11::renderScene(ID3D11RenderTargetView* target, ID3D11DepthStencilView* depthTarget, RenderView& view)
	{
		m_timeUpdate = 0;
		m_timeDraw = 0;
		m_timeFrame = 0;
		m_numDrawCalls = 0;
		m_numTransparentDrawCalls = 0;
		m_nextSprite = 0;
		m_numRoomsTransparentDrawCalls = 0;
		m_numMoveablesTransparentDrawCalls = 0;
		m_numStaticsTransparentDrawCalls = 0;
		m_numSpritesTransparentDrawCalls = 0;
		m_biggestRoomIndexBuffer = 0;

		m_transparentFaces.clear();

		using ns = std::chrono::nanoseconds;
		using get_time = std::chrono::steady_clock;

		GameScriptLevel* level = g_GameFlow->GetLevel(CurrentLevel);

		m_stLights.CameraPosition = view.camera.WorldPosition;

		// Prepare the scene to draw
		auto time1 = std::chrono::high_resolution_clock::now();
		CollectRooms(view, false);
		updateLaraAnimations(false);
		UpdateItemsAnimations(view);
		UpdateEffects(view);

		// Prepare the shadow map
		if (g_Configuration.EnableShadows)
			RenderShadowMap(view);

		// Setup Lara item
		m_items[Lara.ItemNumber].ItemNumber = Lara.ItemNumber;
		CollectLightsForItem(LaraItem->RoomNumber, &m_items[Lara.ItemNumber], view);

		auto time2 = std::chrono::high_resolution_clock::now();
		m_timeUpdate = (std::chrono::duration_cast<ns>(time2 - time1)).count() / 1000000;
		time1 = time2;

		// Reset GPU state
		setBlendMode(BLENDMODE_OPAQUE);
		m_context->RSSetState(m_states->CullCounterClockwise());

		// Bind and clear render target
		m_context->ClearRenderTargetView(m_renderTarget.RenderTargetView.Get(), Colors::Black);
		m_context->ClearDepthStencilView(m_renderTarget.DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		                                 1.0f, 0);
		m_context->OMSetRenderTargets(1, m_renderTarget.RenderTargetView.GetAddressOf(),
		                              m_renderTarget.DepthStencilView.Get());
		m_context->RSSetViewports(1, &view.viewport);

		// The camera constant buffer contains matrices, camera position, fog values and other 
		// things that are shared for all shaders
		CCameraMatrixBuffer cameraConstantBuffer;
		view.fillConstantBuffer(cameraConstantBuffer);
		cameraConstantBuffer.Frame = GlobalCounter;
		cameraConstantBuffer.CameraUnderwater = g_Level.Rooms[cameraConstantBuffer.RoomNumber].flags & ENV_FLAG_WATER;
		GameScriptFog fog = g_GameFlow->GetLevel(CurrentLevel)->Fog;
		if (fog.Enabled)
		{
			cameraConstantBuffer.FogColor = Vector4(fog.R / 255.0f, fog.G / 255.0f, fog.B / 255.0f, 1.0f);
			cameraConstantBuffer.FogMinDistance = fog.MinDistance;
			cameraConstantBuffer.FogMaxDistance = fog.MaxDistance;
		}
		else
		{
			cameraConstantBuffer.FogMaxDistance = 0;
		}
		m_cbCameraMatrices.updateData(cameraConstantBuffer, m_context.Get());
		m_context->VSSetConstantBuffers(0, 1, m_cbCameraMatrices.get());

		// Draw the horizon and the sky
		DrawHorizonAndSky(view, m_renderTarget.DepthStencilView.Get());

		// Draw rooms and objects
		DrawRooms(view);
		DrawStatics(view);
		DrawLara(false, view);
		DrawItems(view);
		DrawEffects(view, false);
		DrawGunFlashes(view);
		DrawGunShells(view);
		DrawBaddieGunflashes(view);
		DrawDebris(view, false);
		DrawBats(view);
		DrawRats(view);
		DrawSpiders(view);
		DrawScarabs(view);
		DrawLocusts(view);

		// Do special effects and weather 
		// NOTE: functions here just fill the sprites to draw array
		DrawFires(view);
		DrawSmokes(view);
		drawSmokeParticles(view);
		drawSimpleParticles(view);
		drawSparkParticles(view);
		drawExplosionParticles(view);
		drawFootprints(view);
		drawDripParticles(view);
		DrawBlood(view);
		DrawWeatherParticles(view);
		DrawSparks(view);
		DrawBubbles(view);
		DrawDrips(view);
		DrawRipples(view);
		DrawUnderwaterDust(view);
		DrawSplashes(view);
		DrawShockwaves(view);
		DrawLightning(view);
		DrawRopes(view);

		// Here is where we actually output sprites
		DrawSprites(view);
		DrawLines3D(view);

		// Here we sort transparent faces and draw them with a simplified shaders for alpha blending
		DrawTransparentFaces(view);

		// Draw GUI stuff at the end
		DrawLines2D();

		// Bars
		int flash = FlashIt();
		UpdateSprintBar(LaraItem);
		UpdateHealthBar(LaraItem, flash);
		UpdateAirBar(LaraItem, flash);
		DrawAllPickups();

		DrawOverlays(view); // Draw binoculars or lasersight

		time2 = std::chrono::high_resolution_clock::now();
		m_timeFrame = (std::chrono::duration_cast<ns>(time2 - time1)).count() / 1000000;
		time1 = time2;

		DoFadingAndCinematicBars(target, depthTarget, view);

		drawDebugInfo();
		DrawAllStrings();
		clearScene();
	}

	void Renderer11::renderSimpleScene(ID3D11RenderTargetView* target, ID3D11DepthStencilView* depthTarget,
	                                   RenderView& view)
	{
		GameScriptLevel* level = g_GameFlow->GetLevel(CurrentLevel);

		CollectRooms(view, true);
		// Draw shadow map

		// Reset GPU state
		m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
		m_context->RSSetState(m_states->CullCounterClockwise());
		m_context->OMSetDepthStencilState(m_states->DepthDefault(), 0);

		// Bind and clear render target

		m_context->ClearRenderTargetView(target, Colors::Black);
		m_context->ClearDepthStencilView(depthTarget, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		m_context->OMSetRenderTargets(1, &target, depthTarget);

		m_context->RSSetViewports(1, &view.viewport);

		// Opaque geometry
		m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
		CCameraMatrixBuffer cameraConstantBuffer;
		view.fillConstantBuffer(cameraConstantBuffer);
		cameraConstantBuffer.CameraUnderwater = g_Level.Rooms[cameraConstantBuffer.RoomNumber].flags & ENV_FLAG_WATER;
		m_cbCameraMatrices.updateData(cameraConstantBuffer, m_context.Get());
		m_context->VSSetConstantBuffers(0, 1, m_cbCameraMatrices.get());
		DrawHorizonAndSky(view, depthTarget);
		DrawRooms(view);
	}

	void Renderer11::DumpGameScene()
	{
		renderScene(m_dumpScreenRenderTarget.RenderTargetView.Get(), m_dumpScreenRenderTarget.DepthStencilView.Get(),
		            gameCamera);
	}

	void Renderer11::DrawItems(RenderView& view)
	{
		UINT stride = sizeof(RendererVertex);
		UINT offset = 0;

		m_context->IASetVertexBuffers(0, 1, m_moveablesVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetInputLayout(m_inputLayout.Get());
		m_context->IASetIndexBuffer(m_moveablesIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		// Set shaders
		m_context->VSSetShader(m_vsItems.Get(), NULL, 0);
		m_context->PSSetShader(m_psItems.Get(), NULL, 0);

		for (auto room : view.roomsToDraw)
		{
			for (auto itemToDraw : room->ItemsToDraw)
			{
				ITEM_INFO* nativeItem = &g_Level.Items[itemToDraw->ItemNumber];
				RendererRoom& room = m_rooms[nativeItem->RoomNumber];
				RendererObject& moveableObj = *m_moveableObjects[nativeItem->ObjectNumber];

				if (moveableObj.DoNotDraw)
					continue;

				short objectNumber = nativeItem->ObjectNumber;

				if (objectNumber >= ID_WATERFALL1 && objectNumber <= ID_WATERFALLSS2)
				{
					// We'll draw waterfalls later
					continue;
				}
				else if (objectNumber >= ID_WRAITH1 && objectNumber <= ID_WRAITH3)
				{
					// Wraiths have some additional special effects
					DrawAnimatingItem(itemToDraw, view);
					DrawWraithExtra(itemToDraw, view);
				}
				else if (objectNumber == ID_DARTS)
				{
					//TODO: for now legacy way, in the future mesh
					DrawDarts(itemToDraw, view);
				}
				else
				{
					DrawAnimatingItem(itemToDraw, view);
				}
			}
		}
	}

	void Renderer11::DrawAnimatingItem(RendererItem* item, RenderView& view)
	{
		ITEM_INFO* nativeItem = &g_Level.Items[item->ItemNumber];
		RendererRoom* room = &m_rooms[nativeItem->RoomNumber];
		RendererObject& moveableObj = *m_moveableObjects[nativeItem->ObjectNumber];
		OBJECT_INFO* obj = &Objects[nativeItem->ObjectNumber];

		Vector3 itemPosition = Vector3(nativeItem->Position.xPos, nativeItem->Position.yPos, nativeItem->Position.zPos);
		Vector3 cameraPosition = Vector3(Camera.pos.x, Camera.pos.y, Camera.pos.z);

		m_stItem.World = item->World;
		m_stItem.Position = Vector4(nativeItem->Position.xPos, nativeItem->Position.yPos, nativeItem->Position.zPos, 1.0f);
		m_stItem.AmbientLight = item->AmbientLight;
		memcpy(m_stItem.BonesMatrices, item->AnimationTransforms, sizeof(Matrix) * 32);
		m_cbItem.updateData(m_stItem, m_context.Get());
		m_context->VSSetConstantBuffers(1, 1, m_cbItem.get());
		m_context->PSSetConstantBuffers(1, 1, m_cbItem.get());

		m_stLights.NumLights = item->LightsToDraw.size();
		for (int j = 0; j < m_stLights.NumLights; j++)
			memcpy(&m_stLights.Lights[j], item->LightsToDraw[j], sizeof(ShaderLight));
		m_cbLights.updateData(m_stLights, m_context.Get());
		m_context->PSSetConstantBuffers(2, 1, m_cbLights.get());

		for (int k = 0; k < moveableObj.ObjectMeshes.size(); k++)
		{
			if (!(nativeItem->MeshBits & (1 << k)))
				continue;

			RendererMesh* mesh = moveableObj.ObjectMeshes[k];

			if (obj->meshSwapSlot != -1 && ((nativeItem->SwapMeshFlags >> k) & 1))
			{
				RendererObject& swapMeshObj = *m_moveableObjects[obj->meshSwapSlot];
				if (swapMeshObj.ObjectMeshes.size() > k)
					mesh = swapMeshObj.ObjectMeshes[k];
			}

			drawMoveableMesh(item, mesh, room, k);
		}
	}

	void Renderer11::DrawItemsTransparent(RendererTransparentFaceInfo* info, RenderView& view)
	{
		UINT stride = sizeof(RendererVertex);
		UINT offset = 0;

		// Set vertex buffer
		//m_transparentFacesVertexBuffer.Update(m_context.Get(), m_transparentFacesVertices, 0, m_transparentFacesVertices.size());

		m_context->IASetVertexBuffers(0, 1, m_moveablesVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetInputLayout(m_inputLayout.Get());

		ITEM_INFO* nativeItem = &g_Level.Items[info->item->ItemNumber];
		RendererRoom& room = m_rooms[nativeItem->RoomNumber];
		RendererObject& moveableObj = *m_moveableObjects[nativeItem->ObjectNumber];
		OBJECT_INFO* obj = &Objects[nativeItem->ObjectNumber];

		// Set shaders
		m_context->VSSetShader(m_vsItems.Get(), NULL, 0);
		m_context->PSSetShader(m_psItems.Get(), NULL, 0);

		m_stItem.World = info->item->World;
		m_stItem.Position = Vector4(info->position.x, info->position.y, info->position.z, 1.0f);
		m_stItem.AmbientLight = room.AmbientLight;
		memcpy(m_stItem.BonesMatrices, info->item->AnimationTransforms, sizeof(Matrix) * 32);
		m_cbItem.updateData(m_stItem, m_context.Get());
		m_context->VSSetConstantBuffers(1, 1, m_cbItem.get());
		m_context->PSSetConstantBuffers(1, 1, m_cbItem.get());

		m_stLights.NumLights = info->item->LightsToDraw.size();
		for (int j = 0; j < info->item->LightsToDraw.size(); j++)
			memcpy(&m_stLights.Lights[j], info->item->LightsToDraw[j], sizeof(ShaderLight));
		m_cbLights.updateData(m_stLights, m_context.Get());
		m_context->PSSetConstantBuffers(2, 1, m_cbLights.get());

		// Set texture
		BindTexture(TextureRegister::MainTexture, &std::get<0>(m_moveablesTextures[info->bucket->Texture]),
		            SamplerStateType::AnisotropicClamp);
		BindTexture(TextureRegister::NormalMapTexture, &std::get<1>(m_moveablesTextures[info->bucket->Texture]),
		            SamplerStateType::None);

		m_stMisc.AlphaTest = false;
		m_cbMisc.updateData(m_stMisc, m_context.Get());
		m_context->PSSetConstantBuffers(3, 1, m_cbMisc.get());

		setBlendMode(info->blendMode);

		int drawnVertices = 0;
		int size = m_transparentFacesIndices.size();

		while (drawnVertices < size)
		{
			int count = (drawnVertices + TRANSPARENT_BUCKET_SIZE > size
				             ? size - drawnVertices
				             : TRANSPARENT_BUCKET_SIZE);

			m_transparentFacesIndexBuffer.Update(m_context.Get(), m_transparentFacesIndices, drawnVertices, count);
			m_context->IASetIndexBuffer(m_transparentFacesIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

			m_context->DrawIndexed(count, 0, 0);

			m_numDrawCalls++;
			m_numTransparentDrawCalls++;
			m_numStaticsTransparentDrawCalls++;

			drawnVertices += TRANSPARENT_BUCKET_SIZE;
		}
	}

	void Renderer11::DrawDarts(RendererItem* item, RenderView& view)
	{
		ITEM_INFO* nativeItem = &g_Level.Items[item->ItemNumber];

		Vector3 start = Vector3(
			nativeItem->Position.xPos,
			nativeItem->Position.yPos,
			nativeItem->Position.zPos);

		float speed = (-96 * phd_cos(TO_RAD(nativeItem->Position.xRot)));

		Vector3 end = Vector3(
			nativeItem->Position.xPos + speed * phd_sin(TO_RAD(nativeItem->Position.yRot)),
			nativeItem->Position.yPos + 96 * phd_sin(TO_RAD(nativeItem->Position.xRot)),
			nativeItem->Position.zPos + speed * phd_cos(TO_RAD(nativeItem->Position.yRot)));

		addLine3D(start, end, Vector4(30 / 255.0f, 30 / 255.0f, 30 / 255.0f, 0.5f));
	}

	void Renderer11::DrawWraithExtra(RendererItem* item, RenderView& view)
	{
		ITEM_INFO* nativeItem = &g_Level.Items[item->ItemNumber];
		WraithInfo* info = (WraithInfo*)nativeItem->Data;

		for (int j = 0; j <= 4; j++)
		{
			Matrix rotation;

			switch (j)
			{
			case 0:
				rotation = Matrix::CreateRotationY(TO_RAD(-1092));
				break;
			case 1:
				rotation = Matrix::CreateRotationY(TO_RAD(1092));
				break;
			case 2:
				rotation = Matrix::CreateRotationZ(TO_RAD(-1092));
				break;
			case 3:
				rotation = Matrix::CreateRotationZ(TO_RAD(1092));
				break;
			default:
				rotation = Matrix::Identity;
				break;
			}

			Matrix world = rotation * item->World;

			for (int i = 0; i < 7; i++)
			{
				Vector3 p1 = Vector3(info[i].Position.xPos, info[i].Position.yPos, info[i].Position.zPos);
				Vector3 p2 = Vector3(info[i + 1].Position.xPos, info[i + 1].Position.yPos, info[i + 1].Position.zPos);

				addLine3D(p1, p2, Vector4(info[i].r / 255.0f, info[i].g / 255.0f, info[i].b / 255.0f, 1.0f));
			}
		}
	}

	void Renderer11::DrawStatics(RenderView& view)
	{
		// Bind vertex and index buffer
		UINT stride = sizeof(RendererVertex);
		UINT offset = 0;

		m_context->IASetVertexBuffers(0, 1, m_staticsVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetInputLayout(m_inputLayout.Get());
		m_context->IASetIndexBuffer(m_staticsIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		// Bind shaders
		m_context->VSSetShader(m_vsStatics.Get(), NULL, 0);
		m_context->PSSetShader(m_psStatics.Get(), NULL, 0);

		Vector3 cameraPosition = Vector3(Camera.pos.x, Camera.pos.y, Camera.pos.z);

		for (auto room : view.roomsToDraw)
		{
			for (auto msh : room->StaticsToDraw)
			{
				if (!m_staticObjects[msh->staticNumber])
					continue;

				Matrix world = (Matrix::CreateFromYawPitchRoll(TO_RAD(msh->pos.yRot), TO_RAD(msh->pos.xRot),
				                                               TO_RAD(msh->pos.zRot)) * Matrix::CreateTranslation(
					msh->pos.xPos, msh->pos.yPos, msh->pos.zPos));
				m_stStatic.World = world;
				m_stStatic.Position = Vector4(msh->pos.xPos, msh->pos.yPos, msh->pos.zPos, 1);
				m_stStatic.Color = msh->color;
				m_cbStatic.updateData(m_stStatic, m_context.Get());
				m_context->VSSetConstantBuffers(1, 1, m_cbStatic.get());

				RendererObject& staticObj = *m_staticObjects[msh->staticNumber];

				if (staticObj.ObjectMeshes.size() > 0)
				{
					RendererMesh* mesh = staticObj.ObjectMeshes[0];

					for (auto& bucket : mesh->buckets)
					{
						if (bucket.NumVertices == 0)
							continue;

						if (isSortingRequired(bucket.BlendMode))
						{
							// Collect transparent faces
							for (int j = 0; j < bucket.Polygons.size(); j++)
							{
								RendererPolygon* p = &bucket.Polygons[j];

								// As polygon distance, for moveables, we use the averaged distance
								Vector3 centre = Vector3::Transform(p->centre, world);
								int distance = (centre - cameraPosition).Length();

								RendererTransparentFace face;
								face.type = RendererTransparentFaceType::TRANSPARENT_FACE_STATIC;
								face.info.polygon = p;
								face.distance = distance;
								face.info.animated = bucket.Animated;
								face.info.texture = bucket.Texture;
								face.info.room = room;
								face.info.staticMesh = msh;
								face.info.world = m_stStatic.World;
								face.info.position = Vector3(msh->pos.xPos, msh->pos.yPos, msh->pos.zPos);
								face.info.color = Vector4(msh->color.x, msh->color.y, msh->color.z, 1.0f);
								face.info.blendMode = bucket.BlendMode;
								face.info.bucket = &bucket;
								room->TransparentFacesToDraw.push_back(face);
							}
						}
						else
						{
							int passes = bucket.BlendMode == BLENDMODE_ALPHATEST ? 2 : 1;

							for (int pass = 0; pass < passes; pass++)
							{
								if (pass == 0)
								{
									m_stMisc.AlphaTest = bucket.BlendMode == BLENDMODE_ALPHATEST;
									m_cbMisc.updateData(m_stMisc, m_context.Get());
									m_context->PSSetConstantBuffers(3, 1, m_cbMisc.get());

									setBlendMode(bucket.BlendMode);
								}
								else
								{
									m_stMisc.AlphaTest = false;
									m_cbMisc.updateData(m_stMisc, m_context.Get());
									m_context->PSSetConstantBuffers(3, 1, m_cbMisc.get());

									setBlendMode(BLENDMODE_ALPHABLEND);
								}

								BindTexture(TextureRegister::MainTexture,
								            &std::get<0>(m_staticsTextures[bucket.Texture]),
								            SamplerStateType::AnisotropicClamp);
								BindTexture(TextureRegister::NormalMapTexture,
								            &std::get<1>(m_staticsTextures[bucket.Texture]), SamplerStateType::None);

								m_context->DrawIndexed(bucket.NumIndices, bucket.StartIndex, 0);
								m_numDrawCalls++;
							}
						}
					}
				}
			}
		}
	}

	void Renderer11::DrawRooms(RenderView& view)
	{
		UINT stride = sizeof(RendererVertex);
		UINT offset = 0;

		// Bind vertex and index buffer
		m_context->IASetVertexBuffers(0, 1, m_roomsVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetInputLayout(m_inputLayout.Get());
		m_context->IASetIndexBuffer(m_roomsIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		// Bind pixel shaders
		m_context->PSSetShader(m_psRooms.Get(), NULL, 0);

		// Bind caustics and shadow map textures
		int nmeshes = -Objects[ID_CAUSTICS_TEXTURES].nmeshes;
		int meshIndex = Objects[ID_CAUSTICS_TEXTURES].meshIndex;
		int causticsFrame = nmeshes ? meshIndex + ((GlobalCounter) % nmeshes) : 0;

		BindTexture(TextureRegister::CausticsTexture, m_sprites[causticsFrame].Texture, SamplerStateType::None);
		BindTexture(TextureRegister::ShadowMapTexture, &m_shadowMap, SamplerStateType::ShadowMap);

		// Set shadow map data
		if (shadowLight != NULL)
		{
			memcpy(&m_stShadowMap.Light, shadowLight, sizeof(ShaderLight));
			m_stShadowMap.CastShadows = true;
		}
		else
		{
			m_stShadowMap.CastShadows = false;
		}
		m_cbShadowMap.updateData(m_stShadowMap, m_context.Get());
		m_context->VSSetConstantBuffers(4, 1, m_cbShadowMap.get());
		m_context->PSSetConstantBuffers(4, 1, m_cbShadowMap.get());

		Vector3 cameraPosition = Vector3(Camera.pos.x, Camera.pos.y, Camera.pos.z);

		numRoomsTransparentPolygons = 0;

		//for (int i = 0; i < view.roomsToDraw.size(); i++)
		for (int i = 0; i < view.roomsToDraw.size(); i++)
		{
			int index = i;
			RendererRoom* room = view.roomsToDraw[index];
			ROOM_INFO* nativeRoom = &g_Level.Rooms[room->RoomNumber];

			Vector3 roomPosition = Vector3(nativeRoom->x, nativeRoom->y, nativeRoom->z);

			m_stLights.NumLights = view.lightsToDraw.size();
			for (int j = 0; j < view.lightsToDraw.size(); j++)
				memcpy(&m_stLights.Lights[j], view.lightsToDraw[j], sizeof(ShaderLight));
			m_cbLights.updateData(m_stLights, m_context.Get());
			m_context->PSSetConstantBuffers(1, 1, m_cbLights.get());

			m_stMisc.Caustics = (nativeRoom->flags & ENV_FLAG_WATER);
			m_cbMisc.updateData(m_stMisc, m_context.Get());
			m_context->PSSetConstantBuffers(3, 1, m_cbMisc.get());

			m_stRoom.AmbientColor = room->AmbientLight;
			m_stRoom.Water = (nativeRoom->flags & ENV_FLAG_WATER) != 0 ? 1 : 0;
			m_cbRoom.updateData(m_stRoom, m_context.Get());

			m_context->VSSetConstantBuffers(5, 1, m_cbRoom.get());
			m_context->PSSetConstantBuffers(5, 1, m_cbRoom.get());

			for (int animated = 0; animated < 2; animated++)
			{
				if (animated == 0)
				{
					m_context->VSSetShader(m_vsRooms.Get(), nullptr, 0);
				}
				else
				{
					m_context->VSSetConstantBuffers(6, 1, m_cbAnimated.get());
					m_context->VSSetShader(m_vsRooms_Anim.Get(), nullptr, 0);
				}

				for (auto& bucket : room->Buckets)
				{
					if ((animated == 1) ^ bucket.Animated || bucket.NumVertices == 0)
						continue;

					if (isSortingRequired(bucket.BlendMode))
					{
						// Collect transparent faces
						for (int j = 0; j < bucket.Polygons.size(); j++)
						{
							RendererPolygon* p = &bucket.Polygons[j];

							// Don't queue back-face polygons
							Vector3 cameraVector = (roomPosition + p->centre) - cameraPosition;
							cameraVector.Normalize();

							numRoomsTransparentPolygons++;

							// As polygon distance, for rooms, we use the farthest vertex distance                            
							int d1 = (roomsVertices[roomsIndices[p->baseIndex + 0]].Position - cameraPosition).Length();
							int d2 = (roomsVertices[roomsIndices[p->baseIndex + 1]].Position - cameraPosition).Length();
							int d3 = (roomsVertices[roomsIndices[p->baseIndex + 2]].Position - cameraPosition).Length();
							int d4 = 0;
							if (p->shape == 0)
								d4 = (roomsVertices[roomsIndices[p->baseIndex + 3]].Position - cameraPosition).Length();

							int distance = std::max(std::max(std::max(d1, d2), d3), d4);

							RendererTransparentFace face;
							face.type = RendererTransparentFaceType::TRANSPARENT_FACE_ROOM;
							face.info.polygon = p;
							face.distance = distance;
							face.info.animated = bucket.Animated;
							face.info.texture = bucket.Texture;
							face.info.room = room;
							face.info.blendMode = bucket.BlendMode;
							face.info.bucket = &bucket;
							room->TransparentFacesToDraw.push_back(face);
						}
					}
					else
					{
						int passes = bucket.BlendMode == BLENDMODE_ALPHATEST ? 2 : 1;

						for (int pass = 0; pass < passes; pass++)
						{
							if (pass == 0)
							{
								m_stMisc.AlphaTest = bucket.BlendMode == BLENDMODE_ALPHATEST;
								m_cbMisc.updateData(m_stMisc, m_context.Get());
								m_context->PSSetConstantBuffers(3, 1, m_cbMisc.get());

								setBlendMode(bucket.BlendMode);
							}
							else
							{
								m_stMisc.AlphaTest = false;
								m_cbMisc.updateData(m_stMisc, m_context.Get());
								m_context->PSSetConstantBuffers(3, 1, m_cbMisc.get());

								setBlendMode(BLENDMODE_ALPHABLEND);
							}

							// Draw geometry
							if (animated)
							{
								BindTexture(TextureRegister::MainTexture,
								            &std::get<0>(m_animatedTextures[bucket.Texture]),
								            SamplerStateType::AnisotropicClamp);
								BindTexture(TextureRegister::NormalMapTexture,
								            &std::get<1>(m_animatedTextures[bucket.Texture]), SamplerStateType::None);

								RendererAnimatedTextureSet& set = m_animatedTextureSets[bucket.Texture];
								m_stAnimated.NumFrames = set.NumTextures;
								for (unsigned char i = 0; i < set.NumTextures; i++)
								{
									auto& tex = set.Textures[i];
									m_stAnimated.Textures[i].topLeft = set.Textures[i].UV[0];
									m_stAnimated.Textures[i].topRight = set.Textures[i].UV[1];
									m_stAnimated.Textures[i].bottomRight = set.Textures[i].UV[2];
									m_stAnimated.Textures[i].bottomLeft = set.Textures[i].UV[3];
								}
								m_cbAnimated.updateData(m_stAnimated, m_context.Get());
							}
							else
							{
								BindTexture(TextureRegister::MainTexture, &std::get<0>(m_roomTextures[bucket.Texture]),
								            SamplerStateType::AnisotropicClamp);
								BindTexture(TextureRegister::NormalMapTexture,
								            &std::get<1>(m_roomTextures[bucket.Texture]), SamplerStateType::None);
							}

							m_context->DrawIndexed(bucket.NumIndices, bucket.StartIndex, 0);
							m_numDrawCalls++;
						}
					}
				}
			}
		}
	}
	
	void Renderer11::DrawHorizonAndSky(RenderView& renderView, ID3D11DepthStencilView* depthTarget)
	{
		GameScriptLevel* level = g_GameFlow->GetLevel(CurrentLevel);

		bool anyOutsideRooms = false;
		for (int k = 0; k < renderView.roomsToDraw.size(); k++)
		{
			ROOM_INFO* nativeRoom = &g_Level.Rooms[renderView.roomsToDraw[k]->RoomNumber];
			if (nativeRoom->flags & ENV_FLAG_OUTSIDE)
			{
				anyOutsideRooms = true;
				break;
			}
		}

		if (!level->Horizon || !anyOutsideRooms)
			return;

		if (BinocularRange)
			AlterFOV(14560 - BinocularRange);

		ID3D11SamplerState* sampler;
		UINT stride = sizeof(RendererVertex);
		UINT offset = 0;

		// Draw the sky
		Matrix rotation = Matrix::CreateRotationX(PI);

		RendererVertex vertices[4];
		float size = 9728.0f;

		vertices[0].Position.x = -size / 2.0f;
		vertices[0].Position.y = 0.0f;
		vertices[0].Position.z = size / 2.0f;
		vertices[0].UV.x = 0.0f;
		vertices[0].UV.y = 0.0f;
		vertices[0].Color.x = 1.0f;
		vertices[0].Color.y = 1.0f;
		vertices[0].Color.z = 1.0f;
		vertices[0].Color.w = 1.0f;

		vertices[1].Position.x = size / 2.0f;
		vertices[1].Position.y = 0.0f;
		vertices[1].Position.z = size / 2.0f;
		vertices[1].UV.x = 1.0f;
		vertices[1].UV.y = 0.0f;
		vertices[1].Color.x = 1.0f;
		vertices[1].Color.y = 1.0f;
		vertices[1].Color.z = 1.0f;
		vertices[1].Color.w = 1.0f;

		vertices[2].Position.x = size / 2.0f;
		vertices[2].Position.y = 0.0f;
		vertices[2].Position.z = -size / 2.0f;
		vertices[2].UV.x = 1.0f;
		vertices[2].UV.y = 1.0f;
		vertices[2].Color.x = 1.0f;
		vertices[2].Color.y = 1.0f;
		vertices[2].Color.z = 1.0f;
		vertices[2].Color.w = 1.0f;

		vertices[3].Position.x = -size / 2.0f;
		vertices[3].Position.y = 0.0f;
		vertices[3].Position.z = -size / 2.0f;
		vertices[3].UV.x = 0.0f;
		vertices[3].UV.y = 1.0f;
		vertices[3].Color.x = 1.0f;
		vertices[3].Color.y = 1.0f;
		vertices[3].Color.z = 1.0f;
		vertices[3].Color.w = 1.0f;

		m_context->VSSetShader(m_vsSky.Get(), NULL, 0);
		m_context->PSSetShader(m_psSky.Get(), NULL, 0);

		m_stMisc.AlphaTest = true;
		m_cbMisc.updateData(m_stMisc, m_context.Get());
		m_context->PSSetConstantBuffers(3, 1, m_cbMisc.get());

		BindTexture(TextureRegister::MainTexture, &m_skyTexture, SamplerStateType::AnisotropicClamp);

		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetInputLayout(m_inputLayout.Get());

		setBlendMode(BLEND_MODES::BLENDMODE_ADDITIVE);

		for (int i = 0; i < 2; i++)
		{
			auto weather = TEN::Effects::Environment::Weather;

			Matrix translation = Matrix::CreateTranslation(Camera.pos.x + weather.SkyLayer1Position() - i * 9728.0f,
			                                               Camera.pos.y - 1536.0f, Camera.pos.z);
			Matrix world = rotation * translation;

			m_stStatic.World = (rotation * translation);
			m_stStatic.Color = weather.SkyColor();
			m_cbStatic.updateData(m_stStatic, m_context.Get());
			m_context->VSSetConstantBuffers(1, 1, m_cbStatic.get());
			m_context->PSSetConstantBuffers(1, 1, m_cbStatic.get());

			m_primitiveBatch->Begin();
			m_primitiveBatch->DrawQuad(vertices[0], vertices[1], vertices[2], vertices[3]);
			m_primitiveBatch->End();
		}
		m_context->ClearDepthStencilView(depthTarget, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);

		// Draw horizon
		if (m_moveableObjects[ID_HORIZON].has_value())
		{
			m_context->IASetVertexBuffers(0, 1, m_moveablesVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);
			m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			m_context->IASetInputLayout(m_inputLayout.Get());
			m_context->IASetIndexBuffer(m_moveablesIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

			RendererObject& moveableObj = *m_moveableObjects[ID_HORIZON];

			m_stStatic.World = Matrix::CreateTranslation(Camera.pos.x, Camera.pos.y, Camera.pos.z);
			m_stStatic.Position = Vector4::Zero;
			m_stStatic.Color = Vector4::One;
			m_cbStatic.updateData(m_stStatic, m_context.Get());
			m_context->VSSetConstantBuffers(1, 1, m_cbStatic.get());
			m_context->PSSetConstantBuffers(1, 1, m_cbStatic.get());

			m_stMisc.AlphaTest = true;
			m_cbMisc.updateData(m_stMisc, m_context.Get());
			m_context->PSSetConstantBuffers(3, 1, m_cbMisc.get());

			for (int k = 0; k < moveableObj.ObjectMeshes.size(); k++)
			{
				RendererMesh* mesh = moveableObj.ObjectMeshes[k];

				for (auto& bucket : mesh->buckets)
				{
					if (bucket.NumVertices == 0)
						continue;

					BindTexture(TextureRegister::MainTexture, &std::get<0>(m_moveablesTextures[bucket.Texture]),
					            SamplerStateType::AnisotropicClamp);
					BindTexture(TextureRegister::NormalMapTexture, &std::get<1>(m_moveablesTextures[bucket.Texture]),
					            SamplerStateType::None);

					setBlendMode(bucket.BlendMode);

					// Draw vertices
					m_context->DrawIndexed(bucket.NumIndices, bucket.StartIndex, 0);
					m_numDrawCalls++;
				}
			}
		}

		// Clear just the Z-buffer so we can start drawing on top of the horizon
		m_context->ClearDepthStencilView(depthTarget, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	}

	void Renderer11::Draw()
	{
		//renderToCubemap(m_reflectionCubemap, Vector3(LaraItem->pos.xPos, LaraItem->pos.yPos - 1024, LaraItem->pos.zPos), LaraItem->roomNumber);
		renderScene(m_backBufferRTV, m_depthStencilView, gameCamera);
		m_context->ClearState();
		m_swapChain->Present(1, 0);
	}

	void Renderer11::drawMoveableMesh(RendererItem* itemToDraw, RendererMesh* mesh, RendererRoom* room, int boneIndex)
	{
		Vector3 cameraPosition = Vector3(Camera.pos.x, Camera.pos.y, Camera.pos.z);

		for (auto& bucket : mesh->buckets)
		{
			if (bucket.NumVertices == 0)
				continue;

			if (isSortingRequired(bucket.BlendMode))
			{
				// Collect transparent faces
				for (int j = 0; j < bucket.Polygons.size(); j++)
				{
					RendererPolygon* p = &bucket.Polygons[j];

					// As polygon distance, for moveables, we use the averaged distance
					Vector3 centre = Vector3::Transform(
						p->centre, itemToDraw->AnimationTransforms[boneIndex] * itemToDraw->World);
					int distance = (centre - cameraPosition).Length();

					RendererTransparentFace face;
					face.type = RendererTransparentFaceType::TRANSPARENT_FACE_MOVEABLE;
					face.info.polygon = p;
					face.distance = distance;
					face.info.animated = bucket.Animated;
					face.info.texture = bucket.Texture;
					face.info.room = room;
					face.info.item = itemToDraw;
					face.info.blendMode = bucket.BlendMode;
					face.info.bucket = &bucket;
					room->TransparentFacesToDraw.push_back(face);
				}
			}
			else
			{
				int passes = bucket.BlendMode == BLENDMODE_ALPHATEST ? 2 : 1;

				BindTexture(TextureRegister::MainTexture, &std::get<0>(m_moveablesTextures[bucket.Texture]),
				            SamplerStateType::AnisotropicClamp);
				BindTexture(TextureRegister::NormalMapTexture, &std::get<1>(m_moveablesTextures[bucket.Texture]),
				            SamplerStateType::None);

				for (int pass = 0; pass < passes; pass++)
				{
					if (pass == 0)
					{
						m_stMisc.AlphaTest = bucket.BlendMode == BLENDMODE_ALPHATEST;
						m_cbMisc.updateData(m_stMisc, m_context.Get());
						m_context->PSSetConstantBuffers(3, 1, m_cbMisc.get());

						setBlendMode(bucket.BlendMode);
					}
					else
					{
						m_stMisc.AlphaTest = false;
						m_cbMisc.updateData(m_stMisc, m_context.Get());
						m_context->PSSetConstantBuffers(3, 1, m_cbMisc.get());

						setBlendMode(BLENDMODE_ALPHABLEND);
					}

					m_context->DrawIndexed(bucket.NumIndices, bucket.StartIndex, 0);
					m_numDrawCalls++;
				}
			}
		}
	}
}
