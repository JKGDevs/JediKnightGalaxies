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
		Text = "[You've entered a dialog conversation with an NPC (non player character).\nChoose how to reply.\nYou can click on the option you want to reply back with, or press the corresponding number.]",
		Duration = 10000,
		HasCondition = false,
		HasResolver = false,
	},
	T12 = {
		Type = 2,
		SubNode = "O16",
		Text = "[Pressing esc will allow you to leave the dialog (if possible).\nYou can talk to an NPC by looking at them and pressing the use button (e).]",
		Duration = 7000,
		HasCondition = false,
		HasResolver = false,
	},
	O16 = {
		Type = 3,
		SubNode = "S17",
		NextNode = "O15",
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
			ply.Quests["tutorial"].status = 1
			ply.Quests["tutorial"].objective = "I should talk to my boss to get started with my new job."
			ply:SendCenterPrint("Started: " .. ply.Quests["tutorial"].title)
			ply:SendPrint("Started " .. ply.Quests["tutorial"].title)
			ply:SendPrint("Objective: " .. ply.Quests["tutorial"].objective)
			ply.EntPtr:PlaySound(11, "sound/interface/quest/QuestStartReveal00.mp3")
		end,
		HasCondition = false,
	},
	D18 = {
		Type = 5,
	},
	O15 = {
		Type = 3,
		SubNode = "L19",
		Text = "Explain it to me again.",
		HasCondition = false,
		HasResolver = false,
	},
	L19 = {
		Type = 4,
		Target = "T11",
	},
	E2 = {
		Type = 1,
		SubNode = "T0",
		NextNode = "E29",
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
	T0 = {
		Type = 2,
		SubNode = "T9",
		Text = "There you are $name$!  Been looking for you since we dropped out of hyperspace.\nBoss wants me to show you the ropes.",
		Duration = 10000,
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
			return stage == 1
		end,
		HasResolver = true,
		ResolveFunc = function(owner, ply, var, data)
			--[[-----------------------------------------------
				Resolver Script
			
				This allows the use of variables in the text
				(single word enclosed in $ signs)
				For example: 'Hello there $name$, how are you.',
				where name is the variable here.
			
				Available Variables:
				owner - Entity that runs this conversation
				ply - Player the owner is talking to
				var - Variable to resolve (string)
				data - Local storage table
			
				Wanted return value: The text to use in place of
				the variable
			--]]-----------------------------------------------
			
			return ply.name
		end,
	},
	T9 = {
		Type = 2,
		SubNode = "T21",
		Text = "Let's get started.\nLots of things to learn about the Silver Crow before you'll be much use to anyone here.",
		Duration = 4000,
		HasCondition = false,
		HasResolver = false,
	},
	T21 = {
		Type = 2,
		SubNode = "O13",
		Text = "Why don't you go grab your things in the locker over there?",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	O13 = {
		Type = 3,
		SubNode = "T20",
		NextNode = "O12",
		Text = "Where are we?",
		HasCondition = false,
		HasResolver = false,
	},
	T20 = {
		Type = 2,
		SubNode = "L25",
		Text = "Aboard the Silver Crow, we're in orbit above Tantooine at the moment.",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	L25 = {
		Type = 4,
		Target = "T21",
	},
	O12 = {
		Type = 3,
		SubNode = "T16",
		NextNode = "O14",
		Text = "Who are you?",
		HasCondition = false,
		HasResolver = false,
	},
	T16 = {
		Type = 2,
		SubNode = "L23",
		Text = "I'm Kennigan Vleet, been with the ship for about a year and half.\nShe's got a fair captain and decent pay.  Not too much more a man could ask for.",
		Duration = 7000,
		HasCondition = false,
		HasResolver = false,
	},
	L23 = {
		Type = 4,
		Target = "T21",
	},
	O14 = {
		Type = 3,
		SubNode = "T24",
		NextNode = "O11",
		Text = "What are we doing?",
		HasCondition = false,
		HasResolver = false,
	},
	T24 = {
		Type = 2,
		SubNode = "L26",
		Text = "Supply drop off at a spaceport, Captain is just waiting for clearance to land. You can \nmake quite a profit selling supplies to Outer-Rim worlds like Tatooine that lack the\nsignificant natural resources of the Core Worlds. You and me?  We're training you.\n",
		Duration = 7000,
		HasCondition = false,
		HasResolver = false,
	},
	L26 = {
		Type = 4,
		Target = "T21",
	},
	O11 = {
		Type = 3,
		SubNode = "S27",
		Text = "You got it.  I'll be right back.",
		HasCondition = false,
		HasResolver = false,
	},
	S27 = {
		Type = 6,
		SubNode = "D28",
		ScriptFunc = function(owner, ply, data)
			--update the quest's stage to 2, and notify player
			ply.Quests["tutorial"].stage = 2
			table.insert(ply.Quests["tutorial"].questlog, ply.Quests["tutorial"].objective)
			ply.Quests["tutorial"].objective = "Kinnigan Vleet wants me to retrieve my equipment from the locker."
			ply:SendCenterPrint("Retrieve equipment from locker.")
			ply:SendPrint("Objective: " .. ply.Quests["tutorial"].objective)
			ply.EntPtr:PlaySound(11, "sound/interface/quest/notify00.wav")
		end,
		HasCondition = false,
	},
	D28 = {
		Type = 5,
	},
	E29 = {
		Type = 1,
		SubNode = "T30",
		NextNode = "E32",
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			--player has read the conversation tutorial
			local stage = 0
						
			if ply.Quests["tutorial"] then
				stage = ply.Quests["tutorial"].stage
			end
						
			return stage == 2
		end,
	},
	T30 = {
		Type = 2,
		SubNode = "S34",
		Text = "Found it without too much trouble?  Good.  Let's get started.\nYou'll notice we've include a Blastech DL-18 Blaster Pistol, Tatooine can be a rough\nplace and it is best to be prepared in case some of the locals give us trouble.  Follow\nme to the firing range and we'll run you through the basics.",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	S34 = {
		Type = 6,
		SubNode = "D35",
		ScriptFunc = function(owner, ply, data)
			--update quest to stage to 3, and notify player
			ply.Quests["tutorial"].stage = 3
			table.insert(ply.Quests["tutorial"].questlog, ply.Quests["tutorial"].objective)
			ply.Quests["tutorial"].objective = "Follow Kinnigan Vleet to the firing range."
			ply:SendCenterPrint("Follow Vleet to the firing range.")
			ply:SendPrint("Objective: " .. ply.Quests["tutorial"].objective)
			ply.EntPtr:PlaySound(11, "sound/interface/quest/notify00.wav")
		end,
		HasCondition = false,
	},
	D35 = {
		Type = 5,
	},
	E32 = {
		Type = 1,
		SubNode = "T33",
		NextNode = "E40",
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			--player has retrieved equipment
			local stage = 0
						
			if ply.Quests["tutorial"] then
				stage = ply.Quests["tutorial"].stage
			end
						
			return stage == 3
		end,
	},
	T33 = {
		Type = 2,
		SubNode = "T36",
		Text = "Found it without too much trouble?  Good.  Let's get started.\nYou'll notice we've include a Blastech DL-18 Blaster Pistol, Tatooine can be a rough\nplace and it is best to be prepared in case some of the locals give us trouble.  Follow\nme to the firing range and we'll run you through the basics.",
		Duration = 9000,
		HasCondition = false,
		HasResolver = false,
	},
	T36 = {
		Type = 2,
		SubNode = "T37",
		Text = "Once you've equipped your blaster, pick a target down the\nrange and see if you can shoot it.  Use (Mouse1) to fire,\nyou can also hold down (Mouse2) to aim down the weapon's sights.",
		Duration = 9000,
		HasCondition = false,
		HasResolver = false,
	},
	T37 = {
		Type = 2,
		SubNode = "S38",
		Text = "And make sure you aim DOWN RANGE.  I don't want to\nsee your finger on a the trigger while facing me.",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	S38 = {
		Type = 6,
		SubNode = "D39",
		ScriptFunc = function(owner, ply, data)
			--update quest to stage to 4, and notify player
			ply.Quests["tutorial"].stage = 4
			table.insert(ply.Quests["tutorial"].questlog, ply.Quests["tutorial"].objective)
			ply.Quests["tutorial"].objective = "Equip your blaster and shoot a target."
			ply:SendCenterPrint("Equip your blaster and shoot a target.")
			ply:SendPrint("Objective: " .. ply.Quests["tutorial"].objective)
			ply.EntPtr:PlaySound(11, "sound/interface/quest/notify00.wav")
		end,
		HasCondition = false,
	},
	D39 = {
		Type = 5,
	},
	E40 = {
		Type = 1,
		SubNode = "T41",
		HasCondition = true,
		ConditionFunc = function(owner, ply, data)
			--player has retrieved equipment
			local stage = 0
						
			if ply.Quests["tutorial"] then
				stage = ply.Quests["tutorial"].stage
			end
						
			return stage == 4
		end,
	},
	T41 = {
		Type = 2,
		SubNode = "D42",
		Text = "Equip your blaster and shoot a target down range!",
		Duration = 3000,
		HasCondition = false,
		HasResolver = false,
	},
	D42 = {
		Type = 5,
	},
}
