////////////////////////////////////////////////////////////
//
// Jedi Knight Galaxies
//
// Party Management UI Module
//
// Written by BobaFett
//
////////////////////////////////////////////////////////////


#include "ui_local.h"

// UI includes
void Menu_ShowItemByName(menuDef_t *menu, const char *p, qboolean bShow);
itemDef_t *Menu_ClearFocus(menuDef_t *menu);
//
extern displayContextDef_t *DC;
int MenuFontToHandle(int iMenuFont);

// Team Management Datastruct
struct {
	int active;
	int inParty;
	int partyLead;					// Am i the leader of the current party?
	int partyListEntries;			// Amount of entries in the party/invite list (max 5)
	union {							// Union of 2 structs, so we dont waste space
		struct {					// Party members (inParty == 1)
			int id;					// Member ID
			int slotid;				// Slot ID
			char playerName[64];	// Name
			char playerStatus[64];	// Status (Leader, Member or Pending)
			char location[64];		// Location
			char playerClass[64];			// Class
		} PartyMembers[5];
		struct {					// Pending invitations (inParty == 0)
			int id;					// Party ID
			int slotid;				// Slot ID
			char leaderName[64];	// Leader name
			char memberCount[6];	// Members (number)
		} Invites[5];
	} Data;
	
	int partySeekers;	// Amount of people seeking a party
	int amSeeker;		// Am i in the seeker list?
	struct {
		int id;
		char playerName[64];
		char message[64];
		char playerClass[64];
	} partySeeker[MAX_CLIENTS];
	
	int seekerListPending;			// Still waiting for the list

	int partySelection;				// Selected index in party/invite list
	int seekerSelection;			// Selected index in seekers list

	int dlgid;						// Current dialog ID
} PMngtData;


// Party/Team defines (from cgame)
#define PARTY_SLOT_EMPTY	64
#define PARTY_SLOT_MEMBERS	 5
#define PARTY_SLOT_MAX		64
#define PARTY_SLOT_INVITES	 5

typedef struct
{

	char				id;								// The party identifier (reference only).
	char				leaderId;						// The leader identifier, used to lookup the name.
	char				memberCount;					// The current amount of members of this party.

} teamPartyInvites_t;

typedef struct
{

	char				id;								// This is the member identifier, used to lookup the name
	char				classId;						// This is the member class identifier so we can identify the class
	char				status;							// This is the status: -1 (Pending), 0 (Member) or 1 (Leader)

} teamPartyMember_t;

typedef struct
{

	char				active;							// Set to true when actively in a party.
	char				number;							// Party number we are in (or have been in last).
	teamPartyInvites_t	invites[PARTY_SLOT_INVITES];	// The pending invites to be displayed.
	teamPartyMember_t	members[PARTY_SLOT_MEMBERS];	// The members of the current (or the last) party.

} teamParty_t;

typedef struct
{

	unsigned int		 time;							// Time of the last update (even unregister gets this to send delta's!)
	int					 id;							// The player identifier.
	int					 classId;						// The player class identifier.
	char				 message[64];					// The message that has been set.

} teamPartyList_t;


static void JKG_PartyMngt_UpdateState() {
	// Updates team state (invites/team members)
	teamParty_t *party;
	uiClientState_t	cs;
	
	menuDef_t *menu;
	char buff[1024];
	int i;
	int j=0;
	menu = Menus_FindByName("jkg_partymanagement");
	if (!menu) {
		return;
	}
	trap->GetClientState( &cs );
	party = (teamParty_t *)cgImports->PartyMngtDataRequest(0);
	
	if (!party) {
		return;
	}

	if (party->active) {

		PMngtData.inParty = 1;
		PMngtData.partyLead = 0;

		for (i=0; i<PARTY_SLOT_MEMBERS; i++) {
			
			if (party->members[i].id == PARTY_SLOT_EMPTY) {
				continue;
			}

			PMngtData.Data.PartyMembers[j].slotid = i;
			PMngtData.Data.PartyMembers[j].id = party->members[i].id;

			if (party->members[i].id == cs.clientNum && party->members[i].status == 1) {
				PMngtData.partyLead = 1;
			}

			trap->GetConfigString(CS_PLAYERS + party->members[i].id, buff, sizeof(buff));
			Q_strncpyz(PMngtData.Data.PartyMembers[j].playerName, Info_ValueForKey(buff, "n"), 64);

			if (party->members[i].status == -1) {
				Q_strncpyz(PMngtData.Data.PartyMembers[j].playerStatus, "Pending", 64);
			} else if (party->members[i].status == 0) {
				Q_strncpyz(PMngtData.Data.PartyMembers[j].playerStatus, "Member", 64);
			} else if (party->members[i].status == 1) {
				Q_strncpyz(PMngtData.Data.PartyMembers[j].playerStatus, "Leader", 64);
			} else {
				Q_strncpyz(PMngtData.Data.PartyMembers[j].playerStatus, "Unknown", 64);
			}

			Q_strncpyz(PMngtData.Data.PartyMembers[j].location, "N/A", 64);
			Q_strncpyz(PMngtData.Data.PartyMembers[j].playerClass, "N/A", 64);
			j++;
		}
		PMngtData.partyListEntries = j;

		if (PMngtData.partyLead) {
			Menu_ShowItemByName(menu, "noparty", qfalse);
			Menu_ShowItemByName(menu, "btn_noparty", qfalse);
			Menu_ShowItemByName(menu, "inparty", qtrue);
			Menu_ShowItemByName(menu, "btn_inparty", qfalse);
			Menu_ShowItemByName(menu, "inpartylead", qtrue);
			Menu_ShowItemByName(menu, "btn_inpartylead", qtrue);
			Menu_ShowItemByName(menu, "btn_bottom", qfalse);
		} else {
			Menu_ShowItemByName(menu, "noparty", qfalse);
			Menu_ShowItemByName(menu, "btn_noparty", qfalse);
			Menu_ShowItemByName(menu, "inparty", qtrue);
			Menu_ShowItemByName(menu, "btn_inparty", qtrue);
			Menu_ShowItemByName(menu, "inpartylead", qfalse);
			Menu_ShowItemByName(menu, "btn_inpartylead", qfalse);
			Menu_ShowItemByName(menu, "btn_bottom", qfalse);
		}
	} else {

		PMngtData.inParty = 0;
		j=0;

		for (i=0; i<PARTY_SLOT_INVITES; i++) {
			if (party->invites[i].id == PARTY_SLOT_EMPTY) {
				continue;
			}

			PMngtData.Data.Invites[j].slotid = i;
			PMngtData.Data.Invites[j].id = party->invites[i].id;

			trap->GetConfigString(CS_PLAYERS + party->invites[i].leaderId, buff, sizeof(buff));
			Q_strncpyz(PMngtData.Data.Invites[j].leaderName, Info_ValueForKey(buff, "n"), 64);
			Com_sprintf(PMngtData.Data.Invites[j].memberCount, 6, "%i", party->invites[i].memberCount);

			j++;
		}

		PMngtData.partyListEntries = j;

		Menu_ShowItemByName(menu, "noparty", qtrue);
		Menu_ShowItemByName(menu, "btn_noparty", qtrue);
		Menu_ShowItemByName(menu, "inparty", qfalse);
		Menu_ShowItemByName(menu, "btn_inparty", qfalse);
		Menu_ShowItemByName(menu, "inpartylead", qfalse);
		Menu_ShowItemByName(menu, "btn_inpartylead", qfalse);
		Menu_ShowItemByName(menu, "btn_bottom", qfalse);

		if (PMngtData.amSeeker) {
			Menu_ShowItemByName(menu, "btn_removefromlist", qtrue);
		} else {
			Menu_ShowItemByName(menu, "btn_addtolist", qtrue);
		}
	}
}

static void JKG_PartyMngt_UpdateSeekers() {
	// Updates team state (invites/team members)
	teamPartyList_t *list;
	uiClientState_t	cs;
	
	menuDef_t *menu;
	char buff[1024];
	int i;
	int j=0;
	menu = Menus_FindByName("jkg_partymanagement");
	if (!menu) {
		return;
	}
	trap->GetClientState( &cs );
	list = (teamPartyList_t *)cgImports->PartyMngtDataRequest(1);

	if (!list) {
		return;
	}
	PMngtData.seekerListPending = 0;
	PMngtData.amSeeker = 0;

	for (i=0; i<PARTY_SLOT_MAX; i++) {
		if (list[i].id == PARTY_SLOT_EMPTY) {
			continue;
		}
		PMngtData.partySeeker[j].id = list[i].id;
		if (list[i].id == cs.clientNum) {
			PMngtData.amSeeker = 1;
		}
		
		trap->GetConfigString(CS_PLAYERS + list[i].id, buff, sizeof(buff));
		Q_strncpyz(PMngtData.partySeeker[j].playerName, Info_ValueForKey(buff, "n"), 64);
		Q_strncpyz(PMngtData.partySeeker[j].message, list[i].message, 64);
		Q_strncpyz(PMngtData.partySeeker[j].playerClass, "N/A", 64);
		j++;
	}
	PMngtData.partySeekers = j;

	if (!PMngtData.inParty) {
		// If we're in a party, the other function will have taken care of the button here
		// So only change it if we're not in a party

		Menu_ShowItemByName(menu, "btn_bottom", qfalse);

		if (PMngtData.amSeeker) {
			Menu_ShowItemByName(menu, "btn_removefromlist", qtrue);
		} else {
			Menu_ShowItemByName(menu, "btn_addtolist", qtrue);
		}
	}
}

void JKG_PartyMngt_UpdateNotify(int msg) {
	// Gets called from CGame if there's an update
	// CAUTION: To use trap/UI calls, use the syscall override first!
	// Msg 0: New team/invite information
	// Msg 1: New seekers information
	// Msg 10: Open the team manager
	if (msg == 0) {
		if (!PMngtData.active) {
			return;
		}
		JKG_PartyMngt_UpdateState();
	} else if (msg == 1) {
		if (!PMngtData.active) {
			return;
		}
		JKG_PartyMngt_UpdateSeekers();
	} else if (msg == 10) {
		memset(&PMngtData, 0, sizeof(PMngtData));
		PMngtData.active = 1;
		if (Menus_ActivateByName("jkg_partymanagement"))
		{
			trap->Key_SetCatcher( trap->Key_GetCatcher() | KEYCATCH_UI );
		}
	}
}

int JKG_PartyMngt_FeederCount(int feeder) {
	if (feeder == FEEDER_PARTYMEMBERSINVITES) {
		return PMngtData.partyListEntries;
	} else if (feeder == FEEDER_PARTYSEEKERS) {
		if (PMngtData.seekerListPending) {
			return 0;
		} else {
			return PMngtData.partySeekers;
		}
	}
	return 0;
}


qboolean JKG_PartyMngt_FeederSelection(int feeder, int index, itemDef_t *item) {
	if (feeder == FEEDER_PARTYMEMBERSINVITES) {
		PMngtData.partySelection = index;
	} else if (feeder == FEEDER_PARTYSEEKERS) {
		PMngtData.seekerSelection = index;
	}
	return qtrue;
}

const char *JKG_PartyMngt_FeederItemText(int feeder, int index, int column, qhandle_t *handle1, qhandle_t *handle2, qhandle_t *handle3) {
	if (feeder == FEEDER_PARTYMEMBERSINVITES) {
		if (index < 0 || index >= PMngtData.partyListEntries) {
			return NULL;
		}
		if (PMngtData.inParty) {
			switch (column) {
				case 0:
					return PMngtData.Data.PartyMembers[index].playerName;
				case 1:
					return PMngtData.Data.PartyMembers[index].playerStatus;
				case 2:
					return PMngtData.Data.PartyMembers[index].location;
				case 3:
					return PMngtData.Data.PartyMembers[index].playerClass;
				default:
					return NULL;
			}
		} else {
			switch (column) {
				case 0:
					return PMngtData.Data.Invites[index].leaderName;
				case 1:
					return PMngtData.Data.Invites[index].memberCount;
				default:
					return NULL;
			}
		}
	} else if (feeder == FEEDER_PARTYSEEKERS) {
		if (index < 0 || index >= PMngtData.partySeekers) {
			return NULL;
		}
		if (PMngtData.seekerListPending) {
			return NULL;
		}
		switch (column) {
			case 0:
				return PMngtData.partySeeker[index].playerName;
			case 1:
				return PMngtData.partySeeker[index].message;
			case 2:
				return PMngtData.partySeeker[index].playerClass;
			default:
				return NULL;
		}
	}
	return NULL;
}

typedef void (*PDlgCallback)(int response, const char *text);

enum {
	PDLG_OK,		// Ok only
	PDLG_YESNO,		// Yes/No
	PDLG_TEXT,		// Text entry with Ok/Cancel
};

enum {
	PDLGRESP_OK,
	PDLGRESP_YES,
	PDLGRESP_NO,
	PDLGRESP_CANCEL,
};

// Button ID's
enum {
	PBTN_STARTPARTY,
	PBTN_ACCEPTINVITE,
	PBTN_REJECTINVITE,
	PBTN_LEAVEPARTY,
	PBTN_DISBANDPARTY,
	PBTN_MAKELEADER,
	PBTN_DISMISSMEMBER,
	PBTN_ADDTOLIST,
	PBTN_REMOVEFROMLIST,
	PBTN_INVITE,
};

enum {
	PDLGID_ANY,
	PDLGID_NEWPARTY,
	PDLGID_ACCEPTINVITE,
	PDLGID_REJECTINVITE,
	PDLGID_LEAVEPARTY,
	PDLGID_DISBANDPARTY,
	PDLGID_MAKELEADER,
	PDLGID_DISMISSMEMBER,
	PDLGID_ADDTOLIST,
	PDLGID_REMOVEFROMLIST,
	PDLGID_INVITE,
};

// Pazaak Dialog Struct
static struct {
	int InUse;
	char line1[256];
	char line2[256];
	char line3[256];
	int type; // TDLG_xxx
	PDlgCallback callback;
} PDlgData;

// Open up the dialog window
// Line1, 2 and 3 are the text to show on the respective lines (can be NULL)
// Type is either TDLG_OK, TDLG_YESNO or TDLG_TEXT, depending on the desired buttons and input
// The callback is called when the user confirms the dialog, the response (TDLGRESP_xx) will be sent as arg

void Menu_ItemDisable(menuDef_t *menu, char *name,int disableFlag);


void Menu_SetTextFieldFocus(itemDef_t *item);

static void PartyMngt_Dialog_Show(const char *line1, const char *line2, const char *line3, int type, PDlgCallback callback) {
	menuDef_t *menu;
	itemDef_t *item;
	if (!callback) {
		return;
	}
	
	PDlgData.InUse = 1;

	if (line1) {
		Q_strncpyz(&PDlgData.line1[0], line1, 255);
	} else {
		PDlgData.line1[0] = 0;
	}

	if (line2) {
		Q_strncpyz(&PDlgData.line2[0], line2, 255);
	} else {
		PDlgData.line2[0] = 0;
	}

	if (line3) {
		Q_strncpyz(&PDlgData.line3[0], line3, 255);
	} else {
		PDlgData.line3[0] = 0;
	}

	PDlgData.type = type;
	PDlgData.callback = callback;

	
	menu = Menus_FindByName("jkg_partymanagement");

	if (!menu) {
		return;
	}

	// First, disable all controls on the interface
	Menu_ItemDisable(menu, "list", 1);
	Menu_ItemDisable(menu, "btn_noparty", 1);
	Menu_ItemDisable(menu, "btn_inparty", 1);
	Menu_ItemDisable(menu, "btn_inpartylead", 1);
	Menu_ItemDisable(menu, "btn_bottom", 1);
	
	Menu_ClearFocus(menu);

	Menu_ShowItemByName(menu, "dialog", qtrue);
	if (type == PDLG_OK) {
		Menu_ShowItemByName(menu, "btn_dialogok", qtrue);
	} else if (type == PDLG_YESNO) {
		Menu_ShowItemByName(menu, "btn_dialogyesno", qtrue);
	} else if (type == PDLG_TEXT) {
		Menu_ShowItemByName(menu, "btn_dialogtext", qtrue);
		// Clear the text
		item = Menu_FindItemByName(menu, "dlg_textentry");
		if (item) {
			item->typeData.edit->buffer[0] = 0;
			item->cursorPos = 0;
			Menu_SetTextFieldFocus(item);
		}
	} else {
		// Invalid dialog type!
		assert(0);
	}
}

static void PartyMngt_Dialog_Close() {
	menuDef_t *menu;

	PDlgData.InUse = 0;
	
	menu = Menus_FindByName("jkg_partymanagement");

	if (!menu) {
		return;
	}
	Menu_ShowItemByName(menu, "dialog", qfalse);
	Menu_ShowItemByName(menu, "btn_dialogok", qfalse);
	Menu_ShowItemByName(menu, "btn_dialogyesno", qfalse);
	Menu_ShowItemByName(menu, "btn_dialogtext", qfalse);
	
	Menu_ItemDisable(menu, "list", 0);
	Menu_ItemDisable(menu, "btn_noparty", 0);
	Menu_ItemDisable(menu, "btn_inparty", 0);
	Menu_ItemDisable(menu, "btn_inpartylead", 0);
	Menu_ItemDisable(menu, "btn_bottom", 0);

	Menu_ClearFocus(menu);
}

void PartyMngt_Script_DialogButton(char **args) {
	int button;
	if (!Int_Parse(args, &button)) {
		return;
	}
	if (button < PDLGRESP_OK || button > PDLGRESP_CANCEL) {
		// Invalid button
		return;
	}
	if (PDlgData.InUse) {
		if (PDlgData.type == PDLG_TEXT) {
			menuDef_t *menu;
			itemDef_t *item;
			menu = Menus_FindByName("jkg_partymanagement");
			if (menu) {
				item = Menu_FindItemByName(menu, "dlg_textentry");
				if (item) {
					(*PDlgData.callback)(button, item->typeData.edit->buffer);
					return;
				}
			}
			(*PDlgData.callback)(button, "");
		} else {
			(*PDlgData.callback)(button, NULL);
		}
	}
}

static const char *StripQuotes(char *str) {
	char *ostr = str;
	while (*str) {
		if (*str == '"') {
			*str = '\'';
		}
		str++;
	}
	return ostr;
}

static const char	*ConcatArgs( int start ) {
	int		i, c, tlen;
	static char	line[MAX_STRING_CHARS];
	int		len;
	char	arg[MAX_STRING_CHARS];

	len = 0;
	c = trap->Cmd_Argc();
	for ( i = start ; i < c ; i++ ) {
		trap->Cmd_Argv( i, arg, sizeof( arg ) );
		tlen = strlen( arg );
		if ( len + tlen >= MAX_STRING_CHARS - 1 ) {
			break;
		}
		memcpy( line + len, arg, tlen );
		len += tlen;
		if ( i != c - 1 ) {
			line[len] = ' ';
			len++;
		}
	}

	line[len] = 0;

	return line;
}

static void PartyMngt_Dlg_Response(int response, const char *text) {
	PartyMngt_Dialog_Close();
	switch (PMngtData.dlgid) {
		case PDLGID_NEWPARTY:
			if (response == PDLGRESP_YES) {
				cgImports->SendClientCommand("~pmngt partycreate");
			}
			break;
		case PDLGID_ACCEPTINVITE:
			if (response == PDLGRESP_YES) {
				cgImports->SendClientCommand(va("~pmngt partyaccept %i", PMngtData.partySelection));
			}
			break;
		case PDLGID_REJECTINVITE:
			if (response == PDLGRESP_YES) {
				cgImports->SendClientCommand(va("~pmngt partyreject %i", PMngtData.partySelection));
			}
			break;
		case PDLGID_LEAVEPARTY:
			if (response == PDLGRESP_YES) {
				cgImports->SendClientCommand("~pmngt partyleave");
			}
			break;
		case PDLGID_DISBANDPARTY:
			if (response == PDLGRESP_YES) {
				cgImports->SendClientCommand("~pmngt partydisband");
			}
			break;
		case PDLGID_MAKELEADER:
			if (response == PDLGRESP_YES) {
				cgImports->SendClientCommand(va("~pmngt partychangeleader %i", PMngtData.Data.PartyMembers[PMngtData.partySelection].slotid ));
			}
			break;
		case PDLGID_DISMISSMEMBER:
			if (response == PDLGRESP_YES) {
				cgImports->SendClientCommand(va("~pmngt partydismiss %i", PMngtData.Data.PartyMembers[PMngtData.partySelection].slotid ));
			}
			break;
		case PDLGID_ADDTOLIST:
			if (response == PDLGRESP_OK) {
				cgImports->SendClientCommand(va("~pmngt partylistregister \"%s\"", StripQuotes((char *)text)));
			}
			break;
		case PDLGID_REMOVEFROMLIST:
			if (response == PDLGRESP_YES) {
				cgImports->SendClientCommand("~pmngt partylistunregister");
			}
			break;
		case PDLGID_INVITE:
			if (response == PDLGRESP_YES) {
				cgImports->SendClientCommand(va("~pmngt partyinvite %i", PMngtData.partySeeker[PMngtData.seekerSelection].id));
			}
			break;
	}
}

void PartyMngt_ShowMessage_f() {
	// We received a pmr command
	PMngtData.dlgid = PDLGID_ANY;
	PartyMngt_Dialog_Show(ConcatArgs(1), NULL, NULL, PDLG_OK, PartyMngt_Dlg_Response);
}

void PartyMngt_Script_Button(char **args) {
	int button;
	uiClientState_t	cs;

	if (!Int_Parse(args, &button)) {
		return;
	}
	trap->GetClientState( &cs );
	switch (button) {
		case PBTN_STARTPARTY:
			if (PMngtData.inParty) {
				return;
			}
			PMngtData.dlgid = PDLGID_NEWPARTY;
			PartyMngt_Dialog_Show("Do you want to create a new party?", NULL, NULL, PDLG_YESNO, PartyMngt_Dlg_Response);
			break;
		case PBTN_ACCEPTINVITE:
			if (PMngtData.inParty) {
				return;
			}
			// Check if we got a valid slot selected
			if (PMngtData.partySelection < 0 || PMngtData.partySelection >= PMngtData.partyListEntries) {
				return;
			}
			PMngtData.dlgid = PDLGID_ACCEPTINVITE;
			PartyMngt_Dialog_Show(va("Do you want to join ^7%s^7's team?", PMngtData.Data.Invites[PMngtData.partySelection].leaderName), NULL, NULL, PDLG_YESNO, PartyMngt_Dlg_Response);
			break;
		case PBTN_REJECTINVITE:
			if (PMngtData.inParty) {
				return;
			}
			if (PMngtData.partySelection < 0 || PMngtData.partySelection >= PMngtData.partyListEntries) {
				return;
			}
			PMngtData.dlgid = PDLGID_REJECTINVITE;
			PartyMngt_Dialog_Show("Are you sure you want to reject this invitation?", NULL, NULL, PDLG_YESNO, PartyMngt_Dlg_Response);
			break;
		case PBTN_LEAVEPARTY:
			if (!PMngtData.inParty) {
				return;
			}
			PMngtData.dlgid = PDLGID_LEAVEPARTY;
			PartyMngt_Dialog_Show("You cannot return unless you get re-invited.", "Are you sure you want to leave the party?", NULL, PDLG_YESNO, PartyMngt_Dlg_Response);
			break;
		case PBTN_DISBANDPARTY:
			if (!PMngtData.inParty || !PMngtData.partyLead) {
				return;
			}
			PMngtData.dlgid = PDLGID_DISBANDPARTY;
			PartyMngt_Dialog_Show("Are you sure you want to disband your party?", NULL, NULL, PDLG_YESNO, PartyMngt_Dlg_Response);
			break;
		case PBTN_MAKELEADER:
			if (!PMngtData.inParty || !PMngtData.partyLead) {
				return;
			}
			// Check if we got a valid slot selected
			if (PMngtData.partySelection < 0 || PMngtData.partySelection >= PMngtData.partyListEntries) {
				return;
			}
			if (PMngtData.Data.PartyMembers[PMngtData.partySelection].id == cs.clientNum) {
				PMngtData.dlgid = PDLGID_ANY;
				PartyMngt_Dialog_Show("You are already this party's leader.", NULL, NULL, PDLG_OK, PartyMngt_Dlg_Response);
			} else {
				PMngtData.dlgid = PDLGID_MAKELEADER;
				PartyMngt_Dialog_Show("If you make someone else the party leader", "you will give up your own leader position.", "Are you sure you want to do this?", PDLG_YESNO, PartyMngt_Dlg_Response);
			}
			break;
		case PBTN_DISMISSMEMBER:
			if (!PMngtData.inParty || !PMngtData.partyLead) {
				return;
			}
			// Check if we got a valid slot selected
			if (PMngtData.partySelection < 0 || PMngtData.partySelection >= PMngtData.partyListEntries) {
				return;
			}
			if (PMngtData.Data.PartyMembers[PMngtData.partySelection].id == cs.clientNum) {
				PMngtData.dlgid = PDLGID_ANY;
				PartyMngt_Dialog_Show("You can't dismiss yourself.", NULL, NULL, PDLG_OK, PartyMngt_Dlg_Response);
			} else {
				PMngtData.dlgid = PDLGID_DISMISSMEMBER;
				PartyMngt_Dialog_Show(va("Are you sure you want to dismiss ^7%s^7?", PMngtData.Data.PartyMembers[PMngtData.partySelection].playerName), NULL, NULL, PDLG_YESNO, PartyMngt_Dlg_Response);
			}
			break;
		case PBTN_ADDTOLIST:
			if (PMngtData.inParty || PMngtData.amSeeker) {
				return;
			}
			PMngtData.dlgid = PDLGID_ADDTOLIST;
			PartyMngt_Dialog_Show("To get on the 'seeking a party' list, please", "type a message to show on the list.", NULL, PDLG_TEXT, PartyMngt_Dlg_Response);
			break;
		case PBTN_REMOVEFROMLIST:
			if (PMngtData.inParty || !PMngtData.amSeeker) {
				return;
			}
			PMngtData.dlgid = PDLGID_REMOVEFROMLIST;
			PartyMngt_Dialog_Show("Are you sure you want to remove yourself from the list?", NULL, NULL, PDLG_YESNO, PartyMngt_Dlg_Response);
			break;
		case PBTN_INVITE:
			if (!PMngtData.inParty || !PMngtData.partyLead) {
				return;
			}
			if (PMngtData.seekerSelection < 0 || PMngtData.seekerSelection >= PMngtData.partySeekers) {
				return;
			}
			PMngtData.dlgid = PDLGID_INVITE;
			PartyMngt_Dialog_Show(va("Do you want want to invite ^7%s^7?", PMngtData.partySeeker[PMngtData.seekerSelection].playerName), NULL, NULL, PDLG_YESNO, PartyMngt_Dlg_Response);
			break;
		default:
			break;
	}
}

void PartyMngt_Script_OpenDlg(char **args) {
	// We just opened the dialog, inform the server and request updates
	PartyMngt_Dialog_Close();
	cgImports->SendClientCommand("~pmngt on");
	JKG_PartyMngt_UpdateState();
	// Request a new list
	PMngtData.seekerListPending = 1;
	cgImports->SendClientCommand(va("~pmngt partylistrefresh %i", *(int*)cgImports->PartyMngtDataRequest(2)));
	trap->Cvar_Set("ui_hidehud", "1");
	PMngtData.active = 1;
}

void PartyMngt_Script_CloseDlg(char **args) {
	// Dialog got closed, inform the server
	cgImports->SendClientCommand("~pmngt off");
	trap->Cvar_Set("ui_hidehud", "0");
	PMngtData.active = 0;
}

void JKG_PartyMngt_DrawDialog(int line, float x, float y, float w, float h) {
	float width;
	const char *text;
	//vec4_t shadow;

	if (!PDlgData.InUse) {
		return;
	}
	if (line == 1) {
		text = PDlgData.line1;
	} else if (line == 2) {
		text = PDlgData.line2;
	} else {
		text = PDlgData.line3;
	}

	width = (float)trap->R_Font_StrLenPixels(text, MenuFontToHandle(1), 1) * 0.5f;

	x = x + ((w / 2) - (width / 2));
	//MAKERGBA(shadow,0,0,0,0.2f);
	//trap->R_Font_DrawString(	x+1, y+1, text, shadow, MenuFontToHandle(1), -1, 0.5f);
	trap->R_Font_DrawString(	x, y, text, colorWhite, MenuFontToHandle(1) | 0x80000000 , -1, 0.5f);
}