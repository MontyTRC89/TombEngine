-----
--- Basic timer - after a specified number of seconds, the specified thing happens.
-- Usage:
--	local Timer = require("Timer")
--
--	LevelFuncs.FinishTimer = function(healthWhenStarted, victoryMessage)
--		DoSomething(healthWhenStarted, victoryMessage)
--	end
--	
--	LevelFuncs.TriggerTimer = function(obj) 
--		local myTimer = Timer.Create("my_timer", 5.0, false, true, "FinishTimer", Lara:GetHP(), "Well done!")
--		myTimer:Start()
--	end
--
--	LevelFuncs.OnControlPhase = function(dt)
--		Timer.UpdateAll(dt)
--	end
--
-- @luautil Timer

LevelVars.__TEN_timer = {timers = {}}

local Timer

local unpausedColor = Color(255, 255, 255)
local pausedColor = Color(255, 255, 0)
local str = TEN.Strings.DisplayString("TIMER", 0, 0, unpausedColor, false, {TEN.Strings.DisplayStringOption.CENTER, TEN.Strings.DisplayStringOption.SHADOW} )

Timer = {
	--- Create (but do not start) a new timer.
	-- @string name A label to give this timer; used to retrieve the timer later
	-- @number totalTime The duration of the timer, in seconds
	-- @bool loop if true, the timer will start again immediately after the time has elapsed
	-- @bool showString if true, the remaining time, rounded up, will show at the bottom of the screen. __At any given time, only one timer can show its remaining time__.
	-- @string func The name of the LevelFunc member to call when the time is up
	-- @param funcArgs the arguments with which the above function will be called
	-- @return The timer in its paused state
	Create = function(name, totalTime, loop, showString, func, ...)
		local obj = {}
		local mt = {}
		mt.__index = Timer
		setmetatable(obj, mt)

		obj.name = name
		LevelVars.__TEN_timer.timers[name] ={} 
		local thisTimer = LevelVars.__TEN_timer.timers[name]
		thisTimer.name = name
		thisTimer.showString = showString
		thisTimer.totalTime = totalTime
		thisTimer.remainingTime = totalTime
		thisTimer.func = func
		thisTimer.funcArgs = {...}
		thisTimer.loop = loop
		thisTimer.active = false
		thisTimer.paused = true
		return obj
	end;

	--- Get a timer by its name.
	-- @string name The label that was given to the timer when it was created
	-- @return The timer
	Get = function(name)
		if LevelVars.__TEN_timer.timers[name] then
			local obj = {}
			local mt = {}
			mt.__index = Timer
			setmetatable(obj, mt)
			obj.name = name
			return obj
		end
		return nil
	end,

	--- Give the timer a new function and args
	-- @param t the timer in question
	-- @string func The name of the LevelFunc member to call when the time is up
	-- @param funcArgs the arguments with which the above function will be called
	SetFunction = function(t, func, ...)
		local thisTimer = LevelVars.__TEN_timer.timers[t.name]
		thisTimer.func = func
		thisTimer.funcArgs = {...}
	end,

	--- Begin or unpause a timer. If showing the remaining time on-screen, its color will be set to white.
	-- @param t the timer in question
	Start = function(t)
		local thisTimer = LevelVars.__TEN_timer.timers[t.name]
		if not thisTimer.active then
			thisTimer.active = true
		end

		LevelVars.__TEN_timer.timers[t.name].paused = false

		if thisTimer.showString then
			str:SetColor(unpausedColor)
		end
	end;

	--- Pause the timer. If showing the remaining time on-screen, its color will be set to yellow.
	-- @param t the timer in question
	Pause = function(t)
		local thisTimer = LevelVars.__TEN_timer.timers[t.name]
		thisTimer.paused = true
		if thisTimer.showString then
			str:SetColor(pausedColor)
		end
	end,

	--- Stop the timer.
	-- @param t the timer in question
	Stop = function(t)
		LevelVars.__TEN_timer.timers[t.name].active = false
	end,
	
	--- Get the remaining time for a timer.
	-- @param t the timer in question
	-- @return the time in seconds remaining on the clock
	GetRemainingTime = function(t)
		return LevelVars.__TEN_timer.timers[t.name].remainingTime
	end,

	--- Set the remaining time for a timer
	-- @param t the timer in question
	-- @number remainingTime the new time remaining for the timer
	SetRemainingTime = function(t, remainingTime)
		LevelVars.__TEN_timer.timers[t.name].remainingTime = remainingTime
	end,

	--- Get the total time for a timer.
	-- This is the amount of time the timer will start with, as well as when starting a new loop
	-- @param t the timer in question
	-- @return the timer's total time
	GetTotalTime = function(t)
		return LevelVars.__TEN_timer.timers[t.name].totalTime
	end,

	--- Set the total time for a timer
	-- @param t the timer in question
	-- @number totalTime timer's new total time
	SetTotalTime = function(t, totalTime)
		LevelVars.__TEN_timer.timers[t.name].totalTime = totalTime
	end,

	--- Set whether or not the timer loops 
	-- @param t the timer in question
	-- @bool looping whether or not the timer loops
	SetLooping = function(t, looping)
		LevelVars.__TEN_timer.timers[t.name].loop = looping
	end,

	--- Update all active timers.
	-- Should be called in LevelFuncs.OnControlPhase
	-- @number dt The time in seconds since the last frame
	UpdateAll = function(dt)
		for _, t in pairs(LevelVars.__TEN_timer.timers) do
			Timer.Update(t, dt)
		end
	end;

	Update = function(t, dt)
		if t.active then
			if not t.paused then
				t.remainingTime = t.remainingTime - dt

				if t.remainingTime <= 0 then
					LevelFuncs[t.func](table.unpack(t.funcArgs))

					if not t.loop then
						t.active = false
					else
						t.remainingTime = t.remainingTime + t.totalTime
					end
				end
			end

			if t.showString then
				TEN.Strings.HideString(str)
				str:SetKey(tostring(math.ceil(t.remainingTime)))
				local myX, myY = PercentToScreen(50, 90)
				str:SetPosition(myX, myY)
				TEN.Strings.ShowString(str, 1)
			end

		end
	end;

}

return Timer

