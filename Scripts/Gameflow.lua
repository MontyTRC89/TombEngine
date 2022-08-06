-- Place in this LUA script all the levels of your game
-- Title is mandatory and must be the first level.

-- Shorten some of the internal data types.

local Flow = TEN.Flow
local Level = Flow.Level
local Color = TEN.Color
local Rotation = TEN.Rotation
local InventoryItem = Flow.InventoryItem
local InvID = Flow.InvID
local RotationAxis = Flow.RotationAxis
local ItemAction = Flow.ItemAction

-- These variables are unused for now.

Flow.SetIntroImagePath("Screens\\main.jpg")
Flow.SetTitleScreenImagePath("Screens\\main.jpg")

-- Flow.SetFarView sets global far view distance in blocks.
-- It will be overwritten by level.farView value, if it is specified.

Flow.SetFarView(20)

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
test.scriptFile = "Scripts\\TestLevel.lua"
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
		InvID.PUZZLE_ITEM3_COMBO1,
		0,
		0.5,
		Rotation.new(0, 0, 0),
		RotationAxis.Y,
		-1,
		ItemAction.USE
	),
	myObj, 
	InventoryItem.new(
		"tut1_ba_cartouche2",
		InvID.PUZZLE_ITEM3_COMBO2,
		0,
		0.5,
		Rotation.new(0, 0, 0),
		RotationAxis.Y,
		-1,
		ItemAction.USE
	),
	InventoryItem.new(
		"tut1_ba_cartouche",
		InvID.PUZZLE_ITEM3,
		0,
		0.5,
		Rotation.new(0, 0, 0),
		RotationAxis.Y,
		-1,
		ItemAction.USE
	),
	InventoryItem.new(
		"tut1_hand_orion",
		InvID.PUZZLE_ITEM6,
		0,
		0.5,
		Rotation.new(270, 180, 0),
		RotationAxis.Y,
		-1,
		ItemAction.USE
	),
	InventoryItem.new(
		"tut1_hand_sirius",
		InvID.PUZZLE_ITEM8,
		0,
		0.5,
		Rotation.new(270, 180, 0),
		RotationAxis.X,
		-1,
		ItemAction.USE
	)
}

Flow.AddLevel(test)