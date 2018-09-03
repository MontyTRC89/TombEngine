-- TR5Main Gameflow file
-- Created by MontyTRC
-- Place in this LUA script all the levels of your game
-- Title is mandatory and must be the first level

-- Title level
title = Level.new();

title.script = "andrea2.lua";
title.soundtrack = 110;
title.filename = "Data\\title.trc";
title.horizon = true;
title.coladdhorizon = true;
title.layer1 = SkyLayer.new(120, 80, 50, -4);
title.storm = true;
title.background = "title.png";

Gameflow:addLevel(title);

-- Streets of rome
streets = Level.new();

streets.name = 101;
streets.script = "andrea1.lua";
streets.soundtrack = 128;
streets.filename = "Data\\andrea1.trc";
streets.horizon = true;
streets.coladdhorizon = true;
streets.layer1 = SkyLayer.new(120, 80, 50, -4);

Gameflow:addLevel(streets);

-- Trajan markets
trajan = Level.new();

trajan.name = 102;
trajan.script = "andrea2.lua";
trajan.soundtrack = 126;
trajan.filename = "Data\\andrea2.trc";
trajan.horizon = true;
trajan.coladdhorizon = true;
trajan.layer1 = SkyLayer.new(120, 80, 50, -4);
trajan.storm = true;

Gameflow:addLevel(trajan);

-- Colosseum
colosseum = Level.new();

colosseum.name = 103;
colosseum.script = "andrea3.lua";
colosseum.soundtrack = 118;
colosseum.filename = "Data\\andrea3.trc";
colosseum.horizon = true;
colosseum.coladdhorizon = true;
colosseum.layer1 = SkyLayer.new(120, 80, 50, -4);

Gameflow:addLevel(colosseum);

-- The base
theBase = Level.new();

theBase.name = 104;
theBase.script = "joby2.lua";
theBase.soundtrack = 130;
theBase.filename = "Data\\joby2.trc";
theBase.horizon = true;
theBase.coladdhorizon = true;
theBase.layer1 = SkyLayer.new(120, 80, 50, -4);

Gameflow:addLevel(theBase);

-- The submarine
submarine = Level.new();

submarine.name = 105;
submarine.script = "joby3.lua";
submarine.soundtrack = 121;
submarine.filename = "Data\\joby3.trc";
submarine.horizon = true;
submarine.coladdhorizon = true;
submarine.layer1 = SkyLayer.new(120, 80, 50, -4);

Gameflow:addLevel(submarine);

-- Deepsea dive
deepsea = Level.new();

deepsea.name = 106;
deepsea.script = "joby4.lua";
deepsea.soundtrack = 127;
deepsea.filename = "Data\\joby4.trc";
deepsea.horizon = true;
deepsea.coladdhorizon = true;
deepsea.layer1 = SkyLayer.new(120, 80, 50, -4);

Gameflow:addLevel(deepsea);

-- Sinking submarine
sinking = Level.new();

sinking.name = 107;
sinking.script = "joby5.lua";
sinking.soundtrack = 121;
sinking.filename = "Data\\joby5.trc";
sinking.horizon = true;
sinking.coladdhorizon = true;
sinking.layer1 = SkyLayer.new(120, 80, 50, -4);

Gameflow:addLevel(sinking);

-- Gallows tree
gallowstree = Level.new();

gallowstree.name = 108;
gallowstree.script = "andy1.lua";
gallowstree.soundtrack = 124;
gallowstree.filename = "Data\\andy1.trc";
gallowstree.horizon = true;
gallowstree.coladdhorizon = true;
gallowstree.rain = true;
gallowstree.storm = true;
gallowstree.layer1 = SkyLayer.new(120, 80, 50, -4);

Gameflow:addLevel(gallowstree);