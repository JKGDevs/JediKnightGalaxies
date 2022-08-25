//////////////////////////////////////////
//
//	Jedi Knight Galaxies Player Isolation system
//	This system allows JKG to make players
//  non-existant to each other.
//
//  They cant see each other, they cant collide...nothing
//  The integral parts are controlled by an engine hook and trap call replacement
//  But the control over the system is in here

#include "g_local.h"


//clears isolation status (send back to the main dimension)
void JKG_ClearIsolate(gentity_t* ent)
{
	if (!ent->client)
		return;

	ent->dimension.dNum = 0;
	ent->dimension.properties = { 0, 0, 0, 0, 0, 0, 0, 0 }; //clear out isolation properties (if any)
}

//isolates an entity with standard isolation, assigns a random dimension number if not specified
void JKG_IsolateEntity(gentity_t* ent, int dNum = Q_irandSafe(1,9999))
{
	//only clients please
	if (!ent->client)
		return;

	//setting this to 0 is the same as removing isolation you idiot
	if (dNum == 0)
	{
		JKG_ClearIsolate(ent);
		return;
	}

	ent->dimension.dNum = dNum;
	ent->dimension.properties = { 0, 0, 0, 0, 0, 0 }; //Performs a standard isolation (only dNum matters)
}

//isolates client to the specified dimension, with specific properties
void JKG_IsolateEntity(gentity_t* ent, int dNum, const std::vector<bool>& properties)
{
	if (!ent->client)
		return;

	if (dNum == 0)
		JKG_ClearIsolate(ent);

	ent->dimension.dNum = dNum;
	ent->dimension.properties = properties;
}

//sets ent1 to the same dimension as ent2
void JKG_CopyIsolateProperties(gentity_t* ent1, gentity_t* ent2)
{
	//useful if you want to send player(s) to the same instance as another player

	if (!ent1->client || !ent2->client)
		return;

	ent1->dimension.dNum = ent2->dimension.dNum;
	ent1->dimension.properties = ent2->dimension.properties;
}

//checks if an entity is isolated
bool JKG_IsIsolated(gentity_t* ent)
{
	//note ent might not be isolated from a specific entity (eg: they share the same dimension)

	//we can't check that, so return false
	if (!ent->client)
		return false;

	if (ent->dimension.dNum == 0)
		return false;

	else
		return true;	 
}


//compares two clients to see if they are isolated from each other
bool JKG_IsIsolated(gentity_t* ent1, gentity_t* ent2)
{
	//can only be isolated from other clients, not other random ents
	if ( !ent1->client || !ent2->client )
		return false; 

	if ( ent1->dimension.dNum != ent2->dimension.dNum )
		return true;

	//they live together
	if ( ent1->dimension.dNum == ent2->dimension.dNum )
	{
		//dimension 0 is the global space, no one there is isolated
		if (ent1->dimension.dNum == 0)
			return false;

		//negative dimensions are always isolated
		if (ent1->dimension.dNum < 0 || ent2->dimension.dNum < 0)
			return true;

		//positive dimensions
		else
		{
			//so far the same, but we need to check the bit values (for when they have different isolation properties)
			for (int i = 0; i < ent1->dimension.properties.size(); i++)
			{
				if ( ent1->dimension.properties.at(i) != ent2->dimension.properties.at(i) )
					return false;
			}

			//if we made it here, they're the same
			return true;
		}
	}
}


/*
dimension.properties values:

0 = damage,
1 = collision(knockback, movement, etc),
2 = interact(pazaak, trading, etc),
3 = visual(draw, efx, etc),
4 = audio(footsteps, pain etc),
5 = chat(local chat)
*/

//Skips damage checks (different dimensions)
void JKG_IsolateDamage()
{
	
	//check JKG_IsIsolated(attacker, target)
		//target is immune to damage
}

//Skips collision checks (different dimensions)
void JKG_IsolateCollison()
{
	// Special trace for the PM system (movement)
}

//skips misc interaction checks (different dimensions)
void JKG_IsolateInteract()
{

}

//skips visual draw calls (different dimensions)
void JKG_IsolateVis()
{

}

//skips audio checks (different dimensions)
void JKG_IsolateAudio()
{

}

//skips local chat (different dimensions)
void JKG_IsolateChat()
{

}



//end of new system


//old system below:


char PlayerHide[MAX_CLIENTS][MAX_CLIENTS];	// PlayerHide[client][target]. 1 = hidden, 0 = normal

typedef struct {
	int entNum;
	int contents;
} contbackup_t;

contbackup_t contbackup[MAX_GENTITIES];

void JKG_PlayerIsolationInit() {
	memset(&PlayerHide, 0, sizeof(PlayerHide));
}

void JKG_PlayerIsolationClear(int client) {
	int i;
	for (i=0; i<MAX_CLIENTS; i++) {
		PlayerHide[i][client] = 0;
		PlayerHide[client][i] = 0;
	}

}

void JKG_PlayerIsolate(int client1, int client2) {
	if (client1 >= MAX_CLIENTS || client1 < 0) return;
	if (client2 >= MAX_CLIENTS || client2 < 0) return;
	if (client1 == client2) return;
	PlayerHide[client1][client2] = 1;
	PlayerHide[client2][client1] = 1;
}

void JKG_PlayerReveal(int client1, int client2) {
	if (client1 >= MAX_CLIENTS || client1 < 0) return;
	if (client2 >= MAX_CLIENTS || client2 < 0) return;
	PlayerHide[client1][client2] = 0;
	PlayerHide[client2][client1] = 0;
}

int JKG_IsIsolated(int client1, int client2) {
	if (PlayerHide[client1][client2] || PlayerHide[client2][client1]) {
		return 1;
	} else {
		return 0;
	}
}

void JKG_PMTrace( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask ) {
	// Special trace for the PM system (movement)
	// This will pretend player collisions dont happen by ignoring those
	// The player we're tracing from should be the one provided as passEntityNum
	// If that is not the a valid client we just ignore it and do a regular trace
	int i, j;
	if (passEntityNum < 0 || passEntityNum >= MAX_CLIENTS) {
		// Not a valid client
		trap->Trace(results, start, mins, maxs, end, passEntityNum, contentmask, 0, 0, 0);
		return;
	}
	// This of for a player, so clear out the contents of all player's he's hidden to
	for (i=0, j=0; i<sv_maxclients.integer; i++) {
		if (g_entities[i].inuse && g_entities[i].client && JKG_IsIsolated(passEntityNum, i)) {
			contbackup[j].entNum = i;
			contbackup[j].contents = g_entities[i].r.contents;
			j++;
			g_entities[i].r.contents = 0;
		}
	}

	trap->Trace(results, start, mins, maxs, end, passEntityNum, contentmask, 0, 0, 0);
	
	for (i=0; i<j; i++) {
		g_entities[contbackup[i].entNum].r.contents = contbackup[i].contents;
	}
}