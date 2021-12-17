#pragma once

#include <string>

namespace sol {
	class state;
}

struct GameScriptAudioTrack
{
	std::string trackName;
	bool looped;

	GameScriptAudioTrack(std::string const & trackName, bool looped);
	static void Register(sol::state* lua);
};
