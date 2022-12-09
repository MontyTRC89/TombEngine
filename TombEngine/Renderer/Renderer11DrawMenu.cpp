#include "framework.h"
#include "Renderer/Renderer11.h"

#include "Game/animation.h"
#include "Game/control/control.h"
#include "Game/control/volume.h"
#include "Game/Gui.h"
#include "Game/health.h"
#include "Game/Lara/lara.h"
#include "Game/savegame.h"
#include "Math/Math.h"
#include "Scripting/Internal/TEN/Flow//Level/FlowLevel.h"
#include "Specific/configuration.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "Specific/trutils.h"
#include "Specific/winmain.h"

using namespace TEN::Input;
using namespace TEN::Math;

extern TEN::Renderer::RendererHUDBar* g_SFXVolumeBar;
extern TEN::Renderer::RendererHUDBar* g_MusicVolumeBar;

namespace TEN::Renderer
{
	// Horizontal alignment constants
	constexpr auto MenuLeftSideEntry = 200;
	constexpr auto MenuCenterEntry = 400;
	constexpr auto MenuRightSideEntry = 500;

	constexpr auto MenuLoadNumberLeftSide = 80;
	constexpr auto MenuLoadNameLeftSide   = 150;

	// Vertical spacing templates
	constexpr auto MenuVerticalLineSpacing = 30;
	constexpr auto MenuVerticalNarrowLineSpacing = 25;
	constexpr auto MenuVerticalBlockSpacing = 50;
	
	// Vertical menu positioning templates
	constexpr auto MenuVerticalTop = 15;
	constexpr auto MenuVerticalDisplaySettings = 200;
	constexpr auto MenuVerticalOtherSettings = 150;
	constexpr auto MenuVerticalBottomCenter = 400;
	constexpr auto MenuVerticalStatisticsTitle = 150;
	constexpr auto MenuVerticalOptionsTitle = 350;
	constexpr auto MenuVerticalPause = 220;
	constexpr auto MenuVerticalOptionsPause = 275;

	// Title logo positioning
	constexpr auto LogoTop = 50;
	constexpr auto LogoWidth = 300;
	constexpr auto LogoHeight = 150;

	// Used with distance travelled
	constexpr auto UnitsToMeters = 419;

	// Helper functions to jump caret to new line
	inline void GetNextLinePosition(int* value) { *value += MenuVerticalLineSpacing; }
	inline void GetNextNarrowLinePosition(int* value) { *value += MenuVerticalNarrowLineSpacing; }
	inline void GetNextBlockPosition(int* value) { *value += MenuVerticalBlockSpacing; }

	// Helper functions to construct string flags
	inline int SF(bool selected = false) { return PRINTSTRING_OUTLINE | (selected ? PRINTSTRING_BLINK : 0); }
	inline int SF_Center(bool selected = false) { return PRINTSTRING_OUTLINE | PRINTSTRING_CENTER | (selected ? PRINTSTRING_BLINK : 0); }
	
	// Helper functions to get specific generic strings
	inline const char* Str_Enabled(bool enabled = false) { return g_GameFlow->GetString(enabled ? STRING_ENABLED : STRING_DISABLED); }
	inline const char* Str_LoadSave(bool save = false) { return g_GameFlow->GetString(save ? STRING_SAVE_GAME : STRING_LOAD_GAME); }

	// These bars are only used in menus
	TEN::Renderer::RendererHUDBar* g_MusicVolumeBar = nullptr;
	TEN::Renderer::RendererHUDBar* g_SFXVolumeBar = nullptr;

	void Renderer11::InitialiseMenuBars(int y)
	{
		std::array<Vector4, 5> soundSettingColors =
		{
			//top
			Vector4(0.18f,0.3f,0.72f,1),
			Vector4(0.18f,0.3f,0.72f,1),
			//center
			Vector4(0.18f,0.3f,0.72f,1),
			//bottom
			Vector4(0.18f,0.3f,0.72f,1),
			Vector4(0.18f,0.3f,0.72f,1),
		};

		int shift = MenuVerticalLineSpacing / 2;

		g_MusicVolumeBar = new RendererHUDBar(m_device.Get(), MenuRightSideEntry, y + shift, 150, 8, 1, soundSettingColors);
		GetNextLinePosition(&y);
		g_SFXVolumeBar = new RendererHUDBar(m_device.Get(), MenuRightSideEntry, y + shift, 150, 8, 1, soundSettingColors);
	}

	void Renderer11::RenderOptionsMenu(Menu menu, int initialY)
	{
		int y = 0;
		auto title_option = g_Gui.GetSelectedOption();

		char stringBuffer[32] = {};
		auto screenResolution = g_Configuration.SupportedScreenResolutions[g_Gui.GetCurrentSettings().SelectedScreenResolution];
		sprintf(stringBuffer, "%d x %d", screenResolution.x, screenResolution.y);

		auto* shadowMode = g_Gui.GetCurrentSettings().Configuration.ShadowType != ShadowMode::None ?
			(g_Gui.GetCurrentSettings().Configuration.ShadowType == ShadowMode::Lara ? STRING_SHADOWS_PLAYER : STRING_SHADOWS_ALL) : STRING_SHADOWS_NONE;

		const char* antialiasMode;
		switch (g_Gui.GetCurrentSettings().Configuration.Antialiasing)
		{
			default:
			case AntialiasingMode::None:
				antialiasMode = STRING_ANTIALIASING_NONE;
				break;

			case AntialiasingMode::Low:
				antialiasMode = STRING_ANTIALIASING_LOW;
				break;

			case AntialiasingMode::Medium:
				antialiasMode = STRING_ANTIALIASING_MEDIUM;
				break;

			case AntialiasingMode::High:
				antialiasMode = STRING_ANTIALIASING_HIGH;
				break;
		}

		switch (menu)
		{
		case Menu::Options:

			// Setup needed parameters
			y = initialY;

			// Display
			AddString(MenuCenterEntry, y, g_GameFlow->GetString(STRING_DISPLAY), PRINTSTRING_COLOR_WHITE, SF_Center(title_option == 0));
			GetNextLinePosition(&y);

			// Other options
			AddString(MenuCenterEntry, y, g_GameFlow->GetString(STRING_OTHER_SETTINGS), PRINTSTRING_COLOR_WHITE, SF_Center(title_option == 1));
			GetNextLinePosition(&y);

			// Controls
			AddString(MenuCenterEntry, y, g_GameFlow->GetString(STRING_CONTROLS), PRINTSTRING_COLOR_WHITE, SF_Center(title_option == 2));
			break;

		case Menu::Display:

			// Setup needed parameters
			y = MenuVerticalDisplaySettings;

			// Title
			AddString(MenuCenterEntry, y, g_GameFlow->GetString(STRING_DISPLAY), PRINTSTRING_COLOR_YELLOW, SF_Center());
			GetNextBlockPosition(&y);

			// Screen resolution
			AddString(MenuLeftSideEntry, y, g_GameFlow->GetString(STRING_SCREEN_RESOLUTION), PRINTSTRING_COLOR_ORANGE, SF(title_option == 0));
			AddString(MenuRightSideEntry, y, stringBuffer, PRINTSTRING_COLOR_WHITE, SF(title_option == 0));
			GetNextLinePosition(&y);

			// Windowed mode
			AddString(MenuLeftSideEntry, y, g_GameFlow->GetString(STRING_WINDOWED), PRINTSTRING_COLOR_ORANGE, SF(title_option == 1));
			AddString(MenuRightSideEntry, y, Str_Enabled(g_Gui.GetCurrentSettings().Configuration.Windowed), PRINTSTRING_COLOR_WHITE, SF(title_option == 1));
			GetNextLinePosition(&y);

			// Enable dynamic shadows
			AddString(MenuLeftSideEntry, y, g_GameFlow->GetString(STRING_SHADOWS), PRINTSTRING_COLOR_ORANGE, SF(title_option == 2));
			AddString(MenuRightSideEntry, y, g_GameFlow->GetString(shadowMode), PRINTSTRING_COLOR_WHITE, SF(title_option == 2));
			GetNextLinePosition(&y);

			// Enable caustics
			AddString(MenuLeftSideEntry, y, g_GameFlow->GetString(STRING_CAUSTICS), PRINTSTRING_COLOR_ORANGE, SF(title_option == 3));
			AddString(MenuRightSideEntry, y, Str_Enabled(g_Gui.GetCurrentSettings().Configuration.EnableCaustics), PRINTSTRING_COLOR_WHITE, SF(title_option == 3));
			GetNextLinePosition(&y);

			// Enable antialiasing
			AddString(MenuLeftSideEntry, y, g_GameFlow->GetString(STRING_ANTIALIASING), PRINTSTRING_COLOR_ORANGE, SF(title_option == 4));
			AddString(MenuRightSideEntry, y, g_GameFlow->GetString(antialiasMode), PRINTSTRING_COLOR_WHITE, SF(title_option == 4));
			GetNextBlockPosition(&y);

			// Apply
			AddString(MenuCenterEntry, y, g_GameFlow->GetString(STRING_APPLY), PRINTSTRING_COLOR_ORANGE, SF_Center(title_option == 5));
			GetNextLinePosition(&y);

			// Cancel
			AddString(MenuCenterEntry, y, g_GameFlow->GetString(STRING_CANCEL), PRINTSTRING_COLOR_ORANGE, SF_Center(title_option == 6));
			break;

		case Menu::OtherSettings:

			// Setup needed parameters
			y = MenuVerticalOtherSettings;

			// Title
			AddString(MenuCenterEntry, y, g_GameFlow->GetString(STRING_OTHER_SETTINGS), PRINTSTRING_COLOR_YELLOW, SF_Center());
			GetNextBlockPosition(&y);

			// Enable sound special effects
			AddString(MenuLeftSideEntry, y, g_GameFlow->GetString(STRING_REVERB), PRINTSTRING_COLOR_ORANGE, SF(title_option == 0));
			AddString(MenuRightSideEntry, y, Str_Enabled(g_Gui.GetCurrentSettings().Configuration.EnableReverb), PRINTSTRING_COLOR_WHITE, SF(title_option == 0));
			GetNextLinePosition(&y);

			// Initialise bars, if not yet done. Must be done here because we're calculating Y coord on the fly.
			if (g_MusicVolumeBar == nullptr)
				InitialiseMenuBars(y);

			// Music volume
			AddString(MenuLeftSideEntry, y, g_GameFlow->GetString(STRING_MUSIC_VOLUME), PRINTSTRING_COLOR_ORANGE, SF(title_option == 1));
			DrawBar(g_Gui.GetCurrentSettings().Configuration.MusicVolume / 100.0f, g_MusicVolumeBar, ID_SFX_BAR_TEXTURE, 0, false);
			GetNextLinePosition(&y);

			// Sound FX volume
			AddString(MenuLeftSideEntry, y, g_GameFlow->GetString(STRING_SFX_VOLUME), PRINTSTRING_COLOR_ORANGE, SF(title_option == 2));
			DrawBar(g_Gui.GetCurrentSettings().Configuration.SfxVolume / 100.0f, g_SFXVolumeBar, ID_SFX_BAR_TEXTURE, 0, false);
			GetNextBlockPosition(&y);


			// Auto targeting
			AddString(MenuLeftSideEntry, y, g_GameFlow->GetString(STRING_AUTO_TARGET), PRINTSTRING_COLOR_ORANGE, SF(title_option == 3));
			AddString(MenuRightSideEntry, y, Str_Enabled(g_Gui.GetCurrentSettings().Configuration.AutoTarget), PRINTSTRING_COLOR_WHITE, SF(title_option == 3));
			GetNextLinePosition(&y);

			// Vibration
			AddString(MenuLeftSideEntry, y, g_GameFlow->GetString(STRING_RUMBLE), PRINTSTRING_COLOR_ORANGE, SF(title_option == 4));
			AddString(MenuRightSideEntry, y, Str_Enabled(g_Gui.GetCurrentSettings().Configuration.EnableRumble), PRINTSTRING_COLOR_WHITE, SF(title_option == 4));
			GetNextLinePosition(&y);

			// Thumbstick camera
			AddString(MenuLeftSideEntry, y, g_GameFlow->GetString(STRING_THUMBSTICK_CAMERA), PRINTSTRING_COLOR_ORANGE, SF(title_option == 5));
			AddString(MenuRightSideEntry, y, Str_Enabled(g_Gui.GetCurrentSettings().Configuration.EnableThumbstickCameraControl), PRINTSTRING_COLOR_WHITE, SF(title_option == 5));
			GetNextBlockPosition(&y);


			// Apply
			AddString(MenuCenterEntry, y, g_GameFlow->GetString(STRING_APPLY), PRINTSTRING_COLOR_ORANGE, SF_Center(title_option == 6));
			GetNextLinePosition(&y);

			// Cancel
			AddString(MenuCenterEntry, y, g_GameFlow->GetString(STRING_CANCEL), PRINTSTRING_COLOR_ORANGE, SF_Center(title_option == 7));
			break;

		case Menu::Controls:

			// Setup needed parameters
			y = MenuVerticalTop;

			// Title
			AddString(MenuCenterEntry, y, g_GameFlow->GetString(STRING_CONTROLS), PRINTSTRING_COLOR_YELLOW, SF_Center());
			GetNextBlockPosition(&y);

			// Control listing
			for (int k = 0; k < KEY_COUNT; k++)
			{
				AddString(MenuLeftSideEntry, y, g_GameFlow->GetString(ControlStrings[k]), PRINTSTRING_COLOR_WHITE, SF(title_option == k));

				if (g_Gui.GetCurrentSettings().WaitingForKey && title_option == k)
					AddString(MenuRightSideEntry, y, g_GameFlow->GetString(STRING_WAITING_FOR_INPUT), PRINTSTRING_COLOR_YELLOW, SF(true));
				else
				{
					int index = KeyboardLayout[1][k] ? KeyboardLayout[1][k] : KeyboardLayout[0][k];
					AddString(MenuRightSideEntry, y, (char*)g_KeyNames[index], PRINTSTRING_COLOR_ORANGE, SF(false));
				}

				if (k < KEY_COUNT - 1)
					GetNextNarrowLinePosition(&y);
				else
					GetNextBlockPosition(&y);
			}

			// Apply
			AddString(MenuCenterEntry, y, g_GameFlow->GetString(STRING_APPLY), PRINTSTRING_COLOR_ORANGE, SF_Center(title_option == 17));
			GetNextLinePosition(&y);

			// Cancel
			AddString(MenuCenterEntry, y, g_GameFlow->GetString(STRING_CANCEL), PRINTSTRING_COLOR_ORANGE, SF_Center(title_option == 18));
			break;
		}
	}

	void Renderer11::RenderTitleMenu(Menu menu)
	{
		int y = MenuVerticalBottomCenter;
		auto title_option = g_Gui.GetSelectedOption();

		// HACK: Check if it works properly -- Lwmte, 07.06.22
		if (menu == Menu::LoadGame && !g_GameFlow->EnableLoadSave)
			menu = Menu::Title;

		switch (menu)
		{
		case Menu::Title:

			// New game
			AddString(MenuCenterEntry, y, g_GameFlow->GetString(STRING_NEW_GAME), PRINTSTRING_COLOR_WHITE, SF_Center(title_option == 0));
			GetNextLinePosition(&y);

			// Load game
			AddString(MenuCenterEntry, y, g_GameFlow->GetString(STRING_LOAD_GAME), PRINTSTRING_COLOR_WHITE, SF_Center(title_option == 1));
			GetNextLinePosition(&y);

			// Options
			AddString(MenuCenterEntry, y, g_GameFlow->GetString(STRING_OPTIONS), PRINTSTRING_COLOR_WHITE, SF_Center(title_option == 2));
			GetNextLinePosition(&y);

			// Exit game
			AddString(MenuCenterEntry, y, g_GameFlow->GetString(STRING_EXIT_GAME), PRINTSTRING_COLOR_WHITE, SF_Center(title_option == 3));
			break;

		case Menu::LoadGame:
			RenderLoadSaveMenu();
			break;

		case Menu::SelectLevel:

			// Setup needed parameters
			y = MenuVerticalLineSpacing;

			// Title
			AddString(MenuCenterEntry, 26, g_GameFlow->GetString(STRING_SELECT_LEVEL), PRINTSTRING_COLOR_ORANGE, SF_Center());
			GetNextBlockPosition(&y);

			// Level listing (starts with 1 because 0 is always title)
			for (int i = 1; i < g_GameFlow->GetNumLevels(); i++)
			{
				AddString(MenuCenterEntry, y, g_GameFlow->GetString(g_GameFlow->GetLevel(i)->NameStringKey.c_str()),
					PRINTSTRING_COLOR_WHITE, SF_Center(title_option == i - 1));
				GetNextNarrowLinePosition(&y);
			}
			break;

		case Menu::Options:
		case Menu::Controls:
		case Menu::Display:
		case Menu::OtherSettings:
			RenderOptionsMenu(menu, MenuVerticalOptionsTitle);
			break;
		}
	}

	void Renderer11::RenderPauseMenu(Menu menu)
	{
		int y = 0;
		auto pause_option = g_Gui.GetSelectedOption();

		switch (g_Gui.GetMenuToDisplay())
		{
		case Menu::Pause:

			// Setup needed parameters
			y = MenuVerticalPause;

			// Header
			AddString(MenuCenterEntry, y, g_GameFlow->GetString(STRING_CONTROLS_PAUSE), PRINTSTRING_COLOR_ORANGE, SF_Center());
			GetNextBlockPosition(&y);

			// Statistics
			AddString(MenuCenterEntry, y, g_GameFlow->GetString(STRING_STATISTICS), PRINTSTRING_COLOR_WHITE, SF_Center(pause_option == 0));
			GetNextLinePosition(&y);

			// Options
			AddString(MenuCenterEntry, y, g_GameFlow->GetString(STRING_OPTIONS), PRINTSTRING_COLOR_WHITE, SF_Center(pause_option == 1));
			GetNextLinePosition(&y);

			// Exit to title
			AddString(MenuCenterEntry, y, g_GameFlow->GetString(STRING_EXIT_TO_TITLE), PRINTSTRING_COLOR_WHITE, SF_Center(pause_option == 2));
			break;

		case Menu::Statistics:
			DrawStatistics();
			break;

		case Menu::Options:
		case Menu::Controls:
		case Menu::Display:
		case Menu::OtherSettings:
			RenderOptionsMenu(menu, MenuVerticalOptionsPause);
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

		// Setup needed parameters
		int y = MenuVerticalLineSpacing;
		short selection = g_Gui.GetLoadSaveSelection();
		char stringBuffer[255];
		LoadSavegameInfos();

		// Title
		AddString(MenuCenterEntry, MenuVerticalNarrowLineSpacing, Str_LoadSave(g_Gui.GetInventoryMode() == InventoryMode::Save), 
			PRINTSTRING_COLOR_ORANGE, SF_Center());
		GetNextBlockPosition(&y);

		// Savegame listing
		for (int n = 0; n < SAVEGAME_MAX; n++)
		{
			auto& save = SavegameInfos[n];

			if (!save.Present)
			{
				AddString(MenuCenterEntry, y, g_GameFlow->GetString(STRING_EMPTY), PRINTSTRING_COLOR_WHITE, SF_Center(selection == n));
			}
			else
			{
				// Number
				sprintf(stringBuffer, "%03d", save.Count);
				AddString(MenuLoadNumberLeftSide, y, stringBuffer, PRINTSTRING_COLOR_WHITE, SF(selection == n));

				// Level name
				AddString(MenuLoadNameLeftSide, y, (char*)save.LevelName.c_str(), PRINTSTRING_COLOR_WHITE, SF(selection == n));

				// Timestamp
				sprintf(stringBuffer, g_GameFlow->GetString(STRING_SAVEGAME_TIMESTAMP), save.Days, save.Hours, save.Minutes, save.Seconds);
				AddString(MenuRightSideEntry, y, stringBuffer, PRINTSTRING_COLOR_WHITE, SF(selection == n));
			}

			GetNextLinePosition(&y);
		}

		DrawAllStrings();
	}

	void Renderer11::DrawStatistics()
	{
		char buffer[40];

		auto seconds = GameTimer / FPS;
		auto days    = (seconds / (24 * 60 * 60));
		auto hours   = (seconds % (24 * 60 * 60)) / (60 * 60);
		auto min     = (seconds / 60) % 60;
		auto sec     = (seconds % 60);

		ScriptInterfaceLevel* lvl = g_GameFlow->GetLevel(CurrentLevel);
		auto y = MenuVerticalStatisticsTitle;

		// Title
		AddString(MenuCenterEntry, y, g_GameFlow->GetString(STRING_STATISTICS), PRINTSTRING_COLOR_ORANGE, SF_Center());
		GetNextBlockPosition(&y);

		// Level name
		AddString(MenuCenterEntry, y, g_GameFlow->GetString(lvl->NameStringKey.c_str()), PRINTSTRING_COLOR_WHITE, SF_Center());
		GetNextBlockPosition(&y);

		// Time taken
		sprintf(buffer, "%02d:%02d:%02d", (days * 24) + hours, min, sec);
		AddString(MenuRightSideEntry, y, buffer, PRINTSTRING_COLOR_WHITE, SF());
		AddString(MenuLeftSideEntry, y, g_GameFlow->GetString(STRING_TIME_TAKEN), PRINTSTRING_COLOR_WHITE, SF());
		GetNextLinePosition(&y);

		// Distance travelled
		sprintf(buffer, "%dm", Statistics.Game.Distance / UnitsToMeters);
		AddString(MenuRightSideEntry, y, buffer, PRINTSTRING_COLOR_WHITE, SF());
		AddString(MenuLeftSideEntry, y, g_GameFlow->GetString(STRING_DISTANCE_TRAVELLED), PRINTSTRING_COLOR_WHITE, SF());
		GetNextLinePosition(&y);

		// Ammo used
		sprintf(buffer, "%d", Statistics.Game.AmmoUsed);
		AddString(MenuRightSideEntry, y, buffer, PRINTSTRING_COLOR_WHITE, SF());
		AddString(MenuLeftSideEntry, y, g_GameFlow->GetString(STRING_AMMO_USED), PRINTSTRING_COLOR_WHITE, SF());
		GetNextLinePosition(&y);

		// Medipacks used
		sprintf(buffer, "%d", Statistics.Game.HealthUsed);
		AddString(MenuRightSideEntry, y, buffer, PRINTSTRING_COLOR_WHITE, SF());
		AddString(MenuLeftSideEntry, y, g_GameFlow->GetString(STRING_USED_MEDIPACKS), PRINTSTRING_COLOR_WHITE, SF());
		GetNextLinePosition(&y);

		// Secrets found in Level
		if (g_GameFlow->GetLevel(CurrentLevel)->GetSecrets() > 0)
		{
			std::bitset<32> levelSecretBitSet(Statistics.Level.Secrets);
			sprintf(buffer, "%d / %d", (int)levelSecretBitSet.count(), g_GameFlow->GetLevel(CurrentLevel)->GetSecrets());
			AddString(MenuRightSideEntry, y, buffer, PRINTSTRING_COLOR_WHITE, SF());
			AddString(MenuLeftSideEntry, y, g_GameFlow->GetString(STRING_LEVEL_SECRETS_FOUND), PRINTSTRING_COLOR_WHITE, SF());
			GetNextLinePosition(&y);
		}

		// Secrets found total
		if (g_GameFlow->TotalNumberOfSecrets > 0)
		{
			sprintf(buffer, "%d / %d", Statistics.Game.Secrets, g_GameFlow->TotalNumberOfSecrets);
			AddString(MenuRightSideEntry, y, buffer, PRINTSTRING_COLOR_WHITE, SF());
			AddString(MenuLeftSideEntry, y, g_GameFlow->GetString(STRING_TOTAL_SECRETS_FOUND), PRINTSTRING_COLOR_WHITE, SF());
		}

		DrawAllStrings();
	}

	void Renderer11::RenderNewInventory()
	{
		g_Gui.DrawCurrentObjectList(LaraItem, (int)RingTypes::Inventory);

		if (g_Gui.GetRings((int)RingTypes::Ammo)->RingActive)
			g_Gui.DrawCurrentObjectList(LaraItem, (int)RingTypes::Ammo);

		g_Gui.DrawAmmoSelector();
		g_Gui.FadeAmmoSelector();
		g_Gui.DrawCompass(LaraItem);

		DrawAllStrings();
	}

	void Renderer11::DrawPickup(short objectNum)
	{
		// Clear just the Z-buffer so we can start drawing on top of the scene
		ID3D11DepthStencilView* dsv;
		m_context->OMGetRenderTargets(1, nullptr, &dsv);
		m_context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		DrawObjectOn2DPosition(700 + PickupX, 450, objectNum, EulerAngles(0, m_pickupRotation, 0), 0.5f); // TODO: + PickupY
		m_pickupRotation += 45 * 360 / 30;
	}

	void Renderer11::DrawObjectOn2DPosition(short x, short y, short objectNum, EulerAngles orient, float scale1, int meshBits)
	{
		Matrix translation;
		Matrix rotation;
		Matrix world;
		Matrix view;
		Matrix projection;
		Matrix scale;

		UINT stride = sizeof(RendererVertex);
		UINT offset = 0;

		float factorX = m_screenWidth / REFERENCE_RES_WIDTH;
		float factorY = m_screenHeight / REFERENCE_RES_HEIGHT;

		x *= factorX;
		y *= factorY;
		scale1 *= factorX > factorY ? factorY : factorX;

		auto index = g_Gui.ConvertObjectToInventoryItem(objectNum);

		if (index != -1)
		{
			auto& invObject = InventoryObjectTable[index];
			y += invObject.YOffset;
			orient += invObject.Orientation;
		}

		view = Matrix::CreateLookAt(Vector3(0.0f, 0.0f, 2048.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, -1.0f, 0.0f));
		projection = Matrix::CreateOrthographic(m_screenWidth, m_screenHeight, -1024.0f, 1024.0f);

		auto& moveableObj = m_moveableObjects[objectNum];
		if (!moveableObj)
			return;

		auto* obj = &Objects[objectNum];

		if (obj->animIndex != -1)
		{
			AnimFrame* frame[] = { &g_Level.Frames[g_Level.Anims[obj->animIndex].FramePtr] };
			UpdateAnimation(nullptr, *moveableObj, frame, 0, 0, 0xFFFFFFFF);
		}

		auto pos = m_viewportToolkit.Unproject(Vector3(x, y, 1), projection, view, Matrix::Identity);

		// Set vertex buffer.
		m_context->IASetVertexBuffers(0, 1, m_moveablesVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetInputLayout(m_inputLayout.Get());
		m_context->IASetIndexBuffer(m_moveablesIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		// Set shaders.
		m_context->VSSetShader(m_vsInventory.Get(), nullptr, 0);
		m_context->PSSetShader(m_psInventory.Get(), nullptr, 0);

		// Set matrices.
		CCameraMatrixBuffer HudCamera;
		HudCamera.CamDirectionWS = -Vector4::UnitZ;
		HudCamera.ViewProjection = view * projection;
		m_cbCameraMatrices.updateData(HudCamera, m_context.Get());
		BindConstantBufferVS(CB_CAMERA, m_cbCameraMatrices.get());

		for (int n = 0; n < (*moveableObj).ObjectMeshes.size(); n++)
		{
			if (meshBits && !(meshBits & (1 << n)))
				continue;

			auto* mesh = (*moveableObj).ObjectMeshes[n];

			// Finish the world matrix
			translation = Matrix::CreateTranslation(pos.x, pos.y, pos.z + 1024.0f);
			rotation = orient.ToRotationMatrix();
			scale = Matrix::CreateScale(scale1);

			world = scale * rotation;
			world = world * translation;

			if (obj->animIndex != -1)
				m_stItem.World = ((*moveableObj).AnimationTransforms[n] * world);
			else
				m_stItem.World = ((*moveableObj).BindPoseTransforms[n] * world);

			m_stItem.BoneLightModes[n] = LIGHT_MODES::LIGHT_MODE_DYNAMIC;
			m_stItem.Color = Vector4::One;
			m_stItem.AmbientLight = Vector4(0.5f, 0.5f, 0.5f, 1.0f);

			m_cbItem.updateData(m_stItem, m_context.Get());
			BindConstantBufferVS(CB_ITEM, m_cbItem.get());
			BindConstantBufferPS(CB_ITEM, m_cbItem.get());

			for (auto& bucket : mesh->Buckets)
			{
				if (bucket.NumVertices == 0)
					continue;

				SetBlendMode(BLENDMODE_OPAQUE);
				SetCullMode(CULL_MODE_CCW);
				SetDepthState(DEPTH_STATE_WRITE_ZBUFFER);

				BindTexture(TEXTURE_COLOR_MAP, &std::get<0>(m_moveablesTextures[bucket.Texture]), SAMPLER_ANISOTROPIC_CLAMP);
				BindTexture(TEXTURE_NORMAL_MAP, &std::get<1>(m_moveablesTextures[bucket.Texture]), SAMPLER_NONE);

				SetAlphaTest(
					bucket.BlendMode == BLENDMODE_ALPHATEST ? ALPHA_TEST_GREATER_THAN : ALPHA_TEST_NONE,
					ALPHA_TEST_THRESHOLD
				);

				DrawIndexedTriangles(bucket.NumIndices, bucket.StartIndex, 0);

				m_numMoveablesDrawCalls++;
			}
		}
	}

	void Renderer11::RenderTitleImage()
	{
		Texture2D texture;
		SetTextureOrDefault(texture, TEN::Utils::FromChar(g_GameFlow->IntroImagePath.c_str()));

		if (!texture.Texture)
			return;

		int timeout = 20;
		float currentFade = FADE_FACTOR;

		while (timeout || currentFade > 0.0f)
		{
			if (timeout)
			{
				if (currentFade < 1.0f)
					currentFade = std::clamp(currentFade += FADE_FACTOR, 0.0f, 1.0f);
				else
					timeout--;
			}
			else
				currentFade = std::clamp(currentFade -= FADE_FACTOR, 0.0f, 1.0f);

			DrawFullScreenImage(texture.ShaderResourceView.Get(), Smoothstep(currentFade), m_backBufferRTV, m_depthStencilView);
			Synchronize();
			m_swapChain->Present(0, 0);
			m_context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		}
	}

	void Renderer11::DrawExamines()
	{
		static EulerAngles orient = EulerAngles::Zero;
		static float scaler = 1.2f;

		short invItem = g_Gui.GetRings((int)RingTypes::Inventory)->CurrentObjectList[g_Gui.GetRings((int)RingTypes::Inventory)->CurrentObjectInList].InventoryItem;

		auto& object = InventoryObjectTable[invItem];

		if (TrInput & IN_FORWARD)
			orient.x += ANGLE(3.0f);

		if (TrInput & IN_BACK)
			orient.x -= ANGLE(3.0f);

		if (TrInput & IN_LEFT)
			orient.y += ANGLE(3.0f);

		if (TrInput & IN_RIGHT)
			orient.y -= ANGLE(3.0f);

		if (TrInput & IN_SPRINT)
			scaler += 0.03f;

		if (TrInput & IN_CROUCH)
			scaler -= 0.03f;

		if (scaler > 1.6f)
			scaler = 1.6f;

		if (scaler < 0.8f)
			scaler = 0.8f;

		float savedScale = object.Scale1;
		object.Scale1 = scaler;
		DrawObjectOn2DPosition(400, 300, g_Gui.ConvertInventoryItemToObject(invItem), orient, object.Scale1);
		object.Scale1 = savedScale;
	}

	void Renderer11::DrawDiary()
	{
		unsigned int currentPage = Lara.Inventory.Diary.CurrentPage;

		const auto& object = InventoryObjectTable[INV_OBJECT_OPEN_DIARY];

		DrawObjectOn2DPosition(400, 300, g_Gui.ConvertInventoryItemToObject(INV_OBJECT_OPEN_DIARY), object.Orientation, object.Scale1);

		for (size_t i = 0; i < MAX_DIARY_STRINGS_PER_PAGE; i++)
		{
			if (!Lara.Inventory.Diary.Pages[Lara.Inventory.Diary.CurrentPage].Strings[i].Position.x && !Lara.Inventory.Diary.Pages[Lara.Inventory.Diary.CurrentPage].
				Strings[i].Position.y && !Lara.Inventory.Diary.Pages[Lara.Inventory.Diary.CurrentPage].Strings[i].StringID)
				break;

			//AddString(Lara.Diary.Pages[currentPage].Strings[i].x, Lara.Diary.Pages[currentPage].Strings[i].y, g_GameFlow->GetString(Lara.Diary.Pages[currentPage].Strings[i].stringID), PRINTSTRING_COLOR_WHITE, 0);
		}

		DrawAllStrings();
	}

	void Renderer11::RenderInventoryScene(ID3D11RenderTargetView* target, ID3D11DepthStencilView* depthTarget,
		ID3D11ShaderResourceView* background)
	{
		// Set basic render states
		SetBlendMode(BLENDMODE_OPAQUE, true);
		SetDepthState(DEPTH_STATE_WRITE_ZBUFFER, true);
		SetCullMode(CULL_MODE_CCW, true);

		// Bind and clear render target
		m_context->OMSetRenderTargets(1, &target, depthTarget);
		m_context->RSSetViewports(1, &m_viewport);
		ResetScissor();

		if (background != nullptr)
			DrawFullScreenImage(background, 0.5f, target, depthTarget);

		m_context->ClearDepthStencilView(depthTarget, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		UINT stride = sizeof(RendererVertex);
		UINT offset = 0;

		// Set vertex buffer
		m_context->IASetVertexBuffers(0, 1, m_moveablesVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetInputLayout(m_inputLayout.Get());
		m_context->IASetIndexBuffer(m_moveablesIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		// Set shaders
		m_context->VSSetShader(m_vsInventory.Get(), nullptr, 0);
		m_context->PSSetShader(m_psInventory.Get(), nullptr, 0);

		// Set texture
		BindTexture(TEXTURE_COLOR_MAP, &std::get<0>(m_moveablesTextures[0]), SAMPLER_ANISOTROPIC_CLAMP);
		BindTexture(TEXTURE_NORMAL_MAP, &std::get<1>(m_moveablesTextures[0]), SAMPLER_NONE);

		if (CurrentLevel == 0)
		{
			auto titleMenu = g_Gui.GetMenuToDisplay();
			bool drawLogo = (titleMenu == Menu::Title || titleMenu == Menu::Options);

			if (drawLogo)
			{
				float factorX = (float)m_screenWidth / REFERENCE_RES_WIDTH;
				float factorY = (float)m_screenHeight / REFERENCE_RES_HEIGHT;
				float scale = m_screenWidth > m_screenHeight ? factorX : factorY;

				int logoLeft   = (REFERENCE_RES_WIDTH / 2) - (LogoWidth / 2);
				int logoRight  = (REFERENCE_RES_WIDTH / 2) + (LogoWidth / 2);
				int logoBottom = LogoTop + LogoHeight;

				RECT rect;
				rect.left   = logoLeft   * scale;
				rect.right  = logoRight  * scale;
				rect.top    = LogoTop    * scale;
				rect.bottom = logoBottom * scale;

				m_spriteBatch->Begin(SpriteSortMode_BackToFront, m_states->NonPremultiplied());
				m_spriteBatch->Draw(m_logo.ShaderResourceView.Get(), rect, Vector4::One * ScreenFadeCurrent);
				m_spriteBatch->End();
			}

			RenderTitleMenu(titleMenu);
		}
		else
		{
			switch (g_Gui.GetInventoryMode())
			{
			case InventoryMode::Load:
			case InventoryMode::Save:
				RenderLoadSaveMenu();
				break;

			case InventoryMode::InGame:
				RenderNewInventory();
				break;

			case InventoryMode::Statistics:
				DrawStatistics();
				break;

			case InventoryMode::Examine:
				DrawExamines();
				break;

			case InventoryMode::Pause:
				RenderPauseMenu(g_Gui.GetMenuToDisplay());
				break;

			case InventoryMode::Diary:
				DrawDiary();
				break;
			}
		}
	}

	void Renderer11::SetLoadingScreen(std::wstring& fileName)
	{
		SetTextureOrDefault(loadingScreenTexture, fileName);
	}

	void Renderer11::RenderLoadingScreen(float percentage)
	{
		// Set basic render states
		SetBlendMode(BLENDMODE_OPAQUE);
		SetCullMode(CULL_MODE_CCW);

		do
		{
			// Clear screen
			m_context->ClearRenderTargetView(m_backBufferRTV, Colors::Black);
			m_context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

			// Bind the back buffer
			m_context->OMSetRenderTargets(1, &m_backBufferRTV, m_depthStencilView);
			m_context->RSSetViewports(1, &m_viewport);
			ResetScissor();

			// Draw the full screen background
			if (loadingScreenTexture.Texture)
				DrawFullScreenQuad(
					loadingScreenTexture.ShaderResourceView.Get(),
					Vector3(ScreenFadeCurrent, ScreenFadeCurrent, ScreenFadeCurrent));

			if (ScreenFadeCurrent && percentage > 0.0f && percentage < 100.0f)
				DrawLoadingBar(percentage);

			m_swapChain->Present(0, 0);
			m_context->ClearState();

			Synchronize();
			UpdateFadeScreenAndCinematicBars();

		} while (ScreenFading || !ScreenFadedOut);
	}

	void Renderer11::RenderInventory()
	{
		m_context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_STENCIL | D3D11_CLEAR_DEPTH, 1.0f, 0);
		m_context->ClearRenderTargetView(m_backBufferRTV, Colors::Black);
		RenderInventoryScene(m_backBufferRTV, m_depthStencilView, m_dumpScreenRenderTarget.ShaderResourceView.Get());
		m_swapChain->Present(0, 0);
	}

	void Renderer11::RenderTitle()
	{
		m_context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_STENCIL | D3D11_CLEAR_DEPTH, 1.0f, 0);
		m_context->ClearRenderTargetView(m_backBufferRTV, Colors::Black);

		RenderScene(m_backBufferRTV, m_depthStencilView, gameCamera);
		m_context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_STENCIL | D3D11_CLEAR_DEPTH, 1.0f, 0);

		RenderInventoryScene(m_backBufferRTV, m_depthStencilView, nullptr);
#if _DEBUG
		AddString(0, 0, commit.c_str(), D3DCOLOR_ARGB(255, 255, 255, 255), 0);
		DrawAllStrings();
#else
		DrawAllStrings();
#endif
		m_swapChain->Present(0, 0);
	}

	void Renderer11::DrawDebugInfo(RenderView& view)
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
				PrintDebugMessage("GPU: %s", g_Configuration.AdapterName.c_str());
				PrintDebugMessage("Resolution: %d x %d", m_screenWidth, m_screenHeight);
				PrintDebugMessage("Fps: %3.2f", m_fps);
				PrintDebugMessage("Update time: %d", m_timeUpdate);
				PrintDebugMessage("Frame time: %d", m_timeFrame);
				PrintDebugMessage("Total draw calls: %d", m_numDrawCalls);
				PrintDebugMessage("    For rooms: %d", m_numRoomsDrawCalls);
				PrintDebugMessage("    For movables: %d", m_numMoveablesDrawCalls);
				PrintDebugMessage("    For statics: %d", m_numStaticsDrawCalls);
				PrintDebugMessage("    For sprites: %d (%d instanced)", m_numSpritesDrawCalls, m_numInstancedSpritesDrawCalls);
				PrintDebugMessage("Total triangles: %d", m_numPolygons);
				PrintDebugMessage("Total sprites: %d", view.spritesToDraw.size());
				PrintDebugMessage("Transparent faces draw calls: %d", m_numTransparentDrawCalls);
				PrintDebugMessage("    For rooms: %d", m_numRoomsTransparentDrawCalls);
				PrintDebugMessage("    For movables: %d", m_numMoveablesTransparentDrawCalls);
				PrintDebugMessage("    For statics: %d", m_numStaticsTransparentDrawCalls);
				PrintDebugMessage("    For sprites: %d", m_numSpritesTransparentDrawCalls);
				PrintDebugMessage("Biggest room's index buffer: %d", m_biggestRoomIndexBuffer);
				PrintDebugMessage("Total rooms transparent polygons: %d", m_numRoomsTransparentPolygons);
				PrintDebugMessage("Rooms: %d", view.roomsToDraw.size());
				break;

			case RENDERER_DEBUG_PAGE::DIMENSION_STATS:
				PrintDebugMessage("Lara Location: %d %d", LaraItem->Location.roomNumber, LaraItem->Location.yNumber);
				PrintDebugMessage("Lara RoomNumber: %d", LaraItem->RoomNumber);
				PrintDebugMessage("LaraItem BoxNumber: %d",/* canJump: %d, canLongJump: %d, canMonkey: %d,*/
					LaraItem->BoxNumber);
				PrintDebugMessage("Lara Pos: %d %d %d", LaraItem->Pose.Position.x, LaraItem->Pose.Position.y, LaraItem->Pose.Position.z);
				PrintDebugMessage("Lara Rot: %d %d %d", LaraItem->Pose.Orientation.x, LaraItem->Pose.Orientation.y, LaraItem->Pose.Orientation.z);
				PrintDebugMessage("Lara WaterSurfaceDist: %d", Lara.WaterSurfaceDist);
				PrintDebugMessage("Room: %d %d %d %d", r->x, r->z, r->x + r->xSize * SECTOR(1),
					r->z + r->zSize * SECTOR(1));
				PrintDebugMessage("Room.y, minFloor, maxCeiling: %d %d %d ", r->y, r->minfloor, r->maxceiling);
				PrintDebugMessage("Camera.pos: %d %d %d", Camera.pos.x, Camera.pos.y, Camera.pos.z);
				PrintDebugMessage("Camera.target: %d %d %d", Camera.target.x, Camera.target.y, Camera.target.z);
				PrintDebugMessage("Camera.roomNumber: %d", Camera.pos.RoomNumber);
				break;

			case RENDERER_DEBUG_PAGE::LARA_STATS:
				PrintDebugMessage("Lara AnimNumber: %d", LaraItem->Animation.AnimNumber);
				PrintDebugMessage("Lara FrameNumber: %d", LaraItem->Animation.FrameNumber);
				PrintDebugMessage("Lara ActiveState: %d", LaraItem->Animation.ActiveState);
				PrintDebugMessage("Lara TargetState: %d", LaraItem->Animation.TargetState);
				PrintDebugMessage("Lara Velocities: %.3f %.3f %.3f", LaraItem->Animation.Velocity.z, LaraItem->Animation.Velocity.y, LaraItem->Animation.Velocity.x);
				PrintDebugMessage("Lara WaterStatus: %d", Lara.Control.WaterStatus);
				PrintDebugMessage("Lara IsMoving: %d", Lara.Control.IsMoving);
				PrintDebugMessage("Lara HandStatus: %d", Lara.Control.HandStatus);
				PrintDebugMessage("Lara IsAirborne: %d", LaraItem->Animation.IsAirborne);
				PrintDebugMessage("Lara CanClimbLadder: %d", Lara.Control.CanClimbLadder);
				break;

			case RENDERER_DEBUG_PAGE::LOGIC_STATS:
				PrintDebugMessage("Target HitPoints: %d", Lara.TargetEntity ? Lara.TargetEntity->HitPoints : 0);
				PrintDebugMessage("CollidedVolume: %d", TEN::Control::Volumes::CurrentCollidedVolume);
				PrintDebugMessage("Move axis vertical: %f", AxisMap[InputAxis::MoveVertical]);
				PrintDebugMessage("Move axis horizontal: %f", AxisMap[InputAxis::MoveHorizontal]);
				PrintDebugMessage("Look axis vertical: %f", AxisMap[InputAxis::CameraVertical]);
				PrintDebugMessage("Look axis horizontal: %f", AxisMap[InputAxis::CameraHorizontal]);
				break;
			}
		}
	}

	void Renderer11::SwitchDebugPage(bool back)
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
}
