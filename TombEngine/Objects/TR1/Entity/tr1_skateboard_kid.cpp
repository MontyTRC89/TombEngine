#include "framework.h"
#include "Objects/TR1/Entity/tr1_skateboard_kid.h"
#include "Game/control/box.h"

namespace TEN::Entities::Creatures::TR1
{
    constexpr auto SKATEKID_TURN_RATE = ANGLE(4.0f);
    constexpr auto SKATEKID_TOOCLOSE_RANGE = SQUARE(BLOCK(1));
    constexpr auto SKATEKID_DONTSTOP_RANGE = SQUARE(BLOCK(2.5f));
    constexpr auto SKATEKID_STOP_RANGE = SQUARE(BLOCK(4));
    constexpr auto SKATEKID_PUSH_CHANCE = 512;
    constexpr auto SKATEKID_SKATE_CHANCE = 1024;

    // TODO: fix it later when the PR #1063 is merged to develop.
    const auto kid_gun1 = BiteInfo(Vector3(0.0f, 150.0f, 34.0f), 7);
    const auto kid_gun2 = BiteInfo(Vector3(0.0f, 150.0f, 37.0f), 4);

    enum SkateKidState
    {
        KID_STATE_STOP = 0,
        KID_STATE_SHOOT,
        KID_STATE_SKATE,
        KID_STATE_PUSH,
        KID_STATE_SHOOT2,
        KID_STATE_DEATH
    };

    enum SkateKidAnim
    {

    };

    void InitialiseSkateboardKid(short itemNumber)
    {

    }

    void SkateboardKidControl(short itemNumber)
    {

    }
}
