--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Framework
	Initialization
	
	DO NOT MODIFY THIS FILE
	
	Written by BobaFett
--------------------------------------------------]]

-- Define global namespace for Jedi Knight Galaxies
JKG = JKG or {}

-- Extensions
include("extensions/string.lua")
include("extensions/util.lua")

-- Modules
include("modules/entstorage.lua")
include("modules/commands.lua")
include("modules/items.lua")
include("modules/chatcommands.lua")
include("modules/hooks.lua")
include("modules/timer.lua")
include("modules/threads.lua")
include("modules/json.lua")
include("modules/stringbuilder.lua")
include("modules/cinematics.lua")
include("modules/conversations.lua")
include("modules/vectorfnc.lua")

-- Managers
include("managers/cinematics.lua")
include("managers/entmanager.lua")
include("managers/npcmanager.lua")
include("managers/dialogues.lua")

-- Enums
include ("enums/cvars.lua")
include ("enums/forces.lua")
include ("enums/ents.lua")
include ("enums/trajectories.lua")

-- Minigames
include("minigames/init.lua")

-- Post-framework-init extensions
include("extensions/player.lua")

-- Init the math randomizer
math.randomseed(sys.Time())
