-----
--- Event sequence - a chain of functions to call at specified times, modeled after TRNG's organizers.
--
-- Works atop the Timer, and so is updated automatically pre-OnControlPhase, and saved automatically when the game is saved.
--
-- Example usage:
--	local EventSequence = require("EventSequence")
--
--	-- These will be called by the sequence
--	LevelFuncs.HealLara = function()
--		Lara:SetHP(Lara:GetHP()+10)
--	end
--	
--	local nSpawned = 0
--	LevelFuncs.SpawnBaddy = function(baddy, name, pos)
--		local myBaddy = TEN.Objects.Moveable(baddy, name..nSpawned, pos, nil, 0)
--		myBaddy:Enable()
--		nSpawned = nSpawned + 1
--	end
--
--	-- This function triggers the sequence
--	LevelFuncs.TriggerSequence = function(obj) 
--		local posSteve = TEN.Objects.GetMoveableByName("stevePosNullmesh"):GetPosition()
--		local posChris = TEN.Objects.GetMoveableByName("chrisPosNullmesh"):GetPosition()
--		local mySeq = EventSequence.Create("my_seq",
--			false, -- does not loop
--			{seconds = true, deciseconds = true}, -- timer format, see Timer for details
--			6, -- seconds until call the function specified in next arg 
--			"HealLara", -- first function to call. If we don't need to pass any arguments, we can just give the func name as a string
--			2.1, -- seconds until the next function, after the previous one has been called
--			{"SpawnBaddy", TEN.Objects.ObjID.BADDY1, "steve", posSteve}, -- if we DO want to pass arguments to the function to be called, we give a table with the name of the function ("SpawnBaddy" in this case) followed by the args to pass to it
--			0.5,
--			{"SpawnBaddy", TEN.Objects.ObjID.SAS_CAIRO, "chris", posChris},
--			1,
--			"HealLara")
--
--		-- event sequences are inactive to begin with and so need to be started
--		mySeq:Start()
--	end
--
-- @luautil EventSequence

local Timer = require("Timer")

local EventSequence

LevelVars.__TEN_eventSequence = {sequences = {}}

LevelFuncs.__TEN_eventSequence_callNext = function(sequenceName, nextTimerName, func, ...)
	local thisES = LevelVars.__TEN_eventSequence.sequences[sequenceName]
	LevelFuncs[func](...)

	thisES.currentTimer = thisES.currentTimer + 1
	if thisES.currentTimer <= #thisES.timers then
		local theTimer = Timer.Get(nextTimerName)
		theTimer:SetRemainingTime(theTimer:GetTotalTime())
		theTimer:Start()
	elseif thisES.loop then
		local theTimer = Timer.Get(thisES.firstTimerName)
		theTimer:SetRemainingTime(theTimer:GetTotalTime())
		theTimer:Start()
		thisES.currentTimer = 1
	else
		thisES.currentTimer = 1
	end
end

EventSequence = {
	--- Create (but do not start) a new event sequence.
	--
	-- @string name A label to give the sequence; used to retrieve the timer later as well as internally by TEN.
	-- @bool loop if true, the sequence will start again from its first timer once its final function has been called
	-- @tparam ?table|bool timerFormat same as in Timer. This is mainly for debugging. __This will not work properly if another sequence or timer is showing a countdown.__
	-- @param[opt] ... a variable number of pairs of arguments - a time in seconds, followed by the function (must be defined in the LevelFuncs table) to call once the time has elapsed, followed by another duration in seconds, another function name, etc. You can specify a function either by its name as a string, or by a table with the function name as the first member, followed by its arguments (see above example).
	-- @return The inactive sequence.
	Create = function(name, loop, timerFormat, ...)
		local obj = {}
		local mt = {}
		mt.__index = EventSequence
		setmetatable(obj, mt)

		obj.name = name

		LevelVars.__TEN_eventSequence.sequences[name] = {} 
		local thisES = LevelVars.__TEN_eventSequence.sequences[name]
		thisES.name = name
		thisES.timesFuncsAndArgs = {...}
		thisES.loop = loop
		
		local tfa = thisES.timesFuncsAndArgs
		thisES.timers = {}
		thisES.currentTimer = 1
		local prevTimer = nil
		local prevFuncName = nil

		for i = 1, #tfa, 2 do
			local nextTimer = i + 2
			local timerIndex = #thisES.timers + 1 

			local funcName = "__TEN_eventSequence_" .. name .. "_func" .. timerIndex 
			local timerName = "__TEN_eventSequence_" .. name .. "_timer" .. timerIndex 
			local nextTimerName = "__TEN_eventSequence_" .. name .. "_timer" .. timerIndex + 1

			local funcAndArgs = tfa[i+1]
			local func

			if i == 1 then
				thisES.firstTimerName = timerName
			end

			if type(funcAndArgs) == "string" then
				-- we only have a function
				func = funcAndArgs
				funcAndArgs = {}
			else
				-- we have a function and possible args
				func = table.remove(funcAndArgs, 1)
			end

			local thisTimer = Timer.Create(timerName,
						tfa[i], -- time
						false,
						timerFormat,
						"__TEN_eventSequence_callNext",
						name,
						nextTimerName,
						func,
						table.unpack(funcAndArgs) -- now with func removed
						)

			thisES.timers[timerIndex] = timerName
		end

		return obj
	end;

	--- Get an event sequence by its name.
	-- @string name The label that was given to the sequence when it was created
	-- @return The sequence
	Get = function(name)
		if LevelVars.__TEN_eventSequence.sequences[name] then
			local obj = {}
			local mt = {}
			mt.__index = EventSequence
			setmetatable(obj, mt)
			obj.name = name
			return obj
		end
		return nil
	end;

	--- Pause or unpause the sequence. If showing the remaining time on-screen, its color will be set to yellow (paused) or white (unpaused).
	-- @function mySequence:SetPaused
	-- @bool p if true, the sequence will be paused; if false, it will be unpaused 
	SetPaused = function(t, p)
		local thisES = LevelVars.__TEN_eventSequence.sequences[t.name]
		Timer.Get(thisES.timers[thisES.currentTimer]):SetPaused(p)
	end;

	--- Get whether or not the sequence is paused
	-- @function mySequence:IsPaused
	-- @return true if the timer is paused, false if otherwise
	IsPaused = function(t)
		local thisES = LevelVars.__TEN_eventSequence.sequences[t.name]
		return Timer.Get(thisES.timers[thisES.currentTimer]):IsPaused()
	end;
	
	--- Begin or unpause a sequence. If showing the remaining time on-screen, its color will be set to white.
	-- @function mySequence:Start
	Start = function(t)
		local thisES = LevelVars.__TEN_eventSequence.sequences[t.name]
		Timer.Get(thisES.timers[thisES.currentTimer]):Start()
	end;

	--- Stop the sequence.
	--@function mySequence:Stop
	Stop = function(t)
		local thisES = LevelVars.__TEN_eventSequence.sequences[t.name]
		Timer.Get(thisES.timers[thisES.currentTimer]):Stop()
	end;

	--- Get whether or not the sequence is active
	-- @function mySequence:IsActive
	-- @return true if the sequence is active, false if otherwise
	IsActive = function(t)
		local thisES = LevelVars.__TEN_eventSequence.sequences[t.name]
		return Timer.Get(thisES.timers[thisES.currentTimer]):IsActive()
	end;
}

return EventSequence
