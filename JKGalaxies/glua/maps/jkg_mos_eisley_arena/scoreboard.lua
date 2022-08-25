--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Code
	Tatooine arena scoreboard controls
	
	Written by BobaFett
--------------------------------------------------]]
if (RELOADING) then
	return
end

local DisplayData = {}

local function EnableSBShaderAnims(ents)
	local k,v
	for k,v in pairs(ents) do
		v:EnableShaderAnim(true)
		v:SetShaderFrame(0)
	end
end

local function SetSBShaderFrame(ents, frame)
	local k,v
	if #ents ~= 2 then
		print("WARNING: Failed to locate scoreboard digits")
	end
	for k,v in pairs(ents) do
		v:SetShaderFrame(frame)
	end
end

local function InitScoreboard()
	print("Initializing tatooine arena scoreboard")
	DisplayData.blue2 = ents.GetByName("ArenaBlueReinf2")
	EnableSBShaderAnims(DisplayData.blue2)
	DisplayData.blue1 = ents.GetByName("ArenaBlueReinf1")
	EnableSBShaderAnims(DisplayData.blue1)
	
	DisplayData.time4 = ents.GetByName("ArenaTime4")
	EnableSBShaderAnims(DisplayData.time4)
	DisplayData.time3 = ents.GetByName("ArenaTime3")
	EnableSBShaderAnims(DisplayData.time3)
	DisplayData.time2 = ents.GetByName("ArenaTime2")
	EnableSBShaderAnims(DisplayData.time2)
	DisplayData.time1 = ents.GetByName("ArenaTime1")
	EnableSBShaderAnims(DisplayData.time1)
	
	DisplayData.red2 = ents.GetByName("ArenaRedReinf2")
	EnableSBShaderAnims(DisplayData.red2)
	DisplayData.red1 = ents.GetByName("ArenaRedReinf1")
	EnableSBShaderAnims(DisplayData.red1)
	
	DisplayData.initialized = true
end

hook.Add("MapLoaded", "InitScoreboard", InitScoreboard)

-- Frame 0 = blank
-- Frame 1 = Dash
-- Frame 2+ = Digits, startin with 0

-- Special values:
-- -1 = dashes
-- -2 = blank
-- -3 = digit test (88)

function ArenaControls.SetBlueReinforcements(reinf)
	if not DisplayData.initialized then return end
	if reinf > 99 then
		reinf = 99
	elseif reinf < 0 then
		-- Special cases
		if reinf == -1 then
			-- Dashes
			SetSBShaderFrame(DisplayData.blue1, 1)
			SetSBShaderFrame(DisplayData.blue2, 1)
		elseif reinf == -2 then
			-- Blank
			SetSBShaderFrame(DisplayData.blue1, 0)
			SetSBShaderFrame(DisplayData.blue2, 0)
		elseif reinf == -3 then
			-- Digit test, so 8
			SetSBShaderFrame(DisplayData.blue1, 10)
			SetSBShaderFrame(DisplayData.blue2, 10)
		end
		return
	end
	local tens
	local ones
	tens = math.floor(reinf/10)
	ones = reinf - (tens * 10)
	-- Frame 0 = blank
	-- Frame 1 = Dash
	-- Frame 2+ = Digits, startin with 0
	SetSBShaderFrame(DisplayData.blue2, tens+2)
	SetSBShaderFrame(DisplayData.blue1, ones+2)
end

ArenaControls.SetBlueScore = ArenaControls.SetBlueReinforcements

function ArenaControls.SetRedReinforcements(reinf)
	if not DisplayData.initialized then return end
	if reinf > 99 then
		reinf = 99
	elseif reinf < 0 then
		-- Special cases
		if reinf == -1 then
			-- Dashes
			SetSBShaderFrame(DisplayData.red1, 1)
			SetSBShaderFrame(DisplayData.red2, 1)
		elseif reinf == -2 then
			-- Blank
			SetSBShaderFrame(DisplayData.red1, 0)
			SetSBShaderFrame(DisplayData.red2, 0)
		elseif reinf == -3 then
			-- Digit test, so 8
			SetSBShaderFrame(DisplayData.red1, 10)
			SetSBShaderFrame(DisplayData.red2, 10)
		end
		return
	end
	local tens
	local ones
	tens = math.floor(reinf/10)
	ones = reinf - (tens * 10)
	-- Frame 0 = blank
	-- Frame 1 = Dash
	-- Frame 2+ = Digits, startin with 0
	SetSBShaderFrame(DisplayData.red2, tens+2)
	SetSBShaderFrame(DisplayData.red1, ones+2)
end

ArenaControls.SetRedScore = ArenaControls.SetRedReinforcements

function ArenaControls.SetTime(ltime)
	if not DisplayData.initialized then return end
	if ltime < 0 then
		-- Special cases
		if ltime == -1 then
			-- Dashes
			SetSBShaderFrame(DisplayData.time1, 1)
			SetSBShaderFrame(DisplayData.time2, 1)
			SetSBShaderFrame(DisplayData.time3, 1)
			SetSBShaderFrame(DisplayData.time4, 1)
		elseif ltime == -2 then
			-- Blank
			SetSBShaderFrame(DisplayData.time1, 0)
			SetSBShaderFrame(DisplayData.time2, 0)
			SetSBShaderFrame(DisplayData.time3, 0)
			SetSBShaderFrame(DisplayData.time4, 0)
		elseif ltime == -3 then
			-- Digit test, so 8
			SetSBShaderFrame(DisplayData.time1, 10)
			SetSBShaderFrame(DisplayData.time2, 10)
			SetSBShaderFrame(DisplayData.time3, 10)
			SetSBShaderFrame(DisplayData.time4, 10)
		end
		return
	end
	local mins
	local secs
	mins = math.floor(ltime/60)
	secs = ltime-(mins*60)
	if mins > 99 then
		mins = 99
		secs = 59
	end
	
	local tens
	local ones
	tens = math.floor(mins/10)
	ones = mins - (tens * 10)
	
	SetSBShaderFrame(DisplayData.time4, tens+2)
	SetSBShaderFrame(DisplayData.time3, ones+2)
	
	tens = math.floor(secs/10)
	ones = secs - (tens * 10)
	
	SetSBShaderFrame(DisplayData.time2, tens+2)
	SetSBShaderFrame(DisplayData.time1, ones+2)
end