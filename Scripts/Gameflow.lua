-- Place in this LUA script all the levels of your game
-- Title is mandatory and must be the first level

-- Title level
SetIntroImagePath("SCREENS\\MAIN.PNG")
SetTitleScreenImagePath("Screens\\Title.png")
SetGameFarView(210)

title = Level.new();

title.ambientTrack = "108_A8_Coastal";
title.levelFile = "Data\\title.trc";
title.scriptFile = "Scripts\\title.lua";
title.loadScreenFile = "Screens\\rome.jpg";

AddLevel(title);

-- Test
test = Level.new();

test.nameKey = "level_andrea1";
test.scriptFile = "Scripts\\andrea1.lua";
test.ambientTrack = "108_A8_Coastal";
test.levelFile = "Data\\andrea1.trc";
test.loadScreenFile = "Screens\\rome.jpg";
test.weather = 1;
test.weatherStrength = 1;
test.horizon = true
test.farView = 10
test.colAddHorizon = true
test.layer1 = SkyLayer.new(Color.new(255, 0, 0), 15)

test.objects = {
	InventoryObject.new(
		"tut1_ba_cartouche1",
		InvItem.PUZZLE_ITEM3_COMBO1,
		0,
		0.5,
		Rotation.new(0, 0, 0),
		RotationAxis.Y,
		-1,
		ItemAction.USE
	),
	myObj, 
	InventoryObject.new(
		"tut1_ba_cartouche2",
		InvItem.PUZZLE_ITEM3_COMBO2,
		0,
		0.5,
		Rotation.new(0, 0, 0),
		RotationAxis.Y,
		-1,
		ItemAction.USE
	),
	InventoryObject.new(
		"tut1_ba_cartouche",
		InvItem.PUZZLE_ITEM3,
		0,
		0.5,
		Rotation.new(0, 0, 0),
		RotationAxis.Y,
		-1,
		ItemAction.USE
	),
	InventoryObject.new(
		"tut1_hand_orion",
		InvItem.PUZZLE_ITEM6,
		0,
		0.5,
		Rotation.new(270, 180, 0),
		RotationAxis.Y,
		-1,
		ItemAction.USE
	),
	InventoryObject.new(
		"tut1_hand_sirius",
		InvItem.PUZZLE_ITEM8,
		0,
		0.5,
		Rotation.new(270, 180, 0),
		RotationAxis.X,
		-1,
		ItemAction.USE
	)

};

AddLevel(test);
