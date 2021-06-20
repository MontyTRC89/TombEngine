-- TR5Main GameFlow file
-- Created by MontyTRC
-- Place in this LUA script all the levels of your game
-- Title is mandatory and must be the first level

-- Title level
GameFlow:WriteDefaults();
GameFlow:AddTracks();

title = Level.new();

title.soundTrack = 110;
title.fileName = "Data\\title.trc";
title.loadScreen = "Screens\\rome.jpg";
title.background = "Screens\\Title.png";

GameFlow:AddLevel(title);

-- Test
test = Level.new();

test.name = "level_andrea1";
-- test.script = "test.lua";
test.soundTrack = 128;
test.fileName = "Data\\andrea1.trc";
test.loadScreen = "Screens\\rome.jpg";
test.horizon = true;
test.colAddHorizon = true;
test.layer1 = SkyLayer.new(120, 80, 50, -4);

GameFlow:AddLevel(test);