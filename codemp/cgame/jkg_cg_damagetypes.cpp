#include "cg_local.h"
#include "game/bg_public.h"
#include "jkg_cg_damagetypes.h"

/*
 *	Applies a buff shader to a refEntity_t
 */
static void JKG_ShaderBuffRefEntity(refEntity_t* refEntity, debuffVisualsData_t* entData, jkgBuff_t* buffData)
{
	if (!buffData->visuals.useShader)
	{
		// don't use shader data
		return;
	}
	if (entData->debuffStartTime + buffData->visuals.shaderLen < cg.time)
	{
		// expired
		return;
	}

	for (int j = 0; j < 4; j++)
	{
		refEntity->shaderRGBA[j] = buffData->visuals.shaderRGBA[j];
	}
	refEntity->renderfx = 0; // maybe add some stuff here later on

	if (buffData->visuals.shader[0])
	{
		refEntity->customShader = trap->R_RegisterShader(buffData->visuals.shader);
	}
}

/*
 *	 Applies a buff efx to a refEntity_t
 */
static void JKG_EfxBuffRefEntity(centity_t* cent, refEntity_t* refEntity, debuffVisualsData_t* entData, jkgBuff_t* buffData)
{
	if (!buffData->visuals.useEFX)
	{
		// don't use efx data
		return;
	}
	if (entData->lastEFXTime + buffData->visuals.efxDebounce > cg.time)
	{
		// still debouncing
		return;
	}
	if (buffData->visuals.efx[0] == '\0' || buffData->visuals.efxBolt[0] == '\0')
	{
		// someone decided to be funny and not put valid data here
		return;
	}

	int lowerLumbarBolt = trap->G2API_AddBolt(cent->ghoul2, 0, buffData->visuals.efxBolt);
	qhandle_t efx = trap->FX_RegisterEffect(buffData->visuals.efx);
	trap->FX_PlayBoltedEffectID(efx, cent->lerpOrigin, cent->ghoul2, lowerLumbarBolt, cent->currentState.number, 0, 0, qtrue);
	entData->lastEFXTime = cg.time;
}

/*
 *	Displays debuffs on the player model.
 */
void JKG_PlayerDebuffVisuals ( centity_t *cent, refEntity_t *refEntity )
{
	const entityState_t *es = &cent->currentState;

	for (int i = 0; i < PLAYERBUFF_BITS; i++)
	{
		if (es->buffsActive & (1 << i))
		{
			jkgBuff_t* pBuff = &buffTable[es->buffs[i].buffID];
			if (!cent->debuffVisuals[i].debuffStartTime)
			{
				cent->debuffVisuals[i].debuffStartTime = cg.time;
			}

			JKG_ShaderBuffRefEntity(refEntity, &cent->debuffVisuals[i], pBuff);
			JKG_EfxBuffRefEntity(cent, refEntity, &cent->debuffVisuals[i], pBuff);
			trap->R_AddRefEntityToScene(refEntity);
		}
		else
		{
			cent->debuffVisuals[i].debuffStartTime = 0;
		}
	}
}

/*
 *	Displays debuffs on the viewmodel.
 */
void JKG_WeaponDebuffVisuals ( centity_t *cent, refEntity_t *refEntity )
{
	const entityState_t *es = &cent->currentState;

	for (int i = 0; i < PLAYERBUFF_BITS; i++)
	{
		if (es->buffsActive & (1 << i))
		{
			jkgBuff_t* pBuff = &buffTable[es->buffs[i].buffID];
			JKG_ShaderBuffRefEntity(refEntity, &cent->debuffVisuals[i], pBuff);
		}
	}
}