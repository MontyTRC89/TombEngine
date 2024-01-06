-- New level script file.
-- To include other script files, you can use require("filename") command.

-- Called when entering a level, either after leveljump, new game or loading game
LevelFuncs.OnStart = function() end

-- Called after loading from a save
LevelFuncs.OnLoad = function() end

-- Called after saving game
LevelFuncs.OnSave = function() end

-- Called on every frame of the game
-- dt stands for "delta time", and holds the time in seconds since the last call to OnControlPhase
LevelFuncs.OnControlPhase = function(dt) end

-- Called when level is ended, either after leveljump, quitting to title or loading game
LevelFuncs.OnEnd = function() end


-- An example function which prints a string and leaves it on screen for 1 second.
-- Argument should be typed in TE trigger manager window's argument text field.

LevelFuncs.PrintText = function(Triggerer, Argument)
    local TestText = DisplayString(Argument, 100, 100, Color.new(250,250,250))
    ShowString(TestText, 1)
end
