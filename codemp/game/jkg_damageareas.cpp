#include "jkg_damageareas.h"
#include "g_local.h"
#include "game/bg_weapons.h"
#include "qcommon/q_shared.h"

static std::vector<damageArea_t> damageAreas;

static float CalculateDamageRadius ( const damageArea_t *area )
{
    int t = level.time - area->startTime;
    const radiusParams_t *dmgParams = &area->data->radiusParams;
    float stop = 0.0f;
    
    switch ( dmgParams->radiusFunc )
    {
        case RF_CONSTANT:
            return dmgParams->startRadius;
        break;
        
        case RF_LINEAR:
            return dmgParams->startRadius + ((float)t / (float)area->data->lifetime) * (dmgParams->endRadius - dmgParams->startRadius);
        break;
        
        case RF_NONLINEAR:
            stop = (float)area->data->lifetime * (dmgParams->generic1 * 0.01f);
            if ( t <= stop )
            {
                return dmgParams->startRadius;
            }
            else
            {
                t -= stop;
                return dmgParams->startRadius + ((float)t / (float)(area->data->lifetime - stop)) * (dmgParams->endRadius - dmgParams->startRadius);
            }
        break;
        
        case RF_WAVE:
            // FIXME: I R BORKED.
            stop = sin ((double)(t * dmgParams->generic1));
            stop = stop * 0.5f + 0.5f;
            return dmgParams->startRadius + stop * (dmgParams->endRadius - dmgParams->startRadius);
        break;
        
        case RF_CLAMP:
            stop = (float)area->data->lifetime * (dmgParams->generic1 * 0.01f);
            if ( t >= stop )
            {
                return dmgParams->endRadius;
            }
            else
            {
                return dmgParams->startRadius + ((float)t / stop) * (dmgParams->endRadius - dmgParams->startRadius);
            }
        break;
    }
    
    return 0.0f;
}

static float gaussian ( float x )
{
    // mean = 0, sigma squared = 0.5
    return expf (-2.0f * x * x) / sqrt (0.5f * M_PI);
}

static void SmallestVectorToBBox ( vec3_t out, const vec3_t position, const vec3_t absmin, const vec3_t absmax )
{
    int k;
    for ( k = 0; k < 3; k++ )
    {
        if ( position[k] < absmin[k] ) {
	        out[k] = absmin[k] - position[k];
        } else if ( position[k] > absmax[k] ) {
	        out[k] = position[k] - absmax[k];
        } else {
	        out[k] = 0.0f;
        }
    }
}

static int CalculateDamageForDistance ( const damageArea_t *area, const vec3_t playerMins, const vec3_t playerMaxs, const vec3_t playerOrigin, float damageRadius )
{
    int d; // = area->data->damage;
    vec3_t v;
    float distanceFromOrigin;
    float f = 0.0f;

	if(area->context.damageOverride != 0 && area->context.damageOverride != area->data->damage) {
		d = area->context.damageOverride;
	}
	else {
		d = area->data->damage;
	}

	if (area->context.attacker && area->context.ammoType > 0)
	{
		JKG_ApplyAmmoOverride(d, ammoTable[area->context.ammoType].overrides.damage);
	}
    
    SmallestVectorToBBox (v, area->origin, playerMins, playerMaxs);
    distanceFromOrigin = VectorLength (v);
    
    if ( distanceFromOrigin > damageRadius )
    {
        return 0;
    }
    
    switch ( area->data->radiusParams.damageFunc )
    {
        case DF_CONSTANT:
            return d;
        break;
        
        case DF_GAUSSIAN: // change to sigmoid?
            f = gaussian (distanceFromOrigin / damageRadius);
            f = Q_max (0.0f, Q_min (f, 1.0f));
            return d * f;
        break;
        
        case DF_LINEAR:
            return d * (1.0f - (distanceFromOrigin / damageRadius));
        break;
    }
    
    return d;
}

/*
 *	Removes a buff from a character
 */
void G_RemoveBuff(gentity_t* ent, int index)
{
	jkgBuff_t* pBuff = &buffTable[ent->client->ps.buffs[index].buffID];

	ent->client->ps.buffsActive &= ~(1 << index);
	if (pBuff->passive.overridePmoveType.first)
	{
		if (pBuff->passive.overridePmoveType.second == PM_FREEZE)
		{
			ent->client->pmlock = ent->client->pmfreeze = qfalse;
		}

		if (pBuff->passive.overridePmoveType.second == PM_LOCK)
		{
			ent->client->pmlock = qfalse;
		}

		if (pBuff->passive.overridePmoveType.second == PM_NOMOVE)
		{
			ent->client->pmnomove = qfalse;
		}
	}
	pBuff->passive.stacks = 0;
	pBuff->passive.movemodifier_cur = 1.0;
	pBuff->remove_f = false; //reset flag
}

/*
 *	Adds a buff to a character
 */
void G_BuffEntity(gentity_t* ent, gentity_t* buffer, int buffID, float intensity, int duration)
{
	int i;
	jkgBuff_t* pBuff;

	if (!ent->client)
	{
		// can only be used on clients for now
		return;
	}

	if (buffID < 0)
	{	// not a valid buff
		return;
	}

	pBuff = &buffTable[buffID];
	
	// Try and cancel out any existing buffs first, this will clear out room for the new buff ideally
	for (i = 0; i < PLAYERBUFF_BITS; i++)
	{

		if (ent->client->ps.buffsActive & (1 << i))
		{
			jkgBuff_t* pOtherBuff = &buffTable[ent->client->ps.buffs[i].buffID];

			// this buff is active
			if (pBuff->cancel.noBuffStack)
			{
				// don't allow buffs of the same kind to stack
				if (ent->client->ps.buffs[i].buffID == buffID)
				{
					// extend the duration and change the buffer
					ent->buffData[i].buffer = buffer;
					ent->buffData[i].endTime = level.time + duration;
					return;
				}
			}

			if (pBuff->cancel.noCategoryStack)
			{
				// don't allow buffs in the same category to stack
				if (!Q_stricmp(pOtherBuff->category, pBuff->category))
				{
					if (pOtherBuff->cancel.priority < pBuff->cancel.priority)
					{
						// cancel the other buff and put this one on instead
						G_RemoveBuff(ent, i);
					}
					else
					{
						return; // we can't apply this buff to the player
					}
				}
			}

			if (pBuff->cancel.other.size() > 0)
			{
				// try to cancel out buffs in other categories
				for (auto it = pBuff->cancel.other.begin(); it != pBuff->cancel.other.end(); ++it)
				{
					if (!Q_stricmp(pOtherBuff->category, it->category) && pOtherBuff->cancel.priority < it->priority)
					{
						G_RemoveBuff(ent, i);
					}
				}
			}
		}
	}

	// Now that some space has been cleared out, we need to apply this new buff
	for (i = 0; i < PLAYERBUFF_BITS; i++)
	{
		if (!(ent->client->ps.buffsActive & (1 << i)))
		{
			// this slot is not using anything
			ent->client->ps.buffsActive |= (1 << i);
			ent->client->ps.buffs[i].buffID = buffID;
			ent->client->ps.buffs[i].intensity = intensity;
			ent->buffData[i].buffer = buffer;
			ent->buffData[i].endTime = level.time + duration;
			ent->buffData[i].lastDamageTime = level.time;

			//check if having a shield blocks the buff
			if (pBuff->cancel.shieldRemoval && ent->client->ps.stats[STAT_SHIELD] > 0)
			{
				// Play the shield hit effect
				gentity_t* evEnt;
				evEnt = G_TempEntity(ent->client->ps.origin, EV_SHIELD_HIT);
				evEnt->s.otherEntityNum = ent->s.number;
				evEnt->s.eventParm = DirToByte(ent->client->ps.viewangles);
				evEnt->s.time2 = 10; //strength
				G_RemoveBuff(ent, i);  //--futuza: remove it?  could also just not apply the effects, but leave the debuff on the player...
				return;
			}

			//****
			//
			//	Don't forget to undo anything you add below in the G_RemoveBuff() function.
			//
			//****
			pBuff->remove_f = false; //should already be set, but just in case
			
			//override movement type
			if (pBuff->passive.overridePmoveType.first)
			{
				if (pBuff->passive.overridePmoveType.second == PM_FREEZE) //freeze in place no movement - note this includes a pmlock as well
				{
					// clear out the velocity vector only if we are not in midair
					if (ent->client->ps.groundEntityNum != ENTITYNUM_NONE)
					{
						VectorClear(ent->client->ps.velocity);
					}
					ent->client->ps.freezeLegsAnim = ent->client->ps.legsAnim;
					ent->client->ps.freezeTorsoAnim = ent->client->ps.torsoAnim;
					ent->client->pmlock = ent->client->pmfreeze = qtrue;
				}
				else if (pBuff->passive.overridePmoveType.second == PM_NOMOVE) //can't move legs, but can twist/aim
				{
					// clear out the velocity vector only if we are not in midair
					if (ent->client->ps.groundEntityNum != ENTITYNUM_NONE)
					{
						VectorClear(ent->client->ps.velocity);
					}
					ent->client->ps.freezeLegsAnim = ent->client->ps.legsAnim;
					ent->client->pmnomove = qtrue;
				}
				else if (pBuff->passive.overridePmoveType.second == PM_LOCK) 
				{
					//need more animation freeze here too?
					ent->client->pmlock = qtrue;
				}
			}

			//do a knockback if applicable
			if (pBuff->passive.knockdown)
			{
				G_Knockdown(ent, buffer, buffer->client->ps.origin, 50.0, qtrue);
			}

			//override movement speed
			if (pBuff->passive.maxstacks && pBuff->passive.movemodifier != 1.0)
			{
				if (pBuff->passive.maxstacks > pBuff->passive.stacks) //if we can fit more stacks, increase the effect
				{
					if (pBuff->passive.stacks == 0)
					{
						pBuff->passive.movemodifier_cur = pBuff->passive.movemodifier; //initialize first stack
					}

					//add/subtract additional stacks of movement effects
					else
					{
						if (pBuff->passive.movemodifier > 1)
						{
							pBuff->passive.movemodifier_cur += (pBuff->passive.movemodifier - 1.0);		//caution: could go potentially infinite speed
						}

						if (pBuff->passive.movemodifier < 1 && pBuff->passive.movemodifier > 0)
						{
							pBuff->passive.movemodifier_cur -= (1.0 - pBuff->passive.movemodifier);
							pBuff->passive.movemodifier_cur < 0 ? pBuff->passive.movemodifier_cur = 0 : 0; //can't go slower than 0
						}
					}
					pBuff->passive.stacks++;
				}
			}

			if (pBuff->passive.empstaggered)
			{
				if (ent->client->ps.eFlags & EF_JETPACK_ACTIVE)
					Jetpack_Off(ent);

				//todo: tell client to display jetpack is disabled icon
			}

			return;
		}
	}
}

int JKG_AdjustAmbientHeatDamage(bool heat, int damage)
{
	//heat == 1 is fire debuff, 0==cold debuff

	//don't waste time here, if we're already at default heat
	if (jkg_heatDissipateTime.integer == 100)
	{
		return damage;
	}

	enum categories { verycold = -2, cold = -1, temperate = 0, hot = 1, veryhot = 2 };  //categories of tempature
	int temp = 0;

	float percent = (jkg_heatDissipateTime.integer > 0 ? static_cast<float>(jkg_heatDissipateTime.integer) : 1.0f) / 100.0f;
	if (percent > 2.0f)
		percent = 2.0f;		//max cap

	if (percent < 0.01f)
		percent = 0.01f;	//low cap

	//determine ranges of categories
	//very cold
	if (percent <= 0.5f)
	{
		temp = verycold;
	}

	//cold
	else if (percent > 0.5 && percent < 0.9f)
	{
		temp = cold;
	}

	//temperate
	else if (percent >= 0.9 && percent <= 1.1)
	{
		temp = temperate;
	}

	//hot
	else if (percent > 1.1 && percent <= 1.5f)
	{
		temp = hot;
	}

	//very hot
	else //(percent > 1.5f)
	{
		temp = veryhot;
	}

	switch (temp)
	{
		case verycold:
			if (heat)
				damage -= 2;
			else
				damage += 2;
			break;

		case cold:
			if (heat)
				damage -= 1;
			else
				damage += 1;
			break;

		case temperate:
			break;

		case hot:
			if (heat)
				damage += 1;
			else
				damage -= 1;
			break;

		case veryhot:
			if (heat)
				damage += 2;
			else
				damage -= 2;
			break;

		default:
			break;
	}

	if (damage < 1)
		damage = 1;	//it should always do at least 1

	return damage;
}

/*
 *	Ticks all of the buffs on this entity and sees if they should be removed or if they should do damage
 */
void G_TickBuffs(gentity_t* ent)
{
	if (!JKG_ClientAlive(ent))
	{
		// if we are dead, we have no buffs active
		ent->client->ps.buffsActive = 0;
		return;
	}

	for (int i = 0; i < PLAYERBUFF_BITS; i++)
	{
		// check to see if it's active
		if (ent->client->ps.buffsActive & (1 << i))
		{
			jkgBuff_t* pBuff = &buffTable[ent->client->ps.buffs[i].buffID];
			// check for removal
			if (ent->buffData[i].endTime < level.time)
			{
				G_RemoveBuff(ent, i);
				continue;
			}

			// check for removal
			if (pBuff->remove_f)
			{
				G_RemoveBuff(ent, i);
				continue;
			}

			// check for dealing damage
			if (ent->buffData[i].lastDamageTime + pBuff->damage.damageRate <= level.time)
			{
				int damage = pBuff->damage.damage;
				int health = ent->client->ps.stats[STAT_HEALTH];

				//if % damage
				if ( pBuff->damage.percentage && ((0 < damage && damage < 101) || (damage < 0 && damage > -101)) )	//and the range is between 1% to 100% or -1% to -100%
				{
					int dmg = damage;
					damage = health * (damage * .01);
					if (dmg > 0)
						damage > 1 ? damage : damage = 1;	//do at least 1 damage
					else
						damage < 0 ? damage : damage = -1;
				}
				
				//adjust damage based on ambient tempature for fire
				if (!Q_stricmp(pBuff->category, "fire"))
				{
					damage = JKG_AdjustAmbientHeatDamage(1, damage);
				}

				//adjust damage based on ambient tempature for cold
				if (!Q_stricmp(pBuff->category, "cold"))
				{
					damage = JKG_AdjustAmbientHeatDamage(0, damage);
				}

				//check if debuff is doing damage
				if (damage > 0)
				{
					//if jkg_allowDebuffKills == 0 (or less), we have carebear debuffs enabled
					if (jkg_allowDebuffKills.integer < 1)  
					{
						//only allow debuffs to whittle us down, not kill us
						if (health - damage <= 0)
						{
							damage = 0;
						}
					}

					//jkg_allowDebuffKills allows debuffs to finish off targets
					else
					{
						//debuffs are deadly, depending on the wpn (default behavior)
						if (jkg_allowDebuffKills.integer == 1)
						{
							if (pBuff->damage.deadly == false)	//if debuff type isn't deadly, carebear treatment
							{
								if (health - damage <= 0)
									damage = 0;
							}
						}

						//all damaging debuffs are deadly, don't adjust damage
						else
							;
					}
				}

				G_Damage(ent, ent->buffData[i].buffer, ent->buffData[i].buffer, vec3_origin, ent->client->ps.origin, 
					damage, DAMAGE_NO_KNOCKBACK | DAMAGE_NO_DISMEMBER | DAMAGE_NO_HIT_LOC, pBuff->damage.meansOfDeath);
				ent->buffData[i].lastDamageTime = level.time;
			}
		}
		
	}
}

/*
 *	Applies debuffs as given to the player with damage areas
 */
static void DebuffPlayer( gentity_t *player, const damageArea_t *area, int damage, int mod )
{
    int i = 0;
	debuffData_t debuffs[MAX_DEBUFFS_PRESENT];
	int numDebuffs;
	int ammoType;
    
    if ( !player || !player->client )
    {
        return;
    }

	if (!JKG_ClientAlive(player)) {	// Don't allow us to be debuffed if we are dead
		return;
	}

	if (player->flags & FL_NO_DEBUFF)  //check for no debuff flag
	{
		return;
	}

	if (mod > 0 && mod < allMeansOfDamage.size())
	{
		meansOfDamage_t means = allMeansOfDamage.at(mod);
		if (means.modifiers.shieldBlocks && player && player->client && player->client->ps.stats[STAT_SHIELD] > 0)
		{	// Don't debuff them if shield blocks this.
			return;
		}
	}

	// Do ammo type override for debuffs
	memcpy(debuffs, area->data->debuffs, sizeof(debuffs));
	numDebuffs = area->data->numberDebuffs;
	ammoType = area->context.ammoType;
	if (area->context.attacker->client && ammoType >= 0)
	{
		// valid ammo type, check for buff overrides
		if (ammoTable[ammoType].overrides.buffs.size() > 0)
		{
			// do REMOVE buffs first because those might clear up a blank space
			for (auto it = ammoTable[ammoType].overrides.buffs.begin();
				it != ammoTable[ammoType].overrides.buffs.end();
				++it)
			{
				if (it->bRemove)
				{
					// see if our debuffs includes this buff
					for (int i = 0; i < numDebuffs; i++)
					{
						if (debuffs[i].debuff == it->buff)
						{
							debuffs[i].debuff = -1; // remove it!
						}
					}
				}
			}

			// do ADD buffs next
			for (auto it = ammoTable[ammoType].overrides.buffs.begin();
				it != ammoTable[ammoType].overrides.buffs.end();
				++it)
			{
				if (it->bAddBuff)
				{
					// Check to see if this buff already exists
					qboolean bBuffAlreadyExists = qfalse;
					int slot = 0;

					for (int i = 0; i < numDebuffs; i++)
					{
						if (debuffs[i].debuff == it->buff)
						{
							bBuffAlreadyExists = qtrue;
							break;
						}
						else if (i >= numDebuffs || debuffs[i].debuff == -1)
						{
							// this slot is free.
							slot = i;
							break;
						}
					}

					if (!bBuffAlreadyExists && slot < MAX_DEBUFFS_PRESENT)
					{
						debuffs[slot].debuff = it->buff;
						debuffs[slot].intensity = 1.0f; // default intensity to 1

						if (slot >= numDebuffs)
						{
							numDebuffs++;
						}
					}
				}
			}

			// do SET, MULTIPLY, ADD operations
			for (auto it = ammoTable[ammoType].overrides.buffs.begin();
				it != ammoTable[ammoType].overrides.buffs.end();
				++it)
			{
				// Iterate through all buffs
				for (int i = 0; i < numDebuffs; i++)
				{
					if (debuffs[i].debuff == it->buff)
					{
						if (it->bAddDuration)
						{
							debuffs[i].duration += it->addDuration;
						}

						if (it->bAddIntensity)
						{
							debuffs[i].intensity += it->addIntensity;
						}

						if (it->bMultiplyDuration)
						{
							debuffs[i].duration *= it->multiplyDuration;
						}

						if (it->bMultiplyIntensity)
						{
							debuffs[i].intensity *= it->multiplyIntensity;
						}

						if (it->bSetDuration)
						{
							debuffs[i].duration = it->setDuration;
						}

						if (it->bSetIntensity)
						{
							debuffs[i].intensity = it->setIntensity;
						}
					}
				}
			}
		}
	}

	for (i = 0; i < numDebuffs; i++)
	{
		G_BuffEntity(player, area->context.attacker, 
			debuffs[i].debuff, debuffs[i].intensity, debuffs[i].duration);
	}
}

/*
 *	Finds a free damage area to work on.
 *	Really lame old relic from how this code used to work.
 */
static damageArea_t *GetFreeDamageArea()
{
	damageArea_t area;
	damageAreas.push_back(area);
    
    return &damageAreas.back();
}

static void DamageEntityAndDebuffPlayer(
	gentity_t *ent,
	const damageArea_t *area,
	vec3_t dir,
	int damage
) {
	G_Damage(
		ent,
		area->context.inflictor,
		area->context.attacker,
		dir,
		ent->r.currentOrigin,
		damage,
		area->context.damageFlags,
		area->context.methodOfDeath);
	if (ent->client) {
		DebuffPlayer(ent, area, damage, area->context.methodOfDeath);
	}
}

/*
 * Damages players in the damage area. Returns qtrue if the area has decayed and we need to remove it.
 */
static void DamageEntitiesInArea(damageArea_t *area) {
	if (area->startTime > level.time) {
		// Delayed start. Doing do anything yet.
		return;
	}

	if ((area->startTime + area->data->lifetime) < level.time) {
		// Area has decayed, set as inactive.
		return;
	}

	if ((area->lastDamageTime + area->data->damageDelay) > level.time) {
		// Too soon to try to damage players again.
		return;
	}

	const float damageRadius = CalculateDamageRadius(area);
	const float damageRadiusSquared = damageRadius * damageRadius;

	vec3_t areaMins, areaMaxs;
	for (int j = 0; j < 3; j++) {
		areaMins[j] = area->origin[j] - damageRadius;
		areaMaxs[j] = area->origin[j] + damageRadius;
	}

	int entList[MAX_GENTITIES] = {0};
	int numEnts = 0;
	numEnts = trap->EntitiesInBox(areaMins, areaMaxs, entList, MAX_GENTITIES);
	for (int j = 0; j < numEnts; j++) {
		vec3_t v;

		gentity_t *ent = &g_entities[entList[j]];
		if (!ent->inuse) {
			continue;
		}

		if (!ent->takedamage || ent == area->context.ignoreEnt) {
			continue;
		}

		if (ent->health <= 0) {
			continue;
		}

		// Check to make sure the player is within the radius.
		SmallestVectorToBBox(v, area->origin, ent->r.absmin, ent->r.absmax);
		if (VectorLengthSquared(v) > damageRadiusSquared) {
			continue;
		}

		vec3_t entOrigin;
		VectorCopy(ent->r.currentOrigin, entOrigin);
		if (area->data->penetrationType != PT_WALLS) {
			trace_t tr;
			trap->Trace(&tr, area->origin, NULL, NULL, entOrigin, -1, CONTENTS_SOLID, 0, 0, 0);
			if (tr.fraction != 1.0f) {
				continue;
			}
		}

		// Apply the damage and its effects.
		const int damage = CalculateDamageForDistance(area, ent->r.absmin, ent->r.absmax, entOrigin, damageRadius);
		vec3_t dir;
		VectorSubtract(entOrigin, area->origin, dir);
		VectorNormalize(dir);

		DamageEntityAndDebuffPlayer(ent, area, dir, damage);
	}

	area->lastDamageTime = level.time;
}

//=========================================================
// JKG_ChargeDamageOverride
//---------------------------------------------------------
// Description: Calculates damage override, which is 
// often modified as a result of a charging weapon
//=========================================================
int JKG_ChargeDamageOverride( gentity_t *inflictor, bool bIsTraceline ) {
	int damage = 0;
	if(inflictor->s.generic1 != 0 || bIsTraceline) {
		weaponData_t *wp = GetWeaponData(inflictor->s.weapon, inflictor->s.weaponVariation);
		int firemode = inflictor->s.firingMode;
		if(wp->firemodes[firemode].chargeTime) {
			if(bIsTraceline) {
				damage = WP_GetWeaponDamage(inflictor, inflictor->s.firingMode);
			}
			else {
				int current = ( level.time - inflictor->s.generic1 ) / wp->firemodes[firemode].chargeTime;
				float maximum = wp->firemodes[firemode].chargeMaximum / wp->firemodes[firemode].chargeTime;

				if ( current > maximum ) current = maximum;

				damage = wp->firemodes[firemode].baseDamage * current * wp->firemodes[firemode].chargeMultiplier;
				if(damage < wp->firemodes[firemode].baseDamage) {
					damage = wp->firemodes[firemode].baseDamage;
				}
			}
		}
	}
	return damage;
}

//=========================================================
// JKG_DoDirectDamage
//---------------------------------------------------------
// Description: This is a wrapper for the G_Damage
// function, which also does debuffs. It does _not_ create
// damage areas. It only does direct damage like with
// G_Damage.   
//=========================================================
static void JKG_DoDirectDamage(
    damageSettings_t *data,
    gentity_t *targ,
    gentity_t *inflictor,
    gentity_t *attacker,
    vec3_t dir,
    vec3_t origin,
    int dflags,
    int mod,
    const damageDecay_t *decay)
{
	damageArea_t area;
	int damage;

	if (!targ->takedamage) {
		return;
	}

	if (targ->health <= 0) {
		return;
	}

	memset(&area, 0, sizeof(area));

	area.data = data;

	// The firing mode's base damage can lie! It doesn't account for dynamic damage amounts (ie weapon charging)
	area.context.damageOverride = JKG_ChargeDamageOverride(inflictor, inflictor == attacker);
	if (area.context.damageOverride != 0 && area.data->damage != area.context.damageOverride) {
		damage = area.context.damageOverride;
	} else {
		damage = data->damage;
	}

	if (decay != nullptr) {
		if (decay->maxRange > decay->recommendedRange &&
			decay->distanceToDamageOrigin < decay->maxRange) {
			/* Fired at a distance beyond recommended range. */
			damage = JKG_CalculateDamageDecay(
				damage,
				decay->distanceToDamageOrigin,
				decay->recommendedRange,
				decay->decayRate);
		}
	}

	VectorCopy(dir, area.context.direction);
	if (attacker->client && attacker->client->ps.ammoType) {
		area.context.ammoType = attacker->client->ps.ammoType;
		JKG_ApplyAmmoOverride(damage, ammoTable[area.context.ammoType].overrides.damage);
	} else {
		area.context.ammoType = -1;
	}

	area.context.ignoreEnt = NULL;
	area.context.attacker = attacker;
	area.context.damageFlags = dflags;
	area.context.inflictor = inflictor;
	area.context.methodOfDeath = mod;
	area.startTime = level.time;
	area.lastDamageTime = 0;
	VectorCopy(origin, area.origin);

	DamageEntityAndDebuffPlayer(targ, &area, dir, damage);
}

void JKG_DoDirectDamage(weaponFireModeStats_t* fireMode, gentity_t *targ, gentity_t *inflictor, gentity_t *attacker, vec3_t dir, vec3_t origin, int dflags, int mod, const damageDecay_t *decay)
{
	JKG_DoDirectDamage(&fireMode->primary, targ, inflictor, attacker, dir, origin, dflags, mod, decay);
	if (fireMode->secondaryDmgPresent) {
		JKG_DoDirectDamage(&fireMode->secondary, targ, inflictor, attacker, dir, origin, dflags, mod, decay);
	}
}

// This is a _replacement_ function for G_RadiusDamage. It does all the same
// things, in addition to the debuffs.
void JKG_DoSplashDamage( damageSettings_t* data, const vec3_t origin, gentity_t *inflictor, gentity_t *attacker, gentity_t *ignoreEnt, int mod )
{
    if ( !data->radial )
    {
        return;
    }
    
	const bool bDoDamageOverride = (inflictor != attacker); 
    if ( data->lifetime )
    {
        damageArea_t *area = GetFreeDamageArea();
        if ( !area )
        {
            return;
        }

        if ( !data->planar )
        {
            area->data = data;
            area->lastDamageTime = 0;
            VectorCopy (origin, area->origin);
            VectorClear (area->context.direction);
            area->context.ignoreEnt = ignoreEnt;
            area->context.attacker = attacker;
            area->context.damageFlags = data->damageFlags;
            area->context.inflictor = inflictor;
            area->context.methodOfDeath = mod;
			area->context.ammoType = inflictor->s.ammoType;
            area->startTime = level.time + data->delay;

			if(bDoDamageOverride) {
				area->context.damageOverride = JKG_ChargeDamageOverride(inflictor, false);
			}
			else {
				area->context.damageOverride = area->data->damage;
			}
        }
        else
        {
            // check nearest surface. If can't find one, then don't create an area..
        }
    }
    else
    {
        // This is similar to the old style splash damage
        damageArea_t a = {};
        a.data = data;
        a.lastDamageTime = 0;
        VectorCopy (origin, a.origin);
        VectorClear (a.context.direction);
        a.context.ignoreEnt = ignoreEnt;
        a.context.attacker = attacker;
        a.context.damageFlags = a.data->damageFlags;
        a.context.inflictor = inflictor;
        a.context.methodOfDeath = mod;
        a.startTime = level.time;
		a.context.ammoType = inflictor->s.ammoType;

		if (attacker->client)
		{
			a.context.ammoType = attacker->client->ps.ammoType;
		}

		if(bDoDamageOverride) {
			a.context.damageOverride = JKG_ChargeDamageOverride(inflictor, false);
		}
		else {
			a.context.damageOverride = a.data->damage;
		}
        
        DamageEntitiesInArea(&a);
    }
}

//=========================================================
// JKG_DoDamage
//---------------------------------------------------------
// Description: This is a wrapper for the G_Damage
// function, which also does debuffs and stuff. Might
// create a damage area instead, if the handle does
// splash damage.
//=========================================================
static void JKG_DoDamage(
	damageSettings_t* data,
	gentity_t *targ,
	gentity_t *inflictor,
	gentity_t *attacker,
	vec3_t dir,
	vec3_t origin,
	int dflags,
	int mod,
	const damageDecay_t* decay)
{
	if (data->radial) {
		JKG_DoSplashDamage(data, origin, inflictor, attacker, NULL, mod);
	}

	JKG_DoDirectDamage(data, targ, inflictor, attacker, dir, origin, dflags, mod, decay);
}

void JKG_DoDamage(
	weaponFireModeStats_t *fireMode,
	gentity_t *targ,
	gentity_t *inflictor,
	gentity_t *attacker,
	vec3_t dir,
	vec3_t origin,
	int dflags,
	int mod,
	const damageDecay_t *decay)
{
	JKG_DoDamage(&fireMode->primary, targ, inflictor, attacker, dir, origin, dflags, mod, decay);
	if (fireMode->secondaryDmgPresent) {
		JKG_DoDamage(&fireMode->secondary, targ, inflictor, attacker, dir, origin, dflags, mod, decay);
	}
}

//=========================================================
// JKG_DamagePlayers
//---------------------------------------------------------
// Description: To be called on every server frame update.
// This goes through all active areas and deals damage to
// players within those areas.
//=========================================================
void JKG_DamagePlayers(void) {
	for (auto &area : damageAreas) {
		DamageEntitiesInArea(&area);
	}
}

/**************************************************
* WP_CalculateDamageDecay
*
* Calculates how much damage to do if a shot exceeds
* the weapons maximum range (exponetial decay).
* Note that if the distance exceeds 10x the range
* we don't bother with the decay function and just
* reduce the damage to 0 nothing.  Can also handle
* negative (healing) damage.
**************************************************/

//how much damage a shot should do if it exceeds its range
int JKG_CalculateDamageDecay(int damage, float distance, float range, float decayRate)
{
	if (damage == 0)
		return damage;

	//flip dmg for heals
	bool heal = false;
	if (damage < 0)
	{
		heal = true;
		damage = -damage; //flip
	}

	if (distance <= range)
		return damage;

	if (decayRate <= 0.0)
		return 0;

	//decay does not effect this weapon warn devs they probably did something dumb
	if (decayRate >= 1.0)
	{
#ifdef _DEBUG
		Com_Printf("WARNING: Weapon damage not affected over distance! Invalid decayRate?");
#endif
		return damage;
	}

	//clamp extreme values
	if (distance >= range * MAX_RANGE_MULTIPLIER)  //if we're firing this weapon 10x its range, we've gone too far
		return 0;

	if (decayRate < 0.0001)
		decayRate = 0.0001;

	/*Okay we're good to engage*/
	float relDistance = static_cast<float>((distance - range) / range);  //determine how far, percentage wise the projectile has travelled.
	if (relDistance < 0.0001)
		relDistance = 0.0001;   //clamp

	damage = static_cast<int>(damage * std::pow(decayRate, relDistance));  //eg y = 5*(0.25)^1 == 1.25dmg

	//clamp
	if (damage > 999)
		damage = 999;

	//flip damage
	if (heal)
		return -damage;

	else
		return damage;
}