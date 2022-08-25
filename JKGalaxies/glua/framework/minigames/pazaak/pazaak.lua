--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Code
	Pazaak
	
	Written by BobaFett
--------------------------------------------------]]

JKG.Pazaak = JKG.Pazaak or {}

-- Load card definitions and functions
include("cards.lua")
-- Load utility functions
include("util.lua")
-- Load AI
include("AI.lua")

	
local function CheckVector(vec, sError)
	if vec ~= nil then 
		if type(vec) == "userdata" then
			if vec.ObjName == "Vector" then 
				return true
			end
		end
	end
	if sError then
		error(sError)
	else
		return false
	end
end

local function CheckEntity(ent, sError)
	if ent ~= nil then 
		if type(ent) == "userdata" then
			if ent.ObjName == "Entity" then 
				return true
			end
		end
	end
	if sError then
		error(sError)
	else
		return false
	end
end

local function CheckPlayer(ply, sError)
	if ply ~= nil then 
		if type(ply) == "userdata" then
			if ply.ObjName == "Player" then 
				return true
			end
		end
	end
	if sError then
		error(sError)
	else
		return false
	end
end

local PazaakGame = {}

PazaakGame.__index = PazaakGame

function sys.CreatePazaakGame()
	local o = {}
	setmetatable(o, PazaakGame)
	o.Deck = nil	-- Card deck, will be generated at the start of the set
	o.NextDeckCard = 1 -- Next card to draw from the deck
	o.Phase = 0		-- (pending) Phase of the turn
					-- 0: Game init
					-- 1: Initial turn/turn switch
					-- 2: Draw card
					-- 3: Use hand
					-- 4: Turn end
					-- 5: Game finished
	o.AIGame = false	-- If this is played vs AI
	o.FirstPlayer = 0	-- First player to begin at the next set
	o.NewSet = true -- True if we need to start a new set
	o.Finished = false -- True if the pazaak game is finished
	o.Winner = nil	-- Player that won the pazaak game (nil if the AI won), only valid if Finished is true
	
	o.Players =	{	-- Player table
		{
			Cards = nil,	-- Deck of the player
			SideDeck = nil,	-- 10 card side-deck used by the player
			Hand = nil,		-- 4 cards in the hand of this player
			NextFieldSlot = 1, -- Next free field slot
			Field = nil,	-- the 9 field slots of the player
			FieldParams = nil, -- Params of the 9 field slots
			Points = 0,		-- Card total
			Score = 0,		-- Sets won
			Standing = false, -- Whether the player is standing
			CanUseCard = false, -- Whether or not we can use cards still
			DlgAccepted = false, -- Whether the player accepted the opened dialog
			Forfeited = false, -- Whether the player forfeited the match
			Player = nil,	-- The player who's using this slot
			Ready = false,	-- Player is ready to play
		},
		{					-- Player 2
			IsAI = false,	-- Is this player run by AI?
			AIName = "AI",
			AIRoutine = nil, -- Set to the AI routine to use (if AI is used to begin with)
			Cards = nil,	-- Deck of the player
			SideDeck = nil,	-- 10 card side-deck used by the player
			Hand = nil,		-- 4 cards in the hand of this player
			NextFieldSlot = 1, -- Next free field slot
			Field = nil,	-- the 9 field slots of the player
			FieldParams = nil, -- Params of the 9 field slots
			Points = 0,		-- Card total
			Score = 0,		-- Sets won
			Standing = false, -- Whether the player is standing
			CanUseCard = false, -- Whether or not we can use cards still
			DlgAccepted = false, -- Whether the player accepted the opened dialog
			Forfeited = false, -- Whether the player forfeited the match
			Player = nil,	-- The player who's using this slot (can be nil if AI is used)
			Ready = false,	-- Player is ready to play
		}
	}
	
	o.TieBreaker = false -- Player used a tie-breaker card in the last turn
	o.Turn = 0			-- Whose turn it is (0 = game hasnt started :P)
	o.FinishCallback = nil 	-- Function to call when the game has finished
	o.ShowCardSelect = true -- Whether or not to show card selection first
	o.TimerName = nil		-- Name for the delay timer
	o.TimeoutName = nil		-- Name for the timeout timer (for players only)
	return o
end


function PazaakGame:SetPlayers(ply1, ply2)
	if ply2 then
		-- 2 player game
		self.Players[2].IsAI = false
		self.Players[2].AIRoutine = nil
	else
		self.Players[2].IsAI = true
	end
	self.Players[1].Player = ply1
	self.Players[2].Player = ply2
	self.Players[1].Player.Busy = true --mark player1 with busy status
	
	
	if ply2 then
	self.Players[2].Player.Busy = true --mark player2 with busy status
	--futuza: enable invincibility if there are 2 players
	self.Players[1].Player.GodMode = true
	self.Players[1].Player.NoKnockback = true
	self.Players[1].Player.NoDebuff = true
	self.Players[2].Player.GodMode = true
	self.Players[2].Player.NoKnockback = true
	self.Players[2].Player.NoDebuff = true
	print( tostring(self.Players[1].Player:GetName()) .. "^7 and " .. tostring(self.Players[2].Player:GetName()) .. "^7 have been made invulnerable for a Pazaak match." )
	end
end

function PazaakGame:SetAILevel(level)
	if level == 1 then
		self.Players[2].AIRoutine = JKG.Pazaak.AI.Normal
	end
end

function PazaakGame:ShowCardSelection(bool)
	self.ShowCardSelect = bool
end

function PazaakGame:SetAIName(name)
	self.Players[2].AIName = name
end

function PazaakGame:SetCards(player, cards)
	self.Players[player].Cards = cards
end

function PazaakGame:SetSideDeck(player, sidedeck)
	self.Players[player].SideDeck = sidedeck
end

function PazaakGame:SetFinishCallback(callback)
	self.FinishCallback = callback
end

function PazaakGame:IsValidSideDeck(sidedeck)
	if not sidedeck then
		return false
	end
	if not #sidedeck == 10 then
		return false
	end
	local k,v
	local cards = JKG.Pazaak.Cards
	for k,v in pairs(sidedeck) do
		if v < cards.PZCARD_PLUS_1 or v > cards.PZCARD_3N6 then
			return false
		end
	end
	return true
end

function PazaakGame:IsValidDeck(deck)
	if not deck then
		return false
	end
	if not #deck == 23 then
		return false
	end
	local k,v
	local count = 0
	for k,v in pairs(deck) do
		count = count + v
	end
	if count > 9 then
		return true
	else 
		return false
	end
end


function PazaakGame:ValidateSideDeck(sidedeck, deck)
	-- Check whether the side deck fits the player's cards
	-- This means it'll check whether the side deck contains cards the player doesnt have, or more of a single card than the player has
	local i
	local card
	local cards = JKG.Pazaak.Cards
	local cardcount = {0,0,0,0,0,0,
					   0,0,0,0,0,0,
					   0,0,0,0,0,0,
					   0,0,0,0,0}		-- Array we'll use to count and compare
	
	for i=1,10 do
		card = sidedeck[i]
		if card < cards.PZCARD_PLUS_1 or card > cards.PZCARD_3N6 then
			return false
		end
		cardcount[card - cards.PZCARD_PLUS_1 + 1] = cardcount[card - cards.PZCARD_PLUS_1 + 1] + 1
	end
	-- We counted the cards, now lets compare the results to our deck
	for i=1,23 do
		if deck[i] < cardcount[i] then
			return false
		end
	end
	return true
end

function PazaakGame:PrepareForNewSet()
	-- Initialize the field
	self.Players[1].Field =  { -1, -1, -1,
							   -1, -1, -1,
							   -1, -1, -1 }
	self.Players[2].Field =  { -1, -1, -1,
							   -1, -1, -1,
							   -1, -1, -1 }
	self.Players[1].FieldParams =  { 0, 0, 0,
									 0, 0, 0,
									 0, 0, 0 }
	self.Players[2].FieldParams =  { 0, 0, 0,
									 0, 0, 0,
									 0, 0, 0 }
	
	self.Players[1].NextFieldSlot = 1
	self.Players[2].NextFieldSlot = 1
	
	-- Reset the points
	self.Players[1].Points = 0
	self.Players[2].Points = 0
	
	-- Ensure no-one is 'standing'
	self.Players[1].Standing = false
	self.Players[2].Standing = false
	
	-- Create a new deck
	self.Deck = JKG.Pazaak.GenerateDeck()
	self.NextDeckCard = 1
end

function PazaakGame:GetTurnIDs()
	-- Utility function, returns the ID of the current player, and the opponent, respectively
	if self.Turn == 1 then
		return 1, 2
	else
		return 2, 1
	end
end

function PazaakGame:StartGame()
	-- Sanity checks, ensure we got proper data available
		
	-- First, validate our sidedecks, if anyone is bad, bail out
	if self.ShowCardSelect then
		if not self:IsValidDeck(self.Players[1].Cards) then
			return false, "Invalid deck (Player 1)"
		end
		if not self:IsValidSideDeck(self.Players[1].SideDeck) then
			-- Not mandatory, but if it's not valid, create an empty one
			self.Players[1].SideDeck = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1}
		elseif not self:ValidateSideDeck(self.Players[1].SideDeck, self.Players[1].Cards) then
			self.Players[1].SideDeck = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1}
		end
		if self.Players[2].IsAI then
			-- The AI doesnt pick cards, so check the side-deck instead
			if not self:IsValidSideDeck(self.Players[2].SideDeck) then
				return false, "Invalid side deck (Player 2)"
			end
		else
			if not self:IsValidDeck(self.Players[2].Cards) then
				return false, "Invalid deck (Player 2)"
			end
			if not self:IsValidSideDeck(self.Players[2].SideDeck) then
				-- Not mandatory, but if it's not valid, create an empty one
				self.Players[2].SideDeck = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1}
			elseif not self:ValidateSideDeck(self.Players[2].SideDeck, self.Players[2].Cards) then
				self.Players[2].SideDeck = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1}
			end
		end
	else
		-- If we skip card selection, our sidedecks must be valid
		if not self:IsValidSideDeck(self.Players[1].SideDeck) then
			return false, "Invalid side deck (Player 1)"
		end	
		if not self:IsValidSideDeck(self.Players[2].SideDeck) then
			return false, "Invalid side deck (Player 2)"
		end
	end
	
	--- Check if the players are valid
	if not self.Players[1].Player or not self.Players[1].Player:IsValid() then
		return false, "Player 1 invalid"
	end
	if not self.Players[2].IsAI and (not self.Players[2].Player or not self.Players[2].Player:IsValid()) then
		return false, "Player 2 invalid"
	end
	
	-- Alright, initial checkups passed, lets go into phase 0 and prepare everything
	self.Phase = 0
	if self.Players[2].IsAI then
		self.AIGame = true
	else
		self.AIGame = false
	end
	
	-- Start pazaak for the client(s)
	self.Players[1].Player:SendCommand("pzk start")
	if not self.AIGame then
		self.Players[2].Player:SendCommand("pzk start")
	end
	
	-- Put a reference to this object in the player's data, so we can access it later (commands 'n such)
	self.Players[1].Player.__sys.PazaakGame = self
	if not self.AIGame then
		self.Players[2].Player.__sys.PazaakGame = self
	end
	
	-- Create a proxy for the player if it's AI, so we can still use SendCommand and GetName on both, but on the second player it just wont have much effect
	if self.AIGame then
		local name = self.Players[2].AIName
		self.Players[2].Player = { SendCommand = function() end, GetName = function() return name end }
	end
	
	-- Generate timer names
	local suffix
	if self.AIGame then
		suffix = string.format("%i_AI", self.Players[1].Player:GetID())
	else
		suffix = string.format("%i_%i", self.Players[1].Player:GetID(), self.Players[2].Player:GetID())
	end
	self.TimerName = "PzkTimer" .. suffix
	self.TimeoutName = "PzkTimeout" .. suffix
	
	self.Players[1].Ready = false
	self.Players[2].Ready = false
	if self.ShowCardSelect then
		self:CardSelection()
	else
		self:PrepareMatch()
	end
	return true
end

function PazaakGame:CardSelection()
	self.Phase = -1
	-- Network all our cards (oooh boy)
	local i
	local SB = sys.CreateStringBuilder()
	SB:Append("pzk gtc sdc ")
	for i=1,23 do
		SB:Append(string.format("%i ", self.Players[1].Cards[i]))
	end
	SB:Append("ssd ")
	for i=1,10 do
		SB:Append(string.format("%i ", self.Players[1].SideDeck[i]))
	end
	if not self.Players[2].IsAI then
		SB:Append("sto 60 ")	-- PvP, so enable a 60 second time limit
	end
	self.Players[1].Player:SendCommand(SB:ToString())
	if not self.Players[2].IsAI then
		SB:Clear()
		SB:Append("pzk gtc sdc ")
		for i=1,23 do
			SB:Append(string.format("%i ", self.Players[2].Cards[i]))
		end
		SB:Append("ssd ")
		for i=1,10 do
			SB:Append(string.format("%i ", self.Players[2].SideDeck[i]))
		end
		SB:Append("sto 60 ")
		self.Players[2].Player:SendCommand(SB:ToString())
		timer.Create(self.TimeoutName, 60000, 1, self.CardSelTimeout, self)
	else
		self.Players[2].Ready = true
	end
end

function PazaakGame:CardSelTimeout()
	-- Alright, it's taking too long, send the tsdn command to force transfer of sidedecks
	if not self.Players[1].Ready then
		self.Players[1].Player:SendCommand("pzk tsdn sto 0")
	end
	if not self.Players[2].Ready then
		self.Players[2].Player:SendCommand("pzk tsdn sto 0")
	end
end

function PazaakGame:PrepareMatch()
	timer.Remove(self.TimeoutName)
	
	self.Players[1].Ready = true
	self.Players[2].Ready = true
	
	-- Create hands
	self.Players[1].Hand = JKG.Pazaak.GenerateHand(self.Players[1].SideDeck)
	self.Players[2].Hand = JKG.Pazaak.GenerateHand(self.Players[2].SideDeck)
	
	-- Initialize the new set
	self:PrepareForNewSet()
	self.NewSet = false
	
	-- Clear the score
	self.Players[1].Score = 0
	self.Players[2].Score = 0
	
	-- The players are not forfeiting.. not yet that is :P
	self.Players[1].Forfeited = false
	self.Players[2].Forfeited = false
	
	-- Determine who'll start
	self.FirstPlayer = math.random(1,2)
	-- Set Turn to 0, so it'll be set to FirstPlayer in phase 1
	self.Turn = 0
	
	-- Alright, we're all set. Now lets make sure the participants are set too
	-- Start pazaak and network the names 'n hand cards
	self.Players[1].Player:SendCommand(string.format("pzk gtg db 0 sto 0 sn \"%s\" \"%s\" shcs %i 0 %i 0 %i 0 %i 0 15",
														self.Players[1].Player:GetName(),
														self.Players[2].Player:GetName(),
														self.Players[1].Hand[1],
														self.Players[1].Hand[2],
														self.Players[1].Hand[3],
														self.Players[1].Hand[4]) )

	if not self.AIGame then	-- For big messages, do the check anyway, so we dont waste time
		self.Players[2].Player:SendCommand(string.format("pzk gtg db 0 sto 0 sn \"%s\" \"%s\" shcs %i 0 %i 0 %i 0 %i 0 15",
														self.Players[2].Player:GetName(),
														self.Players[1].Player:GetName(),
														self.Players[2].Hand[1],
														self.Players[2].Hand[2],
														self.Players[2].Hand[3],
														self.Players[2].Hand[4]) )
	end
	
	
	self.Phase = 1
	self:QueuePhase(250)
end

function PazaakGame:QueuePhase(delay)
	timer.Create(self.TimerName, delay, 1, self.RunPhase, self)
end

function PazaakGame:RunPhase()
	-- Little wrapper here, this will disable all timers, and then pass us along to the designated RunPhasex function
	timer.Remove(self.TimeoutName)
	-- Call RunPhasex
	self["RunPhase" .. self.Phase](self)
end

function PazaakGame:RunPhase1()
	--[[ 
		JKG Pazaak - PHASE 1: Turn Initialisation
	
		Tasks:
		
		* Verify players
		* Start a new set if needed
		* Update the turn indicator
	]]

	-- Verify players (TODO)
	
	
	
	-- Start new set if needed
	if self.NewSet then
		-- Yep, we need to start a new set
		self:PrepareForNewSet()
		self.Players[1].Player:SendCommand("pzk cf")
		self.Players[2].Player:SendCommand("pzk cf")
		self.NewSet = false
	end
	
	-- Switch turn
	if self.Turn == 0 then
		self.Turn = self.FirstPlayer
	elseif self.Turn == 1 then
		self.Turn = 2
	else
		self.Turn = 1
	end
	
	-- Update turn indicator
	if self.Turn == 1 then
		self.Players[1].Player:SendCommand("pzk st 1")
		self.Players[2].Player:SendCommand("pzk st 2")
	else
		self.Players[1].Player:SendCommand("pzk st 2")
		self.Players[2].Player:SendCommand("pzk st 1")
	end
	
	-- Go to the next Phase
	self.Phase = 2
	self:QueuePhase(250)
end

function PazaakGame:RunPhase2()
	--[[ 
		JKG Pazaak - PHASE 2: Draw Deck Card
	
		Tasks:
		
		* Verify players
		* Draw the next card from the deck
		* Update the fields and points
		* Check if we hit 20, if so, stand and go to phase 4
		* If this card fills out field, stand as well (go to phase 4 too)
	]]
	-- Verify players (TODO)
	
	
	-- First, are we standing?
	local player = self.Players[self.Turn]
	if player.Standing then
		-- Yep we are, skip to phase 4
		self.Phase = 4
		self:RunPhase4() -- Go there right away
		return
	end
	
	-- Draw the next card
	local newcard = self.Deck[self.NextDeckCard]
	local pid, oid = self:GetTurnIDs()
	self.NextDeckCard = self.NextDeckCard + 1
		
	newcard = newcard - 1 -- the deck goes from 1 to 10, the ID's for them are 0 to 9, so we do -1
	
	-- Put the new card into the fields
	local nextslot = player.NextFieldSlot
	player.Field[nextslot] = newcard
	player.NextFieldSlot = nextslot + 1
	
	player.Points = player.Points + newcard + 1
	
	if player.NextFieldSlot == 10 or player.Points == 20 then
		-- We just filled our slots or hit 20, stand
		player.Standing = true
		self.Players[pid].Player:SendCommand(string.format("pzk sc %i %i 0 spt 1 %i ss 1 1", nextslot, newcard, player.Points, newcard))
		self.Players[oid].Player:SendCommand(string.format("pzk sc %i %i 0 spt 2 %i ss 2 1", nextslot+9, newcard, player.Points, newcard))
		self.Phase = 4
		self:QueuePhase(250)
		return
	end
	
	self.Players[pid].Player:SendCommand(string.format("pzk sc %i %i 0 spt 1 %i", nextslot, newcard, player.Points, newcard))
	self.Players[oid].Player:SendCommand(string.format("pzk sc %i %i 0 spt 2 %i", nextslot+9, newcard, player.Points, newcard))
	
	-- Go to the next Phase
	self.Phase = 3
	self:QueuePhase(250)
end

function PazaakGame:PlayCard(card, param)
	-- Place the specified card on the board
	-- Assumes the current player is doing it
	-- Will automatically recalculate points

	local cardid, cardval
	local cards = JKG.Pazaak.Cards
	local i
	
	-- Create two stringbuilders for our messages
	local SBp = sys.CreateStringBuilder() -- Commands for the current player
	local SBo = sys.CreateStringBuilder() -- Commands for the opponent
	SBp:Append("pzk ")
	SBo:Append("pzk ")
	
	-- Get the current player and the pid and oid
	local pid, oid = self:GetTurnIDs()
	local player = self.Players[self.Turn]
	

	cardid = player.Hand[card]
	player.Hand[card] = -1 -- Card is no longer available
	if cardid < cards.PZCARD_PLUS_1 then
		return	-- Bad card?
	end
	
	-- Check for special cases:
	if cardid == cards.PZCARD_DOUBLE then
		-- Obtain the value of the latest card, that'll be our param, and hence our value
		param = JKG.Pazaak.Cards.CardValue(player.Field[player.NextFieldSlot-1], player.FieldParams[player.NextFieldSlot-1])
	elseif cardid == cards.PZCARD_TIEBREAKER then
		self.TieBreaker = true
	elseif cardid == cards.PZCARD_2N4 then	-- Complicated one, we gotta analyze all cards for this one...
		param = 1
		-- We got a 2&4 here, run through the field and find all cards which are subject to a flip
		local pts = 0
		for i=1,9 do
			local fieldcard = player.Field[i]
			local fieldparam = player.FieldParams[i]
			if fieldcard == cards.PZCARD_NORMAL_2 or fieldcard == cards.PZCARD_NORMAL_4 then
				if fieldparam == 1 then
					fieldparam = 0
				else
					fieldparam = 1
				end
			elseif fieldcard == cards.PZCARD_PLUS_2 or fieldcard == cards.PZCARD_PLUS_4 then
				if fieldparam == 1 then
					fieldparam = 0
				else
					fieldparam = 1
				end
			elseif fieldcard == cards.PZCARD_MINUS_2 or fieldcard == cards.PZCARD_MINUS_4 then
				if fieldparam == 1 then
					fieldparam = 0
				else
					fieldparam = 1
				end
			elseif fieldcard == cards.PZCARD_FLIP_2 or fieldcard == cards.PZCARD_FLIP_4 then
				if fieldparam == 0 then
					fieldparam = 2
				elseif fieldparam == 1 then
					fieldparam = 3
				elseif fieldparam == 2 then
					fieldparam = 0
				elseif fieldparam == 3 then
					fieldparam = 1
				end
			end
			if fieldparam ~= player.FieldParams[i] then
				-- We changed it, so apply it, and network it
				player.FieldParams[i] = fieldparam
				SBp:Append(string.format("sc %i %i %i ", i, fieldcard, fieldparam))
				SBo:Append(string.format("sc %i %i %i ", i+9, fieldcard, fieldparam))
			end
			pts = pts + JKG.Pazaak.Cards.CardValue(fieldcard, fieldparam)
		end
		player.Points = pts
	elseif cardid == cards.PZCARD_3N6 then	-- Same story as 2&4
		param = 1
		-- We got a 3&6 here, run through the field and find all cards which are subject to a flip
		local pts = 0
		for i=1,9 do
			local fieldcard = player.Field[i]
			local fieldparam = player.FieldParams[i]
			if fieldcard == cards.PZCARD_NORMAL_3 or fieldcard == cards.PZCARD_NORMAL_6 then
				if fieldparam == 1 then
					fieldparam = 0
				else
					fieldparam = 1
				end
			elseif fieldcard == cards.PZCARD_PLUS_3 or fieldcard == cards.PZCARD_PLUS_6 then
				if fieldparam == 1 then
					fieldparam = 0
				else
					fieldparam = 1
				end
			elseif fieldcard == cards.PZCARD_MINUS_3 or fieldcard == cards.PZCARD_MINUS_6 then
				if fieldparam == 1 then
					fieldparam = 0
				else
					fieldparam = 1
				end
			elseif fieldcard == cards.PZCARD_FLIP_3 or fieldcard == cards.PZCARD_FLIP_6 then
				if fieldparam == 0 then
					fieldparam = 2
				elseif fieldparam == 1 then
					fieldparam = 3
				elseif fieldparam == 2 then
					fieldparam = 0
				elseif fieldparam == 3 then
					fieldparam = 1
				end
			end
			if fieldparam ~= player.FieldParams[i] then
				-- We changed it, so apply it, and network it
				player.FieldParams[i] = fieldparam
				SBp:Append(string.format("sc %i %i %i ", i, fieldcard, fieldparam))
				SBo:Append(string.format("sc %i %i %i ", i+9, fieldcard, fieldparam))
			end
			pts = pts + JKG.Pazaak.Cards.CardValue(fieldcard, fieldparam)
		end
		player.Points = pts
	end
	
	player.Field[player.NextFieldSlot] = cardid
	player.FieldParams[player.NextFieldSlot] = param
	
	cardval = JKG.Pazaak.Cards.CardValue(cardid, param)
	player.Points = player.Points + cardval
	SBp:Append(string.format("sc %i %i %i spt 1 %i shc %i -1 0 ", player.NextFieldSlot, cardid, param, player.Points, card ))
	SBo:Append(string.format("sc %i %i %i spt 2 %i shc %i -1 0 ", player.NextFieldSlot+9, cardid, param, player.Points, card+4 ))
	player.NextFieldSlot = player.NextFieldSlot + 1
	self.Players[pid].Player:SendCommand(SBp:ToString())
	self.Players[oid].Player:SendCommand(SBo:ToString())
end

function PazaakGame:RunPhase3()
	--[[ 
		JKG Pazaak - PHASE 3: User turn
	
		Tasks:
		
		* Verify players
		* Allow the player to use a card, or run the AI
		* Update the fields and points
		* Check if we hit 20, if so, stand and go to phase 4
		* If this card fills out field, stand as well (go to phase 4 too)
		
	]]
	
	-- Verify players (TODO)
	
	
	-- Get our IDs
	local pid, oid = self:GetTurnIDs()
	local player = self.Players[self.Turn]
	
	if player.IsAI then
		-- It's AI, so run it
		local card, param, stand = player.AIRoutine(self)
		if card ~= 0 then
			-- We played a card, clear that card slot and update the field
			self:PlayCard(card, param)

			if player.NextFieldSlot or player.Points == 20 then
				stand = true	-- Whether the AI wants to or not :P
			end
		end
		if stand then
			player.Standing = true
			self.Players[pid].Player:SendCommand("pzk ss 1 1 ")
			self.Players[oid].Player:SendCommand("pzk ss 2 1 ")
		end
		-- Go to the next Phase
		self.Phase = 4
		self:QueuePhase(250)
	else
		if not self.AIGame then
			-- The player gets a timeout to make its choices: 15 seconds, after that, we pretend he clicked on end turn
			timer.Create(self.TimeoutName, 15000, 1, self.P3EndTurn, self)
			player.CanUseCard = true
			self.Players[pid].Player:SendCommand("pzk db 1 sto 15")
			self.Players[oid].Player:SendCommand("pzk sto 15")
		else
			player.CanUseCard = true
			self.Players[pid].Player:SendCommand("pzk db 1")
		end
	end
end

function PazaakGame:P3EndTurn()
	timer.Remove(self.TimeoutName)
	local pid, oid = self:GetTurnIDs()
	local player = self.Players[self.Turn]
	self.Players[pid].CanUseCard = false
	self.Players[pid].Player:SendCommand("pzk sto 0 db 0")
	self.Players[oid].Player:SendCommand("pzk sto 0")
	-- Go to the next Phase
	self.Phase = 4
	self:QueuePhase(250)
end

function PazaakGame:P3Stand()
	timer.Remove(self.TimeoutName)
	local pid, oid = self:GetTurnIDs()
	local player = self.Players[self.Turn]
	
	player.Standing = true
	player.CanUseCard = false
	self.Players[pid].Player:SendCommand("pzk sto 0 db 0 ss 1 1 ")
	self.Players[oid].Player:SendCommand("pzk sto 0 ss 2 1 ")
	-- Go to the next Phase
	self.Phase = 4
	self:QueuePhase(250)
end

function PazaakGame:P3UseCard(card, param)
	local stand = false
	local pid, oid = self:GetTurnIDs()
	local player = self.Players[self.Turn]
	
	if not player.CanUseCard then
		return
	end
	-- We played a card, clear that card slot and update the field
	player.CanUseCard = false
	
	self:PlayCard(card, param)
	
	if player.NextFieldSlot == 10 or player.Points == 20 then
		stand = true	-- Automatically stand
	end
	if stand then
		timer.Remove(self.TimeoutName)
		player.Standing = true
		self.Players[pid].Player:SendCommand("pzk sto 0 db 0 ss 1 1 ")
		self.Players[oid].Player:SendCommand("pzk ss 2 1 ")
		-- Go to the next Phase
		self.Phase = 4
		self:QueuePhase(250)
	else
		self.Players[pid].Player:SendCommand("pzk db 2")
		player.CanUseCard = false
	end
end

function PazaakGame:RunPhase4()
	--[[ 
		JKG Pazaak - PHASE 4: End turn
	
		Tasks:
		
		* Verify players
		* Check if someone won the set, or tied it
		* Display dialogs and start a new set if required
		*Resume at phase 1 if all is fine
		
	]]
		
	local cont = true
	local gotophase5 = false
	-- Create two stringbuilders for our messages
	local SBp = sys.CreateStringBuilder() -- Commands for the current player
	local SBo = sys.CreateStringBuilder() -- Commands for the opponent
	SBp:Append("pzk ")
	SBo:Append("pzk ")

	-- Verify players (TODO)
	
	
	-- Get our IDs
	local pid, oid = self:GetTurnIDs()
	local player = self.Players[pid]
	local opponent = self.Players[oid]
	
	if player.Points > 20 then
		cont = false
		-- Player busted out
		-- Player lost the set
		opponent.Score = opponent.Score + 1
		SBp:Append(string.format("ssc 2 %i ", opponent.Score))
		SBo:Append(string.format("ssc 1 %i ", opponent.Score))
		if opponent.Score == 3 then
			-- The opponent wins the match
			gotophase5 = true
		else
			-- We lost the set, display the dialog
			SBp:Append("sd 2 ")
			SBo:Append("sd 1 ")
			player.DlgAccepted = false
			opponent.DlgAccepted = false
			if self.AIGame then
				self.Players[2].DlgAccepted = true
			else
				SBp:Append("sto 8 ")
				SBo:Append("sto 8 ")
			end
		end
	elseif player.Standing then
		-- Player is standing, check if the opponent is standing too
		if opponent.Standing then
			cont = false
			-- He is,compare the score
			if player.Points > opponent.Points or player.NextFieldSlot == 10 then
				-- Player won the set
				player.Score = player.Score + 1
				SBp:Append(string.format("ssc 1 %i ", player.Score))
				SBo:Append(string.format("ssc 2 %i ", player.Score))
				if player.Score == 3 then
					-- We won the match!
					gotophase5 = true
				else
					-- We won the set, display the dialog
					SBp:Append("sd 1 ")
					SBo:Append("sd 2 ")
					player.DlgAccepted = false
					opponent.DlgAccepted = false
					if self.AIGame then
						self.Players[2].DlgAccepted = true
					else
						SBp:Append("sto 8 ")
						SBo:Append("sto 8 ")
					end
				end
			elseif player.Points < opponent.Points then
				-- Player lost the set
				opponent.Score = opponent.Score + 1
				SBp:Append(string.format("ssc 2 %i ", opponent.Score))
				SBo:Append(string.format("ssc 1 %i ", opponent.Score))
				if opponent.Score == 3 then
					-- The opponent wins the match
					gotophase5 = true
				else
					-- We lost the set, display the dialog
					SBp:Append("sd 2 ")
					SBo:Append("sd 1 ")
					player.DlgAccepted = false
					opponent.DlgAccepted = false
					if self.AIGame then
						self.Players[2].DlgAccepted = true
					else
						SBp:Append("sto 8 ")
						SBo:Append("sto 8 ")
					end
				end
			else
				if self.TieBreaker then
					-- We still won!
					player.Score = player.Score + 1
					SBp:Append(string.format("ssc 1 %i ", player.Score))
					SBo:Append(string.format("ssc 2 %i ", player.Score))
					if player.Score == 3 then
						-- We win the match
						gotophase5 = true
					else
						-- We won the set, display the dialog
						SBp:Append("sd 1 ")
						SBo:Append("sd 2 ")
						player.DlgAccepted = false
						opponent.DlgAccepted = false
						if self.AIGame then
							self.Players[2].DlgAccepted = true
						else
							SBp:Append("sto 8 ")
							SBo:Append("sto 8 ")
						end
					end
				else
					-- Set is tied
					SBp:Append("sd 3 ")
					SBo:Append("sd 3 ")
					player.DlgAccepted = false
					opponent.DlgAccepted = false
					if self.AIGame then
						self.Players[2].DlgAccepted = true
					else
						SBp:Append("sto 8 ")
						SBo:Append("sto 8 ")
					end
				end
			end
		end
	end
	
	if not cont then
		player.Player:SendCommand(SBp:ToString())
		opponent.Player:SendCommand(SBo:ToString())
	end
	self.TieBreaker = false
	if cont then
		-- If we get here, nothing important is going on, so just continue
		self.Phase = 1
		self:QueuePhase(250)
	elseif gotophase5 then
		-- Someone just won the game, so pass on to phase 5
		self.Phase = 5
		self:RunPhase5()
	else
		-- If we get here, we just finished a set, so prepare for the next and and stop (so we can wait for ppl to confirm their dialogs)
		self.NewSet = true
		if self.FirstPlayer == 1 then
			self.FirstPlayer = 2
		else
			self.FirstPlayer = 1
		end
		self.Turn = 0
		if not self.AIGame then
			timer.Create(self.TimeoutName, 8000, 1, self.DialogConfirmed, self)
		end
	end
end

function PazaakGame:RunPhase5()
	--[[ 
		JKG Pazaak - PHASE 5: Game end
	
		Tasks:
		
		* Verify players
		* Display the dialog
		* Wait for feedback and close pazaak and remove the reference
		
		
	]]
	
	-- Create two stringbuilders for our messages
	local SBp = sys.CreateStringBuilder() -- Commands for the current player
	local SBo = sys.CreateStringBuilder() -- Commands for the opponent
	SBp:Append("pzk ")
	SBo:Append("pzk ")
	
	local pid, oid = self:GetTurnIDs()
	local player = self.Players[pid]
	local opponent = self.Players[oid]

	-- Verify players (TODO)

	
	-- We finished the game
	self.Finished = true
	
	local pdlg
	local odlg
	
	-- Display the dialogs
	-- But first, check if we're dealing with a forfeit here, or with a downright win
	if player.Forfeited or opponent.Forfeited then
		-- Forfeited
		if player.Forfeited then
			self.Winner = opponent
			odlg = 6
			pdlg = 7
		else
			self.Winner = player
			odlg = 7
			pdlg = 6
		end
	else
		-- Geniune win
		if player.Score == 3 then
			self.Winner = player
			odlg = 5
			pdlg = 4
		else
			self.Winner = opponent
			odlg = 4
			pdlg = 5
		end
	end
	self.Players[1].DlgAccepted = false
	self.Players[2].DlgAccepted = false
	if not self.AIGame then
		player.Player:SendCommand(string.format("pzk sd %i sto 8", pdlg))
		opponent.Player:SendCommand(string.format("pzk sd %i sto 8", odlg))
		timer.Create(self.TimeoutName, 8000, 1, self.CleanUp, self)
	else 
		player.Player:SendCommand(string.format("pzk sd %i", pdlg))
		opponent.Player:SendCommand(string.format("pzk sd %i", odlg))
		self.Players[2].DlgAccepted = true
	end
	if self.Winner.IsAI then
		self.Winner = nil
	else
		self.Winner = self.Winner.Player
	end
end

function PazaakGame:CleanUp()
	-- We're done folks
	self.Players[1].Player.__sys.PazaakGame = nil
	if not self.Players[2].IsAI then
		self.Players[2].Player.__sys.PazaakGame = nil
	end
	
	-- Erase our timers, just to be sure
	timer.Remove(self.TimeoutName)
	timer.Remove(self.TimerName)
	
	--remove player invincibility: futuza
	self.Players[1].Player.GodMode = false
	self.Players[1].Player.NoKnockback = false
	self.Players[1].Player.NoDebuff = false
	self.Players[2].Player.GodMode = false
	self.Players[2].Player.NoKnockback = false
	self.Players[2].Player.NoDebuff = false
	if not self.Players[2].IsAI then
		print( "Invulnerability disabled for " .. tostring(self.Players[1].Player:GetName()) .. "^7 and " .. tostring(self.Players[2].Player:GetName()) .. "^7." )
	end
	self.Players[1].Player.Busy = false --no longer busy
	self.Players[2].Player.Busy = false
	
	-- Close the pazaak board and we're finished
	self.Players[1].Player:SendCommand("pzk stop")
	self.Players[2].Player:SendCommand("pzk stop")
	
	
	if self.FinishCallback then
		self.FinishCallback(self.Winner)
	end
end

function PazaakGame:DialogConfirmed()
	-- We timed out, force the closure of the dialogs and proceed, depending on the phase we're in
	timer.Remove(self.TimeoutName)
	self.Players[1].Player:SendCommand("pzk cd sto 0")
	self.Players[2].Player:SendCommand("pzk cd sto 0")
	
	if self.Phase == 4 then
		-- Go to the next phase, it's already prepared for us
		self.Phase = 1
		self:QueuePhase(250)
	elseif self.Phase == 5 then
		self:CleanUp()
	end
end

function PazaakGame:AcceptDialog(player)
	-- A player accepted a dialog, to mark him as such, once both players do so, continue
	if self.Players[1].Player == player then
		self.Players[1].DlgAccepted = true
	elseif self.Players[2].Player == player then
		self.Players[2].DlgAccepted = true
	else
		return
	end
	
	if self.Players[1].DlgAccepted and self.Players[2].DlgAccepted then
		-- Both accepted it
		self:DialogConfirmed()
	end
end

function PazaakGame:Forfeit(player)
	-- A player has forfeited the match, lets see who he is and mark his as forfeiting, and go to phase 5, after ensureing the game has been stopped for both sides
	if self.Players[1].Player == player then
		self.Players[1].Forfeited = true
	elseif self.Players[2].Player == player then
		self.Players[2].Forfeited = true
	else
		return
	end
	
	-- If we get here, one of the players was just marked as forfeiting
	timer.Remove(self.TimeoutName)
	timer.Remove(self.TimerName)
	-- Ensure all imput is locked, timers are disabled, and advance to phase 5
	self.Players[1].Player:SendCommand("pzk db 0 sto 0")
	self.Players[2].Player:SendCommand("pzk db 0 sto 0")
	
	self.Phase = 5
	self:QueuePhase(250)	
end

function PazaakGame:PlayerQuit(player)
	-- Player just quit the game in card selection
	if self.Phase ~= -1 then
		return
	end
	local pid, oid
	if self.Players[1].Player == player then
		pid = 1
		oid = 2
	elseif self.Players[2].Player == player then
		pid = 2
		oid = 1
	else
		return
	end
	local player = self.Players[pid]
	local opponent = self.Players[oid]

	self.Winner = opponent
	
	timer.Remove(self.TimeoutName)
	timer.Remove(self.TimerName)
	-- Verify players (TODO)

	
	-- We finished the game
	self.Finished = true
	self.Phase = 5
	
	self.Players[1].DlgAccepted = false
	self.Players[2].DlgAccepted = false
	if not self.AIGame then
		player.Player:SendCommand("pzk sd 7 sto 8")
		opponent.Player:SendCommand("pzk sd 6 sto 8")
		timer.Create(self.TimeoutName, 8000, 1, self.CleanUp, self)
	else 
		player.Player:SendCommand("pzk sd 8")
		self.Players[2].DlgAccepted = true
	end
	if self.Winner.IsAI then
		self.Winner = nil
	else
		self.Winner = self.Winner.Player
	end
end

function PazaakGame:PlayerSetSideDeck(player, argv)
	local i
	local pid, oid
	
	if self.Phase ~= -1 then
		return
	end
	
	if self.Players[1].Player == player then
		pid = 1
		oid = 2
	elseif self.Players[2].Player == player then
		pid = 2
		oid = 1
	else
		return
	end
	local player = self.Players[pid]
	local opponent = self.Players[oid]
	-- Verify this sidedeck, if its bad, consider this player the loser of this match
	-- Why? Because UI is responsible for double checking this, if it failed, it means the player is trying to cheat
	player.SideDeck = {}
	for i=1,10 do
		player.SideDeck[i] = tonumber(argv[i+1] or "-1")
	end
	
	if not self:IsValidSideDeck(player.SideDeck) then
		-- Player is screwed
		self:PlayerQuit(ply)
		return
	elseif not self:ValidateSideDeck(player.SideDeck, player.Cards) then
		-- Again, player is screwed
		self:PlayerQuit(ply)
		return
	end
	-- Alright this sidedeck is all good
	player.Ready = true
	if opponent.Ready then
		-- We're both set, proceed
		self:PrepareMatch()
		return
	end
end

function PazaakGame:FinishSideDeck(sidedeck, cards) 
	-- Fill up all empty slots on the sidedeck
	local remainingcards = {}
	local availcards = {}
	local emptyslots = 0
	local cardcount = 0
	local k,v,i
	local cardvals = JKG.Pazaak.Cards
	-- Copy table
	for k,v in pairs(cards) do
		remainingcards[k] = v
	end
	for i=1,10 do
		if sidedeck[i] < cardvals.PZCARD_PLUS_1 then
			emptyslots = emptyslots + 1
		else
			remainingcards[sidedeck[i] - cardvals.PZCARD_PLUS_1 + 1] = remainingcards[sidedeck[i] - cardvals.PZCARD_PLUS_1 + 1] -1
		end
	end
	for k,v in pairs(remainingcards) do
		if v < 0 then
			-- SHOULD NEVER HAPPEN!
			return false
		end
		cardcount = cardcount + v
	end
	if emptyslots > cardcount then
		-- This should..NEVER...happen
		return false
	end
	-- Create an array with available cards
	for k,v in pairs(remainingcards) do
		for i=1,remainingcards[k] do
			table.insert(availcards, cardvals.PZCARD_PLUS_1 + (k-1))
		end
	end
	-- Alright, we finally got our array set, time to start picking new cards
	local rnd
	for i=1,10 do
		if sidedeck[i] < cardvals.PZCARD_PLUS_1 then
			rnd = math.random(1, #availcards)
			sidedeck[i] = availcards[rnd]
			table.remove(availcards, rnd)
		end
	end
	return true
end

function PazaakGame:PlayerSetPartialSideDeck(player, argv)
	local i
	local pid, oid
	
	if self.Phase ~= -1 then
		return
	end
	
	if self.Players[1].Player == player then
		pid = 1
		oid = 2
	elseif self.Players[2].Player == player then
		pid = 2
		oid = 1
	else
		return
	end
	local player = self.Players[pid]
	local opponent = self.Players[oid]
	-- Verify this sidedeck, if its bad, consider this player the loser of this match
	-- Why? Because UI is responsible for double checking this, if it failed, it means the player is trying to cheat
	player.SideDeck = {}
	local incomplete = false
	local cards = JKG.Pazaak.Cards
	for i=1,10 do
		player.SideDeck[i] = tonumber(argv[i+1] or "-1")
		if player.SideDeck[i] < cards.PZCARD_PLUS_1 then
			player.SideDeck[i] = -1
			incomplete = true
		end
	end
	
	if incomplete then
		self:FinishSideDeck(player.SideDeck, player.Cards)
	end
	
	if not self:IsValidSideDeck(player.SideDeck) then
		-- Player is screwed
		self:PlayerQuit(ply)
		return
	elseif not self:ValidateSideDeck(player.SideDeck, player.Cards) then
		-- Again, player is screwed
		self:PlayerQuit(ply)
		return
	end
	-- Alright this sidedeck is all good
	player.Ready = true
	if opponent.Ready then
		-- We're both set, proceed
		self:PrepareMatch()
		return
	end
end

local function Pzk_Command(ply, argc, argv)
	local cmd = argv[1]
	local pzk = ply.__sys.PazaakGame
	if not pzk then
		return
	end
	if cmd == "forfeit" then
		pzk:Forfeit(ply)	-- Can always be used
		return
	elseif cmd == "acceptdlg" then
		pzk:AcceptDialog(ply)		-- Player confirmed a dialog trigged using 'sd'
		return
	elseif cmd == "quit" then		-- Player clicked Exit in the card selection screen
		pzk:PlayerQuit(ply)
		return
	elseif cmd == "setsd" then		-- Player chose his sidedeck
		pzk:PlayerSetSideDeck(ply, argv)
		return
	elseif cmd == "setsdt" then		-- Player's time ran out and sent the (potentially incomplete) side deck
		pzk:PlayerSetPartialSideDeck(ply, argv)
		return
	end

	if not pzk.Players[pzk.Turn].Player == ply then		-- Must be us who's currently playing
		return
	elseif not pzk.Phase == 3 then		-- The rest can only be used in phase 3
		return
	end	

	if cmd == "endturn" then
		pzk:P3EndTurn()
		return
	elseif cmd == "stand" then
		pzk:P3Stand()
		return
	elseif cmd == "usecard" then
		local card = tonumber(argv[2] or "0")
		local param = tonumber(argv[3] or "0")
		pzk:P3UseCard(card, param)
		return
	end
end

cmds.Add("~pzk", Pzk_Command)
