-----
--- Basic timer - after a specified number of seconds, the specified thing happens.
-- Example usage:
--	local Timer = require("Timer")
--
--	-- This will be called when the timer runs out
--	LevelFuncs.FinishTimer = function(healthWhenStarted, victoryMessage)
--		-- Open a door, display a message, make an explosion... whatever you wish
--		DoSomething(healthWhenStarted, victoryMessage)
--	end
--	
--	-- This function triggers the timer
--	LevelFuncs.TriggerTimer = function(obj) 
--		local myTimer = Timer.Create("my_timer",
--			5.0,
--			false,
--			{minutes = false, seconds = true, deciseconds = true},
--			"FinishTimer",
--			Lara:GetHP(),
--			"Well done!")
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

local unpausedColor = TEN.Color(255, 255, 255)
local pausedColor = TEN.Color(255, 255, 0)
local str = TEN.Strings.DisplayString("TIMER", 0, 0, unpausedColor, false, {TEN.Strings.DisplayStringOption.CENTER, TEN.Strings.DisplayStringOption.SHADOW} )

Timer = {
	--- Create (but do not start) a new timer.
	--
	-- You have the option of displaying the remaining time on the clock. Timer format details:
	--
	-- -- deciseconds are 1/10th of a second
	--	
	--	-- mins:secs
	--	local myTimeFormat1 = {minutes = true, seconds = true, deciseconds = false}
	--
	--	-- also mins:secs
	--	local myTimeFormat2 = {minutes = true, seconds = true}
	--	
	--	-- secs:decisecs
	--	local myTimeFormat3 = {seconds = true, deciseconds = true}
	--
	--	-- secs; also what is printed if you pass true instead of a table
	--	local myTimeFormat4 = {seconds = true}
	--
	--__At any given time, only one timer can show its countdown__.
	--
	--Use this sparingly; in the classics, timed challenges did not have visible countdowns. For shorter timers, the gameplay benefit from showing the remaining time might not be necessary, and could interfere with the atmosphere of the level.
	--
	-- @string name A label to give this timer; used to retrieve the timer later
	-- @number totalTime The duration of the timer, in seconds
	-- @bool loop if true, the timer will start again immediately after the time has elapsed
	-- @tparam ?table|bool timerFormat If a table is given, the remaining time will be shown as a string, formatted according to the values in the table. If true, the remaining seconds, rounded up, will show at the bottom of the screen. If false, the remaining time will not be shown on screen. 
	-- @string func The name of the LevelFunc member to call when the time is upssss
	-- @param[opt] ... a variable number of arguments with which the above function will be called
	-- @return The timer in its paused state
	--
	Create = function(name, totalTime, loop, timerFormat, func, ...)
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
		thisTimer.func = func
		thisTimer.funcArgs = {...}
		thisTimer.loop = loop
		thisTimer.active = false
		thisTimer.paused = true
		if type(timerFormat) == "table" then
			thisTimer.timerFormat = timerFormat
		elseif timerFormat then
			thisTimer.timerFormat = {seconds = true}
		end
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

		thisTimer.paused = false

		if thisTimer.timerFormat then
			str:SetColor(unpausedColor)
		end
	end;

	--- Pause the timer. If showing the remaining time on-screen, its color will be set to yellow.
	-- @param t the timer in question
	Pause = function(t)
		local thisTimer = LevelVars.__TEN_timer.timers[t.name]
		thisTimer.paused = true
		if thisTimer.timerFormat then
			str:SetColor(pausedColor)
		end
	end,

	--- Stop the timer.
	-- @param t the timer in question
	Stop = function(t)
		LevelVars.__TEN_timer.timers[t.name].active = false
	end,
	
	--- Get whether or not the timer is active
	-- @param t the timer in question
	-- @return true if the timer is active, false if otherwise
	IsActive = function(t)
		return LevelVars.__TEN_timer.timers[t.name].active
	end,

	--- Get whether or not the timer is paused
	-- @param t the timer in question
	-- @return true if the timer is paused, false if otherwise
	IsPaused = function(t)
		return LevelVars.__TEN_timer.timers[t.name].paused
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

			if t.timerFormat then
				TEN.Strings.HideString(str)

				local fmt = ""
				local remaining = math.max(t.remainingTime, 0)

				local round = math.floor
				local subSecond = remaining - math.floor(remaining)

				local fmtBefore = false

				-- deciseconds
				if t.timerFormat.deciseconds then
					fmt = math.floor(10*subSecond)
					fmtBefore = true
				end

				-- seconds
				if t.timerFormat.seconds then
					if not fmtBefore then
						round = math.ceil
					else
						round = math.floor
						fmt = ":" .. fmt
					end
					local roundedSeconds = round(remaining)
					local toBeDisplayed = roundedSeconds
					if t.timerFormat.minutes then
						toBeDisplayed = roundedSeconds % 60
					end
					fmt = string.format("%02d", toBeDisplayed) .. fmt

					remaining = roundedSeconds 
					fmtBefore = true
				end

				-- minutes
				if t.timerFormat.minutes then
					if not fmtBefore then
						round = math.ceil
					else
						round = math.floor
						fmt = ":" .. fmt
					end

					local roundedMinutes = round(remaining/60)
					local toBeDisplayed = roundedMinutes

					fmt = string.format("%02d", toBeDisplayed) .. fmt
					fmtBefore = true
				end

				str:SetKey(fmt)
				local myX, myY = PercentToScreen(50, 90)
				str:SetPosition(myX, myY)

				-- Do this again in case the player has loaded while the timer was paused already
				-- Need a better solution for this
				if t.paused then
					str:SetColor(pausedColor)
				end

				TEN.Strings.ShowString(str, 1)
			end

		end
	end;

}

return Timer

