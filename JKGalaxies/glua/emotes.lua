-- Emotes System
-- Written by eezstreet (2012)

-- This is a simple emote system that I decided to hack in at the last minute. Basically adds a series of chat commands to the game, along with some help as to how to use it

local function EmoteHelp(ply, argc, argv)
	ply:SendChat("The following emotes are available: /salute, /signalRally, /signalFallIn, /signalConfusion, /signalFlashbang, /signalFreeze, /danceTwist, /danceMufa, /danceBofa")	
end

local function EmoteSalute(ply, argc, argv)
	ply:SetAnimUpper("TORSO_SALUTE")
end

local function EmoteSignalFreeze(ply, argc, argv)
	ply:SetAnimUpper("TORSO_HANDSIGNAL3")
end

local function EmoteSignalRally(ply, argc, argv)
	ply:SetAnimUpper("TORSO_HANDSIGNAL7")
end

local function EmoteSignalFallIn(ply, argc, argv)
	ply:SetAnimUpper("TORSO_HANDSIGNAL8")
end

local function EmoteSignalConfusion(ply, argc, argv)
	ply:SetAnimUpper("TORSO_HANDSIGNAL12")
end

local function EmoteSignalFlashbang(ply, argc, argv)
	ply:SetAnimUpper("TORSO_HANDSIGNAL13")
end

local function EmoteDanceTwist(ply, argc, argv)
	ply:SetAnimBoth("BOTH_TWIST")
end

local function EmoteDanceMufa(ply, argc, argv)
	ply:SetAnimBoth("BOTH_MUFA")
end

local function EmoteDanceBofa(ply, argc, argv)
	ply:SetAnimBoth("BOTH_BOFA")
end

local function EmoteSadPanda(ply, argc, argv)
	ply:SetAnimBoth("BOTH_BOW")
end

chatcmds.Add("salute", EmoteSalute)
chatcmds.Add("signalFreeze", EmoteSignalFreeze)
chatcmds.Add("signalRally", EmoteSignalRally)
chatcmds.Add("signalFallIn", EmoteSignalFallIn)
chatcmds.Add("signalConfusion", EmoteSignalConfusion)
chatcmds.Add("signalFlashbang", EmoteSignalFlashbang)
chatcmds.Add("danceTwist", EmoteDanceTwist)
chatcmds.Add("danceMufa", EmoteDanceMufa)
chatcmds.Add("danceBofa", EmoteDanceBofa)
chatcmds.Add("sadPanda", EmoteSadPanda)
chatcmds.Add("emotehelp", EmoteHelp)