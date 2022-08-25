NPC.NPCName = "pazaaktrainer"

function NPC:OnSpawn()
	-- Initial setup
	-- Prevent the AI from messing things up
	self:SetBehaviorState("BS_CINEMATIC") 
	-- No weapon for this guy
	self:GiveWeapon(WP_MELEE)
	self:SetWeapon(WP_MELEE)
	-- Godmode it, so they don't get killed
	self:SetGodMode(true)
	-- Explicitly tell it to walk and not run
	self:SetWalking(true)
	self:SetRunning(false)	
	-- Prevent him from targeting anyone
	self:SetLookForEnemies(false)
	self:SetChaseEnemies(false)
	-- Set up local variables
	self.LastUse = 0
	self.ConvoState = {}
end

function NPC:OnUse(other, activator)
	if sys.Time() - self.LastUse < 500 then
		return
	end
	self.LastUse = sys.Time()
	
	if not activator:IsPlayer() then
		return		-- Only talk to players, nothing else
	end
	
	local ply = activator:ToPlayer()
end

function NPC:OnRemove()
	
end
