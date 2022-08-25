-- Exported by JKG Dialogue Creator

DLG.Name = "lost_trooper"
DLG.RootNode = "E1"
DLG.Nodes = {
	E1 = {
		Type = 1,
		SubNode = "T5",
		NextNode = "E2",
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			--basic operating procedure for starting a quest
			--Check if quests are not setup, 
			--setup if they aren't, then initialize the quest in either case
			local stage = 0
			local qn = "losttrooper" --shorthand for questname
						
			if not ply.Quests or not ply.Quests[qn] then
				if InitQuest(ply, qn) then
					ply.Quests[qn].title = "The Lost Trooper"
			    end
			end
			
			-- Get the state of the quest (or default to 0)
			if ply.Quests[qn] then
			    stage = ply.Quests[qn].stage
			end
						
			-- Return whether state equals 0, if not returns false
			return stage == 0
		end,
	},
	T5 = {
		Type = 2,
		SubNode = "T6",
		Text = "So...cold...",
		Duration = 2000,
		HasCondition = false,
		HasResolver = false,
	},
	T6 = {
		Type = 2,
		SubNode = "S7",
		Text = "What was that?  Is...is someone there?",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	S7 = {
		Type = 6,
		SubNode = "D8",
		ScriptFunc = function(owner, ply, data)
			--update the quest's stage to 1, and notify player
			ply.Quests["losttrooper"].stage = 1
			ply:SendCenterPrint("Started: " .. ply.Quests["losttrooper"].title)
			ply:SendPrint("Started " .. ply.Quests["losttrooper"].title)
			ply.EntPtr:PlaySound(11, "sound/interface/quest/QuestStartReveal00.mp3")
			
			--allow him to sell us stuff too
			owner:MakeVendor("basicvendor")
			owner:RefreshVendorStock()
		end,
		HasCondition = false,
	},
	D8 = {
		Type = 5,
	},
	E2 = {
		Type = 1,
		SubNode = "T9",
		NextNode = "E3",
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			local stage = 1
						
			if ply.Quests["losttrooper"] then
				stage = ply.Quests["losttrooper"].stage
			end
						
			return stage == 1
		end,
	},
	T9 = {
		Type = 2,
		SubNode = "O10",
		Text = "I must be hallucinating...you can't be real, can you?",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	O10 = {
		Type = 3,
		SubNode = "T11",
		NextNode = "O14",
		Text = "I'm definitely real.",
		HasCondition = false,
		HasResolver = false,
	},
	T11 = {
		Type = 2,
		SubNode = "T12",
		Text = "That's just what a hallucination would say!",
		Duration = 2500,
		HasCondition = false,
		HasResolver = false,
	},
	T12 = {
		Type = 2,
		SubNode = "D13",
		Text = "I've got to find a way off this rock before I completely lose it...\nCome on Gary, keep it together. They're not real.",
		Duration = 5000,
		HasCondition = false,
		HasResolver = false,
	},
	D13 = {
		Type = 5,
	},
	O14 = {
		Type = 3,
		SubNode = "T17",
		NextNode = "O15",
		Text = "No, not at all.",
		HasCondition = false,
		HasResolver = false,
	},
	T17 = {
		Type = 2,
		SubNode = "O18",
		Text = "That's-wait...are you real?",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	O18 = {
		Type = 3,
		SubNode = "T23",
		NextNode = "O19",
		Text = "Yes.  My name is $name$.",
		HasCondition = false,
		HasResolver = true,
		ResolveFunc = function(owner, ply, var, data)
			if var == "name" then
				return sys.StripColorcodes(ply.Name)
			end
			return "<name-error>"
		end,
	},
	T23 = {
		Type = 2,
		SubNode = "T24",
		Text = "Oh that's great...just great $name$...",
		Duration = 3000,
		HasCondition = false,
		HasResolver = true,
		ResolveFunc = function(owner, ply, var, data)
			if var == "name" then
				return sys.StripColorcodes(ply.Name)
			end
			return "<name-error>"
		end,
	},
	T24 = {
		Type = 2,
		SubNode = "S25",
		Text = "...the blasted hallucinations have names now...",
		Duration = 2000,
		HasCondition = false,
		HasResolver = false,
	},
	S25 = {
		Type = 6,
		SubNode = "D26",
		ScriptFunc = function(owner, ply, data)
			ply.Quests["losttrooper"].stage = 2
		end,
		HasCondition = false,
	},
	D26 = {
		Type = 5,
	},
	O19 = {
		Type = 3,
		SubNode = "T27",
		NextNode = "O20",
		Text = "[Persuade]If I shoot you, will the pain convince you I'm real?",
		HasCondition = false,
		HasResolver = false,
	},
	T27 = {
		Type = 2,
		SubNode = "T28",
		Text = "Why do these blasted hallucinations make so much sense...",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	T28 = {
		Type = 2,
		SubNode = "D29",
		Text = "Oh...hahaha very funny.  \nWon't trick me, I can imagine pain well enough.",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	D29 = {
		Type = 5,
	},
	O20 = {
		Type = 3,
		SubNode = "T30",
		NextNode = "O21",
		Text = "[Lie]You know who I am.",
		HasCondition = false,
		HasResolver = false,
	},
	T30 = {
		Type = 2,
		SubNode = "T33",
		NextNode = "T31",
		Text = "...TK-313?   Is that you buddy?",
		Duration = 3000,
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			if ply:GetCreditCount() > 500 then
			    return true
			end
			return false
		end,
		HasResolver = false,
	},
	T33 = {
		Type = 2,
		SubNode = "D34",
		Text = "Wow, you sure have changed a lot.  Guess the cold rea-h-hang on!\nBlast!  You're just my imagination aren't you?  Why?\nWhy is this happening...",
		Duration = 6000,
		HasCondition = false,
		HasResolver = false,
	},
	D34 = {
		Type = 5,
	},
	T31 = {
		Type = 2,
		SubNode = "D32",
		Text = "Liar!  You're j-just a figment of my imagination!",
		Duration = 3000,
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			if ply:GetCreditCount() <= 500 then
			    return true
			end
			return false
		end,
		HasResolver = false,
	},
	D32 = {
		Type = 5,
	},
	O21 = {
		Type = 3,
		SubNode = "D22",
		Text = "Nevermind...",
		HasCondition = false,
		HasResolver = false,
	},
	D22 = {
		Type = 5,
	},
	O15 = {
		Type = 3,
		SubNode = "T35",
		NextNode = "O16",
		Text = "Just another Jedi mind trick I'm afraid...",
		HasCondition = false,
		HasResolver = false,
	},
	T35 = {
		Type = 2,
		SubNode = "T36",
		Text = "O-of course!  That's why it doesn't make any sense!\nA Jedi must be messing with my mind.",
		Duration = 4000,
		HasCondition = false,
		HasResolver = false,
	},
	T36 = {
		Type = 2,
		SubNode = "D37",
		Text = "Otherwise there's no way I could have been the only one to survive in the squad.\nJedi Mind tricks...must be. I've got to find that Jedi!\n",
		Duration = 5000,
		HasCondition = false,
		HasResolver = false,
	},
	D37 = {
		Type = 5,
	},
	O16 = {
		Type = 3,
		SubNode = "T38",
		Text = "[Lie]Snap out of it soldier and give me a report!",
		HasCondition = false,
		HasResolver = false,
	},
	T38 = {
		Type = 2,
		SubNode = "O39",
		Text = "Sir!  There rebels are hiding here they overran our-wait, who are you!?",
		Duration = 4000,
		HasCondition = false,
		HasResolver = false,
	},
	O39 = {
		Type = 3,
		SubNode = "T46",
		NextNode = "O40",
		Text = "A friend.",
		HasCondition = false,
		HasResolver = false,
	},
	T46 = {
		Type = 2,
		SubNode = "T47",
		Text = "I must be losing it...A friend?  Out here?\nJ-just my imagination.  N-nothing r-real out here.",
		Duration = 5000,
		HasCondition = false,
		HasResolver = false,
	},
	T47 = {
		Type = 2,
		SubNode = "D48",
		Text = "Keep it together Gary!  None of these people are real.\nJ-just you...and the cold.",
		Duration = 4000,
		HasCondition = false,
		HasResolver = false,
	},
	D48 = {
		Type = 5,
	},
	O40 = {
		Type = 3,
		SubNode = "L49",
		NextNode = "O41",
		Text = "I'm $name$.",
		HasCondition = false,
		HasResolver = true,
		ResolveFunc = function(owner, ply, var, data)
			--$name$ is "name" here, we should probably strip color codes out
			if var == "name" then
				return sys.StripColorcodes(ply.Name)
			end
			return "<name-error>"
		end,
	},
	L49 = {
		Type = 4,
		Target = "T23",
	},
	O41 = {
		Type = 3,
		SubNode = "T43",
		NextNode = "O42",
		Text = "The last face you'll ever see.",
		HasCondition = false,
		HasResolver = false,
	},
	T43 = {
		Type = 2,
		SubNode = "T44",
		Text = "No...no you're not real!",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	T44 = {
		Type = 2,
		SubNode = "L58",
		Text = "None of you are real!  IT WON'T WORK!\nIT WON'T WORK, DO YOU HEAR!?",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	L58 = {
		Type = 4,
		Target = "T12",
	},
	O42 = {
		Type = 3,
		SubNode = "T50",
		Text = "[Lie]I'm your senior officer private!  Are you questioning me?",
		HasCondition = false,
		HasResolver = false,
	},
	T50 = {
		Type = 2,
		SubNode = "T51",
		Text = "Sir, I would neve-no!  It's a trick!",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	T51 = {
		Type = 2,
		SubNode = "S52",
		Text = "The enemy must be trying to lower our defenses!\nFOR THE EMPIRE!",
		Duration = 4000,
		HasCondition = false,
		HasResolver = false,
	},
	S52 = {
		Type = 6,
		SubNode = "D57",
		ScriptFunc = function(owner, ply, data)
			--make npc vulnerable
			owner.GodMode = false
			owner.NoKnockback = false
			owner.Walking = false
			owner.Running = true
			local player = ply.Name
			owner.Enemy = player
			owner.LookForEnemies = true
			owner.ChaseEnemies = true
			--fail the quest and notify user they failed
			ply.Quests["losttrooper"].stage = -1
			ply:SendCenterPrint("Failed: " .. ply.Quests["losttrooper"].title)
			ply:SendPrint("Failed " .. ply.Quests["losttrooper"].title)
			ply.EntPtr:PlaySound(11, "sound/interface/quest/QuestFailed.mp3")
		end,
		HasCondition = false,
	},
	D57 = {
		Type = 5,
	},
	E3 = {
		Type = 1,
		SubNode = "T54",
		NextNode = "E4",
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			--player has given Gary their name
			local stage = 0
						
			if ply.Quests["losttrooper"] then
				stage = ply.Quests["losttrooper"].stage
			end
						
			return stage == 2
		end,
	},
	T54 = {
		Type = 2,
		SubNode = "T76",
		Text = "Oh it's you $name$.  You're just part of my imagination, right?",
		Duration = 4000,
		HasCondition = false,
		HasResolver = true,
		ResolveFunc = function(owner, ply, var, data)
			if var == "name" then
				return sys.StripColorcodes(ply.Name)
			end
			return "<name-error>"
		end,
	},
	T76 = {
		Type = 2,
		SubNode = "O84",
		Text = "What do you want now?",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	O84 = {
		Type = 3,
		SubNode = "T85",
		NextNode = "O79",
		Text = "You play pazaak?",
		HasCondition = false,
		HasResolver = false,
	},
	T85 = {
		Type = 2,
		SubNode = "W86",
		Text = "You imaginary types aren't all bad, you're on!",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	W86 = {
		Type = 7,
		SubNode = "T87",
		ScriptFunc = function(owner, ply, resumefunc, data)
			local function pzkFinish(winner)
			    if(winner == nil) then
			        owner.WonMatch = true
			    else
			        owner.WonMatch = false
			    end
			    
				resumefunc()
			end
						
			local pzk = sys.CreatePazaakGame()
			local cards = JKG.Pazaak.Cards
			pzk:SetPlayers(ply, nil)
			if not opp then
				pzk:SetAILevel(1)
				pzk:SetAIName("Lost Stormtrooper")
			end
						
			pzk:SetFinishCallback(pzkFinish)
						
			pzk:SetCards(1, {10,10,10,10,10,10,
			                10,10,10,10,10,10,
			                10,10,10,10,10,10,
			                10,10,10,10,10 } )
			
			pzk:SetCards(2, {10,10,10,10,10,10,
			                10,10,10,10,10,10,
			                10,10,10,10,10,10,
			                10,10,10,10,10 } )
			
			pzk:SetSideDeck(2, {cards.PZCARD_FLIP_1,
								cards.PZCARD_FLIP_2,
			                    cards.PZCARD_FLIP_3,
			                    cards.PZCARD_FLIP_4,
			                    cards.PZCARD_FLIP_5,
			                    cards.PZCARD_FLIP_6,
			                    cards.PZCARD_FLIP_2,
			                    cards.PZCARD_FLIP_3,
			                    cards.PZCARD_FLIP_4,
			                    cards.PZCARD_FLIP_5	
			                    } )
										
			pzk:ShowCardSelection(true)
						
			timer.Simple(50, function()
				local success, reason = pzk:StartGame()
				if not success then
					resumefunc()
				end
			end)
			return true
		end,
		HasCondition = false,
	},
	T87 = {
		Type = 2,
		SubNode = "L89",
		NextNode = "T88",
		Text = "Hahaha another win for me. You didn't put up much of a fight.\nBetter luck next time scrub.",
		Duration = 7000,
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			--[[-----------------------------------------------
				Conditional Script
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				data - Local storage table
			
				Wanted return value: Bool (true/false)
			--]]-----------------------------------------------
			if owner.WonMatch == true then
			    return true
			else
			    return false
			end
		end,
		HasResolver = false,
	},
	L89 = {
		Type = 4,
		Target = "T76",
	},
	T88 = {
		Type = 2,
		SubNode = "L90",
		Text = "Dank farrik. You had some real lucky draws.  \nWon't happen next time.  Well played.",
		Duration = 5000,
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			--[[-----------------------------------------------
				Conditional Script
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				data - Local storage table
			
				Wanted return value: Bool (true/false)
			--]]-----------------------------------------------
			if owner.WonMatch == false then
			    return true
			else
			    return false
			end
		end,
		HasResolver = false,
	},
	L90 = {
		Type = 4,
		Target = "T76",
	},
	O79 = {
		Type = 3,
		SubNode = "T80",
		NextNode = "O140",
		Text = "Can I buy something off of you?",
		HasCondition = false,
		HasResolver = false,
	},
	T80 = {
		Type = 2,
		SubNode = "W81",
		Text = "Not like I need any of this.  And you're not real anyway so what does it matter?\nUnless you have food?  No?  Well, take a look then.",
		Duration = 6000,
		HasCondition = false,
		HasResolver = false,
	},
	W81 = {
		Type = 7,
		SubNode = "D94",
		ScriptFunc = function(owner, ply, resumefunc, data)
			timer.Simple(50, function()
				local success = owner:UseVendor(ply)
			    ply:SendCommand("cin de")
			    ply:SendCommand("cin stop")
			    --ply:SendCommand("conv stop") --exclude this cause it'll break the waitfunction
			    ply:SetCinematicMode(false)
				--if success then
			    --[[
			    if success == 1 then
					resumefunc()
				end]]
			end)
			return true
		end,
		HasCondition = false,
	},
	D94 = {
		Type = 5,
	},
	O140 = {
		Type = 3,
		SubNode = "T141",
		NextNode = "O97",
		Text = "You're a really cool dude.",
		HasCondition = false,
		HasResolver = false,
	},
	T141 = {
		Type = 2,
		SubNode = "S142",
		Text = "Uh thanks.",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	S142 = {
		Type = 6,
		SubNode = "D143",
		ScriptFunc = function(owner, ply, data)
			--[[-----------------------------------------------
				Dialogue Script
			
				Available Variables:
				owner - Entity that runs this conversation
				ply	  - Player the owner is talking to
				data - Local storage table
			--]]-----------------------------------------------
			owner:SetAnimBoth("BOTH_THUMB_UP")
			ply:SetAnimBoth("BOTH_THUMB_UP")
		end,
		HasCondition = false,
	},
	D143 = {
		Type = 5,
	},
	O97 = {
		Type = 3,
		SubNode = "T98",
		NextNode = "O77",
		Text = "I know why you keep seeing things...",
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			if ply.Quests["losttrooper"].foundSpice then
			    return true
			else
			    return false
			end
		end,
		HasResolver = false,
	},
	T98 = {
		Type = 2,
		SubNode = "O99",
		Text = "...",
		Duration = 2000,
		HasCondition = false,
		HasResolver = false,
	},
	O99 = {
		Type = 3,
		SubNode = "T105",
		NextNode = "O100",
		Text = "There's a spice mine here.  [Show spice.]",
		HasCondition = false,
		HasResolver = false,
	},
	T105 = {
		Type = 2,
		SubNode = "D138",
		Text = "Here?  On this frozen wasteland.  No wonder I'm losing it.\nThe rebels must've been selling it on the blackmarket to fund their ops.\nThat also explains this horrid twitch!  But why would?\nWait...then who are you?  Are you a rebel?",
		Duration = 5000,
		HasCondition = false,
		HasResolver = false,
	},
	D138 = {
		Type = 5,
	},
	O100 = {
		Type = 3,
		SubNode = "T106",
		NextNode = "O101",
		Text = "[Lie]You were always crazy.",
		HasCondition = false,
		HasResolver = false,
	},
	T106 = {
		Type = 2,
		SubNode = "O107",
		Text = "I...I think I've always known.",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	O107 = {
		Type = 3,
		SubNode = "T108",
		NextNode = "O109",
		Text = "You ate them, didn't you?  Your fellow troopers?",
		HasCondition = false,
		HasResolver = false,
	},
	T108 = {
		Type = 2,
		SubNode = "T117",
		Text = "I...we...they were already dead...\nWe were out of rations...",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	T117 = {
		Type = 2,
		SubNode = "O118",
		Text = "I...we didn't have a choice!  You know that, you were there.\nI would have died!",
		Duration = 4000,
		HasCondition = false,
		HasResolver = false,
	},
	O118 = {
		Type = 3,
		SubNode = "T122",
		NextNode = "O119",
		Text = "It's alright.  Anyone would do the same in your position.",
		HasCondition = false,
		HasResolver = false,
	},
	T122 = {
		Type = 2,
		SubNode = "D132",
		Text = "No!  No it's not alright, it's eating me up.",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	D132 = {
		Type = 5,
	},
	O119 = {
		Type = 3,
		SubNode = "T123",
		NextNode = "O120",
		Text = "Why should you deserve to live?  Monster.",
		HasCondition = false,
		HasResolver = false,
	},
	T123 = {
		Type = 2,
		SubNode = "O124",
		Text = "You're right.  I should just end it.",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	O124 = {
		Type = 3,
		SubNode = "T126",
		NextNode = "O125",
		Text = "It'll bring you peace at last.",
		HasCondition = false,
		HasResolver = false,
	},
	T126 = {
		Type = 2,
		SubNode = "T128",
		Text = "I'm...I'm so sorry everyone.",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	T128 = {
		Type = 2,
		SubNode = "S130",
		Text = "[He reaches for his e-11 and points it at his face.]",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	S130 = {
		Type = 6,
		SubNode = "D131",
		ScriptFunc = function(owner, ply, data)
			--[[-----------------------------------------------
				Dialogue Script
			
				Available Variables:
				owner - Entity that runs this conversation
				ply	  - Player the owner is talking to
				data - Local storage table
			--]]-----------------------------------------------
			owner.GodMode = false
			owner.Health = 0
		end,
		HasCondition = false,
	},
	D131 = {
		Type = 5,
	},
	O125 = {
		Type = 3,
		SubNode = "T127",
		Text = "Die like a coward.",
		HasCondition = false,
		HasResolver = false,
	},
	T127 = {
		Type = 2,
		SubNode = "L129",
		Text = "I...",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	L129 = {
		Type = 4,
		Target = "T128",
	},
	O120 = {
		Type = 3,
		SubNode = "D133",
		NextNode = "O121",
		Text = "And you'd do it again wouldn't you?",
		HasCondition = false,
		HasResolver = false,
	},
	D133 = {
		Type = 5,
	},
	O121 = {
		Type = 3,
		SubNode = "D134",
		Text = "You need help, I can save you.",
		HasCondition = false,
		HasResolver = false,
	},
	D134 = {
		Type = 5,
	},
	O109 = {
		Type = 3,
		SubNode = "T113",
		NextNode = "O110",
		Text = "You're talking to me aren't you? Doesn't that make you crazy?",
		HasCondition = false,
		HasResolver = false,
	},
	T113 = {
		Type = 2,
		SubNode = "O114",
		Text = "I...don't know anymore.",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	O114 = {
		Type = 3,
		SubNode = "D135",
		NextNode = "O115",
		Text = "Being crazy isn't so bad.",
		HasCondition = false,
		HasResolver = false,
	},
	D135 = {
		Type = 5,
	},
	O115 = {
		Type = 3,
		SubNode = "D136",
		NextNode = "O116",
		Text = "How can you be so weak?",
		HasCondition = false,
		HasResolver = false,
	},
	D136 = {
		Type = 5,
	},
	O116 = {
		Type = 3,
		SubNode = "D137",
		Text = "I can help you.",
		HasCondition = false,
		HasResolver = false,
	},
	D137 = {
		Type = 5,
	},
	O110 = {
		Type = 3,
		SubNode = "T111",
		Text = "I'm joking!  You just need some company.",
		HasCondition = false,
		HasResolver = false,
	},
	T111 = {
		Type = 2,
		SubNode = "D112",
		Text = "Why do these hallucinations torment me so?\nGo...go away.",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	D112 = {
		Type = 5,
	},
	O101 = {
		Type = 3,
		SubNode = "D139",
		NextNode = "O102",
		Text = "You lost all your friends, anyone would go crazy...",
		HasCondition = false,
		HasResolver = false,
	},
	D139 = {
		Type = 5,
	},
	O102 = {
		Type = 3,
		SubNode = "L104",
		Text = "Nevermind.  Let's talk about something else.",
		HasCondition = false,
		HasResolver = false,
	},
	L104 = {
		Type = 4,
		Target = "T76",
	},
	O77 = {
		Type = 3,
		SubNode = "S95",
		Text = "Nevermind.",
		HasCondition = false,
		HasResolver = false,
	},
	S95 = {
		Type = 6,
		SubNode = "D96",
		ScriptFunc = function(owner, ply, data)
			--[[-----------------------------------------------
				Dialogue Script
			
				Available Variables:
				owner - Entity that runs this conversation
				ply	  - Player the owner is talking to
				data - Local storage table
			--]]-----------------------------------------------
			ply.EntPtr:PlaySound(4, "sound/chars/stofficer1/misc/giveup4.mp3")
		end,
		HasCondition = false,
	},
	D96 = {
		Type = 5,
	},
	E4 = {
		Type = 1,
		SubNode = "D67",
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			--Has done next step
			local stage = 0
			
			if ply.Quests["losttrooper"] then
				stage = ply.Quests["losttrooper"].stage
			end
			
			return stage == 3
		end,
	},
	D67 = {
		Type = 5,
	},
}
