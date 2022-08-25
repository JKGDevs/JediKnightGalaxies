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
        Origin = Vector(314, 212, 320),
        Angles = Vector(0, -90, 0)
    },
	{
        NPCType = "vendor_medic",
        NPCScript = "medic_merchant",
        Origin = Vector(312, 123, 145),
        Angles = Vector(0, -180, 0)
    },
	-- the vendors @ red base
	{
        NPCType = "vendor_equipment",
        NPCScript = "equipment_merchant",
        Origin = Vector(1506, 5303, 112),
        Angles = Vector(0, -90, 0)
    },
	{
        NPCType = "vendor_medic",
        NPCScript = "medic_merchant",
        Origin = Vector(1542, 5099, 112),
        Angles = Vector(0, 180, 0)
    },

	-- neutral vendors
	--red side, next to bed and barrels
	{
        NPCType = "vendor_general",
        NPCScript = "general_merchant",
        Origin = Vector(961 , 3055, -70),
        Angles = Vector(0, 90, 0)
    },
	--blue side, wine cellar
	{
        NPCType = "vendor_general",
        NPCScript = "general_merchant",
        Origin = Vector(-5 , 1360, 0),
        Angles = Vector(0, 180, 0)
    },
	--blue side, outside wolf path
	{
        NPCType = "vendor_grenades",
        NPCScript = "grenade_merchant",
        Origin = Vector(-2180 , 1985, 115),
        Angles = Vector(0, -90, 0)
    },
	--red side, outside stairs
	{
        NPCType = "vendor_grenades",
        NPCScript = "grenade_merchant",
        Origin = Vector(-1080 , 3010, -70),
        Angles = Vector(0, -110, 0)
    },
	--hidden vendor by ice
	{
        NPCType = "vendor_ammo",
        NPCScript = "ammo_merchant",
        Origin = Vector(240 , 3741, -99),
        Angles = Vector(0, -90, 0)
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

hook.Add("MapLoaded", "NightfallNPCInit", InitNPCs)
