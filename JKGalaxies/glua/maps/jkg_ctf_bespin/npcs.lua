--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Code
	JKG Bespin backend: npcs
	
	Written by Futuza
--------------------------------------------------]]

-- Here we'll spawn all the npcs that belong to the map

local npctable = {
	-- The vendors @ bespincops
    {
        NPCType = "vendor_medic",
        NPCScript = "medic_merchant",
        Origin = Vector(4103, -407, 92),
        Angles = Vector(0, 145, 0)
    },
	{
        NPCType = "vendor_equipment",
        NPCScript = "equipment_merchant", 
        Origin = Vector(4167, -96, 92),
        Angles = Vector(0, -180, 0)
    },	
    {
        NPCType = "vendor_grenades",
        NPCScript = "grenade_merchant",
        Origin = Vector(4112, 285, 92),
        Angles = Vector(0, -145, 0)
    },

	-- the vendors @ ugnaught union
    {
        NPCType = "vendor_equipment",
        NPCScript = "equipment_merchant",
        Origin = Vector(-3912, 350, 92),    
        Angles = Vector(0, 0, 0)
    },
	{
        NPCType = "vendor_medic",
        NPCScript = "medic_merchant",
        Origin = Vector(-3833, 642, 92),    
        Angles = Vector(0, -45, 0)
    },
	{
        NPCType = "vendor_grenades",
        NPCScript = "grenade_merchant",
        Origin = Vector(-3860, -15, 92),    
        Angles = Vector(0, 45, 0)
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

hook.Add("MapLoaded", "BespinNPCInit", InitNPCs)
