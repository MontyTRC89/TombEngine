-----
--- Basic timer - after a specified number of seconds, the specified thing happens.
-- Usage:
--	timer = require("Timer")
--
--	LevelFuncs.FinishTimer = function(healthWhenStarted, victoryMessage)
--		DoSomething(healthWhenStarted, victoryMessage)
--	end
--	
--	LevelFuncs.TriggerTimer = function(obj) 
--		local myTimer = Timer.Create("my_timer", 5.0, false, "FinishTimer", Lara:GetHP(), "Well done!")
--		myTimer:Start()
--	end
--
-- @luautil Timer

LevelVars.__TEN_timer = {timers = {}}

local Timer

Timer = {
	--- Create (but do not start) a new timer.
	-- @string name A label to give this timer; used to retrieve the timer later
	-- @number totalTime The duration of the timer, in seconds
	-- @bool loop if true, the timer will start again immediately after the time has elapsed
	-- @string func The name of the LevelFunc member to call when the time is up
	-- @param funcArgs the arguments with which the above function will be called
	-- @return The timer in its paused state
	Create = function(name, totalTime, loop, func, ...)
		local obj = {}
		local mt = {}
		mt.__index = Timer
		setmetatable(obj, mt)

		obj.name = name
		LevelVars.__TEN_timer.timers[name] ={} 
		local thisTimer = LevelVars.__TEN_timer.timers[name]
		thisTimer.name = name
		thisTimer.totalTime = totalTime
		thisTimer.remainingTime = totalTime
		thisTimer.func = func -- todo save name?
		thisTimer.funcArgs = {...}
		thisTimer.loop = loop
		thisTimer.active = false
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

	--- Begin or unpause a timer.
	-- @param t the timer in question
	Start = function(t)
		local thisTimer = LevelVars.__TEN_timer.timers[t.name]
		if not thisTimer.active then
			thisTimer.active = true
		end
	end;

	--- Pause the timer
	-- @param t the timer in question
	Pause = function(t)
		LevelVars.__TEN_timer.timers[t.name].active = false
	end,

	--- Get the remaining time for a timer
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

	--- Get the total time for a timer
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
		for k, t in pairs(LevelVars.__TEN_timer.timers) do
			Timer.Update(t, dt)
		end
	end;

	Update = function(t, dt)
		if t.active then
			t.remainingTime = t.remainingTime - dt

			if t.remainingTime <= 0 then
				LevelFuncs[t.func](table.unpack(t.funcArgs))

				local timeCarryOver = t.remainingTime

				if not t.loop then
					t.active = false
					timeCarryOver = 0
				end

				t.remainingTime = t.totalTime + timeCarryOver
			end
		end
	end;

}

return Timer

