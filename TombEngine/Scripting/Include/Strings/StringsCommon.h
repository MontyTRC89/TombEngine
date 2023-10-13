
using CallbackDrawString = std::function<
	void(
		const std::string&,
		D3DCOLOR,
		int,   // X
		int,   // X
		float, // Scale
		int)>; // Flags
