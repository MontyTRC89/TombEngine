#pragma once
#include "Specific/Input/Bindings.h"
#include "Specific/Input/InputAction.h"
#include "Specific/Input/Keys.h"

struct ItemInfo;

namespace TEN::Input
{
	enum InputAxis
	{
		MoveVertical,
		MoveHorizontal,
		CameraVertical,
		CameraHorizontal,
		Count
	};

	enum class QueueState
	{
		None,
		Update,
		Clear
	};

	enum class RumbleMode
	{
		None,
		Left,
		Right,
		Both
	};

	struct RumbleData
	{
		float	   Power	 = 0.0f;
		RumbleMode Mode		 = RumbleMode::None;
		float	   LastPower = 0.0f;
		float	   FadeSpeed = 0.0f;
	};

	extern std::vector<InputAction> ActionMap;
	extern std::vector<QueueState>	ActionQueue;
	extern std::vector<float>		KeyMap;
	extern std::vector<float>		AxisMap;

	void InitializeInput(HWND handle);
	void DeinitializeInput();
	void DefaultConflict();
	void UpdateInputActions(ItemInfo* item, bool applyQueue = false);
	void ApplyActionQueue();
	void ClearActionQueue();
	void ClearAllActions();
	void Rumble(float power, float delayInSec = 0.3f, RumbleMode mode = RumbleMode::Both);
	void StopRumble();
    void ApplyDefaultBindings();
    bool ApplyDefaultXInputBindings();

	// TODO: Later, all these global action accessor functions should be tied to a specific controller/player.
	// Having them loose like this is very inelegant, but since this is only the first iteration, they will do for now. -- Sezz 2022.10.12
	void  ClearAction(ActionID actionID);
	bool  NoAction();
	bool  IsClicked(ActionID actionID);
	bool  IsHeld(ActionID actionID, float delayInSec = 0.0f);
	bool  IsPulsed(ActionID actionID, float delayInSec, float initialDelayInSec = 0.0f);
	bool  IsReleased(ActionID actionID, float maxDelayInSec = INFINITY);
	float GetActionValue(ActionID actionID);
	float GetActionTimeActive(ActionID actionID);
	float GetActionTimeInactive(ActionID actionID);

	bool IsDirectionalActionHeld();
	bool IsWakeActionHeld();
	bool IsOpticActionHeld();
}
