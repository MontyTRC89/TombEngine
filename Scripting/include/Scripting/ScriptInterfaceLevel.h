#pragma once

enum class WeatherType
{
	None,
	Rain,
	Snow
};

class ScriptInterfaceLevel {
public:
	virtual ~ScriptInterfaceLevel() = default;

	virtual bool GetSkyLayerEnabled(int index) = 0;
	virtual short GetSkyLayerSpeed(int index) = 0;
};
