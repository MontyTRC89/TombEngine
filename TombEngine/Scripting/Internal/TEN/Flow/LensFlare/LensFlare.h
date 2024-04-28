#pragma once

#include "Scripting/Internal/TEN/Color/Color.h"
#include "Scripting/Internal/TEN/Vec2/Vec2.h"
#include "Objects/objectslist.h"

namespace sol { class state; }

struct LensFlare
{
    bool Enabled;
    int SunSpriteID = SPR_LENSFLARE3; // Index into sprites
    byte R;
    byte G;
    byte B;
    float Yaw;
    float Pitch;

    LensFlare() = default;
    LensFlare(Vec2 const& yawPitchInDegrees, ScriptColor const& col);

    void SetColor(ScriptColor const& color);
    ScriptColor GetColor() const;

    void SetPosition(Vec2 const& yawPitchInDegrees);
    Vec2 GetPosition() const;

    void SetSunSpriteID(int const& spriteIndex);
    int GetSunSpriteID() const;

    bool GetEnabled() const;

    static void Register(sol::table&);
};