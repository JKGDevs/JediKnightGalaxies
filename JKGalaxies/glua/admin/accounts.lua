--[[ ------------------------------------------------
	JKGalaxies Lua Code
	Accounts and admin system
		
	written by eezstreet
	improved by Futuza
--------------------------------------------------]]

--[[ ------------------------------------------------
	Accounts / Data Structures
--------------------------------------------------]]

accounts = { }
ranks = { }
-- Never access sortedAccounts/sortedRanks directly, it's used for saving et al
sortedAccounts = { }
sortedRanks = { }

-- Felt like refactoring this, SO THEN I DOUBLED IT
permissions = { }
sortedpermissions = { }
local numPermissions = 0


--[[ ------------------------------------------------
	Generic Functions
 ------------------------------------------------]]

--used to count the total elements in a table
function tablelength(T)
	local count = 0
	for _ in pairs(T) do count = count + 1 end
	return count
  end



--[[ ------------------------------------------------
	AddPermission( permissionname, permissiondefault, friendlyname, color )
	Adds a permission to the permission table.
--------------------------------------------------]]

local function AddPermission( permissionname, permissiondefault, friendlyname, color )
	local permission = { }
	permission["name"] = permissionname 
	permission["default"] = permissiondefault or 0  --if no default permission specified use 0
	permission["friendlyname"] = friendlyname
	permission["color"] = color

	permissions[permissionname] = permission
	table.insert( sortedpermissions, numPermissions, permission )

	numPermissions = numPermissions + 1
end

--[[ ------------------------------------------------
	InitPermissions()
	Creates all of the standard permissions.
--------------------------------------------------]]

-- High-Risk (Red/^1): Kicking/Banning, mostly. These are dangerous and should only be given to high-ranking admins.
-- Harmless (Green/^2): Status, listing stuff, help, etc. These aren't harmful from a security standpoint.
-- Self-Exploitable (Yellow/^3): From a security standpoint, these are only harmful to the person who uses it. Changing details falls into this category.
-- Building Commands (Blue/^4): Building stuff. Built stuff can be destroyed by the users (later on, when we implement that, anyway), so no harm done.
-- Communication (Cyan/^5): Talking with clients, and amongst admins. Can be extremely annoying if spoofed.
-- Structural (Magenta/^6): Alters the structure of administration, such as dealing with ranks. Not necessarily needed on accounts, but they're there for convenience.
-- Annoying (Orange/^8): While not necessarily harmful, if used, they can be annoying. Stuff like slap, slay, etc falls into this category.

local function InitPermissions( )
	AddPermission( "can-changedetails", 		1, "admchangedetails", 		"^3" )
	AddPermission( "can-addaccounts", 			0, "admnewaccount", 		"^6" )
	AddPermission( "can-deleteaccounts", 		0, "admdeleteaccount", 		"^6" )
	AddPermission( "can-kick", 					0, "admkick", 		"^6" )
	AddPermission( "can-list-online", 			1, "admlist online", 		"^2" )
	AddPermission( "can-list-admins", 			0, "admlist admins", 		"^2" )
	AddPermission( "can-list-powers", 			0, "admlist ranks", 		"^2" )
	AddPermission( "can-list-ranks", 			0, "admlist powers", 		"^2" )
	AddPermission( "can-list-permissions", 		0, "admlist permissions", 	"^2" )
	AddPermission( "can-rank-inspect", 			0, "admrank inspect", 		"^2" )
	AddPermission( "can-rank-create", 			0, "admrank create", 		"^1" )
	AddPermission( "can-rank-delete", 			0, "admrank delete", 		"^1" )
	AddPermission( "can-rank-addpermission", 	0, "admrank addpermission", 	"^1" )
	AddPermission( "can-rank-deletepermission", 	0, "admrank deletepermission", 	"^1" )
	AddPermission( "can-alter-rank", 		0, "admalter rank", 		"^6" )
	AddPermission( "can-alter-password", 	0, "admalter password", 	"^6" )
	AddPermission( "can-status", 			1, "admstatus", 			"^2" )
	AddPermission( "can-say", 				0, "admsay", 				"^5" )
	AddPermission( "can-tell", 				0, "admtell", 				"^5" )
	AddPermission( "can-speak", 			0, "admspeak", 				"^5" )
	AddPermission( "can-announce",			0, "admannounce",       	"^8" )
	AddPermission( "can-puppet", 			0, "admpuppet", 			"^8" )
	AddPermission( "can-changemap",			0, "admchangemap",			"^8" )
	AddPermission( "can-teleport", 			0, "teleport", 				"^8" )
	AddPermission( "can-setclip", 			0, "setclip",				"^8" )
	AddPermission( "can-usetarg",			1, "usetarg",				"^8" )
	AddPermission( "use-cheats", 			0, "Use Cheats", 			"^8" )
	AddPermission( "can-place", 			0, "bPlace", 				"^4" )
	AddPermission( "can-delent", 			0, "bDelent", 				"^4" )
	AddPermission( "can-entcount", 			1, "bEntCount", 			"^4" )
	AddPermission( "can-showspawnvars", 	0, "bShowSpawnVars", 		"^4" )
	AddPermission( "can-rotate", 			0, "bRotate", 				"^4" )
	AddPermission( "can-cinbuild", 			0, "cinbuild", 				"^6" )
end

--[[ ------------------------------------------------
	GetRank
	Gets the rank of a player.
--------------------------------------------------]]

function GetRank(ply)
	if ply.isLoggedIn then
		local account = ply:GetAccount()
		return ranks[accounts[account]["rank"]]
	end
	return nil
end

--[[ ------------------------------------------------
	InitRanks
	Inits all of the information about ranks
--------------------------------------------------]]

local function InitRanks( )
	-- Grab our file
	local jRoot = json.RegisterFile("server/ranks.json")
	
	if jRoot == nil then
		json.Clear()
		securitylog("Could not register ranklist.json")
		return
	end

	-- Load up the list of ranks
	local rankArray = json.GetObjectItem( jRoot, "ranks" )
	if rankArray == nil then
		-- Lol, does not exist.
		json.Clear()
		securitylog("ranks array in ranks.json non-existing.")
		return
	end

	-- Loop through objects
	local i, j
	for i = 0, json.GetArraySize(rankArray)-1 do
		local jObject = json.GetArrayItem( rankArray, i )
		local rank = {}
		local jObjectItem

		jObjectItem = json.GetObjectItem( jObject, "name" )
		rank["name"] = json.ToString( jObjectItem )

		--
		-- INSERT COMMAND-BASED STUFF HERE
		--

		for j = 0, numPermissions-1 do
			local permname = sortedpermissions[j]["name"]
			local defvalue = sortedpermissions[j]["default"]

			jObjectItem = json.GetObjectItem( jObject, permname )

			--so lua is a bit dumb, 0 evaluates as true, so this will ensure only positive values == true
			if defvalue < 1 then
				defvalue = false
			else
				defvalue = true
			end

			rank[permname] = json.ToBooleanOpt( jObjectItem , defvalue )
		end
	
		--
		-- END COMMAND-BASED STUFF
		--

		rank["sorted"] = i

		ranks[rank["name"]] = rank
		table.insert(sortedRanks, i, rank)
	end

	ranks["numRanks"] = json.GetArraySize(rankArray)

	-- Make sure to clean up our mess.
	json.Clear()
end

--[[ ------------------------------------------------
	InitAccountList
	Opens up the json/savefile for the admins,
	and then applies this information to the ingame system
--------------------------------------------------]]

local function InitAccountList ( )
	-- Grab our file
	local jRoot = json.RegisterFile("server/accountlist.json")

	if jRoot == nil then
		json.Clear()
		securitylog("Could not register accountlist.json")
		return
	end

	-- Instead of using a global Lua variable, let's load up a list of users
	local accountsArray = json.GetObjectItem( jRoot, "accounts" )
	if accountsArray == nil then
		-- Lol, does not exist.
		json.Clear()
		securitylog("accounts array in accountlist.json non-existing.")
		return
	end

	-- ok, so loop through the objects
	local i
	for i = 0, json.GetArraySize(accountsArray)-1 do
		local jObject = json.GetArrayItem( accountsArray, i )
		local account = {}
		local jObjectItem

		jObjectItem = json.GetObjectItem( jObject, "username" )
		account["username"] = json.ToString( jObjectItem )
		
		jObjectItem = json.GetObjectItem( jObject, "password" )
		account["password"] = json.ToString( jObjectItem )

		if account["password"] == "password" then
			print "^1***** SECURITY WARNING *****: Detected an admin account with default password. Change it in server/accountlist.json."
		end

		jObjectItem = json.GetObjectItem( jObject, "rank" )
		account["rank"] = json.ToString( jObjectItem )

		if ranks[account["rank"]] == nil then
			securitylog("WARNING: Account " .. account["username"] .. " pointing to invalid rank " .. account["rank"])
			print("^3WARNING: Account " .. account["username"] .. " pointing to invalid rank " .. account["rank"])
		end

		account["sorted"] = i

		accounts[account["username"]] = account
		table.insert(sortedAccounts, i, account)
		
	end

	accounts["numAccounts"] = json.GetArraySize(accountsArray)

	-- Make sure to clean up our mess.
	json.Clear()
end

--[[ ------------------------------------------------
	AdminHelp_ListPowers
	Lists powers for a given rank, or, lists
	all powers. OR, this can be used on
	permissions also. It's like a 4-in-1 func.
--------------------------------------------------]]

local function AdminHelp_ListPowers( rank, listpermissions, listall )
	-- This code is used in multiple places, I figured it would be wise to put this into a function
	if rank == nil then
		return
	end

	local printedtext = ""
	local i

	-- Loop through everything.
	for i = 0, numPermissions-1 do
		local perm = sortedpermissions[i]
		local printcolor = perm["color"]
		local permname = perm["name"]
		local addtext

		-- Make sure we have this permission
		if listall == true or rank[permname] then
			if listpermissions == true then
				addtext = permname
			else
				addtext = perm["friendlyname"]
			end
			printedtext = printedtext .. printcolor .. addtext .. "^7, "
		end
	end
						
	return printedtext
end

--[[ ------------------------------------------------
	IsValid
	Checks if a permission is valid or not
--------------------------------------------------]]

local function IsValid( permissionname )
	if permissions[permissionname] ~= nil then
		return true
	else
		return false
	end
end

--[[ ------------------------------------------------
	SaveRanks
	Saves all the ranks into a file.
--------------------------------------------------]]

local function SaveRanks( reason )
	-- Save the .json file
	json.RegisterStream( 3, 1 )

	-- Root object
	json.BeginObject( "0" )

	-- "ranks" array
	json.BeginArray( "ranks" )

	-- Okay, now to save all of the ranks...
	local k, l
	for k = 0, ranks["numRanks"]-1 do
		-- Array base object
		json.BeginObject( "0" )

		local sortedrank = sortedRanks[k]

		json.WriteString( "name", sortedrank["name"] )

		for l = 0, numPermissions-1 do
			local permission = sortedpermissions[l]
			local permname = permission["name"]

			if sortedrank[permname] ~= nil then
				json.WriteBoolean( permname, sortedrank[permname] )
			else
				json.WriteBoolean( permname, permission["default"] )
			end
		end
		
		-- Array base object
		json.EndObject( )
	end

	-- "ranks" array
	json.EndObject()

	-- Root object
	json.EndObject()

	-- Now save it all up
	json.FinishStream( "server/ranks.json" )

	-- Log it to the security log
	securitylog("Ranks saved. (Reason: " .. reason .. " )")
end

--[[ ------------------------------------------------
	UpdateRank
	Updates the savefile.
--------------------------------------------------]]

local function UpdateRank( rank, reason )
	ranks[rank["name"]] = rank
	if sortedRanks[rank["sorted"]] ~= nil then
		sortedRanks[rank["sorted"]] = account
	else
		-- Likely a new rank
		table.insert(sortedRanks, ranks["numRanks"]-1, rank)
	end
	SaveRanks( reason )
end

--[[ ------------------------------------------------
	AddRank
	Adds a new rank into the system and
	saves the ranks.
--------------------------------------------------]]

local function AddRank( rank )
	ranks["numRanks"] = ranks["numRanks"] + 1
	table.insert(sortedRanks, ranks["numRanks"]-1, rank)
	UpdateRank( rank, "Added new rank: " .. rank["name"] )
end

--[[ ------------------------------------------------
	DeleteRank
	Deletes a rank and saves the ranks.
--------------------------------------------------]]

local function DeleteRank( rank )
	local rankname = rank["name"]
	table.remove(sortedRanks, rank["sorted"])
	ranks[rank["name"]] = nil
	-- One sec, make sure we aren't deleting the last rank...
	if ranks["numRanks"] == 1 then
		print("^1ERROR: admin tried to delete last rank...")
		return
	end
	-- Okay, we need to change the sorted stuff now
	local k
	for k = 0, ranks["numRanks"]-1 do
		if sortedRanks[k] ~= nil and ranks[sortedRanks[k]] ~= nil then
			ranks[sortedRanks[k]]["sorted"] = k
		end
	end

	ranks["numRanks"] = ranks["numRanks"] - 1
	SaveRanks( "Deleted rank: " .. rankname )
end

--[[ ------------------------------------------------
	SaveAccounts
	Saves all the accounts into a file.
--------------------------------------------------]]

local function SaveAccounts( reason )
	-- Save the .json file
	json.RegisterStream( 3, 1 )
	
	-- Root object
	json.BeginObject( "0" )

	-- "accounts" array
	json.BeginArray( "accounts" )

	-- Okay, now we're going to save all the accounts...
	local k
	for k = 0, accounts["numAccounts"]-1 do

		-- Array base object
		json.BeginObject( "0" )

		local sortedaccount = sortedAccounts[k]

		json.WriteString( "username", sortedaccount["username"] )
		json.WriteString( "password", sortedaccount["password"] )
		json.WriteString( "rank", sortedaccount["rank"] )

		-- Array base object
		json.EndObject( )
	end

	-- "accounts" array
	json.EndObject( )

	-- Root object
	json.EndObject( )

	-- Now save it all up
	json.FinishStream( "server/accountlist.json" )

	-- Log it to the security log
	securitylog("Accounts saved. (Reason: " .. reason .. " )")
end

--[[ ------------------------------------------------
	UpdateAccount
	Updates the accounts and calls SaveAccounts
--------------------------------------------------]]

local function UpdateAccount( account, reason )
	accounts[account["username"]] = account
	if sortedAccounts[account["sorted"]] ~= nil then
		sortedAccounts[account["sorted"]] = account
	else
		-- Likely a new account
		table.insert(sortedAccounts, accounts["numAccounts"]-1, account)
	end
	SaveAccounts( reason )
end

--[[ ------------------------------------------------
	AddAccount
	Adds a new account into the system
	Also saves all the accounts.
--------------------------------------------------]]

local function AddAccount( account )
	accounts["numAccounts"] = accounts["numAccounts"] + 1
	table.insert(sortedAccounts, accounts["numAccounts"]-1, account)
	UpdateAccount( account, "Added new account: " .. account["username"] .. " [Rank: " .. account["rank"] .. "]" )
end

--[[ ------------------------------------------------
	DeleteAccount
	Deletes an account and saves all accounts.
--------------------------------------------------]]

local function DeleteAccount( account )
	local username = account["username"]
	table.remove(sortedAccounts, account["sorted"])
	accounts[account["username"]] = nil
	-- Okay, we need to change the sorted stuff now
	local k
	for k = 0, accounts["numAccounts"]-1 do
		if sortedAccounts[k] ~= nil and accounts[sortedAccounts[k]] ~= nil then
			accounts[sortedAccounts[k]]["sorted"] = k
		end
	end

	accounts["numAccounts"] = accounts["numAccounts"] - 1
	SaveAccounts( "Deleted account: " .. username )
end

--[[ ------------------------------------------------
	InitAccounts
	Inits ranks, admin list, permissions, etc
--------------------------------------------------]]

local function InitAccounts( )
	InitPermissions()
	InitRanks()
	InitAccountList()
end

--[[ ------------------------------------------------
	Commands
--------------------------------------------------]]

local function SystemReply(ply, message)
	ply:SendChat("^7System: " .. message)
end

local function Login(ply, argc, argv)
	if argc ~= 3 then
		-- Invalid number of arguments
		SystemReply(ply, "^1Invalid arguments")
		return
	end
	if ply.isLoggedIn then
		-- We are logged in, so no need to re-log us
		local account = ply:GetAccount()
		SystemReply(ply, "^4You are already logged in as ^7" .. account .. "")
		return
	end
	if accounts[argv[1]] ~= nil then
		if accounts[argv[1]]["password"] == argv[2] then
			ply.isLoggedIn = true
			ply:SetAccount(argv[1])
			SystemReply(ply, "^2Login successful")

			local account = accounts[argv[1]]
			local rank = ranks[account["rank"]]

			if rank["use-cheats"] == true then
				-- We can use cheats. Alright, mang! Let's set things up so that the C++ code knows that we can...
				ply.CanUseCheats = true
			end
			return
		end
	end
	SystemReply(ply, "^1Bad username/password")
end

local function Logout(ply, argc, argv)
	local k,v
	if ply.isLoggedIn then
		ply.isLoggedIn = false
		ply:SetAccount(" ")
		if ply.CanUseCheats then
			ply.CanUseCheats = false -- don't want us using cheats when we shouldn't be able to...
		end
		SystemReply(ply, "^5You are now logged out.")
		return
	else
	SystemReply(ply, "^5You are already logged out.")
	end
	chatcmds.Ignore()
end


local function ChangePassword(ply, argc, argv)
	if ply.isLoggedIn then
			if argc ~= 3 then
				SystemReply(ply, "Syntax: /changepassword <oldpass> <newpass>")
			else
				local accountname = ply:GetAccount()
				local account = accounts[accountname]
				if argv[1] ~= account["password"] then
					SystemReply(ply, "Your old password did not match correctly. Please try again.")
				else
					-- kk, change.
					account["password"] = argv[2]
					UpdateAccount( account, account["username"] .. " changed their password" )
					SystemReply(ply, "^4Password changed.")
				end
			end
	else
	SystemReply(ply, "^1You are not logged in.")
	end
end

local function Register(ply, argc, argv)
	local len = 0
	username = argv[1]
	password = argv[2]
	
	if ply.isLoggedIn then
		-- We are logged in, so no need to re-log us
		local account = ply:GetAccount()
		SystemReply(ply, "^4You are already logged in as ^7" .. account .. "")
		return
	end
	
	if argc ~= 3 then
		SystemReply(ply, "^3Syntax: /register <username> <password>")
	else
		if accounts[argv[1]] ~= nil then
			SystemReply(ply, "^1Could not create account, an account by this name already exists.")
			return
		end
			
		len = string.len(username)
		if (len < 3) then
			SystemReply(ply, "^1Username needs to be at least 3 characters in length.")
			return
		end

		if (len > 36) then
			SystemReply(ply, "^1Username needs to be no more than 36 characters in length.")
			return 
		end

		len = string.len(password)
		if (len < 6) then
			SystemReply(ply, "^1Password needs to be at least 6 characters in length.")
			return
		end

		-- Set the username to lowercase so it is case INsensitve
		lowercase_username = string.lower(username)

		local newaccount = {}
		newaccount["username"] = argv[1]
		newaccount["password"] = argv[2]
		newaccount["rank"] = "Client"
		AddAccount( newaccount )
		SystemReply(ply, "^4Account created! You can now /login.")
		-- try automatic login here? wasn't working when I tried it.
	end
end


local function Kick(ply, argc, argv)
	local k,v
	if ply.isLoggedIn then
		local rank = GetRank(ply)
		if rank["can-kick"] == true then
			if argc < 2 then
				SystemReply(ply, "^1Please specify a player to kick.")
				return
			end
			local target =  players.GetByArg(argv[1])
			if not target then
				SystemReply(ply, "^1Invalid player specified")
				return
			end
			local reason
			if argc > 2 then
				reason = table.concat(argv," ",2, argc-1)
			else 
				reason = nil
			end
			SystemReply(ply, "^5Player " .. target:GetName() .. " ^5has been kicked.")
			target:Kick(reason)
			return
		end
	end
	chatcmds.Ignore()
end

local function ChangeDetails(ply, argc, argv)
	if ply.isLoggedIn then
		local rank = GetRank(ply)
		if rank["can-changedetails"] == true then
			-- Change our account details then?
			if argc < 2 then
				SystemReply(ply, "Please specify which details you'd like to change: password.")
			elseif argv[1] == "password" or argv[1] == "Password" then
				-- We want to change the password on this account.
				-- Syntax:
				-- /admchangedetails password <oldpass> <newpass>
				if argc ~= 4 then
					SystemReply(ply, "Syntax: /admchangedetails password <oldpass> <newpass>")
				else
					local accountname = ply:GetAccount()
					local account = accounts[accountname]
					if argv[2] ~= account["password"] then
						SystemReply(ply, "Your old password did not match correctly. Please try again.")
					else
						-- kk, change.
						account["password"] = argv[3]
						UpdateAccount( account, account["username"] .. " changed their password" )
						SystemReply(ply, "^4Password changed.")
					end
				end
			else
				SystemReply(ply, "Invalid details type specified. Valid details: password")
			end
		else
			SystemReply(ply, "^4You don't have permission to perform this action.")
		end
	else
		SystemReply(ply, "^1You are not logged in.")
	end
end

local function Profile(ply, argc, argv)
	if ply.isLoggedIn then
		local plyselaccountname = ply:GetAccount()
		local plyselaccount = accounts[plyselaccountname]
		SystemReply(ply, "^2You are logged in as ^4" .. plyselaccountname .. "^5 [Rank: " .. plyselaccount["rank"] .. "]")
		return
	else
		SystemReply(ply, "^1You are not logged in.")
	end
		chatcmds.Ignore()
end

local function CreateAccount(ply, argc, argv)
	if ply.isLoggedIn then
		-- Ya they're an logged in...but are they authorized?
		local rank = GetRank(ply)
		if rank["can-addaccounts"] ~= true then
			SystemReply(ply, "^1You do not have permission to perform this action.")
		else
			-- Syntax:
			-- /admnewaccount <username> <password> <rank>
			if argc ~= 4 then
				SystemReply(ply, "^3Syntax: /admnewaccount <username> <password> <rank>")
			else
				if accounts[argv[1]] ~= nil then
					SystemReply(ply, "^1Could not create account, an account by this name already exists.")
				else
					local newaccount = {}
					newaccount["username"] = argv[1]
					newaccount["password"] = argv[2]
					newaccount["rank"] = argv[3]
					if ranks[newaccount["rank"]] ~= nil then
						AddAccount( newaccount )
						SystemReply(ply, "^4Account added.")
					else
						SystemReply(ply, "^3The account you added does not have a valid rank. Please add the rank and try again." )
					end
				end
			end
		end
	else
		SystemReply(ply, "^1You are not logged in.")
	end
end

local function LogoutAccount(account)
	local k = 0
	while players.GetByID(k) ~= nil do
		ply = players.GetByID(k)
		if ply.isLoggedIn then
			if account["username"] == ply:GetAccount() then
				-- soz you've been booted
				SystemReply(ply, "^6Your account has been deleted. You have been logged out.")
				ply.isLoggedIn = false
				ply:SetAccount(" ")
			end
		end

		k = k + 1
	end
end

local function RemoveAccount(ply, argc, argv)
	if ply.isLoggedIn then
		local rank = GetRank(ply)
		if rank["can-deleteaccounts"] ~= true then
			SystemReply(ply, "^1You do not have permission to perform this action.")
			return
		else
			if argc ~= 2 then
				SystemReply(ply, "^3Syntax: /admdeleteaccount <username>")
				return
			end
			
			local desiredaccount = accounts[argv[1]]
			if desiredaccount ~= nil then
				local ourAccount = accounts[ply:GetAccount()]
				if ourAccount["username"] ~= argv[1] then
					LogoutAccount(desiredaccount)
					DeleteAccount(desiredaccount)
					SystemReply(ply, "^4Account deleted.")
				else
					SystemReply(ply, "^1You cannot delete your own account.")
				end
			else
				SystemReply(ply, "^1Cannot delete account, it does not exist.")
			end
		end
	else
		SystemReply(ply, "^1You are not logged in.")
	end
end

local function List(ply, argc, argv)
	if ply.isLoggedIn then
		if argc < 2 then
			SystemReply(ply, "^3Syntax: /admlist <online/accounts/ranks/powers>")
		else
			local rank = GetRank(ply)
			if argv[1] == "online" then
				if rank["can-list-online"] ~= true then
					SystemReply(ply, "^1You do not have permission to perform this action.")
				else
					local k = 0
					local printstring = ""
					SystemReply(ply, "^4Results printed to console.")
					while players.GetByID(k) ~= nil do
						local plysel = players.GetByID(k)
						if plysel:IsValid() then
							if plysel.isLoggedIn then
								local plyselname = plysel:GetName()
								local plyselaccountname = plysel:GetAccount()
								local plyselaccount = accounts[plyselaccountname]
								if plyselaccount ~= nil then
									printstring = printstring .. plysel:GetName() .. " ^7[" .. plyselaccountname .. "--" .. plyselaccount["rank"] .. "], "
								end
							end
						end
						k = k + 1
					end
					ply:SendPrint(printstring)
				end
			elseif argv[1] == "accounts" then
				if rank["can-list-admins"] ~= true then
					SystemReply(ply, "^1You do not have permission to perform this action.")
				else
					SystemReply(ply, "^4Results printed to console.")
					ply:SendPrint("^2All admin accounts:")
					local k
					local printstring = ""
					for k = 0, accounts["numAccounts"]-1 do
						printstring = printstring .. sortedAccounts[k]["username"] .. " [" .. sortedAccounts[k]["rank"] .. "], "
					end
					ply:SendPrint(printstring)
				end
			elseif argv[1] == "ranks" then
				if rank["can-list-ranks"] ~= true then
					SystemReply(ply, "^1You do not have permission to perform this action.")
				else
					SystemReply(ply, "^4Results printed to console.")
					ply:SendPrint("^2All ranks:")
					local k
					local printstring = ""
					for k = 0, ranks["numRanks"]-1 do
						printstring = printstring .. sortedRanks[k]["name"] .. ", "
					end
					ply:SendPrint(printstring)
				end
			elseif argv[1] == "powers" then
				if rank["can-list-powers"] ~= true then
					SystemReply(ply, "^1You do not have permission to perform this action.")
				else
					SystemReply(ply, "^4Results printed to console.")
					ply:SendPrint("^2Your powers:")
					
					ply:SendPrint(AdminHelp_ListPowers( rank, false, false ))
				end
			elseif argv[1] == "permissions" then
				if rank["can-list-permissions"] ~= true then	
					SystemReply(ply, "^1You do not have permission to perform this action.")
				else
					SystemReply(ply, "^4Results printed to console.")
					ply:SendPrint("^2All permissions:")

					ply:SendPrint(AdminHelp_ListPowers( rank, true, true ))
				end
			else
				SystemReply(ply, "^3Unknown admlist mode. Valid modes are online, accounts, ranks, powers")
			end
		end
	else
		SystemReply(ply, "^1You are not logged in.")
	end
end

local function Rank(ply, argc, argv)
	if ply.isLoggedIn then
		if argc < 2 then
			SystemReply(ply, "^3Syntax: /admrank <inspect> [...args...]")
		else
			local rank = GetRank(ply)
			if argv[1] == "inspect" then
				if argc < 3 then
					SystemReply(ply, "^3Syntax: /admrank inspect <rank>")
				elseif rank["can-rank-inspect"] ~= true then
					SystemReply(ply, "^1You do not have permission to perform this action.")
				else
					local inspectedrank = ranks[argv[2]]
					if inspectedrank == nil then
						SystemReply(ply, "^1'" .. argv[2] .. "' is not a valid rank.")
					else
						SystemReply(ply, "^4Rank permissions printed to console.")
						ply:SendPrint("^5This rank (" .. argv[2] .. ") can...")
						
						ply:SendPrint(AdminHelp_ListPowers( inspectedrank, false, false ))

						ply:SendPrint("^5Rank permissions:")

						ply:SendPrint(AdminHelp_ListPowers( inspectedrank, true, false ))
					end
				end
			elseif argv[1] == "create" then
				if argc < 3 then
					SystemReply(ply, "^3Syntax: /admrank create <rank name>")
				elseif rank["can-rank-create"] ~= true then
					SystemReply(ply, "^1You do not have permission to perform this action.")
				else
					local blankrank = {}
					blankrank["name"] = argv[2]
					AddRank( blankrank )
					SystemReply(ply, "^4Rank successfully added.")
				end
			elseif argv[1] == "delete" then
				if argc < 3 then
					SystemReply(ply, "^3Syntax: /admrank delete <rank> [change admins using this rank to THIS rank]")
				elseif rank["can-rank-delete"] ~= true then
					SystemReply(ply, "^1You do not have permission to perform this action.")
				else
					local rank = ranks[argv[2]]
					if rank == nil then
						SystemReply("^1ERROR: Invalid rank specified.")
					end

					-- Loop through the admins and make sure none of us are using this rank...
					local k
					for k = 0, accounts["numAccounts"]-1 do
						if sortedAccounts[k]["rank"]["name"] == argv[2] then
							if argc < 4 then
								SystemReply(ply, "^3This rank is in use. You will need to specify a rank for people to use, since this rank is gone.")
								return
							end
							sortedAccounts[k]["rank"] = ranks[argv[3]]
						end
					end

					-- K, guess it's okay to kill the rank, now...
					-- You will be missed, rank.
					DeleteRank( rank )
					SystemReply(ply, "^4Rank deleted.")
				end
			elseif argv[1] == "addpermission" then
				if argc < 4 then
					SystemReply(ply, "^3Syntax: /admrank addpermission <rank> <permission>")
				elseif rank["can-rank-addpermission"] ~= true then
					SystemReply(ply, "^1You do not have permission to perform this action.")
				else
					local rank = ranks[argv[2]]
					if rank == nil then
						SystemReply(ply, "^1Invalid rank specified.")
						return
					end

					local permissionname = argv[3]
					if IsValid( permissionname ) == false then
						SystemReply(ply, "^1Invalid permission specified.")
						return
					end

					rank[permissionname] = true
					UpdateRank( rank, ply:GetAccount() .. " added permission '" .. permissionname .. "' to rank " .. rank["name"])
					SystemReply(ply, "^4Rank updated successfully.")
				end
			elseif argv[1] == "deletepermission" then
				if argc < 4 then
					SystemReply(ply, "^3Syntax: /admrank deletepermission <rank> <permission>")
				elseif rank["can-rank-deletepermission"] ~= true then
					SystemReply(ply, "^1You do not have permission to perform this action.")
				else
					local rank = ranks[argv[2]]
					if rank == nil then
						SystemReply(ply, "^1Invalid rank specified.")
						return
					end

					local permissionname = argv[3]
					if IsValid( permissionname ) == false then
						SystemReply(ply, "^1Invalid permission specified.")
						return
					end

					rank[permissionname] = false
					UpdateRank( rank, ply:GetAccount() .. " deleted permission '" .. permissionname .. "' from rank " .. rank["name"])
					SystemReply(ply, "^4Rank updated successfully.")
				end
			else
				SystemReply(ply, "^3Unknown admrank mode, valid modes are: inspect, create, delete, addpermission, deletepermission")
			end
		end
	else
		SystemReply(ply, "^1You are not logged in.")
	end
end

local function Alter(ply, argc, argv)
	if ply.isLoggedIn then
		local accountname = ply:GetAccount()
		local rank = GetRank(ply)
		if argc ~= 4 then
			SystemReply(ply, "^3Syntax: /admalter <username> <rank/password> <change to..>")
		else
			local alteraccount = accounts[argv[1]]
			if alteraccount == nil then
				SystemReply(ply, "^1Invalid username for /admalter.")
			else
				if argv[2] == "rank" then
					if rank["can-alter-rank"] ~= true then
						SystemReply(ply, "^1You do not have permission to perform this action.")
					else
						local newrank = argv[3]
						if ranks[newrank] == nil then
							SystemReply(ply, "^1Invalid rank.")
						else
							SystemReply(ply, "^4Account altered.")
							alteraccount["rank"] = argv[3]
							UpdateAccount( alteraccount, accountname .. " altered " .. alteraccount["username"] .. "'s rank (changed to " .. argv[3] .. ")" )
						end
					end
				elseif argv[2] == "password" then
					if rank["can-alter-password"] ~= true then
						SystemReply(ply, "^1You do not have permission to perform this action.")
					else
						SystemReply(ply, "^4Account altered.")
						alteraccount["password"] = argv[3]
						UpdateAccount( alteraccount, accountname .. " altered " .. alteraccount["username"] .. "'s password" )
					end
				else
					SystemReply(ply, "^3Invalid alter type. Valid types are rank, password.")
				end
			end
		end
	else
		SystemReply(ply, "^1You are not logged in.")
	end
end

local function Status(ply, argc, argv)
	if ply.isLoggedIn then
		local rank = GetRank(ply)
		if rank["can-status"] ~= true then
			SystemReply(ply, "^1You do not have permission to perform this action.")
		else
			local k = 0
			SystemReply(ply, "^4Results printed to console.")
			ply:SendPrint("^2Status:")
			local printstring = ""
			while players.GetByID(k) ~= nil do
				local plysel = players.GetByID(k)
				if plysel:IsValid() then
					local plyselname = plysel:GetName()
					printstring = printstring .. "\n^7" .. k .. " - " .. plyselname
					if plysel.isLoggedIn then
						local plyselaccountname = plysel:GetAccount()
						local plyselaccount = accounts[plyselaccountname]
						printstring = printstring .. " ^7(^2Logged in as ^4" .. plyselaccountname .. " ^5[Rank: ^7" .. plyselaccount["rank"] .. "^5]^7)"
					else
						printstring = printstring .. " ^7(^1Not logged in.^7)"
					end
				end
				k = k + 1
			end
			ply:SendPrint(printstring)
		end
	else
		SystemReply(ply, "^1You are not logged in.")
	end
end

local function Speak(ply, argc, argv)
	if ply.isLoggedIn then
		local rank = GetRank(ply)
		if rank["can-speak"] ~= true then
			SystemReply(ply, "^1You do not have permission to perform this action.")
		else
			local message = table.concat(argv," ",1, argc-1)
			--local accountname = ply:GetAccount()
			--chatmsg( "^7[^5" .. accountname .. "^7] <" .. rank["name"] .. "> " .. message )
			chatmsg( "^7[^x39cAdmin^7] ^5" .. "^7<" .. rank["name"] .. "^7> ".. ply:GetName() .. "^7: ^3" .. message )
		end
	else
		SystemReply(ply, "^1You are not logged in.")
	end
end

local function Tell(ply, argc, argv)
	if ply.isLoggedIn then
		local rank = GetRank(ply)
		if rank["can-tell"] ~= true then
			SystemReply(ply, "^1You do not have permission to perform this action.")
		else
			if argc < 3 then
				SystemReply(ply, "^3Syntax: /admtell <player id/name> <message>")
			else
				local plytarg = players.GetByArg(argv[1])
				if plytarg == nil then
					SystemReply(ply, "^1Invalid player specified.")
					return
				end

				local account = accounts[ply:GetAccount()]
				local message = table.concat(argv," ",2, argc-1)

				plytarg:SendChat( "^5Admin " .. ply.Name .. " ^5whispers: ^7" .. message )
				ply:SendChat("^5You whisper: ^7" .. message )

				if plytarg.lastadmtell == nil then
					plytarg:SendChat( "^8Reply to this message using /Reply <msg>" )
				end
				plytarg.lastadmtell = account["username"]
			end
		end
	else
		SystemReply(ply, "^1You are not logged in.")
	end
end

-- NOT to be confused with SystemReply!!  For replying to admtell messages.
local function Reply(ply, argc, argv)
	if argc < 2 then
		SystemReply(ply, "^3Syntax: /Reply <message>")
		return
	end

	if ply.lastadmtell == nil then
		ply:SendChat( "^1You have not been messaged by an admin in this play session.  Use /Tell <player name> <message> to msg a specific player." )
		return
	end

	local account = ply.lastadmtell
	local k = 0
	local message = table.concat(argv," ",1, argc-1)
	while players.GetByID(k) ~= nil do
		local plytarg = players.GetByID(k)

		if( account == plytarg:GetAccount() and plytarg ~= ply ) then
			plytarg:SendChat( "^7" .. ply.Name .. "^5 replies: ^7" .. message )
		end

		k = k + 1
	end

	ply:SendChat( "^5You reply: ^7" .. message )
end

local function Say(ply, argc, argv)
	if ply.isLoggedIn then
		local rank = GetRank(ply)
		if rank["can-say"] ~= true then
			SystemReply(ply, "^1You do not have permission to perform this action.")
			return
		end

		if argc < 2 then
			SystemReply(ply, "^3Syntax: /admsay <message>")
			return
		end

		local k = 0
		local message = table.concat(argv," ",1, argc-1)
		local ourAccount = accounts[ply:GetAccount()]
		local normalRadius = 1281 --how far away other players can hear us
		local plyVec = vectorfnc.ObtainPlyVector(ply)

		while players.GetByID(k) ~= nil do
            local plytarg = players.GetByID(k)
            local vecResult = vectorfnc.VectorSubtract(plyVec, vectorfnc.ObtainPlyVector(plytarg))       --subtract player's origin from target's origin
            if vectorfnc.VectorLength(vecResult) < normalRadius then      --if the length is inside the radius
                local fadelevel = vectorfnc.GetChatFadeLevel(vectorfnc.VectorLength(vecResult), normalRadius)
				plytarg:SendFadedChat(fadelevel, "^7[^x39cAdmin^7] ^5" .. ply:GetName() .. "^7: " .. message )
					--ourAccount["username"] --consider using actual account name?
			end
			k = k + 1
		end
		
	else
		SystemReply(ply, "^1You are not logged in.")
	end
end

local function Announce(ply, argc, argv)
	if ply.isLoggedIn then
		local rank = GetRank(ply)
		if rank["can-announce"] ~= true then
			SystemReply(ply, "^1You do not have permission to perform this action.")
			return
		end

		if argc < 2 then
			SystemReply(ply, "^3Syntax: /admannounce <message>")
			return
		end

		local message = table.concat(argv," ",1, argc-1)
		local len = string.len(message)
		
		if  len > 75 then
			SystemReply(ply, "^3Syntax: Announcements must be 75 characters or less.")
			return
		end

		chatmsg( "^7System: " .. message )
		ply:SendCenterPrintAll(message)

	else
		SystemReply(ply, "^1You are not logged in.")
	end
end

local function Puppet(ply, argc, argv)
	-- EVIL... we can mimic the actions of other people.
	-- Not for regular use, mostly for test purposes but some admins might find this humorous..

	if ply.isLoggedIn then
		local rank = GetRank(ply)
		if rank["can-puppet"] ~= true then
			SystemReply(ply, "^1You do not have permission to perform this action.")
			return
		end

		if argc < 3 then
			SystemReply(ply, "^3Syntax: /admpuppet <player> <channel> <message> -OR-")
			SystemReply(ply, "/admpuppet <player> <message>")
			return
		end

		local plytarg = players.GetByArg(argv[1])
		if plytarg == nil then
			SystemReply(ply, "^1Invalid player specified.")
			return
		end

		local channeltarg = argv[2]
		local targetchan = 0

		if channeltarg == "global" then
			targetchan = 1
		elseif channeltarg == "yell" then
			targetchan = 2
		elseif channeltarg == "emote" then
			targetchan = 3
		elseif channeltarg == "me" then
			targetchan = 3
		elseif channeltarg == "action" then
			targetchan = 3
		elseif channeltarg == "team" then
			targetchan = 4
		end
		
		local message
		if targetchan == 0 then
			message = table.concat(argv," ",2, argc-1)
		else
			message = table.concat(argv," ",3, argc-1)
		end

		if targetchan == 0 then
			plytarg:ExecuteChatCommand("say " .. message)
		elseif targetchan == 1 then
			plytarg:ExecuteChatCommand("sayglobal " .. message)
		elseif targetchan == 2 then
			plytarg:ExecuteChatCommand("sayyell " .. message)
		elseif targetchan == 3 then
			plytarg:ExecuteChatCommand("sayact " .. message)
		elseif targetchan == 4 then
			plytarg:ExecuteChatCommand("say_team " .. message)
		else
			SystemReply(ply, "^1ERROR: This wasn't supposed to happen...abort, abort!")
			return
		end
	else
		SystemReply(ply, "^1You are not logged in.")
	end
end

local function ChangeMap(ply, argc, argv)
	if ply.isLoggedIn then
		local rank = GetRank(ply)
		if rank["can-changemap"] ~= true then
			SystemReply(ply, "^1You do not have permission to perform this action.")
			return
		end

		if (argc < 2 or argc > 2) then
			SystemReply(ply, "/admchangemap <mapname>")
			return
		end

		local maptarg = argv[1];
		if maptarg == nil then
			SystemReply(ply, "^1Invalid map specified.")
			return
		end

		--SystemReply(ply, "Attempting to load map...")
		


		--get a list of all maps on the server in an array
		--local maplist = ply:GetMapList() --maplist, cmd currently doesn't work

		--local maps = {} --array to hold list of maps
		--for k in (maplist .. ";"):gmatch("([^,]*),") do 
			--table.insert(maps, k) 
		--end

		--for i,v in ipairs(maps) do 
			--SystemReply(ply, v) --list all maps
		--end
		
		-- Loop through the maps and make sure none match
		--local k
		--for k = 0, map_size-1 do
			--if maps[k] == maptarg then --found our map
				--SystemReply(ply, "^2Changing map...")
				--send server map cmd
				--return
			--end
		--end

		SystemReply(ply, "^1This feature is not yet complete.")
		return

	else
		SystemReply(ply, "^1You are not logged in.")
	end
end

local function teleport(ply, argc, argv)
	if ply.isLoggedIn then
		local rank = GetRank(ply)
		if rank["can-teleport"] ~= true then
			SystemReply(ply, "^1You do not have permission to perform this action.")
			return
		end

		--teleport to your cursor
		if argc < 2 then
			local trace = ply:GetEyeTrace()
			ply:Teleport(trace.EndPos + trace.HitNormal * 25)
		else
			local target = players.GetByArg(argv[1])
			if not target then
				SystemReply(ply,"^1Target player not found")
				return
			end
			if argc == 2 then
				local trace = ply:GetEyeTrace()
				target:Teleport(trace.EndPos + trace.HitNormal * 25)
				return
			end

			if argc > 2 then
				local destination = {}
				local request = table.concat(argv," ",2, argc-1)

				for i in (request .. " "):gmatch("%S+") do
					if(tonumber(i)) then --make sure it can convert to a number
						table.insert(destination, tonumber(i))
					else
						SystemReply(ply, "^3Syntax: /teleport - self to cursor, /teleport <playername/clientnumber> - target to cursor, /teleport <playername/clientnumber> <x> <y> <z> <yaw> - target to coords")
						return
					end
				end

				if tablelength(destination) == 4 then --check to make sure table is appropriate length
					target:SetPos(Vector( tostring(destination[1]) .. " " .. tostring(destination[2]) .. " " .. tostring(destination[3]) ))
					target:SetAngles(Vector("0 " .. tostring(destination[4]) .. " 0"))
					return
				end

				SystemReply(ply, "^3Syntax: /teleport - self to cursor, /teleport <playername/clientnumber> - target to cursor, /teleport <playername/clientnumber> <x> <y> <z> <yaw> - target to coords")
				return
			end
		end
	else
		SystemReply(ply, "^1You are not logged in.")
	end
end

local function setclip(ply, argc, argv)
	if ply.isLoggedIn then
		local rank = GetRank(ply)
		if rank["can-setclip"] ~= true then
			SystemReply(ply, "^1You do not have permission to perform this action.")
			return
		end
		
		--toggle noclip on our self
		if argc < 2 then
			ply.NoClip = !ply.NoClip
		else
			local target = players.GetByArg(argv[1])
			if not target then
				SystemReply(ply, "^1Invalid player specified")
				return
			else --toggle noclip on specified player
				target.NoClip = !target.NoClip
			end
		end
	else
		SystemReply(ply, "^1You are not logged in.")
	end
end

local function usetarg(ply, argc, argv)
	if ply.isLoggedIn then
		local rank = GetRank(ply)
		if rank["can-usetarg"] ~= true then
			SystemReply(ply, "^1You do not have permission to perform this action.")
			return
		end

		if argc < 2 then
			SystemReply(ply, "^3Please specify a target.")
		else
			local target = argv[1]
			local plyent = ply:GetEntity()
			local entlist = ents.GetByName(target)
			local k,v
			for k,v in pairs(entlist) do
				v:Use(plyent, plyent)
			end
		end
	else
		SystemReply(ply, "^1You are not logged in.")
	end
end
--if not ply.IsAdmin then  --old way of verifying admins?


local function Help(ply, argc, argv)
	if ply.isLoggedIn then
		local printstring = ""
		SystemReply(ply, "See console for cmdlist.")
		ply:SendPrint("Commands:")
		printstring = "^3login ^7- allows user to sign into an account.\n^3logout ^7- signs out of the current account.\n^3changepassword ^7- change current account password.\n^3register ^7- register for a new client account.\n"
		printstring = printstring .. "^3admkick ^7- kick a user off the server.\n^3admchangedetails ^7- edit account details.\n^3admprofile ^7- check what account you're logged in with.\n^3admnewaccount ^7- create a new account as an admin.\n"
		printstring = printstring .. "^3admremoveaccount ^7- delete an account as an admin.\n^3admlist ^7- query for info about accounts.\n^3admrank ^7- query for permission held by a rank\n^3admalter ^7- alter an accounts rank or password.\n"
		printstring = printstring .. "^3admstatus ^7- list logged in users and status.\n^3admsay ^7- imitates the /say cmd for admins.\n^3admtell ^7- imitates the /tell cmd for admins.\n^3admspeak ^7- imitates the /sayglobal cmd for admins."
		ply:SendPrint(printstring) 	 --getting too long, gotta break it up
		printstring = "^3admannounce ^7- make a server announcement.\n^3admpuppet ^7- allows admins to masquerade messages as other users.\n^3admchangemap ^7- change the current map (not implemented).\n"
		printstring = printstring .. "^3teleport ^7- teleport to location or send player to location.\n^3toggleclip ^7- toggle noclip on/off self or specified player.\n^3admhelp^7/^3admcmds ^7- list admin cmds."
		ply:SendPrint(printstring)
	else
		SystemReply(ply, "^1You are not logged in.  ^7To login, use the login cmd. eg: ^3/login <username> <password>")
	end	
end

--[[ ------------------------------------------------
	InitAccountCmds
	This is what links all the commands into chat/console.
--------------------------------------------------]]

local function InitAccountCmds()
	chatcmds.Add("login", Login)
	chatcmds.Add("logout", Logout)
	chatcmds.Add("changepassword", ChangePassword)
	chatcmds.Add("register", Register)
	chatcmds.Add("admkick", Kick)
	chatcmds.Add("admchangedetails", ChangeDetails)
	chatcmds.Add("admprofile", Profile)
	chatcmds.Add("admnewaccount", CreateAccount)
	chatcmds.Add("admremoveaccount", RemoveAccount)
	chatcmds.Add("admlist", List)
	chatcmds.Add("admrank", Rank)
	chatcmds.Add("admalter", Alter)
	chatcmds.Add("admstatus", Status)
	chatcmds.Add("admspeak", Speak)
	chatcmds.Add("admtell", Tell)
	chatcmds.Add("admsay", Say)
	chatcmds.Add("admannounce", Announce)
	chatcmds.Add("admpuppet", Puppet)
	chatcmds.Add("admchangemap", ChangeMap)
	chatcmds.Add("teleport", teleport)
	chatcmds.Add("toggleclip", setclip)
	chatcmds.Add("usetarg", usetarg)
	chatcmds.Add("admhelp", Help)
	chatcmds.Add("admcmds", Help)

	-- For replying to /admtells..this isn't an admin command, it can be used by any client
	chatcmds.Add("Reply", Reply)

	-- Add them to the console too.
	cmds.Add("login", Login)
	cmds.Add("logout", Logout)
	cmds.Add("changepassword", ChangePassword)
	cmds.Add("register", Register)
	cmds.Add("admkick", Kick)
	cmds.Add("admchangedetails", ChangeDetails)
	cmds.Add("admprofile", Profile)
	cmds.Add("admnewaccount", CreateAccount)
	cmds.Add("admremoveaccount", RemoveAccount)
	cmds.Add("admlist", List)
	cmds.Add("admrank", Rank)
	cmds.Add("admalter", Alter)
	cmds.Add("admstatus", Status)
	cmds.Add("admspeak", Speak)
	cmds.Add("admtell", Tell)
	cmds.Add("admsay", Say)
	cmds.Add("admannounce", Announce)
	cmds.Add("admpuppet", Puppet)
	cmds.Add("admchangemap", ChangeMap)
	cmds.Add("teleport", teleport)
	cmds.Add("toggleclip", setclip)
	cmds.Add("usetarg", usetarg)
	cmds.Add("admhelp", Help)
	cmds.Add("admcmds", Help)
	

	-- For replying to /admtells..this isn't an admin command, it can be used by any client
	cmds.Add("Reply", Reply)
end

--[[ ------------------------------------------------
	Init
--------------------------------------------------]]

local function InitAccountSys()
	InitAccounts()
	InitAccountCmds()
end

InitAccountSys()
