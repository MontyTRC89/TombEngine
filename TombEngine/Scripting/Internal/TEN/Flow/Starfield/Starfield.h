#pragma once

#include "Scripting/Internal/TEN/Color/Color.h"
#include "Scripting/Internal/TEN/Vec2/Vec2.h"

namespace sol { class state; }

struct Starfield
{
    int StarsCount = 0; // No need for StarryNight flag, if stars count = 0, shader is bypassed

    int MeteorsCount = 0; // No need for EnableMeteors flag, if meteors count = 0, shader is bypassed
    int MeteorsSpawnDensity = 0;
    int MeteorsSpeed = 0;

    Starfield() = default;
    Starfield(int starsCount);
    Starfield(int starsCount, int meteorsCount, int meteorsSpawnDensity, int meteorsSpeed);

    void SetStarsCount(int const& starsCount);
    int GetStarsCount() const;

    void SetMeteorsCount(int const& meteorsCount);
    int GetMeteorsCount() const;

    void SetMeteorsSpawnDensity(int const& spawnDensity);
    int GetMeteorsSpawnDensity() const;

    void SetMeteorsSpeed(float const& meteorsSpeed);
    float GetMeteorsSpeed() const;

    bool GetEnabled() const;
    bool GetMeteorsEnabled() const;

    static void Register(sol::table&);
};