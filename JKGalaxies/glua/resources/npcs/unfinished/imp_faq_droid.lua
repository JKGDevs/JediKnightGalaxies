NPC.NPCName = "imp_faq_droid"

function NPC:OnInit(spawner)
	self.Spawner = spawner
end

function NPC:OnSpawn()
	self:SetBehaviorState("BS_CINEMATIC") 
	self:GiveWeapon(WP_NONE)
	self:SetWeapon(WP_NONE)
	self.GodMode = true
	self.NoKnockback = true
	self.Walking = true
	self.Running = false
	self.LookForEnemies = false
	self.ChaseEnemies = false
	self.UseRange = 125
	self.LastUse = 0
	self.CanTalk = true
	self.InUse = false
	self:SetAnimBoth("BOTH_STAND1")
	self:SetAnimHoldTime(3, -1)	-- SetAnimHoldTime(SETANIM_BOTH, INFINITE)
end


function NPC:OnAnger()
	self:SetEnemy(nil)
end


function NPC:OnUse(other, activator)
	if sys.Time() - self.LastUse < 500 then
		return
	end
	self.LastUse = sys.Time()

	if not self.CanTalk then return end

	if self.InUse then
		ply:SendChat("^7This droid is already speaking with someone else.")
	end
	
	if not activator:IsPlayer() then
		return
	end

	self:SetViewTarget(activator)
	self.InUse = true
	local ply = activator:ToPlayer()
	
	local dlg = dialogue.CreateDialogueObject("imp_faq_droid_dlg")
	if not dlg then
		print("ERROR: Cannot find dialogue 'imp_faq_droid_dlg'")
		return
	end
	self:SetAnimBoth("BOTH_TALK2")
	self:SetAnimHoldTime(3, -1)	-- SetAnimHoldTime(SETANIM_BOTH, INFINITE)
	dlg:RunDialogue(self, ply)
	self.InUse = false
end

function NPC:OnRemove()
	self.Spawner:Use(self, self)
end
