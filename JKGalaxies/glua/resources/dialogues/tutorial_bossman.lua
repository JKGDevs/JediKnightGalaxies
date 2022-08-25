-- Exported by JKG Dialogue Creator

DLG.Name = "tutorial_bossman"
DLG.RootNode = "E1"
DLG.Nodes = {
	E1 = {
		Type = 1,
		SubNode = "T11",
		NextNode = "E2",
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			--basic operating procedure for starting a quest
			--Check if quests are not setup, 
			--setup if they aren't, then initialize the quest in either case
			local stage = 0
			local qn = "tutorial" --shorthand for questname
						
			if not ply.Quests or not ply.Quests[qn] then
				if InitQuest(ply, qn) then
					ply.Quests[qn].title = "Tutorial"
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
	T11 = {
		Type = 2,
		SubNode = "T12",
		Text = "[You've entered a dialog conversation with an NPC(non player character).\nMost NPCs will have something to say, and then you can choose how to reply.\nYou can click on the option you want to reply back with, or press the corresponding digit.\nBeware actions may have consequences, be careful of what you say back.]",
		Duration = 15000,
		HasCondition = false,
		HasResolver = false,
	},
	T12 = {
		Type = 2,
		SubNode = "T14",
		Text = "[Pressing esc will allow you to leave the dialog (if possible).\nSome NPCs can only be talked to if you've met certain prerequisites.\nYou can start talking to an NPC by looking at them and pressing the use button (e).]",
		Duration = 10000,
		HasCondition = false,
		HasResolver = false,
	},
	T14 = {
		Type = 2,
		SubNode = "O15",
		Text = "[You just started your first quest.  Quests are usually objectives or jobs to do.\nYou'll usually be rewarded with valuables and xp after completing them.  Completing a\nquest will usually improve your relationship with those who gave it to you and increase\ntheir trust in you.]",
		Duration = 10000,
		HasCondition = false,
		HasResolver = false,
	},
	O15 = {
		Type = 3,
		SubNode = "L19",
		NextNode = "O16",
		Text = "Explain it to me again.",
		HasCondition = false,
		HasResolver = false,
	},
	L19 = {
		Type = 4,
		Target = "T11",
	},
	O16 = {
		Type = 3,
		SubNode = "S17",
		Text = "Okay, I got it.",
		HasCondition = false,
		HasResolver = false,
	},
	S17 = {
		Type = 6,
		SubNode = "D18",
		ScriptFunc = function(owner, ply, data)
			--update the quest's stage to 1, and notify player
			ply.Quests["tutorial"].stage = 1
			ply:SendCenterPrint("Started: " .. ply.Quests["tutorial"].title)
			ply:SendPrint("Started " .. ply.Quests["tutorial"].title)
			ply.EntPtr:PlaySound(11, "sound/interface/quest/QuestStartReveal00.mp3")
		end,
		HasCondition = false,
	},
	D18 = {
		Type = 5,
	},
	E2 = {
		Type = 1,
		SubNode = "T20",
		NextNode = "E3",
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			--player has read the conversation tutorial
			local stage = 0
						
			if ply.Quests["tutorial"] then
				stage = ply.Quests["tutorial"].stage
			end
						
			return stage == 1
		end,
	},
	T20 = {
		Type = 2,
		SubNode = "T26",
		Text = "About time.  Been looking for you since we dropped out of hyperspace.\nIf I'm going to show you the ropes, we better get started.\nLots of things to learn about the Silver Crow before you'll be much use to anyone here.\nFirst time off world right?  I'll get you up to speed in no time.",
		Duration = 6000,
		HasCondition = false,
		HasResolver = false,
	},
	T26 = {
		Type = 2,
		SubNode = "O21",
		Text = "I'm Kinnigan Vleet, Master Chief Petty Officer of the Crow.\nYou're new, but so was I once.  Why don't you go grab your things in the locker over there?",
		Duration = 4000,
		HasCondition = false,
		HasResolver = false,
	},
	O21 = {
		Type = 3,
		SubNode = "T28",
		NextNode = "O22",
		Text = "How do I open the locker?",
		HasCondition = false,
		HasResolver = false,
	},
	T28 = {
		Type = 2,
		SubNode = "L29",
		Text = "Just walk over to it, look at it and press e.  It should pop right open.",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	L29 = {
		Type = 4,
		Target = "T26",
	},
	O22 = {
		Type = 3,
		SubNode = "T30",
		NextNode = "O23",
		Text = "How long have you been working on this ship?",
		HasCondition = false,
		HasResolver = false,
	},
	T30 = {
		Type = 2,
		SubNode = "O31",
		Text = "Year and a half.  She's been real good to me.\nCaptain's fair, pay's good, and that's all I can really ask for.",
		Duration = 4000,
		HasCondition = false,
		HasResolver = false,
	},
	O31 = {
		Type = 3,
		SubNode = "S35",
		NextNode = "O32",
		Text = "[Persuade]How much are they paying you?",
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			if ply.Quests["tutorial"].talkMoney then
			    return true
			else
			    return false
			end
		end,
		HasResolver = false,
	},
	S35 = {
		Type = 6,
		SubNode = "T36",
		ScriptFunc = function(owner, ply, data)
			--[[-----------------------------------------------
				Dialogue Script
			
				Available Variables:
				owner - Entity that runs this conversation
				ply	  - Player the owner is talking to
				data - Local storage table
			--]]-----------------------------------------------
			
			
			local result = sys.GetRandomInt(1, 2)
			if result == 2 then
			    ply.Quests["tutorial"].talkMoney = true
			else
			    ply.Quests["tutorial"].talkMoney = false
			end
		end,
		HasCondition = false,
	},
	T36 = {
		Type = 2,
		SubNode = "T38",
		NextNode = "T37",
		Text = "[Success]: About 5000 credits a week.  You do what I say, and I'm sure you'll deserve a pay raise in no time.",
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
			if ply.Quests["tutorial"].talkMoney then
			    return true
			else
			    return false
			end
		end,
		HasResolver = false,
	},
	T38 = {
		Type = 2,
		SubNode = "S39",
		Text = "Anyway like I was saying...",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	S39 = {
		Type = 6,
		SubNode = "L40",
		ScriptFunc = function(owner, ply, data)
			--[[-----------------------------------------------
				Dialogue Script
			
				Available Variables:
				owner - Entity that runs this conversation
				ply	  - Player the owner is talking to
				data - Local storage table
			--]]-----------------------------------------------
			ply.Quests["tutorial"].talkMoney = false
		end,
		HasCondition = false,
	},
	L40 = {
		Type = 4,
		Target = "T26",
	},
	T37 = {
		Type = 2,
		SubNode = "L41",
		Text = "[Failed]: None of your business, but you do what I say and you'll be due for a raise in no time.",
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
			if ply.Quests["tutorial"].talkMoney then
			    return false
			else
			    return true
			end
		end,
		HasResolver = false,
	},
	L41 = {
		Type = 4,
		Target = "T38",
	},
	O32 = {
		Type = 3,
		SubNode = "T33",
		Text = "I have another question.",
		HasCondition = false,
		HasResolver = false,
	},
	T33 = {
		Type = 2,
		SubNode = "L34",
		Text = "Sure thing.",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	L34 = {
		Type = 4,
		Target = "T26",
	},
	O23 = {
		Type = 3,
		SubNode = "T42",
		NextNode = "O24",
		Text = "Where are we at?",
		HasCondition = false,
		HasResolver = false,
	},
	T42 = {
		Type = 2,
		SubNode = "T43",
		Text = "Dantooine.  Dropping off supplies.  I'll show you what later.",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	T43 = {
		Type = 2,
		SubNode = "L44",
		Text = "Anyway...",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	L44 = {
		Type = 4,
		Target = "T26",
	},
	O24 = {
		Type = 3,
		SubNode = "T45",
		NextNode = "O25",
		Text = "What are we doing?",
		HasCondition = false,
		HasResolver = false,
	},
	T45 = {
		Type = 2,
		SubNode = "O46",
		Text = "Training you of course!\nI was hoping you'd be a little brighter than this...",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	O46 = {
		Type = 3,
		SubNode = "T50",
		NextNode = "O47",
		Text = "No, I mean what is the ship doing here?",
		HasCondition = false,
		HasResolver = false,
	},
	T50 = {
		Type = 2,
		SubNode = "T51",
		Text = "Supply run to some colonials on Dantooine.  They don't have a lot of raw materials.",
		Duration = 4000,
		HasCondition = false,
		HasResolver = false,
	},
	T51 = {
		Type = 2,
		SubNode = "L52",
		Text = "Anyway we should get back to the task at hand...",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	L52 = {
		Type = 4,
		Target = "T26",
	},
	O47 = {
		Type = 3,
		SubNode = "T48",
		Text = "Don't treat me like a child.  You know what I meant.",
		HasCondition = false,
		HasResolver = false,
	},
	T48 = {
		Type = 2,
		SubNode = "L49",
		Text = "No, I really don't.",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	L49 = {
		Type = 4,
		Target = "T26",
	},
	O25 = {
		Type = 3,
		SubNode = "S53",
		Text = "Got it, I'll go grab it.",
		HasCondition = false,
		HasResolver = false,
	},
	S53 = {
		Type = 6,
		SubNode = "D54",
		ScriptFunc = function(owner, ply, data)
			--update the quest's stage to 2, and notify player
			ply.Quests["tutorial"].stage = 2
			ply:SendCenterPrint("Objective: Get your things from the locker.")
			ply:SendPrint("Objective: Get your things from the locker.")
			ply.EntPtr:PlaySound(11, "sound/interface/quest/notify00.wav")
		end,
		HasCondition = false,
	},
	D54 = {
		Type = 5,
	},
	E3 = {
		Type = 1,
		SubNode = "T55",
		NextNode = "E4",
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			--player has been told to check locker
			local stage = 0
						
			if ply.Quests["tutorial"] then
				stage = ply.Quests["tutorial"].stage
			end
						
			return stage == 2
		end,
	},
	T55 = {
		Type = 2,
		SubNode = "O56",
		Text = "Ready to start learning?  \nGot your things from the locker?",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	O56 = {
		Type = 3,
		SubNode = "T60",
		NextNode = "O57",
		Text = "I'm ready.",
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			--player has aquired stuff from locker
			local stage = 0
			stage = ply.Quests["tutorial"].stage
			
			if stage == 3 then
				return true
			else
			    return false
			end
		end,
		HasResolver = false,
	},
	T60 = {
		Type = 2,
		SubNode = "S61",
		Text = "Okay perfect.  Follow me to the room over, we'll start by getting you stocked with ammo.\nIn our line of work, there's always a risk of being boarded.\nIt's good to know you have ammunition at hand in case things...well get out of hand.",
		Duration = 6000,
		HasCondition = false,
		HasResolver = false,
	},
	S61 = {
		Type = 6,
		SubNode = "D62",
		ScriptFunc = function(owner, ply, data)
			--update the quest's stage to 4, and notify player
			ply.Quests["tutorial"].stage = 4
			ply:SendCenterPrint("Objective: Follow Officer Vleet.")
			ply:SendPrint("Objective: Follow Officer Vleet.")
			ply.EntPtr:PlaySound(11, "sound/interface/quest/notify00.wav")
		end,
		HasCondition = false,
	},
	D62 = {
		Type = 5,
	},
	O57 = {
		Type = 3,
		SubNode = "T58",
		Text = "Give me another moment.",
		HasCondition = false,
		HasResolver = false,
	},
	T58 = {
		Type = 2,
		SubNode = "D59",
		Text = "Alright, but don't take all day.",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	D59 = {
		Type = 5,
	},
	E4 = {
		Type = 1,
		SubNode = "T63",
		NextNode = "E5",
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			--player has been told to check locker
			local stage = 0
						
			if ply.Quests["tutorial"] then
				stage = ply.Quests["tutorial"].stage
			end
						
			return stage == 4
		end,
	},
	T63 = {
		Type = 2,
		SubNode = "T64",
		Text = "Okay you can resupply by talking to Toni here, he's a vendor and would normally charge, but this is a business expense so the boss pays for it.\n  Well most of it.  When you open the shop, you'll see your inventory on the left, and what's available on the right.\nSelect an item on the right and press the buy button in the middle to complete the transaction.  You can press esc when finished.",
		Duration = 4000,
		HasCondition = false,
		HasResolver = false,
	},
	T64 = {
		Type = 2,
		SubNode = "O65",
		Text = "You got what you need?",
		Duration = 4000,
		HasCondition = false,
		HasResolver = false,
	},
	O65 = {
		Type = 3,
		SubNode = "T68",
		NextNode = "O66",
		Text = "Yeah.",
		HasCondition = false,
		HasResolver = false,
	},
	T68 = {
		Type = 2,
		SubNode = "S73",
		Text = "Alright!  Let's see how your aim is.",
		Duration = 4000,
		HasCondition = false,
		HasResolver = false,
	},
	S73 = {
		Type = 6,
		SubNode = "D74",
		ScriptFunc = function(owner, ply, data)
			--update the quest's stage to 5, and notify player
			ply.Quests["tutorial"].stage = 5
			ply:SendCenterPrint("Objective: Shoot the target.")
			ply:SendPrint("Objective: Shoot the target.")
			ply.EntPtr:PlaySound(11, "sound/interface/quest/notify00.wav")
		end,
		HasCondition = false,
	},
	D74 = {
		Type = 5,
	},
	O66 = {
		Type = 3,
		SubNode = "D67",
		Text = "One more thing...",
		HasCondition = false,
		HasResolver = false,
	},
	D67 = {
		Type = 5,
	},
	E5 = {
		Type = 1,
		SubNode = "T69",
		NextNode = "E6",
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			--player needs to shoot target
			local stage = 0
						
			if ply.Quests["tutorial"] then
				stage = ply.Quests["tutorial"].stage
			end
						
			return stage == 5
		end,
	},
	T69 = {
		Type = 2,
		SubNode = "O70",
		Text = "Take a shot at the target on the wall over there.  You can select your weapon by pressing the corresponding number key the weapon is assigned to or using your mouse wheel to scroll to the weapon.\nYou can fire by pressing Mouse1 and you can aim by moving the mouse.  You can help focus on your target and increase your accuracy by looking down the sites of the gun.\nPress Mouse2 to aim down the gun's site, or scope if it has one.\nMost weapons have a limited number of charges or ammunition they can hold.  You can reload by pressing R, you can see how many total shots you have left in the current clip by looking at the number in the bottom right corner.  The number in parenthesis represents how many total clips you have.",
		Duration = 10000,
		HasCondition = false,
		HasResolver = false,
	},
	O70 = {
		Type = 3,
		SubNode = "D75",
		NextNode = "O71",
		Text = "You got it bossman.",
		HasCondition = false,
		HasResolver = false,
	},
	D75 = {
		Type = 5,
	},
	O71 = {
		Type = 3,
		SubNode = "L72",
		Text = "Can you repeat that again?",
		HasCondition = false,
		HasResolver = false,
	},
	L72 = {
		Type = 4,
		Target = "T69",
	},
	E6 = {
		Type = 1,
		SubNode = "T76",
		NextNode = "E7",
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			--player has been told to check locker
			local stage = 0
						
			if ply.Quests["tutorial"] then
				stage = ply.Quests["tutorial"].stage
			end
						
			return stage == 6
		end,
	},
	T76 = {
		Type = 2,
		SubNode = "T77",
		Text = "Nice shooting.  Just so you know, some guns have multiple firing modes.\nYou can toggle between them with MOUSE3 or L.\nIf you buy special ammunition you can insert it by pressing N.\nIf you crouch (C) while shooting, you're going to be more accurate.\nRunning while shooting makes your aim sloppier, but can help you avoid taking unfriendly blasters shots to your face.",
		Duration = 10000,
		HasCondition = false,
		HasResolver = false,
	},
	T77 = {
		Type = 2,
		SubNode = "O78",
		Text = "If you do happen to get in a firefight with someone, it's usually a good idea to keep moving.  Try straffing back and forth with A or D.\nSometimes rolling out of the way can help you dodge the worst of the damage, just press C and W at the same time to roll forward.\nIf you find it easier to keep aware of your surroundings, you can press Q to toggle between 3rd Person and 1st Person perspectives.  \nGot it?",
		Duration = 8000,
		HasCondition = false,
		HasResolver = false,
	},
	O78 = {
		Type = 3,
		SubNode = "L79",
		NextNode = "O80",
		Text = "Sorry, can you repeat it once more?",
		HasCondition = false,
		HasResolver = false,
	},
	L79 = {
		Type = 4,
		Target = "T76",
	},
	O80 = {
		Type = 3,
		SubNode = "T81",
		Text = "Got it boss.",
		HasCondition = false,
		HasResolver = false,
	},
	T81 = {
		Type = 2,
		SubNode = "D82",
		Text = "Alright nice work.  Let's talk about equipment next?\nYou can press I to bring up your inventory, from there you can decide what equipment to use on hand or store away.\nFor active weaponry or equipment, assign it to an ACI slot to use it.  For armor or other equippables, just select the item and press equip.\n",
		Duration = 1000,
		HasCondition = false,
		HasResolver = false,
	},
	D82 = {
		Type = 5,
	},
	E7 = {
		Type = 1,
		SubNode = "D83",
		NextNode = "E8",
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			--player has been told to check locker
			local stage = 0
						
			if ply.Quests["tutorial"] then
				stage = ply.Quests["tutorial"].stage
			end
						
			return stage == 7
		end,
	},
	D83 = {
		Type = 5,
	},
	E8 = {
		Type = 1,
		SubNode = "D84",
		NextNode = "E9",
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			--player has been told to check locker
			local stage = 0
						
			if ply.Quests["tutorial"] then
				stage = ply.Quests["tutorial"].stage
			end
						
			return stage == 8
		end,
	},
	D84 = {
		Type = 5,
	},
	E9 = {
		Type = 1,
		SubNode = "D85",
		NextNode = "E10",
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			--player has been told to check locker
			local stage = 0
						
			if ply.Quests["tutorial"] then
				stage = ply.Quests["tutorial"].stage
			end
						
			return stage == 9
		end,
	},
	D85 = {
		Type = 5,
	},
	E10 = {
		Type = 1,
		SubNode = "D86",
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			--player has been told to check locker
			local stage = 0
						
			if ply.Quests["tutorial"] then
				stage = ply.Quests["tutorial"].stage
			end
						
			return stage == 10
		end,
	},
	D86 = {
		Type = 5,
	},
}
