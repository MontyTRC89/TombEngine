-- Place in this LUA script all the levels of your game
-- Title is mandatory and must be the first level

local Flow = TEN.Flow
local Level = Flow.Level
local Color = TEN.Color
local Rotation = TEN.Rotation
local InventoryItem = Flow.InventoryItem
local InvID = Flow.InvID
local RotationAxis = Flow.RotationAxis
local ItemAction = Flow.ItemAction

Flow.SetIntroImagePath("Screens\\Main.png")
Flow.SetTitleScreenImagePath("Screens\\Title.jpg")
Flow.SetFarView(80)

-- Title level

title = Level.new()

title.ambientTrack = "108_A8_Coastal"
title.levelFile = "Data\\title.ten"
title.scriptFile = "Scripts\\title.lua"
title.loadScreenFile = "Screens\\Main.png"

Flow.AddLevel(title)

-- First test level
test = Level.new()

test.nameKey = "level_andrea1"
test.scriptFile = "Scripts\\andrea1.lua"
test.ambientTrack = "108_A8_Coastal"
test.levelFile = "Data\\andrea1.ten"
test.loadScreenFile = "Screens\\rome.jpg"
test.weather = 0
test.weatherStrength = 1
test.horizon = true
test.farView = 80
test.layer1 = Flow.SkyLayer.new(Color.new(255, 0, 0), 15)

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