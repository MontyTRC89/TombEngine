-- Main gameflow file

-- Trajan markets
trajan = Level.new();

trajan.name = 102;
trajan.script = "andrea2.lua";
trajan.soundtrack = 110;
trajan.filename = "andrea2.trc";
trajan.horizon = true;
trajan.coladdhorizon = true;
trajan.layer1 = SkyLayer.new(120, 80, 50, -4);

levels[2] = trajan; 