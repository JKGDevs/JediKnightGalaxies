NPC.NPCName = "tutorial_bossman"

function NPC:OnInit(spawner)
	self.Spawner = spawner
end

function NPC:OnSpawn()
	-- Initial setup
	-- Prevent the AI from messing things up
	self:SetBehaviorState("BS_CINEMATIC") 
	-- No weapon for this guy
	--self:GiveWeapon(WP_MELEE)  --futuza: these no longer work, investigate why
	--self:SetWeapon(WP_MELEE)
	-- Godmode it, so they dont get killed
		self.GodMode = true
	-- No knockback
	self.NoKnockback = true
	-- Explicitly tell it to walk and not run
	self.Walking = true
	self.Running = false
	-- Prevent him from targetting anyone
	self.LookForEnemies = false
	self.ChaseEnemies = false
	self.Enemy = nil 	--no enemies please
	-- Raise our use range so we can be used across a counter
	self.UseRange = 120
	-- Set up local variables
	self.LastUse = 0
end

function NPC:OnUse(other, activator)
	if sys.Time() - self.LastUse < 500 then
		return
	end
	self.LastUse = sys.Time()
	
	if not activator:IsPlayer() then
		return		-- Only talk to players, nothin else
	end
	
	self.Enemy = nil 	--no enemies please
	self:SetViewTarget(activator) 	--look at me, I am the captain now
	
	-- Voice lines
	local randosnd = math.random(0,2)
	if  randosnd == 0 then
		other:PlaySound(4, "sound/chars/st1/misc/look1.mp3")
	elseif randosnd == 1 then
		other:PlaySound(4, "sound/chars/st2/misc/suspicious1.mp3")
	else
		other:PlaySound(4, "sound/chars/st1/misc/sight3.mp3")
	end
	
	
	local ply = activator:ToPlayer()
	ply.EntPtr = other --give us access to ent functions if necessary (like playing sounds)
	
	local dlg = dialogue.CreateDialogueObject("tutorial_bossman")
	if not dlg then
		print("ERROR: Cannot find dialogue 'tutorial_bossman'")
		return
	end
	dlg:RunDialogue(self, ply)
	
	--if npc needs to open a shop
	--[[
	if self.OpenShop then
		print("We reached open shop.")
		self:UseVendor(ply)
		self.OpenShop = false
	end
	]]--
	
end

function NPC:OnPain(ply, dmg)
	--if you do damage to the npc
end

function NPC:OnRemove()
	self.Spawner:Use(self, self)
end
