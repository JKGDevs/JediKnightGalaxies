--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Code
	JKG ImperialBase backend: npcs
	
	Written by Futuza
--------------------------------------------------]]

-- Here we'll spawn all the npcs that belong to the map

local npctable = {
	-- The vendors @ jawas
    {
        NPCType = "vendor_medic",
        NPCScript = "medic_merchant",
        Origin = Vector(2334, 5095, 92),
        Angles = Vector(0, 45, 0)
    },
	-- the vendors @ tuskens
    {
        NPCType = "vendor_medic",
        NPCScript = "medic_merchant",
        Origin = Vector(-1408, 1536, 100),
        Angles = Vector(0, 90, 0)
    },


    --neutral vendors
    {
        NPCType = "vendor_grenades",
        NPCScript = "grenade_merchant",
        Origin = Vector(-580, 5752, 255),
        Angles = Vector(0, -160, 0)
    },
	{
        NPCType = "vendor_grenades",
        NPCScript = "grenade_merchant",
        Origin = Vector(858, 1723, 275),
        Angles = Vector(0, 175, 0)
    },
    {
        NPCType = "vendor_grenades",
        NPCScript = "grenade_merchant",
        Origin = Vector(-120, 3925, -32),
        Angles = Vector(0, 0, 0)
    },
    {
        NPCType = "vendor_grenades",
        NPCScript = "grenade_merchant",
        Origin = Vector(-309, 4377, 95),
        Angles = Vector(0, 90, 0)
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

hook.Add("MapLoaded", "JawaFortressNPCInit", InitNPCs)
