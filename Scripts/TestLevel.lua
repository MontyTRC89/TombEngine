-- Test level script file

local Util = require("Util")
Util.ShortenTENCalls()

LevelFuncs.OnLoad = function() end
LevelFuncs.OnSave = function() end
LevelFuncs.OnControlPhase = function() end
LevelFuncs.OnEnd = function() end

-- An example function which prints a string and leaves it on screen for 1 second.
-- Argument should be typed in TE trigger manager window's argument text field.

LevelFuncs.PrintText = function(Triggerer, Argument)
    local TestText = DisplayString(Argument, 100, 100, Color.new(250,250,250))
    ShowString(TestText, 1)
end


-- Another example function which emits rotating electric halo around Lara, when
-- action key is pressed.

local currentX = 0.0
local currentY = 0.0

LevelFuncs.OnControlPhase = function() 
    
	-- This is a list of all possible keys which can be checked for their pushed/not pushed state.
	-- Later we will move them to separate internal file or make them internal TEN constants.
	
    local Keys =
    {
        Forward = 0,
        Back = 1,
        Left = 2,
        Right = 3,
        Crouch = 4,
        Sprint = 5,
        Walk = 6,
        Jump = 7, 
        Action = 8,
        Draw = 9,
        Flare = 10,
        Look = 11,
        Roll = 12,
        Inventory = 13,
        Pause = 14,
        StepLeft = 15,
        StepRight = 16
    }
	
	-- Your Lara in your level should have Lua name "lara".

    local pos = GetMoveableByName("lara"):GetJointPosition(0)

    local color = math.random(200, 255)

    local vel = Vec3(0, 0, 0)
    local rot = math.random(60, 80) * math.random(-1, 1)

    local circleLength = 3.14 * 2.0
    
    currentX = currentX + 0.2
    currentY = currentY + 0.1
    
    if (currentX > circleLength) then
        currentX = currentX - circleLength;
    end
        
    if (currentY > circleLength) then
        currentY = currentY - circleLength;
    end
    
    local partX = math.cos(currentX) * 256
    local partZ = math.sin(currentX) * 256
    
    local partY = math.sin(currentY) * 384
    
    pos.x = pos.x + partX
    pos.y = pos.y + partY
    pos.z = pos.z + partZ
    
    if (KeyIsHit(Keys.Action)) then
        Misc.PlaySound(198, pos)
		Misc.Vibrate(0.5, 0.1)
    end
    
    if (KeyIsHeld(Keys.Action)) then
        Misc.PlaySound(197, pos)
        Effects.EmitParticle(pos, vel, 5, 1, rot, Color.new(color * 0.5, color * 0.5, color), Color.new(color * 0.2, color * 0.1, color), 2, 16, 64, 32, false, false)
        Effects.EmitLight(pos, Color.new(color * 0.5, color * 0.5, color), 7)
    end
end