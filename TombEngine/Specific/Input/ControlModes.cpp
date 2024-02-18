#include "framework.h"
#include "Specific/Input/ControlModes.h"

#include "Specific/configuration.h"

namespace TEN::Input
{
	bool IsUsingClassicTankControls()
	{
		return (g_Configuration.ControlMode == ControlMode::ClassicTank);
	}

	bool IsUsingEnhancedTankControls()
	{
		return (g_Configuration.ControlMode == ControlMode::EnhancedTank);
	}

	bool IsUsingModernControls()
	{
		return (g_Configuration.ControlMode == ControlMode::Modern);
	}
}
