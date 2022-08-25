--Spice Code

local SPICE_STD_DMG = 10


--[[
Flag references:
	--local dmg_norm = 0  		--regular damage
	--local dmg_rad = 1   		--indirect radius damage
	--local dmg_noshld = 2  	--ignores shield protection
	--local dmg_noknckbk = 4  	--doesn't do knockback
	--local dmg_noprotect = 8  	--ignores shields, armor, godmode, self dmg reduction, etc
	--local dmg_nohitloc = 16  	--no hit location
	--local dmg_nodsmbr = 32  	--no dismemberment
To calculate damage correctly simply add the flags relevant to the attack together and use the total as the flag.
eg: 2+4+8+16 = 30: will do full damage, bypass shields, no knockback, no hit location (this is good for self inflicted damage).
--]]

function GiveSpiceSmall(ply, quantity)
	--Damage(victim, attacker, direction, location, damage, flags , means of damage)
	ply:Damage(ply:GetEntity(ply), ply:GetEntity(ply), ply:GetOrigin(ply), ply:GetOrigin(ply), SPICE_STD_DMG, 30, "MOD_POISONED") --note that poison damage does 1.3x extra dmg to organics
	ply.Entity:PlaySound(1, "sound/items/use_bacta.wav")
	ply:AddBuff(ply, "standard-haste", 7000, 1.5); --speed
end

function GiveSpiceLarge(ply, quantity)
	ply:Damage(ply:GetEntity(ply), ply:GetEntity(ply), ply:GetOrigin(ply), ply:GetOrigin(ply), SPICE_STD_DMG*2.5, 30, "MOD_POISONED")
	ply.Entity:PlaySound(1, "sound/items/use_bacta.wav")
	ply:AddBuff(ply, "standard-haste") --speed
	ply:AddBuff(ply, "standard-haste", 10000, 1.5) --large dose boost
	ply:AddBuff(ply, "standard-poison", 3000, 2.0) --speed
end


function Item_Spice_Functions()
	items.AddConsumableFunction("item_spice_small", GiveSpiceSmall)
	items.AddConsumableFunction("item_spice_large", GiveSpiceLarge)
end
