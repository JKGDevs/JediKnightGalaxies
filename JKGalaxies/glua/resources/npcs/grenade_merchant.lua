NPC.NPCName = "grenade_merchant"

function NPC:OnInit(spawner)
	self.Spawner = spawner
end

function NPC:OnSpawn()
	-- Initial setup
	-- Prevent the AI from messing things up
	self:SetBehaviorState("BS_CINEMATIC") 
	self.GodMode = true
	self.NoKnockback = true
	self.LookForEnemies = false
	self.ChaseEnemies = false
	
	--vendor setup
	self:MakeVendor("grenadevendor")
	self:RefreshVendorStock()
	self.UseRange = 150 -- make us easier to use
	self.TimeToRestock = 1000 * sys.GetCvarInt("jkg_shop_replenish_time") -- how often (milliseconds) to restock?
	self.RestockTimer = sys.Time()
	self.LastUse = 0
	
	--for advertising wares animation
	self.LastAdvertisement = 0
	math.randomseed(sys.Time()) --seed random
end

function NPC:OnUse(other, activator)
	if sys.Time() - self.LastUse < 500 then
		return
	end
	self.LastUse = sys.Time()
	
	-- Only talk to players, nothin else
	if not activator:IsPlayer() then
		return		
	end
	
	local ply = activator:ToPlayer()
	self:UseVendor(ply)
	
end

function NPC:OnTouch(other)
	self:SetAnimBoth("BOTH_STAND10TOSTAND1") --if we get bumped into, react
end

function NPC:OnThink(other)
	--every 20000 seconds do an advertisement animation
	if sys.Time() - self.LastAdvertisement < 20000 then
		return
	else
		self:SetAnimBoth("BOTH_TUSKENTAUNT1")
		self.LastAdvertisement = (sys.Time() + math.random(1150,3550))
	end

	--refresh stocks
	if sys.Time() - self.RestockTimer > self.TimeToRestock then
		self:RefreshVendorStock()
		self.RestockTimer = sys.Time()
	end
end

function NPC:OnRemove()
	self.Spawner:Use(self, self)
end