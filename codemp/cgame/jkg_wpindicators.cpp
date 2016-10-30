/////////////////////////////////
//
// JKG Weapon Indicators
// 
// By BobaFett
//
/////////////////////////////////

#include "cg_local.h"

static void DissectNumber(int number, int *hundreds, int *tens, int *ones) {
	int h, t, o;
	if (number < 0) {
		number *= -1;	// We cant handle negative numbers here
	}
	h = floor((double)number/100);
	number -= (h*100);
	if (h>9) {
		h=9;
	}
	t = floor((double)number/10);
	o = number - (t*10);

	*hundreds = h;
	*tens = t;
	*ones = o;
}

static __inline qboolean PlayerIsReloading()
{
    return (qboolean)(cg.reloadTimeStart && cg.time <= (cg.reloadTimeStart + cg.reloadTimeDuration));
}

void JKG_WeaponIndicators_Update(const centity_t *cent, const playerState_t *ps) {
	int h,t,o;
	int ammo;
	weaponInfo_t *weaponInfo = CG_WeaponInfo (cent->currentState.weapon, cent->currentState.weaponVariation);
	weaponData_t *weaponData = GetWeaponData (cent->currentState.weapon, cent->currentState.weaponVariation);

	if (!ps) {
		// Check if it's me anyway
		if (cent->currentState.number == cg.predictedPlayerState.clientNum) {
			ps = &cg.predictedPlayerState;
		}
	}

	if (ps) {
		if ( GetWeaponAmmoClip( cent->currentState.weapon, cent->currentState.weaponVariation ))
		{
			ammo = ps->stats[STAT_AMMO];
		}
		else
		{
			ammo = ps->ammo;
		}

	} else {
		ammo = 0;
	}

	if ( ammo )
	{
		if ( weaponData->firemodes[ps->firingMode].cost > 0 )
		{
			ammo /= weaponData->firemodes[ps->firingMode].cost;
		}
	}

	if (ps) {	// This is my weapon
        switch ( weaponData->visuals.indicatorType )
        {
			default:
            case IND_NORMAL:
				if ( weaponData->visuals.visualFireModes[ps->firingMode].overrideIndicatorFrame != -1 )
				{
					trap->R_OverrideShaderFrame (weaponInfo->fireModeIndicator, weaponData->visuals.visualFireModes[ps->firingMode].overrideIndicatorFrame, cg.time);
				}
				else
				{
					trap->R_OverrideShaderFrame (weaponInfo->fireModeIndicator, ps->firingMode, cg.time);
				}

                if ( PlayerIsReloading() )
                {
                    trap->R_OverrideShaderFrame (weaponInfo->groupedIndicators[0], 11, cg.time);
                    trap->R_OverrideShaderFrame (weaponInfo->groupedIndicators[1], 11, cg.time);
                    trap->R_OverrideShaderFrame (weaponInfo->groupedIndicators[2], 11, cg.time);
                }
                else
                {
                    DissectNumber (ammo, &h, &t, &o);
                    trap->R_OverrideShaderFrame (weaponInfo->groupedIndicators[0], o, cg.time);
                    trap->R_OverrideShaderFrame (weaponInfo->groupedIndicators[1], (!t && !h) ? 10 : t, cg.time);
                    trap->R_OverrideShaderFrame (weaponInfo->groupedIndicators[2], !h ? 10 : h, cg.time);
                }
                break;
                
            case IND_GRENADE:
                if ( cg.jkg_grenadeCookTimer >= cg.time )
			    {
			        int i = 0, j = 0;
			        int led1 = 0;
			        int led2 = 0;
			        int led3 = 0;
			        int dt = cg.jkg_grenadeCookTimer - cg.time - 1000;
			        
			        trap->R_OverrideShaderFrame (weaponInfo->fireModeIndicator, 1, cg.time);
			        
			        if ( dt <= 0 )
			        {
			            led1 = led2 = led3 = 1;
			        }
			        else
			        {
			            int detonationTime = GetWeaponData (cg.snap->ps.weapon, cg.snap->ps.weaponVariation)->weaponReloadTime;
			            int interval;
			        
			            while ( i < dt )
			            {
			                interval = 600 * (1.0f - ((float)(detonationTime - i) / (float)detonationTime));
			                if ( interval < 50 )
			                    interval = 50;
    			        
			                i += interval;
			                j = (j + 1) % 4;
			            }
    			        
			            switch ( j )
			            {
			                case 0:
			                    led1 = 1;
			                    break;
    			                
			                case 1:
			                case 3:
			                    led2 = 1;
			                    break;
    			                
			                case 2:
			                    led3 = 1;
			                    break;
    			                
			                default:
			                    break;
			            }
			        }

		            trap->R_OverrideShaderFrame (weaponInfo->groupedIndicators[0], led1, cg.time);
		            trap->R_OverrideShaderFrame (weaponInfo->groupedIndicators[1], led2, cg.time);
		            trap->R_OverrideShaderFrame (weaponInfo->groupedIndicators[2], led3, cg.time);
			    }
			    else
			    {
			        trap->R_OverrideShaderFrame (weaponInfo->fireModeIndicator, 0, cg.time);
			        trap->R_OverrideShaderFrame (weaponInfo->groupedIndicators[0], 0, cg.time);
			        trap->R_OverrideShaderFrame (weaponInfo->groupedIndicators[1], 0, cg.time);
			        trap->R_OverrideShaderFrame (weaponInfo->groupedIndicators[2], 0, cg.time);
			    }
                break;
                
            /*default:
            case IND_NONE:
                trap->R_OverrideShaderFrame (weaponInfo->fireModeIndicator, 0, cg.time);
                break;*/
        }
	} else {
		// This isnt me, check if the player uses a different weapon than i do, if so, clear the display
		if (cent->currentState.weapon == cg.predictedPlayerState.weapon) {
			return;
		}

        trap->R_OverrideShaderFrame (weaponInfo->fireModeIndicator, 0, cg.time);
        trap->R_OverrideShaderFrame (weaponInfo->groupedIndicators[0], 10, cg.time);
        trap->R_OverrideShaderFrame (weaponInfo->groupedIndicators[1], 10, cg.time);
        trap->R_OverrideShaderFrame (weaponInfo->groupedIndicators[2], 10, cg.time);
	}
}