/*
===========================================================================
Copyright (C) 1999 - 2005, Id Software, Inc.
Copyright (C) 2000 - 2013, Raven Software, Inc.
Copyright (C) 2001 - 2013, Activision, Inc.
Copyright (C) 2005 - 2015, ioquake3 contributors
Copyright (C) 2013 - 2015, OpenJK contributors

This file is part of the OpenJK source code.

OpenJK is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, see <http://www.gnu.org/licenses/>.
===========================================================================
*/

// q_shared.c -- stateless support routines that are included in each code dll
#include "q_shared.h"

#include "../game/jkg_gangwars.h"
gangWarsTeam_t bgGangWarsTeams[32];
int bgnumGangWarTeams;

#if defined(_GAME) || defined(_CGAME) || defined(IN_UI)
void (*Com_Error)( int level, const char *error, ... );
void (*Com_Printf)( const char *msg, ... );
#endif

/**************************************************
* Q_stratt
*
* Attaches a string to the destination buffer, returns
* qtrue on success or qfalse on failure (buffer full).
**************************************************/

qboolean Q_stratt( char *dest, unsigned int iSize, char *source )
{
	if ( strlen( dest ) + strlen( source ) > iSize )
	{
		return qfalse;
	}

	strcat( dest, source );
	return qtrue;
}


/*
-------------------------
GetIDForString
-------------------------
*/


int GetIDForString ( stringID_table_t *table, const char *string )
{
	int	index = 0;

	while ( ( table[index].name != NULL ) &&
			( table[index].name[0] != 0 ) )
	{
		if ( !Q_stricmp( table[index].name, string ) )
			return table[index].id;

		index++;
	}

	return -1;
}

#include <stdio.h>
#include <ctype.h>

qboolean StringContainsWord(const char *haystack, const char *needle)
{
	if ( !*needle )
	{
		return qfalse;
	}
	for ( ; *haystack; ++haystack )
	{
		if ( toupper(*haystack) == toupper(*needle) )
		{
			/*
			* Matched starting char -- loop through remaining chars.
			*/
			const char *h, *n;
			for ( h = haystack, n = needle; *h && *n; ++h, ++n )
			{
				if ( toupper(*h) != toupper(*n) )
				{
					break;
				}
			}
			if ( !*n ) /* matched all of 'needle' to null termination */
			{
				return qtrue; /* return the start of the match */
			}
		}
	}
	return qfalse;
}

/*
-------------------------
GetStringForID
-------------------------
*/

const char *GetStringForID( stringID_table_t *table, int id )
{
	int	index = 0;

	while ( ( table[index].name != NULL ) &&
			( table[index].name[0] != 0 ) )
	{
		if ( table[index].id == id )
			return table[index].name;

		index++;
	}

	return NULL;
}

int Com_Clampi( int min, int max, int value )
{
	if ( value < min )
	{
		return min;
	}
	if ( value > max )
	{
		return max;
	}
	return value;
}

float Com_Clamp( float min, float max, float value ) {
	if ( value < min ) {
		return min;
	}
	if ( value > max ) {
		return max;
	}
	return value;
}

int Com_AbsClampi( int min, int max, int value )
{
	if( value < 0 )
	{
		return Com_Clampi( -max, -min, value );
	}
	else
	{
		return Com_Clampi( min, max, value );
	}
}

float Com_AbsClamp( float min, float max, float value )
{
	if( value < 0.0f )
	{
		return Com_Clamp( -max, -min, value );
	}
	else
	{
		return Com_Clamp( min, max, value );
	}
}


/*
============
COM_SkipPath
============
*/
char *COM_SkipPath (char *pathname)
{
	char	*last;

	last = pathname;
	while (*pathname)
	{
		if (*pathname=='/')
			last = pathname+1;
		pathname++;
	}
	return last;
}

/*
============
COM_GetExtension
============
*/
const char *COM_GetExtension( const char *name )
{
	const char *dot = strrchr(name, '.'), *slash;
	if (dot && (!(slash = strrchr(name, '/')) || slash < dot))
		return dot + 1;
	else
		return "";
}

/*
============
COM_StripExtension
============
*/
void COM_StripExtension( const char *in, char *out, int destsize )
{
	const char *dot = strrchr(in, '.'), *slash;
	if (dot && (!(slash = strrchr(in, '/')) || slash < dot))
		destsize = (destsize < dot-in+1 ? destsize : dot-in+1);

	if ( in == out && destsize > 1 )
		out[destsize-1] = '\0';
	else
		Q_strncpyz(out, in, destsize);
}

/*
============
COM_CompareExtension

string compare the end of the strings and return qtrue if strings match
============
*/
qboolean COM_CompareExtension(const char *in, const char *ext)
{
	int inlen, extlen;

	inlen = strlen(in);
	extlen = strlen(ext);

	if(extlen <= inlen)
	{
		in += inlen - extlen;

		if(!Q_stricmp(in, ext))
			return qtrue;
	}

	return qfalse;
}

/*
==================
COM_DefaultExtension
==================
*/
void COM_DefaultExtension( char *path, int maxSize, const char *extension )
{
	const char *dot = strrchr(path, '.'), *slash;
	if (dot && (!(slash = strrchr(path, '/')) || slash < dot))
		return;
	else
		Q_strcat(path, maxSize, extension);
}

/*
============================================================================

					BYTE ORDER FUNCTIONS

============================================================================
*/
/*
// can't just use function pointers, or dll linkage can
// mess up when qcommon is included in multiple places
static short	(*_BigShort) (short l);
static short	(*_LittleShort) (short l);
static int		(*_BigLong) (int l);
static int		(*_LittleLong) (int l);
static qint64	(*_BigLong64) (qint64 l);
static qint64	(*_LittleLong64) (qint64 l);
static float	(*_BigFloat) (const float *l);
static float	(*_LittleFloat) (const float *l);

short	BigShort(short l){return _BigShort(l);}
short	LittleShort(short l) {return _LittleShort(l);}
int		BigLong (int l) {return _BigLong(l);}
int		LittleLong (int l) {return _LittleLong(l);}
qint64 	BigLong64 (qint64 l) {return _BigLong64(l);}
qint64 	LittleLong64 (qint64 l) {return _LittleLong64(l);}
float	BigFloat (const float *l) {return _BigFloat(l);}
float	LittleFloat (const float *l) {return _LittleFloat(l);}
*/

void CopyShortSwap( void *dest, void *src )
{
	byte *to = (byte *)dest, *from = (byte *)src;

	to[0] = from[1];
	to[1] = from[0];
}

void CopyLongSwap( void *dest, void *src )
{
	byte *to = (byte *)dest, *from = (byte *)src;

	to[0] = from[3];
	to[1] = from[2];
	to[2] = from[1];
	to[3] = from[0];
}

short   ShortSwap (short l)
{
	byte    b1,b2;

	b1 = l&255;
	b2 = (l>>8)&255;

	return (b1<<8) + b2;
}

short	ShortNoSwap (short l)
{
	return l;
}

int    LongSwap (int l)
{
	byte    b1,b2,b3,b4;

	b1 = l&255;
	b2 = (l>>8)&255;
	b3 = (l>>16)&255;
	b4 = (l>>24)&255;

	return ((int)b1<<24) + ((int)b2<<16) + ((int)b3<<8) + b4;
}

int	LongNoSwap (int l)
{
	return l;
}

qint64 Long64Swap (qint64 ll)
{
	qint64	result;

	result.b0 = ll.b7;
	result.b1 = ll.b6;
	result.b2 = ll.b5;
	result.b3 = ll.b4;
	result.b4 = ll.b3;
	result.b5 = ll.b2;
	result.b6 = ll.b1;
	result.b7 = ll.b0;

	return result;
}

qint64 Long64NoSwap (qint64 ll)
{
	return ll;
}

float FloatSwap (const float *f) {
	byteAlias_t out;

	out.f = *f;
	out.ui = LongSwap(out.ui);

	return out.f;
}

float FloatNoSwap (const float *f)
{
	return *f;
}

/*
================
Swap_Init
================
*/
/*
void Swap_Init (void)
{
	byte	swaptest[2] = {1,0};

// set the byte swapping variables in a portable manner
	if ( *(short *)swaptest == 1)
	{
		_BigShort = ShortSwap;
		_LittleShort = ShortNoSwap;
		_BigLong = LongSwap;
		_LittleLong = LongNoSwap;
		_BigLong64 = Long64Swap;
		_LittleLong64 = Long64NoSwap;
		_BigFloat = FloatSwap;
		_LittleFloat = FloatNoSwap;
	}
	else
	{
		_BigShort = ShortNoSwap;
		_LittleShort = ShortSwap;
		_BigLong = LongNoSwap;
		_LittleLong = LongSwap;
		_BigLong64 = Long64NoSwap;
		_LittleLong64 = Long64Swap;
		_BigFloat = FloatNoSwap;
		_LittleFloat = FloatSwap;
	}

}
*/

/*
============================================================================

PARSING

============================================================================
*/

static	char	com_token[MAX_TOKEN_CHARS];
static	char	com_parsename[MAX_TOKEN_CHARS];
static	int		com_lines;
static	int		com_tokenline;

void COM_BeginParseSession( const char *name )
{
	com_lines = 1;
	com_tokenline = 0;
	Com_sprintf(com_parsename, sizeof(com_parsename), "%s", name);
}

int COM_GetCurrentParseLine( void )
{
	if ( com_tokenline )
	{
		return com_tokenline;
	}

	return com_lines;
}

char *COM_Parse( const char **data_p )
{
	return COM_ParseExt( data_p, qtrue );
}

void COM_ParseError( char *format, ... )
{
	va_list argptr;
	static char string[4096];

	va_start (argptr, format);
	Q_vsnprintf (string, sizeof( string ), format, argptr);
	va_end (argptr);

	Com_Printf("ERROR: %s, line %d: %s\n", com_parsename, COM_GetCurrentParseLine(), string);
}

void COM_ParseWarning( char *format, ... )
{
	va_list argptr;
	static char string[4096];

	va_start (argptr, format);
	Q_vsnprintf (string, sizeof(string), format, argptr);
	va_end (argptr);

	Com_Printf("WARNING: %s, line %d: %s\n", com_parsename, COM_GetCurrentParseLine(), string);
}

/*
==============
COM_Parse

Parse a token out of a string
Will never return NULL, just empty strings

If "allowLineBreaks" is qtrue then an empty
string will be returned if the next token is
a newline.
==============
*/
const char *SkipWhitespace( const char *data, qboolean *hasNewLines ) {
	int c;

	while( (c = *(const unsigned char* /*eurofix*/)data) <= ' ') {
		if( !c ) {
			return NULL;
		}
		if( c == '\n' ) {
			com_lines++;
			*hasNewLines = qtrue;
		}
		data++;
	}

	return data;
}

int COM_Compress( char *data_p ) {
	char *in, *out;
	int c;
	qboolean newline = qfalse, whitespace = qfalse;

	in = out = data_p;
	if (in) {
		while ((c = *in) != 0) {
			// skip double slash comments
			if ( c == '/' && in[1] == '/' ) {
				while (*in && *in != '\n') {
					in++;
				}
			// skip /* */ comments
			} else if ( c == '/' && in[1] == '*' ) {
				while ( *in && ( *in != '*' || in[1] != '/' ) )
					in++;
				if ( *in )
					in += 2;
				// record when we hit a newline
			} else if ( c == '\n' || c == '\r' ) {
				newline = qtrue;
				in++;
				// record when we hit whitespace
			} else if ( c == ' ' || c == '\t') {
				whitespace = qtrue;
				in++;
				// an actual token
			} else {
				// if we have a pending newline, emit it (and it counts as whitespace)
				if (newline) {
					*out++ = '\n';
					newline = qfalse;
					whitespace = qfalse;
				} if (whitespace) {
					*out++ = ' ';
					whitespace = qfalse;
				}

				// copy quoted strings unmolested
				if (c == '"') {
					*out++ = c;
					in++;
					while (1) {
						c = *in;
						if (c && c != '"') {
							*out++ = c;
							in++;
						} else {
							break;
						}
					}
					if (c == '"') {
						*out++ = c;
						in++;
					}
				} else {
					*out = c;
					out++;
					in++;
				}
			}
		}

		*out = 0;
	}
	return out - data_p;
}

char *COM_ParseExt( const char **data_p, qboolean allowLineBreaks )
{
	int c = 0, len;
	qboolean hasNewLines = qfalse;
	const char *data;

	data = *data_p;
	len = 0;
	com_token[0] = 0;
	com_tokenline = 0;

	// make sure incoming data is valid
	if ( !data )
	{
		*data_p = NULL;
		return com_token;
	}

	while ( 1 )
	{
		// skip whitespace
		data = SkipWhitespace( data, &hasNewLines );
		if ( !data )
		{
			*data_p = NULL;
			return com_token;
		}
		if ( hasNewLines && !allowLineBreaks )
		{
			*data_p = data;
			return com_token;
		}

		c = *data;

		// skip double slash comments
		if ( c == '/' && data[1] == '/' )
		{
			data += 2;
			while (*data && *data != '\n') {
				data++;
			}
		}
		// skip /* */ comments
		else if ( c=='/' && data[1] == '*' )
		{
			data += 2;
			while ( *data && ( *data != '*' || data[1] != '/' ) )
			{
				if ( *data == '\n' )
				{
					com_lines++;
				}
				data++;
			}
			if ( *data )
			{
				data += 2;
			}
		}
		else
		{
			break;
		}
	}

	// token starts on this line
	com_tokenline = com_lines;

	// handle quoted strings
	if (c == '\"')
	{
		data++;
		while (1)
		{
			c = *data++;
			if (c=='\"' || !c)
			{
				com_token[len] = 0;
				*data_p = ( char * ) data;
				return com_token;
			}
			if ( c == '\n' )
			{
				com_lines++;
			}
			if (len < MAX_TOKEN_CHARS - 1)
			{
				com_token[len] = c;
				len++;
			}
		}
	}

	// parse a regular word
	do
	{
		if (len < MAX_TOKEN_CHARS - 1)
		{
			com_token[len] = c;
			len++;
		}
		data++;
		c = *data;
	} while (c>32);

	com_token[len] = 0;

	*data_p = ( char * ) data;
	return com_token;
}

/*
===============
COM_ParseString
===============
*/
qboolean COM_ParseString( const char **data, const char **s )
{
//	*s = COM_ParseExt( data, qtrue );
	*s = COM_ParseExt( data, qfalse );
	if ( s[0] == 0 )
	{
		COM_ParseWarning( "COM_ParseString: unexpected EOF" );
		return qtrue;
	}
	return qfalse;
}

/*
===============
COM_ParseInt
===============
*/
qboolean COM_ParseInt( const char **data, int *i )
{
	const char	*token;

	token = COM_ParseExt( data, qfalse );
	if ( token[0] == 0 )
	{
		COM_ParseWarning( "COM_ParseInt: unexpected EOF" );
		return qtrue;
	}

	*i = atoi( token );
	return qfalse;
}

/*
===============
COM_ParseFloat
===============
*/
qboolean COM_ParseFloat( const char **data, float *f )
{
	const char	*token;

	token = COM_ParseExt( data, qfalse );
	if ( token[0] == 0 )
	{
		COM_ParseWarning( "COM_ParseFloat: unexpected EOF" );
		return qtrue;
	}

	*f = atof( token );
	return qfalse;
}

/*
===============
COM_ParseVec4
===============
*/
qboolean COM_ParseVec4( const char **buffer, vec4_t *c)
{
	int i;
	float f;

	for (i = 0; i < 4; i++)
	{
		if (COM_ParseFloat(buffer, &f))
		{
			return qtrue;
		}
		(*c)[i] = f;
	}
	return qfalse;
}

/*
==================
COM_MatchToken
==================
*/
void COM_MatchToken( const char **buf_p, char *match ) {
	char	*token;

	token = COM_Parse( buf_p );
	if ( strcmp( token, match ) ) {
		Com_Error( ERR_DROP, "MatchToken: %s != %s", token, match );
	}
}


/*
=================
SkipBracedSection

The next token should be an open brace or set depth to 1 if already parsed it.
Skips until a matching close brace is found.
Internal brace depths are properly skipped.
=================
*/
qboolean SkipBracedSection (const char **program, int depth) {
	char			*token;

	do {
		token = COM_ParseExt( program, qtrue );
		if( token[1] == 0 ) {
			if( token[0] == '{' ) {
				depth++;
			}
			else if( token[0] == '}' ) {
				depth--;
			}
		}
	} while( depth && *program );

	return (qboolean)( depth == 0 );
}

/*
=================
SkipRestOfLine
=================
*/
void SkipRestOfLine ( const char **data ) {
	const char	*p;
	int		c;

	p = *data;

	if ( !*p )
		return;

	while ( (c = *p++) != 0 ) {
		if ( c == '\n' ) {
			com_lines++;
			break;
		}
	}

	*data = p;
}


void Parse1DMatrix (const char **buf_p, int x, float *m) {
	char	*token;
	int		i;

	COM_MatchToken( buf_p, "(" );

	for (i = 0 ; i < x ; i++) {
		token = COM_Parse(buf_p);
		m[i] = atof(token);
	}

	COM_MatchToken( buf_p, ")" );
}

void Parse2DMatrix (const char **buf_p, int y, int x, float *m) {
	int		i;

	COM_MatchToken( buf_p, "(" );

	for (i = 0 ; i < y ; i++) {
		Parse1DMatrix (buf_p, x, m + i * x);
	}

	COM_MatchToken( buf_p, ")" );
}

void Parse3DMatrix (const char **buf_p, int z, int y, int x, float *m) {
	int		i;

	COM_MatchToken( buf_p, "(" );

	for (i = 0 ; i < z ; i++) {
		Parse2DMatrix (buf_p, y, x, m + i * x*y);
	}

	COM_MatchToken( buf_p, ")" );
}

/*
===================
Com_HexStrToInt
===================
*/
int Com_HexStrToInt( const char *str )
{
	if ( !str || !str[ 0 ] )
		return -1;

	// check for hex code
	if( str[ 0 ] == '0' && str[ 1 ] == 'x' )
	{
		int  n = 0;
		size_t i;

		for( i = 2; i < strlen( str ); i++ )
		{
			char digit;

			n *= 16;

			digit = tolower( str[ i ] );

			if( digit >= '0' && digit <= '9' )
				digit -= '0';
			else if( digit >= 'a' && digit <= 'f' )
				digit = digit - 'a' + 10;
			else
				return -1;

			n += digit;
		}

		return n;
	}

	return -1;
}

/*
============================================================================

					LIBRARY REPLACEMENT FUNCTIONS

============================================================================
*/

int Q_isprint( int c )
{
	if ( c >= 0x20 && c <= 0x7E )
		return ( 1 );
	return ( 0 );
}

int Q_isprintext( int c )
{
	if ( c >= 0x20 && c <= 0x7E )
		return (1);
	if ( c >= 0x80 && c <= 0xFE )
		return (1);
	return (0);
}

int Q_isgraph( int c )
{
	if ( c >= 0x21 && c <= 0x7E )
		return (1);
	if ( c >= 0x80 && c <= 0xFE )
		return (1);
	return (0);
}

int Q_islower( int c )
{
	if (c >= 'a' && c <= 'z')
		return ( 1 );
	return ( 0 );
}

int Q_isupper( int c )
{
	if (c >= 'A' && c <= 'Z')
		return ( 1 );
	return ( 0 );
}

int Q_isalpha( int c )
{
	if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
		return ( 1 );
	return ( 0 );
}

qboolean Q_isanumber( const char *s )
{
	char *p;
	double ret;

	if( *s == '\0' )
		return qfalse;

	ret = strtod( s, &p );

	if ( ret == HUGE_VAL || errno == ERANGE )
		return qfalse;

	return (qboolean)(*p == '\0');
}

qboolean Q_isintegral( float f )
{
	return (qboolean)( (int)f == f );
}

char* Q_strrchr( const char* string, int c )
{
	char cc = c;
	char *s;
	char *sp=(char *)0;

	s = (char*)string;

	while (*s)
	{
		if (*s == cc)
			sp = s;
		s++;
	}
	if (cc == 0)
		sp = s;

	return sp;
}

/*
=============
Q_strncpyz

Safe strncpy that ensures a trailing zero
=============
*/
void Q_strncpyz( char *dest, const char *src, int destsize ) {
	if ( !dest ) {
		Com_Error( ERR_FATAL, "Q_strncpyz: NULL dest" );
		return;
	}
	if ( !src ) {
		Com_Error( ERR_FATAL, "Q_strncpyz: NULL src" );
		return;
	}
	if ( destsize < 1 ) {
		Com_Error( ERR_FATAL,"Q_strncpyz: destsize < 1" );
		return;
	}

	strncpy( dest, src, destsize-1 );
	dest[destsize-1] = 0;
}

int Q_stricmpn (const char *s1, const char *s2, int n) {
	int		c1, c2;

	if ( s1 == NULL ) {
		if ( s2 == NULL )
			return 0;
		else
			return -1;
	}
	else if ( s2==NULL )
		return 1;

	do {
		c1 = *s1++;
		c2 = *s2++;

		if (!n--) {
			return 0;		// strings are equal until end point
		}

		if (c1 != c2) {
			if (c1 >= 'a' && c1 <= 'z') {
				c1 -= ('a' - 'A');
			}
			if (c2 >= 'a' && c2 <= 'z') {
				c2 -= ('a' - 'A');
			}
			if (c1 != c2) {
				return c1 < c2 ? -1 : 1;
			}
		}
	} while (c1);

	return 0;		// strings are equal
}

int Q_strncmp (const char *s1, const char *s2, int n) {
	int		c1, c2;

	do {
		c1 = *s1++;
		c2 = *s2++;

		if (!n--) {
			return 0;		// strings are equal until end point
		}

		if (c1 != c2) {
			return c1 < c2 ? -1 : 1;
		}
	} while (c1);

	return 0;		// strings are equal
}

int Q_stricmp (const char *s1, const char *s2) {
	return (s1 && s2) ? Q_stricmpn (s1, s2, 99999) : -1;
}

char *Q_strlwr( char *s1 ) {
    char	*s;

    s = s1;
	while ( *s ) {
		*s = tolower(*s);
		s++;
	}
    return s1;
}

char *Q_strupr( char *s1 ) {
    char	*s;

    s = s1;
	while ( *s ) {
		*s = toupper(*s);
		s++;
	}
    return s1;
}


// never goes past bounds or leaves without a terminating 0
void Q_strcat( char *dest, int size, const char *src ) {
	int		l1;

	l1 = strlen( dest );
	if ( l1 >= size ) {
		Com_Error( ERR_FATAL, "Q_strcat: already overflowed" );
	}
	Q_strncpyz( dest + l1, src, size - l1 );
}

/*
* Find the first occurrence of find in s.
*/
const char *Q_stristr( const char *s, const char *find ) {
	char c, sc;
	size_t len;

	if ((c = *find++) != 0)
	{
		if (c >= 'a' && c <= 'z')
		{
			c -= ('a' - 'A');
		}
		len = strlen(find);
		do
		{
			do
			{
				if ((sc = *s++) == 0)
					return NULL;
				if (sc >= 'a' && sc <= 'z')
				{
					sc -= ('a' - 'A');
				}
			} while (sc != c);
		} while (Q_stricmpn(s, find, len) != 0);
		s--;
	}
	return s;
}

int Q_PrintStrlen( const char *string ) {
	int			len;
	const char	*p;

	if( !string ) {
		return 0;
	}

	len = 0;
	p = string;
	while( *p ) {
		if( Q_IsColorString( p ) ) {
			p += 2;
			continue;
		}
		p++;
		len++;
	}

	return len;
}


/* This function modifies INPUT (is mutable) */
char *Q_CleanStr( char *string ) {
	char*	d;
	char*	s;
	int		c;

	s = string;
	d = string;
	while ((c = *s) != 0 ) {
		// eezstreet fix: deal with non standard color codes as well..
		/*if( s[0] == '^' && s[1] == 'x' &&
			((s[2] >= '0' && s[2] <= '9') ||
			(s[2] >= 'a' && s[2] <= 'f') ||
			(s[2] >= 'A' && s[2] <= 'F')) &&
			((s[3] >= '0' && s[3] <= '9') ||
			(s[3] >= 'a' && s[3] <= 'f') ||
			(s[3] >= 'A' && s[3] <= 'F')) &&
			((s[4] >= '0' && s[4] <= '9') ||
			(s[4] >= 'a' && s[4] <= 'f') ||
			(s[4] >= 'A' && s[4] <= 'F')) )*/
		if (*s == '^' && (s[1] == 'X' || s[1] == 'x') && (ExtColor_IsValid(s[2]) && ExtColor_IsValid(s[3]) && ExtColor_IsValid(s[4])) )	//--futuza this is cleaner and better, cuz we can use either x/X
		{
			s+=4;
		}
		else if ( Q_IsColorString( s ) ) {
			s++;
		}
		else if ( c >= 0x20 && c <= 0x7E ) {
			*d++ = c;
		}
		s++;
	}
	*d = '\0';

	return string;
}


/*
==================
Q_StripColor

Strips coloured strings in-place using multiple passes: "fgs^^56fds" -> "fgs^6fds" -> "fgsfds"

This function modifies INPUT (is mutable)

(Also strips ^8 and ^9)

//--Futuza todo: simplify this whole damn function
==================
*/
void Q_StripColor(char *text)
{
	/*
	qboolean doPass = qtrue;
	char *read;
	char *write;

	while ( doPass )
	{
		doPass = qfalse;
		read = write = text;
		while ( *read )
		{
			if ( Q_IsColorStringExt(read) )		//if (*read == '^' && *read + 1 >= '0' && *read + 1 <= '9')	//--futuza: explaining the damn function, because it is fracking unreadable
			{
				doPass = qtrue;
				read += 2;
			}

			if (*read == '^' && (read[1] == 'X' || read[1] == 'x') && (ExtColor_IsValid(read[2]) && ExtColor_IsValid(read[3]) && ExtColor_IsValid(read[4])))	//if ^xRGB (and a valid one)
			{
				doPass = qtrue;
				read += 5;
			}

			else
			{
				// Avoid writing the same data over itself
				if (write != read)
				{
					*write = *read;
				}
				write++;
				read++;
			}
		}
		if ( write < read )
		{
			// Add trailing NUL byte if string has shortened
			*write = '\0';
		}
	}*/
	Q_StripColor_Simple(text);	//just use simple version, original version is idiotic
}

//simplified Q_StripColor - courtesy of eezstreet
void Q_StripColor_Simple(char *text)
{
	int nOffset = 0;

	while (*text)	//while not null
	{
		if (*text == '^')	//if we encounter color code
		{
			if (text[1] == 'x' || text[1] == 'X')	//xRGB color codes
			{
				if ( ExtColor_IsValid(text[2]) && ExtColor_IsValid(text[3]) && ExtColor_IsValid(text[4]) ) //valid color?
					nOffset += 5;
			}
			else if ( isdigit(text[1]) )	//number color codes	--futuza: possibly unsafe to use isdigit? if so change to check 0-9 range instead
			{
				nOffset += 2;
			}
		}
		text[0] = text[nOffset];
		nOffset++;
		text++;
	}

}



/*
Q_strstrip

	Description:	Replace strip[x] in string with repl[x] or remove characters entirely
	Mutates:		string
	Return:			--

	Examples:		Q_strstrip( "Bo\nb is h\rairy!!", "\n\r!", "123" );	// "Bo1b is h2airy33"
					Q_strstrip( "Bo\nb is h\rairy!!", "\n\r!", "12" );	// "Bo1b is h2airy"
					Q_strstrip( "Bo\nb is h\rairy!!", "\n\r!", NULL );	// "Bob is hairy"
*/

void Q_strstrip( char *string, const char *strip, const char *repl )
{
	char		*out=string, *p=string, c;
	const char	*s=strip;
	int			replaceLen = repl?strlen( repl ):0, offset=0;
	qboolean	recordChar = qtrue;

	while ( (c = *p++) != '\0' )
	{
		recordChar = qtrue;
		for ( s=strip; *s; s++ )
		{
			offset = s-strip;
			if ( c == *s )
			{
				if ( !repl || offset >= replaceLen )
					recordChar = qfalse;
				else
					c = repl[offset];
				break;
			}
		}
		if ( recordChar )
			*out++ = c;
	}
	*out = '\0';
}

/*
Q_strchrs

	Description:	Find any characters in a string. Think of it as a shorthand strchr loop.
	Mutates:		--
	Return:			first instance of any characters found
					 otherwise NULL
*/

const char *Q_strchrs( const char *string, const char *search )
{
	const char *p = string, *s = search;

	while ( *p != '\0' )
	{
		for ( s=search; *s; s++ )
		{
			if ( *p == *s )
				return p;
		}
		p++;
	}

	return NULL;
}

#ifdef _MSC_VER
/*
=============
Q_vsnprintf

Special wrapper function for Microsoft's broken _vsnprintf() function.
MinGW comes with its own snprintf() which is not broken.
=============
*/

int Q_vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
	int retval;

	retval = _vsnprintf(str, size, format, ap);

	if(retval < 0 || retval == size)
	{
		// Microsoft doesn't adhere to the C99 standard of vsnprintf,
		// which states that the return value must be the number of
		// bytes written if the output string had sufficient length.
		//
		// Obviously we cannot determine that value from Microsoft's
		// implementation, so we have no choice but to return size.

		str[size - 1] = '\0';
		return size;
	}

	return retval;
}
#endif

int QDECL Com_sprintf( char *dest, int size, const char *fmt, ...) {
	int		len;
	va_list		argptr;

	va_start (argptr,fmt);
	len = Q_vsnprintf(dest, size, fmt, argptr);
	va_end (argptr);

	if(len >= size)
		Com_Printf("Com_sprintf: Output length %d too short, require %d bytes.\n", size, len + 1);

	return len;
}

int FloatAsInt( float f ) {
	byteAlias_t fi;
	fi.f = f;
	return fi.i;
}

/*
============
va

does a varargs printf into a temp buffer, so I don't need to have
varargs versions of all text functions.
FIXME: make this buffer size safe someday
============
*/
#define	MAX_VA_STRING	32000
#define MAX_VA_BUFFERS 4

char * QDECL va( const char *format, ... )
{
	va_list		argptr;
	static char	string[MAX_VA_BUFFERS][MAX_VA_STRING];	// in case va is called by nested functions
	static int	index = 0;
	char		*buf;

	va_start( argptr, format );
	buf = (char *)&string[index++ & 3];
	Q_vsnprintf( buf, sizeof(*string), format, argptr );
	va_end( argptr );

	return buf;
}

/*
============
Com_TruncateLongString

Assumes buffer is atleast TRUNCATE_LENGTH big
============
*/
void Com_TruncateLongString( char *buffer, const char *s ) {
	int length = strlen( s );

	if ( length <= TRUNCATE_LENGTH )
		Q_strncpyz( buffer, s, TRUNCATE_LENGTH );
	else {
		Q_strncpyz( buffer, s, (TRUNCATE_LENGTH/2) - 3 );
		Q_strcat( buffer, TRUNCATE_LENGTH, " ... " );
		Q_strcat( buffer, TRUNCATE_LENGTH, s + length - (TRUNCATE_LENGTH/2) + 3 );
	}
}

/*
=====================================================================

  INFO STRINGS

=====================================================================
*/

/*
===============
Info_ValueForKey

Searches the string for the given
key and returns the associated value, or an empty string.
FIXME: overflow check?
===============
*/
char *Info_ValueForKey( const char *s, const char *key ) {
	char	pkey[BIG_INFO_KEY];
	static	char value[2][BIG_INFO_VALUE];	// use two buffers so compares
											// work without stomping on each other
	static	int	valueindex = 0;
	char	*o;

	if ( !s || !key ) {
		return "";
	}

	if ( strlen( s ) >= BIG_INFO_STRING ) {
		Com_Error( ERR_DROP, "Info_ValueForKey: oversize infostring" );
	}

	valueindex ^= 1;
	if (*s == '\\')
		s++;
	while (1)
	{
		o = pkey;
		while (*s != '\\')
		{
			if (!*s)
				return "";
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value[valueindex];

		while (*s != '\\' && *s)
		{
			*o++ = *s++;
		}
		*o = 0;

		if (!Q_stricmp (key, pkey) )
			return value[valueindex];

		if (!*s)
			break;
		s++;
	}

	return "";
}

/*
===================
Info_NextPair

Used to itterate through all the key/value pairs in an info string
Return qfalse if we discover the infostring is invalid
===================
*/
qboolean Info_NextPair( const char **head, char *key, char *value ) {
	char *o;
	const char *s = *head;

	if ( *s == '\\' )
		s++;
	key[0] = 0;
	value[0] = 0;

	o = key;
	while ( *s != '\\' ) {
		if ( !*s ) {
			key[0] = 0;
			*head  = s;
			return qtrue;
		}
		*o++ = *s++;
	}
	*o = 0;
	s++;

	// If they key is empty at this point with a slash after it
	// then this is considered invalid, possibly an attempt at hacked userinfo strings
	if ( !key[0] )
		return qfalse;

	o = value;
	while ( *s != '\\' && *s ) {
		*o++ = *s++;
	}
	*o = 0;

	*head = s;

	return qtrue;
}

/*
===================
Info_RemoveKey
===================
*/
void Info_RemoveKey( char *s, const char *key ) {
	char	*start = NULL, *o = NULL;
	char	pkey[MAX_INFO_KEY] = {0};
	char	value[MAX_INFO_VALUE] = {0};

	if ( strlen( s ) >= MAX_INFO_STRING ) {
		Com_Error( ERR_DROP, "Info_RemoveKey: oversize infostring" );
		return;
	}

	if (strchr (key, '\\')) {
		return;
	}

	while (1)
	{
		start = s;
		if (*s == '\\')
			s++;
		o = pkey;
		while (*s != '\\')
		{
			if (!*s)
				return;
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value;
		while (*s != '\\' && *s)
		{
			if (!*s)
				return;
			*o++ = *s++;
		}
		*o = 0;

		//OJKNOTE: static analysis pointed out pkey may not be null-terminated
		if (!strcmp (key, pkey) )
		{
			memmove(start, s, strlen(s) + 1);	// remove this part
			return;
		}

		if (!*s)
			return;
	}
}

/*
===================
Info_RemoveKey_Big
===================
*/
void Info_RemoveKey_Big( char *s, const char *key ) {
	char	*start;
	static char	pkey[BIG_INFO_KEY], value[BIG_INFO_VALUE];
	char	*o;

	pkey[0] = value[0] = '\0';

	if ( strlen( s ) >= BIG_INFO_STRING ) {
		Com_Error( ERR_DROP, "Info_RemoveKey_Big: oversize infostring" );
		return;
	}

	if (strchr (key, '\\')) {
		return;
	}

	while (1)
	{
		start = s;
		if (*s == '\\')
			s++;
		o = pkey;
		while (*s != '\\')
		{
			if (!*s)
				return;
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value;
		while (*s != '\\' && *s)
		{
			if (!*s)
				return;
			*o++ = *s++;
		}
		*o = 0;

		//OJKNOTE: static analysis pointed out pkey may not be null-terminated
		if (!strcmp (key, pkey) )
		{
			memmove(start, s, strlen(s) + 1);	// remove this part
			return;
		}

		if (!*s)
			return;
	}

}

/*
==================
Info_Validate

Some characters are illegal in info strings because they
can mess up the server's parsing
==================
*/
qboolean Info_Validate( const char *s ) {
	const char *c = s;

	while ( *c != '\0' )
	{
		if( !Q_isprint( *c ) )
			return qfalse;

		if( *c == '\"' )
			return qfalse;

		if( *c == ';' )
			return qfalse;

		++c;
	}

	return qtrue;
}

/*
==================
Info_SetValueForKey

Changes or adds a key/value pair
==================
*/
void Info_SetValueForKey( char *s, const char *key, const char *value ) {
	char	newi[MAX_INFO_STRING];
	const char* blacklist = "\\;\"";

	if ( strlen( s ) >= MAX_INFO_STRING ) {
		Com_Error( ERR_DROP, "Info_SetValueForKey: oversize infostring" );
	}

	for(; *blacklist; ++blacklist)
	{
		if (strchr (key, *blacklist) || strchr (value, *blacklist))
		{
			Com_Printf (S_COLOR_YELLOW "Can't use keys or values with a '%c': %s = %s\n", *blacklist, key, value);
			return;
		}
	}

	Info_RemoveKey (s, key);
	if (!value || !strlen(value))
		return;

	Com_sprintf (newi, sizeof(newi), "\\%s\\%s", key, value);

	if (strlen(newi) + strlen(s) >= MAX_INFO_STRING)
	{
		Com_Printf ("Info string length exceeded\n");
		return;
	}

	strcat (newi, s);
	strcpy (s, newi);
}

/*
==================
Info_SetValueForKey_Big

Changes or adds a key/value pair
Includes and retains zero-length values
==================
*/
void Info_SetValueForKey_Big( char *s, const char *key, const char *value ) {
	char	newi[BIG_INFO_STRING];
	const char* blacklist = "\\;\"";

	if ( strlen( s ) >= BIG_INFO_STRING ) {
		Com_Error( ERR_DROP, "Info_SetValueForKey_Big: oversize infostring" );
	}

	for(; *blacklist; ++blacklist)
	{
		if (strchr (key, *blacklist) || strchr (value, *blacklist))
		{
			Com_Printf (S_COLOR_YELLOW "Can't use keys or values with a '%c': %s = %s\n", *blacklist, key, value);
			return;
		}
	}

	Info_RemoveKey_Big (s, key);
	if (!value)
		return;

	Com_sprintf (newi, sizeof(newi), "\\%s\\%s", key, value);

	if (strlen(newi) + strlen(s) >= BIG_INFO_STRING)
	{
		Com_Printf ("BIG Info string length exceeded\n");
		return;
	}

	strcat (s, newi);
}

/*
==================
Com_CharIsOneOfCharset
==================
*/
static qboolean Com_CharIsOneOfCharset( char c, char *set ) {
	size_t i;

	for ( i=0; i<strlen( set ); i++ ) {
		if ( set[i] == c )
			return qtrue;
	}

	return qfalse;
}

/*
==================
Com_SkipCharset
==================
*/
char *Com_SkipCharset( char *s, char *sep ) {
	char *p = s;

	while ( p ) {
		if ( Com_CharIsOneOfCharset( *p, sep ) )
			p++;
		else
			break;
	}

	return p;
}

/*
==================
Com_SkipTokens
==================
*/
char *Com_SkipTokens( char *s, int numTokens, char *sep ) {
	int sepCount = 0;
	char *p = s;

	while ( sepCount < numTokens ) {
		if ( Com_CharIsOneOfCharset( *p++, sep ) ) {
			sepCount++;
			while ( Com_CharIsOneOfCharset( *p, sep ) )
				p++;
		}
		else if ( *p == '\0' )
			break;
	}

	if ( sepCount == numTokens )
		return p;
	else
		return s;
}

qboolean Q_InBitflags( const uint32_t *bits, int index, uint32_t bitsPerByte ) {
	return ( bits[index / bitsPerByte] & (1 << (index % bitsPerByte)) ) ? qtrue : qfalse;
}

void Q_AddToBitflags( uint32_t *bits, int index, uint32_t bitsPerByte ) {
	bits[index / bitsPerByte] |= (1 << (index % bitsPerByte));
}

void Q_RemoveFromBitflags( uint32_t *bits, int index, uint32_t bitsPerByte ) {
	bits[index / bitsPerByte] &= ~(1 << (index % bitsPerByte));
}

//====================================================================

// JKG Add - Generic Memory Objects
// Creates a pool of memory which can be transformed and manipulated.

void JKG_NewGenericMemoryObject(GenericMemoryObject *gmo, size_t size)
{
	gmo->numElements = 0;
	
	gmo->elements = (void**)malloc(size);
	gmo->memAllocated = 1;
	gmo->elementSize = size;
}

void JKG_DeleteGenericMemoryObject(GenericMemoryObject *gmo)
{
	gmo->numElements = -2;
	free(gmo->elements);
}

void JKG_ClearGenericMemoryObject(GenericMemoryObject *gmo)
{
	gmo->numElements = 0;
}

void JKG_GenericMemoryObject_AddElement(GenericMemoryObject *gmo, void *element)
{
	if(gmo->memAllocated <= gmo->numElements+1)
	{
		gmo->memAllocated *= 2;
		void *elementcheck = realloc(gmo->elements, gmo->memAllocated*gmo->elementSize);
		if(!elementcheck) return;
		gmo->elements = (void **)elementcheck;
	}
	gmo->elements[gmo->numElements++] = element;
}

void JKG_GenericMemoryObject_DeleteElement(GenericMemoryObject *gmo, unsigned int number)
{
	gmo->elements[number] = NULL;
	memcpy(gmo->elements+number, gmo->elements+number+1, gmo->elementSize*(gmo->numElements-number));
	gmo->numElements--;
}

void Q_RGBCopy( vec4_t *output, vec4_t source )
{
	(*output)[0] = source[0];
	(*output)[1] = source[1];
	(*output)[2] = source[2];
}

qboolean Text_IsExtColorCode(const char *text) {
	const char *r, *g, *b;
	if ( strlen (text) < 4 )
	{
	    return qfalse;
	}
	
	r = text+1;
	g = text+2;
	b = text+3;
	// Get the color levels (if the numbers are invalid, it'll return -1, which we can use to validate)
	if ((*r < '0' || *r > '9') && (*r < 'a' || *r > 'f') && (*r < 'A' || *r > 'F')) {
		return qfalse;
	}
	if ((*g < '0' || *g > '9') && (*g < 'a' || *g > 'f') && (*g < 'A' || *g > 'F')) {
		return qfalse;
	}
	if ((*b < '0' || *b > '9') && (*b < 'a' || *b > 'f') && (*b < 'A' || *b > 'F')) {
		return qfalse;
	}
	return qtrue;
}


/*
====================================================================
//JKG Extended ^xRBG Color Codes adopted from jkg_chatbox.cpp
====================================================================  
So far only JKG_xRBG_ConvertExtToNormal() can be used.  Might add some more from jkg_chatbox.h later

*/

qboolean ExtColor_IsValid(char chr)
{
	if (chr >= '0' && chr <= '9')
		return qtrue;

	if (chr >= 'A' && chr <= 'F')
		return qtrue;

	if (chr >= 'a' && chr <= 'f')
		return qtrue;

	return qfalse;
}

static vec4_t tColorTable[10] = 
{
	{ 0, 0, 0, 1 }, // ^0
	{ 1, 0, 0, 1 }, // ^1
	{ 0, 1, 0, 1 }, // ^2
	{ 1, 1, 0, 1 }, // ^3
	{ 0, 0, 1, 1 }, // ^4
	{ 0, 1, 1, 1 }, // ^5
	{ 1, 0, 1, 1 }, // ^6
	{ 1, 1, 1, 1 }, // ^7
	{ 0, 0, 0, 1 }, // ^8
	{ 1, 0, 0, 1 }  // ^9
};

static float ExtColor_GetLevel(char chr) 
{
	if (chr >= '0' && chr <= '9') {
		return ((float)(chr - '0') / 15.0f);
	}
	if (chr >= 'A' && chr <= 'F') {
		return ((float)(chr - 'A' + 10) / 15.0f);
	}
	if (chr >= 'a' && chr <= 'f') {
		return ((float)(chr - 'a' + 10) / 15.0f);
	}
	return -1;
}

static int Text_ExtColorCodes(const char *text, vec4_t color) 
{
	const char *r, *g, *b;
	float red, green, blue;
	r = text + 1;
	g = text + 2;
	b = text + 3;
	// Get the color levels (if the numbers are invalid, it'll return -1, which we can use to validate)
	red = ExtColor_GetLevel(*r);
	green = ExtColor_GetLevel(*g);
	blue = ExtColor_GetLevel(*b);
	// Determine if all 3 are valid
	if (red == -1 || green == -1 || blue == -1) {
		return 0;
	}

	// We're clear to go, lets construct our color

	color[0] = red;
	color[1] = green;
	color[2] = blue;

	// HACK: Since cgame will use a palette override to implement dynamic opacity (like the chatbox)
	// we must ensure we use that alpha as well.
	// So copy the alpha of colorcode 0 (^0) instead of assuming 1.0

	//color[3] =*(float *)(0x56DF54 /*0x56DF48 + 12*/);
	color[3] = 1.0f;
	return 1;
}

// This function converts a text with extended color codes (^xRGB) into a text with normal color codes
// The extended colors will be clamped so the closest normal color available
// Used primarily to display text with extended colorcodes in the console

//futuza note:  I accidentally wrote this wrong should be xRGB not RBG oh well too late to change now.
char *JKG_xRBG_ConvertExtToNormal(const char *text)	//for converting ^xRBG names to regular ^1names, return a non-const
{
	static char buff[2048];
	const char *r;		// Reader
	char *w;			// Writer
	char *cutoff;
	vec4_t color;
	int hicolors;
	int i;
	r = text;
	w = buff;
	cutoff = &buff[2046];
	while (*r) {
		if (w >= cutoff) {
			// Time to stop, we reached the limit
			*w = 0;
			return &buff[0];
		}
		if (*r == '^' && (*(r + 1) == 'x' || *(r + 1) == 'X')) {
			if (Text_ExtColorCodes(r + 1, color)) {
				// Extended colorcode alright, determine which base color is closest to this one
				*w = *r;	// write the ^
				w++;
				r += 5;

				// Determine how many of the R G and B components are over 50%
				hicolors = 0;
				for (i = 0; i<3; i++) {
					if (color[i] >= 0.5f) {
						hicolors++;
					}
				}
				switch (hicolors) {
				case 0:
					// Color is black
					*w = '0';
					break;
				case 1:
					// It's a primary color, find out which
					if (color[0] >= 0.5f) {
						// It's red
						*w = '1';
					}
					else if (color[1] >= 0.5f) {
						// It's green
						*w = '2';
					}
					else {
						// Must be blue
						*w = '4';
					}
					break;
				case 2:
					// It's a secondary color, find out which
					if (color[0] >= 0.5f && color[1] >= 0.5f) {
						// It's yellow
						*w = '3';
					}
					else if (color[1] >= 0.5f && color[2] >= 0.5f) {
						// It's cyan
						*w = '5';
					}
					else {
						// Must be purple
						*w = '6';
					}
					break;
				case 3:
					// Color is white
					*w = '7';
					break;
				default:
					assert(0);	// Never happens, telling the compiler so
				}
				w++;
				continue;
			}
		}
		*w = *r;
		r++; w++;
	}
	*w = *r;	// Write the null terminator
	return &buff[0];
}

//didn't provide a limit value okay?
void Global_SanitizeString_MaxQPath(char *in, char *out)
{
	Global_SanitizeString(in, out, MAX_QPATH);
}

//--futuza: got sick of rewritting SanitizeString() functions, so here's a global one
void Global_SanitizeString(char *in, char *out, int limit)		//note: users can optionally pass in a limit value, rather then using the default
{
	int i = 0;
	int r = 0;

	while (in[i])
	{
		if (i >= limit - 1)		//the ui truncates the name here...
			break;

		if (in[i] == '^')
		{
			if (in[i + 1] == 'x' || in[i + 1] == 'X')	//if ^xRGB
			{
				if (ExtColor_IsValid(in[i + 2]) && ExtColor_IsValid(in[i + 3]) && ExtColor_IsValid(in[i + 4]))	//if a valid color
				{
					i += 5;
					continue;
				}
			}
			else if (isdigit(in[i + 1]))	//if ^0-9
			{
				i += 2;
				continue;
			}
			else	//treat ^'s without color codes like a normal char
				;
		}

		if (in[i] < 32)		//if a control character?
		{
			i++;
			continue;
		}

		out[r] = in[i];
		r++;
		i++;
	}
	out[r] = 0;

}

void getGalacticTimeStamp(char* outStr)	//to use : char myarray[17]; getBuildTimeStamp(myarray); 
{
	char result[17];
	time_t t = time(0);   // get time now
	struct tm * now = localtime(&t);

	//format current time (UTC)
	strftime(result, sizeof(result) - 1, "%y-%m-%d  %H:%M", now);

	//store results
	for (int i = 0; i<sizeof(result); i++)
		outStr[i] = result[i];

	return;
}