-- Bacta code

local BACTA_SMALL_AMOUNT = 5
local BACTA_MEDIUM_AMOUNT = 10
local BACTA_LARGE_AMOUNT = 20

function GiveBactaSmall(ply, quantity)
	if ply.Health + BACTA_SMALL_AMOUNT > ply.MaxHealth then
		ply.Health = ply.MaxHealth
	else
		ply.Health = ply.Health + BACTA_SMALL_AMOUNT
	end

	ply.Entity:PlaySound(1, "sound/items/use_bacta.wav")
	ply:AddBuff(ply, "standard-heal", 5000, 1.5); --healing buff
	ply:AddBuff(ply, "standard-slow", 1000, 1.0) --slow us down while applying buff
    ply:SetAnimUpper("BOTH_USEMED")
end

function GiveBactaMedium(ply, quantity)
	if ply.Health + BACTA_MEDIUM_AMOUNT > ply.MaxHealth then
		ply.Health = ply.MaxHealth
	else
		ply.Health = ply.Health + BACTA_MEDIUM_AMOUNT
	end

	ply.Entity:PlaySound(1, "sound/items/use_bacta.wav")
	ply:AddBuff(ply, "standard-heal", 7000, 1.5); --healing buff
	ply:AddBuff(ply, "standard-slow", 2000, 1.0) --slow us down while applying buff
    ply:SetAnimUpper("BOTH_USEMED")
end

function GiveBactaLarge(ply, quantity)
	if ply.Health + BACTA_LARGE_AMOUNT > ply.MaxHealth then
		ply.Health = ply.MaxHealth
	else
		ply.Health = ply.Health + BACTA_LARGE_AMOUNT
	end

	ply.Entity:PlaySound(1, "sound/items/use_bacta.wav")
	ply:AddBuff(ply, "refined-heal", 7000, 1.5); --healing buff
	ply:AddBuff(ply, "standard-slow", 2500, 1.0) --slow us down while applying buff
    ply:SetAnimUpper("BOTH_USEMED")
end

function Item_Bacta_Functions()
	items.AddConsumableFunction("item_bacta_small", GiveBactaSmall)
	items.AddConsumableFunction("item_bacta_medium", GiveBactaMedium)
	items.AddConsumableFunction("item_bacta_large", GiveBactaLarge)
end