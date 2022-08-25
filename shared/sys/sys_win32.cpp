/*
===========================================================================
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
#include "sys_local.h"
#include <direct.h>
#include <io.h>
#include <shlobj.h>
#include <windows.h>
#include <psapi.h>

#pragma comment( lib, "psapi.lib" )	//fix for winxp compiles --futuza
#pragma comment( lib, "dbghelp.lib" )

#include <DbgHelp.h>

#define MEM_THRESHOLD (128*1024*1024)

// Used to determine where to store user-specific files
static char homePath[ MAX_OSPATH ] = { 0 };

static UINT timerResolution = 0;

static bool bLoadedImgHelp = false;
static HANDLE stackProcess;
static HANDLE stackThread;

/*
==============
Sys_Basename
==============
*/
const char *Sys_Basename( char *path )
{
	static char base[ MAX_OSPATH ] = { 0 };
	int length;

	length = strlen( path ) - 1;

	// Skip trailing slashes
	while( length > 0 && path[ length ] == '\\' )
		length--;

	while( length > 0 && path[ length - 1 ] != '\\' )
		length--;

	Q_strncpyz( base, &path[ length ], sizeof( base ) );

	length = strlen( base ) - 1;

	// Strip trailing slashes
	while( length > 0 && base[ length ] == '\\' )
    base[ length-- ] = '\0';

	return base;
}

/*
==============
Sys_Dirname
==============
*/
const char *Sys_Dirname( char *path )
{
	static char dir[ MAX_OSPATH ] = { 0 };
	int length;

	Q_strncpyz( dir, path, sizeof( dir ) );
	length = strlen( dir ) - 1;

	while( length > 0 && dir[ length ] != '\\' )
		length--;

	dir[ length ] = '\0';

	return dir;
}

/*
================
Sys_Milliseconds
================
*/
int Sys_Milliseconds (bool baseTime)
{
	static int sys_timeBase = timeGetTime();
	int			sys_curtime;

	sys_curtime = timeGetTime();
	if(!baseTime)
	{
		sys_curtime -= sys_timeBase;
	}

	return sys_curtime;
}

int Sys_Milliseconds2( void )
{
	return Sys_Milliseconds(false);
}

/*
================
Sys_RandomBytes
================
*/
bool Sys_RandomBytes( byte *string, int len )
{
	HCRYPTPROV  prov;

	if( !CryptAcquireContext( &prov, NULL, NULL,
		PROV_RSA_FULL, CRYPT_VERIFYCONTEXT ) )  {

		return false;
	}

	if( !CryptGenRandom( prov, len, (BYTE *)string ) )  {
		CryptReleaseContext( prov, 0 );
		return false;
	}
	CryptReleaseContext( prov, 0 );
	return true;
}

/*
==================
Sys_GetCurrentUser
==================
*/
char *Sys_GetCurrentUser( void )
{
	static char s_userName[1024];
	DWORD size = sizeof( s_userName );

	if ( !GetUserName( s_userName, &size ) )
		strcpy( s_userName, "player" );

	if ( !s_userName[0] )
	{
		strcpy( s_userName, "player" );
	}

	return s_userName;
}

/*
* Builds the path for the user's game directory
*/
char *Sys_DefaultHomePath( void )
{
#if defined(_PORTABLE_VERSION)
	Com_Printf( "Portable install requested, skipping homepath support\n" );
	return NULL;
#else
	if ( !homePath[0] )
	{
		TCHAR homeDirectory[MAX_PATH];

		if( !SUCCEEDED( SHGetFolderPath( NULL, CSIDL_PERSONAL, NULL, 0, homeDirectory ) ) )
		{
			Com_Printf( "Unable to determine your home directory.\n" );
			return NULL;
		}

		Com_sprintf( homePath, sizeof( homePath ), "%s%cMy Games%c", homeDirectory, PATH_SEP, PATH_SEP );
		if ( com_homepath && com_homepath->string[0] )
			Q_strcat( homePath, sizeof( homePath ), com_homepath->string );
		else
			Q_strcat( homePath, sizeof( homePath ), HOMEPATH_NAME_WIN );
	}

	return homePath;
#endif
}

static const char *GetErrorString( DWORD error ) {
	static char buf[MAX_STRING_CHARS];
	buf[0] = '\0';

	if ( error ) {
		LPVOID lpMsgBuf;
		DWORD bufLen = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, error, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), (LPTSTR)&lpMsgBuf, 0, NULL );
		if ( bufLen ) {
			LPCSTR lpMsgStr = (LPCSTR)lpMsgBuf;
			Q_strncpyz( buf, lpMsgStr, Q_min( (size_t)(lpMsgStr + bufLen), sizeof(buf) ) );
			LocalFree( lpMsgBuf );
		}
	}
	return buf;
}

void Sys_SetProcessorAffinity( void ) {
	DWORD_PTR processMask, processAffinityMask, systemAffinityMask;
	HANDLE handle = GetCurrentProcess();

	if ( !GetProcessAffinityMask( handle, &processAffinityMask, &systemAffinityMask ) )
		return;

	if ( sscanf( com_affinity->string, "%X", &processMask ) != 1 )
		processMask = 1; // set to first core only

	if ( !processMask )
		processMask = systemAffinityMask; // use all the cores available to the system

	if ( processMask == processAffinityMask )
		return; // no change

	if ( !SetProcessAffinityMask( handle, processMask ) )
		Com_DPrintf( "Setting affinity mask failed (%s)\n", GetErrorString( GetLastError() ) );
}

/*
==================
Sys_LowPhysicalMemory()
==================
*/

qboolean Sys_LowPhysicalMemory(void) {
	static MEMORYSTATUSEX stat;
	static qboolean bAsked = qfalse;
	static cvar_t* sys_lowmem = Cvar_Get( "sys_lowmem", "0", 0 );

	if (!bAsked)	// just in case it takes a little time for GlobalMemoryStatusEx() to gather stats on
	{				//	stuff we don't care about such as virtual mem etc.
		bAsked = qtrue;

		stat.dwLength = sizeof (stat);
		GlobalMemoryStatusEx (&stat);
	}
	if (sys_lowmem->integer)
	{
		return qtrue;
	}
	return (stat.ullTotalPhys <= MEM_THRESHOLD) ? qtrue : qfalse;
}

/*
==============
Sys_Mkdir
==============
*/
qboolean Sys_Mkdir( const char *path ) {
	if( !CreateDirectory( path, NULL ) )
	{
		if( GetLastError( ) != ERROR_ALREADY_EXISTS )
			return qfalse;
	}
	return qtrue;
}

/*
==============
Sys_Cwd
==============
*/
char *Sys_Cwd( void ) {
	static char cwd[MAX_OSPATH];

	_getcwd( cwd, sizeof( cwd ) - 1 );
	cwd[MAX_OSPATH-1] = 0;

	return cwd;
}

/* Resolves path names and determines if they are the same */
/* For use with full OS paths not quake paths */
/* Returns true if resulting paths are valid and the same, otherwise false */
bool Sys_PathCmp( const char *path1, const char *path2 ) {
	char *r1, *r2;

	r1 = _fullpath(NULL, path1, MAX_OSPATH);
	r2 = _fullpath(NULL, path2, MAX_OSPATH);

	if(r1 && r2 && !Q_stricmp(r1, r2))
	{
		free(r1);
		free(r2);
		return true;
	}

	free(r1);
	free(r2);
	return false;
}

/*
==============================================================

DIRECTORY SCANNING

==============================================================
*/

#define	MAX_FOUND_FILES	0x1000

void Sys_ListFilteredFiles( const char *basedir, char *subdirs, char *filter, char **psList, int *numfiles ) {
	char		search[MAX_OSPATH], newsubdirs[MAX_OSPATH];
	char		filename[MAX_OSPATH];
	intptr_t	findhandle;
	struct _finddata_t findinfo;

	if ( *numfiles >= MAX_FOUND_FILES - 1 ) {
		return;
	}

	if (strlen(subdirs)) {
		Com_sprintf( search, sizeof(search), "%s\\%s\\*", basedir, subdirs );
	}
	else {
		Com_sprintf( search, sizeof(search), "%s\\*", basedir );
	}

	findhandle = _findfirst (search, &findinfo);
	if (findhandle == -1) {
		return;
	}

	do {
		if (findinfo.attrib & _A_SUBDIR) {
			if (Q_stricmp(findinfo.name, ".") && Q_stricmp(findinfo.name, "..")) {
				if (strlen(subdirs)) {
					Com_sprintf( newsubdirs, sizeof(newsubdirs), "%s\\%s", subdirs, findinfo.name);
				}
				else {
					Com_sprintf( newsubdirs, sizeof(newsubdirs), "%s", findinfo.name);
				}
				Sys_ListFilteredFiles( basedir, newsubdirs, filter, psList, numfiles );
			}
		}
		if ( *numfiles >= MAX_FOUND_FILES - 1 ) {
			break;
		}
		Com_sprintf( filename, sizeof(filename), "%s\\%s", subdirs, findinfo.name );
		if (!Com_FilterPath( filter, filename, qfalse ))
			continue;
		psList[ *numfiles ] = CopyString( filename );
		(*numfiles)++;
	} while ( _findnext (findhandle, &findinfo) != -1 );

	_findclose (findhandle);
}

static qboolean strgtr(const char *s0, const char *s1) {
	int l0, l1, i;

	l0 = strlen(s0);
	l1 = strlen(s1);

	if (l1<l0) {
		l0 = l1;
	}

	for(i=0;i<l0;i++) {
		if (s1[i] > s0[i]) {
			return qtrue;
		}
		if (s1[i] < s0[i]) {
			return qfalse;
		}
	}
	return qfalse;
}

char **Sys_ListFiles( const char *directory, const char *extension, char *filter, int *numfiles, qboolean wantsubs ) {
	char		search[MAX_OSPATH];
	int			nfiles;
	char		**listCopy;
	char		*list[MAX_FOUND_FILES];
	struct _finddata_t findinfo;
	intptr_t	findhandle;
	int			flag;
	int			i;
	int			extLen;

	if (filter) {

		nfiles = 0;
		Sys_ListFilteredFiles( directory, "", filter, list, &nfiles );

		list[ nfiles ] = 0;
		*numfiles = nfiles;

		if (!nfiles)
			return NULL;

		listCopy = (char **)Z_Malloc( ( nfiles + 1 ) * sizeof( *listCopy ), TAG_LISTFILES );
		for ( i = 0 ; i < nfiles ; i++ ) {
			listCopy[i] = list[i];
		}
		listCopy[i] = NULL;

		return listCopy;
	}

	if ( !extension) {
		extension = "";
	}

	// passing a slash as extension will find directories
	if ( extension[0] == '/' && extension[1] == 0 ) {
		extension = "";
		flag = 0;
	} else {
		flag = _A_SUBDIR;
	}

	extLen = strlen( extension );

	Com_sprintf( search, sizeof(search), "%s\\*%s", directory, extension );

	// search
	nfiles = 0;

	findhandle = _findfirst (search, &findinfo);
	if (findhandle == -1) {
		*numfiles = 0;
		return NULL;
	}

	do {
		if ( (!wantsubs && flag ^ ( findinfo.attrib & _A_SUBDIR )) || (wantsubs && findinfo.attrib & _A_SUBDIR) ) {
			if (*extension) {
				if ( strlen( findinfo.name ) < extLen ||
					Q_stricmp(
						findinfo.name + strlen( findinfo.name ) - extLen,
						extension ) ) {
					continue; // didn't match
				}
			}
			if ( nfiles == MAX_FOUND_FILES - 1 ) {
				break;
			}
			list[ nfiles ] = CopyString( findinfo.name );
			nfiles++;
		}
	} while ( _findnext (findhandle, &findinfo) != -1 );

	list[ nfiles ] = 0;

	_findclose (findhandle);

	// return a copy of the list
	*numfiles = nfiles;

	if ( !nfiles ) {
		return NULL;
	}

	listCopy = (char **)Z_Malloc( ( nfiles + 1 ) * sizeof( *listCopy ), TAG_LISTFILES );
	for ( i = 0 ; i < nfiles ; i++ ) {
		listCopy[i] = list[i];
	}
	listCopy[i] = NULL;

	do {
		flag = 0;
		for(i=1; i<nfiles; i++) {
			if (strgtr(listCopy[i-1], listCopy[i])) {
				char *temp = listCopy[i];
				listCopy[i] = listCopy[i-1];
				listCopy[i-1] = temp;
				flag = 1;
			}
		}
	} while(flag);

	return listCopy;
}

void	Sys_FreeFileList( char **psList ) {
	int		i;

	if ( !psList ) {
		return;
	}

	for ( i = 0 ; psList[i] ; i++ ) {
		Z_Free( psList[i] );
	}

	Z_Free( psList );
}

/*
========================================================================

LOAD/UNLOAD DLL

========================================================================
*/
//make sure the dll can be opened by the file system, then write the
//file back out again so it can be loaded is a library. If the read
//fails then the dll is probably not in the pk3 and we are running
//a pure server -rww
UnpackDLLResult Sys_UnpackDLL(const char *name)
{
	UnpackDLLResult result = {};
	void *data;
	int len = FS_ReadFile(name, &data);

	if (len >= 1)
	{
		if (FS_FileIsInPAK(name, NULL) == 1)
		{
			char *tempFileName;
			if ( FS_WriteToTemporaryFile(data, len, &tempFileName) )
			{
				result.tempDLLPath = tempFileName;
				result.succeeded = true;
			}
		}
	}

	FS_FreeFile(data);

	return result;
}

bool Sys_DLLNeedsUnpacking()
{
	return Cvar_VariableIntegerValue("sv_pure") != 0;
}

/*
================
Sys_Diagnostics

Prints diagnostic information to the console
================
*/
void Sys_Diagnostics(const char* diagCategory)
{
	HANDLE handle = GetCurrentProcess();

	if (!Q_stricmpn(diagCategory, "mem", 3))
	{
		// memory
		PROCESS_MEMORY_COUNTERS pmc;
		MEMORYSTATUSEX mem;

		mem.dwLength = sizeof(mem);

		GlobalMemoryStatusEx(&mem);

		if (GetProcessMemoryInfo(handle, &pmc, sizeof(pmc)))
		{
			Com_Printf("--------Process--------\n");
			Com_Printf("PageFaultCount: %i\n", pmc.PageFaultCount);
			Com_Printf("PagefileUsage: %i (peak: %i)\n", pmc.PagefileUsage, pmc.PeakPagefileUsage);
			Com_Printf("WorkingSetSize: %i (peak: %i)\n", pmc.WorkingSetSize, pmc.PeakWorkingSetSize);
			Com_Printf("QuotaPagedPoolUsage: %i (peak: %i)\n", pmc.QuotaPagedPoolUsage, pmc.QuotaPeakPagedPoolUsage);
			Com_Printf("QuotaNonPagedPoolUsage: %i (peak: %i)\n", pmc.QuotaNonPagedPoolUsage, pmc.QuotaPeakNonPagedPoolUsage);
			Com_Printf("--------Machine--------\n");
			Com_Printf("Available: %llu mb / %llu mb / %llu mb\n", mem.ullAvailPhys / 1048576, 
				mem.ullAvailVirtual / 1048576, mem.ullAvailExtendedVirtual / 1048576);
			Com_Printf("(Physical / Virtual / Extended Virtual)\n");
			Com_Printf("Total: %llu mb / %llu mb\n", mem.ullTotalPhys / 1048576, mem.ullTotalVirtual / 1048576);
			Com_Printf("(Physical / Virtual)\n");
			Com_Printf("Page Available: %llu / %llu\n", mem.ullAvailPageFile / 1048576, mem.ullTotalPageFile / 1048576);
			Com_Printf("%d PERCENT MEMORY IN USE\n", mem.dwMemoryLoad);
		}
		return;
	}

	Com_Printf("not valid category, try using \"mem\" as the second argument\n");
}

/*
================
Sys_CleanModuleName

Changes modules from being a long absolute path to being a short relative one
================
*/
void Sys_CleanModuleName(char* name, size_t size)
{
	std::string moduleName = name;
	unsigned int i = moduleName.find_last_of("\\/");
	if (i == std::string::npos)
	{
		return;
	}

	moduleName = moduleName.substr(i + 1);
	Q_strncpyz(name, moduleName.c_str(), size);
}

/*
================
Sys_PrintModule
================
*/
#ifdef _WIN64
BOOL CALLBACK Sys_PrintModule(PCTSTR moduleName, DWORD64 base, void* userContext)
#else
BOOL CALLBACK Sys_PrintModule(PCTSTR moduleName, DWORD base, void* userContext)
#endif
{
	Com_Printf("* %08X | %s\n", base, moduleName);
	return TRUE;
}

/*
================
Sys_PrintStackTrace

Occurs when we encounter a fatal error.
Prints the stack trace as well as some other data.
================
*/
LONG WINAPI Sys_PrintStackTrace(EXCEPTION_POINTERS* exception)
{
	STACKFRAME frame = {};
#ifdef WIN32
	DWORD machine = IMAGE_FILE_MACHINE_I386;
#else
	DWORD machine = IMAGE_FILE_MACHINE_AMD64;
#endif
	HANDLE process = GetCurrentProcess();
	HANDLE thread = GetCurrentThread();
	int i = 0;

	frame.AddrPC.Mode = AddrModeFlat;
	frame.AddrFrame.Mode = AddrModeFlat;
	frame.AddrStack.Mode = AddrModeFlat;
#ifdef _WIN64
	frame.AddrPC.Offset = exception->ContextRecord->Rip;
	frame.AddrFrame.Offset = exception->ContextRecord->Rbp;
	frame.AddrStack.Offset = exception->ContextRecord->Rsp;
#else
	frame.AddrPC.Offset = exception->ContextRecord->Eip;
	frame.AddrFrame.Offset = exception->ContextRecord->Ebp;
	frame.AddrStack.Offset = exception->ContextRecord->Esp;
#endif

	Com_Printf("------------------------\n");
	Com_Printf("Enumerate Modules:\n");
	Com_Printf("------------------------\n");
	SymRefreshModuleList(process);
	SymEnumerateModules(process, Sys_PrintModule, nullptr);

	Com_Printf("\n\n");
	Com_Printf("------------------------\n");
	Com_Printf("Stack trace : \n");
	Com_Printf("------------------------\n");
	while (StackWalk(machine, process, thread, &frame, exception->ContextRecord, nullptr, SymFunctionTableAccess, SymGetModuleBase, nullptr))
	{
		DWORD moduleBase = SymGetModuleBase(process, frame.AddrPC.Offset);
		char moduleName[MAX_PATH];
		char funcName[MAX_PATH];
		char fileName[MAX_PATH];
		DWORD address = frame.AddrPC.Offset;
		char symbolBuffer[sizeof(IMAGEHLP_SYMBOL) + 255];
		PIMAGEHLP_SYMBOL symbol = (PIMAGEHLP_SYMBOL)symbolBuffer;
		IMAGEHLP_LINE line;
#ifdef _WIN64
DWORD64 offset = 0;
#else
		DWORD offset = 0;
#endif

		line.SizeOfStruct = sizeof(IMAGEHLP_LINE);
		symbol->SizeOfStruct = (sizeof IMAGEHLP_SYMBOL) + 255;
		symbol->MaxNameLength = 254;

		if (moduleBase && GetModuleFileNameA((HINSTANCE)moduleBase, moduleName, MAX_PATH))
		{
			Sys_CleanModuleName(moduleName, MAX_PATH);
		}
		else
		{
			moduleName[0] = '\0';
		}

		if (SymGetSymFromAddr(process, frame.AddrPC.Offset, &offset, symbol))
		{
			Q_strncpyz(funcName, symbol->Name, MAX_PATH);
		}
		else
		{
			funcName[0] = '\0';
		}

		if (SymGetLineFromAddr(process, frame.AddrPC.Offset, (PDWORD)&offset, &line))
		{
			Q_strncpyz(fileName, line.FileName, MAX_PATH);
			Sys_CleanModuleName(fileName, MAX_PATH);
			Com_sprintf(fileName, MAX_PATH, "%s:%i", fileName, line.LineNumber);
		}
		else
		{
			fileName[0] = '\0';
		}

		Com_Printf("%03i %20s 0x%08X | %s (%s)\n", i, moduleName, address, funcName, fileName);
		i++;
	}

	Sys_Error("Unhanded Exception: 0x%08X", exception->ExceptionRecord->ExceptionCode);
#ifdef _DEBUG
	return EXCEPTION_CONTINUE_SEARCH;
#else
	return EXCEPTION_EXECUTE_HANDLER;
#endif
	
}

/*
================
Sys_PlatformInit

Platform-specific initialization
================
*/
void Sys_PlatformInit(void) {
	TIMECAPS ptc;
	if (timeGetDevCaps(&ptc, sizeof(ptc)) == MMSYSERR_NOERROR)
	{
		timerResolution = ptc.wPeriodMin;

		if (timerResolution > 1)
		{
			Com_Printf("Warning: Minimum supported timer resolution is %ums "
				"on this system, recommended resolution 1ms\n", timerResolution);
		}

		timeBeginPeriod(timerResolution);
	}
	else
		timerResolution = 0;

	if (!SymInitialize(GetCurrentProcess(), NULL, TRUE))
	{
		SymSetOptions(SYMOPT_LOAD_LINES);

		bLoadedImgHelp = true;
		Com_Printf("imghelp.dll loaded for debug stack walk info\n");
	}

	stackProcess = GetCurrentProcess();
	stackThread = GetCurrentThread();
	SetUnhandledExceptionFilter(Sys_PrintStackTrace);
}

/*
================
Sys_PlatformExit

Platform-specific exit code
================
*/
void Sys_PlatformExit( void )
{
	if ( timerResolution )
		timeEndPeriod( timerResolution );

	if (bLoadedImgHelp)
	{
		SymCleanup(GetCurrentProcess());
	}
}

void Sys_Sleep( int msec )
{
	if ( msec == 0 )
		return;

#ifdef DEDICATED
	if ( msec < 0 )
		WaitForSingleObject( GetStdHandle( STD_INPUT_HANDLE ), INFINITE );
	else
		WaitForSingleObject( GetStdHandle( STD_INPUT_HANDLE ), msec );
#else
	// Client Sys_Sleep doesn't support waiting on stdin
	if ( msec < 0 )
		return;

	Sleep( msec );
#endif
}
