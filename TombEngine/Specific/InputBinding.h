#pragma once

using std::vector;

// NOTE TO SELF: Doing it wrong! Actions SHOULD NOT hold their own input bindings.
// Do it the other way around: bindings map to actions. Every other engine referenced (TRAE, Unity, Unreal) has it like this.
// Bindings can still hold multiple input gestures, however. This is an original idea I think is worth pursuing. @Sezz
namespace TEN::Input
{
	constexpr int NUM_DEFAULT_BINDINGS = 1;
	constexpr int MAX_BINDINGS		   = 4;
	constexpr int MAX_GESTURES		   = 4;

	class InputBinding
	{
	public:
		void Add(vector<int> binding);
		void Clear(vector<int> binding);
		vector<vector<int>> Get();

	private:
		vector<vector<int>> Bindings = {};
	};
}
