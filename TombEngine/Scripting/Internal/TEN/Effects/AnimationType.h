#pragma once
#include <string>
#include <unordered_map>

#include "Game/effects/effects.h"

/***
Constants for animation type constants.
@enum Effects.AnimationType
@pragma nostrip
*/

/*** Table of Effects.Animation type constants (for use with particles).

 - `OPAQUE`
 - `ALPHATEST`
 - `ADDITIVE`
 - `SUBTRACTIVE`
 - `EXCLUDE`
 - `SCREEN`
 - `LIGHTEN`
 - `ALPHABLEND`

@table Effects.AnimationType
*/

static const std::unordered_map<std::string, ParticleAnimationMode> PARTICLE_ANIMATION_TYPE
{
	{ "LOOP", ParticleAnimationMode::LOOP },
	{ "ONESHOT", ParticleAnimationMode::ONESHOT },
	{ "PINGPONG", ParticleAnimationMode::PINGPONG },
	{ "LIFETIMESPREAD", ParticleAnimationMode::LIFETIMESPREAD },

};
