local Timer = require("Timer")

local EventSequence

LevelVars.__TEN_eventSequence = {sequences = {}}

EventSequence = {
	Create = function(name, showString, ...)
		local obj = {}
		local mt = {}
		mt.__index = EventSequence
		setmetatable(obj, mt)

		obj.name = name

		LevelVars.__TEN_eventSequence.sequences[name] ={} 
		local thisES = LevelVars.__TEN_eventSequence.sequences[name]
		thisES.name = name
		thisES.timesFuncsAndArgs = {...}
		
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

			if type(funcAndArgs) == "string" then
				-- we only have a function
				func = funcAndArgs
				funcAndArgs = {}
			else
				-- we have a function and possible args
				func = table.remove(funcAndArgs, 1)
			end

			if nextTimer < #tfa then
				-- This function must start next timer 
				-- AND do its function
				LevelFuncs[funcName] = function(...)
					LevelFuncs[func](...)
					Timer.Get(nextTimerName):Start()
					thisES.currentTimer = timerIndex + 1
				end
			else
				-- final timer
				LevelFuncs[funcName] = function(...)
					LevelFuncs[func](...)
					Timer.Get(timerName):Stop()
					thisES.currentTimer = 1
				end
			end

			local thisTimer = Timer.Create(timerName,
						tfa[i], -- time
						false,
						showString,
						funcName,
						funcAndArgs -- now with func removed
						)

			thisES.timers[timerIndex] = timerName
		end

		return obj
	end;

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
	end,

	Pause = function(t)
		local thisES = LevelVars.__TEN_eventSequence.sequences[t.name]
		Timer.Get(thisES.timers[thisES.currentTimer]):Pause()
	end,

	Stop = function(t)
		local thisES = LevelVars.__TEN_eventSequence.sequences[t.name]
		Timer.Get(thisES.timers[thisES.currentTimer]):Stop()
	end,

	Start = function(t)
		local thisES = LevelVars.__TEN_eventSequence.sequences[t.name]
		Timer.Get(thisES.timers[thisES.currentTimer]):Start()
	end;
}

return EventSequence
