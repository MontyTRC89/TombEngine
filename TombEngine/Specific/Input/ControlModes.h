#pragma once

namespace TEN::Input
{
	// TODO: Move to configuration.h
	enum class ControlMode
	{
		ClassicTank,
		EnhancedTank,
		Modern,

		Count
	};

	bool IsUsingClassicTankControls();
	bool IsUsingEnhancedTankControls();
	bool IsUsingModernControls();
}
