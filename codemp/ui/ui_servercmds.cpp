// This is a JKG system, not basejka

#include "ui_local.h"

#if 0
static void UI_ServerRedirect( void )
{
	// clc.serverMessage will be no more than 1024 bytes so there's
	// no need to buffer these, we can just pass the argv pointers :D
	char *connAddress = Cmd_Argv(1);

	// We must have the server address at least
	if ( Cmd_Argc() < 2 ) {
		return;
	}

	// Because we're executing a command containing bits of the server command, we need to filter for \n, \r, ;, and "
	if ( strchr(connAddress,'\n') || strchr(connAddress,'\r') || strchr(connAddress,';') || strchr(connAddress,'"') ) {
		return;
	}
	// Well actually, on further research, the command breaking symbols above won't affect Cbuf_ExecuteText here because
	// the EXEC_NOW that's specified will call Cmd_ExecuteString not Cbuf_AddText, and will only execute one command.
	// Regardless, we'll filter them anyway because they shouldn't be sent from the server at all.
	// For confirmation on that, follow line 827 of cl_ui.c in the Quake 3 source code.

	trap->Cvar_Set( "cflag", Cmd_Argv(2) );
	trap->Cvar_Set( "connmsg", Cmd_Argv(3) );
	trap->Cmd_ExecuteText( EXEC_NOW, va( "connect \"%s\"", connAddress ) ); // Also with EXEC_NOW, no need to pass a trailing \n
}
#endif

qboolean UI_ServerCommand( const char *cmd_string )
{
	//char *cmd;

	if ( !cmd_string[0] ) {
		return qfalse;
	}

	if ( cmd_string[0] == '@' && cmd_string[1] == '@' ) {
		// Oh god... server returned a localized string ref for some reason, ignore
		return qfalse;
	}

	// TODO: This should be reimplemented at some point.
#if 0
	Cmd_TokenizeString( cmd_string );

	if ( Cmd_Argc() == 0 ) {
		return qfalse;
	}

	cmd = Cmd_Argv(0);

	if ( !strcmp( cmd, "svr" ) ) {
		UI_ServerRedirect();
		return qtrue;
	} else if ( !strcmp( cmd, "update" ) ) {
		//UI_PromptUpdate(); // Just an example of a UI Server Command that we could have
		//return qtrue;
	}
#endif

	return qfalse;
}
