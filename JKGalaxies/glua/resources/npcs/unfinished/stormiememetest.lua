NPC.NPCName = "stormiememetest"

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
	-- Raise our use range so we can be used across a counter
	self.UseRange = 150
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
	
	local ply = activator:ToPlayer()
	
	local dlg = dialogue.CreateDialogueObject("apathetic_stormie")
	if not dlg then
		print("ERROR: Cannot find dialogue 'apathetic_stormie'")
		return
	end
	dlg:RunDialogue(self, ply)
end

function NPC:OnRemove()
	self.Spawner:Use(self, self)
end
