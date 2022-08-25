--Dice Roller System
--Written by Futuza (2021)
--[[
    Simple dice rolling system.  Currently it rolls dice locally in a small range by default (same as /say's range), but you can specify a chat channel.
    eg: 
        /roll 20, rolls a 20 sided dice.
        /roll 75 338, rolls 75 338 sided dice.
        /roll 20 team, rolls a 20 sided dice for the team.
        /roll 20 tell bob, rolls a 20 sided dice and send it to bob.
]]--


-- [[ General Functions() ]] --
local vector = vectorfnc --making vector access more clear

--System replies to player
local function SystemReply(ply, message)
	ply:SendChat("^7System: " .. message)
end

local function RollOneDice(ply, sides, chan, targ)
    local result = sys.GetRandomInt(1, sides)    --using GetRandomInt(), because built in lua random functions suck
    if chan == 1 then --/say
        local k = 0
        local normalRadius = 1281
        local plyVec = vector.ObtainPlyVector(ply)

        while players.GetByID(k) ~= nil do
            local plytarg = players.GetByID(k)
            local vecResult = vector.VectorSubtract(plyVec, vector.ObtainPlyVector(plytarg))       --subtract player's origin from target's origin

            if vector.VectorLength(vecResult) < normalRadius then      --if the length is inside the radius
                local fadelevel = vector.GetChatFadeLevel(vector.VectorLength(vecResult), normalRadius)
                plytarg:SendFadedChat(fadelevel, "^3Dice: ^7" .. ply:GetName() .. "^7 rolls a " .. math.floor(sides) .. "-sided dice: ^3" .. result)
            end
            k = k + 1
        end
    elseif chan == 2 then --/team
        local k = 0
        while players.GetByID(k) ~= nil do
            local plytarg = players.GetByID(k)
        
            if ply.GetTeam(plytarg) == ply.GetTeam(ply) then
                plytarg:SendChat("^3Dice[^5Team^3]: ^5" .. ply:GetName() .. "^5 rolls a " .. math.floor(sides) .. "-sided dice: ^3" .. result)
            end
            k = k + 1
        end
    elseif chan == 3 then --/global
        chatmsg("^3Dice[^2Global^3]: ^2" .. ply:GetName() .. "^2 rolls a " .. math.floor(sides) .. "-sided dice: ^3" .. result);
    elseif chan == 4 then --/tell
        if targ == nil then
			SystemReply(ply, "^1Invalid player specified.")
			return
		end
        if targ != ply then
            ply:SendChat("^3Dice[^6Tell^3]: ^6" .. ply:GetName() .. "^6 rolls a " .. math.floor(sides) .. "-sided dice: ^3" .. result) --send to self, if we're not the target
        end
        targ:SendChat("^3Dice[^6Tell^3]: ^6" .. ply:GetName() .. "^6 rolls a " .. math.floor(sides) .. "-sided dice: ^3" .. result) --send to target
    else
        print("^3Error - aborting dice roll, invalid channel specified.")
    end
end


-- [[ Main Dice Roller ]] --
local function DiceRoller(ply, argc, argv)

	if argc > 4 or argc < 2 then
        SystemReply(ply, "^3Syntax: /roll <sides>^7, ^3/roll <dice> <sides>^7, ^3/roll <sides> <say/team/global>^7, ^3/roll <sides> <tell> <player>")
        return
	end

	local diceargs = {}     --contains dice/sides request 
    local channel = 1       --used to determine chat channel
    local loops = 1         --used to determine how many numbers we need to iterate through
    local plytarg = nil     --target player, for tell cmd

    if argc > 2 then
        loops = 2
        if argc == 3 then
            if tonumber(argv[2]) then--/roll <dice> <sides>
                channel = 3
                loops = 1
            else
                channel = string.lower(tostring(argv[2]))
                if channel == "say" then
                    channel = 1
                elseif channel == "team" then
                    channel = 2
                elseif channel == "global" then
                    channel = 3
                else
                    SystemReply(ply, "^3Syntax: /roll <sides>^7, ^3/roll <dice> <sides>^7, ^3/roll <sides> <say/team/global>^7, ^3/roll <sides> <tell> <player>")
                    return
                end
            end
        else
            if string.lower(tostring(argv[2])) == "tell" then
                channel = 4
                loops = 3
                plytarg = players.GetByArg(argv[3])
            else
                SystemReply(ply, "^3Syntax: /roll <sides>^7, ^3/roll <dice> <sides>^7, ^3/roll <sides> <say/team/global>^7, ^3/roll <sides> <tell> <player>")
                return
            end 
        end
    end

    for i=1,argc-loops do
        if tonumber(argv[i]) then
            if tonumber(argv[i]) > 0 and tonumber(argv[i]) < 32768 then --make sure it can convert to a number, and is between 2 and 32767
                table.insert( diceargs, tonumber(argv[i]) )
            else
                SystemReply(ply, "^3Error: Range of sides: 2 - 32767.")
                return
            end	
        else
            SystemReply(ply, "^3Syntax: /roll <sides>^7, ^3/roll <dice> <sides>^7, ^3/roll <sides> <say/team/global>^7, ^3/roll <sides> <tell> <player>")
            return
        end
    end


    if tablelength(diceargs) == 1 then --if rolling one die
        RollOneDice(ply, diceargs[1], channel, plytarg)
        return
    end

    if tablelength(diceargs) == 2 then --if rolling several dice
        local dice = {}
        local sum = 0
        local highest = 0
        local lowest = 0

        if(diceargs[1] > 100) then
            diceargs[1] = 100
            SystemReply(ply, "^3Error: Requested too many dice! Rolling 100 instead.")
        end

        --if it's just 1
        if(diceargs[1] == 1) then
            RollOneDice(ply, diceargs[2], channel, plytarg)
            return
        end

        chatmsg( "^3Dice[^2Global^3]: ^7Rolling " .. diceargs[1] .. ", " .. diceargs[2] .. "-sided dice for " .. ply:GetName() .. "^7:");
        lowest = diceargs[2] --initialize lowest roll
        for i=1,diceargs[1] do
            table.insert(dice, sys.GetRandomInt(1, diceargs[2]))
            sum = sum + dice[i]
            if highest < dice[i] then
                highest = dice[i]
            end

            if lowest > dice[i] then 
                lowest = dice[i]
            end

            if i < 5 then
                chatmsg("^7Roll[" .. tostring(i) ..  "]: ^3" .. dice[i])
            else
                ply:SendPrint("^7Roll[" .. tostring(i) ..  "]: ^3" .. dice[i])
            end
        end

        if tablelength(dice) > 4 then
            ply:SendChat("^3Dice: ^7Too many dice.  See console to view more dice rolls.")
        end

        chatmsg("^3Dice[^2Global]:^7 Sum: ^3" .. tostring(sum) .. "^7, Avg: ^3" .. tostring( math.floor( (sum / tablelength(dice))*100 )/100 ) .. "^7, High Roll: ^2" .. tostring(highest) .. "^7, Low Roll: ^1" .. tostring(lowest) .. "^7.") -- display summary
        return
    end

    SystemReply(ply, "^3Syntax: /roll <sides>^7, ^3/roll <dice> <sides>^7, ^3/roll <sides> <say/team/global>^7, ^3/roll <sides> <tell> <player>")
    return
end


--add to cmd table
cmds.Add("roll", DiceRoller)
chatcmds.Add("roll", DiceRoller)
