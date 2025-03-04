#include "framework.h"
#include "Objects/Effects/Fireflies.h"

#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/Point.h"
#include "Game/control/box.h"
#include "Game/control/flipeffect.h"
#include "Game/effects/effects.h"
#include "Game/effects/Streamer.h"
#include "Game/effects/tomb4fx.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/misc.h"
#include "Game/Setup.h"
#include "Math/Math.h"
#include "Renderer/Renderer.h"
#include "Specific/clock.h"
#include "Specific/level.h"

using namespace TEN::Collision::Point;
using namespace TEN::Math;
using namespace TEN::Renderer;
using namespace TEN::Effects::Streamer;

namespace TEN::Effects::Fireflys
{
    constexpr auto FIREFLY_COHESION_FACTOR = 600.1f;
    constexpr auto FIREFLY_SPACING_FACTOR = 600.0f;
    constexpr auto FIREFLY_CATCH_UP_FACTOR = 0.2f;
    constexpr auto FIREFLY_TARGET_DISTANCE_MAX = SQUARE(BLOCK(1.0f));
    constexpr auto FIREFLY_BASE_SEPARATION_DISTANCE = 10.0f;
    constexpr auto FIREFLY_FLEE_DISTANCE = BLOCK(0.7);
    constexpr auto FIREFLY_RETURN_DISTANCE = BLOCK(4);
    constexpr auto MAX_FIREFLIES = 64;
    constexpr auto DEFAULT_FIREFLY_COUNT = 24;
    constexpr auto FIREFLY_RISE_UP_FACTOR = 200;

    std::vector<FireflyData> FireflySwarm = {};
    std::unordered_map<int, int> nextFireflyNumberMap; // Numbering the Fireflies for Streamer effect.

    void InitializeFireflySwarm(short itemNumber)
    {
        auto& item = g_Level.Items[itemNumber];

        item.Animation.Velocity.z = Random::GenerateFloat(32.0f, 160.0f);

        item.HitPoints = DEFAULT_FIREFLY_COUNT;
        item.ItemFlags[FirefliesItemFlags::TargetItemPtr] = item.Index;
        item.ItemFlags[FirefliesItemFlags::Light] = 1; // 0 = Turn off light effect, 1 = turn on light (DEFAULT).

        item.ItemFlags[FirefliesItemFlags::TriggerFlags] = item.TriggerFlags;

		if (item.TriggerFlags < -8)
			item.ItemFlags[FirefliesItemFlags::TriggerFlags] = -8;
		
        if (item.TriggerFlags > 8)
            item.ItemFlags[FirefliesItemFlags::TriggerFlags] = 8;
       
        item.ItemFlags[FirefliesItemFlags::Spawncounter] = 0;
        item.ItemFlags[FirefliesItemFlags::FliesEffect] = 0;
    }

    void SpawnFireflySwarm(ItemInfo& item, int TriggerFlags)
    {
        constexpr auto VEL_MAX = 34.0f;
        constexpr auto VEL_MIN = 6.0f;

        // Create new firefly.
        auto& firefly = GetNewEffect(FireflySwarm, MAX_FIREFLIES);

        unsigned char r = 255;
        unsigned char g = 255;
        unsigned char b = 255;

        if (TriggerFlags >= 0)
        {

            float brightnessShift = Random::GenerateFloat(-0.1f, 0.1f);
            r = std::clamp(item.Model.Color.x / 2.0f + brightnessShift, 0.0f, 1.0f) * UCHAR_MAX;
            g = std::clamp(item.Model.Color.y / 2.0f + brightnessShift, 0.0f, 1.0f) * UCHAR_MAX;
            b = std::clamp(item.Model.Color.z / 2.0f + brightnessShift, 0.0f, 1.0f) * UCHAR_MAX;

            firefly.SpriteSeqID = ID_FIREFLY_SPRITES;
            firefly.SpriteID = 0;
            firefly.blendMode = BlendMode::Additive;
            firefly.scalar = 3.0f;
            firefly.size = 1.0f;
        }
        else
        {
            firefly.SpriteSeqID = ID_FIREFLY_SPRITES;
            firefly.SpriteID = 1;
            firefly.blendMode = BlendMode::Subtractive;
            firefly.scalar = 1.2f;
            firefly.size = 1.2f;
        }

        firefly.r = firefly.rB = r;
        firefly.g = firefly.gB = g;
        firefly.b = firefly.bB = b;
        
        firefly.rotAng = ANGLE(0.0f);
        firefly.on = true;
 
        firefly.Position = item.Pose.Position.ToVector3();
        firefly.RoomNumber = item.RoomNumber;
        firefly.Orientation = item.Pose.Orientation;
        firefly.Velocity = Random::GenerateFloat(VEL_MIN, VEL_MAX);

        firefly.Life = Random::GenerateInt(1, 200);

        firefly.LeaderItemPtr = &g_Level.Items[item.ItemFlags[FirefliesItemFlags::TargetItemPtr]];
        firefly.TargetItemPtr = &g_Level.Items[item.ItemFlags[FirefliesItemFlags::TargetItemPtr]];

        int& nextFireflyNumber = nextFireflyNumberMap[item.Index];
        firefly.Number = nextFireflyNumber++;
    }
        
    void ControlFireflySwarm(short itemNumber)
    {
        auto& item = g_Level.Items[itemNumber];

        if (!TriggerActive(&item))
            return;

        if (item.HitPoints != NOT_TARGETABLE)
        {
            int fireflyCount = item.HitPoints - item.ItemFlags[FirefliesItemFlags::Spawncounter];

            if (fireflyCount < 0)
            {
                int firefliesToTurnOff = -fireflyCount;
                for (auto& firefly : FireflySwarm)
                {
                    if (firefly.LeaderItemPtr == &item && firefly.Life > 0.0f)
                    {
                        firefly.Life = 0.0f;
                        firefly.on = false;
                        firefliesToTurnOff--;

                        if (firefliesToTurnOff == 0)
                            break;
                    }
                }
            }
            else if (fireflyCount > 0)
            {
                for (int i = 0; i < fireflyCount; i++)
                {
                    SpawnFireflySwarm(item, item.TriggerFlags);
                }
            }

            item.ItemFlags[FirefliesItemFlags::Spawncounter] = item.HitPoints;
            item.HitPoints = NOT_TARGETABLE;
        }

        for (auto& firefly : FireflySwarm)
        {
            if (firefly.TargetItemPtr == &item)
            {
                auto posBase = firefly.Position;
                auto rotMatrix = firefly.Orientation.ToRotationMatrix();
                auto pos = posBase + Vector3::Transform(Vector3(0, 0, 0), rotMatrix);

                auto direction0 = Geometry::RotatePoint(posBase, EulerAngles(0, 0, 0));
                
                short orient2D = firefly.Orientation.z;
                
                if (firefly.TargetItemPtr->ItemFlags[FirefliesItemFlags::Light] == 1 && firefly.TargetItemPtr->ItemFlags[FirefliesItemFlags::TriggerFlags] >= 0)
                {               
                   SpawnDynamicLight(firefly.Position.x, firefly.Position.y, firefly.Position.z, 2, firefly.r, firefly.g, firefly.b);

                   StreamerEffect.Spawn(firefly.TargetItemPtr->Index, firefly.Number, pos, direction0, orient2D,
                       Vector4(firefly.r / (float)UCHAR_MAX, firefly.g / (float)UCHAR_MAX, firefly.b / (float)UCHAR_MAX, 1.0f),
                       Vector4::Zero,
                       0.0f, 0.11f, 20.0f, 0.1f, 0.0f, StreamerFeatherType::None, BlendMode::Additive);                    
                }

                else if (firefly.TargetItemPtr->ItemFlags[FirefliesItemFlags::TriggerFlags] < 0)
                {                      
                    StreamerEffect.Spawn(firefly.TargetItemPtr->Index, firefly.Number, pos, direction0, orient2D,        
                        Vector4((firefly.r / 2) / (float)UCHAR_MAX, (firefly.g / 2) / (float)UCHAR_MAX, (firefly.b / 2) / (float)UCHAR_MAX, 0.2f),   
                        Vector4((firefly.r / 3) / (float)UCHAR_MAX, (firefly.g / 3) / (float)UCHAR_MAX, (firefly.b / 3) / (float)UCHAR_MAX, 0.2f),     
                        0.0f, 0.4f, 0.0f, 0.2f, 0.0f, StreamerFeatherType::None, BlendMode::Subtractive);             
                }                   
            }
        }
    }

    void UpdateFireflySwarm()
    {
        constexpr auto FLEE_VEL = 1.5f;
        constexpr auto ALPHA_CYCLE_DURATION = 100.0f; 
        constexpr auto ALPHA_PAUSE_DURATION = 90.0f;

        static const auto SPHERE = BoundingSphere(Vector3::Zero, BLOCK(1 / 8.0f));

        if (FireflySwarm.empty())
            return;

        const auto& playerItem = *LaraItem;
        static float frameCounter = 0.0f;

        // Increment the counter variable in each frame.
        frameCounter += 1.0f; 

        for (auto& firefly : FireflySwarm)
        {  
            if (firefly.Life <= 0.0f || !firefly.on)
                continue;

            if (firefly.TargetItemPtr->ItemFlags[FirefliesItemFlags::FliesEffect])
            {
                firefly.r = 0;
                firefly.g = 0;
                firefly.b = 0;
                continue;
            }

            firefly.StoreInterpolationData();

            auto& leaderItem = *firefly.LeaderItemPtr;

            firefly.PositionTarget = Random::GeneratePointInSphere(SPHERE);

            int multiplierX = CLICK(firefly.TargetItemPtr->ItemFlags[FirefliesItemFlags::TriggerFlags] * 2);
            int multiplierY = CLICK(firefly.TargetItemPtr->ItemFlags[FirefliesItemFlags::TriggerFlags] * 4);
            int multiplierZ = CLICK(firefly.TargetItemPtr->ItemFlags[FirefliesItemFlags::TriggerFlags] * 2);

            auto SPHEROID_SEMI_MAJOR_AXIS = Vector3(multiplierX, multiplierY, multiplierZ);
            auto itemPos = Vector3i(firefly.TargetItemPtr->Pose.Position.x, firefly.TargetItemPtr->Pose.Position.y - FIREFLY_RISE_UP_FACTOR, firefly.TargetItemPtr->Pose.Position.z);
            
            // Calculate desired position based on target object and random offsets.
            auto desiredPos = itemPos + Random::GeneratePointInSpheroid(firefly.PositionTarget, EulerAngles::Identity, SPHEROID_SEMI_MAJOR_AXIS);
            auto dir = desiredPos - firefly.Position;

            auto dirs = dir.ToVector3();
            dirs.Normalize();
            auto dirNorm = dirs;

            // Define cohesion factor to keep fireflies close together.
            float distToTarget = dirs.Length();

            float targetVel = (distToTarget * FIREFLY_COHESION_FACTOR) + Random::GenerateFloat(3.0f, 5.0f);
            firefly.Velocity = std::min(targetVel, firefly.TargetItemPtr->Animation.Velocity.z - 21.0f);

            // If firefly is too far from target, increase velocity to catch up.
            if (distToTarget > FIREFLY_TARGET_DISTANCE_MAX)
                firefly.Velocity += FIREFLY_CATCH_UP_FACTOR;

            // Translate.
            auto moveDir = firefly.Orientation.ToDirection();
            moveDir.Normalize();
            firefly.Position += (moveDir * firefly.Velocity) / 26.0f;
            firefly.Position += (moveDir * FIREFLY_SPACING_FACTOR) / 26.0f;

            auto orientTo = Geometry::GetOrientToPoint(firefly.Position, desiredPos.ToVector3());
            firefly.Orientation.Lerp(orientTo, 0.1f);

                for (const auto& otherFirefly : FireflySwarm)
                {
                    if (&firefly == &otherFirefly)
                        continue;

                    float distToOtherFirefly = Vector3i::Distance(firefly.Position, otherFirefly.Position);
                    float distToPlayer = Vector3i::Distance(firefly.Position, playerItem.Pose.Position);

                    // If player is too close, flee.
                    if (distToPlayer < FIREFLY_FLEE_DISTANCE && playerItem.Animation.ActiveState != 2)
                    {
                        auto separationDir = firefly.Position - playerItem.Pose.Position.ToVector3();
                        separationDir.Normalize();

                        // Reduce the Y component of the escape direction.
                        separationDir.y *= Random::GenerateFloat(0.0f, 0.4f);

                        // Normalize the direction again to get the length of the vector.
                        separationDir.Normalize();

                        firefly.Position += separationDir * FLEE_VEL;

                        auto orientTo = Geometry::GetOrientToPoint(firefly.Position, separationDir);
                        firefly.Orientation.Lerp(orientTo, 0.05f);

                        firefly.Velocity -= std::min(FLEE_VEL, firefly.TargetItemPtr->Animation.Velocity.z - 1.0f);
                    }
                    else if (distToPlayer > FIREFLY_RETURN_DISTANCE)
                    {
                        // Return to the leader item.
                        firefly.TargetItemPtr = firefly.LeaderItemPtr;
                    }

                    if (distToOtherFirefly < FIREFLY_BASE_SEPARATION_DISTANCE)
                    {
                        auto separationDir = firefly.Position - otherFirefly.Position;
                        separationDir.Normalize();

                        firefly.Position += separationDir * (FIREFLY_BASE_SEPARATION_DISTANCE - distToOtherFirefly);
                    }
                }

            auto pointColl = GetPointCollision(firefly.Position, firefly.RoomNumber);

            // Update firefly room number.
            if (pointColl.GetRoomNumber() != firefly.RoomNumber &&
                pointColl.GetRoomNumber() != NO_VALUE)
            {
                firefly.RoomNumber = pointColl.GetRoomNumber();
            }

            // Update color values for blinking effect.
            float alphaTime = fmod(frameCounter + firefly.Life, ALPHA_CYCLE_DURATION + ALPHA_PAUSE_DURATION);
            float alphaFactor;
            if (alphaTime < ALPHA_CYCLE_DURATION)
            {
                alphaFactor = 0.5f * (1.0f + sinf((alphaTime / ALPHA_CYCLE_DURATION) * PI_MUL_2));
            }
            else
            {
                alphaFactor = 1.0f;
            }

            firefly.r = static_cast<unsigned char>(firefly.rB * alphaFactor);
            firefly.g = static_cast<unsigned char>(firefly.gB * alphaFactor);
            firefly.b = static_cast<unsigned char>(firefly.bB * alphaFactor);
        }
    }

    void ClearFireflySwarm()
    {
        FireflySwarm.clear();
    }
}
