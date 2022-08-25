--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Code
	JK Spaceport backend: npcs
	
	Written by Futuza
--------------------------------------------------]]

-- Here we'll spawn all the npcs that belong to the map

local npctable = {
	-- poor guy in bldg by bar
    {
        NPCType = "vendor_ammo",
        NPCScript = "ammo_merchant",
        Origin = Vector(-12197, 697, 0),
        Angles = Vector(0, 50, 0)
    },
	--bouncer by bar exit
	{
        NPCType = "vendor_medic",
        NPCScript = "medic_merchant",
        Origin = Vector(-12611, 1943, -39),
        Angles = Vector(0, -77, 0)
    },
	-- jawa stand #1 (by poor guy)
	{
        NPCType = "vendor_grenades",
        NPCScript = "grenade_merchant",
        Origin = Vector(-11798, 856, -5),
        Angles = Vector(0, 90, 0)
    },
	-- technician #1 in tunnel
	{
        NPCType = "vendor_general",
        NPCScript = "general_merchant",
        Origin = Vector(-8713, 3096, -5),
        Angles = Vector(0, 158, 0)
    },
		-- technician #0 at tunnel entrance
	{
        NPCType = "vendor_general",
        NPCScript = "general_merchant",
        Origin = Vector(-9038, 2828, -4),
        Angles = Vector(0, -100, 0)
    },
	--merchant #1 on wallsand ave
	{
        NPCType = "vendor_general",
        NPCScript = "general_merchant",
        Origin = Vector(-8624, 2699, -7),
        Angles = Vector(0, -90, 0)
    },
		--merchant #2 on wallsand ave
	{
        NPCType = "vendor_general",
        NPCScript = "general_merchant",
        Origin = Vector(-8006, 2203, -5),
        Angles = Vector(0, 90, 0)
    },
	--jawa merchant #1 at junk corner on wallsand ave
	{
        NPCType = "vendor_general",
        NPCScript = "general_merchant",
        Origin = Vector(-7487, 3492, -1),
        Angles = Vector(0, -125, 0)
    },
	--jawa merchant #2 on wallsand ave
	{
        NPCType = "vendor_general",
        NPCScript = "general_merchant",
        Origin = Vector(-6871, 2889, -2),
        Angles = Vector(0, -125, 0)
    },
	--swoop guy on wallsand ave
	{
        NPCType = "vendor_general",
        NPCScript = "general_merchant",
        Origin = Vector(-6110, 3220, -5),
        Angles = Vector(0, -108, 0)
    },
	-- shady corner guy near market tent
	{
        NPCType = "vendor_general",
        NPCScript = "general_merchant",
        Origin = Vector(-4892, 2338, 15),
        Angles = Vector(0, 35, 0)
    },
	-- bobafett at market tent
	{
        NPCType = "vendor_general",
        NPCScript = "general_merchant",
        Origin = Vector(-3692, 1575, 66),
        Angles = Vector(0, -160, 0)
    },
	-- merchant #3 at market tent
	{
        NPCType = "vendor_general",
        NPCScript = "general_merchant",
        Origin = Vector(-2612, 982, 58),
        Angles = Vector(0, -177, 0)
    },
	-- merchant #4 on canyon street
	{
        NPCType = "vendor_general",
        NPCScript = "general_merchant",
        Origin = Vector(-4353, -246, -8),
        Angles = Vector(0, -168, 0)
    },
	-- hidden npc in lower tarp area
	{
        NPCType = "vendor_general",
        NPCScript = "general_merchant",
        Origin = Vector(-3334, -2493, -174),
        Angles = Vector(0, 149, 0)
    },
	-- high class jawa
	{
        NPCType = "vendor_general",
        NPCScript = "general_merchant",
        Origin = Vector(-4725, -4063, 31),
        Angles = Vector(0, 0, 0)
    },
	-- stormtrooper guard imp entrance 1
	{
        NPCType = "vendor_general",
        NPCScript = "general_merchant",
        Origin = Vector(-1602, -3211, 2),
        Angles = Vector(0, -175, 0)
    },
	-- hidden npc in secret path near imp entrance 1
	{
        NPCType = "vendor_general",
        NPCScript = "general_merchant",
        Origin = Vector(-1226, -3995, 25),
        Angles = Vector(0, -90, 0)
    },
	-- stormtrooper canyon sentry inner
	{
        NPCType = "vendor_general",
        NPCScript = "general_merchant",
        Origin = Vector(-5244, -7292, 33),
        Angles = Vector(0, 90, 0)
    },
	-- stormtrooper canyon sentry outer #1
	{
        NPCType = "vendor_general",
        NPCScript = "general_merchant",
        Origin = Vector(-5581, -7860, 35),
        Angles = Vector(0, -100, 0)
    },
	-- stormtrooper canyon sentry outer #2
	{
        NPCType = "vendor_general",
        NPCScript = "general_merchant",
        Origin = Vector(-5184, -7810, 41),
        Angles = Vector(0, -58, 0)
    },
	-- stormtroper officer imp entrance 2
	{
        NPCType = "vendor_general",
        NPCScript = "general_merchant",
        Origin = Vector(-3038, 3112, 13),
        Angles = Vector(0, -175, 0)
    },
	-- stormtrooper imp entrance 2
	{
        NPCType = "vendor_general",
        NPCScript = "general_merchant",
        Origin = Vector(-3152, 2757, 5),
        Angles = Vector(0, 175, 0)
    },
	-- stormtrooper dunesea sentry inner
	{
        NPCType = "vendor_general",
        NPCScript = "general_merchant",
        Origin = Vector(-13644, 5118, -3),
        Angles = Vector(0, -90, 0)
    },
	-- stormtrooper dunesea sentry outer #1
	{
        NPCType = "vendor_general",
        NPCScript = "general_merchant",
        Origin = Vector(-13659, 5682, -1),
        Angles = Vector(0, 90, 0)
    },
	-- stormtrooper dunesea sentry outer #2
	{
        NPCType = "vendor_general",
        NPCScript = "general_merchant",
        Origin = Vector(-13201, 5690, -5),
        Angles = Vector(0, 96, 0)
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
	entfact:SetSpawnVar("targetname", "map_npcs")
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

hook.Add("MapLoaded", "MosEisleyDayNPCInit", InitNPCs)
