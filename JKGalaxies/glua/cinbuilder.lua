--[[ ------------------------------------------------
	Jedi Knight Galaxies Lua Code
	Cinematic Builder
	
	Written by BobaFett
--------------------------------------------------]]

-- Command list for CinBuild:

local helptext_t1 = {
"^5Valid commands:",
"^5help ^7- Shows command info",
"^5init ^7- Activates the cinematic builder (if you're authorized)",
}
local helptext_t2 = {
"^5Valid commands:",
"^5help ^7- Shows command info",
"^5init ^7- Activates the cinematic builder (if you're authorized)",
"^5end ^7- Deactivates the cinematic builder",
"^5type <type> ^7- Sets trajectory type (0 = static (default), 1 = linear, 2 = spline)",
"^5addpoint [index] [position] ^7- Adds a new point (or changes one if index is specified) at your location or the location specified",
"^5setorigin <index> [position] ^7- Moves the specified point to your position or the position specified",
"^5setangles <index> [angles] ^7- Changes the point's angles to your view angles or the angles specified",
"^5settime <index> <time> ^7- Set move time on specified point",
"^5remove <index> ^7- Removes specified point",
}
local helptext_t2b = {
"^5insert <index> [position] ^7- Inserts a new point before the specified point at your position or the specified one",
"^5clear ^7- Removes all points",
"^5setalgos <x> <y> <z> <pitch> <yaw> <roll> ^7- Set spline interpolation algorithms: 0 = Cubic spline, 1 = B-Spline, 2 = Catmull-Rom, 3 = Linear",
"^5interval <ms interval> ^7- Set spline curve interval (time distance between points in the visualisation), default is 250 ms",
"^5settarget [origin] ^7- Places the view target at your position or the position specified",
"^5cleartarget ^7- Removes the view target",
"^5preview ['clean'] ^7- Play back the cinematic, if you use preview clean, the visualisation will be disabled during the preview",
}
local helptext_t2c = {
"^5fixangles ^7 - Corrects angle twists in the path",
"^5setdeftime <time> ^7- Sets default move time for new points (in ms)",
"^5setdelays <start delay> <end delay) ^7- Sets the start and end delays for preview (in ms)",
"^5save <name> ^7- Saves the cinematic",
"^5load <name> ^7- Loads a saved cinematic",
"^5export <name> ^7- Exports the cinematic as a lua script",
}

local helptext_m1 = table.concat(helptext_t1, "\n")
local helptext_m2 = table.concat(helptext_t2, "\n")
local helptext_m2b = table.concat(helptext_t2b, "\n")
local helptext_m2c = table.concat(helptext_t2c, "\n")

local function SaveCinematic(ply, name)
	local data = {}
	data.TrType = ply.CinBuild.TrType
	if ply.CinBuild.AimTarget then
		data.AimTarget = string.format("%.3f %.3f %.3f", ply.CinBuild.AimTarget.x, ply.CinBuild.AimTarget.y, ply.CinBuild.AimTarget.z)
	end
	data.SplineAlgos = ply.CinBuild.SplineAlgos
	data.PointCount = ply.CinBuild.PointCount
	data.DefaultTime = ply.CinBuild.DefaultTime
	data.Delays = ply.CinBuild.Delays
	data.Points = {}
	for k,v in pairs(ply.CinBuild.Points) do
		data.Points[k] = {}
		data.Points[k].origin = string.format("%.3f %.3f %.3f", v.origin.x, v.origin.y, v.origin.z)
		data.Points[k].angles = string.format("%.3f %.3f %.3f", v.angles.x, v.angles.y, v.angles.z)
		data.Points[k].time = v.time
	end
	-- Time to save :D
	local txt = Json.Encode(data)
	file.Write("cinbuild/" .. name .. ".dat", txt)
end

local function LoadCinematic(ply, name)
	local data
	local txt
	txt = file.Read("cinbuild/" .. name .. ".dat")
	if (txt == nil) then
		return false
	end
	data = Json.Decode(txt)
	
	ply.CinBuild.TrType = data.TrType
	if data.AimTarget then
		ply.CinBuild.AimTarget = Vector(data.AimTarget)
	end
	ply.CinBuild.SplineAlgos = data.SplineAlgos
	ply.CinBuild.PointCount = data.PointCount
	ply.CinBuild.DefaultTime = data.DefaultTime
	ply.CinBuild.Delays = data.Delays
	ply.CinBuild.Points = {}
	for k,v in pairs(data.Points) do
		ply.CinBuild.Points[k] = {}
		ply.CinBuild.Points[k].origin = Vector(v.origin)
		ply.CinBuild.Points[k].angles = Vector(v.angles)
		ply.CinBuild.Points[k].time = v.time
	end
	return true
end

local function CinBuildPlayCin(ply, clean)

	cin = sys.CreateCinematic()
	cin:SetAllowAbort(true)
	cin:SetCallbacks(function(cinobj, ply, clean)
						ply:StopCinematic()
						if clean then 
							ply:SendCommand("cb vis 1")
						end
					end, nil, nil, ply, clean)
	if ply.CinBuild.TrType == 0 then
		-- Static
		cin:AddFadeOut(0, 500)
		cin:AddFadeIn(500, 0)
		local offset = 500 + ply.CinBuild.Delays[1]
		for i = 1, ply.CinBuild.PointCount do
			if i ~= 1 then
				offset = offset + ply.CinBuild.Points[i].time
			end
			if ply.CinBuild.AimTarget then
				local vec = ply.CinBuild.AimTarget - ply.CinBuild.Points[i].origin
				vec:ToAngles()
				cin:AddStaticCam(offset, ply.CinBuild.Points[i].origin, vec)		
			else 
				cin:AddStaticCam(offset, ply.CinBuild.Points[i].origin, ply.CinBuild.Points[i].angles)		
			end
		end
		cin:AddFadeOut(offset+ply.CinBuild.Delays[2], 1000)
		cin:StartSkip(offset+ply.CinBuild.Delays[2], offset+ply.CinBuild.Delays[2])
		cin:FinishSkip()
		cin:AddRestoreCam(offset+1000+ply.CinBuild.Delays[2])
		cin:AddFinish(offset+1000+ply.CinBuild.Delays[2])
	elseif ply.CinBuild.TrType == 1 then
		-- Linear
		cin:AddFadeOut(0, 500)
		cin:AddFadeIn(500, 0)
		local offset = 500 + ply.CinBuild.Delays[1]
		if ply.CinBuild.AimTarget then
			cin:StartLinearCam(500, ply.CinBuild.AimTarget)
			for i = 1, ply.CinBuild.PointCount do
				if i ~= 1 then
					offset = offset + ply.CinBuild.Points[i].time
				end
				cin:AddLinearCamPoint(offset, ply.CinBuild.Points[i].origin)		
			end
			cin:FinishLinearCam()
		else
			cin:StartLinearCam(500)
			for i = 1, ply.CinBuild.PointCount do
				if i ~= 1 then
					offset = offset + ply.CinBuild.Points[i].time
				end
				cin:AddLinearCamPoint(offset, ply.CinBuild.Points[i].origin, ply.CinBuild.Points[i].angles)		
			end
			cin:FinishLinearCam()
		end
		cin:AddFadeOut(offset+ply.CinBuild.Delays[2], 1000)
		cin:StartSkip(offset+ply.CinBuild.Delays[2], offset+ply.CinBuild.Delays[2])
		cin:FinishSkip()
		cin:AddRestoreCam(offset+1000+ply.CinBuild.Delays[2])
		cin:AddFinish(offset+1000+ply.CinBuild.Delays[2])
	elseif ply.CinBuild.TrType == 2 then
		-- Spline
		cin:AddFadeOut(0, 500)
		cin:AddFadeIn(500, 0)
		local offset = 500 + ply.CinBuild.Delays[1]
		if ply.CinBuild.AimTarget then
			cin:StartSplineCam(500, ply.CinBuild.AimTarget)
			local algs = ply.CinBuild.SplineAlgos
			cin:SetSplineCamAlgos(algs[1], algs[2], algs[3], algs[4], algs[5], algs[6])
			for i = 1, ply.CinBuild.PointCount do
				if i ~= 1 then
					offset = offset + ply.CinBuild.Points[i].time
				end
				cin:AddSplineCamPoint(offset, ply.CinBuild.Points[i].origin)		
			end
			cin:FinishSplineCam()
		else 
			cin:StartSplineCam(500)
			local algs = ply.CinBuild.SplineAlgos
			cin:SetSplineCamAlgos(algs[1], algs[2], algs[3], algs[4], algs[5], algs[6])
			for i = 1, ply.CinBuild.PointCount do
				if i ~= 1 then
					offset = offset + ply.CinBuild.Points[i].time
				end
				cin:AddSplineCamPoint(offset, ply.CinBuild.Points[i].origin, ply.CinBuild.Points[i].angles)		
			end
			cin:FinishSplineCam()
		
		end
		cin:AddFadeOut(offset+ply.CinBuild.Delays[2], 1000)
		cin:StartSkip(offset+ply.CinBuild.Delays[2], offset+ply.CinBuild.Delays[2])
		cin:FinishSkip()
		cin:AddRestoreCam(offset+1000+ply.CinBuild.Delays[2])
		cin:AddFinish(offset+1000+ply.CinBuild.Delays[2])
	end
	ply:StartCinematic()
	if clean then 
		ply:SendCommand("cb vis 0")
	end
	cin:PlayCinematic(ply)
end

local function CinBuildExport(ply, name)
	txt = sys.CreateStringBuilder()
	txt:Append("----------------------------------------------------------\n");
	txt:Append("-- Cinematic snipplet exported by the cinematic builder --\n")
	txt:Append("----------------------------------------------------------\n\n");
	txt:Append("local function CinTime(t, time)\n")
	txt:Append("	t.time = t.time + time\n")
	txt:Append("	return t.time\n")
	txt:Append("end\n\n")
	txt:Append("t = {time = --[[ start time here ]] }\n")
	if ply.CinBuild.TrType == 0 then 
		local offset = 0
		-- Static
		for i = 1, ply.CinBuild.PointCount do
			local time
			if i ~= 1 then
				time = ply.CinBuild.Points[i].time
				offset = offset + ply.CinBuild.Points[i].time
			else
				time = 0
			end
			if ply.CinBuild.AimTarget then
				local vec = ply.CinBuild.AimTarget - ply.CinBuild.Points[i].origin
				vec:ToAngles()
				txt:Append(string.format("cin:AddStaticCam(CinTime(t, %i), Vector(%.3f, %.3f, %.3f), Vector(%.3f, %.3f, %.3f))\n", time,  ply.CinBuild.Points[i].origin.x, ply.CinBuild.Points[i].origin.y,  ply.CinBuild.Points[i].origin.z, vec.x, vec.y, vec.z))
			else 
				txt:Append(string.format("cin:AddStaticCam(CinTime(t, %i), Vector(%.3f, %.3f, %.3f), Vector(%.3f, %.3f, %.3f))\n", time,  ply.CinBuild.Points[i].origin.x, ply.CinBuild.Points[i].origin.y,  ply.CinBuild.Points[i].origin.z, ply.CinBuild.Points[i].angles.x, ply.CinBuild.Points[i].angles.y, ply.CinBuild.Points[i].angles.z))		
			end
		end
		txt:Append(string.format("-- End offset: %i\n", offset))
	elseif ply.CinBuild.TrType == 1 then
		-- Linear
		local offset = 0
		local time
		if ply.CinBuild.AimTarget then
			txt:Append(string.format("cin:StartLinearCam(t.time, Vector(%.3f, %.3f, %.3f))\n", ply.CinBuild.AimTarget.x, ply.CinBuild.AimTarget.y, ply.CinBuild.AimTarget.z))
			for i = 1, ply.CinBuild.PointCount do
				if i ~= 1 then
					time = ply.CinBuild.Points[i].time
					offset = offset + ply.CinBuild.Points[i].time
				else 
					time = 0
				end
				txt:Append(string.format("cin:AddLinearCamPoint(CinTime(t, %i), Vector(%.3f, %.3f, %.3f))\n", time, ply.CinBuild.Points[i].origin.x, ply.CinBuild.Points[i].origin.y, ply.CinBuild.Points[i].origin.z))	
			end
			txt:Append("cin:FinishLinearCam()\n")
			txt:Append(string.format("-- End offset: %i\n", offset))
		else
			txt:Append("cin:StartLinearCam(t.time)\n")
			for i = 1, ply.CinBuild.PointCount do
				if i ~= 1 then
					time = ply.CinBuild.Points[i].time
					offset = offset + ply.CinBuild.Points[i].time
				else 
					time = 0
				end
				txt:Append(string.format("cin:AddLinearCamPoint(CinTime(t, %i), Vector(%.3f, %.3f, %.3f), Vector(%.3f, %.3f, %.3f))\n", time, ply.CinBuild.Points[i].origin.x, ply.CinBuild.Points[i].origin.y, ply.CinBuild.Points[i].origin.z, ply.CinBuild.Points[i].angles.x, ply.CinBuild.Points[i].angles.y, ply.CinBuild.Points[i].angles.z))	
			end
			txt:Append("cin:FinishLinearCam()\n")
			txt:Append(string.format("-- End offset: %i\n", offset))
		end
	elseif ply.CinBuild.TrType == 2 then
		-- Spline
		local offset = 0
		local time
		if ply.CinBuild.AimTarget then
			txt:Append(string.format("cin:StartSplineCam(t.time, Vector(%.3f, %.3f, %.3f))\n", ply.CinBuild.AimTarget.x, ply.CinBuild.AimTarget.y, ply.CinBuild.AimTarget.z))
			local algs = ply.CinBuild.SplineAlgos
			txt:Append(string.format("cin:SetSplineCamAlgos(%i, %i, %i, %i, %i, %i)\n", algs[1], algs[2], algs[3], algs[4], algs[5], algs[6]))
			for i = 1, ply.CinBuild.PointCount do
				if i ~= 1 then
					time = ply.CinBuild.Points[i].time
					offset = offset + ply.CinBuild.Points[i].time
				else 
					time = 0
				end
				txt:Append(string.format("cin:AddSplineCamPoint(CinTime(t, %i), Vector(%.3f, %.3f, %.3f))\n", time, ply.CinBuild.Points[i].origin.x, ply.CinBuild.Points[i].origin.y, ply.CinBuild.Points[i].origin.z))	
			end
			txt:Append("cin:FinishSplineCam()\n")
			txt:Append(string.format("-- End offset: %i\n", offset))
		else 
			txt:Append("cin:StartSplineCam(t.time)\n")
			local algs = ply.CinBuild.SplineAlgos
			txt:Append(string.format("cin:SetSplineCamAlgos(%i, %i, %i, %i, %i, %i)\n", algs[1], algs[2], algs[3], algs[4], algs[5], algs[6]))
			for i = 1, ply.CinBuild.PointCount do
				if i ~= 1 then
					time = ply.CinBuild.Points[i].time
					offset = offset + ply.CinBuild.Points[i].time
				else 
					time = 0
				end
				txt:Append(string.format("cin:AddSplineCamPoint(CinTime(t, %i), Vector(%.3f, %.3f, %.3f), Vector(%.3f, %.3f, %.3f))\n", time, ply.CinBuild.Points[i].origin.x, ply.CinBuild.Points[i].origin.y, ply.CinBuild.Points[i].origin.z, ply.CinBuild.Points[i].angles.x, ply.CinBuild.Points[i].angles.y, ply.CinBuild.Points[i].angles.z))	
			end
			txt:Append("cin:FinishSplineCam()\n")
			txt:Append(string.format("-- End offset: %i\n", offset))
		end
	end
	file.Write("cinbuild/exports/" .. name .. ".lua", txt:ToString())
	ply:SendPrint("Cinematic exported successfully")
end

local function CinBuild(ply, argc, argv)
	if ply.isLoggedIn ~=true then
		ply:SendPrint("^1You are not logged in.")
		return
	end
	
	local temp
	local rank = GetRank(ply)
	if rank["can-cinbuild"] ~= true then
		SystemReply(ply, "^1You do not have permission to perform this action.")
		return
	end
	if (argc<2) then
		ply:SendPrint("Jedi Knight Galaxies Cinematic Builder\nUse /cinbuild help for syntax info");
		return
	end
	--- TEMP
	ply.CanUseCinBuild = true
	local cmd = argv[1]
	if (cmd == 'help') then
		if (ply.CinBuildActive) then
			ply:SendPrint(helptext_m2)
			ply:SendPrint(helptext_m2b)
			ply:SendPrint(helptext_m2c)
		else
			ply:SendPrint(helptext_m1)
		end
		return
	elseif (cmd == 'init') then
		if ply.CanUseCinBuild then
			ply.CinBuildActive = true
			ply.CinBuild = {}
			ply.CinBuild.TrType = 0 -- 0 = Static, 1 = Linear, 2 = Spline
			ply.CinBuild.AimTarget = nil
			ply.CinBuild.SplineAlgos = {0, 0, 0, 0, 0, 0} -- Cubic spline for all by default
			ply.CinBuild.DefaultTime = 1000
			ply.CinBuild.Delays = {0, 0}
			ply.CinBuild.PointCount = 0
			ply.CinBuild.Points = {}
			ply:SendCommand("cb on") -- Turn on (and reset to default values) the cinematic builder client-side
			ply:SendPrint("Cinematic builder activated, please use /cinbuild type to set a trajectory type")
		else
			ply:SendPrint("^1You are not authorized to use the cinematic builder")
		end
		return
	end
	-- Dont allow commands other than help and init before activating
	if (not ply.CinBuildActive) then
		ply:SendPrint("Bad command, please use /cinbuild help for a command list")
		return
	end

	
	-- end - Deactivates the cinematic builder
	if (cmd == 'end') then
		ply.CinBuildActive = nil
		ply.CinBuild = nil
		ply:SendCommand("cb off") -- Turn off
		ply:SendPrint("Cinematic builder deactivated")
		return
	end
	
	-- Time for the real deal :D
	-- type <type> - Sets trajectory type (0 = static (default), 1 = linear, 2 = spline)
	if (cmd == 'type' ) then
		if (not argv[2]) then
			ply:SendPrint("^1Missing argument: type")
			return
		end
		temp = tonumber(argv[2])
		if (temp < 0 or temp > 2) then
			temp = 0
		end
		ply.CinBuild.TrType = temp
		ply:SendCommand(string.format("cb type %i", temp))
		-- Incredibly awesome hack here :P
		ply:SendPrint("Trajectory type set to " .. ({[0] = "^5Static", [1] = "^5Linear interpolation", [2] = "^5Spline interpolation"})[temp] )
		return
	end
	
	-- addpoint [index] [position] - Adds a new point (or changes one if index is specified) at your location or the location specified
	if (cmd == 'addpoint' ) then
		-- Check which args we got
		if (argc == 2) then
			-- Place at current pos
			local point
			ply.CinBuild.PointCount = ply.CinBuild.PointCount + 1
			ply.CinBuild.Points[ply.CinBuild.PointCount] = {}
			point = ply.CinBuild.Points[ply.CinBuild.PointCount]
			point.origin = ply.Pos
			point.angles = ply.Angles
			if (point.angles.y) < 0 then
				point.angles.y = point.angles.y + 360	-- Normalize all angles to 0 to 360 range, rather than JA's -180 to 180
			end
			point.time = ply.CinBuild.DefaultTime
			ply:SendPrint(string.format("New point created (Point %i)", ply.CinBuild.PointCount))
			ply:SendCommand(string.format("cb ap %.3f %.3f %.3f %.3f %.3f %.3f %i", point.origin.x, point.origin.y, point.origin.z, point.angles.x, point.angles.y, point.angles.z, point.time))
		    return
		elseif (argc == 3) then
			-- Change pos of specific point
			local pt = tonumber(argv[2])
			if (pt > ply.CinBuild.PointCount or pt < 1) then
				ply:SendPrint("^1Invalid point specified")
			else
				local point = ply.CinBuild.Points[pt]
				point.origin =  ply.Pos
				point.angles = ply.Angles
				if (point.angles.y) < 0 then
					point.angles.y = point.angles.y + 360	-- Normalize all angles to 0 to 360 range, rather than JA's -180 to 180
				end
				ply:SendPrint(string.format("Point %i edited", pt))
				ply:SendCommand(string.format("cb ep %i %.3f %.3f %.3f %.3f %.3f %.3f %i", pt, point.origin.x, point.origin.y, point.origin.z, point.angles.x, point.angles.y, point.angles.z, point.time))
			end
			return
		elseif (argc == 5) then
			-- Fixed pos
			local point
			ply.CinBuild.PointCount = ply.CinBuild.PointCount + 1
			ply.CinBuild.Points[ply.CinBuild.PointCount] = {}
			point = ply.CinBuild.Points[ply.CinBuild.PointCount]
			point.origin = Vector(tonumber(argv[2]), tonumber(argv[3]), tonumber(argv[4]))
			point.angles = ply.Angles
			if (point.angles.y) < 0 then
				point.angles.y = point.angles.y + 360	-- Normalize all angles to 0 to 360 range, rather than JA's -180 to 180
			end
			point.time = ply.CinBuild.DefaultTime
			ply:SendPrint(string.format("New point created (Point %i)", ply.CinBuild.PointCount))
			ply:SendCommand(string.format("cb ap %.3f %.3f %.3f %.3f %.3f %.3f %i", point.origin.x, point.origin.y, point.origin.z, point.angles.x, point.angles.y, point.angles.z, point.time))
		    return
		elseif (argc == 6) then
			-- Fixed pos on specific point
			local pt = tonumber(argv[2])
			if (pt > ply.CinBuild.PointCount or pt < 1) then
				ply:SendPrint("^1Invalid point specified")
			else
				local point = ply.CinBuild.Points[pt]
				point.origin = Vector(tonumber(argv[3]), tonumber(argv[4]), tonumber(argv[5]))
				point.angles = ply.Angles
				if (point.angles.y) < 0 then
					point.angles.y = point.angles.y + 360	-- Normalize all angles to 0 to 360 range, rather than JA's -180 to 180
				end
				ply:SendPrint(string.format("Point %i edited", pt))
				ply:SendCommand(string.format("cb ep %i %.3f %.3f %.3f %.3f %.3f %.3f %i", pt, point.origin.x, point.origin.y, point.origin.z, point.angles.x, point.angles.y, point.angles.z, point.time))
			end
			return
		else
			-- Bad args
			ply:SendPrint("^1Invalid arguments provided")
		end
		return
	end
	
	-- setorigin <index> [position] - Moves the specified point to your position or the position specified
	if (cmd == 'setorigin' ) then
		-- Check which args we got
		if (argc == 3) then
			-- Place at current pos
			local pt = tonumber(argv[2])
			if (pt > ply.CinBuild.PointCount or pt < 1) then
				ply:SendPrint("^1Invalid point specified")
			else
				local point = ply.CinBuild.Points[pt]
				point.origin = ply.Pos
				ply:SendPrint(string.format("Point %i edited (origin modified)", pt))
				ply:SendCommand(string.format("cb ep %i %.3f %.3f %.3f %.3f %.3f %.3f %i", pt, point.origin.x, point.origin.y, point.origin.z, point.angles.x, point.angles.y, point.angles.z, point.time))
			end
			return
		elseif (argc == 6) then
			-- Fixed pos
			local pt = tonumber(argv[2])
			if (pt > ply.CinBuild.PointCount or pt < 1) then
				ply:SendPrint("^1Invalid point specified")
			else
				local point = ply.CinBuild.Points[pt]
				point.origin = Vector(tonumber(argv[3]), tonumber(argv[4]), tonumber(argv[5]))
				ply:SendPrint(string.format("Point %i edited (origin modified)", pt))
				ply:SendCommand(string.format("cb ep %i %.3f %.3f %.3f %.3f %.3f %.3f %i", pt, point.origin.x, point.origin.y, point.origin.z, point.angles.x, point.angles.y, point.angles.z, point.time))
			end
			return
		else
			-- Bad args
			ply:SendPrint("^1Invalid arguments provided")
		end
		return
	end
	
	
	-- setangles <index> [angles] - Changes the point's angles to your view angles or the angles specified
	if (cmd == 'setangles' ) then
		-- Check which args we got
		if (argc == 3) then
			-- Place at current pos
			local pt = tonumber(argv[2])
			if (pt > ply.CinBuild.PointCount or pt < 1) then
				ply:SendPrint("^1Invalid point specified")
			else
				local point = ply.CinBuild.Points[pt]
				point.angles = ply.Angles
				if (point.angles.y) < 0 then
					point.angles.y = point.angles.y + 360	-- Normalize all angles to 0 to 360 range, rather than JA's -180 to 180
				end
				ply:SendPrint(string.format("Point %i edited (angles modified)", pt))
				ply:SendCommand(string.format("cb ep %i %.3f %.3f %.3f %.3f %.3f %.3f %i", pt, point.origin.x, point.origin.y, point.origin.z, point.angles.x, point.angles.y, point.angles.z, point.time))
			end
			return
		elseif (argc == 6) then
			-- Fixed pos
			local pt = tonumber(argv[2])
			if (pt > ply.CinBuild.PointCount or pt < 1) then
				ply:SendPrint("^1Invalid point specified")
			else
				local point = ply.CinBuild.Points[pt]
				point.angles = Vector(tonumber(argv[3]), tonumber(argv[4]), tonumber(argv[5]))
				-- Dont normalize, if we want the cam to spin around, make it possible
				ply:SendPrint(string.format("Point %i edited (angles modified)", pt))
				ply:SendCommand(string.format("cb ep %i %.3f %.3f %.3f %.3f %.3f %.3f %i", pt, point.origin.x, point.origin.y, point.origin.z, point.angles.x, point.angles.y, point.angles.z, point.time))
			end
			return
		else
			-- Bad args
			ply:SendPrint("^1Invalid arguments provided")
		end
		return
	end
	
	--settime <index> <time> - Set move time on specified point
	if (cmd == 'settime' ) then
		-- Check which args we got
		if (argc >= 4) then
			-- Place at current pos
			local pt = tonumber(argv[2])
			if (pt > ply.CinBuild.PointCount or pt < 1) then
				ply:SendPrint("^1Invalid point specified")
			else
				local point = ply.CinBuild.Points[pt]
				local time = tonumber(argv[3])
				if ( time < 0 ) then
					ply:SendPrint("^1Cannot set a negative move time!")
					return
				end
				point.time = time
				ply:SendPrint(string.format("Point %i edited (move time modified)", pt))
				ply:SendCommand(string.format("cb ep %i %.3f %.3f %.3f %.3f %.3f %.3f %i", pt, point.origin.x, point.origin.y, point.origin.z, point.angles.x, point.angles.y, point.angles.z, point.time))
			end
			return
		else
			-- Bad args
			ply:SendPrint("^1Invalid arguments provided")
		end
		return
	end
	
	-- mapvelocity <velocity> - Sets the point times in such a way that the camera moves with the specified speed (approx)
	if (cmd == 'mapvelocity') then
		if (argc >= 3) then
			-- Place at current pos
			local vel = tonumber(argv[2])
			local i
			
			for i=2, ply.CinBuild.PointCount do
				local p1 = ply.CinBuild.Points[i-1]
				local p2 = ply.CinBuild.Points[i]
				local dist = p2.origin - p1.origin
				dist = dist:Length()
				
				p2.time = (dist / vel) * 1000
				ply:SendCommand(string.format("cb ep %i %.3f %.3f %.3f %.3f %.3f %.3f %i", i, p2.origin.x, p2.origin.y, p2.origin.z, p2.angles.x, p2.angles.y, p2.angles.z, p2.time))
			end
			ply:SendPrint(string.format("All points have been velocity mapped (%i units/second)", vel))
			return
		else
			-- Bad args
			ply:SendPrint("^1Invalid arguments provided")
		end
	
	end
	
	-- remove <index> - Removes specified point
	if (cmd == 'remove' ) then
		-- Check which args we got
		if (argc >= 3) then
			-- Remove point
			local pt = tonumber(argv[2])
			if (pt < 1 or pt > ply.CinBuild.PointCount) then
				ply:SendPrint("Invalid point specified")
				return
			end
			if pt < ply.CinBuild.PointCount then
				for i = pt+1, ply.CinBuild.PointCount do
					ply.CinBuild.Points[i-1] = ply.CinBuild.Points[i]
				end
			end
			ply.CinBuild.Points[ply.CinBuild.PointCount] = nil
			ply.CinBuild.PointCount = ply.CinBuild.PointCount -1
			
			ply:SendPrint(string.format("Point %i removed", pt))
			ply:SendCommand(string.format("cb rp %i", pt))
			return
		else
			-- Bad args
			ply:SendPrint("^1Invalid arguments provided")
		end
		return
	end
	
	-- insert <index> [position] - Inserts a new point before the specified point at your position or the specified one
	if (cmd == 'insert' ) then
		-- Check which args we got
		if (argc == 3) then
			-- Insert point
			local pt = tonumber(argv[2])
			if (pt < 1 or pt > ply.CinBuild.PointCount) then
				ply:SendPrint("Invalid point specified")
				return
			end
			for i = ply.CinBuild.PointCount, pt, -1 do
				ply.CinBuild.Points[i+1] = ply.CinBuild.Points[i]
			end
			ply.CinBuild.PointCount = ply.CinBuild.PointCount + 1
			ply.CinBuild.Points[pt] = {}
			local point = ply.CinBuild.Points[pt]
			point.origin = ply.Pos
			point.angles = ply.Angles
			if (point.angles.y) < 0 then
				point.angles.y = point.angles.y + 360	-- Normalize all angles to 0 to 360 range, rather than JA's -180 to 180
			end
			point.time = ply.CinBuild.DefaultTime
			
			ply:SendPrint(string.format("Point inserted (Point: %i)", pt))
			ply:SendCommand(string.format("cb ip %i %.3f %.3f %.3f %.3f %.3f %.3f %i", pt, point.origin.x, point.origin.y, point.origin.z, point.angles.x, point.angles.y, point.angles.z, point.time))
		elseif (argc == 6) then
			-- Insert point
			local pt = tonumber(argv[2])
			if (pt < 1 or pt > ply.CinBuild.PointCount) then
				ply:SendPrint("Invalid point specified")
				return
			end
			for i = ply.CinBuild.PointCount, pt, -1 do
				ply.CinBuild.Points[i+1] = ply.CinBuild.Points[i]
			end
			ply.CinBuild.PointCount = ply.CinBuild.PointCount + 1
			ply.CinBuild.Points[pt] = {}
			local point = ply.CinBuild.Points[pt]
			point.origin = Vector(tonumber(argv[2]), tonumber(argv[3]), tonumber(argv[4]))
			point.angles = ply.Angles
			if (point.angles.y) < 0 then
				point.angles.y = point.angles.y + 360	-- Normalize all angles to 0 to 360 range, rather than JA's -180 to 180
			end
			point.time = ply.CinBuild.DefaultTime
			
			ply:SendPrint(string.format("Point inserted (Point: %i)", pt))
			ply:SendCommand(string.format("cb ip %i %.3f %.3f %.3f %.3f %.3f %.3f %i", pt, point.origin.x, point.origin.y, point.origin.z, point.angles.x, point.angles.y, point.angles.z, point.time))
		else
			-- Bad args
			ply:SendPrint("^1Invalid arguments provided")
		end
		return
	end
		
	--nudge <index> <offset> - Moves the point into the specified direction
	if (cmd == 'nudge' ) then
		-- Check which args we got
		if (argc >= 6) then
			-- Place at current pos
			local pt = tonumber(argv[2])
			if (pt > ply.CinBuild.PointCount or pt < 1) then
				ply:SendPrint("^1Invalid point specified")
			else
				local point = ply.CinBuild.Points[pt]
				point.origin = point.origin + Vector( tonumber(argv[3]), tonumber(argv[4]), tonumber(argv[5]) )
				ply:SendPrint(string.format("Point %i nudged", pt))
				ply:SendCommand(string.format("cb ep %i %.3f %.3f %.3f %.3f %.3f %.3f %i", pt, point.origin.x, point.origin.y, point.origin.z, point.angles.x, point.angles.y, point.angles.z, point.time))
			end
			return
		else
			-- Bad args
			ply:SendPrint("^1Invalid arguments provided")
		end
		return
	end
	
	-- clear - Removes all points
	if (cmd == 'clear' ) then
		-- Check which args we got
		ply.CinBuild.PointCount = 0
		ply.CinBuild.Points = {}
		ply:SendPrint(string.format("All points removed", pt))
		ply:SendCommand(string.format("cb clr"))
		return
	end
	
	-- settarget [origin] - Places the view target at your position or the position specified
	if (cmd == 'settarget' ) then
		if (argc == 2) then
			ply.CinBuild.AimTarget = ply.Pos
			ply:SendPrint("Target set")
			ply:SendCommand(string.format("cb st %.3f %.3f %.3f", ply.CinBuild.AimTarget.x, ply.CinBuild.AimTarget.y, ply.CinBuild.AimTarget.z))
		elseif (argc == 5) then
			ply.CinBuild.AimTarget = Vector(tonumber(argv[2]), tonumber(argv[3]), tonumber(argv[4]))
			ply:SendPrint("Target set")
			ply:SendCommand(string.format("cb st %.3f %.3f %.3f", ply.CinBuild.AimTarget.x, ply.CinBuild.AimTarget.y, ply.CinBuild.AimTarget.z))
		else
			ply:SendPrint("Invalid arguments")
		end
		return
	end
	
	-- cleartarget - Removes the view target
	if (cmd == 'cleartarget' ) then
		ply.CinBuild.AimTarget = nil
		ply:SendCommand("cb ct")
		return
	end
	
	--fixangles - Changes the angles on all points ot make the moves the shortest possible (fixes cam flips when going from 0 to 360)
	if (cmd == 'fixangles' ) then
		-- Go through all points and ensure all angle deltas are below 180 degrees
		if ply.CinBuild.PointCount > 1 then
			ply:SendCommand("cb fixang")
			for i = 1, ply.CinBuild.PointCount - 1 do
				local delta = ply.CinBuild.Points[i+1].angles.y - ply.CinBuild.Points[i].angles.y
				if (delta > 180) then
					-- First check for sign changes
					if ply.CinBuild.Points[i+1].angles.y < 0 and ply.CinBuild.Points[i].angles.y > 0 then
						ply.CinBuild.Points[i+1].angles.y = 360 + ply.CinBuild.Points[i+1].angles.y
					elseif ply.CinBuild.Points[i+1].angles.y > 0 and ply.CinBuild.Points[i].angles.y < 0 then
						ply.CinBuild.Points[i+1].angles.y = ply.CinBuild.Points[i+1].angles.y - 360
					else
						ply.CinBuild.Points[i+1].angles.y = ply.CinBuild.Points[i+1].angles.y - 360
					end
				elseif (delta < -180) then
					if ply.CinBuild.Points[i+1].angles.y < 0 and ply.CinBuild.Points[i].angles.y > 0 then
						ply.CinBuild.Points[i+1].angles.y = 360 + ply.CinBuild.Points[i+1].angles.y
					elseif ply.CinBuild.Points[i+1].angles.y > 0 and ply.CinBuild.Points[i].angles.y < 0 then
						ply.CinBuild.Points[i+1].angles.y = -360 + ply.CinBuild.Points[i+1].angles.y
					else
						ply.CinBuild.Points[i+1].angles.y = 360 + ply.CinBuild.Points[i+1].angles.y
					end					
				end
			end
		end
		return
	end
	
	-- setalgos <x> <y> <z> <pitch> <yaw> <roll> - Set spline interpolation algorithms: 0 = Cubic spline, 1 = B-Spline, 2 = Catmull-Rom, 3 = Linear
	if (cmd == 'setalgos' ) then
		local temp
		if (argc == 3) then
			ply.CinBuild.SplineAlgos = {tonumber(argv[2]),
										tonumber(argv[2]),
										tonumber(argv[2]),
										tonumber(argv[2]), 
										tonumber(argv[2]), 
										tonumber(argv[2])}
			temp = ply.CinBuild.SplineAlgos
			ply:SendCommand(string.format("cb alg %i %i %i %i %i %i", temp[1], temp[2], temp[3], temp[4], temp[5], temp[6]))
		elseif (argc == 4) then
			ply.CinBuild.SplineAlgos = {tonumber(argv[2]),
										tonumber(argv[2]),
										tonumber(argv[2]),
										tonumber(argv[3]), 
										tonumber(argv[3]), 
										tonumber(argv[3])}
			temp = ply.CinBuild.SplineAlgos
			ply:SendCommand(string.format("cb alg %i %i %i %i %i %i", temp[1], temp[2], temp[3], temp[4], temp[5], temp[6]))
		elseif (argc == 8) then
		
			ply.CinBuild.SplineAlgos = {tonumber(argv[2]),
										tonumber(argv[3]),
										tonumber(argv[4]),
										tonumber(argv[5]), 
										tonumber(argv[6]), 
										tonumber(argv[7])}
			temp = ply.CinBuild.SplineAlgos
			ply:SendCommand(string.format("cb alg %i %i %i %i %i %i", temp[1], temp[2], temp[3], temp[4], temp[5], temp[6]))
		else
			ply:SendPrint("Syntax: cinbuild setalgos <x> <y> <z> <pitch> <yaw> <roll>\nAlgos: 0 = Cubic spline, 1 = B-Spline, 2 = Catmull-Rom, 3 = Linear")
		end
		return
	end
	
	-- interval <ms interval> - Set spline curve interval (time distance between points in the visualisation), default is 250 ms
	if (cmd == 'interval' ) then
		if (argc < 3) then
			ply:SendPrint("Please specify an interval")
		else
			local temp
			temp = tonumber(argv[2])
			if (temp < 25) then
				ply:SendPrint("The minimal interval is 25 milliseconds")
				return
			elseif (temp > 2000) then
				ply:SendPrint("The maximum interval is 2000 milliseconds")
				return
			end
			ply:SendCommand(string.format("cb int %i", temp))
		end
		return
	end
	
	-- preview ['clean'] - Play back the cinematic, if you use preview clean, the visualisation will be disabled during the preview
	if (cmd == 'preview' ) then
		if (argv[2] == 'clean') then
			CinBuildPlayCin(ply, true)
		else 
			CinBuildPlayCin(ply, false)
		end
		return
	end
	
	-- setdeftime <time> - Sets default move time for new points (in ms)
	if (cmd == 'setdeftime') then
		if ( argc < 3 ) then
			ply:SendPrint("Please specify an default time")
			return
		end
		local time = tonumber(argv[2])
		if ( time < 0 ) then
			ply:SendPrint("^1Cannot set a negative move time!")
			return
		end
		ply.CinBuild.DefaultTime = time
		ply:SendPrint(string.format("New default move time: %i ms", time))
		return
	end
	
	-- setdelays <start delay> <end delay) - Sets the start and end delays for preview (in ms)
	if (cmd == 'setdelays') then
		if ( argc < 4 ) then
			ply:SendPrint("Please specify an the delays")
			return
		end
		local sdelay = tonumber(argv[2])
		local edelay = tonumber(argv[3])
		if ( sdelay < 0 or edelay < 0 ) then
			ply:SendPrint("^1Delays cannot be negative!")
			return
		end
		ply.CinBuild.Delays = { sdelay, edelay }
		ply:SendPrint("New delays set")
		return
	end
	
	-- save <name> - Saves the cinematic
	if (cmd == 'save' ) then
		if (not argv[2]) then
			ply:SendPrint("Please specify a name")
			return
		end
		SaveCinematic(ply, argv[2])
		ply:SendPrint("Successfully saved cinematic")
		return
	end
	
	if (cmd == 'export' ) then
		if (not argv[2]) then
			ply:SendPrint("Please specify a name")
			return
		end
		CinBuildExport(ply, argv[2])
		return
	end
	
	-- load <name> - Loads a saved cinematic
	if (cmd == 'load' ) then
		if (not argv[2]) then
			ply:SendPrint("Please specify a name")
			return
		end
		local success = LoadCinematic(ply, argv[2])
		if (success) then
			-- OOOOH boy xD, time to send all of it to the client
			local temp
			local i
			local point
			ply:SendCommand(string.format("cb surp 1")) -- Supress trajectory recalculations while we resend the whole thing
			ply:SendCommand(string.format("cb clr"))	-- Clear all points
			-- Update algos
			ply:SendCommand(string.format("cb type %i", ply.CinBuild.TrType))
			temp = ply.CinBuild.SplineAlgos
			ply:SendCommand(string.format("cb alg %i %i %i %i %i %i", temp[1], temp[2], temp[3], temp[4], temp[5], temp[6]))
			-- Transmit points
			for i=1, ply.CinBuild.PointCount do
				point = ply.CinBuild.Points[i]
				ply:SendCommand(string.format("cb ap %.3f %.3f %.3f %.3f %.3f %.3f %i", point.origin.x, point.origin.y, point.origin.z, point.angles.x, point.angles.y, point.angles.z, point.time))
			end
			ply:SendCommand(string.format("cb surp 0")) -- Re-enable trajectory calculations
			ply:SendPrint("Successfully loaded cinematic")
		else
			ply:SendPrint("^1Could not load cinematic")
		end			
		return
	end
	
	ply:SendPrint("Bad command, please use /cinbuild help for a command list")

end


cmds.Add("cinbuild", CinBuild)
