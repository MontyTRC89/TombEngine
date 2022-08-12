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


-- Another example function which emits rotating electric halo around object,
-- which triggered it.

local currentX = 0.0
local currentY = 0.0

LevelFuncs.EmitHaloOnActionPush = function(Triggerer) 
    
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
	
	-- First argument which is passed to function that is triggered from TE volumes is
	-- always an object which triggered it (except cases when triggerer is static mesh or
	-- flyby camera). We can directly use Triggerer argument to get some properties from
	-- it. In this case, we get position of a root joint (in case of Lara, it is her hips).
	
    local pos = Triggerer:GetJointPosition(0)

	-- math.random() is an internal Lua method which returns a value between 2 specified limits,
	-- in our case something between 200 and 255. We use it to vary halo intensity a bit.
	
    local color = math.random(200, 255)
	
    local velocity = Vec3(0, 0, 0) -- No velocity	
	
	-- Again, we can use velocity to get some value between 60 and 80 to vary rotation rate of
	-- a spawned halo particle.
	
    local rot = math.random(60, 80)

	-- circleLength is standard mathematical constant for circle length which is later used to
	-- get sin/cos value to rotate light and particle around an object.

    local circleLength = 3.14 * 2.0
    
	-- Progress currentX and currentY variables to change current X and Y positions of the halo.
	
    currentX = currentX + 0.2
    currentY = currentY + 0.1
    
	-- Here we clamp currentX and currentY values to max circle length, because sin/cos operations
	-- can't operate out of circle length range.
	
    if (currentX > circleLength) then
        currentX = currentX - circleLength;
    end
        
    if (currentY > circleLength) then
        currentY = currentY - circleLength;
    end
    
	local horizontalAmplitude = 384  -- 3 clicks height, where effect wanders about.
	local haloRotationDistance = 256 -- rotate on distance 2 clicks around object.
	
	-- Calculate relative offset of a halo using simple sin/cos functions.
	
    local offsetX = math.cos(currentX) * haloRotationDistance
    local offsetZ = math.sin(currentX) * haloRotationDistance
    
    local offsetY = math.sin(currentY) * horizontalAmplitude
    
	-- Add relative offsets to joint position.
	
    pos.x = pos.x + offsetX
    pos.y = pos.y + offsetY
    pos.z = pos.z + offsetZ
	
	-- Play electrical sound.
	
	Misc.PlaySound(197, pos)
	
	-- Emit particle and light. Look into manual for list of arguments.
	
    Effects.EmitParticle(pos, velocity, 2, 1, rot, Color.new(color * 0.5, color * 0.5, color), Color.new(color * 0.2, color * 0.1, color), 2, 16, 64, 1, false, false)
    Effects.EmitLight(pos, Color.new(color * 0.5, color * 0.5, color), 7)
	
end