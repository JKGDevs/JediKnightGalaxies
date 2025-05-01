///////////////////////////////////
//
// JKG - Chat command processing
//
// Heavilly based on the engine's command tokenizer
// Client-side
//
//////////////////////////////////


#include "cg_local.h"
#include "jkg_chatcmds.h"

typedef struct ccmd_function_s
{
	struct ccmd_function_s	*next;
	char					*name;
	xccommand_t				function;
} ccmd_function_t;


static	int			ccmd_argc;
static	char		*ccmd_argv[MAX_STRING_TOKENS];		// points into cmd_tokenized
static	char		ccmd_tokenized[BIG_INFO_STRING+MAX_STRING_TOKENS];	// will have 0 bytes inserted
static	char		ccmd_cmd[BIG_INFO_STRING]; // the original command we received (no token processing)

static	ccmd_function_t	*ccmd_functions;		// possible chat commands to execute

/*
============
CCmd_Argc
============
*/
int		CCmd_Argc( void ) {
	return ccmd_argc;
}

/*
============
CCmd_Argv
============
*/
char	*CCmd_Argv( int arg ) {
	if ( (unsigned)arg >= ccmd_argc ) {
		return "";
	}
	return ccmd_argv[arg];	
}

/*
============
CCmd_ArgvBuffer
============
*/
void	CCmd_ArgvBuffer( int arg, char *buffer, int bufferLength ) {
	Q_strncpyz( buffer, CCmd_Argv( arg ), bufferLength );
}


/*
============
Cmd_Args

Returns a single string containing argv(1) to argv(argc()-1)
============
*/
char	*CCmd_Args( void ) {
	static	char		ccmd_args[MAX_STRING_CHARS];
	int		i;

	ccmd_args[0] = 0;
	for ( i = 1 ; i < ccmd_argc ; i++ ) {
		strcat( ccmd_args, ccmd_argv[i] );
		if ( i != ccmd_argc-1 ) {
			strcat( ccmd_args, " " );
		}
	}

	return ccmd_args;
}

/*
============
Cmd_Args

Returns a single string containing argv(arg) to argv(argc()-1)
============
*/
char *CCmd_ArgsFrom( int arg ) {
	static	char		ccmd_args[BIG_INFO_STRING];
	int		i;

	ccmd_args[0] = 0;
	if (arg < 0)
		arg = 0;
	for ( i = arg ; i < ccmd_argc ; i++ ) {
		strcat( ccmd_args, ccmd_argv[i] );
		if ( i != ccmd_argc-1 ) {
			strcat( ccmd_args, " " );
		}
	}

	return ccmd_args;
}

/*
============
Cmd_ArgsBuffer
============
*/
void	CCmd_ArgsBuffer( char *buffer, int bufferLength ) {
	Q_strncpyz( buffer, CCmd_Args(), bufferLength );
}

/*
============
CCmd_Cmd
============
*/
char *CCmd_Cmd()
{
	return ccmd_cmd;
}

/*
============
CCmd_TokenizeString

Parses the given string into command line tokens.
The text is copied to a seperate buffer and 0 characters
are inserted in the apropriate place, The argv array
will point into this temporary buffer.
============
*/

void CCmd_TokenizeString( const char *text_in ) {
	const char	*text;
	char	*textOut;

	// clear previous args
	ccmd_argc = 0;

	if ( !text_in ) {
		return;
	}
	
	Q_strncpyz( ccmd_cmd, text_in, sizeof(ccmd_cmd) );

	text = text_in;
	textOut = ccmd_tokenized;

	while ( 1 ) {
		if ( ccmd_argc == MAX_STRING_TOKENS ) {
			return;			// this is usually something malicious
		}

		while ( 1 ) {
			// skip whitespace
			while ( *text && *text <= ' ' ) {
				text++;
			}
			if ( !*text ) {
				return;			// all tokens parsed
			}

			// skip // comments
			if ( text[0] == '/' && text[1] == '/' ) {
				return;			// all tokens parsed
			}

			// skip /* */ comments
			if ( text[0] == '/' && text[1] =='*' ) {
				while ( *text && ( text[0] != '*' || text[1] != '/' ) ) {
					text++;
				}
				if ( !*text ) {
					return;		// all tokens parsed
				}
				text += 2;
			} else {
				break;			// we are ready to parse a token
			}
		}

		// handle quoted strings
    // NOTE TTimo this doesn't handle \" escaping
		if ( *text == '"' ) {
			ccmd_argv[ccmd_argc] = textOut;
			ccmd_argc++;
			text++;
			while ( *text && *text != '"' ) {
				*textOut++ = *text++;
			}
			*textOut++ = 0;
			if ( !*text ) {
				return;		// all tokens parsed
			}
			text++;
			continue;
		}

		// regular token
		ccmd_argv[ccmd_argc] = textOut;
		ccmd_argc++;

		// skip until whitespace, quote, or command
		while ( *text > ' ' ) {
			if ( text[0] == '"' ) {
				break;
			}

			if ( text[0] == '/' && text[1] == '/' ) {
				break;
			}

			// skip /* */ comments
			if ( text[0] == '/' && text[1] =='*' ) {
				break;
			}

			*textOut++ = *text++;
		}

		*textOut++ = 0;

		if ( !*text ) {
			return;		// all tokens parsed
		}
	}
	
}


char *CopyString( const char *in ) {
	char	*out;

	out = (char *)malloc(strlen(in)+1);
	strcpy (out, in);
	return out;
}

/*
============
Cmd_AddCommand
============
*/
void	CCmd_AddCommand( const char *cmd_name, xccommand_t function ) {
	ccmd_function_t	*cmd;
	
	if (!cmd_name || !function) {	// Dont allow nameless/functionless commands
		return;
	}
	// fail if the command already exists
	for ( cmd = ccmd_functions ; cmd ; cmd=cmd->next ) 
	{
		if ( !strcmp( cmd_name, cmd->name ) ) {
			Com_Printf ("CCmd_AddCommand: %s already defined\n", cmd_name);
			return;
		}
	}

	// use a small malloc to avoid zone fragmentation
	cmd = (ccmd_function_t*)malloc(sizeof(ccmd_function_t));
	if (!cmd)
	{
		Com_Printf("CCmd_AddCommand: null cmd!\n");
		free(cmd);
		return;
	}
	cmd->name = CopyString( cmd_name );
	cmd->function = function;
	cmd->next = ccmd_functions;
	ccmd_functions = cmd;
}

/*
============
CCmd_RemoveCommand
============
*/
void	CCmd_RemoveCommand( const char *cmd_name ) {
	ccmd_function_t	*cmd, **back;

	back = &ccmd_functions;
	while( 1 ) {
		cmd = *back;
		if ( !cmd ) {
			// command wasn't active
			return;
		}
		if ( !strcmp( cmd_name, cmd->name ) ) {
			*back = cmd->next;
			if (cmd->name) {
				free(cmd->name);
			}
			free(cmd);
			return;
		}
		back = &cmd->next;
	}
}


int GLua_ChatCommand(int clientNum, const char *cmd);
// NOTE: This function expects the / or \ to be removed!

static const char *ChatBox_UnescapeChat(const char *message) {
	static char buff[1024] = {0};
	char *s, *t;
	char *cutoff = &buff[1023];
	s = &buff[0];
	t = ( char * ) message;
	while (*t && s != cutoff) {
		if (*t == 0x18) {
			*s = '%';
		} else if (*t == 0x17) {
			*s = '"';
		} else {
			*s = *t;
		}
		t++; s++;
	}
	*s = 0;
	return &buff[0];
}

qboolean CCmd_Execute(const char *command) {
	ccmd_function_t	*cmd;

	if (!command || !command[0]) {	// Dont bother handling a bad command
		return qfalse;
	}

	CCmd_TokenizeString(ChatBox_UnescapeChat(command));

	// fail if the command already exists
	for ( cmd = ccmd_functions ; cmd ; cmd=cmd->next ) {
		if ( !strcmp( CCmd_Argv(0), cmd->name ) ) {
			// Found it
			cmd->function();
			return qtrue;
		}
	}

	return qfalse;
}