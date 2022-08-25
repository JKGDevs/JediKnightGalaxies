--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Code
	JKG Spaceport backend: npcs
	
	Written by Futuza
--------------------------------------------------]]

-- Here we'll spawn all the npcs that belong to the map

local npctable = {
	-- The vendors @ hutt base
    {
        NPCType = "vendor_ammo",
        NPCScript = "ammo_merchant",
        Origin = Vector(-988, 2351, 108),
        Angles = Vector(0, 0, 0)
    },
	{
        NPCType = "vendor_medic",
        NPCScript = "medic_merchant",
        Origin = Vector(-981, 2486, 108),
        Angles = Vector(0, 0, 0)
    },
	-- the vendors @ imp base
	{
        NPCType = "vendor_ammo",
        NPCScript = "ammo_merchant",
        Origin = Vector(-1175, -882, 63),
        Angles = Vector(0, -90, 0)
    },
	{
        NPCType = "vendor_medic",
        NPCScript = "medic_merchant",
        Origin = Vector(-1022, -988, 63),
        Angles = Vector(0, 180, 0)
    },
	-- free vendor outside imp base
	{
        NPCType = "vendor_grenades",
        NPCScript = "grenade_merchant",
        Origin = Vector(-1899, -1339, 63),
        Angles = Vector(0, 0, 0)
    },
	-- free vendor outside hutt base
	{
        NPCType = "vendor_grenades",
        NPCScript = "grenade_merchant",
        Origin = Vector(-565, 1107, 60),
        Angles = Vector(0, 135, 0)
    },
	-- free vendor in slave area
	{
        NPCType = "vendor_grenades",
        NPCScript = "grenade_merchant",
        Origin = Vector(1369, -1316, 60),
        Angles = Vector(0, 95, 0)
    },
	-- free vendor in center area
	{
        NPCType = "vendor_general",
        NPCScript = "general_merchant",
        Origin = Vector(-727, -146, 60),
        Angles = Vector(0, 50, 0)
    },
	-- free vendor in bar area
	{
        NPCType = "vendor_general",
        NPCScript = "general_merchant",
        Origin = Vector(-579, 1248, 364),
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

hook.Add("MapLoaded", "SpaceportNPCInit", InitNPCs)
