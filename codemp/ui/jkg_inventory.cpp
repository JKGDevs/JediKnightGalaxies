#include "ui_local.h"
#include "jkg_inventory.h"

static size_t nNumInventoryItems = 0;			// number of items in the list
static std::vector<std::pair<int, itemInstance_t*>> pItems;	// list of items
static int nPosition = 0;					// position in the item list (changed with arrow buttons)
static int nSelected = -1;					// selected item in the list (-1 for no selection)


static void JKG_ConstructInventoryList() {
	itemInstance_t* pAllItems = nullptr;

	pItems.clear();

	if (cgImports == nullptr) {
		// This gets called when the game starts (because ui_inventoryFilter was modified), so just return
		return;
	}

	nNumInventoryItems = *(size_t*)cgImports->InventoryDataRequest(0);
	if (nNumInventoryItems > 0) {
		pAllItems = (itemInstance_t*)cgImports->InventoryDataRequest(1);
		for (int i = 0; i < nNumInventoryItems; i++) {
			itemInstance_t* pThisItem = &pAllItems[i];
			if (ui_inventoryFilter.integer == JKGIFILTER_ARMOR && pThisItem->id->itemType != ITEM_ARMOR && pThisItem->id->itemType != ITEM_CLOTHING) {
				continue;
			}
			else if (ui_inventoryFilter.integer == JKGIFILTER_WEAPONS && pThisItem->id->itemType != ITEM_WEAPON) {
				continue;
			}
			else if (ui_inventoryFilter.integer == JKGIFILTER_CONSUMABLES && pThisItem->id->itemType != ITEM_CONSUMABLE) {
				continue;
			}
			else if (ui_inventoryFilter.integer == JKGIFILTER_MISC) {
				continue; // FIXME
			}
			pItems.push_back(std::make_pair(i, pThisItem));
		}
	}

	// Clear the selected item, if it's invalid
	if (nSelected >= pItems.size()) {
		nSelected = -1;
		Menu_ShowItemByName(Menus_FindByName("jkg_inventory"), "shop_preview", qfalse);
	}
}

/*
==========================
DESCRIPTION CONSTRUCTION
==========================
*/
// Flags
static int nLineOffset = 0;
static int bfTagFields = 0;
static int bfFireModeTags[MAX_FIREMODES] = { 0 };
typedef enum {
	/* Weapons */
	IDTAG_ROLLING = 0,
	IDTAG_NOSPRINT = 1,
	IDTAG_FIREMODE = 2,
	IDTAG_LAST_FIREMODE = 18,
	/* Weapon firing modes */
	IDMTAG_BOUNCING = 0,		// Mode has bouncing shots
	IDMTAG_BLEED = 1,			// Mode can cause bleeding
	IDMTAG_DISINTEGRATE = 2,	// Mode disintegrates targets
	IDMTAG_FIRECONTROL = 3,		// Full auto/burst/semi auto
	/* Armor */
	/* Consumables */
	/* Other */
	IDTAG_MAX = 32,
} uiItemDescTags;

// Item description types
typedef enum {
	IDTYPE_NODRAW = 0,			// Doesn't use firing mode schema (melee, lightsaber, etc)
	IDTYPE_GRENADE,				// Grenades (only WP_THERMAL)
	IDTYPE_CHARGEMINE,			// Charges and mines (only WP_TRIP_MINES and WP_DETPACK)
	IDTYPE_EXPLOSIVEGUN,		// Explosive projectile weapon (has special flag checked?)
	IDTYPE_PROJECTILE,			// Any other projectile weapon (blasters, slugthrowers, etc)
} uiItemDescTypes;

// Returns the proper tag that should appear with Blast Damage
char* JKG_GetBlastDamageTag(weaponData_t* pData, const int nFiringMode) {
	if (nFiringMode > pData->numFiringModes || nFiringMode < 0 || nFiringMode > MAX_FIREMODES) {
		return "";
	}
	weaponFireModeStats_t* pFireMode = &pData->firemodes[nFiringMode];
	if (pFireMode->secondaryDmgHandle == NULL_HANDLE) {
		return ""; // We can't have Blast Damage if there's no accompanying Fallout Damage. Use regular Damage instead.
	}
	return ""; // Damage stuff isn't loaded yet :<
}

// Returns the proper tag that should appear with Fallout Damage
char* JKG_GetFalloutDamageTag(weaponData_t* pData, const int nFiringMode) {
	if (nFiringMode > pData->numFiringModes || nFiringMode < 0 || nFiringMode > MAX_FIREMODES) {
		return "";
	}
	weaponFireModeStats_t* pFireMode = &pData->firemodes[nFiringMode];
	if (pFireMode->secondaryDmgHandle == NULL_HANDLE) {
		return ""; // Does not exist
	}
	return ""; // Damage stuff isn't loaded yet :<
}

// Returns the proper tag that should appear (with either "Damage: " or "Direct Damage: ")
char* JKG_GetDamageTag(weaponData_t* pData, const int nFiringMode) {
	if (nFiringMode > pData->numFiringModes || nFiringMode < 0 || nFiringMode > MAX_FIREMODES) {
		return "";
	}
	weaponFireModeStats_t* pFireMode = &pData->firemodes[nFiringMode];
	if (pData->weaponBaseIndex == WP_THERMAL || pData->weaponBaseIndex == WP_TRIP_MINE || pData->weaponBaseIndex == WP_DET_PACK ||
		pData->visuals.visualFireModes[nFiringMode].displayExplosive) {
		// Most likely referring to blast damage
		return JKG_GetBlastDamageTag(pData, nFiringMode);
	}
	return ""; // Damage stuff isn't loaded yet :<
}

// Returns the type of a firing mode
uiItemDescTypes JKG_GetFiringModeType(weaponData_t* pData, const int nFiringMode) {
	if (nFiringMode > pData->numFiringModes || nFiringMode < 0 || nFiringMode > MAX_FIREMODES) {
		return IDTYPE_NODRAW;
	}
	switch (pData->weaponBaseIndex) {
		case WP_THERMAL:
			return IDTYPE_GRENADE;
		case WP_TRIP_MINE:
		case WP_DET_PACK:
			return IDTYPE_CHARGEMINE;
		case WP_MELEE:
		case WP_EMPLACED_GUN:
		case WP_SABER:
		case WP_NONE:
			return IDTYPE_NODRAW;
		default:
			{
				if (pData->visuals.visualFireModes[nFiringMode].displayExplosive) {
					return IDTYPE_EXPLOSIVEGUN;
				} else {
					return IDTYPE_PROJECTILE;
				}
			}
			break;
	}
}

// Returns the recoil string (Light, Medium, Heavy, Intense)
const char* JKG_GetRecoilString(weaponData_t* pData, const int nFiringMode) {
	if (nFiringMode > pData->numFiringModes || nFiringMode < 0 || nFiringMode > MAX_FIREMODES) {
		return "";
	}
	weaponFireModeStats_t* pFireMode = &pData->firemodes[nFiringMode];
	if (pFireMode->recoil <= 0.9f) {
		return UI_GetStringEdString2("@JKG_INVENTORY_WEP_RECOIL_LIGHT");
	} else if (pFireMode->recoil <= 1.5f) {
		return UI_GetStringEdString2("@JKG_INVENTORY_WEP_RECOIL_MEDIUM");
	} else if (pFireMode->recoil <= 3.0f) {
		return UI_GetStringEdString2("@JKG_INVENTORY_WEP_RECOIL_HEAVY");
	} else {
		return UI_GetStringEdString2("@JKG_INVENTORY_WEP_RECOIL_INTENSE");
	}
}

// Returns the string that should appear on nLineNum of an item description
#define MAX_RECURSION_LEVEL	5
char* JKG_GetItemDescLine(itemInstance_t* pItem, int nLineNum, int recursionLevel) {
	/* Weapon variables */
	bool bDontCareAboutAccuracy = false;
	int nFiringMode = -1;

	/* Misc */
	int nAccuracyBase;
	int nAccuracyMax;
	int nCookTime;
	int nRPM;
	int nFireTime;
	weaponData_t* pWeaponData;
	uiItemDescTypes idType;
	weaponFireModeStats_t *pFireMode;
	weaponVisualFireMode_t *pVFireMode;

	if (recursionLevel >= MAX_RECURSION_LEVEL) {
		return ""; // base case to stop stack overflow
	}

	assert(pItem != nullptr);

	// If we're drawing the first line, subsequent line draws are therefore guaranteed.
	// We should clear all of the flags that we need to ensure a correct display.
	if (nLineNum == 0) {
		nLineOffset = 0;
		bfTagFields = 0;
		memset(bfFireModeTags, 0, sizeof(bfFireModeTags));
	} else {
		nLineNum += nLineOffset;
	}

	switch (pItem->id->itemType) {
		case ITEM_WEAPON:
			if (nLineNum == 0) {
				return (char*)UI_GetStringEdString2("@JKG_INVENTORY_ITYPE_WEAPON");
			}
			pWeaponData = (weaponData_t*)cgImports->GetWeaponDatas(pItem->id->weaponData.weapon, pItem->id->weaponData.variation);
			
			// Reload time / cook time
			if (nLineNum == 1) {
				switch (pItem->id->weaponData.weapon) {
					case WP_THERMAL:
						// Cook time
						if (pWeaponData->hasCookAbility) {
							return va(UI_GetStringEdString2("@JKG_INVENTORY_WEP_COOKTIME"), pWeaponData->weaponReloadTime);
						} else {
							nLineNum++;
							nLineOffset++;
						}
						break;
					case WP_SABER:
					case WP_MELEE:
					case WP_TRIP_MINE:
					case WP_DET_PACK:
						// Get offset / don't display anything for this line
						nLineNum++;
						nLineOffset++;
						break;
					default:
						// Reload time
						if (pWeaponData->weaponReloadTime > 1000) {
							return va(UI_GetStringEdString2("@JKG_INVENTORY_WEP_RELOADTIME_SEC"),
								(float)(pWeaponData->weaponReloadTime / 1000.0f));
						} else {
							return va(UI_GetStringEdString2("@JKG_INVENTORY_WEP_RELOADTIME"), pWeaponData->weaponReloadTime);
						}
					}
			}

			// Movement penalty/bonus, if there is one.
			if (nLineNum == 2) {
				const float fSpeedModifier = pWeaponData->speedModifier;
				if (fSpeedModifier > 1.0f) {
					// Movement speed bonus
					float fMovementBonus = fSpeedModifier - 1.0f;
					fMovementBonus *= 100.0f;
					return va(UI_GetStringEdString2("@JKG_INVENTORY_WEP_SPEEDBONUS"), fMovementBonus);
				} else if(fSpeedModifier < 1.0f) {
					// Movement speed penalty
					float fMovementPenalty = 1.0f - fSpeedModifier;
					fMovementPenalty *= 100.0f;
					return va(UI_GetStringEdString2("@JKG_INVENTORY_WEP_SPEEDPENALTY"), fMovementPenalty);
				}
				else {
					// Not affected
					nLineNum++;
					nLineOffset++;
				}
			}

			if (nLineNum == 3) {
				// Additional weapon tags
				if (pWeaponData->hasRollAbility && !(bfTagFields & (1 << IDTAG_ROLLING))) {
					bfTagFields |= (1 << IDTAG_ROLLING); // Flag it so we don't draw it again
					return (char*)UI_GetStringEdString2("@JKG_INVENTORY_WEP_TAG_ROLLING");
				}
			}

			if (pItem->id->weaponData.weapon == WP_SABER) {
				// TODO
				goto blankLine;
			}

			if (pItem->id->weaponData.weapon == WP_THERMAL || pItem->id->weaponData.weapon == WP_TRIP_MINE || pItem->id->weaponData.weapon == WP_DET_PACK) {
				// Don't care about accuracy rating for these weapons.
				// TODO: make this a flag in the .itm file
				bDontCareAboutAccuracy = true;
			}

			// For mines and charges:
			// nLineNum 4 is blank
			// nLineNum 5 is fire mode number
			// nLineNum 6 is blast damage
			// nLineNum 7 is fallout damage
			// nLineNum 8 is additional tags
			// For charges, the second fire mode is ignored (it's always the detonator)

			// For grenades:
			// nLineNum 4 is blank
			// nLineNum 5 is fire mode number
			// nLineNum 6 is blast damage
			// nLineNum 7 is fallout damage
			// nLineNum 8 is cook time
			// nLineNum 9 is additional tags
			// For charges, the second fire mode is ignored (it's always the detonator)

			// For weapons with an explosive firing mode:
			// nLineNum 4 is blank
			// nLineNum 5 is fire mode number
			// nLineNum 6 is blast damage (?)
			// nLineNum 7 is fallout damage, if it exists (otherwise everything is pushed down)
			// nLineNum 8 is accuracy rating
			// nLineNum 9 is recoil
			// nLineNum 10 is fire rate
			// nLineNum 11 is additional tags

			// For projectile weapons (ie, everything else)
			// nLineNum 4 is blank
			// nLineNum 5 is fire mode number
			// nLineNum 6 is damage
			// nLineNum 7 is accuracy rating
			// nLineNum 8 is recoil
			// nLineNum 9 is fire rate
			// nLineNum 10 is additional tags

			// Figure out which firing mode we haven't done yet.
			for (int i = 0; i < pWeaponData->numFiringModes; i++) {
				if (!(bfTagFields & (1 << (i + IDTAG_FIREMODE)))) {
					if (i == 1 && pWeaponData->weaponBaseIndex == WP_DET_PACK) {
						// Plunger mode... we don't care about this one and shouldn't draw it
						continue;
					}
					nFiringMode = i;
					break;
				}
			}

			if (nFiringMode >= pWeaponData->numFiringModes || nFiringMode < 0) {
				return "";		// Don't draw anything for this line
			}

			idType = JKG_GetFiringModeType(pWeaponData, nFiringMode);
			pFireMode = &pWeaponData->firemodes[nFiringMode];
			pVFireMode = &pWeaponData->visuals.visualFireModes[nFiringMode];

			if (idType == IDTYPE_NODRAW) {
				return "";
			}

			switch (nLineNum) {
				case 3:
					nLineOffset++;
				case 4:
					goto blankLine;
				case 5:
					// Fire mode number
					if (pWeaponData->numFiringModes <= 1) {
						return (char*)UI_GetStringEdString2("@JKG_INVENTORY_WEAPON_STATS");
					}
					switch (nFiringMode) {
						case 0:
							return va(UI_GetStringEdString2("@JKG_INVENTORY_PRIMARY_FIRE"), pWeaponData->visuals.visualFireModes[nFiringMode].displayName);
						case 1:
							return va(UI_GetStringEdString2("@JKG_INVENTORY_SECONDARY_FIRE"), pWeaponData->visuals.visualFireModes[nFiringMode].displayName);
						case 2:
							return va(UI_GetStringEdString2("@JKG_INVENTORY_TERTIARY_FIRE"), pWeaponData->visuals.visualFireModes[nFiringMode].displayName);
						default:
							return va(UI_GetStringEdString2("@JKG_INVENTORY_FIRING_MODE"), nFiringMode + 1, pWeaponData->visuals.visualFireModes[nFiringMode].displayName);
					}
					break;
				case 6:
					switch (idType) {
						case IDTYPE_CHARGEMINE:
						case IDTYPE_GRENADE:
						case IDTYPE_EXPLOSIVEGUN:
							return "";	// FIXME - blast damage
						case IDTYPE_PROJECTILE:
							int nDamage = pFireMode->baseDamage <= 0 ? 0 : pFireMode->baseDamage; // FIXME
							if (nDamage <= 0) { // No damage. Don't draw.
								char* out = JKG_GetItemDescLine(pItem, nLineNum - nLineOffset + 1, recursionLevel + 1);
								nLineOffset++;
								return out;
							}

							char* szDamageTag = JKG_GetDamageTag(pWeaponData, nFiringMode);
							int nShotCount = pFireMode->shotCount;
							if (nShotCount > 1) {
								return va(UI_GetStringEdString2("@JKG_INVENTORY_WEP_DAMAGE_SCATTERGUN"), nDamage, nShotCount, szDamageTag);
							} else {
								return va(UI_GetStringEdString2("@JKG_INVENTORY_WEP_DAMAGE"), nDamage, szDamageTag);
							}
							break;
					}
					break;
				case 7:
					switch (idType) {
						case IDTYPE_CHARGEMINE:
						case IDTYPE_EXPLOSIVEGUN:
						case IDTYPE_GRENADE:
							return ""; // FIXME: fallout damage
						case IDTYPE_PROJECTILE:
							// Accuracy rating
							int nAccuracyBase = pFireMode->weaponAccuracy.accuracyRating;
							int nAccuracyMax = pFireMode->weaponAccuracy.maxAccuracyAdd + nAccuracyBase;
							return va(UI_GetStringEdString2("@JKG_INVENTORY_WEP_ACCURACY"), nAccuracyBase, nAccuracyMax);
							break;
					}
					break;
				case 8:
					// Grenades: Cook time
					// Charges/Mines: Additional tags
					// Explosive weapons: Accuracy rating
					// Projectile weapons: recoil
					switch (idType) {
						case IDTYPE_EXPLOSIVEGUN:
							// Accuracy rating
							nAccuracyBase = pFireMode->weaponAccuracy.accuracyRating;
							nAccuracyMax = pFireMode->weaponAccuracy.maxAccuracyAdd + nAccuracyBase;
							return va(UI_GetStringEdString2("@JKG_INVENTORY_WEP_ACCURACY"), nAccuracyBase, nAccuracyMax);
						case IDTYPE_GRENADE:
							nCookTime = pWeaponData->weaponReloadTime;
							return va(UI_GetStringEdString2("@JKG_INVENTORY_WEP_COOKTIME"), nCookTime);
						case IDTYPE_PROJECTILE:
							return (char*)JKG_GetRecoilString(pWeaponData, nFiringMode);
						case IDTYPE_CHARGEMINE:
							goto additionalTags;
					}
					break;
				case 9:
					// Grenades: additional tags
					// Charges/Mines: Additional tags
					// Explosive weapons: Recoil
					// Projectile weapons: Fire rate
					switch (idType) {
						case IDTYPE_EXPLOSIVEGUN:
							return (char*)JKG_GetRecoilString(pWeaponData, nFiringMode);
						case IDTYPE_PROJECTILE:
							nFireTime = pFireMode->delay;
							nRPM = (int)((1000.0f / nFireTime) * 60.0f);
							return va(UI_GetStringEdString2("@JKG_INVENTORY_WEP_FIRETIME"), nRPM);
						default:
							goto additionalTags;
					}
					break;
				case 10:
					// Explosive weapons: Fire rate
					// Everything else: additional tags
					if (idType == IDTYPE_EXPLOSIVEGUN) {
						nFireTime = pFireMode->delay;
						nRPM = (int)((1000.0f / nFireTime) * 60.0f);
						return va(UI_GetStringEdString2("@JKG_INVENTORY_WEP_FIRETIME"), nRPM);
					}
				default:
additionalTags:
					{
						if (!(bfFireModeTags[nFiringMode] & (1 << IDMTAG_FIRECONTROL))) {
							char* szReturnValue = nullptr;
							if (pFireMode->firingType == FT_SEMI) {
								szReturnValue = (char*)UI_GetStringEdString2("@JKG_INVENTORY_WEP_TAG_SEMI_AUTO");
							}
							else if (pFireMode->firingType == FT_AUTOMATIC) {
								szReturnValue = (char*)UI_GetStringEdString2("@JKG_INVENTORY_WEP_TAG_FULL_AUTO");
							}
							else {
								szReturnValue = (char*)UI_GetStringEdString2("@JKG_INVENTORY_WEP_TAG_BURST");
							}
							bfFireModeTags[nFiringMode] |= (1 << IDMTAG_FIRECONTROL);
							nLineOffset--;
							return szReturnValue;
						}
						else {
							bfTagFields |= (1 << (nFiringMode + IDTAG_FIREMODE));
							// Mines and charges have offset of 5?
							// Grenades have offset of 7?
							// Explosive weapons have offset of 8?
							// Projectiles have offset of 7?
							if (idType == IDTYPE_CHARGEMINE) {
								nLineOffset -= 5;
							}
							else if (idType == IDTYPE_GRENADE) {
								nLineOffset -= 7;
							}
							else if (idType == IDTYPE_EXPLOSIVEGUN) {
								nLineOffset -= 8;
							}
							else if (idType == IDTYPE_PROJECTILE) {
								nLineOffset -= 7;
							}
							return JKG_GetItemDescLine(pItem, nLineNum, recursionLevel + 1);
						}
					}
					break;
			}
			break;
		case ITEM_ARMOR:
			if (nLineNum == 0) {
				return (char*)UI_GetStringEdString2("@JKG_INVENTORY_ITYPE_ARMOR");
			}
			break;
		case ITEM_CONSUMABLE:
			if (nLineNum == 0) {
				return (char*)UI_GetStringEdString2("@JKG_INVENTORY_ITYPE_CONSUMABLE");
			}
			break;
	}
blankLine:
	return "";
}

/*
==========================
OWNERDRAW STUFF
==========================
*/

extern void Item_Text_Paint(itemDef_t *item);

// Draws the "Credits: X" text
void JKG_Inventory_OwnerDraw_CreditsText(itemDef_t* item)
{
	int credits = *(int*)cgImports->InventoryDataRequest(3);
	float x = item->window.rect.x;
	float y = item->window.rect.y;
	int font = item->iMenuFont;
	float scale = item->textscale;
	char buffer[256];
	trap->SE_GetStringTextString("JKG_INVENTORY_CREDITS", buffer, sizeof(buffer));

	char* text = va("%s %i", buffer, credits);
	size_t len = strlen(text);
	trap->R_Font_DrawString(x, y, text, item->window.foreColor, font, len * 10, scale);
}

// Draws each item icon in the inventory list (ownerDrawID being the slot)
void JKG_Inventory_OwnerDraw_ItemIcon(itemDef_t* item, int ownerDrawID) {
	int nItemNum = ownerDrawID + nPosition;
	if (nItemNum >= pItems.size()) {
		return;
	}
	itemInstance_t* pItem = pItems[nItemNum].second;
	qhandle_t shader = trap->R_RegisterShaderNoMip(pItem->id->visuals.itemIcon);
	trap->R_DrawStretchPic(item->window.rect.x, item->window.rect.y, item->window.rect.w, item->window.rect.h,
		0, 0, 1, 1, shader);
}

// Draws each item name in the inventory list (ownerDrawID being the slot)
void JKG_Inventory_OwnerDraw_ItemName(itemDef_t* item, int ownerDrawID) {
	int nItemNum = ownerDrawID + nPosition;
	if (nItemNum >= pItems.size()) {
		memset(item->text, 0, sizeof(item->text));
		Item_Text_Paint(item); // FIXME: should we really be trying to paint a blank string?
		return;
	}
	itemInstance_t* pItem = pItems[nItemNum].second;
	strcpy(item->text, pItem->id->displayName);
	Item_Text_Paint(item);
}

// Draws additional stuff below the item name
// Top line. OwnerDrawID is the item slot number
void JKG_Inventory_OwnerDraw_ItemTagTop(itemDef_t* item, int ownerDrawID) {
	int nItemNum = ownerDrawID + nPosition;
	memset(item->text, 0, sizeof(item->text));
	if (nItemNum >= pItems.size()) {
		Item_Text_Paint(item);
		return;
	}
	itemInstance_t* pItem = pItems[nItemNum].second;
	// If it's in an ACI slot, mention this
	// FIXME: pItem->equipped should be valid!! but it's not!!
	int* pACI = (int*)cgImports->InventoryDataRequest(2);
	assert(pACI != nullptr);
	for (int i = 0; i < 10; i++) { // FIXME: use something other than literal 10, in case ACI size goes up or down
		if (pACI[i] == nItemNum) {
			strcpy(item->text, va(UI_GetStringEdString2("@JKG_INVENTORY_INACI"), i));
			memcpy(item->window.foreColor, colorTable[CT_CYAN], sizeof(item->window.foreColor));
			break;
		}
	}
	Item_Text_Paint(item);
}

// Bottom line. OwnerDrawID is the item slot number
void JKG_Inventory_OwnerDraw_ItemTagBottom(itemDef_t* item, int ownerDrawID) {
	int nItemNum = ownerDrawID + nPosition;
	memset(item->text, 0, sizeof(item->text));
	// Currently not used
	//if (nItemNum >= nNumInventoryItems) {
		Item_Text_Paint(item);
		return;
	//}
}

// Draws the highlight when an item is selected (ownerDrawID being the slot)
void JKG_Inventory_OwnerDraw_SelHighlight(itemDef_t* item, int ownerDrawID) {
	int nItemNum = ownerDrawID + nPosition;
	if (nItemNum != nSelected) {
		return;	// Don't draw the highlight because it's not the one that we have selected
	}
	trap->R_DrawStretchPic(item->window.rect.x, item->window.rect.y,
		item->window.rect.w, item->window.rect.h, 0, 0, 1, 1,
		item->window.background);
}

// Draws the selected item's icon
void JKG_Inventory_OwnerDraw_SelItemIcon(itemDef_t* item) {
	if (nSelected == -1 || nSelected > pItems.size()) {
		return;
	}
	itemInstance_t* pItem = pItems[nSelected].second;
	qhandle_t shader = trap->R_RegisterShaderNoMip(pItem->id->visuals.itemIcon);
	trap->R_DrawStretchPic(item->window.rect.x, item->window.rect.y,
		item->window.rect.w, item->window.rect.h, 0, 0, 1, 1, shader);
}

// Draws the selected item's name
void JKG_Inventory_OwnerDraw_SelItemName(itemDef_t* item) {
	if (nSelected == -1 || nSelected > pItems.size()) {
		return;
	}
	itemInstance_t* pItem = pItems[nSelected].second;
	strcpy(item->text, pItem->id->displayName);
	Item_Text_Paint(item);
}

// Draws the selected item's description (ownerDrawID being the line number)
void JKG_Inventory_OwnerDraw_SelItemDesc(itemDef_t* item, int ownerDrawID) {
	if (nSelected == -1 || nSelected > pItems.size()) {
		return;
	}
	itemInstance_t* pItem = pItems[nSelected].second;
	strcpy(item->text, JKG_GetItemDescLine(pItem, ownerDrawID, 0));
	Item_Text_Paint(item);
}

// Draws the interaction buttons
void JKG_Inventory_OwnerDraw_Interact(itemDef_t* item, int ownerDrawID) {
	itemInstance_t* pItem = nullptr;
	memset(item->text, 0, sizeof(item->text));
	if (nSelected == -1 || nSelected > pItems.size()) {
		return;
	}
	pItem = pItems[nSelected].second;
	if (ownerDrawID == 0) {
		// Assign to ACI
		if (pItem->id->itemType == ITEM_ARMOR) {
			if (pItem->equipped) {
				strcpy(item->text, UI_GetStringEdString2("@JKG_INVENTORY_UNEQUIP"));
				item->action = "jkgscript inv_unequip";
			}
			else {
				strcpy(item->text, UI_GetStringEdString2("@JKG_INVENTORY_EQUIP"));
				item->action = "jkgscript inv_equip";
			}
		}
		else {
			strcpy(item->text, UI_GetStringEdString2("@JKG_INVENTORY_ACI"));
			item->action = "clearfocus ; show interact1 ; show interact_submenu1_assignaci ; hide interact_submenu2_destroy ; hide interact2; disable inv_feederSel 1";
		}
	} else if (ownerDrawID == 1) {
		// Destroy button
		strcpy(item->text, UI_GetStringEdString2("@JKG_INVENTORY_DESTROY"));
		item->action = "clearfocus ; show interact2 ; show interact_submenu2_destroy ; disable inv_feederSel 1 ; hide interact_submenu1_assignaci ; hide interact1";
	}
	else if (ownerDrawID == 2) {
		// On consumables, this is the "use" button
		if (pItem->id->itemType != ITEM_ARMOR && pItem->id->itemType != ITEM_CLOTHING &&
			pItem->id->itemType != ITEM_WEAPON) {
			strcpy(item->text, UI_GetStringEdString2("@JKG_INVENTORY_USE"));
		}
		item->action = "jkgscript inv_use";
	}
}

extern void Item_RunScript(itemDef_t *item, const char *s);
void JKG_Inventory_OwnerDraw_Interact_Button(int ownerDrawID, int key) {
	itemDef_t* pTarget = nullptr;
	menuDef_t* pTargetMenu = nullptr;
	if (key != A_MOUSE1 && key != A_MOUSE2) {
		return;
	}
	pTargetMenu = Menus_FindByName("jkg_inventory");
	if (pTargetMenu == nullptr) {
		// how in the f-ck did you get here...
		return;
	}
	pTarget = Menu_FindItemByName(pTargetMenu, va("interact%i_button", ownerDrawID + 1));
	if (pTarget == nullptr) {
		// the item is probably not defined
		return;
	}
	Item_RunScript(pTarget, pTarget->action);
}

bool JKG_Inventory_ShouldDraw_Interact(int nWhich) {
	if (nSelected == -1 || nSelected > pItems.size()) {
		return false;
	}
	return true;
}

/*
==========================
BUTTON PRESSES
==========================
*/
void JKG_Inventory_SelectItem(char** args) {
	int nWhich = atoi(args[0]);
	if (nPosition + nWhich >= pItems.size()) {
		nSelected = -1;
		Menu_ShowItemByName(Menus_FindByName("jkg_inventory"), "shop_preview", qfalse);

	} else {
		nSelected = nPosition + nWhich;
		Menu_ShowItemByName(Menus_FindByName("jkg_inventory"), "shop_preview", qtrue);
	}
}

void JKG_Inventory_ArrowUp(char** args) {
	if (nPosition > 0) {
		nPosition--;
	}
	JKG_ConstructInventoryList();
}

void JKG_Inventory_ArrowDown(char** args) {
	if (nPosition < pItems.size()-1) {
		nPosition++;
	}
	JKG_ConstructInventoryList();
}

void JKG_Inventory_ACISlot(char** args) {
	int nSlot = atoi(args[0]);
	cgImports->InventoryAttachToACI(pItems[nSelected].first, nSlot, true);
}

void JKG_Inventory_ACISlotAuto(char** args) {
	cgImports->InventoryAttachToACI(pItems[nSelected].first, -1, true);
}

void JKG_Inventory_ACIRemove(char** args) {
	cgImports->InventoryAttachToACI(pItems[nSelected].first, -1, false);
}

void JKG_Inventory_Destroy(char** args) {
	if (nSelected == -1 || nSelected > pItems.size()) {
		return;
	}
	cgImports->SendClientCommand(va("inventoryDestroy %d", pItems[nSelected].first));
	pItems.erase(pItems.begin() + nSelected);
	nSelected = -1;
}

void JKG_Inventory_Use(char** args) {
	if (nSelected == -1 || nSelected > pItems.size()) {
		return;
	}
	cgImports->SendClientCommand(va("inventoryUse %d", pItems[nSelected].first));
}

void JKG_Inventory_EquipArmor(char** args) {
	if (nSelected == -1 || nSelected > pItems.size()) {
		return;
	}
	cgImports->SendClientCommand(va("equip %d", pItems[nSelected].first));
}

void JKG_Inventory_UnequipArmor(char** args) {
	if (nSelected == -1 || nSelected > pItems.size()) {
		return;
	}
	cgImports->SendClientCommand(va("unequip %d", pItems[nSelected].first));
}

void JKG_Inventory_Interact(char** args) {
	int nArg = atoi(args[0]);
	if (nSelected == -1 || nSelected > pItems.size()) {
		return;
	}
	itemInstance_t* pItem = pItems[nSelected].second;
	switch (nArg) {
		default:
		case 0:
			// Assign to ACI / Equip
			if (pItem->id->itemType == ITEM_ARMOR) {

			}
			else {
				cgImports->InventoryAttachToACI(pItems[nSelected].first, -1, true);
			}
			JKG_Inventory_UpdateNotify(1);
			break;
		case 1:
			// Does nothing / this is entirely handled by menu code
			break;
		case 2:
			// Use item
			JKG_Inventory_UpdateNotify(1);
			break;
	}
}

void JKG_Inventory_Open(char** args) {
	JKG_Inventory_UpdateNotify(0);
}

void JKG_Inventory_ReconstructList(char** args) {
	JKG_ConstructInventoryList();
}

void JKG_UI_InventoryFilterChanged() {
	JKG_ConstructInventoryList();
}

void JKG_Inventory_UpdateNotify(int msg) {
	switch (msg)
	{
	case 0: // open
		if (Menus_FindByName("jkg_inventory") && Menus_ActivateByName("jkg_inventory"))
		{
			trap->Key_SetCatcher(trap->Key_GetCatcher() | KEYCATCH_UI /*& ~KEYCATCH_CONSOLE*/);
		}
		JKG_ConstructInventoryList();
		break;

	case 1: // update!
		trap->Print("debug: [Inventory::UpdateNotify]\n");
		JKG_ConstructInventoryList();
		break;
	case 2:	// open as shop menu
		if (Menus_FindByName("jkg_inventory") && Menus_ActivateByName("jkg_inventory"))
		{
			trap->Key_SetCatcher(trap->Key_GetCatcher() | KEYCATCH_UI/* & ~KEYCATCH_CONSOLE*/);
		}
		break;
	}
}
