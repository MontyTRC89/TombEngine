local Timer

LevelVars.__TEN_timer = {timers = {}}

-- todo: var args for func, timers that loop
Timer = {
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

	Pause = function(t)
		LevelVars.__TEN_timer.timers[t.name].active = false
	end,

	GetRemainingTime = function(t)
		return LevelVars.__TEN_timer.timers[t.name].remainingTime
	end,

	SetRemainingTime = function(t, remainingTime)
		LevelVars.__TEN_timer.timers[t.name].remainingTime = remainingTime
	end,

	GetTotalTime = function(t)
		return LevelVars.__TEN_timer.timers[t.name].totalTime
	end,

	SetTotalTime = function(t, totalTime)
		LevelVars.__TEN_timer.timers[t.name].totalTime = totalTime
	end,

	SetLooping = function(t, looping)
		LevelVars.__TEN_timer.timers[t.name].loop = looping
	end,

	UpdateAll = function(dt)
		for k, t in pairs(LevelVars.__TEN_timer.timers) do
			Timer.Update(t, dt)
		end
	end;

	Update = function(t, dt)
		print(t.remainingTime)
		if t.active then
			t.remainingTime = t.remainingTime - dt

			if t.remainingTime <= 0 then
				LevelFuncs[t.func](table.unpack(t.funcArgs))
				if not t.loop then
					t.active = false
				end
				t.remainingTime = t.totalTime
			end
		end
	end;

	Start = function(t)
		local thisTimer = LevelVars.__TEN_timer.timers[t.name]
		if not thisTimer.active then
			thisTimer.active = true
		end
	end;
}

return Timer

