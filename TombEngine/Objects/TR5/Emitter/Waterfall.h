#pragma once

namespace TEN::Effects::WaterfallEmitter
{
	void InitializeWaterfall(short itemNumber);
	void ControlWaterfall(short itemNumber);

	void SpawnWaterfallMist(const Vector4& pos, int roomNumber, float scalar, float size, const Color& color);
}
