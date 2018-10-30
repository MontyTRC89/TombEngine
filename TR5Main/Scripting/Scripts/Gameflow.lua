-- TR5Main Gameflow file
-- Created by MontyTRC
-- Place in this LUA script all the levels of your game
-- Title is mandatory and must be the first level

-- Title level
title = Level.new();

title.script = "andrea2.lua";
title.soundTrack = 110;
title.fileName = "Data\\title.trc";
title.loadScreen = "Screens\\rome.jpg";
title.background = "Title.png";

Gameflow:AddLevel(title);

-- Test
test = Level.new();

test.name = 115;
test.script = "test.lua";
test.soundTrack = 128;
test.fileName = "Data\\test.trc";
test.loadScreen = "Screens\\rome.jpg";
test.horizon = true;
test.colAddHorizon = true;
test.layer1 = SkyLayer.new(120, 80, 50, -4);

Gameflow:AddLevel(test);

-- Streets of rome --
streets = Level.new();

streets.name = 101;
streets.soundTrack = 128;
streets.fileName = "Data\\andrea1.trc";
streets.loadScreen = "Screens\\andea1.jpg";
streets.horizon = true;
streets.colAddHorizon = true;
streets.layer1 = SkyLayer.new(120, 80, 50, -4);
streets.resetHub = true;

Gameflow:AddLevel(streets);

-- Trajan markets --
trajan = Level.new();

trajan.name = 102;
trajan.script = "andrea2.lua";
trajan.soundTrack = 126;
trajan.fileName = "Data\\andrea2.trc";
trajan.loadScreen = "Screens\\andrea2.jpg";
trajan.horizon = true;
trajan.colAddHorizon = true;
trajan.layer1 = SkyLayer.new(120, 80, 50, -4);
trajan.storm = true;

Gameflow:AddLevel(trajan);

-- Colosseum
colosseum = Level.new();

colosseum.name = 103;
colosseum.script = "andrea3.lua";
colosseum.soundTrack = 118;
colosseum.fileName = "Data\\andrea3.trc";
colosseum.loadScreen = "Screens\\andrea3.jpg";
colosseum.horizon = true;
colosseum.colAddHorizon = true;
colosseum.layer1 = SkyLayer.new(120, 80, 50, -4);

Gameflow:AddLevel(colosseum);

-- The base
theBase = Level.new();

theBase.name = 104;
theBase.script = "joby2.lua";
theBase.soundTrack = 130;
theBase.fileName = "Data\\joby2.trc";
theBase.loadScreen = "Screens\\joby2.jpg";
theBase.horizon = true;
theBase.colAddHorizon = true;
theBase.layer1 = SkyLayer.new(120, 80, 50, -4);
theBase.resetHub = true;

Gameflow:AddLevel(theBase);

-- The submarine
submarine = Level.new();

submarine.name = 105;
submarine.script = "joby3.lua";
submarine.soundTrack = 121;
submarine.fileName = "Data\\joby3.trc";
submarine.loadScreen = "Screens\\joby3.jpg";
submarine.horizon = true;
submarine.colAddHorizon = true;
submarine.layer1 = SkyLayer.new(120, 80, 50, -4);

Gameflow:AddLevel(submarine);

-- Deepsea dive
deepsea = Level.new();

deepsea.name = 106;
deepsea.script = "joby4.lua";
deepsea.soundTrack = 127;
deepsea.fileName = "Data\\joby4.trc";
deepsea.loadScreen = "Screens\\joby4.jpg";
deepsea.horizon = true;
deepsea.colAddHorizon = true;
deepsea.layer1 = SkyLayer.new(120, 80, 50, -4);

Gameflow:AddLevel(deepsea);

-- Sinking submarine
sinking = Level.new();

sinking.name = 107;
sinking.script = "joby5.lua";
sinking.soundTrack = 121;
sinking.fileName = "Data\\joby5.trc";
sinking.loadScreen = "Screens\\joby5.jpg";
sinking.horizon = true;
sinking.colAddHorizon = true;
sinking.layer1 = SkyLayer.new(120, 80, 50, -4);

Gameflow:AddLevel(sinking);

-- Gallows tree
gallowstree = Level.new();

gallowstree.name = 108;
gallowstree.script = "andy1.lua";
gallowstree.soundTrack = 124;
gallowstree.fileName = "Data\\andy1.trc";
gallowstree.loadScreen = "Screens\\andy1.jpg";
gallowstree.horizon = true;
gallowstree.colAddHorizon = true;
gallowstree.rain = true;
gallowstree.storm = true;
gallowstree.laraType = LaraType.Young;
gallowstree.layer1 = SkyLayer.new(120, 80, 50, -4);
gallowstree.resetHub = true;

Gameflow:AddLevel(gallowstree);

-- Labyrinth
labyrinth = Level.new();

labyrinth.name = 109;
labyrinth.script = "andy2.lua";
labyrinth.soundTrack = 117;
labyrinth.fileName = "Data\\andy2.trc";
labyrinth.loadScreen = "Screens\\andy2.jpg";
labyrinth.horizon = true;
labyrinth.colAddHorizon = true;
labyrinth.rain = true;
labyrinth.storm = true;
labyrinth.laraType = LaraType.Young;
labyrinth.layer1 = SkyLayer.new(120, 80, 50, -4);

Gameflow:AddLevel(labyrinth);

-- Old mill
oldMill = Level.new();

oldMill.name = 110;
oldMill.script = "andy3.lua";
oldMill.soundTrack = 124;
oldMill.fileName = "Data\\andy3.trc";
oldMill.loadScreen = "Screens\\andy3.jpg";
oldMill.horizon = true;
oldMill.colAddHorizon = true;
oldMill.rain = true;
oldMill.storm = true;
oldMill.laraType = LaraType.Young;
oldMill.layer1 = SkyLayer.new(120, 80, 50, -4);

Gameflow:AddLevel(oldMill);

-- 13th floor
rich1 = Level.new();

rich1.name = 111;
rich1.script = "rich1.lua";
rich1.soundTrack = 124;
rich1.fileName = "Data\\rich1.trc";
rich1.loadScreen = "Screens\\rich1.jpg";
rich1.horizon = true;
rich1.colAddHorizon = true;
rich1.layer1 = SkyLayer.new(120, 80, 50, -4);
rich1.resetHub = true;

Gameflow:AddLevel(rich1);

-- Escape with the Iris
iris = Level.new();

iris.name = 112;
iris.script = "rich2.lua";
iris.soundTrack = 129;
iris.fileName = "Data\\rich2.trc";
iris.loadScreen = "Screens\\rich2.jpg";
iris.horizon = true;
iris.colAddHorizon = true;
iris.layer1 = SkyLayer.new(120, 80, 50, -4);

Gameflow:AddLevel(iris);

-- Security breach
sec = Level.new();

sec.name = 113;
sec.script = "rich3.lua";
sec.soundTrack = 129;
sec.fileName = "Data\\rich3.trc";
sec.loadScreen = "Screens\\rich2.jpg";
sec.horizon = true;
sec.colAddHorizon = true;
sec.layer1 = SkyLayer.new(120, 80, 50, -4);

Gameflow:AddLevel(sec);