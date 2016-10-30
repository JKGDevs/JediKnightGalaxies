#include "ui_local.h"
#include "ui_shared.h"
#include <json/cJSON.h>
#include <unordered_map>
#include <string>

using namespace std;

static char stylesheet[MAX_QPATH];
static unordered_map<string, itemDef_t> styleobjects;
extern displayContextDef_t *DC;

void JKG_UI_LoadStylesheet( const char *text )
{
	Q_strncpyz( stylesheet, text, sizeof(stylesheet) );
	styleobjects.clear();
	
	// Parse the actual stylesheet
	fileHandle_t f;
	char filebuffer[32768];
	int filelen = trap->FS_Open( va("ui/styles/%s.iss",stylesheet), &f, FS_READ );

	if ( filelen == -1 || !f )
    {
        Com_Printf (S_COLOR_RED "Couldn't find stylesheet \"%s\"\n", stylesheet);
        return;
    }
    
    if ( filelen == 0 )
    {
        Com_Printf (S_COLOR_RED "Error: stylesheet \"%s\" is empty.\n", stylesheet);
        trap->FS_Close (f);
        return;
    }
    
    if ( filelen > sizeof(filebuffer)-1 )
    {
        Com_Printf (S_COLOR_RED "Error: really big stylesheet \"%s\"\n", stylesheet);
        trap->FS_Close (f);
        return;
    }

	trap->FS_Read( filebuffer, filelen, f );
	filebuffer[filelen] = '\0';
	trap->FS_Close( f );

	// Parse the actual stylesheet now.
	char jsonError[MAX_STRING_CHARS] = {0};
	cJSON *json = cJSON_ParsePooled( filebuffer, jsonError, sizeof(jsonError) );
	if( json == NULL )
	{
		Com_Printf(S_COLOR_RED "Error in stylesheet \"%s\": %s\n", stylesheet, jsonError);
		return;
	}

	if( json )
	{
		for( cJSON *classObject = cJSON_GetFirstItem( json );
			classObject; classObject = cJSON_GetNextItem( json ) )
		{
			itemDef_t item;
			const char *textTemp;
			string name = cJSON_GetItemKey( classObject );

			// fix some junk first off
			item.typeData.data = NULL;

			// parse the different blocks of the stylesheet
			item.accept = cJSON_ToString(cJSON_GetObjectItem(classObject, "accept"));
			item.action = cJSON_ToString(cJSON_GetObjectItem(classObject, "action"));
			item.alignment = cJSON_ToInteger(cJSON_GetObjectItem(classObject, "alignment")); // FIXME: use enum
			item.appearanceSlot = cJSON_ToInteger(cJSON_GetObjectItem(classObject, "appearanceSlot"));
			//item.asset // handled elsewhere
			//item.colorRanges // not used
			//item.cursorPos // runtime variable
			item.cvar = cJSON_ToString(cJSON_GetObjectItem(classObject, "cvar"));
			item.cvarFlags = cJSON_ToInteger(cJSON_GetObjectItem(classObject, "cvarFlags")); // FIXME: use proper flagging
			item.cvarTest = cJSON_ToString(cJSON_GetObjectItem(classObject, "cvarTest"));
			item.descText = cJSON_ToString(cJSON_GetObjectItem(classObject, "descText"));
			item.disabled = cJSON_ToBoolean(cJSON_GetObjectItem(classObject, "disabled"));
			item.enableCvar = cJSON_ToString(cJSON_GetObjectItem(classObject, "enableCvar"));
			item.flags = cJSON_ToInteger(cJSON_GetObjectItem(classObject, "flags")); // FIXME: use proper flagging
			item.focusSound = DC->registerSound(cJSON_ToString(cJSON_GetObjectItem(classObject, "focusSound")));
			item.iMenuFont = cJSON_ToInteger(cJSON_GetObjectItem(classObject, "font")); // FIXME: use enum
			item.iMenuFont2 = cJSON_ToInteger(cJSON_GetObjectItem(classObject, "font2")); // FIXME: use enum
			item.invertYesNo = cJSON_ToInteger(cJSON_GetObjectItem(classObject, "invertYesNo"));
			item.leaveFocus = cJSON_ToString(cJSON_GetObjectItem(classObject, "leaveFocus"));
			item.mouseEnter = cJSON_ToString(cJSON_GetObjectItem(classObject, "mouseEnter"));
			item.mouseEnterText = cJSON_ToString(cJSON_GetObjectItem(classObject, "mouseEnterText"));
			item.mouseExit = cJSON_ToString(cJSON_GetObjectItem(classObject, "mouseExit"));
			item.mouseExitText = cJSON_ToString(cJSON_GetObjectItem(classObject, "mouseExitText"));
			//item.numColors // not used
			item.onEscape = cJSON_ToString(cJSON_GetObjectItem(classObject, "onEscape"));
			item.onFocus = cJSON_ToString(cJSON_GetObjectItem(classObject, "onFocus"));
			item.onReturn = cJSON_ToString(cJSON_GetObjectItem(classObject, "onReturn"));
			item.selectionNext = cJSON_ToString(cJSON_GetObjectItem(classObject, "selectionNext"));
			item.selectionPrev = cJSON_ToString(cJSON_GetObjectItem(classObject, "selectionPrev"));
			item.special = cJSON_ToNumber(cJSON_GetObjectItem(classObject, "special"));
			if ((textTemp = cJSON_ToString(cJSON_GetObjectItem(classObject, "text")))!=nullptr) Q_strncpyz(item.text, textTemp, sizeof(item.text));
			if ((textTemp = cJSON_ToString(cJSON_GetObjectItem(classObject, "text2")))!=nullptr) Q_strncpyz(item.text2, textTemp, sizeof(item.text2));
			item.text2alignment = cJSON_ToInteger(cJSON_GetObjectItem(classObject, "text2alignment")); // FIXME: use enum
			item.text2alignx = cJSON_ToNumber(cJSON_GetObjectItem(classObject, "text2alignx")); // FIXME: allow for percentages
			item.text2aligny = cJSON_ToNumber(cJSON_GetObjectItem(classObject, "text2aligny")); // FIXME: allow for percentages
			item.textalignment = cJSON_ToInteger(cJSON_GetObjectItem(classObject, "textalignment")); // FIXME: use enum
			item.textalignx = cJSON_ToNumber(cJSON_GetObjectItem(classObject, "textalignx")); // FIXME: allow for percentages
			item.textaligny = cJSON_ToNumber(cJSON_GetObjectItem(classObject, "textaligny")); // FIXME: allow for percentages
			//item.textRect // will take a while to write parsing code for this. putting it off for now.
			item.textscale = cJSON_ToNumber(cJSON_GetObjectItem(classObject, "textscale"));
			item.textscale2 = cJSON_ToNumber(cJSON_GetObjectItem(classObject, "textscale2"));
			item.textStyle = cJSON_ToInteger(cJSON_GetObjectItem(classObject, "textStyle")); // FIXME: use proper flags
			item.textStyle2 = cJSON_ToInteger(cJSON_GetObjectItem(classObject, "textStyle2")); // FIXME: use proper flags
			item.type = cJSON_ToInteger(cJSON_GetObjectItem(classObject, "type")); // FIXME: use enum
			//item.window.backColor // need vec parsing
			item.window.background = DC->registerShaderNoMip(cJSON_ToStringOpt(cJSON_GetObjectItem(classObject, "background"), ""));
			item.window.border = cJSON_ToInteger(cJSON_GetObjectItem(classObject, "border"));
			//item.window.borderColor // need vec parsing
			item.window.borderSize = cJSON_ToNumber(cJSON_GetObjectItem(classObject, "borderSize"));
			//item.window.cinematic // not needed
			//item.window.cinematicName // not needed
			item.window.flags = cJSON_ToInteger(cJSON_GetObjectItem(classObject, "windowFlags")); // FIXME: use proper flags
			//item.window.foreColor // need vec parsing
			item.window.foreground = DC->registerShaderNoMip(cJSON_ToStringOpt(cJSON_GetObjectItem(classObject, "foreground"), ""));
			item.window.group = cJSON_ToString(cJSON_GetObjectItem(classObject, "group"));
			item.window.name = cJSON_ToString(cJSON_GetObjectItem(classObject, "name"));
			//item.window.outlineColor // same as borderColor??
			item.window.ownerDraw = cJSON_ToInteger(cJSON_GetObjectItem(classObject, "ownerDraw")); // FIXME: use enum
			item.window.ownerDrawFlags = cJSON_ToNumber(cJSON_GetObjectItem(classObject, "ownerDrawFlags"));
			item.window.ownerDrawID = cJSON_ToInteger(cJSON_GetObjectItem(classObject, "ownerDrawID"));
			//item.window.rect // need parsing
			//item.window.rectClient // need parsing
			//item.window.rectEffects // need parsing
			//item.window.rectEffects2 // need parsing
			//item.window.selectionZone // need parsing
			item.window.style = cJSON_ToInteger(cJSON_GetObjectItem(classObject, "style")); // FIXME: use proper flags

			styleobjects.insert( make_pair (name, item) );
		}
	}
}

void JKG_UI_SetClass( const char *className, itemDef_t *item )
{
	auto it = styleobjects.find(className);
	if( it == styleobjects.end() )
	{
		Com_Printf(S_COLOR_RED "ERROR: class \"%s\" does not exist in stylesheet \"%s\"\n", 
			className, stylesheet );
		return;
	}

	memcpy(item, &it->second, sizeof(itemDef_t));
}