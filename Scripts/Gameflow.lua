-- Place in this Lua script all the levels of your game
-- Title is mandatory and must be the first level.

-- Intro image is a splash screen which appears before actual loading screen.
-- If you don't want it to appear, just remove this line.

Flow.SetIntroImagePath("Screens\\main.jpg")

-- This image should be used for static title screen background (as in TR1-TR3).
-- For now it is not implemented.

Flow.SetTitleScreenImagePath("Screens\\main.jpg")


--------------------------------------------------

-- Title level

title = Level.new()

title.ambientTrack = "108"
title.levelFile = "Data\\title.ten"
title.scriptFile = "Scripts\\title.lua"
title.loadScreenFile = "Screens\\Main.png"

Flow.AddLevel(title)

--------------------------------------------------

-- First test level

test = Level.new()

test.nameKey = "level_test"
test.scriptFile = "Scripts\\New_Level.lua"
test.ambientTrack = "108"
test.levelFile = "Data\\TestLevel.ten"
test.loadScreenFile = "Screens\\rome.jpg"

-- 0 is no weather, 1 is rain, 2 is snow.
-- Strength varies from 0 to 1 (floating-point value, e.g. 0.5 means half-strength).

test.weather = 0
test.weatherStrength = 1

test.horizon = true
test.farView = 20
test.layer1 = Flow.SkyLayer.new(Color.new(255, 0, 0), 15)
test.fog = Flow.Fog.new(Color.new(0, 0, 0), 12, 20)

-- Presets for inventory item placement.

test.objects = {
	InventoryItem.new(
		"tut1_ba_cartouche1",
		ObjID.PUZZLE_ITEM3_COMBO1,
		0,
		0.5,
		Rotation.new(0, 0, 0),
		RotationAxis.Y,
		-1,
		ItemAction.USE
	),
	InventoryItem.new(
		"tut1_ba_cartouche2",
		ObjID.PUZZLE_ITEM3_COMBO2,
		0,
		0.5,
		Rotation.new(0, 0, 0),
		RotationAxis.Y,
		-1,
		ItemAction.USE
	),
	InventoryItem.new(
		"tut1_ba_cartouche",
		ObjID.PUZZLE_ITEM3,
		0,
		0.5,
		Rotation.new(0, 0, 0),
		RotationAxis.Y,
		-1,
		ItemAction.USE
	),
	InventoryItem.new(
		"tut1_hand_orion",
		ObjID.PUZZLE_ITEM6,
		0,
		0.5,
		Rotation.new(270, 180, 0),
		RotationAxis.Y,
		-1,
		ItemAction.USE
	),
	InventoryItem.new(
		"tut1_hand_sirius",
		ObjID.PUZZLE_ITEM8,
		0,
		0.5,
		Rotation.new(270, 180, 0),
		RotationAxis.X,
		-1,
		ItemAction.USE
	)
}

Flow.AddLevel(test)
