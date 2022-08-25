--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Code
	JKG Mos Eisley Arena: npcs
	
	Written by Futuza
--------------------------------------------------]]

-- Here we'll spawn all the npcs that belong to the map

local npctable = {
	-- The vendors @ blue base
    {
        NPCType = "vendor_equipment",
        NPCScript = "equipment_merchant",
        Origin = Vector(-4086, 1419, 60),
        Angles = Vector(0, -90, 0)
    },
	-- the vendors @ red base
	{
        NPCType = "vendor_equipment",
        NPCScript = "equipment_merchant",
        Origin = Vector(4086, -1419, 60),
        Angles = Vector(0, 90, 0)
    },
	-- silly hidden vendor
	{
        NPCType = "vendor_general",
        NPCScript = "general_merchant",
        Origin = Vector(425 , 2800, 855),
        Angles = Vector(0, 180, 0)
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

hook.Add("MapLoaded", "MosEisleyArenaNPCInit", InitNPCs)
