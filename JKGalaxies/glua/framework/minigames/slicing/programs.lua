--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Code
	Slicing programs
	
	Written by BobaFett
--------------------------------------------------]]


JKG.Slicing.Programs = {}

-- Program types:

-- 0: Standalone
-- 1: Run on node
-- 2: Run on inactive node
-- 3: Run on line

JKG.Slicing.Programs.PROBE = {
	Name = "Probe",
	Desc = "@JKG_SLICE_PROBE",
	Type = 0,
	Func = function (state)
		local function finalize(state, results)
			state:__NetEndDialog()
			state:__NetSummary(results)
			state:__LockField(false)
			state:__NetTransmit()	-- Called by a timer, so transmit manually
		end
		local summary = {}
		local row, col
		local value
		local alarms
		local nodeid
		
		if (state.ProgramData.Probed) then
			state:__ShowDialog(2, "You have already probed the security grid", nil, nil)
			return
		end
		
		if (state:__RaiseWarningLevel(1)) then
			return
		end
		
		state.ProgramData.Probed = true
		
		state:__ShowDialog(0, "Probing security grid...", nil, nil)
		state:__LockField(true)

		-- Process columns first
		for col=1, state.Width do
			value = 0
			alarms = 0
			for row=1, state.Height do
				local nodeid = state.Grid[row][col]
				if nodeid == 0 then	-- Alarm node
					alarms = alarms + 1
				elseif nodeid == 1 or nodeid == 2 then -- Relay or reset
					value = value + 1
				else	-- Access (3 - 7)
					value = value + (nodeid - 1)
				end
			end
			table.insert(summary, {Value = value, Alarms = alarms})
		end

		-- Process rows next
		for row=1, #state.Grid do
			value = 0
			alarms = 0
			for col=1, state.Width do
				local nodeid = state.Grid[row][col]
				if nodeid == 0 then	-- Alarm node
					alarms = alarms + 1
				elseif nodeid == 1 or nodeid == 2 then -- Relay or reset
					value = value + 1
				else	-- Access
					value = value + (nodeid - 1)
				end
			end
			table.insert(summary, {Value = value, Alarms = alarms})
		end

		-- Simulate execution time
		timer.Simple(1000, finalize, state, summary)
	end,
}

JKG.Slicing.Programs.SCANNODE = {
	Name = "Scan Node",
	Desc = "@JKG_SLICE_SCANNODE",
	Type = 2,
	Func = function (state, nodeID)
		local col = (nodeID % 8) + 1
		local row = math.floor(nodeID/8) + 1
		
		if (state:__RaiseWarningLevel(1)) then
			return
		end
		
		if (state.Grid[row][col] == 0) then
			state:__NetBlink(row, col, false)
		else
			state:__NetBlink(row, col, true)
		end
	end,
}

JKG.Slicing.Programs.SCANLINE = {
	Name = "Scan Line",
	Desc = "@JKG_SLICE_SCANLINE",
	Type = 3,
	Func = function (state, lineID)
		local row
		local col
		local penalty = state.ProgramData.LinesScanned or 0
		state.ProgramData.LinesScanned = penalty + 1
		
		if (state:__RaiseWarningLevel(2 + penalty)) then
			return
		end
		
		if (lineID < 8) then 	-- Column ID
			col = lineID + 1
			for row=1,state.Height do
				if (state.Grid[row][col] == 0) then
					state:__NetBlink(row, col, false)
				else
					state:__NetBlink(row, col, true)
				end
			end
		else	-- Row ID
			row = lineID - 7
			for col=1,state.Width do
				if (state.Grid[row][col] == 0) then
					state:__NetBlink(row, col, false)
				else
					state:__NetBlink(row, col, true)
				end
			end
		end
	end,
}