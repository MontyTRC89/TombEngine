#pragma once

// Deprecated, but still used by climbable wall test functions for now.
struct VaultTestResult
{
	int Height;
	bool SetBusyHands;
	bool SnapToLedge;
	bool SetJumpVelocity;
	LaraState TargetState;
};
