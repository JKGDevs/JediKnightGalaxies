
	/**************************************************
	* Jedi Knight Galaxies
	*
	* This file deals with anything related to the teams,
	* thus this includes parties, clans and alliances.
	* Simple functions to compare friendly states are
	* available to perform team checks.
	**************************************************/

	#include "g_local.h"
	int G_ClientNumberFromStrippedSubstring ( const char* name, qboolean checkAll );

	/**************************************************
	* level.party[i][0]				= 5				* This is the team leader.
	* level.party[i][1]				= -7			* Pending request for player #6.
	* level.party[i][2]				= 3				* Active party member, player #3.
	* level.party[i][3]				= 64 (0100 000)	* Empty slot which can be utilized.
	***************************************************
	* pers.partyNumber				= 1				* You are part of party #1.
	* pers.partyIndex				= 3				* You're level.party[1][3] in party reference.
	* pers.partyInvite[0]			= 5				* Open request for party #5.
	* pers.partyInvite[1]			= 64 (1000 000)	* Empty invite slot for a party to fill up.
	***************************************************
	* partyaccept <slot>
	* partychangeleader <slot>
	* partycreate
	* partydisband
	* partydismiss <slot>
	* partyinvite <target>
	* partyleave
	* partylistrefresh <time>
	* partylistregister <message>
	* partylistunregister
	* partyreject <slot>
	**************************************************/

	int			uiCaller;
	qboolean	uiFeedBack;

	/**************************************************
	* SendSystem
	*
	* Sends a system message to the client. This might
	* be a response via a UI call, will only work on the
	* caller though.
	**************************************************/

	static void SendSystem( int clientNum, char *fmt, ... )
	{
		va_list argptr;
		char buffer[1024];

		va_start( argptr, fmt );
		Q_vsnprintf( buffer, 1024, fmt, argptr );
		va_end( argptr );

		if ( uiFeedBack && clientNum == uiCaller )
		{
			trap->SendServerCommand( clientNum, va( "pmr \"%s\"", Q_CleanStr( buffer )));
		}
		else
		{
			trap->SendServerCommand( clientNum, va( "chat 100 \"^1System: ^7%s\"", buffer ));
		}
	}

	/**************************************************
	* TeamCommand
	*
	* A client is issueing a command, check for team commands here.
	**************************************************/

	qboolean TeamCommand( int clientNum, char *cmd, char *param )
	{
		//gentity_t	*ent = &g_entities[clientNum];
		char		 parm[1024] = {0};
		char		 cmdLocal[1024] = {0};

		// Only allowed in FFA and RPG modes
		if (g_gametype.integer != GT_FFA)
		{
			return qfalse;
		}
	
		/**************************************************
		* Set the uiFeedback to qfalse, as we expect it to be.
		**************************************************/

		uiCaller = clientNum;
		uiFeedBack = qfalse;

		/**************************************************
		* Retrieve the parameter that could have been set.
		**************************************************/

		if ( trap->Argc() >= 2 )
		{
			if ( param == NULL )
			{
				trap->Argv( 1, parm, sizeof( parm ));
			}
			else
			{
				Q_strncpyz( parm, param, sizeof( parm ));
			}
		}

		/**************************************************
		* Team Management Requests
		**************************************************/

		if ( Q_stricmp( cmd, "~pmngt" ) == 0 )
		{
			/**************************************************
			* Set the uiFeedback to qtrue, as this is a ui command!
			**************************************************/

			uiFeedBack = qtrue;

			/**************************************************
			* Change the command pointer to accept the parameter.
			**************************************************/

			Q_strncpyz( cmdLocal, parm, sizeof( cmdLocal ));
			trap->Argv( 2, parm, sizeof( parm ));
			cmd = ( char * ) &cmdLocal;

			/**************************************************
			* Received when team management is toggled off.
			**************************************************/

			if ( Q_stricmp( cmd, "off" ) == 0 )
			{
				g_entities[clientNum].client->pers.partyManagement = qfalse;
				return qtrue;
			}

			/**************************************************
			* Received when team management is toggled on.
			**************************************************/

			if ( Q_stricmp( cmd, "on" ) == 0 )
			{
				g_entities[clientNum].client->pers.partyManagement = level.time;
				return qtrue;
			}
		}

		/**************************************************
		* These are the party-prefixed commands, dealing with your party.
		**************************************************/

		if ( Q_stricmpn( cmd, "party", 5 ) == 0 )
		{
			/**************************************************
			* Increment the command pointer to remove the prefix.
			**************************************************/

			cmd += 5;

			/**************************************************
			* Accepts the party invitation in the provided slot.
			**************************************************/

			if ( Q_stricmp( cmd, "accept" ) == 0 )
			{
				if (( uiFeedBack && trap->Argc() <= 2 ) || ( !uiFeedBack && trap->Argc() <= 1 ))
				{
					SendSystem( clientNum, "This command requires a parameter." );
					return qtrue;
				}

				TeamPartyCommandAccept( clientNum, atoi( parm ));
				return qtrue;
			}

			/**************************************************
			* Changes the authority of the party leader to another member. The identifier is the slot!
			**************************************************/

			if ( Q_stricmp( cmd, "changeleader" ) == 0 )
			{
				if (( uiFeedBack && trap->Argc() <= 2 ) || ( !uiFeedBack && trap->Argc() <= 1 ))
				{
					SendSystem( clientNum, "This command requires a parameter." );
					return qtrue;
				}

				TeamPartyCommandChangeLeader( clientNum, atoi( parm ), qfalse );
				return qtrue;
			}

			/**************************************************
			* Creates a new party. Find the first slot and set the client as leader.
			**************************************************/

			if ( Q_stricmp( cmd, "create" ) == 0 )
			{
				TeamPartyCommandCreate( clientNum );
				return qtrue;
			}	

			/**************************************************
			* Disbands the party, removes all members and the leader.
			**************************************************/

			if ( Q_stricmp( cmd, "disband" ) == 0 )
			{
				TeamPartyCommandDisband( clientNum );
				return qtrue;
			}

			/**************************************************
			* Dismisses the target member from the party. The identifier is the slot!
			**************************************************/

			if ( Q_stricmp( cmd, "dismiss" ) == 0 )
			{
				if (( uiFeedBack && trap->Argc() <= 2 ) || ( !uiFeedBack && trap->Argc() <= 1 ))
				{
					SendSystem( clientNum, "This command requires a parameter." );
					return qtrue;
				}

				TeamPartyCommandDismiss( clientNum, atoi( parm ));
				return qtrue;
			}

			/**************************************************
			* Invites a player to join your party. The parameter can be a identifier or name.
			**************************************************/

			if ( Q_stricmp( cmd, "invite" ) == 0 )
			{
				TeamPartyCommandInvite( clientNum, parm );
				return qtrue;
			}

			/**************************************************
			* Leave your current party. This may not be performed by the leader.
			**************************************************/

			if ( Q_stricmp( cmd, "leave" ) == 0 )
			{
				TeamPartyCommandLeave( clientNum, qfalse );
				return qtrue;
			}

			/**************************************************
			* Rejects the party invitation in the provided slot.
			**************************************************/

			if ( Q_stricmp( cmd, "reject" ) == 0 )
			{
				if (( uiFeedBack && trap->Argc() <= 2 ) || ( !uiFeedBack && trap->Argc() <= 1 ))
				{
					SendSystem( clientNum, "This command requires a parameter." );
					return qtrue;
				}

				TeamPartyCommandReject( clientNum, atoi( parm ));
				return qtrue;
			}

			/**************************************************
			* Refresh the list, we expect a time stamp to be
			* send (or if it's insane, ignore it) to save some
			* bandwidth and only generate a delta.
			**************************************************/

			if ( !Q_stricmp( cmd, "listrefresh" ))
			{
				TeamPartyListRefresh( clientNum, atoi( parm ));
				return qtrue;
			}

			/**************************************************
			* Register yourself on the list of players seeking
			* a party. We use concat data to get the entered
			* string.
			**************************************************/

			if ( !Q_stricmp( cmd, "listregister" ))
			{
				TeamPartyListRegister( clientNum, ConcatArgs((( uiFeedBack ) ? 2 : 1 )));
				return qtrue;
			}

			/**************************************************
			* Unregister yourself from the list. You must be
			* placed on the list for this to work, you'll be
			* removed then.
			**************************************************/

			if ( !Q_stricmp( cmd, "listunregister" ))
			{
				TeamPartyListUnregister( clientNum, qfalse );
				return qtrue;
			}
		}

		return qfalse;
	}

	/**************************************************
	* TeamDisconnect
	*
	* This client is disconnecting without dealing with his team.
	**************************************************/

	void TeamDisconnect( int clientNum )
	{
		gentity_t *ent = &g_entities[clientNum];
		int i = 0;

		if ( ent->client->pers.partyNumber == PARTY_SLOT_EMPTY )
		{
			return;
		}

		if ( ent->client->pers.partyIndex == 0 )
		{
			for ( i = 0; i < PARTY_SLOT_MEMBERS; i++ )
			{
				if ( level.party[ent->client->pers.partyNumber][i] != PARTY_SLOT_EMPTY && level.party[ent->client->pers.partyNumber][i] >= 0 )
				{
					TeamPartyCommandChangeLeader( clientNum, i, qtrue );
					break;
				}
			}

			if ( i == PARTY_SLOT_MEMBERS )
			{
				TeamPartyCommandDisband( clientNum );
			}
		}

		if ( i < PARTY_SLOT_MEMBERS )
		{
			TeamPartyCommandLeave( clientNum, qfalse );
		}
	}

	/**************************************************
	* TeamFriendly
	*
	* Returns wether or not these players are friendly.
	*************************************************/

	qboolean TeamFriendly( int p1, int p2 )
	{
		gentity_t *e1 = &g_entities[p1];
		gentity_t *e2 = &g_entities[p2];

		if ( e1->client && e2->client && e1->client->pers.partyNumber != PARTY_SLOT_EMPTY && e1->client->pers.partyNumber == e2->client->pers.partyNumber )
		{
			return qtrue;
		}

		return qfalse;
	}

	/**************************************************
	* TeamInitialize
	*
	* Initializes the party identifiers for this player.
	*************************************************/

	void TeamInitialize( int clientNum )
	{
		int i;

		gclient_t *client			= g_entities[clientNum].client;
		client->pers.partyNumber	= PARTY_SLOT_EMPTY;
		client->pers.partyIndex		= PARTY_SLOT_EMPTY;

		for ( i = 0; i < PARTY_SLOT_INVITES; i++ )
		{
			client->pers.partyInvite[i] = PARTY_SLOT_EMPTY;
		}		
	}

	/**************************************************
	* TeamInitializeServer
	*
	* Initializes the party identifiers for the server.
	*************************************************/

	void TeamInitializeServer( void )
	{
		int i, j;

		for ( i = 0; i < MAX_CLIENTS; i++ )	// HACK
		{
			for ( j = 0; j < PARTY_SLOT_MEMBERS; j++ )
			{
				level.party[i][j] = MAX_CLIENTS; // HACK
			}
		}

		for ( i = 0; i < MAX_CLIENTS; i++ )
		{
			level.partyList[i].id = MAX_CLIENTS;
		}
	}

	/**************************************************
	* TeamPartyCommandAccept
	*
	* Accepts the party invitation in the provided slot.
	**************************************************/

	void TeamPartyCommandAccept( int clientNum, int iID )
	{
		gentity_t *ent = &g_entities[clientNum];
		int i, j;

		if ( iID < 0 || iID >= PARTY_SLOT_INVITES )
		{
			iID = 0;
		}

		if ( ent->client->pers.partyInvite[iID] == PARTY_SLOT_EMPTY )
		{
			SendSystem( clientNum, "This slot does not contain an invitation." );
			return;
		}

		for ( i = 0; i < PARTY_SLOT_MEMBERS; i++ )
		{
			if ( level.party[ent->client->pers.partyInvite[iID]][i] == ( 0 - clientNum - 1 ))
			{
				break;
			}
		}

		if ( i == PARTY_SLOT_MEMBERS )
		{
			SendSystem( clientNum, "This slot does not contain a valid invitation." );
			return;
		}

		/* Set this client as an accepted client in the party, overwriting the earlier invitation slot */
		level.party[ent->client->pers.partyInvite[iID]][i] = clientNum;
		ent->client->pers.partyNumber = ent->client->pers.partyInvite[iID];
		ent->client->pers.partyIndex = i;

		/* Find any pending requests to join a party and deny them */
		for ( j = 0; j < PARTY_SLOT_INVITES; j++ )
		{
			if ( ent->client->pers.partyInvite[j] != ent->client->pers.partyNumber && ent->client->pers.partyInvite[j] != PARTY_SLOT_EMPTY )
			{
				TeamPartyCommandReject( clientNum, ent->client->pers.partyInvite[j] );
			}
		}

		/* Clear the remaining slot and send the update to the party */
		ent->client->pers.partyInvite[iID] = PARTY_SLOT_EMPTY;
		TeamPartyUpdate( ent->client->pers.partyNumber, PARTY_SLOT_EMPTY );

		/* Send a confirmation message to all the party members */
		for ( i = 0; i < PARTY_SLOT_MEMBERS; i++ )
		{
			if ( level.party[ent->client->pers.partyNumber][i] != PARTY_SLOT_EMPTY && level.party[ent->client->pers.partyNumber][i] >= 0 )
			{
				iID = level.party[ent->client->pers.partyNumber][i];
				iID = ( iID < 0 ) ? abs( iID + 1 ) : iID;
				SendSystem( iID, "%s^7 has joined the party.", ent->client->pers.netname );
				g_entities[iID].client->pers.partyUpdate = qtrue;
			}
		}

		/* Force unregister */
		TeamPartyListUnregister( clientNum, qtrue );

		/* This player deserves an update, since he just joined a new team */
		ent->client->pers.partyUpdate = qtrue;
	}

	/**************************************************
	* TeamPartyCommandChangeLeader
	*
	* Changes the authority of the party leader to another member. The identifier is the slot!
	**************************************************/

	void TeamPartyCommandChangeLeader( int clientNum, int iID, qboolean forceClient )
	{
		gentity_t *ent = &g_entities[clientNum];
		int i;
		
		if ( ent->client->pers.partyNumber == PARTY_SLOT_EMPTY )
		{
			SendSystem( clientNum, "You are currently not in any party." );
			return;
		}

		if ( iID < 0 || iID >= PARTY_SLOT_MEMBERS || iID == 0 )
		{
			SendSystem( clientNum, "The provided target is not valid." );
			return;
		}

		if ( ent->client->pers.partyIndex )
		{
			SendSystem( clientNum, "You may only use this as the party leader." );
			return;
		}

		if ( level.party[ent->client->pers.partyNumber][iID] == PARTY_SLOT_EMPTY || level.party[ent->client->pers.partyNumber][iID] < 0 )
		{
			SendSystem( clientNum, "The provided target is not valid." );
			return;
		}
		
		/* Swap the player party index, this doesn't happen anywhere but here! */
		g_entities[(int)level.party[ent->client->pers.partyNumber][0]].client->pers.partyIndex = iID;
		g_entities[(int)level.party[ent->client->pers.partyNumber][iID]].client->pers.partyIndex = 0;

		/* Mark the correct leader in the party struct */
		level.party[ent->client->pers.partyNumber][0] = level.party[ent->client->pers.partyNumber][iID];
		level.party[ent->client->pers.partyNumber][iID] = clientNum;

		/* Send a message to all the party members, informing about the change in authority */
		for ( i = 0; i < PARTY_SLOT_MEMBERS; i++ )
		{
			if ( level.party[ent->client->pers.partyNumber][i] != PARTY_SLOT_EMPTY && level.party[ent->client->pers.partyNumber][i] >= 0 )
			{
				SendSystem( level.party[ent->client->pers.partyNumber][i], "%s^7 has become the party leader.", g_entities[(int)level.party[ent->client->pers.partyNumber][0]].client->pers.netname );
			}
		}

		/* Only do the update when we're not forced (otherwise it's done on the leave) */
		if ( !forceClient ) TeamPartyUpdate( ent->client->pers.partyNumber, PARTY_SLOT_EMPTY );
	}

	/**************************************************
	* TeamPartyCommandCreate
	*
	* Creates a new party. Find the first slot and set the client as leader.
	**************************************************/

	void TeamPartyCommandCreate( int clientNum )
	{
		gentity_t *ent = &g_entities[clientNum];
		int i;

		if ( ent->client->pers.partyNumber != PARTY_SLOT_EMPTY )
		{
			SendSystem( clientNum, "You are already in a party." );
			return;
		}

		for ( i = 0; i < PARTY_SLOT_INVITES; i++ )
		{
			if ( ent->client->pers.partyInvite[i] != PARTY_SLOT_EMPTY )
			{
				TeamPartyCommandReject( clientNum, ent->client->pers.partyInvite[i] );
			}
		}

		TeamPartyListUnregister( clientNum, qtrue );

		for ( i = 0; i < MAX_CLIENTS; i++ )
		{
			if ( level.party[i][0] == PARTY_SLOT_EMPTY )
			{
				SendSystem( clientNum, "You have created a new party." );
				ent->client->pers.partyNumber = i;
				ent->client->pers.partyIndex = 0;
				level.party[i][0] = clientNum;
				TeamPartyUpdate( ent->client->pers.partyNumber, PARTY_SLOT_EMPTY );
				return;
			}
		}
	}

	/**************************************************
	* TeamPartyCommandDisband
	*
	* Disbands the party, removes all members and the leader.
	**************************************************/

	void TeamPartyCommandDisband( int clientNum )
	{
		gentity_t *ent = &g_entities[clientNum];
		int i, j, iID, partyNum;

		if ( ent->client->pers.partyNumber == PARTY_SLOT_EMPTY )
		{
			SendSystem( clientNum, "You are currently not in any party." );
			return;
		}

		if ( ent->client->pers.partyIndex )
		{
			SendSystem( clientNum, "You may only use this as the party leader." );
			return;
		}

		partyNum = ent->client->pers.partyNumber;

		for ( i = 0; i < PARTY_SLOT_MEMBERS; i++ )
		{
			if ( level.party[partyNum][i] == PARTY_SLOT_EMPTY )
			{
				continue;
			}

			if ( level.party[partyNum][i] >= 0 )
			{
				SendSystem( level.party[partyNum][i], "The party has been disbanded." );
				iID = level.party[partyNum][i];
				TeamPartyCommandLeave( iID, qtrue );
				TeamPartyUpdate( PARTY_SLOT_EMPTY, iID );
			}
			else
			{
				for ( j = 0; j < PARTY_SLOT_INVITES; j++ )
				{
					if ( g_entities[abs( level.party[partyNum][i] + 1 )].client->pers.partyInvite[j] == ent->client->pers.partyNumber )
					{
						TeamPartyCommandReject( abs( level.party[partyNum][i] + 1 ), j );
					}
				}
			}
		}
	}

	/**************************************************
	* TeamPartyCommandDismiss
	*
	* Dismisses the target member from the party. The identifier is the slot!
	**************************************************/

	void TeamPartyCommandDismiss( int clientNum, int iID )
	{
		gentity_t *ent = &g_entities[clientNum];
		int i, partyNum, targetNum;

		if ( ent->client->pers.partyNumber == PARTY_SLOT_EMPTY )
		{
			SendSystem( clientNum, "You are currently not in any party." );
			return;
		}

		if ( ent->client->pers.partyIndex )
		{
			SendSystem( clientNum, "You may only use this as the party leader." );
			return;
		}

		if ( iID < 0 || iID >= PARTY_SLOT_MEMBERS || level.party[ent->client->pers.partyNumber][iID] == clientNum || level.party[ent->client->pers.partyNumber][iID] == PARTY_SLOT_EMPTY )
		{
			SendSystem( clientNum, "The provided target is not valid." );
		}
		else
		{
			/* Inform all the team members about the removed team member */
			for ( i = 0; i < PARTY_SLOT_MEMBERS; i++ )
			{
				if ( i == iID || level.party[ent->client->pers.partyNumber][i] < 0 )
				{
					continue;
				}

				SendSystem( level.party[ent->client->pers.partyNumber][i], "%s^7 has been dismissed from the party.", g_entities[(int)level.party[ent->client->pers.partyNumber][iID]].client->pers.netname );
			}

			/* Get the target and party number for the dismiss */
			partyNum = ent->client->pers.partyNumber;
			targetNum = level.party[ent->client->pers.partyNumber][iID];

			/* Inform the client himself that he has been dismissed */
			SendSystem( targetNum, "You have been dismissed from the party." );

			/* Remove the client entries about the party */
			g_entities[targetNum].client->pers.partyNumber = PARTY_SLOT_EMPTY;
			g_entities[targetNum].client->pers.partyIndex = PARTY_SLOT_EMPTY;
			level.party[partyNum][iID] = PARTY_SLOT_EMPTY;

			/* Prepare an update for both the team and the dismissed client */
			TeamPartyUpdate( partyNum, targetNum );
			
		}
	}

	/**************************************************
	* TeamPartyCommandInvite
	*
	* Invites a player to join your party. The parameter can be a identifier or name.
	**************************************************/

	void TeamPartyCommandInvite( int clientNum, char *parm )
	{
		gentity_t *ent = &g_entities[clientNum];
		gentity_t *other;
		int i, j, iID;

		if ( ent->client->pers.partyNumber == PARTY_SLOT_EMPTY )
		{
			SendSystem( clientNum, "You are currently not in any party." );
			return;
		}

		if ( ent->client->pers.partyIndex )
		{
			SendSystem( clientNum, "You may only use this as the party leader." );
			return;
		}

		for ( j = 0; j < PARTY_SLOT_MEMBERS; j++ )
		{
			if ( level.party[ent->client->pers.partyNumber][j] == PARTY_SLOT_EMPTY )
			{
				break;
			}
		}

		if ( j == PARTY_SLOT_MEMBERS )
		{
			SendSystem( clientNum, "You may not invite another player." );
			return;
		}

		if (( iID = TeamTarget( ent, parm )) < 0 )
		{
			SendSystem( clientNum, "The provided target is not valid." );
			return;
		}

		other = &g_entities[iID];

		if ( other->client->pers.partyNumber == PARTY_SLOT_EMPTY )
		{
			for ( i = 0; i < PARTY_SLOT_MEMBERS; i++ )
			{
				if ( other->client->pers.partyInvite[i] == ent->client->pers.partyNumber )
				{
					SendSystem( clientNum, "The target already received an invitation." );
					return;
				}
			}

			for ( i = 0; i < PARTY_SLOT_INVITES; i++ )
			{
				if ( other->client->pers.partyInvite[i] == PARTY_SLOT_EMPTY )
				{
					/* Set the slot as pending client according to the specifications, and fill in the invite */
					level.party[ent->client->pers.partyNumber][j] = ( 0 - iID - 1 );
					other->client->pers.partyInvite[i] = ent->client->pers.partyNumber;

					/* Update the team status */
					TeamPartyUpdate( ent->client->pers.partyNumber, iID );

					/* Send some messages to the inviter and invitee */
					SendSystem( clientNum, "You have invited %s^7 to join the party.", other->client->pers.netname );
					SendSystem( iID, "You have been invited to %s^7's party!", ent->client->pers.netname );
					return;
				}
			}
		}

		SendSystem( clientNum, "You cannot invite %s^7 to your party.", other->client->pers.netname );
		return;
	}

	/**************************************************
	* TeamPartyCommandLeave
	*
	* Leave your current party. This may not be performed by the leader.
	**************************************************/

	void TeamPartyCommandLeave( int clientNum, qboolean forceClient )
	{
		gentity_t *ent = &g_entities[clientNum];
		int i, iID;

		if ( ent->client->pers.partyNumber == PARTY_SLOT_EMPTY )
		{
			SendSystem( clientNum, "You are currently not in any party." );
			return;
		}

		if ( !forceClient && ent->client->pers.partyIndex == 0 )
		{
			SendSystem( clientNum, "You may not use this command as a party leader." );
			return;
		}

		if ( !forceClient )
		{
			for ( i = 0; i < PARTY_SLOT_MEMBERS; i++ )
			{
				if ( i == ent->client->pers.partyIndex || level.party[ent->client->pers.partyNumber][i] == PARTY_SLOT_EMPTY || level.party[ent->client->pers.partyNumber][i] < 0 )
				{
					continue;
				}

				SendSystem( level.party[ent->client->pers.partyNumber][i], "%s^7 has left the party.", ent->client->pers.netname );
			}
		}

		/* Remove your identifier from the party and send updates to the rest of the team */
		level.party[ent->client->pers.partyNumber][ent->client->pers.partyIndex] = PARTY_SLOT_EMPTY;
		iID = ent->client->pers.partyNumber;

		/* Remove the party number and index from the player, we're done now */
		ent->client->pers.partyNumber = PARTY_SLOT_EMPTY;
		ent->client->pers.partyIndex = 0;

		/* Update the team party information, somebody has left (unless we're disbanding) */
		if ( !forceClient )
		{
			TeamPartyUpdate( iID, clientNum );
			SendSystem( clientNum, "You have left the party." );
		}
	}

	/**************************************************
	* TeamPartyListRefresh
	*
	* Refreshes the 'seeking party list', where people register to look for a party.
	**************************************************/

	void TeamPartyListRefresh( int clientNum, unsigned int iTime )
	{
		char buffer[1016];	// Minus 'tpl ""' - 8 as safety.
		int i, classId = 0;
		qboolean msgSent = qfalse;

		memset( buffer, 0, sizeof( buffer ));

		if (iTime < 0 || iTime >= UINT_MAX - 1/*4294967296*/) // --futuza: fixed, can't check higher than INT_MAX
		{
			iTime = 0;
		}

		for ( i = 0; i < MAX_CLIENTS; i++ )
		{
			if ( level.partyList[i].time && level.partyList[i].time >= iTime )
			{
				if ( !Q_stratt( buffer, sizeof( buffer ), va( "%i %i %i %i \"%s\" ", i, level.partyList[i].id, classId, level.partyList[i].time, level.partyList[i].message )))
				{
					msgSent = qtrue;
					trap->SendServerCommand( clientNum, va( "tpl %s", buffer ));
					memset( buffer, 0, sizeof( buffer ));
					i--;
				}
			}
		}

		if (( uiFeedBack && !msgSent ) || strlen( buffer ))
		{
			trap->SendServerCommand( clientNum, va( "tpl %s", buffer ));
		}

		if ( g_entities[clientNum].client->pers.partyManagement )
		{
			g_entities[clientNum].client->pers.partyManagement = level.time;
		}
	}

	/**************************************************
	* TeamPartyListRegister
	*
	* Register yourself on the seeking list!
	**************************************************/

	void TeamPartyListRegister( int clientNum, char *message )
	{
		int i, iID;

		if ( trap->Argc() <= 1 )
		{
			SendSystem( clientNum, "This command requires you to specify a parameter." );
			return;
		}

		if ( message == NULL || strlen( message ) >= 64 )
		{
			SendSystem( clientNum, "You may not use this length for your message." );
			return;
		}

		for ( i = 0, iID = PARTY_SLOT_EMPTY; i < MAX_CLIENTS; i++ )
		{
			if ( iID == PARTY_SLOT_EMPTY && level.partyList[i].id == PARTY_SLOT_EMPTY )
			{
				iID = i;
			}

			if ( level.partyList[i].id == clientNum )
			{
				SendSystem( clientNum, "You have already been registered on the list." );
				return;
			}
		}

		level.partyList[iID].id = clientNum;
		level.partyList[iID].time = level.time;
		Q_strncpyz( level.partyList[iID].message, message, sizeof( level.partyList[iID].message ));
		SendSystem( clientNum, "You have been registered on the list." );

		for ( i = 0; i < level.maxclients; i++ )
		{
			if ( g_entities[i].client->pers.partyManagement )
			{
				TeamPartyListRefresh( i, g_entities[i].client->pers.partyManagement );
			}
		}
	}

	/**************************************************
	* TeamPartyListUnregister
	*
	* Unregister yourself from the seeking list!
	**************************************************/

	void TeamPartyListUnregister( int clientNum, qboolean forcedUpdate )
	{
		qboolean bDidUpdate = qfalse;
		int i;

		for ( i = 0; i < MAX_CLIENTS; i++ )
		{
			if ( level.partyList[i].id == clientNum )
			{
				bDidUpdate = qtrue;
				level.partyList[i].id = PARTY_SLOT_EMPTY;
				level.partyList[i].time = level.time;
				memset( level.partyList[i].message, 0, sizeof( level.partyList[i].message ));
				if ( !forcedUpdate ) SendSystem( clientNum, "You have been removed from the list." );
				break;
			}
		}

		if ( !forcedUpdate && !bDidUpdate )
		{
			SendSystem( clientNum, "You are not on the list." );
			return;
		}

		for ( i = 0; i < level.maxclients; i++ )
		{
			if ( g_entities[i].client->pers.partyManagement )
			{
				TeamPartyListRefresh( i, g_entities[i].client->pers.partyManagement );
			}
		}
	}

	/**************************************************
	* TeamPartyCommandReject
	*
	* Rejects the party invitation in the provided slot.
	**************************************************/

	void TeamPartyCommandReject( int clientNum, int iID )
	{
		gentity_t *ent = &g_entities[clientNum];
		int i, partyNum;

		if ( iID < 0 || iID >= PARTY_SLOT_INVITES )
		{
			iID = 0;
		}

		if ( ent->client->pers.partyInvite[iID] == PARTY_SLOT_EMPTY )
		{
			SendSystem( clientNum, "This slot does not contain an invitation." );
			return;
		}

		partyNum = ent->client->pers.partyInvite[iID];

		for ( i = 0; i < PARTY_SLOT_MEMBERS; i++ )
		{
			if ( level.party[partyNum][i] == ( 0 - clientNum - 1 ))
			{
				SendSystem( level.party[partyNum][0], "%s^7 has rejected the party invitation.", ent->client->pers.netname );
				SendSystem( clientNum, "You have rejected %s^7's party invitation.", g_entities[(int)level.party[partyNum][0]].client->pers.netname );

				level.party[partyNum][i] = PARTY_SLOT_EMPTY;
				ent->client->pers.partyInvite[iID] = PARTY_SLOT_EMPTY;

				TeamPartyUpdate( partyNum, clientNum );
				return;
			}
		}
	}

	/**************************************************
	* TeamPartyCount
	*
	* Counts the number of players that are in the party.
	**************************************************/

	int TeamPartyCount( int partyNum )
	{
		int i, c = 0;

		for ( i = 0; i < PARTY_SLOT_MEMBERS; i++ )
		{
			if ( level.party[partyNum][i] == PARTY_SLOT_EMPTY || level.party[partyNum][i] < 0 )
			{
				continue;
			}

			c++;
		}

		return c;
	}

	/**************************************************
	* TeamPartyUpdate
	*
	* Updates the team and individual status. This sends a full incremental update.
	*************************************************/

	void TeamPartyUpdate( int partyNum, int forceClient )
	{
		gentity_t *ent = &g_entities[forceClient];
		int i, iID;

		/**************************************************
		* Individual update will send open invitations, only
		* when this client does not belong to any party.
		**************************************************/

		if ( forceClient != PARTY_SLOT_EMPTY && ent->client->pers.partyNumber == PARTY_SLOT_EMPTY )
		{
			trap->SendServerCommand( forceClient, va( "tpi \"%i %i %i %i %i %i %i %i %i %i %i %i %i %i %i",
				ent->client->pers.partyInvite[0], level.party[ent->client->pers.partyInvite[0]][0], TeamPartyCount( ent->client->pers.partyInvite[0] ),
				ent->client->pers.partyInvite[1], level.party[ent->client->pers.partyInvite[1]][0], TeamPartyCount( ent->client->pers.partyInvite[1] ),
				ent->client->pers.partyInvite[2], level.party[ent->client->pers.partyInvite[2]][0], TeamPartyCount( ent->client->pers.partyInvite[2] ),
				ent->client->pers.partyInvite[3], level.party[ent->client->pers.partyInvite[3]][0], TeamPartyCount( ent->client->pers.partyInvite[3] ),
				ent->client->pers.partyInvite[4], level.party[ent->client->pers.partyInvite[4]][0], TeamPartyCount( ent->client->pers.partyInvite[4] )));
		}

		/**************************************************
		* Party update will be send to all the party members
		* while pending invitations will be handled differently,
		* but will be updated again (for the member count).
		**************************************************/

		if ( partyNum != PARTY_SLOT_EMPTY )
		{
			char *lpData = va( "tpu \"%i %i %i %i %i %i %i %i %i %i %i\"", partyNum, 
				level.party[partyNum][0], 0,
				level.party[partyNum][1], 0,
				level.party[partyNum][2], 0,
				level.party[partyNum][3], 0,
				level.party[partyNum][4], 0 );

			for ( i = 0; i < PARTY_SLOT_MEMBERS; i++ )
			{
				if ( level.party[partyNum][i] == PARTY_SLOT_EMPTY )
				{
					continue;
				}

				/* This is a pending invitation. Do an incremental individual update */
				if ( level.party[partyNum][i] < 0 )
				{
					iID = level.party[partyNum][i];
					iID = ( iID < 0 ) ? abs( iID + 1 ) : iID;
					TeamPartyUpdate( PARTY_SLOT_EMPTY, iID );
				}
				/* This is a fully fledged team member, update the client data */
				else
				{
					trap->SendServerCommand( level.party[partyNum][i], lpData );
				}
			}
		}
	}

	/**************************************************
	* TeamTarget
	*
	* Finds a target based on the client data and partial string.
	*
	*  #	: Target clientNum has been found.
	* -1	: Target was a forward trace, but nothing hit.
	* -2	: Target is a numeric value but is invalid.
	* -3	: Target has not been connected properly.
	* -4	: Target has been found in multiple names.
	**************************************************/

	int TeamTarget( gentity_t *ent, char *cmd ) 
	{
		int	iID;
		
		/* No other paramets means targeting through your crosshair */
		if ( trap->Argc() == 1 || strlen( cmd ) < 1 )
		{ 
			trace_t tr;
			vec3_t forward, end;

			AngleVectors( ent->client->ps.viewangles, forward, NULL, NULL );

			end[0] = ent->client->ps.origin[0] + forward[0] * 8192;
			end[1] = ent->client->ps.origin[1] + forward[1] * 8192;
			end[2] = ent->client->ps.origin[2] + forward[2] * 8192;
				
			trap->Trace( &tr, ent->client->ps.origin, NULL, NULL, end, ent->s.number, MASK_PLAYERSOLID, 0, 0, 0 );

			if (( tr.entityNum < MAX_CLIENTS) && ( tr.entityNum >= 0 ) && ( tr.entityNum != ent->s.number ) && g_entities[tr.entityNum].client && g_entities[tr.entityNum].inuse )
			{
				return tr.entityNum;
			}
			else
			{
				return -1;
			}
		}

		/* When using a numeric value to get the client data, which is possible */
		if ( cmd[0] >= '0' && cmd[0] <= '9' && strlen( cmd ) < 3 ) 
		{
			iID = atoi( cmd );
			
			if ( iID < 0 || iID >= level.maxclients ) 
			{
				return -2;
			}

			if ( g_entities[iID].client->pers.connected != CON_CONNECTED ) 
			{
				return -3;
			}

			return iID;
		}

		/* Check for a partial name match and check if the client is connected properly */
		if (( iID = G_ClientNumberFromStrippedSubstring( cmd, qtrue )) >= 0 )
		{
			if ( g_entities[iID].client->pers.connected != CON_CONNECTED )
			{
				return -3;
			}

			return iID;
		}
		
		/* We have found a multiple name match, this is bad */
		return -4;
	}