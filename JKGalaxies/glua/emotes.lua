-- Emotes System
-- Written by eezstreet (2012)
-- Updated by Futuza (2022)

-- This is a simple emote system that I decided to hack in at the last minute. Basically adds a series of chat commands to the game, along with some help as to how to use it. --eezstreet
-- Keep in mind that none of these do 1st person animations...but that could be possible with more work.

--
-- [[ General Functions() ]]
--



--
--[[ Emote Functions() ]]
--

local function EmoteHelp(ply, argc, argv)
	ply:SendChat("See console for list of available emotes.")
	ply:SendPrint("Emotes available: /salute, /thumbsUp, /signalYes, /signalNo, /signalRally, /signalFallIn, /signalConfusion, /signalFlashbang, /signalFreeze, /danceTwist, /danceMufa, /danceBofa, /danceFreestyle, /sitMeditate, /sitLounge, /sadPanda")
end

local function EmoteThumbsUp(ply, argc, argv)
	ply:SetAnimBoth("BOTH_THUMB_UP")
end

local function EmoteSalute(ply, argc, argv)
	ply:SetAnimUpper("TORSO_SALUTE")
end

local function EmoteSignalNo(ply, argc, argv)
	ply:SetAnimUpper("BOTH_HEADSHAKE")
end

local function EmoteSignalYes(ply, argc, argv)
	ply:SetAnimUpper("BOTH_HEADNOD")
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
	ply:SetAnimHoldTime(3, 5000)
end

local function EmoteDanceMufa(ply, argc, argv)
	ply:SetAnimBoth("BOTH_MUFA")
	ply:SetAnimHoldTime(3, 5000)
end

local function EmoteDanceBofa(ply, argc, argv)
	ply:SetAnimBoth("BOTH_BOFA")
	ply:SetAnimHoldTime(3, 5000)
end

local function EmoteDanceFreestyle(ply, argc, argv)
	ply:SetAnimBoth("BOTH_BUTTERFLY_FR1")
end

local function EmoteMeditate(ply, argc, argv)
	ply:SetAnimBoth("BOTH_SIT2")
	ply:SetAnimHoldTime(3, 7000)
	--ply:SetNoMove(true)
	--todo: figure out how to set a timer to lock player into meditate pose
	--      generate extra stamina/force while meditating, when finished unlock movement
end

local function EmoteLounge(ply, argc, argv)
	ply:SetAnimBoth("BOTH_SIT6")
	ply:SetAnimHoldTime(3, 7000)
end

local function EmoteSadPanda(ply, argc, argv)
	ply:SetAnimBoth("BOTH_BOW")
end

local function EmoteTest(ply, argc, argv)
	--ply:SetAnimBoth("BOTH_ARIAL_F1") --cartwheel
	--ply:SetAnimBoth("BOTH_BUTTERFLY_FR1") --i'm a show off
	--ply:SetAnimBoth("BOTH_SONICPAIN_HOLD") --my ears my ears
	--ply:SetAnimBoth("BOTH_SIT7") --cross legged
	ply:SetAnimBoth("BOTH_ROSH_PAIN") --exhausted/wounded
	ply:SetAnimHoldTime(3, 4000)
end

--
-- [[ Add the cmds to the game so it recongizes the new functions ]]
--

--add to chat cmdlist
chatcmds.Add("salute", EmoteSalute)
chatcmds.Add("signalThumbsUp", EmoteThumbsUp)
chatcmds.Add("thumbsUp", EmoteThumbsUp)
chatcmds.Add("signalYes", EmoteSignalYes)
chatcmds.Add("yes", EmoteSignalYes)
chatcmds.Add("no", EmoteSignalNo)
chatcmds.Add("signalNo", EmoteSignalNo)
chatcmds.Add("signalFreeze", EmoteSignalFreeze)
chatcmds.Add("signalRally", EmoteSignalRally)
chatcmds.Add("signalFallIn", EmoteSignalFallIn)
chatcmds.Add("signalConfusion", EmoteSignalConfusion)
chatcmds.Add("signalFlashbang", EmoteSignalFlashbang)
chatcmds.Add("danceTwist", EmoteDanceTwist)
chatcmds.Add("danceMufa", EmoteDanceMufa)
chatcmds.Add("danceBofa", EmoteDanceBofa)
chatcmds.Add("danceFreestyle", EmoteDanceFreestyle)
chatcmds.Add("sitMeditate", EmoteMeditate)
chatcmds.Add("sitLounge", EmoteLounge)
chatcmds.Add("sadPanda", EmoteSadPanda)
chatcmds.Add("emoteTest", EmoteTest)
chatcmds.Add("emotehelp", EmoteHelp)



--add to console cmdlist as well
cmds.Add("salute", EmoteSalute)
cmds.Add("signalThumbsUp", EmoteThumbsUp)
cmds.Add("thumbsUp", EmoteThumbsUp)
cmds.Add("signalYes", EmoteSignalYes)
cmds.Add("signalNo", EmoteSignalNo)
cmds.Add("signalFreeze", EmoteSignalFreeze)
cmds.Add("signalRally", EmoteSignalRally)
cmds.Add("signalFallIn", EmoteSignalFallIn)
cmds.Add("signalConfusion", EmoteSignalConfusion)
cmds.Add("signalFlashbang", EmoteSignalFlashbang)
cmds.Add("danceTwist", EmoteDanceTwist)
cmds.Add("danceMufa", EmoteDanceMufa)
cmds.Add("danceBofa", EmoteDanceBofa)
cmds.Add("danceFreestyle", EmoteDanceFreestyle)
cmds.Add("sitMeditate", EmoteMeditate)
cmds.Add("sitLounge", EmoteLounge)
cmds.Add("sadPanda", EmoteSadPanda)
cmds.Add("emoteTest", EmoteTest)
cmds.Add("emotehelp", EmoteHelp)

