#pragma once

namespace TEN::Input
{
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
