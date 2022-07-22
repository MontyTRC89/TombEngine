#include "framework.h"
#include "Specific/InputBinding.h"

using std::vector;

namespace TEN::Input
{
	void InputBinding::Add(vector<int> binding)
	{
		// Ensure new binding's number of input gestures does not exceed the max.
		if (binding.size() > MAX_GESTURES)
			return;

		// Sort new binding.
		std::sort(std::begin(binding), std::end(binding));

		// Prevent duplicate bindings (excluding defaults).
		for (size_t i = NUM_DEFAULT_BINDINGS; i < Bindings.size(); i++)
		{
			if (binding == Bindings[i])
				return;
		}

		this->Bindings.push_back(binding);
	}

	void InputBinding::Clear(vector<int> binding)
	{
		// Do not attempt clear if only default bindings exist.
		if (Bindings.size() <= NUM_DEFAULT_BINDINGS)
			return;

		// Sort binding to erase.
		std::sort(std::begin(binding), std::end(binding));

		// Find binding to erase (excluding defaults).
		for (size_t i = NUM_DEFAULT_BINDINGS; i < Bindings.size(); i++)
		{
			if (binding == Bindings[i])
				this->Bindings.erase(Bindings.begin() + i);
		}
	}

	vector<vector<int>>InputBinding::Get()
	{
		return Bindings;
	}
}
