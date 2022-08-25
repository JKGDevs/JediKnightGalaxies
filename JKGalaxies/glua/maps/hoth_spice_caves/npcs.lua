--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Code
	Hoth spice caves backend: npcs
	
	Written by Darth Futuza
--------------------------------------------------]]

-- Here we'll spawn all the npcs that belong to the arena

local npctable = {
	-- The lost stormtrooper
    {
        NPCType = "stormtrooper",
        NPCScript = "lost_trooper",
        Origin = Vector(-3149, -626, 1516),
        Angles = Vector(0, -88, 0)
    },
	--tutorial_bossman
	{
		NPCType = "stormtrooper",
		NPCScript = "tutorial_bossman",
		Origin = Vector(-2462, 255, 1553),
        Angles = Vector(0, 115, 0)
	},
	--test generic vendor
		--tutorial_bossman
	{
		NPCType = "stormtrooper",
		NPCScript = "general_merchant",
		Origin = Vector(-3415, -1325, 1425),
		Angles = Vector(0, -10, 0)
	},
}


local function InitNPCs()
	-- First, delete all spawners we already made
	local k,v
	local entlist = ents.GetByName("arena_npcs")
	for k,v in pairs(entlist) do
		v:Free()
	end

	-- Spawn our npcs
	local entfact = ents.CreateEntityFactory("NPC_spawner")
	-- Fill in the shared data
	entfact:SetSpawnVar("targetname", "arena_npcs")
	entfact:SetSpawnVar("count", "-1")
	for k,v in pairs(npctable) do
		entfact:SetSpawnVar("NPC_type", v.NPCType)
		entfact:SetSpawnVar("origin", string.format("%i %i %i", v.Origin.x, v.Origin.y, v.Origin.z))
		entfact:SetSpawnVar("angles", string.format("%i %i %i", v.Angles.x, v.Angles.y, v.Angles.z))
		entfact:SetSpawnVar("npcscript", v.NPCScript)
		ent = entfact:Create()
		if not ent or ent:IsValid() == false then
			print("ERROR: Failed to spawn arena NPC")
		end
		-- Spawn the npc
		ent:Use(nil, nil)
	end
end

hook.Add("MapLoaded", "HothNPCInit", InitNPCs)
