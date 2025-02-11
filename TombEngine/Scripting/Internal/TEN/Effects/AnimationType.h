#pragma once
#include "Game/effects/effects.h"

/***
Constants for particle animation type constants.
@enum Effects.ParticleAnimationType
@pragma nostrip
*/

/*** Table of Effects.ParticleAnimationType type constants (for use with particles).

 - `LOOP` Frames loop sequentially.
 - `ONE_SHOT` Frames play once, then freeze on the last frame.
 - `BACK_AND_FORTH` Frames loop till end and then reverse.
 - `LIFE_TIME_SPREAD` Frames are distributed over the particle's lifetime

@table Effects.ParticleAnimationType
*/

static const std::unordered_map<std::string, ParticleAnimationMode> PARTICLE_ANIMATION_TYPE
{
	{ "LOOP", ParticleAnimationMode::Loop },
	{ "ONE_SHOT", ParticleAnimationMode::OneShot },
	{ "BACK_AND_FORTH", ParticleAnimationMode::BackAndForth },
	{ "LIFE_TIME_SPREAD", ParticleAnimationMode::LifeTimeSpread },

};
