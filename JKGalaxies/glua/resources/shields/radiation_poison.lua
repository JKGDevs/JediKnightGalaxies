--Radiation Poisoning Code
local RADIATION_STD_DMG = 2


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

function GiveRadiationBurnSmall(ply)
	--Damage(victim, attacker, direction, location, damage, flags , means of damage)
	ply:Damage(ply:GetEntity(ply), ply:GetEntity(ply), ply:GetOrigin(ply), ply:GetOrigin(ply), RADIATION_STD_DMG, 30, "MOD_POISONED") --note that poison damage does 1.3x extra dmg to organics
	ply.Entity:PlaySound(1, "sound/effects/radiation_low.wav")
	ply:AddBuff(ply, "standard-poison", 7000, 1.0); --radiation is poisonous
	ply:AddBuff(ply, "standard-fire", 7000, 1.0);   --and burns
end

function GiveRadiationBurnLarge(ply)
	ply:Damage(ply:GetEntity(ply), ply:GetEntity(ply), ply:GetOrigin(ply), ply:GetOrigin(ply), RADIATION_STD_DMG, 30, "MOD_POISONED")
	ply.Entity:PlaySound(1, "sound/effects/radiation_high.wav")
	ply:AddBuff(ply, "standard-slow", 10000, 1.0) --bad radiation also slows you down
	ply:AddBuff(ply, "standard-poison", 10000, 1.0);
	ply:AddBuff(ply, "standard-fire", 10000, 1.0);
end

function GiveRadiationBurnChance(ply)
	local result = sys.GetRandomInt(1, 100)
	if result > 84 then 	--15% chance of radiation burn
		if result > 94 then --5% chance its a BAD burn
			GiveRadiationBurnLarge(ply)
		else
			GiveRadiationBurnSmall(ply)
		end
	end
end

function Shield_Overload_Functions()
	shields.AddShieldFunction("shield_radiation_small", GiveRadiationBurnSmall)
	shields.AddShieldFunction("shield_radiation_large", GiveRadiationBurnLarge)
	shields.AddShieldFunction("shield_radiation_chance", GiveRadiationBurnChance)
end
