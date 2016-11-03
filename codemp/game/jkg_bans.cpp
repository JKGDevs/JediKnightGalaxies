/************************************************
|*
|* JKG Ban system
|*
|* This module manages IP bans
|*
|* There is support for subnet bans (using * as wildcard)
|*  and temporary bans (with reason)
|*
|* The bans are stored in a file specified by g_banfile (bans.dat by default)
|* in json format
|*
|* Structure sample:
|*

{
	"nextid":2,
	"bans":[
		{
			"id":1
			"ip":[127,0,0,1],
			"mask":15,
			"expire":0,
			"reason":"",
		},
	]
}

\*/

#include "g_local.h"
#include <json/cJSON.h>

typedef struct banentry_s {
	unsigned int	id;
	unsigned char	ip[4];
	unsigned char	mask;
	unsigned int	expireTime;
	char			banreason[64];
	struct banentry_s *next;
} banentry_t;

static banentry_t *bans = 0;
static unsigned int nextBanId = 1;

extern vmCvar_t g_banfile;

void JKG_Bans_Clear()
{
	banentry_t *entry, *next = 0;
	for(entry = bans; entry; entry = next)
	{
		next = entry->next;
		free(entry);
	}
	bans = 0;
	nextBanId = 1;
}

void JKG_Bans_LoadBans()
{
	char *buffer;
	char error[256];
	fileHandle_t f;
	int len;
	int i, banCount;
	cJSON *root, *banlist, *item, *ip;
	banentry_t *entry;

	if (!g_banfile.string[0]) {
		return;
	}

	len = trap->FS_Open(g_banfile.string, &f, FS_READ);
	if (len < 1) {
		return;
	}
	buffer = ( char * )malloc(len+1);
	trap->FS_Read(buffer, len, f);
	trap->FS_Close(f);

	buffer[len] = 0;

	root = cJSON_ParsePooled(buffer, error, sizeof(error));
	free(buffer);

	if (!root) {
		trap->Print("Error: Could not parse banlist: %s\n", error);
		return;
	}
	
	JKG_Bans_Clear();

	nextBanId = cJSON_ToInteger(cJSON_GetObjectItem(root, "nextid"));
	banlist = cJSON_GetObjectItem(root, "bans");

	banCount = cJSON_GetArraySize(banlist);
	for (i=0; i<banCount; i++) {
		item = cJSON_GetArrayItem(banlist, i);
		ip = cJSON_GetObjectItem(item, "ip");

		entry = ( banentry_t * )malloc(sizeof(banentry_t));
		
		entry->id = cJSON_ToInteger(cJSON_GetObjectItem(item, "id"));
		entry->mask = cJSON_ToInteger(cJSON_GetObjectItem(item, "mask"));
		entry->expireTime = cJSON_ToInteger(cJSON_GetObjectItem(item, "expire"));
		Q_strncpyz(entry->banreason, cJSON_ToString(cJSON_GetObjectItem(item, "reason")), sizeof(entry->banreason));
		entry->ip[0] = cJSON_ToInteger(cJSON_GetArrayItem(ip, 0));
		entry->ip[1] = cJSON_ToInteger(cJSON_GetArrayItem(ip, 1));
		entry->ip[2] = cJSON_ToInteger(cJSON_GetArrayItem(ip, 2));
		entry->ip[3] = cJSON_ToInteger(cJSON_GetArrayItem(ip, 3));

		entry->next = bans;
		bans = entry;
	}
	cJSON_Delete(root);
}

void JKG_Bans_Init()
{
	
	JKG_Bans_LoadBans();
}


void JKG_Bans_SaveBans()
{
	cJSONStream *stream;
	const char *buffer;
	fileHandle_t f;
	banentry_t *entry;
	unsigned int curr = time(NULL);
	
	stream = cJSON_Stream_New(8, 1, 0, 0);
	cJSON_Stream_BeginObject(stream, NULL); // Root object
	cJSON_Stream_WriteInteger(stream, "nextid", nextBanId);
	cJSON_Stream_BeginArray(stream, "bans"); // Start bans array
	for(entry = bans; entry; entry = entry->next) {
		if (entry->expireTime && curr >= entry->expireTime) {
			continue;	// Don't save expired bans
		}
		cJSON_Stream_BeginObject(stream, NULL); // Ban entry
		cJSON_Stream_WriteInteger(stream, "id", entry->id );
		cJSON_Stream_WriteInteger(stream, "mask", entry->mask );
		cJSON_Stream_WriteInteger(stream, "expire", entry->expireTime);
		cJSON_Stream_WriteString(stream, "reason", entry->banreason);
		
		cJSON_Stream_BeginArray(stream, "ip"); // IP
		cJSON_Stream_WriteInteger(stream, NULL, entry->ip[0]);
		cJSON_Stream_WriteInteger(stream, NULL, entry->ip[1]);
		cJSON_Stream_WriteInteger(stream, NULL, entry->ip[2]);
		cJSON_Stream_WriteInteger(stream, NULL, entry->ip[3]);
		cJSON_Stream_EndBlock(stream);

		cJSON_Stream_EndBlock(stream); // Ban entry
	}
	cJSON_Stream_EndBlock(stream);	// Bans array
	cJSON_Stream_EndBlock(stream);	// Root object
	buffer = cJSON_Stream_Finalize(stream);

	trap->FS_Open(g_banfile.string, &f, FS_WRITE);
	trap->FS_Write(buffer, strlen(buffer), f);
	trap->FS_Close(f);

	free((void *)buffer);
}

/* Adds a ban to the banlist 
|* Duration format:
|*
|* <count><specifier> (eg. '12h' for a 12 hour ban)
|*
|* Specifiers:
|*
|* m: minutes
|* h: hours
|* d: days
|* n: months (30 days)
|* y: years  (365 days)
|*
|* Specify a NULL duration or a duration of '0' to make the ban permanent
|*
\*/
int JKG_Bans_AddBan(netadr_t adr, const char *duration, const char *reason)
{
	unsigned int expire;
	char type;

	banentry_t *entry;

	if (adr.type != NA_IP) {
		return -1;
	}

	// Parse expire date
	if (!duration || *duration == '0') {
		expire = 0;
	} else {
		if (sscanf(duration, "%u%c", &expire, &type) != 2) {
			// Could not interpet the data, so we'll put in 12 hours
			expire = 12;
			type = 'h';
		}
		switch (type) {
			case 'm':
				expire *= 60;
				break;
			default:	// Assume hours by default

			case 'h':
				expire *= 3600;
				break;
			case 'd':
				expire *= 86400;
				break;
			case 'n':
				expire *= 2592000;
				break;
			case 'y':
				expire *= 31536000;
				break;
		}
		expire += time(NULL);
	}

	// Check if this ban already exists
	for (entry = bans; entry; entry = entry->next) {
		if (entry->mask == 15 && *(unsigned int*)&entry->ip == *(unsigned int*)&adr.ip) {
			entry->expireTime = expire;
			if (reason) {
				Q_strncpyz(entry->banreason, reason, sizeof(entry->banreason));
			}
			return entry->id;
		}
	}

	entry = ( banentry_t * )malloc(sizeof(banentry_t));
	entry->id = nextBanId++;
	*(unsigned int*)&entry->ip = *(unsigned int*)&adr.ip;
	entry->expireTime = expire;
	entry->mask = 15;
	if (reason) {
		Q_strncpyz(entry->banreason, reason, sizeof(entry->banreason));
	} else {
		entry->banreason[0] = 0;
	}
	entry->next = bans;
	bans = entry;
	return entry->id;
}

/* Same as above, but adds bans by string */
/* A '*' can be used as a wildcard to make range bans (ie 150.10.*.*) */

qboolean JKG_Bans_AddBanString( const char *ip, const char *duration, const char *reason)
{
	unsigned char m[4];
	unsigned char mask;
	int i = 0;
	int c;
	const char *p;
	unsigned int expire;
	char type;

	banentry_t *entry;

	mask = 15;	

	while (i < 4)
	{
		m[i] = 0;
		i++;
	}

	i = 0;
	p =  ip;
	while (*p && i < 4) {
		c = 0;
		if (*p == '*') {
			mask &= ~(1 << i);
			c++;
			p++;
		} else {
			while (*p >= '0' && *p <= '9') {
				m[i] = m[i]*10 + (*p - '0');
				c++;
				p++;
			}
		}
		if (!c) {			// Check if we parsed any characters
			return -1;	// Faulty IP
		}
		if (!*p || *p == ':')	// Check if we've reached the end of the IP
			break;
		if (*p != '.') {	// The next character MUST be a period
			return -1;	// Faulty IP
		}
		i++;
		p++;
	}
	if (i < 3) {		// If i < 3, the parser ended prematurely, so abort
		return -1;
	}
	
	// Parse expire date
	if (!duration || *duration == '0') {
		expire = 0;
	} else {
		if (sscanf(duration, "%u%c", &expire, &type) != 2) {
			// Could not interpet the data, so we'll put in 12 hours
			expire = 12;
			type = 'h';
		}
		switch (type) {
			case 'm':
				expire *= 60;
				break;
			default:	// Assume hours by default

			case 'h':
				expire *= 3600;
				break;
			case 'd':
				expire *= 86400;
				break;
			case 'n':
				expire *= 2592000;
				break;
			case 'y':
				expire *= 31536000;
				break;
		}
		expire += time(NULL);
	}

	// Check if this ban already exists
	for (entry = bans; entry; entry = entry->next) {
		if (entry->mask == mask && *(unsigned int*)&entry->ip == *(unsigned int*)&m) {
			entry->expireTime = expire;
			if (reason) {
				Q_strncpyz(entry->banreason, reason, sizeof(entry->banreason));
			}
			return entry->id;
		}
	}

	entry = ( banentry_t * )malloc(sizeof(banentry_t));
	entry->id = nextBanId++;
	*(unsigned int*)&entry->ip = *(unsigned int*)&m;
	entry->expireTime = expire;
	entry->mask = mask;
	if (reason) {
		Q_strncpyz(entry->banreason, reason, sizeof(entry->banreason));
	} else {
		entry->banreason[0] = 0;
	}
	entry->next = bans;
	bans = entry;
	return entry->id;
}

// Returns the ban message for the IP
// The the IP is not banned, NULL will be returned

static const char *GetRemainingTime(unsigned int expireTime)
{
	unsigned int diff;
	unsigned int curr = time(NULL);

	unsigned int days;
	unsigned int hours;
	unsigned int minutes;
	unsigned int seconds;

	if (curr >= expireTime) {
		return "Ban expired";
	}
	
	diff = expireTime - curr;

	days = diff / 86400;
	diff -= (days * 86400);

	hours = diff / 3600;
	diff -= (hours * 3600);

	minutes = diff / 60;
	diff -= (minutes * 60);

	seconds = diff;

	if (days != 0) {
		return va("%i day%s - %02i:%02i:%02i", days, days == 1 ? "" : "s", hours, minutes, seconds);
	} else {
		return va("%02i:%02i:%02i", hours, minutes, seconds);
	}
}

const char *JKG_Bans_IsBanned(netadr_t adr)
{
	banentry_t *entry, *prev, *next;
	

	if (adr.type != NA_IP) {
		return NULL;
	}

	for (entry = bans, prev = 0; entry; prev = entry, entry = next) {
		next = entry->next;
		// Find the ban entry
		if (entry->mask & 1 && entry->ip[0] != adr.ip[0]) continue;
		if (entry->mask & 2 && entry->ip[1] != adr.ip[1]) continue;
		if (entry->mask & 4 && entry->ip[2] != adr.ip[2]) continue;
		if (entry->mask & 8 && entry->ip[3] != adr.ip[3]) continue;
		// If we get here, we got a match
		
		// Check if it's temporary or permanent
		if (entry->expireTime) {
			// See if this ban has expired
			if (time(NULL) >= entry->expireTime) {
				// The ban expired, unlink this ban and then continue the search
				if (!prev) {
					// This is the root item
					bans = entry->next;
					prev = 0;
				} else {
					prev->next = entry->next;
					prev = entry;
				}
				free((void *)entry);
				continue;
			}

			// Temporary ban, calculate the remaining time
			return va("You have been temporarily banned from this server\nTime remaining: %s\nReason: %s\n", GetRemainingTime(entry->expireTime), entry->banreason[0] ? entry->banreason : "Not specified");
		} else {
			// Permaban
			return va("You have been permanently banned from this server\nReason: %s\n", entry->banreason[0] ? entry->banreason : "Not specified");
		}
	}
	return NULL;
}

qboolean JKG_Bans_RemoveBan(unsigned int id)
{

	banentry_t *entry, *prev;

	for (entry = bans, prev = 0; entry; prev = entry, entry = entry->next) {
		// Find the ban entry
		if (entry->id != id) continue;
		
		if (!prev) {
			// This is the root item
			bans = entry->next;
		} else {
			prev->next = entry->next;
		}
		free((void *)entry);
		entry = prev;
		return qtrue;
	}
	return qfalse;
}

static const char *JKG_Bans_IPToString(banentry_t *ban)
{
	static char buffer[32];
	int i;

	buffer[0] = 0;
	for (i=0; i<4 ; i++) {
		if (ban->mask & (1 << i)) {
			Q_strcat(buffer, sizeof(buffer), va("%u", ban->ip[i]));
		} else {
			Q_strcat(buffer, sizeof(buffer), "*");
		}
		if (i != 3) Q_strcat(buffer, sizeof(buffer), ".");
	}
	return buffer;
}

/* List matching IP's, Rcon only! */
/* Wildcards are supported */
void JKG_Bans_ListBans(const char *ip)
{

	banentry_t *entry, *prev, *next;

	unsigned char m[4];
	unsigned char mask;
	int i = 0;
	int c;
	const char *p;

	mask = 15;	

	while (i < 4)
	{
		m[i] = 0;
		i++;
	}

	i = 0;
	p =  ip;
	while (*p && i < 4) {
		c = 0;
		if (*p == '*') {
			mask &= ~(1 << i);
			c++;
			p++;
		} else {
			while (*p >= '0' && *p <= '9') {
				m[i] = m[i]*10 + (*p - '0');
				c++;
				p++;
			}
		}
		if (!c) {			// Check if we parsed any characters
			trap->Print("Invalid IP specified\n");
			return;	// Faulty IP
		}
		if (!*p || *p == ':')	// Check if we've reached the end of the IP
			break;
		if (*p != '.') {	// The next character MUST be a period
			trap->Print("Invalid IP specified\n");
			return;	// Faulty IP
		}
		i++;
		p++;
	}
	if (i < 3) {		// If i < 3, the parser ended prematurely, so abort
		trap->Print("Invalid IP specified\n");
		return;
	}

	for (entry = bans, prev = 0; entry; entry = next) {
		next = entry->next;
		// Find the ban entry
		if (mask & 1 && entry->mask & 1 && entry->ip[0] != m[0]) continue;
		if (mask & 2 && entry->mask & 2 && entry->ip[1] != m[1]) continue;
		if (mask & 4 && entry->mask & 4 && entry->ip[2] != m[2]) continue;
		if (mask & 8 && entry->mask & 8 && entry->ip[3] != m[3]) continue;
		// If we get here, we got a match
		

		// Check if it's temporary or permanent
		if (entry->expireTime) {
			// See if this ban has expired
			if (time(NULL) >= entry->expireTime) {
				// The ban expired, unlink this ban and then continue the search
				if (!prev) {
					// This is the root item
					bans = entry->next;
					prev = 0;
				} else {
					prev->next = entry->next;
					prev = entry;
				}
				free((void *)entry);
				continue;
			}

			// Temporary ban, calculate the remaining time
			trap->Print("%i - %s - %s - %s\n", entry->id, JKG_Bans_IPToString(entry), GetRemainingTime(entry->expireTime), entry->banreason[0] ? entry->banreason : "Reason not specified" );
		} else {
			// Permaban
			trap->Print("%i - %s - Permanent - %s\n", entry->id, JKG_Bans_IPToString(entry), entry->banreason[0] ? entry->banreason : "Reason not specified" );
		}
		prev = entry;
	}
	return;
}

qboolean JKG_Bans_GetBanInfo(unsigned int id, char *ip, size_t ipsize, char *duration, size_t durationsize, char *reason, size_t reasonsize)
{

	banentry_t *entry;

	for (entry = bans; entry; entry = entry->next) {
		// Find the ban entry
		if (entry->id != id) continue;
		
		if (ip) {
			Q_strncpyz(ip, JKG_Bans_IPToString(entry), ipsize);
		}
		if (duration) {
			Q_strncpyz(duration, GetRemainingTime(entry->expireTime), durationsize);
		}
		if (reason) {
			Q_strncpyz(reason, entry->banreason, reasonsize);
		}
		return qtrue;
	}
	return qfalse;
}
