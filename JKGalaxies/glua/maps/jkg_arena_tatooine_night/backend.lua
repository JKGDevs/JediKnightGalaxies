--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Code
	Tatooine arena backend system
	
	Written by BobaFett
--------------------------------------------------]]

-- Namespace for the arena backend
JKG.ArenaBackend = {}

local backend = JKG.ArenaBackend -- Faster and easier access

-- Match stages
local MS_NONE = 0			-- No matches in progress
local MS_AWAITINGPLAYERS = 1 -- Players are being summoned to the waiting rooms
local MS_STARTING = 2		-- Players are in the waiting rooms, doing 10 sec countdown
local MS_INPROGRESS = 3		-- The match is in progress
local MS_FINISHED = 4		-- The match finished, people can now leave the arena

-- Match Game Modes
local MGM_NONE = 0
local MGM_REINFORCEMENTS = 1
local MGM_KILLS = 2
local MGM_MAX = 3

backend.Controls = {} -- Controls table (used to operate the scoreboards)
include("scoreboard.lua")

backend.Events = {}	-- Events table
include("events.lua")

backend.Display = {}
include("dispctrl.lua")

backend.Threads = {}
include("threads.lua")

-- Backend data
backend.Data = {
	MainArena = {
		Active = false,			-- Arena in use?
		-- Participants
		RedTeam = {},			-- Members of the red team
		BlueTeam = {},			-- Members of the blue team
		-- Match data
		GameMode = MGM_NONE,	-- Game type
		ReinfCount = 0,			-- Amount of reinforcements to start with
		KillLimit = 0,			-- Kill limit
		TimeLimit = 0,			-- Time limit
		-- Stage
		Stage = MS_NONE,		-- Stage of the match
		-- Game data
		RedScore = 0,			-- Red score (or reinforcements)
		BlueScore = 0,			-- Blue score (or reinforcements)
		TimeLeft = 0,			-- Time remaining
	},
	MainArenaPending = {		-- Pending match for the main arena
		Active = false,			-- Match pending?
		Ready = false,			-- Ready to start?
		-- Participants
		RedTeam = {},			-- Members of the red team
		BlueTeam = {},			-- Members of the blue team
		-- Match data
		GameMode = MGM_NONE,	-- Game type
		ReinfCount = 0,			-- Reinforcement count
		KillLimit = 0,			-- Kill limit
		TimeLimit = 0,			-- Time limit
	},
	SoloArena = {
		Active = false,			-- Solo arena in use?
		Player1 = nil,			-- Player 1
		Player2 = nil,			-- Player 2
		Stage = MS_NONE,		-- Stage of the match
	},
	SoloArenaPending = {
		Active = false,			-- Match pending?
		Player1 = nil,			-- Player 1
		Player2 = nil,			-- Player 2
	},
}
-- Quick access
local data = backend.Data


--[[--------------------------------------------------------------------------------------------------------------
	backend.GetInUse(arena)
	
	Returns whether the arena is in use or not
	
	Params: arena - Arena to get the status from (1 = main arena, 2 = 1v1 arena)
--]]--------------------------------------------------------------------------------------------------------------
function backend.GetInUse(arena)
	if arena == 1 then
		-- Main arena
		return data.MainArena.Active
	else
		return data.SoloArena.Active
	end
end

--[[--------------------------------------------------------------------------------------------------------------
	backend.GetInUse(arena)
	
	Returns whether the arena is in use or not
	
	Params: arena - Arena to get the status from (1 = main arena, 2 = 1v1 arena)
--]]--------------------------------------------------------------------------------------------------------------
function backend.GetStage(arena)
	if arena == 1 then
		-- Main arena
		return data.MainArena.Stage
	else
		return data.SoloArena.Stage
	end
end


--[[--------------------------------------------------------------------------------------------------------------
	backend.GetPending(arena)
	
	Returns whether the arena has a pending match
	
	Params: arena - Arena to get the status from (1 = main arena, 2 = 1v1 arena)
--]]--------------------------------------------------------------------------------------------------------------
function backend.GetPending(arena)
	if arena == 1 then
		-- Main arena
		return data.MainArenaPending.Active
	else
		return data.SoloArenaPending.Active
	end
end

--[[--------------------------------------------------------------------------------------------------------------
	backend.GetState(arena)
	
	Gets the state of the arena (one of the MS defines)
	
	Params: arena - Arena to get the status from (1 = main arena, 2 = 1v1 arena)
--]]--------------------------------------------------------------------------------------------------------------
function backend.GetState(arena)
	if arena == 1 then
		-- Main arena
		return data.MainArena.Stage
	else
		return data.SoloArena.Stage
	end
end

--[[--------------------------------------------------------------------------------------------------------------
	backend.FindPlayer(ply)
	
	Returns the arena and team this player belongs to
	
	Arena: 1 for main arena, 2 for 1v1 arena, 0 if not found
	Team: 1 for red team, 2 for blue team, 0 if not found
--]]--------------------------------------------------------------------------------------------------------------
function backend.FindPlayer(ply)
	local k,v
	if data.MainArena.Active then
		for k,v in pairs(data.MainArena.RedTeam) do
			if v == ply then
				return 1, 1
			end
		end
		for k,v in pairs(data.MainArena.BlueTeam) do
			if v == ply then
				return 1, 2
			end
		end
	end
	if data.SoloArena.Active then
		if data.SoloArena.Player1 == ply then
			return 2,1
		elseif data.SoloArena.Player2 == ply then
			return 2,2
		end
	end
	return 0,0
end

--[[--------------------------------------------------------------------------------------------------------------
	backend.FindPlayerPending(ply)
	
	Returns the arena and team this player belongs to (in the pending match)
	
	Arena: 1 for main arena, 2 for 1v1 arena, 0 if not found
	Team: 1 for red team, 2 for blue team, 0 if not found
--]]--------------------------------------------------------------------------------------------------------------
function backend.FindPlayerPending(ply)
	local k,v
	for k,v in pairs(data.MainArenaPending.RedTeam) do
		if v == ply then
			return 1, 1
		end
	end
	for k,v in pairs(data.MainArenaPending.BlueTeam) do
		if v == ply then
			return 1, 2
		end
	end
	if data.SoloArenaPending.Player1 == ply then
		return 2,1
	elseif data.SoloArenaPending.Player2 == ply then
		return 2,2
	else
		return 0,0
	end
end

--[[--------------------------------------------------------------------------------------------------------------
	backend.SetupMatch(arena, gamemode, limit, timelimit)
	
	Sets up a new (pending) match for the specified arena
	
	arena: 1 for main arena, 2 for 1v1 arena
	gamemode: See MGM_* defines, main arena only
	limit: Kill or reinforcements limit
	timelimit: Time limit
	
	Returns true if successful, false if not (including the reason why)
--]]--------------------------------------------------------------------------------------------------------------
function backend.SetupMatch(arena, gamemode, limit, timelimit)
	if arena == 1 then
		local MAP = data.MainArenaPending
		
		if gamemode <= MGM_NONE or gamemode >= MGM_MAX then
			return false, "Invalid game mode specified"
		end
		if limit < 5 or limit > 99 then
			return false, "Invalid limit specified"
		end
		if timelimit < 30 or timelimit > 3600 then
			return false, "Invalid time limit specified"
		end
		
		MAP.Active = true
		MAP.Ready = false
		MAP.RedTeam = {}
		MAP.BlueTeam = {}
		MAP.GameMode = gamemode
		if gamemode == MGM_REINFORCEMENTS then
			MAP.ReinfCount = limit
			MAP.KillLimit = 0
		elseif gamemode == MGM_KILLS then
			MAP.ReinfCount = 0
			MAP.KillLimit = limit
		end
		MAP.TimeLimit = timelimit
		return true
	end
	return false, "Invalid arena specified"
end

--[[--------------------------------------------------------------------------------------------------------------
	backend.ChangeMatch(arena, gamemode, limit, timelimit)
	
	Changes the pending match match for the specified arena
	
	arena: 1 for main arena, 2 for 1v1 arena
	gamemode: See MGM_* defines, main arena only
	limit: Kill or reinforcements limit
	timelimit: Time limit
	
	If you dont want to change any of these, pass nil
	
	Returns true if successful, false if not (including the reason why)
--]]--------------------------------------------------------------------------------------------------------------
function backend.ChangeMatch(arena, gamemode, limit, timelimit)
	if arena == 1 then
		local MAP = data.MainArenaPending
		if not MAP.Active then
			return false, "No pending match to change"
		end
		if gamemode then
			if gamemode <= MGM_NONE or gamemode >= MGM_MAX then
				return false, "Invalid game mode specified"
			end
		end
		if limit then
			if limit < 5 or limit > 99 then
				return false, "Invalid limit specified"
			end
		end
		if timelimit then
			if timelimit < 30 or timelimit > 3600 then
				return false, "Invalid time limit specified"
			end
		end
		
		MAP.GameMode = gamemode or MAP.GameMode
		if MAP.GameMode == MGM_REINFORCEMENTS then
			MAP.ReinfCount = limit or MAP.ReinfCount
			if MAP.ReinfCount < 5 then
				MAP.ReinfCount = 5
			end
		elseif MAP.GameMode == MGM_KILLS then
			MAP.KillLimit = limit or MAP.KillLimit
			if MAP.KillLimit < 5 then
				MAP.KillLimit = 5
			end
		end
		MAP.TimeLimit = timelimit or MAP.TimeLimit
		return true
	end
	return false, "Invalid arena specified"
end

--[[--------------------------------------------------------------------------------------------------------------
	backend.GetPendingMatch()
	
	Returns the gamemode, limit and timelimit
		
	Returns true, followed by the gamemode, limit and timelimit if successful, false if not (including the reason why)
--]]--------------------------------------------------------------------------------------------------------------
function backend.GetPendingMatch()
	local MAP = data.MainArenaPending
	if not MAP.Active then
		return false, "No pending match"
	end
	if MAP.GameMode == MGM_REINFORCEMENTS then
		return true, MAP.GameMode, MAP.ReinfCount, MAP.TimeLimit
	elseif MAP.GameMode == MGM_KILLS then
		return true, MAP.GameMode, MAP.KillLimit, MAP.TimeLimit
	else
		return true, MAP.GameMode, 0,  MAP.TimeLimit
	end
end

--[[--------------------------------------------------------------------------------------------------------------
	backend.GetTeam(team)
	
	Returns the red or blue team
	
	team: 1 for red team, 2 for blue team
	
	Returns team array if successful, nil + reason if not
--]]--------------------------------------------------------------------------------------------------------------
function backend.GetTeam(team)
	if not data.MainArena.Active then
		return nil, "No match in progress"
	else
		if team == 1 then
			return data.MainArena.RedTeam
		elseif team == 2 then
			return data.MainArena.BlueTeam
		end
		return nil, "Invalid team specified"
	end
end


--[[--------------------------------------------------------------------------------------------------------------
	backend.GetTeam(team)
	
	Returns the red or blue team
	
	team: 1 for red team, 2 for blue team
	
	Returns team array if successful, nil + reason if not
--]]--------------------------------------------------------------------------------------------------------------
function backend.GetPendingTeam(team)
	if not data.MainArenaPending.Active then
		return nil, "No pending match"
	else
		if team == 1 then
			return data.MainArenaPending.RedTeam
		elseif team == 2 then
			return data.MainArenaPending.BlueTeam
		end
		return nil, "Invalid team specified"
	end
end

--[[--------------------------------------------------------------------------------------------------------------
	backend.SignUpPlayer(arena, team, ply)
	
	Signs up the player for the specified team (pending)
	
	arena: 1 for normal arena, 2 for 1v1 arena
	team: 1 for red team, 2 for blue team
	ply: Player to signup
	
	Returns true if successful, false if not
--]]--------------------------------------------------------------------------------------------------------------
function backend.SignUpPlayer(arena, team, ply)
	local k,v
	if not data.MainArenaPending.Active then
		return false, "No pending match"
	end
	if arena == 1 then
		if team == 1 then
			for k,v in pairs(data.MainArenaPending.RedTeam) do
				if v == ply then
					return false, "The player is already on this team"
				end
			end
			for k,v in pairs(data.MainArenaPending.BlueTeam) do
				if v == ply then
					return false, "The player is already on the other team"
				end
			end
			table.insert(data.MainArenaPending.RedTeam, ply)
			return true
		elseif team == 2 then
			for k,v in pairs(data.MainArenaPending.RedTeam) do
				if v == ply then
					return false, "The player is already on the other team"
				end
			end
			for k,v in pairs(data.MainArenaPending.BlueTeam) do
				if v == ply then
					return false, "The player is already on this team"
				end
			end
			table.insert(data.MainArenaPending.BlueTeam, ply)
			return true
		else
			return false, "Invalid team specified"
		end
	end
	-- Todo: Arena 2
	return false, "Invalid arena specified"
end

--[[--------------------------------------------------------------------------------------------------------------
	backend.RemovePlayer(arena, team, ply)
	
	Removes the player from the specified team (pending)
	
	arena: 1 for normal arena, 2 for 1v1 arena
	team: 1 for red team, 2 for blue team
	ply: Player to remove
	
	Returns true if successful, false if not
--]]--------------------------------------------------------------------------------------------------------------
function backend.RemovePlayer(arena, team, ply)
	local k,v
	if not data.MainArenaPending.Active then
		return false, "No pending match"
	end
	if arena == 1 then
		if team == 1 then
			for k,v in pairs(data.MainArenaPending.RedTeam) do
				if v == ply then
					table.remove(data.MainArenaPending.RedTeam, k)
					return true
				end
			end
			return false, "The player is not on this team"
		elseif team == 2 then
			for k,v in pairs(data.MainArenaPending.BlueTeam) do
				if v == ply then
					table.remove(data.MainArenaPending.BlueTeam, k)
					return true
				end
			end
			return false, "The player is not on this team"
		else
			return false, "Invalid team specified"
		end
	end
	-- Todo: Arena 2
	return false, "Invalid arena specified"
end

--[[--------------------------------------------------------------------------------------------------------------
	backend.ClearTeam(arena, team)
	
	Clears out the specified team (pending)
	
	arena: 1 for normal arena, 2 for 1v1 arena
	team: 1 for red team, 2 for blue team
	
	Returns true if successful, false if not
--]]--------------------------------------------------------------------------------------------------------------
function backend.ClearTeam(arena, team)
	local k,v
	if not data.MainArenaPending.Active then
		return false, "No pending match"
	end
	if arena == 1 then
		if team == 1 then
			data.MainArenaPending.RedTeam = {}
			return true
		elseif team == 2 then
			data.MainArenaPending.BlueTeam = {}
			return true
		else
			return false, "Invalid team specified"
		end
	end
	-- Todo: Arena 2
	return false, "Invalid arena specified"
end

--[[--------------------------------------------------------------------------------------------------------------
	backend.CheckTeam(arena, team)
	
	Checks all players of the specified team (pending) and removes invalid players
	
	arena: 1 for normal arena, 2 for 1v1 arena
	team: 1 for red team, 2 for blue team
	
	Returns true if successful, false if not
--]]--------------------------------------------------------------------------------------------------------------
function backend.CheckTeam(arena, team)
	local k,v
	if not data.MainArenaPending.Active then
		return false, "No pending match"
	end
	if arena == 1 then
		if team == 1 then
			for k,v in pairs(data.MainArenaPending.RedTeam) do
				if not v:IsValid() then
					table.remove(data.MainArenaPending.RedTeam, k)
				end
			end
			return true
		elseif team == 2 then
			for k,v in pairs(data.MainArenaPending.BlueTeam) do
				if not v:IsValid() then
					table.remove(data.MainArenaPending.BlueTeam, k)
				end
			end
			return true
		else
			return false, "Invalid team specified"
		end
	end
	-- Todo: Arena 2
	return false, "Invalid arena specified"
end


--[[--------------------------------------------------------------------------------------------------------------
	backend.FinalizeMatch(arena)
	
	Finalizes match and marks it as ready to go
	
	arena: 1 for normal arena, 2 for 1v1 arena
	
	Returns true if successful, false if not (including reason)
--]]--------------------------------------------------------------------------------------------------------------
function backend.FinalizeMatch(arena)
	if arena == 1 then
		if not data.MainArenaPending.Active then
			return false, "No pending match"
		end
		if #data.MainArenaPending.RedTeam < 1 then
			return false, "No players on red team"
		end
		if #data.MainArenaPending.BlueTeam < 1 then
			return false, "No players on blue team"
		end
		
		data.MainArenaPending.Ready = true
		return true
	elseif arena == 2 then
		return false, "Arena not implemented yet"
	end
	return false, "Invalid arena specified"
end

--[[--------------------------------------------------------------------------------------------------------------
	backend.StartMatch(arena)
	
	Starts the pending match
	
	arena: 1 for nromal arena, 2 for 1v1 arena
	
	Returns true if successful, false if not (including reason)
--]]--------------------------------------------------------------------------------------------------------------
function backend.StartMatch(arena)
	if arena == 1 then
		if data.MainArena.Active then
			return false, "Arena is in use"
		end
		if not data.MainArenaPending.Active then
			return false, "No pending match"
		end
		if not data.MainArenaPending.Ready then
			return false, "Pending match isn't ready"
		end
		if #data.MainArenaPending.RedTeam < 1 then
			return false, "No players on red team"
		end
		if #data.MainArenaPending.BlueTeam < 1 then
			return false, "No players on blue team"
		end
		
		-- Everything checks out, transfer everything and start
		local MA = data.MainArena
		local MAP = data.MainArenaPending
		
		-- Transfer MAP to MA
		MA.RedTeam = MAP.RedTeam
		MA.BlueTeam = MAP.BlueTeam
		MA.GameMode = MAP.GameMode
		MA.ReinfCount = MAP.ReinfCount
		MA.KillLimit = MAP.KillLimit
		MA.TimeLimit = MAP.TimeLimit
		-- Initialize the rest of MA
		MA.Active = true
		if MA.GameMode == MGM_REINFORCEMENTS then
			MA.RedScore = MA.ReinfCount
			MA.BlueScore = MA.ReinfCount
		elseif MA.GameMode == MGM_KILLS then
			MA.RedScore = 0
			MA.BlueScore = 0
		else
			MA.RedScore = 0
			MA.BlueScore = 0
		end
		MA.TimeLeft = 0
		-- Clean up MAP
		MAP.Active = false
		MAP.Ready = false
		MAP.RedTeam = {}
		MAP.BlueTeam = {}
		MAP.GameMode = MGM_NONE
		MAP.ReinfCount = 0
		MAP.KillLimit = 0
		MAP.KillLimit = 0
		-- Start the main thread
		MA.Stage = MS_STARTING
		thread.Create("Arena1Main", backend.Threads.MAMain)
		return true
	elseif arena == 2 then
		return false, "Arena not implemented yet"
	end
	return false, "Invalid arena specified"
end

--[[--------------------------------------------------------------------------------------------------------------
	backend.GameFinished(arena, cause)
	
	Finalizes the match
	
	arena: 1 for nromal arena, 2 for 1v1 arena
	cause: 1 = Limit reached, 2 = Time limit, 3 = Aborted

	Returns true if successful, false if not (including reason)
--]]--------------------------------------------------------------------------------------------------------------
function backend.GameFinished(arena, cause)
	if cause < 1 or cause > 3 then
		return false, "Invalid cause specified"
	end
	
	if arena == 1 then
		-- Change state to finished
		data.MainArena.Stage = MS_FINISHED
		-- And kill the main thread
		thread.TerminateThread("Arena1Main")
		
		-- Determine who won the match
		local timeout = cause == 2
		local aborted = cause == 3
		local condition
		if data.MainArena.RedScore == data.MainArena.BlueScore then
			condition = 3	-- Tied
		elseif data.MainArena.RedScore > data.MainArena.BlueScore then
			condition = 1	-- Red won
		else
			condition = 2	-- Blue won
		end
		
		-- Create the finalize thread
		thread.Create("Arena1Finish", backend.Threads.MAFinalize, condition, timeout, aborted)
		return true		
	elseif arena == 2 then
		return false, "Arena not implemented yet"
	end
	return false, "Invalid arena specified"
end

--[[--------------------------------------------------------------------------------------------------------------
	backend.DoLoadout(arena, ply)
	
	Finalizes the match
	
	arena: 1 for nromal arena, 2 for 1v1 arena
	ply: Player to give the loadout to
	
	Returns true if successful, false if not
--]]--------------------------------------------------------------------------------------------------------------
function backend.DoLoadout(arena, ply)
	if arena == 1 then
		-- TODO: Implement loadout selection
		-- In the meanwhile we'll do a default loadout instead
		
		-- Remove everything first		
		ply:StripForce()
		ply:StripClipAmmo()
		ply:StripAmmo()
		ply:StripWeapons()
		ply:StripHoldables()
		-- Weapons:
		-- 1. Melee
		ply:GiveWeapon(WP_MELEE)
		-- 2. Filled E-11
		ply:GiveWeapon(WP_BLASTER)
		-- 3: Filled Pistol
		ply:GiveWeapon(WP_BRYAR_PISTOL)
		-- 4: Filled Disruptor Rifle
		ply:GiveWeapon(WP_DISRUPTOR)
		
		if data.MainArena.Stage == MS_INPROGRESS then
			ply:SetClipAmmo(WP_BLASTER, 0, sys.WeaponClipSize(WP_BLASTER))
			ply:SetClipAmmo(WP_BRYAR_PISTOL, 0, sys.WeaponClipSize(WP_BRYAR_PISTOL))
			ply:SetClipAmmo(WP_DISRUPTOR, 0, sys.WeaponClipSize(WP_DISRUPTOR))
			-- Ammo:
			-- 1: 10 Blaster ammo clips
			ply:SetAmmo(AMMO_BLASTER, sys.WeaponClipSize(WP_BLASTER) * 10)
			
			-- 2: 10 Powercell clips
			ply:SetAmmo(AMMO_POWERCELL, sys.WeaponClipSize(WP_DISRUPTOR) * 10)
		end
		-- Set the e-11 as default weapon
		ply:SetWeapon(WP_BLASTER)
		
		-- Finally, heal them to max (possible feature: handicaps)
		ply.Health = ply.MaxHealth
		ply.Shield = ply.MaxShield
	end
end
