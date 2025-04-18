--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Framework
	Shields Module
	
	DO NOT MODIFY THIS FILE
	
	This module handles everything related to shields.
--------------------------------------------------]]

-- Start the hook module, all subsequent functions are variables, are stored within the hook. table, and access to the global environment is gone
module("shields")

local ShieldFunctions = {}

function GetShieldFunctions()
	return ShieldFunctions
end

--[[----------------------------------------------------------------------------------------
	shields.AddShieldFunction
	
	Adds a new named consumable item function.
-------------------------------------------------------------------------------------------]]
function AddShieldFunction(overload_name, overload_func)
	ShieldFunctions[overload_name] = overload_func
end


--[[----------------------------------------------------------------------------------------
	shields.RemoveConsumableFunction
	
	Remove a previously registered consumable function.
-------------------------------------------------------------------------------------------]]
function RemoveShieldFunction(overload_name)
	ShieldFunctions[overload_name] = nil
end


--[[----------------------------------------------------------------------------------------
	shields.OverloadScript (Internal function)
	
	This function is called by the engine when an shield is overloaded/broken.
	Do not call this from a script unless you have a very good reason to do so.
-------------------------------------------------------------------------------------------]]
function ShieldOverloadScript(ply, shieldFuncName)
	local func = ShieldFunctions[shieldFuncName]

	if ply.Shield > 0 then
		return
	end

	func(ply)
end