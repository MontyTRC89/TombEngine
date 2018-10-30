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

streets.onLoad = function()
	TR:PlayFMV("Data\\video1.avi");
end

streets.onFinish = function()
	
end

Gameflow:AddLevel(streets);