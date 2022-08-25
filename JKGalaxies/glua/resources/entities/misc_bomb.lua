--[[ ------------------------------------------------
	Jedi Knight Galaxies Entity

	Misc_bomb
	
	This is a test entity, to see if everything works
	This bomb is rigged with a 10 second fuse, which you can trigger by pressing use on it
	If the bomb takes damage, it'll ignite and explode (quite quickly :P)
	
	The entity is heavily commented to make it easier for others to understand how this system works
	
	FUN! :P
	
	Written by BobaFett
--------------------------------------------------]]

-- Entity definition scripts such as this are not loaded as normal scripts.
-- They are loaded by a special entity loader which takes care of the spawning and controlling of scripted entities.
-- To define a scripted entity, all properties, events and variables of the entity must be placed inside the 'ENT' table/namespace.
-- This table is created by the loader prior to running this script, and is destroyed after processing it!
-- Therefore, functions must never reference ENT directly! Instead, events can use 'self' to access their own entity object.
-- Everything defined in 'ENT' is available through 'self' inside the events.

-- Classname to be used for this entity (Mandatory, without this defined, the entity is discarded by the loader)
ENT.ClassName = "misc_bomb"
-- Type of this entity (generic, logical, mover and trigger are currently supported, also mandatory for the loader to accept this entity)
ENT.Type = "generic"
-- Scripted entity to derive this one from.
-- Note that this must be another scripted entity. You cannot provide a fx_runner as base entity for example.
-- The entity manager/loader automatically creates standard base entities for every type, called base_<type>, so for this one, we'll use "base_generic"
ENT.Base = "base_generic"

-- In every event, the entity in question is passed as a special argument, called 'self'
-- ( table:function(a,b,c) is syntactic sugar for table.function(self, a, b, c), so in this case ENT:OnSpawn() is identical to ENT.OnSpawn(self) )

-- OnSpawn event, called when the entity just spawned
-- In here we have to do the initialization of the entity, processing of the spawnvars, and doing all the setup required to get the ent working the way we want to
function ENT:OnSpawn()
	-- Set-up position, rotation (based on spawnvars) and model
	-- The 'or' is to specify a default value. A call to GetSpawnVar with an undefined var will return nil, in which case the alternative value is used
	-- So for example: If the spawnvar 'angles' is not defined, it'll use '0 0 0' instead
	self:SetPos( Vector(self:GetSpawnVar("origin") or "0 0 0") )
	self:SetAngles( Vector(self:GetSpawnVar("angles") or "0 0 0") )
	self:SetModel("models/map_objects/factory/bomb_new.md3")
	-- Automatically create a bounding box to fit the model + rotation
	self:AutoBox()
	-- Creating custom variables in entities is very simple, just use entity.<variable name>.
	-- So creating a custom variable called lol containing "my value", can be done with entity.lol = "my value"
	-- Any valid variable name is allowed as long as the name doesn't match one of the built-in functions of entities (so entity.SetPos = "my value" won't work as expected)
	
	-- Get the 'fuse' spawnvar (or 10 if not defined) to use as fuse time
	self.FuseLength = tonumber(self:GetSpawnVar("fuse") or "10")
	-- Set timeout for the use to 0, so we can use it right now
	self.UseTimeout = 0
	-- Make the entity solid
	self:SetContents(CONTENTS_SOLID)
	self:SetClipmask(MASK_SOLID)
	-- Make the entity usable
	self:SetPlayerUsable(true)
	-- Make it possible to damage this entity, and give it 50 HP
	self:SetHealth(50)
	self:SetTakeDamage(true)
	-- Put this entity in the world
	self:LinkEntity()
end

-- OnDie event, called when the entity gets hit and the HP dropped below 0 (Assuming the entity can take damage to begin with)
-- Inflictor and attacker both signify the entities responsible for the kill
-- Usually the inflictor and attacker are one and the same entity (or inflictor is not defined and only an attacker is provided)
-- An example where they differ, is when a player shoots a rocket, where the rocket is the inflictor, and the player who shot it the attacker
function ENT:OnDie(inflictor, attacker, damage, meansofdeath)
	-- Our HP dropped to 0, set it on fire and get ready to blow
	if self.Detonate then
		-- If we are already exploding, don't bother doing anything 
		return
	end
	if self.Spark then
		-- We got hit after already being set on fire, blow up right now
		self.Detonate = true
		-- Since we set Detonate to true, running OnThink will make it explode
		self:SetTakeDamage(false)
		self:SetNextThink(0)
		return
	end
	-- If we were doing a countdown, abort it. After all, we just broke it :P
	if self.CountDown then
		self.CountDown = nil
	end
	-- When its on fire, you can't arm or disarm it anymore
	self:SetPlayerUsable(false)
	-- Delay before exploding
	-- Each spark (being the thing catching fire) lasts 250 milliseconds, and we get a random number of 5 to 15 sparks
	-- Which gives us an explosion delay of 1.25 to 3.75 seconds
	self.Spark = math.random(5, 15)
	-- Run OnThink as soon as possible
	self:SetNextThink(0)
end

-- OnTakeDamage event, called when the entity takes damage but still has HP left  (Assuming the entity can take damage to begin with)
function ENT:OnTakeDamage(attacker, damage)
	-- Play the 'volumetric/pressurized_steam' effect when we sustain damage
	local fxid = sys.EffectIndex("volumetric/pressurized_steam")
	local explpos = self:GetPos() + Vector(0,0,10)
	sys.PlayEffect(fxid, explpos, Vector(0,0,90))
end

-- OnThink event, this event is called when requested using the SetNextThink( delay ) method.
-- After using that method, the server will call OnThink after the delay (in ms) passed
-- So using self:SetNextThink(500) means OnThink will be called 500 milliseconds after that call
-- A delay of 0 makes it trigger as soon as possible (Either the same frame, or the next, depending on where it was used)
function ENT:OnThink()
	-- We get here after using SetNextThink(delay), this is used for timed 'think' events
	if self.Detonate then
		-- We're set up to  detonate, so lets go boom
		-- First, play the 'explosions/wedge_explosion1' effect
		local fxid = sys.EffectIndex("explosions/wedge_explosion1")
		local explpos = self:GetPos() + Vector(0,0,10)
		sys.PlayEffect(fxid, explpos)
		-- Then do some radius damage: 500 HP damage with a 1024 unit radius
		sys.RadiusDamage(explpos, self, 500, 1024, self, nil, 0)
		-- Destroy the entity (NOTE: After this, 'self' can no longer be used, since the entity in question is now destroyed!)
		self:Free()
		return
	elseif self.CountDown then
		-- Alright, we're in a countdown, play the next beep
		self:PlaySound(1, "sound/vehicles/common/lockalarm2.mp3")
		if self.CountDown <= 0 then
			-- If the counter hit 0, explode
			self.CountDown = nil
			self.Detonate = true
			-- Run OnThink ASAP
			self:SetNextThink(0)
		elseif self.CountDown <= 2 then
			-- Only 2 seconds remaining, speed up the beeps (each 250 ms instead of each second)
			self.CountDown = self.CountDown - 0.25
			self:SetNextThink(250)
		else
			-- Decrement the countdown counter, and wait another second before doing another OnThink
			self.CountDown = self.CountDown - 1
			self:SetNextThink(1000)
		end
	elseif self.Spark > 0 then
		-- We're on fire!
		-- First, play the env/fire effect
		local fxid = sys.EffectIndex("env/fire")
		local explpos = self:GetPos() + Vector(0,0,30)
		sys.PlayEffect(fxid, explpos, Vector(0,0,0))
		-- Decrement the remaining 'sparks'
		self.Spark = self.Spark - 1
		if self.Spark == 0 then
			-- If we ran out of 'sparks', its time to go boom
			self.Detonate = true
		end
		-- Run another OnThink in 250 milliseconds
		self:SetNextThink(250)
	end
end

-- OnUse event, called after a player presses use on the entity (assuming the entity is usable by players) or if the entity is triggered by another entity
-- The 'other' argument is the entity that triggered it (in case it was triggered by another entity),
-- the 'activator' is the (usually player) responsible for the start of the chain
-- So, for example, if you have a button, targetting a target_relay, which targets this entity,
-- the 'other' will be the target_relay entity, and the 'activator' the player that pressed the button
function ENT:OnUse(other, activator)
	-- If we're in a timeout, ignore the use for now
	if self.UseTimeout > sys.Time() then
		return
	end
	-- If the activator is not a player, ignore it
	if not activator:IsPlayer() then
		return
	end
	-- Get the player object
	local ply = activator:ToPlayer()
	-- Check if the player is currently hacking (ya know, the lock with the progress bar under it)
	--if ply:IsHacking() then
	if ply.Hacking then
		-- If so, check if he finished
		if ply:FinishedHacking() then
			-- Yep, stop the hacking and proceed
			ply:StartHacking(nil)
		else
			-- Still busy hacking, wait till he's done
			return
		end
	else
		-- Start hacking this entity (for 1 second to arm, 3 seconds to disarm)
		if self.Armed then
			-- Send a center print message to the player
			ply:SendCenterPrint("Disarming bomb...")
			-- Start hacking this entity for 3000 millisecs
			ply:StartHacking(self, 3000)
		else
			-- Send a center print message to the player
			ply:SendCenterPrint("Arming bomb...")
			-- Start hacking this entity for 1000 millisecs
			ply:StartHacking(self, 1000)
		end
		return
	end
	-- Kill the center print message we sent before
	ply:SendCenterPrint("")
	-- Ensure we get a 500 ms timeout before being able to be used again (to avoid the immediate restart of the hacking process)
	self.UseTimeout = sys.Time() + 500
	-- Check if its already armed
	if self.Armed then
		-- If so, disarm it
		self.Armed = nil
		self.Detonate = nil
		self.CountDown = nil
		-- A SetNextThink of -1 disables it
		self:SetNextThink(-1)
		-- Send a chat message to the player
		ply:SendChat("^2Bomb disarmed!")
	else
		-- Arm it and start the countdown
		self.Armed = true
		self.Detonate = false
		self.CountDown = self.FuseLength
		-- Call OnThink ASAP
		self:SetNextThink(0)
		-- Send a chat message to the player
		ply:SendChat("^1Bomb armed! " , self.FuseLength, " seconds until detonation!")
	end
end
