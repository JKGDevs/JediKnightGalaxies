--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Code
	JKG ImperialBaseCTF backend: npcs
	
	Written by Futuza
--------------------------------------------------]]

-- Here we'll spawn all the npcs that belong to the map

local npctable = {
	-- The vendors @ imperial
    {
        NPCType = "vendor_medic",
        NPCScript = "medic_merchant",
        Origin = Vector(1425, 1590, -3),    
        Angles = Vector(0, -120, 0)
    },
	{
        NPCType = "imperial",
        NPCScript = "equipment_merchant", 
        Origin = Vector(1310, 535, -3),    
        Angles = Vector(0, 90, 0)
    },	


	-- the vendors @ rebels
    {
        NPCType = "vendor_equipment",
        NPCScript = "equipment_merchant",
        Origin = Vector(-4611, 704, 68),
        Angles = Vector(0, 0, 0)
    },
	{
        NPCType = "vendor_medic",
        NPCScript = "medic_merchant",
        Origin = Vector(-4529, 1400, 68),
        Angles = Vector(0, 45, 0)
    },


    --neutral vendors
    {
        NPCType = "vendor_grenades",
        NPCScript = "grenade_merchant",
        Origin = Vector(33, -171, -123),    
        Angles = Vector(0, 90, 0)
    },

	{
        NPCType = "vendor_grenades",
        NPCScript = "grenade_merchant",
        Origin = Vector(-1543, 2922, -131),       
        Angles = Vector(0, -120, 0)
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
