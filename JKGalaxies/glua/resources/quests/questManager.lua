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
    data.name = name        --short reference name
    data.title = ""         --display name of the quest
    data.objective = ""     --the current objective of the quest
    data.hidden = false     --if it can be viewed in the datapad/questlog
    data.status = 0         --0 == quest not started, 1 = in progress, 2 = incomplete, -1 = failed
    data.active = true      --true = set as an active quest / false = not on active quest list (can still be completed, but doesn't show on map/markers etc)
    data.questlog = {}     --contains a list of all the completed quest objectives

    --assign to player
    ply.Quests[name] = data
    ply.QstCount = ply.QstCount + 1
    print(name .. " quest initialized.")
    return true
end

--==============
--helper functions 
--==============

--return true if quest system isn't initialized, otherwise return false
local function isQuestManagerInvalid(ply)
    if ply.Quests == nil then
        ply:SendPrint("System: ^3Quest system not initialized for " .. ply.Name .. "^3, this player has not started any quests.")
        return true
    else
        return false
    end
end

--convert status of quest to string and returns value
local function getQuestStatusString(value)
    local status
    if value == -1 then
        status = "^1Failed^7"
    elseif value == 0 then
        status = "^3Not Started^7"
    elseif value == 1 then
        status = "In Progress"
    elseif value == 2 then
        status = "^2Complete^7"
    else
        status = "^3Unknown^7"
    end
    return status
end

--==============
--functions 
--==============

--lists all the quests in ply.Quests[]
--usage: /listquests
local function ListQuests(ply)
    if isQuestManagerInvalid(ply) then
        return
    end

    ply:SendPrint(ply.Name .. "^7 has started " .. ply.QstCount .. " quests:")
    ply:SendPrint("|   (Name)   |    Quest Title    | Stage |  Status  |")
    ply:SendPrint("-----------------------------------------------------")
    if type(ply.Quests) == 'table' then
        for k,v in pairs(ply.Quests) do
            if type(k) ~= 'number' then
                if ply.Quests[k].title ~= nil then
                    ply:SendPrint("| (" .. ply.Quests[k].name .. ") | " .. ply.Quests[k].title .. " | " .. ply.Quests[k].stage .. " | " .. getQuestStatusString(ply.Quests[k].status) .. " |")
                else
                    ply:SendPrint("| (" .. ply.Quests[k].name ") | <Untitled Quest> | " .. ply.Quests[k].stage .. " | " .. getQuestStatusString(ply.Quests[k].status) .. " |")
                end
            end
        end
    end
end

--displays the current objective of a quest
--usage: /quest.getobjective <questname>
local function GetQuestObjective(ply, argc, argv)
    if (argc < 2 or argc > 2) then
		ply:SendPrint("/quest.getstage <questname>")
		return
	end

    if isQuestManagerInvalid(ply) then
        return
    end

	local questname = argv[1];
	if questname ~= nil then
        if ply.Quests[questname] then
            ply:SendPrint(ply.Quests[questname].title .. " objective: ")
            ply:SendPrint(ply.Quests[questname].objective)
            return
        else
            ply:SendPrint("^1No active quests.")
            return
        end
	end

    ply:SendPrint("System: ^1Invalid questname specified.")
    return
end

local function GetQuestLog(ply, argc, argv)
    if (argc < 2 or argc > 2) then
        ply:SendPrint("/quest.questlog <questname>")
        return
    end

    if isQuestManagerInvalid(ply) then
        return
    end

    local questname = argv[1];
    if questname ~= nil then
        if ply.Quests[questname] then
            if next(ply.Quests[questname].questlog) == nil then
                ply:SendPrint("Questlog: ^3<empty>^7")
            else
                for i, v in ipairs(ply.Quests[questname].questlog) do
                    ply:SendPrint("Questlog: ")
                    ply:SendPrint("[" .. i .. "] " .. ply.Quests[questname].questlog[i])
                end
            end
            if ply.Quests[questname].status < 0 then
                ply:SendPrint("-\n^1Failed Objective^7: " .. ply.Quests[questname].objective)
            else
                ply:SendPrint("-\n^2Current Objective^7: " .. ply.Quests[questname].objective)
            end
        else
            ply:SendPrint("System: ^1Invalid questname ^7'" .. questname .. "'^1 specified.")
            return
        end
    else
        ply:SendPrint("System: ^1Invalid questname specified.")
        return
    end
end

--checks the stage of the specified quest
--usage: /quest.getstage <questshortname>
local function GetQuestStage(ply, argc, argv)
    if (argc < 2 or argc > 2) then
		ply:SendPrint("/quest.getstage <questname>")
		return
	end

    if isQuestManagerInvalid(ply) then
        return
    end

	local questname = argv[1];
    if questname ~= nil then
        if ply.Quests[questname] then
            ply:SendPrint(ply.Quests[questname].title .. " is at stage " .. ply.Quests[questname].stage)
            return
        end
    end

    ply:SendPrint("System: ^1Invalid questname specified.")
end

--note this may break NPCs if they are doing anything other than checking the quest stage  --futuza
local function SetQuestStage(ply, argc, argv)
    if ply.CanUseCheats ~= true then
        ply:SendPrint("System: ^1You do not have permission to perform this action.^7")
            if ply.isLoggedIn ~= true then
                ply:SendPrint("System: ^1You are not logged in.^7")
            end
        return
    end

    if (argc < 3 or argc > 3) then
		ply:SendPrint("/quest.setstage <questname> <stage>")
		return
	end

    if isQuestManagerInvalid(ply) then
        return
    end

	local questname = argv[1];
	if questname == nil then
		ply:SendPrint("^1Invalid questname specified.")
		return
	end

    local stageval = argv[2];
    if (tonumber(stageval)) then
        if ply.Quests[questname] then
            ply.Quests[questname].stage = tonumber(stageval)
            ply:SendPrint(ply.Quests[questname].title .. " set to stage " .. ply.Quests[questname].stage)
            return
        else
            ply:SendPrint("'" .. questname .. "' is not a valid questname or " .. ply.Name .. "^7 has not yet started this quest.")
            return
        end
    end

    ply:SendPrint("^1Invalid stage specified, must be a number.")
end


function QuestHelp(ply)
    ply:SendPrint("The following quest cmds are available:")
    ply:SendPrint("quest.help - get help with using the quest system")
    ply:SendPrint("quest.getstage - check a specified quest's current stage")
    ply:SendPrint("quest.setstage - manually set a specified quest to a value ^3(debug)^7")
    ply:SendPrint("quest.objective - list a specified quest's current objective")
    ply:SendPrint("quest.questlog - list a specified quest's questlog (history)")
    ply:SendPrint("quest.list - list status info about all initialized quests")
end

cmds.Add("quest", QuestHelp)
cmds.Add("quest.help", QuestHelp)
cmds.Add("quest.getstage", GetQuestStage)
cmds.Add("quest.setstage", SetQuestStage)
cmds.Add("quest.objective", GetQuestObjective)
cmds.Add("quest.questlog", GetQuestLog)
cmds.Add("quest.list", ListQuests)