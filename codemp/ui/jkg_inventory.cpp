#include "ui_local.h"
#include "jkg_inventory.h"

static size_t nNumInventoryItems = 0;			// number of items in the list
static std::vector<std::pair<int, itemInstance_t*>> pItems;	// list of items
static int nPosition = 0;					// position in the item list (changed with arrow buttons)
static int nSelected = -1;					// selected item in the list (-1 for no selection)
static std::vector<std::string> vItemDescLines;	// item description

void JKG_ConstructItemDescription(itemInstance_t* pItem, std::vector<std::string>& vDescLines);

void JKG_ConstructInventoryList() {
	itemInstance_t* pAllItems = nullptr;

	vItemDescLines.clear();
	pItems.clear();

	if (cgImports == nullptr) {
		// This gets called when the game starts (because ui_inventoryFilter was modified), so just return
		return;
	}

	nNumInventoryItems = *(size_t*)cgImports->InventoryDataRequest(INVENTORYREQUEST_SIZE, -1);
	if (nNumInventoryItems > 0) {
		pAllItems = (itemInstance_t*)cgImports->InventoryDataRequest(INVENTORYREQUEST_ITEMS, -1);
		for (int i = 0; i < nNumInventoryItems; i++) {
			itemInstance_t* pThisItem = &pAllItems[i];
			if (ui_inventoryFilter.integer == JKGIFILTER_ARMOR
				&& pThisItem->id->itemType != ITEM_ARMOR
				&& pThisItem->id->itemType != ITEM_CLOTHING
				&& pThisItem->id->itemType != ITEM_SHIELD
				&& pThisItem->id->itemType != ITEM_JETPACK) {
				continue;
			}
			else if (ui_inventoryFilter.integer == JKGIFILTER_WEAPONS && pThisItem->id->itemType != ITEM_WEAPON) {
				continue;
			}
			else if (ui_inventoryFilter.integer == JKGIFILTER_CONSUMABLES && pThisItem->id->itemType != ITEM_CONSUMABLE) {
				continue;
			}
			else if (ui_inventoryFilter.integer == JKGIFILTER_TOOLS && pThisItem->id->itemType != ITEM_TOOL)
			{
				continue;
			}
			else if (ui_inventoryFilter.integer == JKGIFILTER_AMMO && pThisItem->id->itemType != ITEM_AMMO)
			{
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

	if (nSelected < pItems.size() && nSelected >= 0) {
		JKG_ConstructItemDescription(pItems[nSelected].second, vItemDescLines);
	}
}

/*
==========================
DESCRIPTION CONSTRUCTION
==========================
*/

// Display the item's tier (quality/rarity)
static QINLINE void JKG_ConstructItemTierDescription(itemTier_t tier, std::vector<std::string>& vDescLines)
{
	const char toWhite[3] = "^7";
	char line[1024] = "";
	switch (tier)
	{
	case TIER_SCRAP:
		Q_strcat(line, sizeof(line), va("^9")); //gray
		Q_strcat(line, sizeof(line), va(UI_GetStringEdString2("@JKG_INVENTORY_ITEM_TIER_0")));
		Q_strcat(line, sizeof(line), toWhite);
		Q_strcat(line, sizeof(line), va( " (%i)", static_cast<int>(tier)));
		vDescLines.push_back(va(UI_GetStringEdString2("@JKG_INVENTORY_ITEM_TIER"), line));
		line[0] = '\0';
		break;

	case TIER_COMMON:
		Q_strcat(line, sizeof(line), va(UI_GetStringEdString2("@JKG_INVENTORY_ITEM_TIER_1")));
		Q_strcat(line, sizeof(line), va(" (%i)", static_cast<int>(tier)));
		vDescLines.push_back(va(UI_GetStringEdString2("@JKG_INVENTORY_ITEM_TIER"), line));
		line[0] = '\0';
		break;

	case TIER_REFINED:
		Q_strcat(line, sizeof(line), va("^xcf5")); //light green
		Q_strcat(line, sizeof(line), va(UI_GetStringEdString2("@JKG_INVENTORY_ITEM_TIER_2")));
		Q_strcat(line, sizeof(line), toWhite);
		Q_strcat(line, sizeof(line), va(" (%i)", static_cast<int>(tier)));
		vDescLines.push_back(va(UI_GetStringEdString2("@JKG_INVENTORY_ITEM_TIER"), line));
		line[0] = '\0';
		break;

	case TIER_ELITE:
		Q_strcat(line, sizeof(line), va("^x9ff")); //light blue
		Q_strcat(line, sizeof(line), va(UI_GetStringEdString2("@JKG_INVENTORY_ITEM_TIER_3")));
		Q_strcat(line, sizeof(line), toWhite);
		Q_strcat(line, sizeof(line), va(" (%i)", static_cast<int>(tier)));
		vDescLines.push_back(va(UI_GetStringEdString2("@JKG_INVENTORY_ITEM_TIER"), line));
		line[0] = '\0';
		break;

	case TIER_SUPERIOR:
		Q_strcat(line, sizeof(line), va("^xf9f")); //light purple
		Q_strcat(line, sizeof(line), va(UI_GetStringEdString2("@JKG_INVENTORY_ITEM_TIER_4")));
		Q_strcat(line, sizeof(line), toWhite);
		Q_strcat(line, sizeof(line), va(" (%i)", static_cast<int>(tier)));
		vDescLines.push_back(va(UI_GetStringEdString2("@JKG_INVENTORY_ITEM_TIER"), line));
		line[0] = '\0';
		break;

	case TIER_LEGENDARY:
		Q_strcat(line, sizeof(line), va("^3")); //yellow
		Q_strcat(line, sizeof(line), va(UI_GetStringEdString2("@JKG_INVENTORY_ITEM_TIER_5")));
		Q_strcat(line, sizeof(line), toWhite);
		Q_strcat(line, sizeof(line), va(" (%i)", static_cast<int>(tier)));
		vDescLines.push_back(va(UI_GetStringEdString2("@JKG_INVENTORY_ITEM_TIER"), line));
		line[0] = '\0';
		break;

	default:
		vDescLines.push_back(va(UI_GetStringEdString2("@JKG_INVENTORY_ITEM_TIER"), "Unknown", static_cast<int>(tier)));
		break;
	}
}

//lighten rgb color to make it easier to read
void JKG_LightenTextColor(vec4_t& color)
{  //todo: move to somewhere more universally useful?
	float percent = 0.6;	//% to lighten colors by
	float highest = color[0] < color[1] ? color[1] : color[0]; highest = highest < color[2] ? color[2] : highest;	//determine highest RGB value

	for (int i = 0; i < 2; i++) //don't change alpha
	{
		color[i] = color[i] + ((1.0 - color[i]) * percent); //lighten color by %

		//give 10% higher weight to the most significant color(s)
		if (color[i] == highest)
			color[i] += 0.1;

		if (color[i] > 1.0)
			color[i] = 1.0; //clamp colors
	}
}

void JKG_SetTierColor(int tier, vec4_t& color)
{
	switch (tier)
	{
	case TIER_SCRAP:
		VectorCopy4(colorMdGrey, color);
		break;
	case TIER_COMMON:
		VectorCopy4(colorWhite, color);
		break;
	case TIER_REFINED:
		VectorCopy4(colorGreen, color);
		JKG_LightenTextColor(color);
		break;
	case TIER_ELITE:
		VectorCopy4(colorCyan, color);
		JKG_LightenTextColor(color);
		break;
	case TIER_SUPERIOR:
		VectorCopy4(colorMagenta, color);
		JKG_LightenTextColor(color);
		break;
	case TIER_LEGENDARY:
		VectorCopy4(colorYellow, color);
		break;
	default:
		VectorCopy4(colorWhite, color);
		break;
	}
}

// Returns the proper tag that should appear with Blast Damage
char* JKG_GetBlastDamageTag(weaponData_t* pData, const int nFiringMode) {
	return ""; // Damage stuff isn't loaded yet :<
}

// Returns the proper tag that should appear with Fallout Damage
char* JKG_GetFalloutDamageTag(weaponData_t* pData, const int nFiringMode) {
	return ""; // Damage stuff isn't loaded yet :<
}

// Returns the proper tag that should appear (with either "Damage: " or "Direct Damage: ")
char* JKG_GetDamageTag(weaponData_t* pData, const int nFiringMode) {
	meansOfDamage_t* means = JKG_GetMeansOfDamage(pData->firemodes[nFiringMode].weaponMOD);
	if (means) {
		return (char*)UI_GetStringEdString2(means->inventoryName);
	}
	else {
		return "";
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

// Returns the slot name (Head, Shoulder, ...) for armor
const char* JKG_GetArmorSlotString(armorData_t* pData) {
	switch (pData->slot) {
		case ARMSLOT_HEAD:
			return UI_GetStringEdString3("@JKG_INVENTORY_SLOT_HEAD");
		case ARMSLOT_NECK:
			return UI_GetStringEdString3("@JKG_INVENTORY_SLOT_NECK");
		case ARMSLOT_TORSO:
			return UI_GetStringEdString3("@JKG_INVENTORY_SLOT_TORSOT");
		case ARMSLOT_ROBE:
			return UI_GetStringEdString3("@JKG_INVENTORY_SLOT_ROBE");
		case ARMSLOT_LEGS:
			return UI_GetStringEdString3("@JKG_INVENTORY_SLOT_LEGS");
		case ARMSLOT_GLOVES:
			return UI_GetStringEdString3("@JKG_INVENTORY_SLOT_GLOVES");
		case ARMSLOT_BOOTS:
			return UI_GetStringEdString3("@JKG_INVENTORY_SLOT_BOOTS");
		case ARMSLOT_SHOULDER:
			return UI_GetStringEdString3("@JKG_INVENTORY_SLOT_SHOULDER");
		case ARMSLOT_IMPLANTS:
			return UI_GetStringEdString3("@JKG_INVENTORY_SLOT_IMPLANT");
	}
	return "";
}

// Create a jetpack item's description
static void JKG_ConstructJetpackDescription(itemInstance_t* pItem, std::vector<std::string>& vDescLines) {
	// Jetpack
	// Capacity: ###
	// Idle Fuel Consumption: ###
	// Thrusting Fuel Consumption: ###
	// Fuel Regeneration: ###
	// Hover Gravity: ###
	vDescLines.push_back(UI_GetStringEdString2("@JKG_INVENTORY_ITYPE_JETPACK"));
	JKG_ConstructItemTierDescription(pItem->id->itemTier, vDescLines);
	if (pItem->id->weight > 0.0f)
	{
		vDescLines.push_back(va(UI_GetStringEdString2("@JKG_INVENTORY_ITEM_WEIGHT"), pItem->id->weight));
	}
	vDescLines.push_back(va(UI_GetStringEdString2("@JKG_INVENTORY_CAPACITY"), pItem->id->jetpackData.pJetpackData->fuelCapacity));
	vDescLines.push_back(va(UI_GetStringEdString2("@JKG_INVENTORY_JETPACK_IDLECONSUMPTION"), pItem->id->jetpackData.pJetpackData->fuelConsumption));
	vDescLines.push_back(va(UI_GetStringEdString2("@JKG_INVENTORY_JETPACK_THRUSTCONSUMPTION"), pItem->id->jetpackData.pJetpackData->thrustConsumption));
	vDescLines.push_back(va(UI_GetStringEdString2("@JKG_INVENTORY_JETPACK_FUELREGEN"), pItem->id->jetpackData.pJetpackData->fuelRegeneration));
	vDescLines.push_back(va(UI_GetStringEdString2("@JKG_INVENTORY_JETPACK_HOVERGRAVITY"), pItem->id->jetpackData.pJetpackData->move.hoverGravity));
}

// Create a shield item's description
static void JKG_ConstructShieldDescription(itemInstance_t* pItem, std::vector<std::string>& vDescLines) {
	// Shield Item
	// Capacity: ###
	// Recharge Time: ### seconds
	// Regeneration: # shields per second
	vDescLines.push_back(UI_GetStringEdString2("@JKG_INVENTORY_ITYPE_SHIELD"));
	JKG_ConstructItemTierDescription(pItem->id->itemTier, vDescLines);
	if (pItem->id->weight > 0.0f)
	{
		vDescLines.push_back(va(UI_GetStringEdString2("@JKG_INVENTORY_ITEM_WEIGHT"), pItem->id->weight));
	}
	vDescLines.push_back(va(UI_GetStringEdString2("@JKG_INVENTORY_CAPACITY"), pItem->id->shieldData.capacity));
	vDescLines.push_back(va(UI_GetStringEdString2("@JKG_INVENTORY_SHIELD_RECHARGE"), pItem->id->shieldData.cooldown / 1000.0f));
	vDescLines.push_back(va(UI_GetStringEdString2("@JKG_INVENTORY_SHIELD_REGEN"), 1000.0f / pItem->id->shieldData.regenrate));
}

// Create an armor item's description
static void JKG_ConstructArmorDescription(itemInstance_t* pItem, std::vector<std::string>& vDescLines) {
	// Armor Item
	// Equipped in %s slot
	// Armor: %i (%.2f reduced %s damage taken)
	// Movement Speed: (+)%.1f%%
	// Maximum Health: (+)%i
	armorData_t* pArmorData = pItem->id->armorData.pArm;

	vDescLines.push_back(UI_GetStringEdString2("@JKG_INVENTORY_ITYPE_ARMOR"));
	JKG_ConstructItemTierDescription(pItem->id->itemTier, vDescLines);
	if (pItem->id->weight > 0.0f)
	{
		vDescLines.push_back(va(UI_GetStringEdString2("@JKG_INVENTORY_ITEM_WEIGHT"), pItem->id->weight));
	}
	vDescLines.push_back(va(UI_GetStringEdString2("@JKG_INVENTORY_ARM_EQUIPPEDSLOT"), JKG_GetArmorSlotString(pArmorData)));
	if (pArmorData->armor) {
		int maxHP = cgImports->GetPredictedPlayerState()->stats[STAT_MAX_HEALTH];
		int itemHP = pArmorData->hp;
		float damageReduction;

		// If the item is already equipped...then that means that the HP modifier is already applied!
		// Applying the HP modifier to the calculation (again) will lead to a bugged display
		if (pItem->equipped) {
			damageReduction = pArmorData->armor / (float)(maxHP + pArmorData->armor);
		}
		else {
			damageReduction = pArmorData->armor / (float)(maxHP + itemHP + pArmorData->armor);
		}

		damageReduction *= 100.0f;
		if (pArmorData->armor < 0) {
			vDescLines.push_back(va(UI_GetStringEdString2("@JKG_INVENTORY_ARM_NEG_DEFENSE"), pArmorData->armor, damageReduction * -1));
		}
		else {
			vDescLines.push_back(va(UI_GetStringEdString2("@JKG_INVENTORY_ARM_DEFENSE"), pArmorData->armor, damageReduction));
		}
	}
	if (pArmorData->movemodifier >= 0.005 || pArmorData->movemodifier <= -0.005) {
		// only differences above 0.5% movement speed will actually matter
		float speed = (1.0f - pArmorData->movemodifier) * 100.0f;
		if (speed < 0) {
			vDescLines.push_back(va(UI_GetStringEdString2("@JKG_INVENTORY_ARM_MOVESPEED_POS"), -speed));
		}
		if (speed > 0)
		{
			vDescLines.push_back(va(UI_GetStringEdString2("@JKG_INVENTORY_ARM_MOVESPEED"), +speed));
		}
	}
	if (pArmorData->hp) {
		if (pArmorData->hp > 0) {
			vDescLines.push_back(va(UI_GetStringEdString2("@JKG_INVENTORY_ARM_MAXHEALTH_POS"), pArmorData->hp));
		}
		else {
			vDescLines.push_back(va(UI_GetStringEdString2("@JKG_INVENTORY_ARM_MAXHEALTH_NEG"), pArmorData->hp));
		}
	}
	if (pArmorData->stamina)
	{
		if (pArmorData->stamina > 0) {
			vDescLines.push_back(va(UI_GetStringEdString2("@JKG_INVENTORY_ARM_MAXSTAMINA_POS"), pArmorData->stamina));
		}
		else {
			vDescLines.push_back(va(UI_GetStringEdString2("@JKG_INVENTORY_ARM_MAXSTAMINA_NEG"), pArmorData->stamina));
		}
	}

	if (pArmorData->filter)
	{
		vDescLines.push_back(va(UI_GetStringEdString2("@JKG_INVENTORY_ARM_FILTER")));
	}
}

// Convert units into feet
static QINLINE float JKG_UnitsToFeet(float units) {
	return units / 8.0f; // according to ensiform
}

// Convert units into meters (metres if you're bad at spelling)
static QINLINE float JKG_UnitsToMeters(float units) {
	return units * 0.305f; // according to ensiform
}

// Add information about a weapon's firing mode
static void JKG_ConstructFiringModeDescription(weaponData_t* pWP, int firemode, std::vector<std::string>& vDescLines) {
	// Primary/Secondary/Tertiary/Firing Mode %i (%s)
	/*
	[Projectile Weapons]
		Damage: %i-%i (%s)
		Accuracy Rating: %i-%i
		Recoil: %s
		Fires %i projectiles per minute
		Additional tags

	[Grenades]
		Blast Damage: %i-%i (%s)
		Blast Radius: %i
		(if present) Fallout Damage: %i-%i (%s)

	[Explosive Firing Modes]
		Direct Damage: %i-%i (%s)
		Blast Damage: %i-%i (%s)
		Blast Radius: %i
		(if present) Fallout Damage: %i-%i (%s)
		Accuracy Rating: %i-%i
		Recoil: %s
		Fires %i projectiles per minute
		Additional tags
	*/
	weaponFireModeStats_t* pFM = &pWP->firemodes[firemode];
	weaponVisualFireMode_t* pVFM = &pWP->visuals.visualFireModes[firemode];

	if (firemode == 1 && pWP->weaponBaseIndex == WP_DET_PACK) {
		// Ultra fringe case...don't draw the plunger mode
		return;
	}

	switch (firemode) {
		case 0:
			vDescLines.push_back(va(UI_GetStringEdString2("@JKG_INVENTORY_PRIMARY_FIRE"), pVFM->displayName));
			break;
		case 1:
			vDescLines.push_back(va(UI_GetStringEdString2("@JKG_INVENTORY_SECONDARY_FIRE"), pVFM->displayName));
			break;
		case 2:
			vDescLines.push_back(va(UI_GetStringEdString2("@JKG_INVENTORY_TERTIARY_FIRE"), pVFM->displayName));
			break;
		default:
			vDescLines.push_back(va(UI_GetStringEdString2("@JKG_INVENTORY_FIRING_MODE"), firemode + 1, pVFM->displayName));
			break;

		//ammo type
		if (pFM->ammoBase->shortname)
		{
			vDescLines.push_back(va(UI_GetStringEdString3("@JKG_INVENTORY_WEP_AMMOTYPE"), pFM->ammoBase->shortname));
		}
	}

	if (pWP->weaponBaseIndex != WP_TRIP_MINE && pWP->weaponBaseIndex != WP_DET_PACK && pWP->weaponBaseIndex != WP_THERMAL && pFM->baseDamage > 0) {
		// Damage
		char* szDamageTag = JKG_GetDamageTag(pWP, firemode);

		if (pFM->rangeSplash) {
			if (pFM->shotCount > 1) {
				vDescLines.push_back(va(UI_GetStringEdString3("@JKG_INVENTORY_WEP_BDAMAGE_SCATTERGUN"), pFM->baseDamage, pFM->shotCount, szDamageTag));
			}
			else {
				vDescLines.push_back(va(UI_GetStringEdString3("@JKG_INVENTORY_WEP_BDAMAGE"), pFM->baseDamage, szDamageTag));
			}
			vDescLines.push_back(va(UI_GetStringEdString2("@JKG_INVENTORY_WEP_BRADIUS"), 
				(int)pFM->rangeSplash, JKG_UnitsToFeet(pFM->rangeSplash), JKG_UnitsToMeters(pFM->rangeSplash)));
		}
		else if (pFM->shotCount > 1) {
			vDescLines.push_back(va(UI_GetStringEdString3("@JKG_INVENTORY_WEP_DAMAGE_SCATTERGUN"), pFM->baseDamage, pFM->shotCount, szDamageTag));
		}
		else {
			vDescLines.push_back(va(UI_GetStringEdString3("@JKG_INVENTORY_WEP_DAMAGE"), pFM->baseDamage, szDamageTag));
		}

		// Accuracy rating
		int nAccuracyBase = pFM->weaponAccuracy.accuracyRating;
		int nAccuracyMax = pFM->weaponAccuracy.maxAccuracyAdd + nAccuracyBase;
		vDescLines.push_back(va(UI_GetStringEdString2("@JKG_INVENTORY_WEP_ACCURACY"), nAccuracyBase, nAccuracyMax));

		// Recoil
		vDescLines.push_back(JKG_GetRecoilString(pWP, firemode));

		// Fire rate
		int nFireTime = pFM->delay;
		int nRPM = (int)((1000.0f / nFireTime) * 60.0f);
		vDescLines.push_back(va(UI_GetStringEdString2("@JKG_INVENTORY_WEP_FIRETIME"), nRPM));

		// Fire control
		switch (pFM->firingType) {
			case FT_SEMI:
				vDescLines.push_back(UI_GetStringEdString2("@JKG_INVENTORY_WEP_TAG_SEMI_AUTO"));
				break;
			case FT_AUTOMATIC:
				vDescLines.push_back(UI_GetStringEdString2("@JKG_INVENTORY_WEP_TAG_FULL_AUTO"));
				break;
			default:
				vDescLines.push_back(UI_GetStringEdString2("@JKG_INVENTORY_WEP_TAG_BURST"));
				break;
		}
	}
	else if (pFM->baseDamage > 0) {
		char* szDamageTag = JKG_GetDamageTag(pWP, firemode);
		vDescLines.push_back(va(UI_GetStringEdString3("@JKG_INVENTORY_WEP_BDAMAGE"), pFM->baseDamage, szDamageTag));
		vDescLines.push_back(va(UI_GetStringEdString2("@JKG_INVENTORY_WEP_BRADIUS"), 
			(int)pFM->rangeSplash, JKG_UnitsToFeet(pFM->rangeSplash), JKG_UnitsToMeters(pFM->rangeSplash)));
	}

	vDescLines.push_back(""); // Add a blank line at the end because formatting is good (TM)
}

// Create a weapon item's description
static void JKG_ConstructWeaponDescription(itemInstance_t* pItem, std::vector<std::string>& vDescLines) {
	// Weapon Item
	// Reload Time --OR-- Cook Time: %.2f seconds/ %i milliseconds
	// Speed Bonus/Penalty: (+)%.2f
	// Additional tags
	// Fire mode
	weaponData_t* wp = cgImports->GetWeaponDatas(pItem->id->weaponData.weapon, pItem->id->weaponData.variation);

	vDescLines.push_back(UI_GetStringEdString2("@JKG_INVENTORY_ITYPE_WEAPON"));
	JKG_ConstructItemTierDescription(pItem->id->itemTier, vDescLines);
	if (pItem->id->weight > 0.0f)
	{
		vDescLines.push_back(va(UI_GetStringEdString2("@JKG_INVENTORY_ITEM_WEIGHT"), pItem->id->weight));
	}
	if (wp->hasRollAbility) {
		vDescLines.push_back(UI_GetStringEdString2("@JKG_INVENTORY_WEP_TAG_ROLLING"));
	}
	if (wp->hasCookAbility) {
		vDescLines.push_back(va(UI_GetStringEdString2("@JKG_INVENTORY_WEP_COOKTIME"), (float)(wp->weaponReloadTime / 1000.0f)));
	}
	else if (wp->weaponReloadTime > 1000) {
		vDescLines.push_back(va(UI_GetStringEdString2("@JKG_INVENTORY_WEP_RELOADTIME_SEC"), (float)(wp->weaponReloadTime / 1000.0f)));
	}
	else if (wp->weaponReloadTime > 0) {
		vDescLines.push_back(va(UI_GetStringEdString2("@JKG_INVENTORY_WEP_RELOADTIME"), wp->weaponReloadTime));
	}

	const float fSpeedModifier = wp->speedModifier;
	if (fSpeedModifier > 1.0f) {
		// Movement speed bonus
		float fMovementBonus = fSpeedModifier - 1.0f;
		fMovementBonus *= 100.0f;
		vDescLines.push_back(va(UI_GetStringEdString2("@JKG_INVENTORY_WEP_SPEEDBONUS"), fMovementBonus));
	}
	else if (fSpeedModifier < 1.0f) {
		// Movement speed penalty
		float fMovementPenalty = 1.0f - fSpeedModifier;
		fMovementPenalty *= 100.0f;
		vDescLines.push_back(va(UI_GetStringEdString2("@JKG_INVENTORY_WEP_SPEEDPENALTY"), fMovementPenalty));
	}

	vDescLines.push_back(""); // Push a blank line because we like nice formatting

	if (wp->weaponBaseIndex == WP_SABER) // FIXME: sabers don't really have a good item description yet
	{
		//const saberCrystalData_t* crystal = JKG_GetSaberCrystal(wp->sab.defaultcrystal); //--futuza JKG_GetSaberCrysta() has an empty table when called from here...wat?
		vDescLines.push_back(va("Hilt: %s", wp->sab.hiltname));
		if (wp->sab.defaultcrystal)
		{
			vDescLines.push_back(va("Crystal: %s", wp->sab.defaultcrystal));
		}
	}

	else
	{ 
		for (int i = 0; i < wp->numFiringModes; i++) {
			JKG_ConstructFiringModeDescription(wp, i, vDescLines);
		}
	}
}

//create an ammo item's description
static void JKG_ConstructAmmoDescription(itemInstance_t* pItem, std::vector<std::string>& vDescLines) {
	vDescLines.push_back(UI_GetStringEdString2("@JKG_INVENTORY_ITYPE_AMMO"));
	JKG_ConstructItemTierDescription(pItem->id->itemTier, vDescLines);
	if (pItem->id->weight > 0.0f)
	{
		vDescLines.push_back(va(UI_GetStringEdString2("@JKG_INVENTORY_ITEM_WEIGHT"), pItem->id->weight));
	}
}

// Create a tool item's description
static void JKG_ConstructToolDescription(itemInstance_t* pItem, std::vector<std::string>& vDescLines) {
	vDescLines.push_back(UI_GetStringEdString2("@JKG_INVENTORY_ITYPE_TOOL"));
	JKG_ConstructItemTierDescription(pItem->id->itemTier, vDescLines);
	if (pItem->id->weight > 0.0f)
	{
		vDescLines.push_back(va(UI_GetStringEdString2("@JKG_INVENTORY_ITEM_WEIGHT"), pItem->id->weight));
	}
}

// Create a consumable item's description
static void JKG_ConstructConsumableDescription(itemInstance_t* pItem, std::vector<std::string>& vDescLines) {
	vDescLines.push_back(UI_GetStringEdString2("@JKG_INVENTORY_ITYPE_CONSUMABLE"));
	JKG_ConstructItemTierDescription(pItem->id->itemTier, vDescLines);
	if (pItem->id->weight > 0.0f)
	{
		vDescLines.push_back(va(UI_GetStringEdString2("@JKG_INVENTORY_ITEM_WEIGHT"), pItem->id->weight));
	}
}

//a stupid simple line splitter for descriptions
void JKG_SplitDescriptionLines(const std::string& s, std::vector<std::string>& vDescLines)
{
	//these 'consts' might need to be calculated based on inventory width of the current screen
	const int MAXLENGTH = 39; //max length of a line
	const int MAXFIRSTLINE = 33; //max length with "info: " preceeding the text
	bool multiline = true;	//for determing if we need to handle multiple lines of description or a single line
	int length = s.length();
	
	if (length - MAXFIRSTLINE < MAXLENGTH)
	{
		vDescLines.push_back(s.c_str());
		multiline = false;
	}
	
	if(multiline)
	{
		//if the last part of the line is not a alphanumeric, don't add a dash
		if (!std::isalnum(s[MAXFIRSTLINE - 1]))
			vDescLines.push_back(va(UI_GetStringEdString2("@JKG_INVENTORY_ITEM_DESCRIPTION"), s.substr(0, MAXFIRSTLINE).c_str()));
		else
			vDescLines.push_back(va(UI_GetStringEdString2("@JKG_INVENTORY_ITEM_DESCRIPTION"), (s.substr(0, MAXFIRSTLINE) + "-").c_str()));

		length = length - MAXFIRSTLINE; //subtrack first line from length
		int start = MAXFIRSTLINE; //where to start the line

		//loop through the description and break it up into lines
		while (length > 0)
		{
			if( (length - MAXLENGTH) < 1)
			{
				vDescLines.push_back(s.substr(start, MAXLENGTH).c_str()); //if it is the last line
			}
			else
			{
				//no dash please, it's not a number or letter
				if (!std::isalnum(s[start+MAXLENGTH-1]))
					vDescLines.push_back(s.substr(start, MAXLENGTH).c_str()); 
				else
					vDescLines.push_back((s.substr(start, MAXLENGTH) + "-").c_str()); //add dash
			}
			
			start = start + MAXLENGTH;
			length = length - MAXLENGTH;
		}
	}
}

// Construct each type of item description
void JKG_ConstructItemDescription(itemInstance_t* pItem, std::vector<std::string>& vDescLines) {
	vDescLines.clear();

	if (pItem == nullptr) {
		return;
	}

	switch (pItem->id->itemType) {
		case ITEM_JETPACK:
			JKG_ConstructJetpackDescription(pItem, vDescLines);
			break;
		case ITEM_SHIELD:
			JKG_ConstructShieldDescription(pItem, vDescLines);
			break;
		case ITEM_ARMOR:
			JKG_ConstructArmorDescription(pItem, vDescLines);
			break;
		case ITEM_WEAPON:
			JKG_ConstructWeaponDescription(pItem, vDescLines);
			break;
		case ITEM_AMMO:
			JKG_ConstructAmmoDescription(pItem, vDescLines);
			break;
		case ITEM_TOOL:
			JKG_ConstructToolDescription(pItem, vDescLines);
			break;
		case ITEM_CONSUMABLE:
			JKG_ConstructConsumableDescription(pItem, vDescLines);
			break;
		default:
			JKG_ConstructItemTierDescription(pItem->id->itemTier, vDescLines);
			if (pItem->id->weight > 0.0f)
			{
				vDescLines.push_back(va(UI_GetStringEdString2("@JKG_INVENTORY_ITEM_WEIGHT"), pItem->id->weight));
			}
			break;
	}

	vDescLines.push_back("--"); // Push a blank line because we like nice formatting
	if(sizeof(pItem->id->itemDescription) > 35)
	{ 
		JKG_SplitDescriptionLines(std::string(pItem->id->itemDescription), vDescLines);
	}
	else
	{
		vDescLines.push_back(va(UI_GetStringEdString2("@JKG_INVENTORY_ITEM_DESCRIPTION"), pItem->id->itemDescription));
	}

	if (!pItem->id->tradeable)
	{
		vDescLines.push_back(va(UI_GetStringEdString2("@JKG_INVENTORY_ITEM_UNTRADEABLE")));
	}

	if (!pItem->id->droppable)
	{
		vDescLines.push_back(va(UI_GetStringEdString2("@JKG_INVENTORY_ITEM_UNDROPPABLE")));
	}
}

// Returns the string that should appear on nLineNum of an item description
const char* JKG_GetItemDescLine(itemInstance_t* pItem, int nLineNum) {
	if (nLineNum >= vItemDescLines.size() || nLineNum < 0) {
		return "";
	}
	return vItemDescLines[nLineNum].c_str();
}

/*
==========================
OWNERDRAW STUFF
==========================
*/

extern void Item_Text_Paint(itemDef_t *item);
extern void Item_Text_Paint(itemDef_t* item, vec4_t &color);

// Draws the "Credits: X" text
void JKG_Inventory_OwnerDraw_CreditsText(itemDef_t* item)
{
	int credits = *(int*)cgImports->InventoryDataRequest(INVENTORYREQUEST_CREDITS, -1);
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

	//figure out color based on item tier
	vec4_t color;
	JKG_SetTierColor(pItem->id->itemTier, color);
	Item_Text_Paint(item, color);
}

// Draws additional stuff below the item name
// Top line. OwnerDrawID is the item slot number
void JKG_Inventory_OwnerDraw_ItemTagTop(itemDef_t* item, int ownerDrawID) {
	int nItemNum = ownerDrawID + nPosition;

	memset(item->text, 0, sizeof(item->text));
	memcpy(item->window.foreColor, colorTable[CT_CYAN], sizeof(item->window.foreColor));

	if (nItemNum >= pItems.size() || nItemNum < 0) {
		Item_Text_Paint(item);
		return;
	}

	// If it's in an ACI slot, mention this
	int* pACI = (int*)cgImports->InventoryDataRequest(INVENTORYREQUEST_ACI, -1);
	assert(pACI != nullptr);
	for (int i = 0; i < MAX_ACI_SLOTS; i++) {
		if (pACI[i] == pItems[nItemNum].first) {
			Com_sprintf(item->text, sizeof(item->text), UI_GetStringEdString2("@JKG_INVENTORY_INACI"), i);
			Item_Text_Paint(item);
			return;
		}
	}

	// Got here, so check and see if it is equipped, or quantity is > 0
	itemInstance_t* pItem = pItems[nItemNum].second;
	if (pItem->equipped) {
		Q_strncpyz(item->text, UI_GetStringEdString2("@JKG_INVENTORY_EQUIPPED"), sizeof(item->text));
		Item_Text_Paint(item);
		return;
	}
	else if (pItem->id->maxStack > 1) {
		Com_sprintf(item->text, sizeof(item->text), UI_GetStringEdString2("@JKG_INVENTORY_QUANTITY"), pItem->quantity);
		Item_Text_Paint(item);
		return;
	}

	Item_Text_Paint(item);
}

// Bottom line. OwnerDrawID is the item slot number
// Only used if we have an item in the ACI that also has a quantity
void JKG_Inventory_OwnerDraw_ItemTagBottom(itemDef_t* item, int ownerDrawID) {
	int nItemNum = ownerDrawID + nPosition;
	qboolean inACI = qfalse;

	memset(item->text, 0, sizeof(item->text));
	memcpy(item->window.foreColor, colorTable[CT_CYAN], sizeof(item->window.foreColor));
	
	// Check for the item being valid
	if (nItemNum >= pItems.size() || nItemNum < 0) {
		Item_Text_Paint(item);
		return;
	}

	int* pACI = (int*)cgImports->InventoryDataRequest(INVENTORYREQUEST_ACI, -1);
	assert(pACI != nullptr);

	for (int i = 0; i < MAX_ACI_SLOTS; i++) {
		if (pACI[i] == pItems[nItemNum].first) {
			inACI = qtrue;
			break;
		}
	}

	if (!inACI) {
		// Can't be triggered (for now) unless it's also equipped
		Item_Text_Paint(item);
		return;
	}

	itemInstance_t* pItem = pItems[nItemNum].second;
	if (pItem->id->maxStack > 1) {
		Com_sprintf(item->text, sizeof(item->text), UI_GetStringEdString2("@JKG_INVENTORY_QUANTITY"), pItem->quantity);
		Item_Text_Paint(item);
		return;
	}
	Item_Text_Paint(item);
	return;
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

	//figure out color based on item tier
	vec4_t color;
	JKG_SetTierColor(pItem->id->itemTier, color);
	Item_Text_Paint(item, color);
}

// Draws the selected item's description (ownerDrawID being the line number)
void JKG_Inventory_OwnerDraw_SelItemDesc(itemDef_t* item, int ownerDrawID) {
	if (nSelected == -1 || nSelected > pItems.size()) {
		return;
	}
	itemInstance_t* pItem = pItems[nSelected].second;
	strcpy(item->text, JKG_GetItemDescLine(pItem, ownerDrawID));
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
		if (pItem->id->itemType == ITEM_CONSUMABLE) {
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
	int nWhich;
	if (!Int_Parse(args, &nWhich)) {
		return;
	}
	if (nPosition + nWhich >= pItems.size()) {
		nSelected = -1;
		Menu_ShowItemByName(Menus_FindByName("jkg_inventory"), "shop_preview", qfalse);
		
		JKG_ConstructItemDescription(nullptr, vItemDescLines);
	} else {
		nSelected = nPosition + nWhich;
		Menu_ShowItemByName(Menus_FindByName("jkg_inventory"), "shop_preview", qtrue);

		JKG_ConstructItemDescription(pItems[nSelected].second, vItemDescLines);
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
	int nSlot;
	if (!Int_Parse(args, &nSlot)) {
		return;
	}
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

// jkgscript to consume an item; takes one argument (the inventory ID). can be called from UI files
void JKG_Inventory_Use(char** args) {
	if (nSelected == -1 || nSelected > pItems.size()) {
		return;
	}
	cgImports->SendClientCommand(va("inventoryUse %d", pItems[nSelected].first));
}

// jkgscript to equip a piece of armor; takes one argument (the inventory ID). can be called from UI files
void JKG_Inventory_EquipArmor(char** args) {
	if (nSelected == -1 || nSelected > pItems.size()) {
		return;
	}
	cgImports->SendClientCommand(va("equip %d", pItems[nSelected].first));
}

// jkgscript to unequip a piece of armor; takes one argument (the inventory ID). can be called from UI files
void JKG_Inventory_UnequipArmor(char** args) {
	if (nSelected == -1 || nSelected > pItems.size()) {
		return;
	}
	cgImports->SendClientCommand(va("unequip %d", pItems[nSelected].first));
}

// 
void JKG_Inventory_Interact(char** args) {
	int nArg;
	if (!Int_Parse(args, &nArg)) {
		return;
	}
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
			JKG_Inventory_UpdateNotify(INVENTORYNOTIFY_UPDATE);
			break;
		case 1:
			// Does nothing / this is entirely handled by menu code
			break;
		case 2:
			// Use item
			JKG_Inventory_UpdateNotify(INVENTORYNOTIFY_UPDATE);
			break;
	}
}

// jkgscript to open the inventory (can be called from ui files)
void JKG_Inventory_Open(char** args) {
	JKG_Inventory_UpdateNotify(INVENTORYNOTIFY_OPEN);
}

// jkgscript to reconstruct the inventory list (can be called from ui files)
void JKG_Inventory_ReconstructList(char** args) {
	JKG_ConstructInventoryList();
}

// the filter cvar has been changed (probably from clicking on the filter button)
void JKG_UI_InventoryFilterChanged() {
	menuDef_t* focusedMenu = Menu_GetFocused();
	if (focusedMenu == nullptr) {
		return;
	}
	if (!Q_stricmp(focusedMenu->window.name, "jkg_inventory")) {
		JKG_ConstructInventoryList();
	}
	else if (!Q_stricmp(focusedMenu->window.name, "jkg_shop")) {
		JKG_ConstructShopLists();
	}
}

// received a message from cgame, handle it appropriately
void JKG_Inventory_UpdateNotify(jkgInventoryNotify_e msg) {
	menuDef_t* focusedMenu = Menu_GetFocused();

	switch (msg)
	{
	case 0: // open
		if (focusedMenu != nullptr && !Q_stricmp(focusedMenu->window.name, "jkg_inventory")) {
			Menus_CloseByName("jkg_inventory");
			return;
		}
		if (Menus_FindByName("jkg_inventory") && Menus_ActivateByName("jkg_inventory"))
		{
			trap->Key_SetCatcher(trap->Key_GetCatcher() | KEYCATCH_UI);
		}
		JKG_ConstructInventoryList();
		break;

	case 1: // update!
		JKG_ConstructInventoryList();
		break;
	}
}

// sends the currently selected item to the ACI
void JKG_Inventory_SendItemToACI(int nACIIndex)
{
	if (nSelected == -1 || nSelected >= pItems.size())
	{
		return;
	}

	itemInstance_t* pItem = pItems[nSelected].second;
	if (pItem->id->itemType == ITEM_JETPACK || pItem->id->itemType == ITEM_CONSUMABLE || pItem->id->itemType == ITEM_WEAPON || pItem->id->itemType == ITEM_SHIELD)
	{
		menuDef_t* menu = Menus_FindByName("jkg_inventory");

		// hide the interact submenu
		Menu_ShowGroup(menu, "interact1", qfalse);
		Menu_ShowGroup(menu, "interact_submenu1_assignaci", qfalse);
		Menu_ShowGroup(menu, "interact1z_hilight", qfalse);
		Menu_ItemDisable(menu, "inv_feederSel", qfalse);
		
		// actually send the thing to the ACI
		cgImports->InventoryAttachToACI(pItems[nSelected].first, nACIIndex, true);
	}
}

// returns qtrue if the key has been handled, qfalse otherwise
qboolean JKG_Inventory_HandleKey(int key)
{
	switch (key)
	{
		case A_MWHEELDOWN:
			JKG_Inventory_ArrowDown(nullptr);
			return qtrue;
		case A_MWHEELUP:
			JKG_Inventory_ArrowUp(nullptr);
			return qtrue;
		case A_0:
		case A_1:
		case A_2:
		case A_3:
		case A_4:
		case A_5:
		case A_6:
		case A_7:
		case A_8:
		case A_9:
			JKG_Inventory_SendItemToACI(key - A_0);
			return qtrue;
	}
	return qfalse;
}
