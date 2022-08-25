--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Code
	Initialization
	
	Written by BobaFett
--------------------------------------------------]]

-- This is the ONLY script thats executed direcly by the engine (excluding glua/framework/init.lua)
-- So everything that needs to be loaded has to be done in here

JKG.Running = true

print("Loading main functions...")
include("admin/accounts.lua") 	--Accounts and Admin commands
include("emotes.lua")	--Emote system
include("dice.lua") 	--Dice Roller
print("Loading entity spawning commands...")
include("entcmds.lua")

print("Loading item functions...")
include "resources/items/init.lua"

print("Loading quests manager...")
include ("resources/quests/questManager.lua")

entmanager.Init()
-- Load the global entities
entmanager.RegisterDir("glua/resources/entities")
-- Load the map specific entities
entmanager.RegisterDir("glua/resources/entities/" .. string.lower(sys.MapName()))
print(string.format("Loaded %i entities(s)", entmanager.GetCount()))

npcmanager.Init()
-- Load the global NPCs
npcmanager.RegisterDir("glua/resources/npcs")
-- Load the map specific NPCs
npcmanager.RegisterDir("glua/resources/npcs/" .. string.lower(sys.MapName()))
print(string.format("Loaded %i NPCs(s)", npcmanager.GetCount()))

dialogue.Init()
-- Load the global dialogues
dialogue.RegisterDir("glua/resources/dialogues")
-- Load the map specific dialogues
dialogue.RegisterDir("glua/resources/dialogues/" .. string.lower(sys.MapName()))
print(string.format("Loaded %i dialogue(s)", dialogue.GetDlgCount()))

include("cinbuilder.lua") -- Load cinematic builder
include("cinematics.lua")

-- Initialize the map's init script (if it exists)
print("Loading map init script...")
if file.Exists("/glua/maps/" .. string.lower(sys.MapName()) .. "/init.lua") then
	include("maps/" .. string.lower(sys.MapName()) .. "/init.lua")
else
	print("No map init script found")
end

print("Loading miscellaneous functions")
include("misc.lua")  --This includes new test functions including stuff like pzktest and slctest
