#include "framework.h"
#include "GameScriptAudioTrack.h"

GameScriptAudioTrack::GameScriptAudioTrack(std::string const & trackName, bool looped)
{
	this->trackName = trackName;
	this->looped = looped;
}

void GameScriptAudioTrack::Register(sol::state* lua)
{
	lua->new_usertype<GameScriptAudioTrack>("AudioTrack",
		sol::constructors<GameScriptAudioTrack(std::string, bool)>(),
		"trackName", &GameScriptAudioTrack::trackName,
		"looped", &GameScriptAudioTrack::looped
		);
}
