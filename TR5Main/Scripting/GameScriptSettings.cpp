#include "framework.h"
#include "GameScriptSettings.h"

void GameScriptSettings::Register(sol::state* lua)
{
	lua->new_usertype<GameScriptSettings>("GameScriptSettings",
		"screenWidth", &GameScriptSettings::ScreenWidth,
		"screenHeight", &GameScriptSettings::ScreenHeight,
		"windowTitle", &GameScriptSettings::WindowTitle,
		"enableDynamicShadows", &GameScriptSettings::EnableDynamicShadows,
		"windowed", &GameScriptSettings::Windowed,
		"enableWaterCaustics", &GameScriptSettings::EnableWaterCaustics,
		"drawingDistance", &GameScriptSettings::DrawingDistance,
		"showRendererSteps", &GameScriptSettings::ShowRendererSteps,
		"showDebugInfo", &GameScriptSettings::ShowDebugInfo
		);
}
