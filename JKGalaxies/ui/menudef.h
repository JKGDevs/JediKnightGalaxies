// menudef.h
// Copyright (c) 2013 Jedi Knight Galaxies

#define CT_LTBLUE1	0.367	0.261	0.722
#define CT_DKBLUE1	0.199	0.0		0.398

#define CT_LTCYAN	0		0.5		0.5
#define CT_DKCYAN	0		0.25	0.25

#define ITEM_TYPE_TEXT 0                  // simple text
#define ITEM_TYPE_BUTTON 1                // button, basically text with a border 
#define ITEM_TYPE_RADIOBUTTON 2           // toggle button, may be grouped 
#define ITEM_TYPE_CHECKBOX 3              // check box
#define ITEM_TYPE_EDITFIELD 4             // editable text, associated with a cvar
#define ITEM_TYPE_COMBO 5                 // drop down list
#define ITEM_TYPE_LISTBOX		6		  // scrollable list  
#define ITEM_TYPE_MODEL			7         // model
#define ITEM_TYPE_OWNERDRAW		8         // owner draw, name specs what it is
#define ITEM_TYPE_NUMERICFIELD	9         // editable text, associated with a cvar
#define ITEM_TYPE_SLIDER		10        // mouse speed, volume, etc.
#define ITEM_TYPE_YESNO			11        // yes no cvar setting
#define ITEM_TYPE_MULTI			12        // multiple list setting, enumerated
#define ITEM_TYPE_BIND			13		  // multiple list setting, enumerated
#define ITEM_TYPE_TEXTSCROLL	14		  // scrolls text
#define ITEM_TYPE_BIND_FIXED	15		  // Jedi Knight Galaxies: fixed bins for controls menu (saves space in the .menu file)
    
#define ITEM_ALIGN_LEFT 0                 // left alignment
#define ITEM_ALIGN_CENTER 1               // center alignment
#define ITEM_ALIGN_RIGHT 2                // right alignment

#define ITEM_TEXTSTYLE_NORMAL 0           // normal text
#define ITEM_TEXTSTYLE_BLINK 1            // fast blinking
#define ITEM_TEXTSTYLE_PULSE 2            // slow pulsing
#define ITEM_TEXTSTYLE_SHADOWED 3         // drop shadow ( need a color for this )
#define ITEM_TEXTSTYLE_OUTLINED 4         // drop shadow ( need a color for this )
#define ITEM_TEXTSTYLE_OUTLINESHADOWED 5  // drop shadow ( need a color for this )
#define ITEM_TEXTSTYLE_SHADOWEDMORE 6         // drop shadow ( need a color for this )
                          
#define WINDOW_BORDER_NONE 0              // no border
#define WINDOW_BORDER_FULL 1              // full border based on border color ( single pixel )
#define WINDOW_BORDER_HORZ 2              // horizontal borders only
#define WINDOW_BORDER_VERT 3              // vertical borders only 
#define WINDOW_BORDER_KCGRADIENT 4        // horizontal border using the gradient bars
  
#define WINDOW_STYLE_EMPTY 0              // no background
#define WINDOW_STYLE_FILLED 1             // filled with background color
#define WINDOW_STYLE_GRADIENT 2           // gradient bar based on background color 
#define WINDOW_STYLE_SHADER   3           // gradient bar based on background color 
#define WINDOW_STYLE_TEAMCOLOR 4          // team color
#define WINDOW_STYLE_CINEMATIC 5          // cinematic

#define MENU_TRUE 1                       // uh.. true
#define MENU_FALSE 0                      // and false

#define HUD_VERTICAL				0x00
#define HUD_HORIZONTAL				0x01

// list box element types
#define LISTBOX_TEXT  0x00
#define LISTBOX_IMAGE 0x01

// list feeders
//#define FEEDER_HEADS						0x00			// model heads
#define FEEDER_MAPS							0x01			// text maps based on game type
#define FEEDER_SERVERS						0x02			// servers
//#define FEEDER_CLANS						0x03			// clan names
#define FEEDER_ALLMAPS						0x04			// all maps available, in graphic format
#define FEEDER_REDTEAM_LIST					0x05			// red team members
#define FEEDER_BLUETEAM_LIST				0x06			// blue team members
#define FEEDER_PLAYER_LIST					0x07			// players
#define FEEDER_TEAM_LIST					0x08			// team members for team voting
#define FEEDER_MODS							0x09			// team members for team voting
#define FEEDER_DEMOS 						0x0a			// team members for team voting
#define FEEDER_SCOREBOARD					0x0b			// team members for team voting
#define FEEDER_Q3HEADS		 				0x0c			// model heads
#define FEEDER_SERVERSTATUS					0x0d			// server status
#define FEEDER_FINDPLAYER					0x0e			// find player
#define FEEDER_CINEMATICS					0x0f			// cinematics

#define FEEDER_FORCECFG						0x10			// force config list

#define FEEDER_SIEGE_TEAM1					0x11			// siege class list for team1
#define FEEDER_SIEGE_TEAM2					0x12			// siege class list for team2

#define FEEDER_PLAYER_SPECIES				0x13			// models/player/*
#define FEEDER_PLAYER_SKIN_HEAD				0x14			// head*.skin files in species folder
#define FEEDER_PLAYER_SKIN_TORSO			0x15			// torso*.skin files in species folder
#define FEEDER_PLAYER_SKIN_LEGS				0x16			// lower*.skin files in species folder
#define FEEDER_COLORCHOICES					0x17			// special hack to feed text/actions from playerchoice.txt in species folder

#define FEEDER_TEAM1_INFANTRY				0x18			// for siege team choice
#define FEEDER_TEAM1_VANGUARD				0x19			// for siege team choice
#define FEEDER_TEAM1_SUPPORT				0x1a			// for siege team choice
#define FEEDER_TEAM1_JEDI					0x1b			// for siege team choice
#define FEEDER_TEAM1_DEMO					0x1c			// for siege team choice
#define FEEDER_TEAM1_HEAVY					0x1d			// for siege team choice

#define FEEDER_TEAM2_INFANTRY				0x1e			// for siege team choice
#define FEEDER_TEAM2_VANGUARD				0x1f			// for siege team choice
#define FEEDER_TEAM2_SUPPORT				0x20			// for siege team choice
#define FEEDER_TEAM2_JEDI					0x21			// for siege team choice
#define FEEDER_TEAM2_DEMO					0x22			// for siege team choice
#define FEEDER_TEAM2_HEAVY					0x23			// for siege team choice

#define FEEDER_SIEGE_BASE_CLASS				0x24			// for siege team choice
#define FEEDER_SIEGE_CLASS_WEAPONS			0x25			// for siege team choice
#define FEEDER_SIEGE_CLASS_INVENTORY		0x26			// for siege team choice
#define FEEDER_SIEGE_CLASS_FORCE			0x27			// for siege team choice
#define FEEDER_LANGUAGES					0x28			// for language choice
#define FEEDER_MOVES						0x29			// moves for the data pad moves screen
#define FEEDER_MOVES_TITLES					0x2a			// move titles for the data pad moves screen
#define FEEDER_SABER_SINGLE_INFO			0x2b			// saber single
#define FEEDER_SABER_STAFF_INFO				0x2c			// saber staff

// JKG - Feeders
#define FEEDER_PARTYMEMBERSINVITES	0x100
#define FEEDER_PARTYSEEKERS			0x101

#define FEEDER_SLICEPROGRAMS		0x102
#define FEEDER_INVENTORY            0x103
#define FEEDER_SHOP					0x104

// display flags
#define CG_SHOW_BLUE_TEAM_HAS_REDFLAG     0x00000001
#define CG_SHOW_RED_TEAM_HAS_BLUEFLAG     0x00000002
#define CG_SHOW_ANYTEAMGAME               0x00000004
#define CG_SHOW_HARVESTER                 0x00000008
#define CG_SHOW_ONEFLAG                   0x00000010
#define CG_SHOW_CTF                       0x00000020
#define CG_SHOW_OBELISK                   0x00000040
#define CG_SHOW_HEALTHCRITICAL            0x00000080
#define CG_SHOW_SINGLEPLAYER              0x00000100
#define CG_SHOW_TOURNAMENT                0x00000200
#define CG_SHOW_DURINGINCOMINGVOICE       0x00000400
#define CG_SHOW_IF_PLAYER_HAS_FLAG				0x00000800
#define CG_SHOW_LANPLAYONLY								0x00001000
#define CG_SHOW_MINED											0x00002000
#define CG_SHOW_HEALTHOK			            0x00004000
#define CG_SHOW_TEAMINFO			            0x00008000
#define CG_SHOW_NOTEAMINFO		            0x00010000
#define CG_SHOW_OTHERTEAMHASFLAG          0x00020000
#define CG_SHOW_YOURTEAMHASENEMYFLAG      0x00040000
#define CG_SHOW_ANYNONTEAMGAME            0x00080000
#define CG_SHOW_2DONLY										0x10000000


#define UI_SHOW_LEADER				            0x00000001
#define UI_SHOW_NOTLEADER			            0x00000002
#define UI_SHOW_FAVORITESERVERS					0x00000004
#define UI_SHOW_ANYNONTEAMGAME					0x00000008
#define UI_SHOW_ANYTEAMGAME						0x00000010
#define UI_SHOW_NEWHIGHSCORE					0x00000020
#define UI_SHOW_DEMOAVAILABLE					0x00000040
#define UI_SHOW_NEWBESTTIME						0x00000080
#define UI_SHOW_FFA								0x00000100
#define UI_SHOW_NOTFFA							0x00000200
#define UI_SHOW_NETANYNONTEAMGAME	 			0x00000400
#define UI_SHOW_NETANYTEAMGAME		 			0x00000800
#define UI_SHOW_NOTFAVORITESERVERS				0x00001000
// Jedi Knight Galaxies
#define UI_SHOW_INVENTORY_1						0x00002000
#define UI_SHOW_INVENTORY_2						0x00004000
#define UI_SHOW_INVENTORY_3						0x00008000



// owner draw types
// ideally these should be done outside of this file but
// this makes it much easier for the macro expansion to 
// convert them for the designers ( from the .menu files )
#define CG_OWNERDRAW_BASE 1
#define CG_PLAYER_ARMOR_ICON 1              
#define CG_PLAYER_ARMOR_VALUE 2
#define CG_PLAYER_HEAD 3
#define CG_PLAYER_HEALTH 4
#define CG_PLAYER_AMMO_ICON 5
#define CG_PLAYER_AMMO_VALUE 6
#define CG_SELECTEDPLAYER_HEAD 7
#define CG_SELECTEDPLAYER_NAME 8
#define CG_SELECTEDPLAYER_LOCATION 9
#define CG_SELECTEDPLAYER_STATUS 10
#define CG_SELECTEDPLAYER_WEAPON 11
#define CG_SELECTEDPLAYER_POWERUP 12

#define CG_FLAGCARRIER_HEAD 13
#define CG_FLAGCARRIER_NAME 14
#define CG_FLAGCARRIER_LOCATION 15
#define CG_FLAGCARRIER_STATUS 16
#define CG_FLAGCARRIER_WEAPON 17
#define CG_FLAGCARRIER_POWERUP 18

#define CG_PLAYER_ITEM 19
#define CG_PLAYER_SCORE 20

#define CG_BLUE_FLAGHEAD 21
#define CG_BLUE_FLAGSTATUS 22
#define CG_BLUE_FLAGNAME 23
#define CG_RED_FLAGHEAD 24
#define CG_RED_FLAGSTATUS 25
#define CG_RED_FLAGNAME 26

#define CG_BLUE_SCORE 27
#define CG_RED_SCORE 28
#define CG_RED_NAME 29
#define CG_BLUE_NAME 30
#define CG_HARVESTER_SKULLS 31					// only shows in harvester
#define CG_ONEFLAG_STATUS 32						// only shows in one flag
#define CG_PLAYER_LOCATION 33
#define CG_TEAM_COLOR 34
#define CG_CTF_POWERUP 35
                                        
#define CG_AREA_POWERUP	36
#define CG_AREA_LAGOMETER	37            // painted with old system
#define CG_PLAYER_HASFLAG 38            
#define CG_GAME_TYPE 39                 // not done

#define CG_SELECTEDPLAYER_ARMOR 40      
#define CG_SELECTEDPLAYER_HEALTH 41
#define CG_PLAYER_STATUS 42
#define CG_FRAGGED_MSG 43               // painted with old system
#define CG_PROXMINED_MSG 44             // painted with old system
#define CG_AREA_FPSINFO 45              // painted with old system
#define CG_AREA_SYSTEMCHAT 46           // painted with old system
#define CG_AREA_TEAMCHAT 47             // painted with old system
#define CG_AREA_CHAT 48                 // painted with old system
#define CG_GAME_STATUS 49
#define CG_KILLER 50
#define CG_PLAYER_ARMOR_ICON2D 51              
#define CG_PLAYER_AMMO_ICON2D 52
#define CG_ACCURACY 53
#define CG_ASSISTS 54
#define CG_DEFEND 55
#define CG_EXCELLENT 56
#define CG_IMPRESSIVE 57
#define CG_PERFECT 58
#define CG_GAUNTLET 59
#define CG_SPECTATORS 60
#define CG_TEAMINFO 61
#define CG_VOICE_HEAD 62
#define CG_VOICE_NAME 63
#define CG_PLAYER_HASFLAG2D 64            
#define CG_HARVESTER_SKULLS2D 65					// only shows in harvester
#define CG_CAPFRAGLIMIT 66	 
#define CG_1STPLACE 67
#define CG_2NDPLACE 68
#define CG_CAPTURES 69
#define CG_PLAYER_FORCE_VALUE 70




#define UI_OWNERDRAW_BASE 200
//#define UI_HANDICAP 200
#define UI_EFFECTS 201
#define UI_GAMETYPE 205
//#define UI_SKILL 207
#define UI_BLUETEAMNAME 208
#define UI_REDTEAMNAME 209
#define UI_BLUETEAM1 210
#define UI_BLUETEAM2 211
#define UI_BLUETEAM3 212
#define UI_BLUETEAM4 213
#define UI_BLUETEAM5 214
#define UI_REDTEAM1 215
#define UI_REDTEAM2 216
#define UI_REDTEAM3 217
#define UI_REDTEAM4 218
#define UI_REDTEAM5 219
#define UI_NETSOURCE 220
#define UI_NETMAPPREVIEW 221
#define UI_NETFILTER 222
#define UI_ALLMAPS_SELECTION 236
#define UI_OPPONENT_NAME 237
#define UI_VOTE_KICK 238
#define UI_BOTNAME 239
#define UI_BOTSKILL 240
#define UI_REDBLUE 241
#define UI_CROSSHAIR 242
#define UI_SELECTEDPLAYER 243
#define UI_NETGAMETYPE 245
#define UI_SERVERREFRESHDATE 247
#define UI_SERVERMOTD 248
#define UI_GLINFO  249
#define UI_KEYBINDSTATUS 250
#define UI_JOINGAMETYPE 253
#define UI_MAPS_SELECTION 256
#define UI_VERSION					277
#define UI_TOTALFORCESTARS			278
#define UI_AUTOSWITCHLIST			279

// Jedi Knight Galaxies
// Convo defines
#define UI_JKG_CONVO_LASTTEXT		1000
#define UI_JKG_CONVO_TEXT			1001
#define UI_JKG_CONVO_OPT1			1002
#define UI_JKG_CONVO_OPT2			1003
#define UI_JKG_CONVO_OPT3			1004
#define UI_JKG_CONVO_OPT4			1005
#define UI_JKG_CONVO_GOUP			1006
#define UI_JKG_CONVO_GODOWN			1007
#define UI_JKG_CONVO_KEYHANDLER		1008
#define UI_JKG_CONVO_TECAPTION		1009

// Pazaak defines
#define UI_JKG_PAZAAK_BCARD			1010
#define UI_JKG_PAZAAK_BHAND			1011
#define UI_JKG_PAZAAK_NAMES			1012
#define UI_JKG_PAZAAK_POINTS		1013
#define UI_JKG_PAZAAK_DLGTEXT		1014
#define UI_JKG_PAZAAK_TIMEOUT		1015
#define UI_JKG_PAZAAK_SELCARD		1016
#define UI_JKG_PAZAAK_SELSD			1017
#define UI_JKG_PAZAAK_WAITING		1018


// Team Management defines
#define UI_PARTYMNGT_DLGTEXT		1019

// Slicing defines
#define UI_JKG_SLICE_GRID			1030
#define UI_JKG_SLICE_SUMMARY		1031
#define	UI_JKG_SLICE_CLEARANCE		1032
#define	UI_JKG_SLICE_WARNING		1033
#define	UI_JKG_SLICE_INTRUSION		1034
#define	UI_JKG_SLICE_DLGTEXT		1035

// Loot menu
#define UI_JKG_LOOT_ITEM		1036
#define UI_JKG_LOOT_NAME		1037
#define UI_JKG_LOOT_ICON		1038
#define UI_JKG_LOOT_DESC		1039
#define UI_JKG_LOOT_WEIG		1040
#define UI_JKG_LOOT_DLG			1041

//#define UI_JKG_INV_TOOLTIP		1042

#define UI_JKG_TEAMRED			1043
#define UI_JKG_TEAMBLUE			1044

//Refactored tooltip code
#define UI_JKG_TOOLTIP			1045

// Inventory menu
#define UI_JKG_INV_ITEMICON		1046
#define UI_JKG_INVSEL_ICON		1047
#define UI_JKG_INV_ITEMNAME		1048
#define UI_JKG_INVSEL_NAME		1049
#define UI_JKG_INVSEL_DESC		1050
#define UI_JKG_INVSEL_STATS		1051

// Inventory common
#define UI_JKG_CREDITS			1052			// Also used for the shop menu
#define UI_JKG_WEIGHT			1053
#define UI_JKG_ITEM_ICON		1054
#define UI_JKG_ITEM_NAME		1055
#define UI_JKG_SEL_HIGHLIGHT	1056
#define UI_JKG_SEL_ITEMNAME		1057
#define UI_JKG_SEL_ITEMDESC		1058
#define UI_JKG_SEL_ITEMICON		1059
#define UI_JKG_ITEM_TTOP		1060
#define UI_JKG_ITEM_TBOTTOM		1061
#define UI_JKG_ITEM_INTERACT	1062

// Padding in case the inventory needs more ownerdraw

// Shop
#define UI_JKG_SHOP_LEFTICON	1083	// param: which icon
#define UI_JKG_SHOP_RIGHTICON	1084	// param: which icon
//#define UI_JKG_SHOP_LEFTARROW	1085	// param: up (qtrue), down (qfalse)
//#define UI_JKG_SHOP_RIGHTARROW	1086	// param: up (qtrue), down (qfalse)
#define UI_JKG_SHOP_LEFTTAB		1087	// param: which side of the menu (left = qtrue/right = qfalse)
#define UI_JKG_SHOP_RIGHTTAB	1088	// param: which side of the menu (left = qtrue/right = qfalse)
#define UI_JKG_SHOP_BUTTON		1089	// param: which button
#define UI_JKG_SHOP_NAMETAG		1090	// param: which side
#define UI_JKG_SHOP_LEFTSELECT	1091	// param: which selection
#define UI_JKG_SHOP_RIGHTSELECT	1092	// param: which selection
#define UI_JKG_SHOP_LEFTNAME	1093	// param: which item
#define UI_JKG_SHOP_RIGHTNAME	1094	// param: which item
#define UI_JKG_SHOP_LEFTPRICE	1095	// param: which item
#define UI_JKG_SHOP_RIGHTPRICE	1096	// param: which item
#define UI_JKG_SHOP_AMMOPRICE	1097	// no param
#define UI_JKG_SHOP_ITEMDESC	1098	// param: which line of text
#define UI_JKG_SHOP_REFRESHTIME 1099    // no param

// Padding in case the shop needs more ownerdraw

//How handy it would be if this were an enum.
#define UI_BLUETEAM6 280
#define UI_BLUETEAM7 281
#define UI_BLUETEAM8 282
#define UI_REDTEAM6 283
#define UI_REDTEAM7 284
#define UI_REDTEAM8 285

// Yes it would be handy
#define UI_SKIN_COLOR 287
#define UI_FORCE_POINTS 288

//extra, for patch
#define UI_JEDI_NONJEDI 289

#define UI_CHAT_MAIN		291
#define UI_CHAT_ATTACK		292
#define UI_CHAT_DEFEND		293
#define UI_CHAT_REQUEST		294
#define UI_CHAT_REPLY		295
#define UI_CHAT_SPOT		296
#define UI_CHAT_TACTICAL	297

#define VOICECHAT_GETFLAG			"getflag"				// command someone to get the flag
#define VOICECHAT_OFFENSE			"offense"				// command someone to go on offense
#define VOICECHAT_DEFEND			"defend"				// command someone to go on defense
#define VOICECHAT_DEFENDFLAG		"defendflag"			// command someone to defend the flag
#define VOICECHAT_PATROL			"patrol"				// command someone to go on patrol (roam)
#define VOICECHAT_CAMP				"camp"					// command someone to camp (we don't have sounds for this one)
#define VOICECHAT_FOLLOWME			"followme"				// command someone to follow you
#define VOICECHAT_RETURNFLAG		"returnflag"			// command someone to return our flag
#define VOICECHAT_FOLLOWFLAGCARRIER	"followflagcarrier"		// command someone to follow the flag carrier
#define VOICECHAT_YES				"yes"					// yes, affirmative, etc.
#define VOICECHAT_NO				"no"					// no, negative, etc.
#define VOICECHAT_ONGETFLAG			"ongetflag"				// I'm getting the flag
#define VOICECHAT_ONOFFENSE			"onoffense"				// I'm on offense
#define VOICECHAT_ONDEFENSE			"ondefense"				// I'm on defense
#define VOICECHAT_ONPATROL			"onpatrol"				// I'm on patrol (roaming)
#define VOICECHAT_ONCAMPING			"oncamp"				// I'm camping somewhere
#define VOICECHAT_ONFOLLOW			"onfollow"				// I'm following
#define VOICECHAT_ONFOLLOWCARRIER	"onfollowcarrier"		// I'm following the flag carrier
#define VOICECHAT_ONRETURNFLAG		"onreturnflag"			// I'm returning our flag
#define VOICECHAT_INPOSITION		"inposition"			// I'm in position
#define VOICECHAT_IHAVEFLAG			"ihaveflag"				// I have the flag
#define VOICECHAT_BASEATTACK		"baseattack"			// the base is under attack
#define VOICECHAT_ENEMYHASFLAG		"enemyhasflag"			// the enemy has our flag (CTF)
#define VOICECHAT_STARTLEADER		"startleader"			// I'm the leader
#define VOICECHAT_STOPLEADER		"stopleader"			// I resign leadership
#define VOICECHAT_TRASH				"trash"					// lots of trash talk
#define VOICECHAT_WHOISLEADER		"whoisleader"			// who is the team leader
#define VOICECHAT_WANTONDEFENSE		"wantondefense"			// I want to be on defense
#define VOICECHAT_WANTONOFFENSE		"wantonoffense"			// I want to be on offense
#define VOICECHAT_KILLINSULT		"kill_insult"			// I just killed you
#define VOICECHAT_TAUNT				"taunt"					// I want to taunt you
#define VOICECHAT_DEATHINSULT		"death_insult"			// you just killed me
#define VOICECHAT_KILLGAUNTLET		"kill_gauntlet"			// I just killed you with the gauntlet
#define VOICECHAT_PRAISE			"praise"				// you did something good
