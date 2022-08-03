#include "framework.h"
#include "Specific/Input/InputBinding.h"

#include <ois/OISKeyboard.h>

using namespace OIS;
using std::array;
using std::vector;

namespace TEN::Input
{
	InputBinding::InputBinding()
	{
		this->Initialize();
	}

	array<int, MAX_KEY_MAPPINGS> InputBinding::Get(int bindingIndex)
	{
		if (!IsIndexWithinRange(bindingIndex))
		{
			array<int, MAX_KEY_MAPPINGS> nullBinding;
			nullBinding.fill(KC_UNASSIGNED);
			return nullBinding;
		}

		return Bindings[bindingIndex];
	}

	void InputBinding::Set(int bindingIndex, array<int, MAX_KEY_MAPPINGS> binding)
	{
		if (!IsIndexWithinRange(bindingIndex))
			return;

		this->Bindings[bindingIndex] = binding;
	}

	void InputBinding::Clear(int bindingIndex)
	{
		if (!IsIndexWithinRange(bindingIndex))
			return;

		this->Bindings[bindingIndex].fill(KC_UNASSIGNED);
	}

	void InputBinding::Initialize()
	{
		// Initialise empty bindings.
		for (auto& binding : this->Bindings)
			binding.fill(KC_UNASSIGNED);
	}

	bool InputBinding::IsIndexWithinRange(int bindingIndex)
	{
		// Ensure defaults are not being overwritten.
		if ((bindingIndex + 1) <= NUM_DEFAULT_BINDINGS)
			return false;

		// Ensure index is not beyond range.
		if ((bindingIndex + 1) > MAX_BINDINGS)
			return false;

		return true;
	}
}
