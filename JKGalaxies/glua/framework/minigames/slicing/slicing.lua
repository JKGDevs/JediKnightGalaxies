--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Code
	Slicing
	
	Written by BobaFett
--------------------------------------------------]]

JKG.Slicing = JKG.Slicing or {}

include("programs.lua")

local SlicingGame = {}

local NODETYPE_ALARM = 0
local NODETYPE_RELAY = 1
local NODETYPE_RESET = 2
local NODETYPE_ACCESS1 = 3
local NODETYPE_ACCESS2 = 4
local NODETYPE_ACCESS3 = 5
local NODETYPE_ACCESS4 = 6
local NODETYPE_ACCESS5 = 7

SlicingGame.__index = SlicingGame

--[[
		Creation
]]

function sys.CreateSlicingGame()
	local o = {}
	setmetatable(o, SlicingGame)
	o.Width = 0			-- Field width
	o.Height = 0		-- Field height
	o.NodeCounts = {	-- Node quantities (used for grid generation)
		Alarm = 0,
		Reset = 0,
		Access = {0,0,0,0,0}
	}
	o.Grid = nil			-- Security grid (generated when the game is started)
	o.ActiveGrid = nil		-- Grid of activated nodes (generated and filled when game is in progress)
	o.AccessLevels = nil	-- Access Level progression
	o.UnlockLevel = 0
		
	o.Intrusion = false		-- Intrusion detection present?
	o.IntrusionTimer = 0	-- Intrusion start time
	o.IntrusionTriggered = false -- Intrusion triggered?
	o.IntrusionTimerName = nil	-- Name of the intrusion timer
	
	o.Warnings = 0			-- Current warning level
	o.WarningThreshold = 0	-- Warning threshold (max level)
	o.SecurityLevels = 0	-- Amount of security levels (1-5)
	o.Active = false		-- Game in progress?
	o.Player = nil			-- Player who's playing
	
	o.DisableGrid = false	-- Disable the grid (prevent activation of any nodes,  temporarily enabled when a node is activated)
	
	o.FinishCallback = nil	-- Callback to call when the game has finished
	o.CallbackUserData = nil -- Userdata to pass along to the callback
	
	o.ProgramList = {}		-- List of available programs (taken from JKG.Slicing.Programs)
	o.ProgramCount = 0
	o.ProgramData = {}		-- Storage for programs
	
	o.MsgStream = nil		-- Message Bitstream to use for networking
	
	o.Seed = nil			-- Generator seed
	
	-- Callback function prototype: func(state, response, data)
	o.DlgCallback = nil		-- Dialog Callback function
	o.DlgCallbackData = nil
	return o
end

--[[
		Configuration functions
]]

function SlicingGame:SetFieldSize(width, height)
	if (self.Active) then return end
	self.Width = width or self.Width
	self.Height = height or self.Height
end

function SlicingGame:SetNodeCounts(alarm, reset, access1, access2, access3, access4, access5)
	if (self.Active) then return end
	self.NodeCounts.Alarm = alarm or self.NodeCounts.Alarm
	self.NodeCounts.Reset = reset or self.NodeCounts.Reset
	
	self.NodeCounts.Access[1] = access1 or self.NodeCounts.Access[1]
	self.NodeCounts.Access[2] = access2 or self.NodeCounts.Access[2]
	self.NodeCounts.Access[3] = access3 or self.NodeCounts.Access[3]
	self.NodeCounts.Access[4] = access4 or self.NodeCounts.Access[4]
	self.NodeCounts.Access[5] = access5 or self.NodeCounts.Access[5]
end

function SlicingGame:SetWarningThreshold(threshold)
	if (self.Active) then return end
	self.WarningThreshold = threshold or self.WarningThreshold
end

function SlicingGame:SetSecurityLevelCount(count)
	if (self.Active) then return end
	self.SecurityLevels = count or self.SecurityLevels
end

function SlicingGame:SetIntrusionDetection(enabled, timer)
	if (self.Active) then return end
	self.Intrusion = enabled
	self.IntrusionTimer = timer or self.IntrusionTimer
end

function SlicingGame:SetCallback(callback, userdata)
	if (self.Active) then return end
	self.FinishCallback = callback
	self.CallbackUserData = userdata
end

function SlicingGame:SetSeed(seed)
	self.Seed = seed
end

function SlicingGame:AddProgram(name)
	if (self.Active) then return end
	if (JKG.Slicing.Programs[name]) then
		if (not self.ProgramList[name]) then
			self.ProgramCount = self.ProgramCount + 1
		end
		self.ProgramList[name] = JKG.Slicing.Programs[name]
	end
end

--[[
		Gameplay
]]

function SlicingGame:StartGame(ply)
	if (self.Active) then return end
	local ret, errmsg
	
	self.Player = ply
	
	ret, errmsg = self:__VerifyParameters()
	if (not ret) then
		printf("Cannot start slicing minigame: %s", errmsg)
		return false
	end
	ply.__sys.SlicingGame = self
	
	-- Generate the grid
	self:__GenerateGrid()
	
	self.AccessLevels = {0,0,0,0,0}
	
	-- We're good to go, tell the player to start up
	self:__NetStartGame()
	self.Active = true
	
	-- Transmit the messages
	self:__NetTransmit()
end

--[[
		Internal functions
]]

function SlicingGame:__VerifyParameters()
	
	local i
	
	if (self.Width < 1 or self.Width > 8) then
		return false, "Invalid field width"
	end
	
	if (self.Height < 1 or self.Height > 8) then
		return false, "Invalid field height"
	end
	
	if (self.NodeCounts.Alarm < 0) then
		return false, "Invalid alarm node count"
	end
	
	if (self.NodeCounts.Reset < 0) then
		return false, "Invalid reset node count"
	end
	
	if (self.SecurityLevels < 1 or self.SecurityLevels > 5) then
		return false, "Invalid security level count"
	end
	
	for i=1,5 do
		if (self.SecurityLevels >= i) then
			if (self.NodeCounts.Access[i] < 1) then
				return false, "Missing access nodes for security level " .. i
			end
		else
			if (self.NodeCounts.Access[i] ~= 0) then
				return false, "Access nodes set for unused security level " .. i
			end
		end
	end
	
	local total = self.NodeCounts.Alarm + self.NodeCounts.Reset + self.NodeCounts.Access[1] + self.NodeCounts.Access[2] + self.NodeCounts.Access[3] + self.NodeCounts.Access[4] + self.NodeCounts.Access[5]
	
	if (total > self.Width * self.Height) then
		return false, "Cannot fit all nodes on the field"
	end
	
	if (self.Intrusion) then
		if (self.IntrusionTimer % 10 ~= 0) then
			return false, "Intrustion timer is not a multiple of 10 seconds"
		end
	end

	return true
end


function SlicingGame:__GenerateGrid()
	local x, y, i, idx
	
	local array = {}
	
	
	self.Grid = {}
	self.ActiveGrid = {}
	
	-- Initialize the PRNG
	if (not self.Seed) then
		self.Seed = math.random(1, 999999)
	end
	local rng = prng.Create(self.Seed)		
	
	-- Generate initial grid
	for y=1,self.Height do
		self.Grid[y] = {}
		self.ActiveGrid[y] = {}
		for x=1,self.Width do

			self.Grid[y][x] = NODETYPE_RELAY
			self.ActiveGrid[y][x] = false
			
			table.insert(array, {x=x,y=y})
		end
	end
	
	-- Place the alarm nodes
	for i=1,self.NodeCounts.Alarm do
		idx = rng:Rand(#array)
		x = array[idx].x
		y = array[idx].y
		self.Grid[y][x] = NODETYPE_ALARM
		table.remove(array, idx)
	end
	
	-- Place the reset nodes
	for i=1,self.NodeCounts.Reset do
		idx = rng:Rand(#array)
		x = array[idx].x
		y = array[idx].y
		self.Grid[y][x] = NODETYPE_RESET
		table.remove(array, idx)
	end
	
	-- Place the access nodes
	for j=1,self.SecurityLevels do
		for i=1,self.NodeCounts.Access[j] do
			idx = rng:Rand(#array)
			x = array[idx].x
			y = array[idx].y
			self.Grid[y][x] = NODETYPE_ACCESS1 + (j-1)
			table.remove(array, idx)
		end
	end
end

function SlicingGame:__IntrusionStart()
	self:__NetIntrusion(1)
	self.IntrusionTimerName = "slcintr" .. self.Player.ID
	timer.Create(self.IntrusionTimerName, self.IntrusionTimer * 1000, 1, self.__IntrusionTrigger, self)
	self.IntrusionTriggered = true
end

function SlicingGame:__IntrusionAbort()
	if (self.IntrusionTriggered) then
		self.IntrusionTriggered = false
		timer.Remove(self.IntrusionTimerName)
		self:__NetIntrusion(2)
	end
end

function SlicingGame:__IntrusionTrigger()
	self.IntrusionTriggered = false
	self:__NetIntrusion(3)
	self:__ActivateAlarm()
end

function SlicingGame:__LockField(locked)
	self.DisableGrid = locked
	self:__NetLock(locked)
end

function SlicingGame:__UpdateSecurityLevels()
	local i
	local unlocked = 0
	local seclvl = {}
		
	for i=1,self.SecurityLevels do
		if (self.AccessLevels[i] == 0) then
			seclvl[i] = 0
		elseif (self.AccessLevels[i] == self.NodeCounts.Access[i]) then
			if (i > 1 and seclvl[i-1] ~= 3) then
				seclvl[i] = 2
			else
				seclvl[i] = 3
				unlocked = unlocked + 1
			end
		else
			seclvl[i] = 1
		end
	end
	
	self:__NetUpdateSecLevels(seclvl)
	
	self.UnlockLevel = unlocked
	if (unlocked == self.SecurityLevels) then
		self:__Victory()
	end
end

function SlicingGame:__ActivateNode(row, col)
	if (self.DisableGrid) then 
		return
	end
	
	if (self.ActiveGrid[row][col]) then
		return		-- Node is already active
	end
	local id = self.Grid[row][col]
		
	self.ActiveGrid[row][col] = true
	
	if (self.Intrusion and not self.IntrusionTriggered) then
		self:__IntrusionStart()
	end
	
	self:__NetRevealNode(row, col, id)
	if (id == NODETYPE_ALARM) then
		self:__LockField(true)
		timer.Simple(500, self.__ActivateAlarm, self)
	elseif (id == NODETYPE_RESET) then
		self.Warnings = 0
		self:__NetWarningLevel(0)
		--self:__ShowDialog(2, "You have activated a reset node\nThe warning level has been reset", nil, nil)
	elseif (id >= NODETYPE_ACCESS1 and id <= NODETYPE_ACCESS5) then
		local level = id - NODETYPE_ACCESS1 + 1
		self.AccessLevels[level] = self.AccessLevels[level] + 1
		self:__UpdateSecurityLevels()
	end
	
	self:__NetTransmit()
end

function SlicingGame:__ActivateAlarm()
	if (self.Intrusion) then
		self:__IntrusionAbort()
	end
	self:__NetAlarm()
	self:__LockField(true)
	self:__ShowDialog(2, "Your actions have triggered the alarm\nYou have been locked out of the system", self.__DlgFailedCallback, nil)
	self:__NetTransmit()
end

function SlicingGame:__Victory()
	if (self.Intrusion) then
		self:__IntrusionAbort()
	end
	self:__LockField(true)
	self:__ShowDialog(2, "You have successfully obtained the highest security clearance!", self.__DlgVictoryCallback, nil)
	self:__NetTransmit()
end

function SlicingGame:__RaiseWarningLevel(amount)
	self.Warnings = self.Warnings + amount
	self:__NetWarningLevel(self.Warnings)
	
	if (self.Warnings > self.WarningThreshold) then
		self:__LockField(true)
		timer.Simple(500, self.__ActivateAlarm, self)
		return true
	else 
		return false
	end
end

function SlicingGame:__ShowDialog(mode, message, callback, data)
	self.DlgCallback = callback
	self.DlgCallbackData = data
	self:__NetDialog(mode, message)
end

function SlicingGame:__FinishGame(result)
	self:__NetEndGame()
	self:__NetTransmit()
	-- Ensure the client gets out of the minigame before calling the callback
	-- As the callback might bring up something else (like a convo)
	
	if (self.FinishCallback) then
		self.FinishCallback(self.Player, result, self.CallbackUserData)
	end
	
	self.Player.__sys.SlicingGame = nil
	self.Active = false
end

function SlicingGame:__RunProgram(progname, arg)
	if (self.DisableGrid) then 
		return
	end
	
	if (not self.ProgramList[progname]) then
		return
	else
		if (self.Intrusion and not self.IntrusionTriggered) then
			self:__IntrusionStart()
		end
		
		if (self.ProgramList[progname].Type == 0) then
			self.ProgramList[progname].Func(self)
		else 
			self.ProgramList[progname].Func(self, tonumber(arg or 0))
		end
	end
	
	self:__NetTransmit()
end

function SlicingGame:__StopSlicing()
	self:__LockField(true)
	if (self.Intrusion and self.IntrusionTriggered) then
		self:__IntrusionAbort()
		self:__NetIntrusion(3)
		self.UnlockLevel = -1
		self:__NetAlarm()
		self:__ShowDialog(2, "As you haven't been able to obtain full security clearance\nthe intrusion detection has activated the alarm.", self.__DlgStoppedCallback, nil)
	else
		if (self.UnlockLevel > 0) then
			self:__ShowDialog(2, "You have successfully obtained security clearance level " .. self.UnlockLevel .. "!", self.__DlgStoppedCallback, nil)
		else
			self:__ShowDialog(2, "You haven't been able to obtain any security clearance.", self.__DlgStoppedCallback, nil)
		end		
	end
	self:__NetTransmit()
end

--[[
		Dialog callbacks
]]

function SlicingGame:__DlgFailedCallback(response, data)
	self:__FinishGame(-1)
end

function SlicingGame:__DlgVictoryCallback(response, data)
	self:__FinishGame(self.SecurityLevels)
end

function SlicingGame:__DlgStoppedCallback(response, data)
	self:__FinishGame(self.UnlockLevel)
end

--[[
		Networking Functions
]]

local NET_BITS = 4

local NET_EOM = 0
local NET_START = 1
local NET_STOP = 2
local NET_CONFIG = 3
local NET_REVEAL = 4
local NET_LOCK = 5
local NET_PROGLST = 6
local NET_SHOWMSG = 7
local NET_ENDMSG = 8
local NET_SUMMARY = 9
local NET_SECUPDATE = 10
local NET_INTRUSION = 11
local NET_WARNLEVEL = 12
local NET_BLINKNODE = 13
local NET_INITFIELD = 14
local NET_ALARM = 15

-- Transmit the bitstream message (add a EOM code to it and send it)
function SlicingGame:__NetTransmit()
	if (not self.MsgStream) then
		return
	end
	
	self.MsgStream:WriteBits(NET_BITS, NET_EOM)		-- End-Of-Message
	self.Player:SendCommand(string.format("slc %s", encoding.Base128Encode(self.MsgStream:GetData())))
	
	self.MsgStream = nil
end

function SlicingGame:__NetStartGame()
	if (not self.MsgStream) then
		self.MsgStream = bitstream.Create(840)
	end
	
	-- Send Start command
	self.MsgStream:WriteBits(NET_BITS, NET_START)
	
	-- Send Config command
	self.MsgStream:WriteBits(NET_BITS, NET_CONFIG)
	
	self.MsgStream:WriteBits(3, self.Width - 1)
	self.MsgStream:WriteBits(3, self.Height - 1)
	self.MsgStream:WriteBits(3, self.SecurityLevels)
	self.MsgStream:WriteBits(5, self.WarningThreshold)
	self.MsgStream:WriteBool(self.Intrusion)
	if (self.Intrusion) then
		self.MsgStream:WriteByte(math.floor(self.IntrusionTimer / 10))
	end
	
	-- Send the program list
	local k,v
	self.MsgStream:WriteBits(NET_BITS, NET_PROGLST)
	self.MsgStream:WriteBits(4, self.ProgramCount)
	for k,v in pairs(self.ProgramList) do
		self.MsgStream:WriteString(k)
		self.MsgStream:WriteString(v.Name)
		self.MsgStream:WriteString(v.Desc)
		self.MsgStream:WriteBits(2, v.Type)
	end
	
	-- Send Initfield command
	self.MsgStream:WriteBits(NET_BITS, NET_INITFIELD)	
end

function SlicingGame:__NetEndGame()
	if (not self.MsgStream) then
		self.MsgStream = bitstream.Create(840)
	end
	
	-- Send Start command
	self.MsgStream:WriteBits(NET_BITS, NET_STOP)
end

function SlicingGame:__NetRevealNode(row, col, id)
	if (not self.MsgStream) then
		self.MsgStream = bitstream.Create(840)
	end
	
	self.MsgStream:WriteBits(NET_BITS, NET_REVEAL)
	self.MsgStream:WriteBits(3, row - 1)
	self.MsgStream:WriteBits(3, col - 1)
	self.MsgStream:WriteBits(3, id)
end

function SlicingGame:__NetWarningLevel(level)
	if (not self.MsgStream) then
		self.MsgStream = bitstream.Create(840)
	end
	
	self.MsgStream:WriteBits(NET_BITS, NET_WARNLEVEL)
	self.MsgStream:WriteBits(5, level)
end

function SlicingGame:__NetAlarm()
	if (not self.MsgStream) then
		self.MsgStream = bitstream.Create(840)
	end
	
	self.MsgStream:WriteBits(NET_BITS, NET_ALARM)
end

function SlicingGame:__NetLock(lock)
	if (not self.MsgStream) then
		self.MsgStream = bitstream.Create(840)
	end
	
	self.MsgStream:WriteBits(NET_BITS, NET_LOCK)
	self.MsgStream:WriteBool(NET_BITS, lock)
end

function SlicingGame:__NetUpdateSecLevels(seclevels)
	if (not self.MsgStream) then
		self.MsgStream = bitstream.Create(840)
	end
	local i
	
	self.MsgStream:WriteBits(NET_BITS, NET_SECUPDATE)
	for _,i in ipairs(seclevels) do
		self.MsgStream:WriteBits(2, i)
	end
end

function SlicingGame:__NetBlink(row, col, safe)
	if (not self.MsgStream) then
		self.MsgStream = bitstream.Create(840)
	end
	
	self.MsgStream:WriteBits(NET_BITS, NET_BLINKNODE)
	self.MsgStream:WriteBits(3, row - 1)
	self.MsgStream:WriteBits(3, col - 1)
	self.MsgStream:WriteBool(safe)
end

-- Modes:
-- 0 - No prompt (use __NetEndDialog)
-- 1 - Yes/No
-- 2 - Ok only
function SlicingGame:__NetDialog(mode, message)
	if (not self.MsgStream) then
		self.MsgStream = bitstream.Create(840)
	end
	
	self.MsgStream:WriteBits(NET_BITS, NET_SHOWMSG)
	self.MsgStream:WriteBits(2, mode)
	
	local msglines = string.split(message, "\n")
	self.MsgStream:WriteString(msglines[1] or "")
	self.MsgStream:WriteString(msglines[2] or "")
	self.MsgStream:WriteString(msglines[3] or "")
end

function SlicingGame:__NetEndDialog()
	if (not self.MsgStream) then
		self.MsgStream = bitstream.Create(840)
	end
	
	self.MsgStream:WriteBits(NET_BITS, NET_ENDMSG)

end


function SlicingGame:__NetSummary(summary)
	if (not self.MsgStream) then
		self.MsgStream = bitstream.Create(840)
	end
	local i
	self.MsgStream:WriteBits(NET_BITS, NET_SUMMARY)
	for _,i in ipairs(summary) do
		self.MsgStream:WriteBits(6, i.Value)
		self.MsgStream:WriteBits(4, i.Alarms)
	end
end

function SlicingGame:__NetIntrusion(state)
	if (not self.MsgStream) then
		self.MsgStream = bitstream.Create(840)
	end
	
	self.MsgStream:WriteBits(NET_BITS, NET_INTRUSION)
	self.MsgStream:WriteBits(2, state)
end

--[[
		Client commands
]]

local function SliceCommand(ply, argc, argv)
	local cmd = argv[1]
	local slc = ply.__sys.SlicingGame
	if not slc then
		return
	end
	
	if (cmd == "actnode") then
		local idx = tonumber(argv[2])
		local col = (idx % 8) + 1
		local row = math.floor(idx / 8) + 1
		slc:__ActivateNode(row, col)
	elseif (cmd == "runprog") then
		slc:__RunProgram(argv[2], argv[3])
	elseif (cmd == "dlgresp") then
		if (slc.DlgCallback) then
			slc.DlgCallback(slc, tonumber(argv[2]), slc.DlgCallbackData)
			slc.DlgCallback = nil	-- To avoid multiple responses
		end
	elseif (cmd == "stop") then
		slc:__StopSlicing()
	end
end

cmds.Add("~slc", SliceCommand)