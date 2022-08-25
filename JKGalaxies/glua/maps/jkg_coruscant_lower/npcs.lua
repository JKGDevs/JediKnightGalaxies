--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Code
	JK Coruscant backend: npcs
	
	Written by Futuza
--------------------------------------------------]]

-- Here we'll spawn all the npcs that belong to the map

local npctable = {
	-- The vendors @ Imp base
    {
        NPCType = "stofficer",
        NPCScript = "ammo_merchant",
        Origin = Vector(9409, 559, 68),
        Angles = Vector(0, -70, 0)
    },
	{
        NPCType = "vendor_medic",
        NPCScript = "medic_merchant",
        Origin = Vector(9403, 108, 60),
        Angles = Vector(0, 75, 0)
    },
	-- the vendors @ BSS base
	{
        NPCType = "vendor_ammo",
        NPCScript = "ammo_merchant",
        Origin = Vector(15551, -7424, 50),
        Angles = Vector(0, -135, 0)
    },
	{
        NPCType = "vendor_medic",
        NPCScript = "medic_merchant",
        Origin = Vector(15440, -7820, 60),
        Angles = Vector(0, 145, 0)
    },
	-- neutral vendor warehouse bridge
	{
        NPCType = "vendor_general",
        NPCScript = "general_merchant",
        Origin = Vector(5399 , -233, 337),
        Angles = Vector(0, 0, 0)
    },
	-- neutral vendor sniper tower
	{
        NPCType = "vendor_medic",
        NPCScript = "medic_merchant",
        Origin = Vector(11693, -1377, 2260 ),
        Angles = Vector(0, 145, 0)
    },
	-- neutral vendor spice guy
	{
        NPCType = "trandoshan",
        NPCScript = "spice_merchant",
        Origin = Vector(9192, -2509, 236),
        Angles = Vector(0, 105, 0)
   },
	-- neutral vendor club barkeep
	{
        NPCType = "bartender",
        NPCScript = "general_merchant",
        Origin = Vector(9496, -3087, 72),
        Angles = Vector(0, 70, 0)
    },
	-- neutral vendor backalley jawa
	{
       NPCType = "jawa",
       NPCScript = "general_merchant",
       Origin = Vector(10684, -7999, 60),
       Angles = Vector(0, -175, 0)
    },
    -- neutral vendor bridge top sniper
	{
        NPCType = "vendor_medic",
        NPCScript = "medic_merchant",
        Origin = Vector(13941, -4966, 844),
        Angles = Vector(0, 180, 0)
    },
    --neutral vendor grenades, backalley top
    {
        NPCType = "vendor_grenades",
        NPCScript = "grenade_merchant",
        Origin = Vector(12896, -7533, 844),
        Angles = Vector(0, -180, 0)
    },
    --neutral vendor grenades, street by bar
    {
        NPCType = "vendor_grenades",
        NPCScript = "grenade_merchant",
        Origin = Vector(8359, -3471, 60),
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

hook.Add("MapLoaded", "CoruscantNPCInit", InitNPCs)
