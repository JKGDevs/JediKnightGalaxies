--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Framework
	Quest Manager
	
	DO NOT MODIFY THIS FILE
	
	Written by Futuza
    Contains functions useful for managing/testing quests.
--------------------------------------------------]]


--standardized quest constructor--
--to use: InitQuest(ply, "shortquestname"), returns true if successful
function InitQuest(ply, name)
    if name == nil then
        print("Invalid quest name!")
		return false
	end

    --if individual ply.Quest not setup
    if not ply.Quests then
        print("Initializing quest system for " .. ply.Name .. "^7...")
        ply.Quests = {}
        ply.QstCount = 0
    end

    --initialize data
    local data = {}
    data.stage = 0          --what stage we're on
                            --0==not started, 1==started, 2==1st part done, etc.  Can only use numbers.
    data.title = ""        --display name of the quest
    data.name = name        --short reference name
    data.hidden = false      --if it can be viewed in the datapad/questlog
        
    --assign to player
    ply.Quests[name] = data
    ply.QstCount = ply.QstCount + 1
    print(name .. " quest initialized.")
    return true
end

--[[
local function ListQuests(ply)

    lists all the quests in ply.Quests[]

end
]]

--checks the stage of the specified quest
--usage: /getquest.stage <questshortname>
local function GetQuestStage(ply, argc, argv)
    if (argc < 2 or argc > 2) then
		ply:SendPrint("/getquest.stage <questname>")
		return
	end

    if ply.Quests == nil then
        ply.Quests = Quests
    end

	local questname = argv[1];
	if questname ~= nil then
        if ply.Quests then
            if ply.Quests[questname] then
                ply:SendPrint(ply.Quests[questname].title .. " is at stage " .. ply.Quests[questname].stage)
                return
            end
        else
            ply:SendPrint("^1No active quests.")
            return
        end
	end

    ply:SendPrint("^1Invalid questname specified.")
    return
end

--Broken function, will change quest stage, but still breaks npcs - needs more work --futuza
local function SetQuestStage(ply, argc, argv)
    if (argc < 3 or argc > 3) then
		ply:SendPrint("/setquest.stage <questname> <stage>")
		return
	end

	local questname = argv[1];
	if questname == nil then
		ply:SendPrint("^1Invalid questname specified.")
		return
	end

    local stageval = argv[2];
    if(tonumber(stageval)) then
        if ply.Quests then
            if ply.Quests[questname] then
                ply.Quests[questname].stage = stageval
                ply:SendPrint(ply.Quests[questname].title .. " set to stage " .. ply.Quests[questname].stage)
                return
            end
        else
            ply:SendPrint("^1No active quests.")
            return
        end
    end
        
    ply:SendPrint("^1Invalid stage specified, must be a number.")
    return
end


cmds.Add("getquest.stage", GetQuestStage)
cmds.Add("setquest.stage", SetQuestStage)
