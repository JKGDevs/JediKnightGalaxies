--[[ ------------------------------------------------
Jedi Knight Galaxies Lua Framework
Vector Functions for Lua
	
Functions for using vectors via lua and related functions.
	
Written by Futuza
--------------------------------------------------]]--

-- Locals (for faster access, and because we're about to lose access to the globals)
local tostring = tostring
local tonumber = tonumber
local sqrt = math.sqrt
local table = table
local insert = insert

module("vectorfnc")

--returns a vector/object containing the players object, (vec[1] == x, vec[2] == y, vec[3] == z)
function ObtainPlyVector(player)
    local vec = {}
    local svector = tostring(player:GetOrigin(player))
    svector = svector:gsub('Vector( ', '')
    svector = svector:gsub('[%(%)]', '')
    for i in (svector .. " "):gmatch("%S+") do
        if(tonumber(i)) then --make sure it can convert to a number
            table.insert(vec, tonumber(i))
        else
            print("Could not obtain requested origin vector!")
            return false
        end
    end

    return vec
end

--returns a vector/object containing the diff between two vectors (vec1 - vec2) without modifying either
function VectorSubtract(vec1, vec2)
    local vec = {}
    for i=1,3 do
        vec[i] = vec1[i] - vec2[i]
    end
    return vec
end

--return normalized vectorlength
function VectorLength(vec)
    return sqrt( (vec[1] * vec[1])+(vec[2] * vec[2])+(vec[3] * vec[3]) )
end

--helps calculate fade level to be used when using ply:SendFadedChat(fadelevel, "msg")
function GetChatFadeLevel(distance, range)
    --see G_Say_GetFadeLevel()
    local cutoff = range*0.75
    local cutoffrange = range*0.25
    local cutoffarea = 0
    local fadelevel = 0

    if distance > range then
        return 15
    end

    if distance < cutoff then
        return 100
    end

    cutoffarea = distance - cutoff
    fadelevel = 1 - cutoffarea/cutoffrange

    if fadelevel < 0.15 then
        fadelevel = 0.15
    elseif fadelevel > 1 then
        fadelevel = 1
    end

    return fadelevel*100
end
