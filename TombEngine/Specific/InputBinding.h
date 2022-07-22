#pragma once

using std::vector;

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
