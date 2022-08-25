-- Adrenal code

local STAMINA_RECOVERED = 45

function GiveAdrenalStamina(ply, quantity)
	if ply.Stamina + STAMINA_RECOVERED > ply.MaxStamina then
		ply.Stamina = ply.MaxStamina
	else
		ply.Stamina = ply.Stamina + STAMINA_RECOVERED
	end
	ply.Entity:PlaySound(1, "sound/items/use_bacta.wav")
    ply:AddBuff(ply, "standard-slow", 2000, 1.0) --slow us down while applying buff
    ply:SetAnimUpper("BOTH_USEMED")
end


function Item_Adrenal_Functions()
    items.AddConsumableFunction("item_adrenal", GiveAdrenalStamina)
end