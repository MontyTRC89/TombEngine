#include "framework.h"
#include "Objects/Effects/Fireflies.h"

#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/Point.h"
#include "Game/control/box.h"
#include "Game/control/flipeffect.h"
#include "Game/effects/effects.h"
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
#include "Game/effects/Streamer.h"


using namespace TEN::Collision::Point;
using namespace TEN::Math;
using namespace TEN::Renderer;
using namespace TEN::Effects::Streamer;

namespace TEN::Effects::Fireflys
{
    constexpr auto FIREFLY_VELOCITY_MAX = 9.0f;
    constexpr auto FIREFLY_COHESION_FACTOR = 600.1f;
    constexpr auto FIREFLY_SPACING_FACTOR = 600.0f;
    constexpr auto FIREFLY_CATCH_UP_FACTOR = 0.2f;
    constexpr auto FIREFLY_TARGET_DISTANCE_MAX = SQUARE(BLOCK(1.0f));
    constexpr auto FIREFLY_BASE_SEPARATION_DISTANCE = 10.0f;
    constexpr auto FIREFLY_UPDATE_INTERVAL_TIME = 0.2f;
    constexpr auto FIREFLY_FLEE_DISTANCE = BLOCK(0.7);
    constexpr auto FIREFLY_RETURN_DISTANCE = BLOCK(4);
    constexpr auto MAX_FIREFLIES = 64;

    std::vector<FireflyData> FireflySwarm = {};
    std::unordered_map<int, int> nextFireflyNumberMap; //Numbering the Fireflies for Streamer effect

    void InitializeFireflySwarm(short itemNumber)
    {
        constexpr auto DEFAULT_FIREFLY_COUNT = 24;

        auto& item = g_Level.Items[itemNumber];

        item.StartPose.Position = item.Pose.Position;
        item.Animation.Velocity.z = Random::GenerateFloat(32.0f, 160.0f);
        item.HitPoints = DEFAULT_FIREFLY_COUNT;
        item.ItemFlags[0] = item.Index;
        item.ItemFlags[1] = item.Index;
        item.ItemFlags[5] = 0;
        item.ItemFlags[4] = 0;
        item.ItemFlags[6] = 1;

        if (item.AIBits)
            item.ItemFlags[6] = true;

        item.HitPoints = 16;
    }

    static void SpawnFireflySwarm(ItemInfo& item)
    {
        constexpr auto VEL_MAX = 34.0f;
        constexpr auto VEL_MIN = 6.0f;
        constexpr auto START_ORIENT_CONSTRAINT = std::pair<EulerAngles, EulerAngles>(
            EulerAngles(ANGLE(-3.0f), ANGLE(-6.0f), 0),
            EulerAngles(ANGLE(3.0f), ANGLE(6.0f), 0));

        // Create new firefly.
        auto& firefly = GetNewEffect(FireflySwarm, MAX_FIREFLIES);

        unsigned char r = 0;
        unsigned char g = 0;
        unsigned char b = 0;

        float brightnessShift = Random::GenerateFloat(-0.1f, 0.1f);
        r = std::clamp(item.Model.Color.x / 2.0f + brightnessShift, 0.0f, 1.0f) * UCHAR_MAX;
        g = std::clamp(item.Model.Color.y / 2.0f + brightnessShift, 0.0f, 1.0f) * UCHAR_MAX;
        b = std::clamp(item.Model.Color.z / 2.0f + brightnessShift, 0.0f, 1.0f) * UCHAR_MAX;

        firefly.SpriteSeqID = ID_DEFAULT_SPRITES;
        firefly.SpriteID = SPR_UNDERWATERDUST;
        firefly.blendMode = BlendMode::Additive;
        firefly.scalar = 3.0f;
        firefly.r = firefly.rB = r;
        firefly.g = firefly.gB = g;
        firefly.b = firefly.bB = b;
        firefly.size = 1.0f;
        firefly.rotAng = Random::GenerateAngle(ANGLE(0.0f), ANGLE(20.0f));
        firefly.on = true;

        firefly.IsLethal = false;
 
        firefly.Position = item.Pose.Position.ToVector3();
        firefly.RoomNumber = item.RoomNumber;
        firefly.Orientation = item.Pose.Orientation;
        firefly.Velocity = Random::GenerateFloat(VEL_MIN, VEL_MAX);

        firefly.Life = Random::GenerateInt(1, 200);
        firefly.Undulation = Random::GenerateFloat(0.0f, PI_MUL_2);

        firefly.LeaderItemPtr = &g_Level.Items[item.ItemFlags[0]];
        firefly.TargetItemPtr = &g_Level.Items[item.ItemFlags[0]];

        int& nextFireflyNumber = nextFireflyNumberMap[item.Index];
        firefly.Number = nextFireflyNumber++;

    }
        

    

    void ControlFireflySwarm(short itemNumber)
    {
        if (!CreatureActive(itemNumber))
            return;

        auto& item = g_Level.Items[itemNumber];
        auto& creature = *GetCreatureInfo(&item);
        const auto& playerItem = *LaraItem;

        AI_INFO ai;
        CreatureAIInfo(&item, &ai);

      

        if (item.HitPoints != NOT_TARGETABLE)
        {
            int fireflyCount = item.HitPoints - item.ItemFlags[5];

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
                    SpawnFireflySwarm(item);
                }
            }

            item.ItemFlags[5] = item.HitPoints;       
            item.HitPoints = NOT_TARGETABLE;
        }

       

        int dx = creature.Target.x - item.Pose.Position.x;
        int dz = creature.Target.z - item.Pose.Position.z;
        ai.distance = SQUARE(dx) + SQUARE(dz);

        item.Animation.Velocity.z = FIREFLY_VELOCITY_MAX;

        auto& playerRoom = g_Level.Rooms[playerItem.RoomNumber];

        // Circle around target item.
        item.ItemFlags[1] = item.ItemFlags[0];

        for (auto& firefly : FireflySwarm)
        {
           firefly.RoomNumber = item.RoomNumber;
           firefly.TargetItemPtr = &g_Level.Items[itemNumber];
        }
    }

    void UpdateFireflySwarm()
    {
        constexpr auto FLEE_VEL = 1.5f;
        constexpr auto ALPHA_CYCLE_DURATION = 100.0f; // Dauer eines vollständigen Alpha-Zyklus in Frames (z.B. 5 Sekunden bei 30 FPS)
        constexpr auto ALPHA_PAUSE_DURATION = 90.0f;  // Dauer der Pause bei Alpha 1.0 in Frames (z.B. 2 Sekunden bei 30 FPS)

        static const auto SPHERE = BoundingSphere(Vector3::Zero, BLOCK(1 / 8.0f));

        if (FireflySwarm.empty())
            return;

        const auto& playerItem = *LaraItem;
        static float frameCounter = 0.0f; // Zählervariable für die Frames

        frameCounter += 1.0f; // Inkrementiere die Zählervariable in jedem Frame

        for (auto& firefly : FireflySwarm)
        {
  
            if (firefly.Life <= 0.0f || !firefly.on)
                continue;

            firefly.StoreInterpolationData();

            auto& leaderItem = *firefly.LeaderItemPtr;

            firefly.PositionTarget = Random::GeneratePointInSphere(SPHERE);

            int multipler = firefly.TargetItemPtr->TriggerFlags;

            int multiplierX = CLICK(firefly.TargetItemPtr->TriggerFlags + 2);
            int multiplierY = CLICK(firefly.TargetItemPtr->TriggerFlags + 4);
            int multiplierZ = CLICK(firefly.TargetItemPtr->TriggerFlags + 2);

            auto SPHEROID_SEMI_MAJOR_AXIS = Vector3(multiplierX, multiplierY, multiplierZ);

            // Calculate desired position based on target object and random offsets.
            auto desiredPos = firefly.TargetItemPtr->Pose.Position + Random::GeneratePointInSpheroid(firefly.PositionTarget, EulerAngles::Identity, SPHEROID_SEMI_MAJOR_AXIS);
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

                    // Reduziere die Y-Komponente der Fluchtrichtung
                    separationDir.y *= Random::GenerateFloat(0.0f, 0.4f); // oder setze sie auf 0

                    // Normalisiere die Richtung erneut, um die Länge des Vektors zu erhalten
                    separationDir.Normalize();

                    firefly.Position += separationDir * FLEE_VEL;

                    auto orientTo = Geometry::GetOrientToPoint(firefly.Position, separationDir);
                    firefly.Orientation.Lerp(orientTo, 0.05f);

                    firefly.Velocity -= std::min(FLEE_VEL, firefly.TargetItemPtr->Animation.Velocity.z - 1.0f);
                }
                else if (distToPlayer > FIREFLY_RETURN_DISTANCE )
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

            // Update color values for blinking effect
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

            auto posBase = firefly.Position;
            auto rotMatrix = firefly.Orientation.ToRotationMatrix();
            auto pos = posBase + Vector3::Transform(Vector3(0,0,0), rotMatrix);

            auto direction0 =  Geometry::RotatePoint(posBase, EulerAngles(0, 0, 0));

            short orient2D =  firefly.Orientation.z;

            SpawnDynamicLight(firefly.Position.x, firefly.Position.y, firefly.Position.z, 2, firefly.r, firefly.g, firefly.b);
            
         if (firefly.TargetItemPtr->ItemFlags[6] = 1)
             SpawnDynamicLight(firefly.Position.x, firefly.Position.y, firefly.Position.z, 2, firefly.r, firefly.g, firefly.b);


            StreamerEffect.Spawn(firefly.TargetItemPtr->Index, firefly.Number, pos, direction0, orient2D, Vector4(firefly.r / (float)UCHAR_MAX, firefly.g / (float)UCHAR_MAX, firefly.b / (float)UCHAR_MAX, 1.0f),
                0.0f, 0.11f, 20.0f, 0.1f, 0.0f, (int)StreamerFlags::BlendModeAdditive);      
        }
    }

    void ClearFireflySwarm()
    {
        FireflySwarm.clear();
    }
}
