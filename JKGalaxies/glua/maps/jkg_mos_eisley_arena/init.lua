--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Code
	Tatooine arena init
	
	Written by BobaFett
--------------------------------------------------]]

-- Spawn npcs
include("npcs.lua")


-- arena scoreboard etc
ArenaControls = ArenaControls or {}

include("scoreboard.lua")

local DisableTimers = false

local function ArenaTestCallback(ltime)
	if DisableTimers then
		return
	end
	ArenaControls.SetTime(ltime)
	ArenaControls.SetRedReinforcements(ltime)
	ArenaControls.SetBlueReinforcements(ltime)
	if ltime == -2 then
		return
	end
	timer.Simple(1000, ArenaTestCallback, ltime-1)
end

local function ArenaTestCallback2(ltime)
	if DisableTimers then
		return
	end
	ArenaControls.SetTime(ltime)
	if ltime == 0 then
		return
	end
	timer.Simple(1000, ArenaTestCallback2, ltime-1)
end

local function ArenaTestSpeedCallback(ltime)
	if DisableTimers then
		return
	end
	ArenaControls.SetTime(ltime)
	if ltime == 0 then
		timer.Simple(2000, ArenaTestSpeedCallback, ltime-1)
	elseif ltime == -1 then
		ArenaControls.SetRedReinforcements(-1)
		ArenaControls.SetBlueReinforcements(-1)
		ArenaControls.SetTime(-1)
		timer.Simple(1000, ArenaTestSpeedCallback, ltime-1)
	elseif ltime == -2 then
		ArenaControls.SetRedReinforcements(-2)
		ArenaControls.SetBlueReinforcements(-2)
		ArenaControls.SetTime(-2)
		return
	else
		timer.Simple(1, ArenaTestSpeedCallback, ltime-1)
	end
end

local function ArenaTestClockCallback(ltime)
	if DisableTimers then
		return
	end
	local remaining = ltime - sys.Time()
	if remaining < 0 then
		ArenaControls.SetRedReinforcements(-1)
		ArenaControls.SetBlueReinforcements(-1)
		ArenaControls.SetTime(-1)
		return
	else
		local secs = math.floor(remaining/1000)
		local msecs = (remaining - (secs*1000))/10
		ArenaControls.SetTime(secs)
		ArenaControls.SetBlueReinforcements(msecs)
		ArenaControls.SetRedReinforcements(msecs)
		timer.Simple(1, ArenaTestClockCallback, ltime)
	end
end

local function ArenaTest(ply, argc, argv)
	if argc<2 then
		ply:SendPrint("Usage: arenatest <program>\n\nValid programs:\ncountdown - starts a 2 min countdown to 0")
		return
	end
	local program = string.lower(argv[1])
	if program == "countdown" then
		ArenaControls.SetRedReinforcements(-3)
		ArenaControls.SetBlueReinforcements(-3)
		ArenaControls.SetTime(-3)
		DisableTimers = false
		timer.Simple(2000, ArenaTestCallback, 120)
	elseif program == "off" then
		DisableTimers = true
		ArenaControls.SetRedReinforcements(-2)
		ArenaControls.SetBlueReinforcements(-2)
		ArenaControls.SetTime(-2)
	elseif program == "dash" then
		DisableTimers = true
		ArenaControls.SetRedReinforcements(-1)
		ArenaControls.SetBlueReinforcements(-1)
		ArenaControls.SetTime(-1)
	elseif program == "normal" then
		ArenaControls.SetRedReinforcements(0)
		ArenaControls.SetBlueReinforcements(0)
		ArenaControls.S1 = 0
		ArenaControls.S2 = 0
		ArenaControls.SetTime(300)
		DisableTimers = false
		timer.Simple(2000, ArenaTestCallback2, 299)	
	elseif program == "normal10" then
		ArenaControls.SetRedReinforcements(0)
		ArenaControls.SetBlueReinforcements(0)
		ArenaControls.S1 = 0
		ArenaControls.S2 = 0
		ArenaControls.SetTime(600)
		DisableTimers = false
		timer.Simple(2000, ArenaTestCallback2, 566)	
	elseif program == "ptr" then
		ArenaControls.S1 = ArenaControls.S1 + 1
		ArenaControls.SetRedReinforcements(ArenaControls.S1)
	elseif program == "ptb" then
		ArenaControls.S2 = ArenaControls.S2 + 1
		ArenaControls.SetBlueReinforcements(ArenaControls.S2)
	elseif program == "ptc" then
		ArenaControls.S1 = 0
		ArenaControls.S2 = 0
		ArenaControls.SetRedReinforcements(0)
		ArenaControls.SetBlueReinforcements(0)
	elseif program == "speed" then
		ArenaControls.SetRedReinforcements(-1)
		ArenaControls.SetBlueReinforcements(-1)
		ArenaControls.SetTime(-3)
		DisableTimers = false
		timer.Simple(2000, ArenaTestSpeedCallback, 300)		
	elseif program == "clock" then
		ArenaControls.SetRedReinforcements(-3)
		ArenaControls.SetBlueReinforcements(-3)
		ArenaControls.SetTime(-3)
		DisableTimers = false
		timer.Simple(2000, ArenaTestClockCallback, sys.Time() + 60000)		
	end		
end

cmds.Add("arenatest", ArenaTest)
