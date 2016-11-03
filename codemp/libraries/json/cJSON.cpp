/*
	JSON framework for C

	Copyright (c) 2010 Lourens "BobaFett" Elzinga

	Based on cJSON by Dave Gamble

    Changes:

  * Cleaned up code formatting to make it much more readable
  * Renamed print to serialize
  * Serializer now uses a stringbuilder instead of a tremendous amount of mallocs
  * Added usage of dynamic arrays and hashtables to drastically speed up lookup times for arrays and objects
  * JSON Type #defines are now internal
  * Array and Object functions now contain sanity checks (to ensure an array/object is being used)
  * cJSON struct is now internal, and is typedef'd as void
  * Replaced all instances of sprintf by snprintf to remove the risk of overflows (#defined as sprintf_s for windows compiles)
  * Added functions to obtain item values as a specific type (with default value in case of an error or incompatible type)
  * Added functions to determine the type of an item
  * Removed 'references'. They are unsafe and not very useful.
  * Added item duplication
  * Added new create functions (for booleans and integers)
  * The string serializer now supports unprintable characters ( < ANSI 32 without \x equivalent )
  * Deleting linked nodes is no longer possible
  * Added a safe version of cJSON_Delete which also clears the pointer if deleting was successful
  * Added function to insert items in arrays
  * Added function to swap 2 items in arrays
  * Added functions to clear arrays and objects
  * Added extended lookup, to allow retreival of deeply nested items in 1 call
  * Implemented iterator functions to iterate over objects (and arrays)
  * The parser now uses a stream buffer to track the current position
  * Errors during parsing now produce a proper error instead of silently failing
  * Fixed memory leaks in parser
  * The parser supports C-style comments (both single-line and block comments)
  * The string parser and serializer is now properly UTF-8 aware, and will encode and decode characters properly (non-ANSI chars are converted to '?')
  * The parser now supports memory pooling, which dramatically increases parse and free time (at the cost of the result being read-only)

*/

// cJSON
// JSON parser in C.

// Sprintf security fix
// All instances of sprintf are change to snprintf to avoid buffer overflows
// On windows, it's called sprintf_s instead of snprintf, so we'll do a little define here
#ifdef WIN32
#if (_MSC_VER < 1900)		//No longer necessary in vs2015  --Futuza
#define snprintf sprintf_s
#endif
#endif

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <float.h>
#include <limits.h>
#include <ctype.h>
#include <stdarg.h>
#include <assert.h>

#define __cJSON_INTERNAL

// cJSON Types:
#define cJSON_False 0
#define cJSON_True 1
#define cJSON_NULL 2
#define cJSON_Number 3
#define cJSON_String 4
#define cJSON_Array 5
#define cJSON_Object 6
	
#define cJSON_HashTableSize 32		// An object's hashtable has cJSON_HashTableSize slots
#define cJSON_ArrayBlockSize 16		// Arrays are alloced with cJSON_ArrayBlockSize slots at a time

// cJSON memory pool (only used by the pooled parser)
typedef struct cJSONMemPool_s {
	char *stringbuf;		// String read buffer (length is made to fit the longest string in the json structure (+ NULL terminator)
	unsigned int strbuflen;	// Length of the string buffer
	char *pool;				// Memory pool
	unsigned int poolsize;	// Size of the pool (in bytes)
	unsigned int used;		// Usage of the pool (in bytes)
} cJSONMemPool_t;

// The cJSON structure:
typedef struct cJSON_s {
	/* Item walking */
	struct cJSON_s *next,*prev;	// next/prev allow you to walk array/object chains. Alternatively, use GetArraySize/GetArrayItem/GetObjectItem
	struct cJSON_s *child;		// An array or object item will have a child pointer pointing to a chain of the items in the array/object.

	/* Array/Object lookup */
	struct cJSON_s **table;		// Dynamic array or hashtable for quick item lookups
	size_t tablesize;			// Size of the dynamic array or hashtable, depending on type
	size_t arraysize;			// Size of an array/object, if applicable

	/* Item type */
	int type;					// The type of the item, as above.

	/* Memory Pool */
	cJSONMemPool_t *pool;		// Non-NULL if a memory pool is associated with this node (blocks edit operations)

	/* Item data */
	char *valuestring;			// The item's string, if type == cJSON_String
	int valueint;				// The item's number, if type == cJSON_Number
	double valuedouble;			// The item's number, if type == cJSON_Number

	/* Subitem data */
	char *string;				// The item's name string, if this item is the child of, or is in the list of subitems of an object.
	int linked;					// 1 if this node is linked somewhere. If set, attempts to add it to another object or array will fail.
	struct cJSON_s *hashnext;	// Next entry in the hashtable (if applicable)
	struct cJSON_s *lastentry;		// Latest entry in the object, so we know which item to link to
} cJSON;

#define SBBLOCKSIZE 512	// Alloc SBBLOCKSIZE bytes at a time for the stringbuilder (by default)

typedef struct sb_s {	// StringBuilder structure for the serializer
	char	*buffer;
	size_t	bufferlen;
	size_t	blocksize;
	size_t	stringlen;
	char	finalized;
	char	staticData;		// non-zero if the buffer is static (and thus cannot be enlarged)
} cJSON_StringBuilder;

typedef struct cJSONWriterStack_s
{
	int type;		// 1 = Object, 2 = Array, anything else is invalid
	char empty;		// 1 if the array/object does not have any items (yet)
	char objectarray;	// 1 if the array contains nested arrays/objects (this causes items to be written on seperate lines if formatting is enabled)
	int depth;		// Depth of the structure (only used if formatting is enabled)
	struct cJSONWriterStack_s *next;	// Internal linkage
} cJSON_StreamStack;

// The cJSON stream writer structure:
typedef struct cJSONStream_s {
	cJSON_StreamStack *slots;		// Array of slots
	cJSON_StreamStack *free;		// Stack of free slots
	cJSON_StreamStack *stack;		// Stack of used slots
	cJSON_StringBuilder sb;			// String builder
	int	fmt;						// Formatted?
} cJSONStream;

#include "cJSON.h"


static int cJSON_strcasecmp(const char *s1, const char *s2)
{
	if (!s1) return (s1 == s2) ? 0 : 1;
	if (!s2) return 1;
	for(; tolower(*s1) == tolower(*s2); ++s1, ++s2)
	{
		if(*s1 == 0)
		{
			return 0;
		}
	}
	return tolower(*(const unsigned char *)s1) - tolower(*(const unsigned char *)s2);
}

static void *(*cJSON_malloc)(size_t sz) = malloc;
static void *(*cJSON_realloc)(void *ptr, size_t sz) = realloc;
static void (*cJSON_free)(void *ptr) = free;

static long cJSON_GenerateHashValue( const char *name, const int size ) {
	int		i;
	long	hash;
	char	letter;

	hash = 0;
	i = 0;
	while (name[i] != '\0') {
		letter = (char)tolower(name[i]);
		hash+=(long)(letter)*(i+119);
		i++;
	}
	hash = (hash ^ (hash >> 10) ^ (hash >> 20));
	hash &= (size-1);
	return hash;
}

static char* cJSON_strdup(const char* str)
{
      size_t len;
      char* copy;

	  if (!str) {
		  return 0;
	  }

      len = strlen(str) + 1;
	  copy = (char*)cJSON_malloc(len);
	  if ( !copy ) {
		  return 0;
	  }
      memcpy(copy,str,len);
      return copy;
}

void cJSON_InitHooks(cJSON_Hooks* hooks)
{
	if (!hooks)	{/* Reset hooks */

        cJSON_malloc = malloc;
		cJSON_realloc = realloc;
        cJSON_free = free;
        return;
    }

	cJSON_malloc = (hooks->malloc_fn) ? hooks->malloc_fn : malloc;
	cJSON_realloc = (hooks->realloc_fn) ? hooks->realloc_fn : realloc;
	cJSON_free	 = (hooks->free_fn) ? hooks->free_fn : free;
}

// String builder

static int cJSON_SB_Init(cJSON_StringBuilder *sb)	// Initializes the stringbuilder structure (Do not call this more than once, as it causes memory leaks)
{
	sb->buffer = (char *)cJSON_malloc(SBBLOCKSIZE);
	sb->bufferlen = SBBLOCKSIZE;
	sb->blocksize = SBBLOCKSIZE;
	sb->stringlen = 0;
	sb->finalized = 0;
	sb->staticData = 0;

	return !!sb->buffer;
}

static int cJSON_SB_InitCustom(cJSON_StringBuilder *sb, size_t initlen, size_t blocksize)	// Initializes the stringbuilder structure (Do not call this more than once, as it causes memory leaks)
{
	sb->buffer = (char *)cJSON_malloc(initlen);
	sb->bufferlen = initlen;
	sb->blocksize = blocksize;
	sb->stringlen = 0;
	sb->finalized = 0;
	sb->staticData = 0;

	return !!sb->buffer;
}

static int cJSON_SB_InitStatic(cJSON_StringBuilder *sb, char *buffer, unsigned int length)	// Initializes the stringbuilder structure using a static buffer
{
	sb->buffer = buffer;
	sb->bufferlen = length;
	sb->blocksize = 0;
	sb->stringlen = 0;
	sb->finalized = 0;
	sb->staticData = 1;

	return !!sb->buffer;
}

static int cJSON_SB_CheckSpace(cJSON_StringBuilder *sb, size_t space)	// Ensures at least 'space' bytes can be appended to the buffer (and enlarges it if needed)
{
	int spaceToAlloc = 0;
	void *newptr;

	while (sb->bufferlen + spaceToAlloc < sb->stringlen + space) {
		spaceToAlloc += sb->blocksize;
	}
	if (!spaceToAlloc) {
		return 1;	// Enough space
	}
	if (sb->staticData) {	// Buffer is static, so if we run out, we run out
		return 0;
	}
	// We need to alloc space, so do it now
	newptr = cJSON_realloc(sb->buffer, sb->bufferlen + spaceToAlloc);
	if (!newptr) {
		return 0;	// Failed to alloc
	} else {
		sb->buffer = (char *)newptr;
		sb->bufferlen += spaceToAlloc;
		return 1;
	}
}

static int cJSON_SB_AddChar(cJSON_StringBuilder *sb, char ch)
{
	if (!cJSON_SB_CheckSpace(sb, 1)) {
		return 0;
	}
	sb->buffer[sb->stringlen++] = ch;
	return 1;
}

static int cJSON_SB_AddCharRep(cJSON_StringBuilder *sb, char ch, int reps)
{
	if (reps < 1) {
		return 1;
	}
	if (!cJSON_SB_CheckSpace(sb, reps)) {
		return 0;
	}
	while (reps) {
		sb->buffer[sb->stringlen++] = ch;
		reps--;
	}
	return 1;
}

static int cJSON_SB_AddStringN(cJSON_StringBuilder *sb, const char *str, size_t len)
{
	if (!cJSON_SB_CheckSpace(sb, len)) {
		return 0;
	}
	strncpy(&sb->buffer[sb->stringlen], str, len);
	sb->stringlen += len;
	return 1;
}

static int cJSON_SB_AddString(cJSON_StringBuilder *sb, const char *str)
{
	int len = strlen(str);
	return cJSON_SB_AddStringN(sb, str, len);
}

static const char *cJSON_SB_Finalize(cJSON_StringBuilder *sb)
{
	void *newptr;
	if (!cJSON_SB_CheckSpace(sb, 1)) {
		return 0;
	}
	sb->buffer[sb->stringlen++] = 0;	// NULL terminator
	if (sb->staticData) {
		return sb->buffer;
	}

	newptr = realloc(sb->buffer, sb->stringlen);
	if (!newptr) {
		return 0;
	} else {
		sb->finalized = 1;
		sb->bufferlen = sb->stringlen;
		sb->buffer = (char *)newptr;
		return (const char *)newptr;
	}
}

/* End of stringbuilder */

// Serialize the cstring provided to an escaped version that can printed/stored.
static void cJSON_Serialize_String_Ptr(const char *str, cJSON_StringBuilder *sb)
{
	const char *ptr;
	char ubuff[5];
	
	if (!str)
	{
		cJSON_SB_AddStringN(sb, "\"\"", 2);	// Add "" and bail
		return;
	}

	ptr = str;
	

	ptr = str;
	cJSON_SB_AddChar(sb, '\"');

	while (*ptr)
	{
		if ( ((unsigned char)*ptr > 31 && (unsigned char)*ptr < 128) && *ptr != '\"' && *ptr != '\\' && *ptr != '/') {
			cJSON_SB_AddChar(sb, *ptr);
		} else {
			cJSON_SB_AddChar(sb, '\\');
			switch (*ptr)
			{
				case '\\':
					cJSON_SB_AddChar(sb, '\\');
					break;
				case '/':
					cJSON_SB_AddChar(sb, '/');
					break;
				case '\"':
					cJSON_SB_AddChar(sb, '\"');
					break;
				case '\b':
					cJSON_SB_AddChar(sb, 'b');
					break;
				case '\f':
					cJSON_SB_AddChar(sb, 'f');
					break;
				case '\n':
					cJSON_SB_AddChar(sb, 'n');
					break;
				case '\r':
					cJSON_SB_AddChar(sb, 'r');
					break;
				case '\t':
					cJSON_SB_AddChar(sb, 't');
					break;
				default:
					cJSON_SB_AddChar(sb, 'u');
					snprintf(ubuff, 5, "%04x", (unsigned char)*ptr);
					cJSON_SB_AddStringN(sb, ubuff, 4);
					break;
			}
		}
		ptr++;
	}

	cJSON_SB_AddChar(sb, '\"');
}

/* Start of Stream Writer */

cJSONStream *cJSON_Stream_New(int maxDepth, int formatted, size_t bufferInitSize, size_t bufferBlockSize)
{
	int i;
	cJSONStream *stream;

	if (maxDepth < 0) {
		return NULL;
	}

	stream = (cJSONStream *)malloc(sizeof(cJSONStream));
	if (!stream) {
		return NULL;
	}
	memset(stream, 0, sizeof(cJSONStream));

	if (maxDepth > 0) {
		stream->slots = (cJSON_StreamStack *)malloc(sizeof(cJSON_StreamStack) * maxDepth);
		if (!stream->slots) {
			free(stream);
			return NULL;
		}
		memset(stream->slots, 0, sizeof(cJSON_StreamStack) * maxDepth);

		// Link the slots to the free stack
		for (i = maxDepth - 1; i >= 0; i--) {
			stream->slots[i].next = stream->free;
			stream->free = &stream->slots[i];
		}
	}
	stream->fmt = formatted;

	cJSON_SB_InitCustom(&stream->sb, bufferInitSize ? bufferInitSize : 8192, bufferBlockSize ? bufferBlockSize : 8192);
	return stream;
}

const char *cJSON_Stream_Finalize(cJSONStream *stream)
{
	const char *result;
	result = cJSON_SB_Finalize(&stream->sb);
	free((void *)stream->slots);
	free((void *)stream);
	return result;
}

void cJSON_Stream_WriteNull(cJSONStream *stream, const char *key)
{
	cJSON_StreamStack *stack;
	stack = stream->stack;

	if (!stack) {
		cJSON_SB_AddStringN(&stream->sb, "null", 4);
	} else if (stack->type == 1) {
		if (!stack->empty) {
			cJSON_SB_AddChar(&stream->sb, ',');
		}
		if (stream->fmt) {
			if (stack->objectarray) {
				cJSON_SB_AddChar(&stream->sb, '\n');
				cJSON_SB_AddCharRep(&stream->sb, '\t', stack->depth);
			} else {
				cJSON_SB_AddChar(&stream->sb, ' ');
			}
		}
		cJSON_SB_AddStringN(&stream->sb, "null", 4);
		stack->empty = 0;
	} else if (stack->type == 2) {
		if (!stack->empty) {
			cJSON_SB_AddChar(&stream->sb, ',');
		}
		if (stream->fmt) {
			cJSON_SB_AddChar(&stream->sb, '\n');
			cJSON_SB_AddCharRep(&stream->sb, '\t', stack->depth);
		}
		cJSON_Serialize_String_Ptr(key ? key : "null", &stream->sb);
		cJSON_SB_AddChar(&stream->sb, ':');
		if (stream->fmt) cJSON_SB_AddChar(&stream->sb, '\t');
		cJSON_SB_AddStringN(&stream->sb, "null", 4);
		stack->empty = 0;
	}
}

void cJSON_Stream_WriteTrue(cJSONStream *stream, const char *key)
{
	cJSON_StreamStack *stack;
	stack = stream->stack;

	if (!stack) {
		cJSON_SB_AddStringN(&stream->sb, "true", 4);
	} else if (stack->type == 1) {
		if (!stack->empty) {
			cJSON_SB_AddChar(&stream->sb, ',');
		}
		if (stream->fmt) {
			if (stack->objectarray) {
				cJSON_SB_AddChar(&stream->sb, '\n');
				cJSON_SB_AddCharRep(&stream->sb, '\t', stack->depth);
			} else {
				cJSON_SB_AddChar(&stream->sb, ' ');
			}
		}
		cJSON_SB_AddStringN(&stream->sb, "true", 4);
		stack->empty = 0;
	} else if (stack->type == 2) {
		if (!stack->empty) {
			cJSON_SB_AddChar(&stream->sb, ',');
		}
		if (stream->fmt) {
			cJSON_SB_AddChar(&stream->sb, '\n');
			cJSON_SB_AddCharRep(&stream->sb, '\t', stack->depth);
		}
		cJSON_Serialize_String_Ptr(key ? key : "null", &stream->sb);
		cJSON_SB_AddChar(&stream->sb, ':');
		if (stream->fmt) cJSON_SB_AddChar(&stream->sb, '\t');
		cJSON_SB_AddStringN(&stream->sb, "true", 4);
		stack->empty = 0;
	}
}

void cJSON_Stream_WriteFalse(cJSONStream *stream, const char *key)
{
	cJSON_StreamStack *stack;
	stack = stream->stack;

	if (!stack) {
		cJSON_SB_AddStringN(&stream->sb, "false", 5);
	} else if (stack->type == 1) {
		if (!stack->empty) {
			cJSON_SB_AddChar(&stream->sb, ',');
		}
		if (stream->fmt) {
			if (stack->objectarray) {
				cJSON_SB_AddChar(&stream->sb, '\n');
				cJSON_SB_AddCharRep(&stream->sb, '\t', stack->depth);
			} else {
				cJSON_SB_AddChar(&stream->sb, ' ');
			}
		}
		cJSON_SB_AddStringN(&stream->sb, "false", 5);
		stack->empty = 0;
	} else if (stack->type == 2) {
		if (!stack->empty) {
			cJSON_SB_AddChar(&stream->sb, ',');
		}
		if (stream->fmt) {
			cJSON_SB_AddChar(&stream->sb, '\n');
			cJSON_SB_AddCharRep(&stream->sb, '\t', stack->depth);
		}
		cJSON_Serialize_String_Ptr(key ? key : "null", &stream->sb);
		cJSON_SB_AddChar(&stream->sb, ':');
		if (stream->fmt) cJSON_SB_AddChar(&stream->sb, '\t');
		cJSON_SB_AddStringN(&stream->sb, "false", 5);
		stack->empty = 0;
	}
}

void cJSON_Stream_WriteBoolean(cJSONStream *stream, const char *key, int boolean)
{
	cJSON_StreamStack *stack;
	stack = stream->stack;

	if (!stack) {
		cJSON_SB_AddStringN(&stream->sb, boolean ? "true" : "false",  boolean ? 4 : 5);
	} else if (stack->type == 1) {
		if (!stack->empty) {
			cJSON_SB_AddChar(&stream->sb, ',');
		}
		if (stream->fmt) {
			if (stack->objectarray) {
				cJSON_SB_AddChar(&stream->sb, '\n');
				cJSON_SB_AddCharRep(&stream->sb, '\t', stack->depth);
			} else {
				cJSON_SB_AddChar(&stream->sb, ' ');
			}
		}
		cJSON_SB_AddStringN(&stream->sb, boolean ? "true" : "false",  boolean ? 4 : 5);
		stack->empty = 0;
	} else if (stack->type == 2) {
		if (!stack->empty) {
			cJSON_SB_AddChar(&stream->sb, ',');
		}
		if (stream->fmt) {
			cJSON_SB_AddChar(&stream->sb, '\n');
			cJSON_SB_AddCharRep(&stream->sb, '\t', stack->depth);
		}
		cJSON_Serialize_String_Ptr(key ? key : "null", &stream->sb);
		cJSON_SB_AddChar(&stream->sb, ':');
		if (stream->fmt) cJSON_SB_AddChar(&stream->sb, '\t');
		cJSON_SB_AddStringN(&stream->sb, boolean ? "true" : "false",  boolean ? 4 : 5);
		stack->empty = 0;
	}
}

void cJSON_Stream_WriteString(cJSONStream *stream, const char *key, const char *string)
{
	cJSON_StreamStack *stack;
	stack = stream->stack;

	if (!stack) {
		cJSON_Serialize_String_Ptr(string, &stream->sb);
	} else if (stack->type == 1) {
		if (!stack->empty) {
			cJSON_SB_AddChar(&stream->sb, ',');
		}
		if (stream->fmt) {
			if (stack->objectarray) {
				cJSON_SB_AddChar(&stream->sb, '\n');
				cJSON_SB_AddCharRep(&stream->sb, '\t', stack->depth);
			} else {
				cJSON_SB_AddChar(&stream->sb, ' ');
			}
		}
		cJSON_Serialize_String_Ptr(string, &stream->sb);
		stack->empty = 0;
	} else if (stack->type == 2) {
		if (!stack->empty) {
			cJSON_SB_AddChar(&stream->sb, ',');
		}
		if (stream->fmt) {
			cJSON_SB_AddChar(&stream->sb, '\n');
			cJSON_SB_AddCharRep(&stream->sb, '\t', stack->depth);
		}
		cJSON_Serialize_String_Ptr(key ? key : "null", &stream->sb);
		cJSON_SB_AddChar(&stream->sb, ':');
		if (stream->fmt) cJSON_SB_AddChar(&stream->sb, '\t');
		cJSON_Serialize_String_Ptr(string, &stream->sb);
		stack->empty = 0;
	}
}

void cJSON_Stream_WriteInteger(cJSONStream *stream, const char *key, int value)
{

	char str[64];
	cJSON_StreamStack *stack;
	stack = stream->stack;
	
	snprintf(str, 21, "%d" , value);

	if (!stack) {
		cJSON_SB_AddString(&stream->sb, str);
	} else if (stack->type == 1) {
		if (!stack->empty) {
			cJSON_SB_AddChar(&stream->sb, ',');
		}
		if (stream->fmt) {
			if (stack->objectarray) {
				cJSON_SB_AddChar(&stream->sb, '\n');
				cJSON_SB_AddCharRep(&stream->sb, '\t', stack->depth);
			} else {
				cJSON_SB_AddChar(&stream->sb, ' ');
			}
		}
		cJSON_SB_AddString(&stream->sb, str);
		stack->empty = 0;
	} else if (stack->type == 2) {
		if (!stack->empty) {
			cJSON_SB_AddChar(&stream->sb, ',');
		}
		if (stream->fmt) {
			cJSON_SB_AddChar(&stream->sb, '\n');
			cJSON_SB_AddCharRep(&stream->sb, '\t', stack->depth);
		}
		cJSON_Serialize_String_Ptr(key ? key : "null", &stream->sb);
		cJSON_SB_AddChar(&stream->sb, ':');
		if (stream->fmt) cJSON_SB_AddChar(&stream->sb, '\t');
		cJSON_SB_AddString(&stream->sb, str);
		stack->empty = 0;
	}
}

void cJSON_Stream_WriteNumber(cJSONStream *stream, const char *key, double value)
{

	char str[64];
	cJSON_StreamStack *stack;
	stack = stream->stack;

	if ( fabs( floor(value) - value ) <= DBL_EPSILON) {
		snprintf(str, 64, "%.0f", value);
	} else if ( fabs(value) < 1.0e-6 || fabs(value) > 1.0e9) {
		snprintf(str, 64, "%e", value);
	} else {
		snprintf(str, 64, "%f", value);
	}

	if (!stack) {
		cJSON_SB_AddString(&stream->sb, str);
	} else if (stack->type == 1) {
		if (!stack->empty) {
			cJSON_SB_AddChar(&stream->sb, ',');
		}
		if (stream->fmt) {
			if (stack->objectarray) {
				cJSON_SB_AddChar(&stream->sb, '\n');
				cJSON_SB_AddCharRep(&stream->sb, '\t', stack->depth);
			} else {
				cJSON_SB_AddChar(&stream->sb, ' ');
			}
		}
		cJSON_SB_AddString(&stream->sb, str);
		stack->empty = 0;
	} else if (stack->type == 2) {
		if (!stack->empty) {
			cJSON_SB_AddChar(&stream->sb, ',');
		}
		if (stream->fmt) {
			cJSON_SB_AddChar(&stream->sb, '\n');
			cJSON_SB_AddCharRep(&stream->sb, '\t', stack->depth);
		}
		cJSON_Serialize_String_Ptr(key ? key : "null", &stream->sb);
		cJSON_SB_AddChar(&stream->sb, ':');
		if (stream->fmt) cJSON_SB_AddChar(&stream->sb, '\t');
		cJSON_SB_AddString(&stream->sb, str);
		stack->empty = 0;
	}
}

void cJSON_Stream_BeginArray(cJSONStream *stream, const char *key)
{
	cJSON_StreamStack *stack, *next;
	stack = stream->stack;
	next = stream->free;

	if (!next) {
		return;		// Max depth reached
	}

	if (!stack) {
		cJSON_SB_AddChar(&stream->sb, '[');
	} else if (stack->type == 1) {
		stack->objectarray = 1;
		if (!stack->empty) {
			cJSON_SB_AddChar(&stream->sb, ',');
		}
		if (stream->fmt) {
			if (stack->objectarray) {
				cJSON_SB_AddChar(&stream->sb, '\n');
				cJSON_SB_AddCharRep(&stream->sb, '\t', stack->depth);
			} else {
				cJSON_SB_AddChar(&stream->sb, ' ');
			}
		}
		cJSON_SB_AddChar(&stream->sb, '[');
		stack->empty = 0;
	} else if (stack->type == 2) {
		if (!stack->empty) {
			cJSON_SB_AddChar(&stream->sb, ',');
		}
		if (stream->fmt) {
			cJSON_SB_AddChar(&stream->sb, '\n');
			cJSON_SB_AddCharRep(&stream->sb, '\t', stack->depth);
		}
		cJSON_Serialize_String_Ptr(key ? key : "null", &stream->sb);
		cJSON_SB_AddChar(&stream->sb, ':');
		if (stream->fmt) cJSON_SB_AddChar(&stream->sb, '\t');
		cJSON_SB_AddChar(&stream->sb, '[');
		stack->empty = 0;
	}

	stream->free = stream->free->next;

	next->type = 1;
	next->empty = 1;
	next->next = stream->stack;
	stream->stack = next;
	next->depth = next->next ? next->next->depth + 1 : 1;
	next->objectarray = 0;
}

void cJSON_Stream_BeginObject(cJSONStream *stream, const char *key)
{
	cJSON_StreamStack *stack, *next;
	stack = stream->stack;
	next = stream->free;

	if (!next) {
		return;		// Max depth reached
	}

	if (!stack) {
		cJSON_SB_AddChar(&stream->sb, '{');
	} else if (stack->type == 1) {
		stack->objectarray = 1;

		if (!stack->empty) {
			cJSON_SB_AddChar(&stream->sb, ',');
		}
		if (stream->fmt) {
			if (stack->objectarray) {
				cJSON_SB_AddChar(&stream->sb, '\n');
				cJSON_SB_AddCharRep(&stream->sb, '\t', stack->depth);
			} else {
				cJSON_SB_AddChar(&stream->sb, ' ');
			}
		}
		cJSON_SB_AddChar(&stream->sb, '{');
		stack->empty = 0;
	} else if (stack->type == 2) {
		if (!stack->empty) {
			cJSON_SB_AddChar(&stream->sb, ',');
		}
		if (stream->fmt) {
			cJSON_SB_AddChar(&stream->sb, '\n');
			cJSON_SB_AddCharRep(&stream->sb, '\t', stack->depth);
		}
		cJSON_Serialize_String_Ptr(key ? key : "null", &stream->sb);
		cJSON_SB_AddChar(&stream->sb, ':');
		if (stream->fmt) cJSON_SB_AddChar(&stream->sb, '\t');
		cJSON_SB_AddChar(&stream->sb, '{');
		stack->empty = 0;
	}

	stream->free = stream->free->next;

	next->type = 2;
	next->empty = 1;
	next->next = stream->stack;
	stream->stack = next;
	next->depth = next->next ? next->next->depth + 1 : 1;
}

void cJSON_Stream_EndBlock(cJSONStream *stream)
{
	cJSON_StreamStack *stack;
	stack = stream->stack;
	


	if (!stack) {
		return;	 // We're at top level already
	} else if (stack->type == 1) {
		if (stream->fmt) {
			if (stack->objectarray) {
				cJSON_SB_AddChar(&stream->sb, '\n');
				cJSON_SB_AddCharRep(&stream->sb, '\t', stack->depth - 1);
			} else {
				cJSON_SB_AddChar(&stream->sb, ' ');
			}
		}
		cJSON_SB_AddChar(&stream->sb, ']');
	} else if (stack->type == 2) {
		if (stream->fmt) {
			cJSON_SB_AddChar(&stream->sb, '\n');
			cJSON_SB_AddCharRep(&stream->sb, '\t', stack->depth - 1);
		}
		cJSON_SB_AddChar(&stream->sb, '}');
	}

	stream->stack = stack->next;

	stack->next = stream->free;
	stream->free = stack;
}

/* End of Stream Writer */

/* Start of stringstreamer */
typedef struct ss_s {	// StringStreamer structure for the parser
	const char	*base;
	const char	*pos;
	size_t		line;
	size_t		col;
	int			error;
	char		errorMsg[256];
} cJSON_StringStream;

static void cJSON_SS_Init(cJSON_StringStream *ss, const char* data)
{
	memset(ss, 0, sizeof(cJSON_StringStream));
	ss->base = data;
	ss->pos = data;
	ss->line = 1;
	ss->col = 1;
}

static int cJSON_vsnprintf( char *dest, int size, const char *fmt, va_list argptr ) {
	int ret;

#ifdef _WIN32
	ret = _vsnprintf( dest, size-1, fmt, argptr );
#else
	ret = vsnprintf( dest, size, fmt, argptr );
#endif

	dest[size-1] = '\0';
	if ( ret < 0 || ret >= size ) {
		return -1;
	}
	return ret;
}

static void cJSON_sprintf( char *dest, int size, const char *fmt, ...) {
	int		ret;
	va_list		argptr;

	va_start (argptr,fmt);
	ret = cJSON_vsnprintf (dest, size, fmt, argptr);
	va_end (argptr);
}

static void cJSON_SS_ParseError(cJSON_StringStream *ss, const char *fmt, ...) 
{
	va_list		argptr;
	static char buff[256];

	va_start(argptr, fmt);
	cJSON_vsnprintf(buff, sizeof(buff), fmt, argptr);
	cJSON_sprintf(ss->errorMsg, sizeof(ss->errorMsg), "line %i, col %i - %s", ss->line, ss->col, buff);
	ss->error = 1;
}

/* Read over all whitespace and stop at the first non-whitespace character */
static void cJSON_SS_SkipWhitespace(cJSON_StringStream *ss)
{
	char ch;

	int inComment = 0;	// 0 = Not in comment, 1 = in // comment, 2 = in block comment
	for (;;) {
		ch = *ss->pos;
		switch(ch) {
			case 0:	// End of data
				return;
			case ' ':
				ss->col++;
				break;
			case '\t':
				ss->col += 4;	// Tabs are treated as 4 spaces
				break;
			case '\n':
				if (inComment == 1) inComment = 0;
				ss->col = 1;
				ss->line++;
				break;
			case '\r':
				if (inComment == 1) inComment = 0;
				ss->col = 1;
				break;
			case '/':
				if (*(ss->pos+1) == '*') {
					inComment = 2;
					ss->pos++;
					ss->col += 2;
				} else if (*(ss->pos+1) == '/') {
					inComment = 1;
					ss->pos++;
					ss->col += 2;
				}
				break;
			case '*':
				if (*(ss->pos+1) == '/') {
					inComment = 0;
					ss->pos++;
				}
				break;
			default:
				// Non-whitespace characters
				if (!inComment)
					return;
				ss->col++;
		}
		ss->pos++;
	}
}


/* Read over the current token and all following whitespace and stop at the first non-whitespace character */
/* WARNING: Will not take formatting into account! Do not use on strings, as they'll be treated as regular tokens */

static void cJSON_SS_SkipToken(cJSON_StringStream *ss)
{
	char ch;
	int inToken = 1;
	int inComment = 0;	// 0 = Not in comment, 1 = in // comment, 2 = in block comment

	while (inToken) {
		ch = *ss->pos;
		switch(ch) {
			case 0:
				return;
			case ' ':
			case '\t':
			case '\n':
			case '\r':
				inToken = 0;
				break;		// Whitespace, bail out
			case '/':
				if (*(ss->pos+1) == '*' || *(ss->pos+1) == '/') {		// Comment, bail out
					inToken = 0;
				} else {
					ss->pos++;
					ss->col++;
				}
				break;
			case '{':
			case '}':
			case '[':
			case ']':
			case ',':
			case '\"':
				inToken = 0;
				break;		// Control character, bail out
			default:
				ss->pos++;
				ss->col++;
				break;		// Anything else: advance
		}
	}

	for (;;) {
		ch = *ss->pos;
		switch(ch) {
			case 0:	// End of data
				return;
			case ' ':
				ss->col++;
				break;
			case '\t':
				ss->col += 4;	// Tabs are treated as 4 spaces
				break;
			case '\n':
				if (inComment == 1) inComment = 0;
				ss->col = 1;
				ss->line++;
				break;
			case '\r':
				if (inComment == 1) inComment = 0;
				ss->col = 1;
				break;
			case '/':
				if (*(ss->pos+1) == '*') {
					inComment = 2;
					ss->pos++;
					ss->col += 2;
				} else if (*(ss->pos+1) == '/') {
					inComment = 1;
					ss->pos++;
					ss->col += 2;
				}
				break;
			case '*':
				if (*(ss->pos+1) == '/') {
					inComment = 0;
					ss->pos++;
				}
				break;
			default:
				// Non-whitespace characters
				if (!inComment)
					return;
				ss->col++;
		}
		ss->pos++;
	}
}

/* Get the next character without advancing */
/* NOTE: Does not support whitespace skipping */
static __inline char cJSON_SS_PeekChar(cJSON_StringStream *ss)
{
	return *ss->pos;
}

/* Get the next character and advance */
static char cJSON_SS_GetChar(cJSON_StringStream *ss)
{
	switch(*ss->pos) {
			case '\n':
				ss->line += 1;
				ss->col = 1;
				break;
			case '\r':
				ss->col = 1;
				break;
			case '\t':
				ss->col += 4;
				break;
			default:
				ss->col++;
				break;
		}
	return *ss->pos++;
}

/* Advance a certain amount of characters */
static void cJSON_SS_Advance(cJSON_StringStream *ss, int amount)
{
	while (amount) {
		switch(*ss->pos) {
			case '\n':
				ss->line += 1;
				ss->col = 1;
				break;
			case '\r':
				ss->col = 1;
				break;
			case '\t':
				ss->col += 4;
				break;
			default:
				ss->col++;
				break;
		}
		ss->pos++;
		amount--;
	}
}

/* Skip over the next character */
static __inline void cJSON_SS_SkipChar(cJSON_StringStream *ss)
{
	cJSON_SS_Advance(ss, 1);
}


/* Do a strncmp on the current position */
static __inline int cJSON_SS_Compare(cJSON_StringStream *ss, const char *str, size_t count)
{
		return strncmp(ss->pos, str, count);
}

/* End of stringstreamer */


// Internal constructor.
static cJSON *cJSON_New_Item()
{
	cJSON* node = (cJSON*)cJSON_malloc( sizeof(cJSON) );
	if (node) {
		memset(node, 0, sizeof(cJSON));
	}
	return node;
}

static void * cJSON_PooledMalloc(cJSONMemPool_t *pool, unsigned int size)
{
	void *ptr;
	if (pool->used + size > pool->poolsize) {
		assert(0);
		return NULL;
	}

	ptr = pool->pool + pool->used;
	pool->used += size;
	return ptr;
}

static cJSON *cJSON_New_ItemPooled(cJSONMemPool_t *pool)
{
	cJSON* node = (cJSON*)cJSON_PooledMalloc(pool, sizeof(cJSON) );
	if (node) {
		memset(node, 0, sizeof(cJSON));
		node->pool = pool;
	}
	return node;
}

// Delete a cJSON structure.
void cJSON_Delete(cJSON *c)
{
	cJSON *next;
	cJSONMemPool_t *pool;

	if (!c) {
		return;
	}

	if (c->linked) {
		return;		// Cannot delete linked items
	}

	if (c->pool) {
		// This item is pooled, so free the pool
		pool = c->pool;
		if (pool->stringbuf) free(pool->stringbuf);
		if (pool->pool) free(pool->pool);
		free(pool);
		return;
	}

	while (c)
	{
		next = c->next;

		if (c->child) {
			c->child->linked = 0;	// Mark it as unlinked so we can delete it
			cJSON_Delete(c->child);
		}
		if (c->valuestring) {
			cJSON_free(c->valuestring);
		}
		if (c->string) {
			cJSON_free(c->string);
		}
		if (c->table) {
			cJSON_free(c->table);
		}

		cJSON_free(c);
		c = next;
	}
}

// Delete a cJSON item and clear the pointer to it (if it can be deleted that is)
void cJSON_SafeDelete(cJSON **c)
{
	if (c && *c && !(*c)->linked) {
		cJSON_Delete(*c);
		*c = 0;
	}
}

// Parse the input text to generate a number, and populate the result into item.
static int cJSON_Parse_Number(cJSON *item, cJSON_StringStream *ss)
{
	double n=0, sign=1, scale=0;
	int subscale=0, signsubscale=1;

	// Could use sscanf for this?
	if (cJSON_SS_PeekChar(ss) == '-') { // Has sign?
		sign = -1;
		cJSON_SS_SkipChar(ss);
	}
	if (cJSON_SS_PeekChar(ss) == '0') { // is zero
		cJSON_SS_SkipChar(ss);
	}
	if (cJSON_SS_PeekChar(ss) >= '1' && cJSON_SS_PeekChar(ss) <= '9') { // Number?
		do {
			n = ( n * 10.0 ) + ( cJSON_SS_GetChar(ss) - '0' );
		} while (cJSON_SS_PeekChar(ss) >= '0' && cJSON_SS_PeekChar(ss) <= '9');
	}
	if (cJSON_SS_PeekChar(ss) == '.')	// Fractional part?
	{
		cJSON_SS_SkipChar(ss);
		do {
			n = ( n * 10.0 ) + ( cJSON_SS_GetChar(ss) - '0' );
			scale--;
		} while (cJSON_SS_PeekChar(ss) >= '0' && cJSON_SS_PeekChar(ss) <= '9');
	}	
	if (cJSON_SS_PeekChar(ss) == 'e' || cJSON_SS_PeekChar(ss) == 'E')		// Exponent?
	{	
		cJSON_SS_SkipChar(ss);
		if (cJSON_SS_PeekChar(ss) == '+')				// With sign?
		{
			cJSON_SS_SkipChar(ss);
		}
		else if (cJSON_SS_PeekChar(ss) == '-')
		{
			signsubscale = -1;
			cJSON_SS_SkipChar(ss);
		}
		while (cJSON_SS_PeekChar(ss) >= '0' && cJSON_SS_PeekChar(ss) <= '9')
		{
			subscale = ( subscale * 10 ) + (cJSON_SS_GetChar(ss) - '0' );	// Number?
		}
	}

	n = sign * n * pow(10.0, (scale + subscale * signsubscale ));	// number = +/- number.fraction * 10^+/- exponent
	
	item->valuedouble = n;
	item->valueint = (int)n;
	item->type = cJSON_Number;
	return 1;
}

// Serialize the number nicely from the given item into a string.
static void cJSON_Serialize_Number(cJSON *item, cJSON_StringBuilder *sb)
{
	char str[64];
	double d = item->valuedouble;
	if ( fabs( ( (double)item->valueint ) - d ) <= DBL_EPSILON && d <= INT_MAX && d >= INT_MIN)
	{
		snprintf(str, 21, "%d" ,item->valueint);
	}
	else
	{
		if ( fabs( floor(d) - d ) <= DBL_EPSILON) {
			snprintf(str, 64, "%.0f", d);
		} else if ( fabs(d) < 1.0e-6 || fabs(d) > 1.0e9) {
			snprintf(str, 64, "%e", d);
		} else {
			snprintf(str, 64, "%f", d);
		}
	}

	cJSON_SB_AddString(sb, str);
}

// Parse the input text into an unescaped cstring, and populate item.
//static const unsigned char firstByteMark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };
static int cJSON_Parse_String(cJSON *item, cJSON_StringStream *ss)
{
	cJSON_StringBuilder sb;
	unsigned uc;

	if (cJSON_SS_PeekChar(ss) != '\"') {
		cJSON_SS_ParseError(ss, "Expected '\"', got '%c'", cJSON_SS_PeekChar(ss));
		return 0;	// not a string!
	}
	
	/* Initialize the string builder */

	if (!cJSON_SB_Init(&sb)) {
		cJSON_SS_ParseError(ss, "Insufficient memory");
		return 0;	// Not enough memory
	}

	cJSON_SS_SkipChar(ss);
	while (cJSON_SS_PeekChar(ss) != '\"')
	{
		if (cJSON_SS_PeekChar(ss) < 32) {
			cJSON_SS_ParseError(ss, "Illegal character in string (%02X)", cJSON_SS_PeekChar(ss), cJSON_SS_PeekChar(ss));
			return 0;	// Illegal character
		}
		if (cJSON_SS_PeekChar(ss) != '\\') {
			if (!cJSON_SB_AddChar(&sb, cJSON_SS_GetChar(ss))) {
				cJSON_SS_ParseError(ss, "Insufficient memory");
				return 0;
			}
		} else {
			cJSON_SS_SkipChar(ss);
			switch (cJSON_SS_PeekChar(ss))
			{
				case 'b': 
					cJSON_SB_AddChar(&sb, '\b');
					break;
				case 'f':
					cJSON_SB_AddChar(&sb, '\f');
					break;
				case 'n':
					cJSON_SB_AddChar(&sb, '\n');
					break;
				case 'r':
					cJSON_SB_AddChar(&sb, '\r');
					break;
				case 't':
					cJSON_SB_AddChar(&sb, '\t');
					break;
				case 'u':	 // transcode utf16 to utf8. DOES NOT SUPPORT SURROGATE PAIRS CORRECTLY.
					uc = 0;
					sscanf( ss->pos + 1, "%4x", &uc);	// get the unicode char.
					
					// Clamp to ANSI range, everything else is replaced by a ?

					if (uc < 0x100) {
						cJSON_SB_AddChar(&sb, (char)uc);
					} else {
						cJSON_SB_AddChar(&sb, '?');
					}
					
					/*
					if (uc < 0x80) {
						len=1;
					} else if (uc < 0x800) {
						len=2;
					} else {
						len=3;
					}
					
					// Little hack here
					if (!cJSON_SB_CheckSpace(&sb, len)) {
						cJSON_SS_ParseError(ss, "Insufficient memory");
						return 0;
					}
					
					switch (len) {
						case 3: sb.buffer[sb.stringlen + 2] = ((uc | 0x80) & 0xBF); uc >>= 6;
						case 2: sb.buffer[sb.stringlen + 1] = ((uc | 0x80) & 0xBF); uc >>= 6;
						case 1: sb.buffer[sb.stringlen + 0] = (char)(uc | firstByteMark[len]);
					}

					sb.stringlen += len;
					*/
					cJSON_SS_Advance(ss, 4);
					break;
				default:  
					if (!cJSON_SB_AddChar(&sb, cJSON_SS_PeekChar(ss))) {
						cJSON_SS_ParseError(ss, "Insufficient memory");
						return 0;
					}
					break;
			}
			cJSON_SS_SkipChar(ss);
		}
	}
	cJSON_SS_SkipChar(ss);

	item->valuestring = (char *)cJSON_SB_Finalize(&sb);
	if (!item->valuestring) {
		cJSON_SS_ParseError(ss, "Insufficient memory");
		return 0;
	}
	item->type = cJSON_String;
	return 1;
}

// Invoke print_string_ptr on an item to serialize it's string value.
__inline static void cJSON_Serialize_String(cJSON *item, cJSON_StringBuilder *sb)
{
	cJSON_Serialize_String_Ptr( item->valuestring, sb );
}

// Predeclare these prototypes.
static int cJSON_Parse_Value(cJSON *item, cJSON_StringStream *ss);
static int cJSON_Parse_Array(cJSON *item, cJSON_StringStream *ss);
static int cJSON_Parse_Object(cJSON *item, cJSON_StringStream *ss);

static void cJSON_Serialize_Value(cJSON *item,int depth,int fmt, cJSON_StringBuilder *sb);
static void cJSON_Serialize_Array(cJSON *item,int depth,int fmt, cJSON_StringBuilder *sb);
static void cJSON_Serialize_Object(cJSON *item,int depth,int fmt, cJSON_StringBuilder *sb);

// Parse an object - create a new root, and populate.
cJSON *cJSON_Parse(const char *data, char *error, size_t errorlen)
{
	cJSON_StringStream ss;
	cJSON *c;

	if (!data) {
		if (error) {
			strncpy(error, "data is NULL", errorlen);
		}
		return 0;       /* memory fail */
	}

	c = cJSON_New_Item();
	if (!c) {
		if (error) {
			strncpy(error, "Insufficient memory", errorlen);
		}
		return 0;       /* memory fail */
	}

	cJSON_SS_Init(&ss, data);

	if (!cJSON_Parse_Value(c, &ss)) {
		if (ss.error && error) {
			strncpy(error, ss.errorMsg, errorlen);
		}
		cJSON_Delete(c);
		return 0;
	}
	return c;
}

// Serialize a cJSON item/entity/structure to text.
const char *cJSON_Serialize(cJSON *item, int format)
{
	cJSON_StringBuilder sb;
	cJSON_SB_Init(&sb);
	cJSON_Serialize_Value(item, 0, format, &sb);
	return cJSON_SB_Finalize(&sb);
}


// Parser Routines
// A StringStream is created in advance which will be used to parse the whole structure
// If the parsing succeeds, return 1, if it fails, call cJSON_SS_ParseError and return 0

static int cJSON_Parse_Value(cJSON *item, cJSON_StringStream *ss)
{
	cJSON_SS_SkipWhitespace(ss);

	if (!cJSON_SS_Compare(ss, "null", 4))	{
		item->type = cJSON_NULL;
		cJSON_SS_Advance(ss, 4);
		return 1;
	}
	if (!cJSON_SS_Compare(ss, "false", 5)) {
		item->type = cJSON_False;
		cJSON_SS_Advance(ss, 5);
		return 1;
	}
	if (!cJSON_SS_Compare(ss, "true", 4))	{
		item->type = cJSON_True;
		item->valueint = 1;
		cJSON_SS_Advance(ss, 4);
		return 1;
	}
	if (cJSON_SS_PeekChar(ss) == '\"')	{
		return cJSON_Parse_String(item, ss);
	}
	if (cJSON_SS_PeekChar(ss) == '-' || (cJSON_SS_PeekChar(ss) >= '0' && cJSON_SS_PeekChar(ss) <= '9')) {
		return cJSON_Parse_Number(item, ss);
	}
	if (cJSON_SS_PeekChar(ss) == '[') {
		return cJSON_Parse_Array(item, ss);
	}
	if (cJSON_SS_PeekChar(ss) == '{') {
		return cJSON_Parse_Object(item, ss);
	}

	if (!cJSON_SS_PeekChar(ss)) {
		cJSON_SS_ParseError(ss, "Unexpected EOF");
	} else {
		cJSON_SS_ParseError(ss, "Unrecognised character: '%c'", cJSON_SS_PeekChar(ss));	
	}
	return 0;
}

// Serialize a value to text.
static void cJSON_Serialize_Value(cJSON *item, int depth, int fmt, cJSON_StringBuilder *sb)
{
	if (!item) {
		return;
	}
	switch ( item->type)
	{
		case cJSON_NULL:
		default:
			cJSON_SB_AddStringN(sb, "null", 4);
			break;
		case cJSON_False:
			cJSON_SB_AddStringN(sb, "false", 5);
			break;
		case cJSON_True:
			cJSON_SB_AddStringN(sb, "true", 4);
			break;
		case cJSON_Number:	
			cJSON_Serialize_Number(item, sb);
			break;
		case cJSON_String:	
			cJSON_Serialize_String(item, sb);
			break;
		case cJSON_Array:
			cJSON_Serialize_Array(item, depth, fmt, sb);
			break;
		case cJSON_Object:
			cJSON_Serialize_Object(item, depth, fmt, sb);
			break;
	}
}

// Build an array from input text.
static int cJSON_Parse_Array(cJSON *item, cJSON_StringStream *ss)
{
	cJSON *child;

	if (cJSON_SS_PeekChar(ss) != '[') {
		cJSON_SS_ParseError(ss, "Expected '[', got '%c", cJSON_SS_PeekChar(ss));
		return 0;	// not an array!
	}
	item->type = cJSON_Array;

	item->table = (cJSON_s **)cJSON_malloc(cJSON_ArrayBlockSize * sizeof(cJSON *));
	item->tablesize = cJSON_ArrayBlockSize;

	cJSON_SS_SkipChar(ss);
	cJSON_SS_SkipWhitespace(ss);

	if (cJSON_SS_PeekChar(ss) == ']') {
		cJSON_SS_SkipChar(ss);
		return 1;	// empty array.
	}
	child = cJSON_New_Item();

	if (!child) {
		cJSON_SS_ParseError(ss, "Insufficient memory");
		return 0;		 // memory fail
	}

	if (!cJSON_Parse_Value(child, ss)) {	// Get the value
		// Do not set an error here, the parse function that retuned 0 will have done that already.
		cJSON_Delete(child);
		return 0;
	}
	cJSON_SS_SkipWhitespace(ss);

	cJSON_AddItemToArray(item, child);

	while (cJSON_SS_PeekChar(ss) == ',')
	{
		child = cJSON_New_Item();
		if ( !child ) {
			cJSON_SS_ParseError(ss, "Failed to allocate item for array");
			return 0; 	// memory fail
		}

		cJSON_SS_SkipChar(ss);
		if (!cJSON_Parse_Value(child, ss)) {
			cJSON_Delete(child);
			return 0;	// parse error
		}
		cJSON_SS_SkipWhitespace(ss);

		cJSON_AddItemToArray(item, child);
	}

	if (cJSON_SS_PeekChar(ss) == ']') {
		cJSON_SS_SkipChar(ss);
		return 1;	// end of array
	}

	cJSON_SS_ParseError(ss, "Expected ']', got '%c'", cJSON_SS_PeekChar(ss));
	return 0;
}

// Serialize an array to text
static void cJSON_Serialize_Array(cJSON *item, int depth, int fmt, cJSON_StringBuilder *sb)
{
	cJSON *child = item->child;
	
	int objectArray = 0;

	cJSON_SB_AddChar(sb, '[');
	while (child)
	{
		// Special handling for objects
		if (child->type == cJSON_Object || child->type == cJSON_Array) {
			objectArray = 1;
		}
		if (fmt && objectArray) {
			cJSON_SB_AddChar(sb, '\n');
			cJSON_SB_AddCharRep(sb, '\t', depth+1);
		}
		
		cJSON_Serialize_Value(child, depth + 1, fmt, sb);
		if (child->next) {
			cJSON_SB_AddChar(sb, ',');
			if (fmt && !objectArray) {
				cJSON_SB_AddChar(sb, ' ');
			}
		}
		child = child->next;
	}

	if (fmt && objectArray) {
		cJSON_SB_AddChar(sb, '\n');
		cJSON_SB_AddCharRep(sb, '\t', depth);
	}

	cJSON_SB_AddChar(sb, ']');
}

// Build an object from the text.
static int cJSON_Parse_Object(cJSON *item, cJSON_StringStream *ss)
{
	cJSON *child;
	const char *name;
	if (cJSON_SS_PeekChar(ss) != '{') {
		cJSON_SS_ParseError(ss, "Expected '{', got '%c'", cJSON_SS_PeekChar(ss));
		return 0;	// not an object!
	}
	
	// Create object
	item->type = cJSON_Object;

	item->table = (cJSON_s **)cJSON_malloc(cJSON_HashTableSize * sizeof(cJSON *));
	memset(item->table, 0, cJSON_HashTableSize * sizeof(cJSON *));
	item->tablesize = cJSON_HashTableSize;

	cJSON_SS_SkipChar(ss);
	cJSON_SS_SkipWhitespace(ss);

	if (cJSON_SS_PeekChar(ss) == '}') {
		cJSON_SS_SkipChar(ss);
		return 1;	// empty object.
	}
	
	child = cJSON_New_Item();
	if (!child) {
		cJSON_SS_ParseError(ss, "Insufficient memory");
		return 0;
	}

	if (!cJSON_Parse_String( child, ss )) {
		cJSON_Delete(child);
		return 0;
	}
	cJSON_SS_SkipWhitespace(ss);

	name = child->valuestring;
	child->valuestring = 0;

	if (cJSON_SS_PeekChar(ss) != ':') {
		cJSON_SS_ParseError(ss, "Expected ':', got '%c'", cJSON_SS_PeekChar(ss));
		return 0;	// unexpected character!
	}

	cJSON_SS_SkipChar(ss);
	if (!cJSON_Parse_Value( child, ss )) {
		cJSON_Delete(child);
		return 0;
	}
	cJSON_SS_SkipWhitespace(ss);

	cJSON_AddItemToObject(item, name, child);
	free((void *)name);
	
	while (cJSON_SS_PeekChar(ss) == ',')
	{
		child = cJSON_New_Item();
		if ( !child ) {
			cJSON_SS_ParseError(ss, "Insufficient memory");
			return 0;
		}

		cJSON_SS_SkipChar(ss);
		cJSON_SS_SkipWhitespace(ss);
		if (!cJSON_Parse_String( child, ss)) {
			cJSON_Delete(child);
			return 0;
		}
		cJSON_SS_SkipWhitespace(ss);

		name = child->valuestring;
		child->valuestring = 0;

		if (cJSON_SS_PeekChar(ss) != ':') {
			cJSON_SS_ParseError(ss, "Expected ':', got '%c'", cJSON_SS_PeekChar(ss));
			return 0;	// unexpected character!
		}

		cJSON_SS_SkipChar(ss);
		if (!cJSON_Parse_Value( child, ss )) {
			cJSON_Delete(child);
			return 0;
		}
		cJSON_SS_SkipWhitespace(ss);

		cJSON_AddItemToObject(item, name, child);
		free((void *)name);
	}
	
	if (cJSON_SS_PeekChar(ss) == '}') {
		cJSON_SS_SkipChar(ss);
		return 1;	// end of object
	}

	cJSON_SS_ParseError(ss, "Expected '}', got '%c'", cJSON_SS_PeekChar(ss));
	return 0;	// malformed.	
}

// Render an object to text.
static void cJSON_Serialize_Object(cJSON *item, int depth, int fmt, cJSON_StringBuilder *sb)
{
	cJSON *child = item->child;

	depth++;
	
	cJSON_SB_AddChar(sb, '{');
	if (fmt) {
		cJSON_SB_AddChar(sb, '\n');
	}
	while (child) {
		if (fmt) {
			cJSON_SB_AddCharRep(sb, '\t', depth);
		}
		
		cJSON_Serialize_String_Ptr(child->string, sb);
		if (fmt) {
			cJSON_SB_AddStringN(sb, ":\t", 2);
		} else {
			cJSON_SB_AddChar(sb, ':');
		}
		
		cJSON_Serialize_Value(child, depth, fmt, sb);
		if (child->next) {
			cJSON_SB_AddChar(sb, ',');
		}
		if (fmt) {
			cJSON_SB_AddChar(sb, '\n');
		}
		child = child->next;
	}
	if (fmt) {
		cJSON_SB_AddCharRep(sb, '\t', depth - 1);
	}
	cJSON_SB_AddChar(sb, '}');
}

// Analyzer for pooled parser
// Scans a json structure to determine the required memory to allocate everything and the max string length

typedef struct cJSON_Analyze_Stack_s
{
	int type;	// 1 = array, 2 = object
	int items;
	struct cJSON_Analyze_Stack_s *next;
} cJSON_Analyze_Stack_t;

// Return codes:
// 0 = Success
// -1 = Invalid token
// -2 = Stack overflow
// -3 = Stack underflow
static int cJSON_ParsePooled_Analyze(const char *json, unsigned int *poolsize, unsigned int *maxstringlen)
{
	cJSON_Analyze_Stack_t __stack[64];
	cJSON_Analyze_Stack_t *free = 0;
	cJSON_Analyze_Stack_t *stack = 0;
	cJSON_Analyze_Stack_t *stemp = 0;
	cJSON_StringStream ss;
	unsigned int _poolsize = 0;
	unsigned int _maxstringlen = 0;
	unsigned int _stringlen = 0;
	int i;
	char symbol;

	// Set up the internal stack
	memset(&__stack, 0, sizeof(__stack));
	for (i=63; i >= 0; i--) {
		__stack[i].next = free;
		free = &__stack[i];
	}

	// Init the stream reader
	cJSON_SS_Init(&ss, json);
	
	// Main loop
	for(;;) 
	{
		cJSON_SS_SkipWhitespace(&ss);
		symbol = cJSON_SS_PeekChar(&ss);
		switch (symbol)
		{
		case 0:
			if (poolsize) *poolsize = _poolsize;
			if (maxstringlen) *maxstringlen = _maxstringlen;
			return 0;
		case '{':	// Start of object
			if (stack) {
				if (!stack->items) {
					stack->items = 1;		// If we're in an object/array, this means we got an item
				}
			}

			if (!free) return -2;		// No more free stack slots

			// Pop an item from the free stack and push it on the main stack
			stemp = free;
			free = free->next;
			stemp->next = stack;
			stack = stemp;

			stack->type = 2;	// Object
			stack->items = 0;

			cJSON_SS_SkipChar(&ss);
			break;
		case '[':	// Start of array
			if (stack) {
				if (!stack->items) {
					stack->items = 1;		// If we're in an object/array, this means we got an item
				}
			}

			if (!free) return -2;	// No more free stack slots

			// Pop an item from the free stack and push it on the main stack
			stemp = free;
			free = free->next;
			stemp->next = stack;
			stack = stemp;

			stack->type = 1;	// Array
			stack->items = 0;
			cJSON_SS_SkipChar(&ss);
			break;
		case ']':	// End of array
			if (!stack) return -3;
			
			if (!stack->next) {
				_poolsize += sizeof(cJSON);	// Only include the size of the array itself if it's not nested
			}
			_poolsize += sizeof(cJSON) * stack->items;	// Add the size of all items
			_poolsize += sizeof(cJSON *) * stack->items;	// Add the size of the table
			

			// Pop the item from the main stack and push it on the free stack
			stemp = stack;
			stack = stack->next;
			stemp->next = free;
			free = stemp;

			cJSON_SS_SkipChar(&ss);
			break;
		case '}':	// End of object
			if (!stack) return -3;
			
			if (!stack->next) {
				_poolsize += sizeof(cJSON);	// Only include the size of the array itself if it's not nested
			}
			_poolsize += sizeof(cJSON) * stack->items;	// Add the size of all items
			_poolsize += cJSON_HashTableSize * sizeof(cJSON *);		// Add the size of the hashtable
			
			// Pop the item from the main stack and push it on the free stack
			stemp = stack;
			stack = stack->next;
			stemp->next = free;
			free = stemp;

			cJSON_SS_SkipChar(&ss);
			break;
		case ',':	// New item in array/structure
			if (stack) {
				stack->items++;	// If we're in an object/array, this means we got a new item
			} else {
				return -1;		// Invalid token
			}
			cJSON_SS_SkipChar(&ss);
			break;
		case '\"':	// String
			if (stack) {
				if (!stack->items) {
					stack->items = 1;		// If we're in an object/array, this means we got an item
				}
			} else if (!_poolsize) {
				_poolsize = sizeof(cJSON);					// If we got nothing yet, this denotes a first (and last) item
			}
			// Analyze the string
			_stringlen = 1;	// Include the null terminator right away
			cJSON_SS_SkipChar(&ss);
			while (cJSON_SS_PeekChar(&ss) != '\"')
			{
				if (cJSON_SS_PeekChar(&ss) < 32) {
					return -1;	// Illegal character
				}
				
				if (cJSON_SS_PeekChar(&ss) != '\\') {
					_stringlen++;
					cJSON_SS_SkipChar(&ss);
				} else {
					cJSON_SS_SkipChar(&ss);
					switch (cJSON_SS_PeekChar(&ss))
					{
						case 'u':
							_stringlen++;
							cJSON_SS_Advance(&ss, 4);
							break;
						default:  
							_stringlen++;
							break;
					}
					cJSON_SS_SkipChar(&ss);
				}
			}
			cJSON_SS_SkipChar(&ss);
			_poolsize += _stringlen;
			if (_stringlen > _maxstringlen) _maxstringlen = _stringlen;
			break;
		default:	// Other token
			if (stack) {
				if (!stack->items) {
					stack->items = 1;		// If we're in an object/array, this means we got an item
				}
			} else if (!_poolsize) {
				_poolsize = sizeof(cJSON);					// If we got nothing yet, this denotes a first (and last) item
			}
			cJSON_SS_SkipToken(&ss);
			break;
		}
	}
}
static int cJSON_ParsePooled_Value(cJSON *item, cJSON_StringStream *ss);
static int cJSON_ParsePooled_String(cJSON *item, cJSON_StringStream *ss)
{
	cJSON_StringBuilder sb;
	unsigned uc;

	if (cJSON_SS_PeekChar(ss) != '\"') {
		cJSON_SS_ParseError(ss, "Expected '\"', got '%c'", cJSON_SS_PeekChar(ss));
		return 0;	// not a string!
	}

	if (!item->pool) {
		cJSON_SS_ParseError(ss, "Could not parse string: missing memory pool");
		return 0;
	}
	
	/* Initialize the string builder */

	if (!cJSON_SB_InitStatic(&sb, item->pool->stringbuf, item->pool->strbuflen)) {
		cJSON_SS_ParseError(ss, "Insufficient memory");
		return 0;	// Not enough memory
	}

	cJSON_SS_SkipChar(ss);
	while (cJSON_SS_PeekChar(ss) != '\"')
	{
		if (cJSON_SS_PeekChar(ss) < 32) {
			cJSON_SS_ParseError(ss, "Illegal character in string (%02X)", cJSON_SS_PeekChar(ss), cJSON_SS_PeekChar(ss));
			return 0;	// Illegal character
		}
		if (cJSON_SS_PeekChar(ss) != '\\') {
			if (!cJSON_SB_AddChar(&sb, cJSON_SS_GetChar(ss))) {
				cJSON_SS_ParseError(ss, "Insufficient memory");
				return 0;
			}
		} else {
			cJSON_SS_SkipChar(ss);
			switch (cJSON_SS_PeekChar(ss))
			{
				case 'b': 
					cJSON_SB_AddChar(&sb, '\b');
					break;
				case 'f':
					cJSON_SB_AddChar(&sb, '\f');
					break;
				case 'n':
					cJSON_SB_AddChar(&sb, '\n');
					break;
				case 'r':
					cJSON_SB_AddChar(&sb, '\r');
					break;
				case 't':
					cJSON_SB_AddChar(&sb, '\t');
					break;
				case 'u':	 // transcode utf16 to utf8. DOES NOT SUPPORT SURROGATE PAIRS CORRECTLY.
					uc = 0;
					sscanf( ss->pos + 1, "%4x", &uc);	// get the unicode char.
					
					// Clamp to ANSI range, everything else is replaced by a ?

					if (uc < 0x100) {
						cJSON_SB_AddChar(&sb, (char)uc);
					} else {
						cJSON_SB_AddChar(&sb, '?');
					}
					
					/*
					if (uc < 0x80) {
						len=1;
					} else if (uc < 0x800) {
						len=2;
					} else {
						len=3;
					}
					
					// Little hack here
					if (!cJSON_SB_CheckSpace(&sb, len)) {
						cJSON_SS_ParseError(ss, "Insufficient memory");
						return 0;
					}
					
					switch (len) {
						case 3: sb.buffer[sb.stringlen + 2] = ((uc | 0x80) & 0xBF); uc >>= 6;
						case 2: sb.buffer[sb.stringlen + 1] = ((uc | 0x80) & 0xBF); uc >>= 6;
						case 1: sb.buffer[sb.stringlen + 0] = (char)(uc | firstByteMark[len]);
					}

					sb.stringlen += len;
					*/
					cJSON_SS_Advance(ss, 4);
					break;
				default:  
					if (!cJSON_SB_AddChar(&sb, cJSON_SS_PeekChar(ss))) {
						cJSON_SS_ParseError(ss, "Insufficient memory");
						return 0;
					}
					break;
			}
			cJSON_SS_SkipChar(ss);
		}
	}
	cJSON_SS_SkipChar(ss);

	cJSON_SB_Finalize(&sb);	// This will add the null-terminator

	item->valuestring = (char *)cJSON_PooledMalloc(item->pool, sb.stringlen);
	if (!item->valuestring) {
		cJSON_SS_ParseError(ss, "Insufficient memory");
		return 0;
	}
	strncpy(item->valuestring, sb.buffer, sb.stringlen);
	item->type = cJSON_String;
	return 1;
}


// Build an array from input text.
static int cJSON_ParsePooled_Array(cJSON *item, cJSON_StringStream *ss)
{
	cJSON *child, *last;
	int i;

	if (cJSON_SS_PeekChar(ss) != '[') {
		cJSON_SS_ParseError(ss, "Expected '[', got '%c", cJSON_SS_PeekChar(ss));
		return 0;	// not an array!
	}

	if (!item->pool) {
		cJSON_SS_ParseError(ss, "Could not parse array: missing memory pool");
		return 0;
	}

	item->type = cJSON_Array;
	
	// The difference between the normal and the pooled parser, is that the dynamic array is created afterwards, rather than in advance

	cJSON_SS_SkipChar(ss);
	cJSON_SS_SkipWhitespace(ss);

	if (cJSON_SS_PeekChar(ss) == ']') {
		cJSON_SS_SkipChar(ss);
		return 1;	// empty array.
	}
	child = cJSON_New_ItemPooled(item->pool);

	if (!child) {
		cJSON_SS_ParseError(ss, "Insufficient memory");
		return 0;		 // memory fail
	}

	if (!cJSON_ParsePooled_Value(child, ss)) {	// Get the value
		// Do not set an error here, the parse function that retuned 0 will have done that already.
		return 0;
	}
	cJSON_SS_SkipWhitespace(ss);

	// Link the item manually
	item->child = child;
	item->arraysize = 1;
	child->linked = 1;

	last = child;

	while (cJSON_SS_PeekChar(ss) == ',')
	{
		child = cJSON_New_ItemPooled(item->pool);
		if ( !child ) {
			cJSON_SS_ParseError(ss, "Failed to allocate item for array");
			return 0; 	// memory fail
		}

		cJSON_SS_SkipChar(ss);
		if (!cJSON_ParsePooled_Value(child, ss)) {
			return 0;	// parse error
		}
		cJSON_SS_SkipWhitespace(ss);

		last->next = child;
		child->prev = last;
	
		item->arraysize++;

		child->linked = 1;
		last = child;
	}

	if (cJSON_SS_PeekChar(ss) != ']') {
		cJSON_SS_ParseError(ss, "Expected ']', got '%c'", cJSON_SS_PeekChar(ss));
		return 0;
	}
	
	// Construct the dynamic array
	item->table = (cJSON_s **)cJSON_PooledMalloc(item->pool, sizeof(cJSON *) * item->arraysize);
	if (!item->table) {
		cJSON_SS_ParseError(ss, "Insufficient memory");
		return 0;
	}

	for (i = 0, child = item->child; child; child = child->next, i++) 
	{
		item->table[i] = child;
	}

	cJSON_SS_SkipChar(ss);
	return 1;	// end of array	
}


// Build an object from the text.
static int cJSON_ParsePooled_Object(cJSON *item, cJSON_StringStream *ss)
{
	cJSON *child;
	long hash;

	if (cJSON_SS_PeekChar(ss) != '{') {
		cJSON_SS_ParseError(ss, "Expected '{', got '%c'", cJSON_SS_PeekChar(ss));
		return 0;	// not an object!
	}

	if (!item->pool) {
		cJSON_SS_ParseError(ss, "Could not parse array: missing memory pool");
		return 0;
	}
	
	// Create object
	item->type = cJSON_Object;

	item->table = (cJSON_s **)cJSON_PooledMalloc(item->pool, cJSON_HashTableSize * sizeof(cJSON *));
	memset(item->table, 0, cJSON_HashTableSize * sizeof(cJSON *));
	item->tablesize = cJSON_HashTableSize;

	cJSON_SS_SkipChar(ss);
	cJSON_SS_SkipWhitespace(ss);

	if (cJSON_SS_PeekChar(ss) == '}') {
		cJSON_SS_SkipChar(ss);
		return 1;	// empty object.
	}
	
	child = cJSON_New_ItemPooled(item->pool);
	if (!child) {
		cJSON_SS_ParseError(ss, "Insufficient memory");
		return 0;
	}

	if (!cJSON_ParsePooled_String( child, ss )) {
		return 0;
	}
	cJSON_SS_SkipWhitespace(ss);

	child->string = child->valuestring;
	child->valuestring = 0;

	if (cJSON_SS_PeekChar(ss) != ':') {
		cJSON_SS_ParseError(ss, "Expected ':', got '%c'", cJSON_SS_PeekChar(ss));
		return 0;	// unexpected character!
	}

	cJSON_SS_SkipChar(ss);
	if (!cJSON_ParsePooled_Value( child, ss )) {
		return 0;
	}
	cJSON_SS_SkipWhitespace(ss);

	hash = cJSON_GenerateHashValue(child->string, cJSON_HashTableSize);	// Get the hash
	child->hashnext = item->table[hash];							// Link it to the hashtable
	item->table[hash] = child;

	item->child = child;
	item->lastentry = child;

	child->linked = 1;
	item->arraysize++;

		
	while (cJSON_SS_PeekChar(ss) == ',')
	{
		child = cJSON_New_ItemPooled(item->pool);
		if ( !child ) {
			cJSON_SS_ParseError(ss, "Insufficient memory");
			return 0;
		}

		cJSON_SS_SkipChar(ss);
		cJSON_SS_SkipWhitespace(ss);
		if (!cJSON_ParsePooled_String( child, ss)) {
			return 0;
		}
		cJSON_SS_SkipWhitespace(ss);

		child->string = child->valuestring;
		child->valuestring = 0;

		if (cJSON_SS_PeekChar(ss) != ':') {
			cJSON_SS_ParseError(ss, "Expected ':', got '%c'", cJSON_SS_PeekChar(ss));
			return 0;	// unexpected character!
		}

		cJSON_SS_SkipChar(ss);
		if (!cJSON_ParsePooled_Value( child, ss )) {
			return 0;
		}
		cJSON_SS_SkipWhitespace(ss);
	
		hash = cJSON_GenerateHashValue(child->string, cJSON_HashTableSize);	// Get the hash
		child->hashnext = item->table[hash];							// Link it to the hashtable
		item->table[hash] = child;

		item->lastentry->next = child;
		child->prev = item->lastentry;
		item->lastentry = child;

		child->linked = 1;
		item->arraysize++;

	}
	
	if (cJSON_SS_PeekChar(ss) == '}') {
		cJSON_SS_SkipChar(ss);
		return 1;	// end of object
	}

	cJSON_SS_ParseError(ss, "Expected '}', got '%c'", cJSON_SS_PeekChar(ss));
	return 0;	// malformed.	
}


static int cJSON_ParsePooled_Value(cJSON *item, cJSON_StringStream *ss)
{
	if (!item->pool) {
		return 0;
	}

	cJSON_SS_SkipWhitespace(ss);
	
	if (!cJSON_SS_Compare(ss, "null", 4))	{
		item->type = cJSON_NULL;
		cJSON_SS_Advance(ss, 4);
		return 1;
	}
	if (!cJSON_SS_Compare(ss, "false", 5)) {
		item->type = cJSON_False;
		cJSON_SS_Advance(ss, 5);
		return 1;
	}
	if (!cJSON_SS_Compare(ss, "true", 4))	{
		item->type = cJSON_True;
		item->valueint = 1;
		cJSON_SS_Advance(ss, 4);
		return 1;
	}
	if (cJSON_SS_PeekChar(ss) == '\"')	{
		return cJSON_ParsePooled_String(item, ss);
	}
	if (cJSON_SS_PeekChar(ss) == '-' || (cJSON_SS_PeekChar(ss) >= '0' && cJSON_SS_PeekChar(ss) <= '9')) {
		return cJSON_Parse_Number(item, ss);
	}
	if (cJSON_SS_PeekChar(ss) == '[') {
		return cJSON_ParsePooled_Array(item, ss);
	}
	if (cJSON_SS_PeekChar(ss) == '{') {
		return cJSON_ParsePooled_Object(item, ss);
	}

	if (!cJSON_SS_PeekChar(ss)) {
		cJSON_SS_ParseError(ss, "Unexpected EOF");
	} else {
		cJSON_SS_ParseError(ss, "Unrecognised character: '%c'", cJSON_SS_PeekChar(ss));	
	}
	return 0;
}


cJSON *cJSON_ParsePooled(const char *data, char *error, size_t errorlen)
{
	unsigned int poolsize;
	unsigned int maxstrlen;

	cJSONMemPool_t *pool;
	cJSON_StringStream ss;
	cJSON *c;

	if (!data) {
		if (error) {
			strncpy(error, "data is NULL", errorlen);
		}
		return 0;       /* memory fail */
	}

	if (cJSON_ParsePooled_Analyze(data, &poolsize, &maxstrlen)) {
		if (error) {
			strncpy(error, "JSON analysis failed", errorlen);
		}
		return 0;       /* analysis failed */
	}

	pool = (cJSONMemPool_t *)cJSON_malloc(sizeof(cJSONMemPool_t));
	if (!pool) {
		if (error) {
			strncpy(error, "Insufficient memory", errorlen);
		}
		return 0;       /* memory fail */
	}
	pool->pool = (char *)cJSON_malloc(poolsize);
	if (!pool->pool) {
		cJSON_free(pool);
		if (error) {
			strncpy(error, "Insufficient memory", errorlen);
		}
		return 0;       /* memory fail */
	}
	pool->stringbuf = (char *)cJSON_malloc(maxstrlen);
	if (!pool->stringbuf) {
		cJSON_free(pool->pool);
		cJSON_free(pool);
		if (error) {
			strncpy(error, "Insufficient memory", errorlen);
		}
		return 0;       /* memory fail */
	}
	pool->poolsize = poolsize;
	pool->used = 0;
	pool->strbuflen = maxstrlen;


	c = cJSON_New_ItemPooled(pool);
	if (!c) {
		cJSON_free(pool->stringbuf);
		cJSON_free(pool->pool);
		cJSON_free(pool);
		if (error) {
			strncpy(error, "Insufficient memory", errorlen);
		}
		return 0;       /* memory fail */
	}

	cJSON_SS_Init(&ss, data);

	if (!cJSON_ParsePooled_Value(c, &ss)) {
		if (ss.error && error) {
			strncpy(error, ss.errorMsg, errorlen);
		}
		cJSON_Delete(c);
		return 0;
	}
	return c;
}

// Get Array size/item / object item.
int cJSON_GetArraySize(cJSON *arry)
{
	if (!arry || (arry->type != cJSON_Array && arry->type != cJSON_Object)) {
		return 0;	// Not an array or object
	}
	return arry->arraysize;
}

cJSON *cJSON_GetArrayItem(cJSON *arry, int item)
{
	if (!arry || arry->type != cJSON_Array || item < 0 || (size_t)item >= arry->arraysize) {
		return 0;	// Not an array or the index is out of bounds
	}
	return arry->table[item];
}

cJSON *cJSON_GetObjectItem(cJSON *object, const char *string)
{
	cJSON *c;
	long hash;
	if (!object || object->type != cJSON_Object) {
		return 0;	// Not an object
	}

	hash = cJSON_GenerateHashValue(string, cJSON_HashTableSize);
	for (c = object->table[hash]; c; c = c->hashnext) {
		if (!cJSON_strcasecmp(c->string,string)) {
			return c;
		}
	}
	return 0;
}

// Extended Item Lookup.
// Allows complex nested lookups
// 
// Format example:
// sub[5].myvar
// [2]
// test.sub[24].something[4].var

cJSON *cJSON_Lookup(cJSON *item, const char *path)
{
	char cmd[256] = {0};
	char *t;
	const char *s;
	int state = 0;	// 0 = awaiting initial input, 1 = pending object name, 2 = pending index, 3 = awaiting next item
	cJSON *c;

	c = item;
	t = &cmd[0];
	s = path;
	for (;;) {
		if (!*s) {
			if (state == 1) {
				*t = 0;
				c = cJSON_GetObjectItem(c, cmd);
				if (!c) {
					return 0;	// Invalid index
				}
				state = 3;
				t = &cmd[0];
			} else if (state == 2) {
				*t = 0;
				c = cJSON_GetArrayItem(c, atoi(cmd));
				if (!c) {
					return 0;	// Invalid index
				}
				state = 3;
				t = &cmd[0];
			}
			break;
		} else if (*s == '[') {
			// Opening bracket for index
			if (state == 1) {
				// Finalized an object name, do the lookup
				*t = 0;
				c = cJSON_GetObjectItem(c, cmd);
				if (!c) {
					return 0;	// Invalid index
				}
				state = 2;
				t = &cmd[0];
			} else if (state == 2) {
				// Syntax error
				return 0;
			} else {
				state = 2;
			}
		} else if ( *s == ']' ) {
			if (state == 2) {
				// Finalized an index name, do the lookup
				*t = 0;
				c = cJSON_GetArrayItem(c, atoi(cmd));
				if (!c) {
					return 0;	// Invalid index
				}
				state = 3;
				t = &cmd[0];
			} else {
				// Syntax error
				return 0;
			}
		} else if ( *s == '.' ) {
			if (state == 1) {
				*t = 0;
				c = cJSON_GetObjectItem(c, cmd);
				if (!c) {
					return 0;	// Invalid index
				}
				state = 1;
				t = &cmd[0];
			} else if (state == 3) {
				state = 1;
			} else {
				// Syntax error
				return 0;
			}
		} else if ( *s >= '0' && *s <= '9' ) {
			if (state == 0 || state == 3) state = 1;
			*t++ = *s;
		} else {
			if (state == 2) {
				// Syntax error, non-number in index
				return 0;
			}
			if (state == 0 || state == 3) state = 1;
			*t++ = *s;
		}
		s++;
	}
	return c;
}

// Iterator functions

cJSON *cJSON_GetFirstItem(cJSON *object)
{
	if (!object || (object->type != cJSON_Object && object->type != cJSON_Array)) {
		return 0;	// Not an object
	}

	return object->child;
}

cJSON *cJSON_GetNextItem(cJSON *item)
{
	return item->next;
}

const char * cJSON_GetItemKey(cJSON *item) {
	return item->string;
}

// Add item to arry/object.
void cJSON_AddItemToArray(cJSON *arry, cJSON *item)
{
	
	cJSON *c;
	if (!arry || arry->pool || arry->type != cJSON_Array) {
		return;
	}
	if (item->linked) {
		return;	// Item is already linked, dont allow a re-link
	}
	if (arry->tablesize == arry->arraysize) {
		// Enlarge the table
		void *tmp;
		tmp = cJSON_realloc(arry->table, (arry->tablesize + cJSON_ArrayBlockSize) * sizeof(cJSON *));
		if (tmp) {
			arry->table = (cJSON_s **)tmp;
			arry->tablesize += cJSON_ArrayBlockSize;
		} else {
			return;		// Allocation failed
		}
	}

	if (!arry->child) {
		arry->child=item;
		arry->table[0] = item;
		arry->arraysize = 1;
	} else {
		c = arry->table[arry->arraysize - 1]; // Get the last object in the array

		c->next = item;
		item->prev = c;
		
		arry->table[arry->arraysize] = item;	// Add it to the array
		arry->arraysize++;
	}
	item->linked = 1;
}

// Add item to arry/object.
void cJSON_InsertItemInArray(cJSON *arry, cJSON *item, int before)
{
	cJSON *c;
	if (!arry || arry->pool || arry->type != cJSON_Array) {
		return;
	}
	if (item->linked) {
		return;	// Item is already linked, dont allow a re-link
	}
	
	if (before < 0) {	// Clamp insertion point
		before = 0;
	} else if ((size_t)before >= arry->arraysize) {
		before = arry->arraysize - 1;
	}

	if (arry->tablesize == arry->arraysize) {
		// Enlarge the table
		void *tmp;
		tmp = cJSON_realloc(arry->table, (arry->tablesize + cJSON_ArrayBlockSize) * sizeof(cJSON *));
		if (tmp) {
			arry->table = (cJSON_s **)tmp;
			arry->tablesize += cJSON_ArrayBlockSize;
		} else {
			return;		// Allocation failed
		}
	}

	if (!arry->child) {
		arry->child=item;
		arry->table[0] = item;
		arry->arraysize = 1;
	} else {
		if (before == 0) {
			arry->child->prev = item;
			item->next = arry->child;
			arry->child = item;
		} else {
			c = arry->table[before-1]; // Get the object we want to insert it in front of

			item->next = c->next;
			item->next->prev = item;
			item->prev = c;
			c->next = item;	
		}
		// Shift the array
		memmove(&arry->table[before+1], &arry->table[before], (arry->arraysize - before) * sizeof(cJSON *));

		arry->table[before] = item;	// Add it to the array
		arry->arraysize++;
	}
	item->linked = 1;
}

void cJSON_AddItemToObject(cJSON *object, const char *string, cJSON *item)
{
	long hash;
	if (!object || object->pool || object->type != cJSON_Object) {
		return;
	}
	if (item->linked) {
		return;	// Item is already linked, dont allow a re-link
	}
	if (item->string) {
		cJSON_free(item->string);
	}
	item->string = cJSON_strdup(string);

	cJSON_DeleteItemFromObject(object, string);		// If the item already exists, delete it
	
	hash = cJSON_GenerateHashValue(string, cJSON_HashTableSize);	// Get the hash
	item->hashnext = object->table[hash];							// Link it to the hashtable
	object->table[hash] = item;

	if (!object->lastentry) {	// No lastentry = no children
		object->child = item;
		object->lastentry = item;
	} else {
		object->lastentry->next = item;
		item->prev = object->lastentry;
		object->lastentry = item;
	}

	item->linked = 1;
	object->arraysize++;
}

cJSON *cJSON_DetachItemFromArray(cJSON *arry, int which)
{
	cJSON *c;
	if (!arry || arry->pool || arry->type != cJSON_Array || which < 0 || (size_t)which >= arry->arraysize) {
		return 0;
	}

	c = arry->table[which];
	
	if (!c) {
		return 0;
	}
	if (c->prev) {
		c->prev->next = c->next;
	}

	if (c->next) {
		c->next->prev = c->prev;
	}
	
	if (c == arry->child) {
		arry->child = c->next;
	}
	c->prev = c->next = 0;

	if ((size_t)which != arry->arraysize - 1) {
		// Shift it out of the dynamic array
		memmove(&arry->table[which], &arry->table[which+1], (arry->arraysize - (which+1)) * sizeof(cJSON *));
	}
	
	arry->arraysize--;
	if (arry->tablesize - arry->arraysize > 31) {
		// Got enough slack, lets shrink the array
		void *tmp;
		tmp = realloc(arry->table, (arry->tablesize - cJSON_ArrayBlockSize) * sizeof(cJSON *));
		if (tmp) {
			arry->table = (cJSON_s **)tmp;
			arry->tablesize = arry->tablesize - cJSON_ArrayBlockSize;
		}
	}

	c->linked = 0;
	return c;
}

void cJSON_DeleteItemFromArray(cJSON *arry, int which)
{
	cJSON_Delete( cJSON_DetachItemFromArray(arry, which) ); 
}

cJSON *cJSON_DetachItemFromObject(cJSON *object, const char *string)
{
	cJSON *c = 0, *p = 0;
	long hash;

	if (!object || object->pool || object->type != cJSON_Object) {
		return NULL;
	}

	hash = cJSON_GenerateHashValue(string, cJSON_HashTableSize);

	for (c = object->table[hash]; c; c = c->hashnext) {
		if (!cJSON_strcasecmp(c->string, string)) {
			if (c->prev) {
				c->prev->next = c->next;
			}

			if (c->next) {
				c->next->prev = c->prev;
			}
			
			if (c == object->child) {
				object->child = c->next;
			}
			if (c == object->lastentry) {
				object->lastentry = c->prev;
			}
			c->prev = c->next = 0;
			
			if (p) {	// Unlink it from the hashtable
				p->hashnext = c->hashnext;
			} else {
				object->table[hash] = c->hashnext;
			}

			c->linked = 0;
			object->arraysize--;
			return c;
		}
		p = c;
	}
	return 0;
}

void cJSON_DeleteItemFromObject(cJSON *object, const char *string)
{
	cJSON_Delete( cJSON_DetachItemFromObject(object, string) );
}

void cJSON_ClearItemsFromObject(cJSON *object)
{
	if (!object || object->pool || object->type != cJSON_Object) {
		return;
	}
	if (object->child) {
		object->child->linked = 0;
		cJSON_Delete(object->child);
		object->child = 0;
	}
	object->arraysize = 0;
	memset(object->table, 0, cJSON_HashTableSize * sizeof(cJSON *));
	return;
}


void cJSON_ClearItemsFromArray(cJSON *arry)
{
	void *tmp;
	if (!arry || arry->pool || arry->type != cJSON_Array) {
		return;
	}
	if (arry->child) {
		arry->child->linked = 0;
		cJSON_Delete(arry->child);
		arry->child = 0;
	}
	tmp = realloc(arry->table, cJSON_ArrayBlockSize * sizeof(cJSON *));
	if (tmp) {
		arry->table = (cJSON_s **)tmp;
		arry->tablesize = cJSON_ArrayBlockSize;
	}
	arry->arraysize = 0;
	return;
}


// Replace arry/object items with new ones.
void cJSON_ReplaceItemInArray(cJSON *arry, int which, cJSON *newitem)
{
	cJSON *c;
	if (!arry || arry->pool || arry->type != cJSON_Array || which < 0 || (size_t)which >= arry->arraysize) {
		return;
	}

	c = arry->table[which];
	
	if (!c) {
		return;
	}

	newitem->next = c->next;
	newitem->prev = c->prev;
	
	if (newitem->next) {
		newitem->next->prev = newitem;
	}

	if (c == arry->child) {
		arry->child = newitem;
	} else { 
		newitem->prev->next = newitem;
	}
	c->next = c->prev = 0;

	arry->table[which] = newitem;
	
	cJSON_Delete(c);
}

__inline void cJSON_ReplaceItemInObject(cJSON *object, const char *string, cJSON *newitem)
{
	// AddItemToObject already deletes the existing item, so lets just call that
	cJSON_AddItemToObject(object, string, newitem);
}


void cJSON_SwapItemsInArray(cJSON *arry, int item1, int item2)
{
	cJSON *i1, *i2;
	cJSON *n, *p;
	if (!arry || arry->pool || arry->type != cJSON_Array || item1 < 0 || (size_t)item1 >= arry->arraysize || item2 < 0 || (size_t)item2 >= arry->arraysize) {
		return;
	}

	i1 = arry->table[item1];
	i2 = arry->table[item2];
	
	if (!i1 || !i2) {
		return;
	}

	// Swap the linkage
	n = i1->next;
	p = i1->prev;
	i1->next = i2->next;
	i1->prev = i2->prev;
	i2->next = n;
	i2->prev = p;

	// Swap the array indexes
	arry->table[item1] = i2;
	arry->table[item2] = i1;

	// Fix links TO the swapped items

	if (i1->prev) {
		i1->prev->next = i1;
	} else {
		// Only way prev is NULL, is if it's the first item
		arry->child = i1;
	}
	if (i1->next) {
		i1->next->prev = i1;
	}

	if (i2->prev) {
		i2->prev->next = i2;
	} else {
		arry->child = i2;
	}
	if (i2->next) {
		i2->next->prev = i2;
	}	
}

// Duplicate items

cJSON *cJSON_DuplicateItem(cJSON *item)
{
	cJSON *c = 0, *n = 0;	// Current, New Used for copying child items
	cJSON *newitem = cJSON_New_Item();
	
	newitem->type = item->type;
	newitem->valuedouble = item->valuedouble;
	newitem->valueint = item->valueint;
	newitem->valuestring = cJSON_strdup(item->valuestring);

	if (newitem->type == cJSON_Array) {
		newitem->table = (cJSON_s **)cJSON_malloc(cJSON_ArrayBlockSize * sizeof(cJSON *));
		newitem->tablesize = cJSON_ArrayBlockSize;
	} else if (item->type == cJSON_Object) {
		newitem->table = (cJSON_s **)cJSON_malloc(cJSON_HashTableSize * sizeof(cJSON *));
		memset(newitem->table, 0, cJSON_HashTableSize * sizeof(cJSON *));
		newitem->tablesize = cJSON_HashTableSize;
	} else {
		return newitem;
	}

	if (item->child) {
		for (c = item->child; c; c = c->next) {
			n = cJSON_DuplicateItem(c);
			
			if (item->type == cJSON_Object) {
				cJSON_AddItemToObject(newitem, c->string, n);
			} else {
				cJSON_AddItemToArray(newitem, n);
			}
		}
	}
	return newitem;
}

// Create basic types:
cJSON *cJSON_CreateNull()
{
	cJSON *item = cJSON_New_Item();
	item->type = cJSON_NULL;
	return item;
}

cJSON *cJSON_CreateTrue()
{
	cJSON *item = cJSON_New_Item();
	item->type = cJSON_True;
	return item;
}

cJSON *cJSON_CreateFalse()
{
	cJSON *item = cJSON_New_Item();
	item->type = cJSON_False;
	return item;
}

cJSON *cJSON_CreateBoolean(int boolean)
{
	cJSON *item = cJSON_New_Item();
	item->type = boolean ? cJSON_True : cJSON_False;
	return item;
}

cJSON *cJSON_CreateNumber(double num)
{
	cJSON *item = cJSON_New_Item();
	item->type = cJSON_Number;
	item->valuedouble = num;
	item->valueint = (int)num;
	return item;
}

cJSON *cJSON_CreateInteger(int num)
{
	cJSON *item = cJSON_New_Item();
	item->type = cJSON_Number;
	item->valuedouble = (double)num;
	item->valueint = num;
	return item;
}

cJSON *cJSON_CreateString(const char *string)
{
	cJSON *item = cJSON_New_Item();
	item->type = cJSON_String;
	item->valuestring = cJSON_strdup(string);
	return item;
}

cJSON *cJSON_CreateArray()
{
	cJSON *item = cJSON_New_Item();
	item->type = cJSON_Array;
	
	item->table = (cJSON_s **)cJSON_malloc(cJSON_ArrayBlockSize * sizeof(cJSON *));
	item->tablesize = cJSON_ArrayBlockSize;
	return item;
}

cJSON *cJSON_CreateObject()
{
	cJSON *item = cJSON_New_Item();
	item->type = cJSON_Object;

	item->table = (cJSON_s **)cJSON_malloc(cJSON_HashTableSize * sizeof(cJSON *));
	memset(item->table, 0, cJSON_HashTableSize * sizeof(cJSON *));
	item->tablesize = cJSON_HashTableSize;
	return item;
}

// Create Arrays
cJSON *cJSON_CreateIntArray(int count, int *numbers)
{
	int i;
	cJSON *n = 0, *a = cJSON_CreateArray();
	
	for(i = 0; i < count; i++) {
		n = cJSON_CreateNumber(numbers[i]);
		cJSON_AddItemToArray(a, n);
	}
	return a;
}

cJSON *cJSON_CreateFloatArray(int count, float *numbers)
{
	int i;
	cJSON *n = 0, *a = cJSON_CreateArray();
	
	for( i = 0; i < count; i++) {
		n = cJSON_CreateNumber(numbers[i]);
		cJSON_AddItemToArray(a, n);
	}
	return a;
}

cJSON *cJSON_CreateDoubleArray(int count, double *numbers)
{
	int i;
	cJSON *n = 0, *a = cJSON_CreateArray();
	
	for( i = 0; i < count; i++) {
		n = cJSON_CreateNumber(numbers[i]);
		cJSON_AddItemToArray(a, n);
	}
	return a;
}

cJSON *cJSON_CreateStringArray(int count, const char **strings)
{
	int i;
	cJSON *n = 0, *a = cJSON_CreateArray();
	
	for( i = 0; i < count; i++) {
		if (!strings[i]) {
			n = cJSON_CreateNull();
		} else {
			n = cJSON_CreateString(strings[i]);
		}
		cJSON_AddItemToArray(a, n);
	}
	return a;
}


// Create Arrays using varargs:
cJSON *cJSON_CreateIntArrayv(int count, ...)
{
	int i;
	va_list va;
	cJSON *n = 0, *a = cJSON_CreateArray();

	va_start(va, count);
	
	for(i = 0; i < count; i++) {
		n = cJSON_CreateNumber(va_arg(va, int));
		cJSON_AddItemToArray(a, n);
	}

	va_end(va);
	return a;
}

cJSON *cJSON_CreateFloatArrayv(int count, ...)
{
	int i;
	va_list va;
	cJSON *n = 0, *a = cJSON_CreateArray();

	va_start(va, count);
	
	for( i = 0; i < count; i++) {
		n = cJSON_CreateNumber(va_arg(va, float));
		cJSON_AddItemToArray(a, n);
	}

	va_end(va);

	return a;
}

cJSON *cJSON_CreateDoubleArrayv(int count, ...)
{
	int i;
	va_list va;
	cJSON *n = 0, *a = cJSON_CreateArray();
	
	va_start(va, count);

	for( i = 0; i < count; i++) {
		n = cJSON_CreateNumber(va_arg(va, double));
		cJSON_AddItemToArray(a, n);
	}
	va_end(va);
	return a;
}

cJSON *cJSON_CreateStringArrayv(int count, ...)
{
	int i;
	const char * arg;
	va_list va;
	cJSON *n = 0, *a = cJSON_CreateArray();
	
	va_start(va, count);

	for( i = 0; i < count; i++) {
		arg = va_arg(va, const char *);
		if (!arg) {
			n = cJSON_CreateNull();
		} else {
			n = cJSON_CreateString(arg);
		}
		cJSON_AddItemToArray(a, n);
	}

	va_end(va);
	return a;
}

// Read Arrays
int cJSON_ReadIntArray(cJSON* arry, int count, int *numbers)
{
	int i;
	int n;
	int len;
	int read = 0;

	if (arry->type != cJSON_Array) {
		return 0;
	}

	len = cJSON_GetArraySize(arry);

	if (len > count) len = count;

	for(i = 0; i < len; i++) {
		n = cJSON_ToInteger(cJSON_GetArrayItem(arry, i));
		numbers[read++] = n;
	}
	return read;
}

int cJSON_ReadFloatArray(cJSON *arry, int count, float *numbers)
{
	int i;
	float n;
	int len;
	int read = 0;

	if (arry->type != cJSON_Array) {
		return 0;
	}

	len = cJSON_GetArraySize(arry);

	if (len > count) len = count;

	for(i = 0; i < len; i++) {
		n = (float)cJSON_ToNumber(cJSON_GetArrayItem(arry, i));
		numbers[read++] = n;
	}
	return read;
}

int cJSON_ReadDoubleArray(cJSON *arry, int count, double *numbers)
{
	int i;
	double n;
	int len;
	int read = 0;

	if (arry->type != cJSON_Array) {
		return 0;
	}

	len = cJSON_GetArraySize(arry);

	if (len > count) len = count;

	for(i = 0; i < len; i++) {
		n = cJSON_ToNumber(cJSON_GetArrayItem(arry, i));
		numbers[read++] = n;
	}
	return read;
}

int cJSON_ReadStringArray(cJSON* arry, int count, const char **strings)
{
	int i;
	const char *s;
	int len;
	int read = 0;

	if (arry->type != cJSON_Array) {
		return 0;
	}

	len = cJSON_GetArraySize(arry);

	if (len > count) len = count;

	for(i = 0; i < len; i++) {
		s = cJSON_ToString(cJSON_GetArrayItem(arry, i));
		strings[read++] = s;
	}
	return read;
}


// Read Arrays using varargs:
int cJSON_ReadIntArrayv(cJSON* arry, int count, ...)
{
	int i;
	int n;
	int *number;
	int len;
	int read = 0;
	va_list va;

	if (arry->type != cJSON_Array) {
		return 0;
	}

	len = cJSON_GetArraySize(arry);

	if (len > count) len = count;

	va_start(va, count);

	for(i = 0; i < len; i++) {
		n = cJSON_ToInteger(cJSON_GetArrayItem(arry, i));
		number = va_arg(va, int *);
		if (number) *number = n;
		read++;
	}
	
	va_end(va);

	return read;
}

int cJSON_ReadFloatArrayv(cJSON *arry, int count, ...)
{
	int i;
	float n;
	float *number;
	int len;
	int read = 0;
	va_list va;

	if (arry->type != cJSON_Array) {
		return 0;
	}

	len = cJSON_GetArraySize(arry);

	if (len > count) len = count;

	va_start(va, count);

	for(i = 0; i < len; i++) {
		n = (float)cJSON_ToNumber(cJSON_GetArrayItem(arry, i));
		number = va_arg(va, float *);
		if (number) *number = n;
		read++;
	}
	
	va_end(va);

	return read;
}

int cJSON_ReadDoubleArrayv(cJSON *arry, int count, ...)
{
	int i;
	double n;
	double *number;
	int len;
	int read = 0;
	va_list va;

	if (arry->type != cJSON_Array) {
		return 0;
	}

	len = cJSON_GetArraySize(arry);

	if (len > count) len = count;

	va_start(va, count);

	for(i = 0; i < len; i++) {
		n = cJSON_ToNumber(cJSON_GetArrayItem(arry, i));
		number = va_arg(va, double *);
		if (number) *number = n;
		read++;
	}
	
	va_end(va);

	return read;
}

int cJSON_ReadStringArrayv(cJSON * arry, int count, ...)
{
	int i;
	const char *s;
	const char **string;
	int len;
	int read = 0;
	va_list va;

	if (arry->type != cJSON_Array) {
		return 0;
	}

	len = cJSON_GetArraySize(arry);

	if (len > count) len = count;

	va_start(va, count);

	for(i = 0; i < len; i++) {
		s = cJSON_ToString(cJSON_GetArrayItem(arry, i));
		string = va_arg(va, const char **);
		if (string) *string = s;
		read++;
	}
	
	va_end(va);

	return read;
}

// Type checking (inlined)

int cJSON_IsNULL(cJSON *item) {
	return item->type == cJSON_NULL;
}

int cJSON_IsTrue(cJSON *item) {
	return item->type == cJSON_True;
}

int cJSON_IsFalse(cJSON *item) {
	return item->type == cJSON_False;
}

int cJSON_IsBoolean(cJSON *item) {
	return item->type == cJSON_True || item->type == cJSON_False;
}

int cJSON_IsNumber(cJSON *item) {
	return item->type == cJSON_Number;
}

int cJSON_IsString(cJSON *item) {
	return item->type == cJSON_String;
}

int cJSON_IsArray(cJSON *item) {
	return item->type == cJSON_Array;
}

int cJSON_IsObject(cJSON *item) {
	return item->type == cJSON_Object;
}

int cJSON_IsLinked(cJSON *item) {
	return item->linked;
}

int cJSON_ToBooleanOpt(cJSON *item, int defval)
{
	if (!item) {
		return defval;
	}
	switch (item->type)
	{
	case cJSON_False:
		return 0;
	case cJSON_True:
		return 1;
	case cJSON_NULL:
		return 0;
	case cJSON_Number:
		return (item->valueint != 0);
	case cJSON_String:
	case cJSON_Array:
	case cJSON_Object:
	default:
		return defval;
	}
}

double cJSON_ToNumberOpt(cJSON *item, double defval)
{
	if (!item) {
		return defval;
	}
	switch (item->type)
	{
	case cJSON_False:
	case cJSON_True:
	case cJSON_NULL:
		return defval;
	case cJSON_Number:
		return item->valuedouble;
	case cJSON_String:
		return atof(item->valuestring);
	case cJSON_Array:
	case cJSON_Object:
	default:
		return defval;
	}
}

int cJSON_ToIntegerOpt(cJSON *item, int defval)
{
	if (!item) {
		return defval;
	}
	switch (item->type)
	{
	case cJSON_False:
	case cJSON_True:
	case cJSON_NULL:
		return defval;
	case cJSON_Number:
		return item->valueint;
	case cJSON_String:
		return atoi(item->valuestring);
	case cJSON_Array:
	case cJSON_Object:
	default:
		return defval;
	}
}

const char *cJSON_ToStringOpt(cJSON *item, const char *defval)
{
	if (!item) {
		return defval;
	}
	switch (item->type)
	{
	case cJSON_False:
	case cJSON_True:
	case cJSON_NULL:
	case cJSON_Number:
		return defval;
	case cJSON_String:
		return item->valuestring;
	case cJSON_Array:
	case cJSON_Object:
	default:
		return defval;
	}
}

int cJSON_ToBoolean(cJSON *item)
{
	return cJSON_ToBooleanOpt(item, 0);
}

double cJSON_ToNumber(cJSON *item)
{
	return cJSON_ToNumberOpt(item, 0);
}

int cJSON_ToInteger(cJSON *item)
{
	return cJSON_ToIntegerOpt(item, 0);
}

const char * cJSON_ToString(cJSON *item)
{
	return cJSON_ToStringOpt(item, 0);
}

int cJSON_ToBooleanRaw(cJSON *item)
{
	return item->type == cJSON_True ? 1 : 0;
}

double cJSON_ToNumberRaw(cJSON *item)
{
	return item->valuedouble;
}

int cJSON_ToIntegerRaw(cJSON *item)
{
	return item->valueint;
}

const char * cJSON_ToStringRaw(cJSON *item)
{
	return item->valuestring;
}


void cJSON_SetStringValue(cJSON *item, const char *value)
{
	if (!item || item->pool || item->type == cJSON_Array || item->type == cJSON_Object) {
		return;
	}
	item->type = cJSON_String;

	if (item->valuestring) {
		free(item->valuestring);
	}
	item->valuestring = cJSON_strdup(value);
	item->valueint = 0;
	item->valuedouble = 0.0;
}

void cJSON_SetNumberValue(cJSON *item, double number)
{
	if (!item || item->pool || item->type == cJSON_Array || item->type == cJSON_Object) {
		return;
	}
	item->type = cJSON_Number;

	if (item->valuestring) {
		free(item->valuestring);
		item->valuestring = 0;
	}
	
	item->valueint = (int)number;
	item->valuedouble = number;
}

void cJSON_SetIntegerValue(cJSON *item, int integer)
{
	if (!item || item->pool || item->type == cJSON_Array || item->type == cJSON_Object) {
		return;
	}
	item->type = cJSON_Number;

	if (item->valuestring) {
		free(item->valuestring);
		item->valuestring = 0;
	}
	
	item->valueint = integer;
	item->valuedouble = (double)integer;
}

void cJSON_SetBooleanValue(cJSON *item, int boolean)
{
	if (!item || item->pool || item->type == cJSON_Array || item->type == cJSON_Object) {
		return;
	}

	item->type = boolean ? cJSON_True : cJSON_False;

	if (item->valuestring) {
		free(item->valuestring);
		item->valuestring = 0;
	}
	
	item->valueint = 0;
	item->valuedouble = 0;
}

void cJSON_SetNULLValue(cJSON *item)
{
	if (!item || item->pool || item->type == cJSON_Array || item->type == cJSON_Object ) {
		return;
	}

	item->type = cJSON_NULL;

	if (item->valuestring) {
		free(item->valuestring);
		item->valuestring = 0;
	}
	
	item->valueint = 0;
	item->valuedouble = 0;
}
