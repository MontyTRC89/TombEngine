#include "frameworkandsol.h"
#include "GameScriptAudioTrack.h"

/***
Metadata about audio tracks (music and ambience).

__In progress__ 

@pregameclass AudioTrack
@pragma nostrip
*/
// TODO FIXME find out what is meant to happen and whether we need this or not

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
