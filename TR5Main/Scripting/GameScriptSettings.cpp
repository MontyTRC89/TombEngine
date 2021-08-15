#include "framework.h"
#include "GameScriptSettings.h"

void GameScriptSettings::Register(sol::state* lua)
{
	lua->new_usertype<GameScriptSettings>("Settings",
		"screenWidth", &GameScriptSettings::ScreenWidth,
		"screenHeight", &GameScriptSettings::ScreenHeight,
		"enableDynamicShadows", &GameScriptSettings::EnableDynamicShadows,
		"windowed", &GameScriptSettings::Windowed,
		"enableWaterCaustics", &GameScriptSettings::EnableWaterCaustics,
		"drawingDistance", &GameScriptSettings::DrawingDistance,
		"showRendererSteps", &GameScriptSettings::ShowRendererSteps,
		"showDebugInfo", &GameScriptSettings::ShowDebugInfo,
		"errorMode", &GameScriptSettings::ErrorMode
		);
}
