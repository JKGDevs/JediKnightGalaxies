// JKGalaxies' Weapons - residing in models/weapons
// Please keep this list in alphabetical order with headers and credits
// _______________________________________________________
//  - TABLE OF CONTENTS - 
// a200_acp_battlerifle
// a200_acp_pistol
// acp_arraygun
// acp_sniperrifle
// arc_casterimperial
// arc_castermk1
// bowcaster
// bowcaster_classic
// bowcaster_heavy
// bryar_rifle
// bryar_shotgun
// carboniterifle
// charge_detonator
// charge_sequencer
// cr-24_flamecarbine
// dc-15a_rifle
// de-10_pistol
// dh-17_pistol
// dl-18_pistol
// dlt-19_heavyblaster
// doublebarrel_arraygun
// dxr6_heavycarbine
// e-11_rifle
// e-11a_rifle
// e-11b_rifle
// e-60r_rocketlauncher
// ee-3_rifle
// elg-3a_pistol
// fwg-5_flechettepistol
// grenade_bacta_antidote-e
// grenade_c-14a
// grenade_c22frag
// grenade_codekey
// grenade_concussion
// grenade_cryoban
// grenade_flashbang
// grenade_plasma
// grenade_smoke
// grenade_stun
// grenade_thermal
// kyd-21_pistol
// laserprojector
// l8striker_pistol
// lj-70_concrifle
// ls-180_acp_heavyrepeater
// lt-60_slugcarbine
// lt-60_slugcarbine_comm
// mine_lasertrip
// packered_mortargun
// protoncarbine
// pulsecannon
// q2_holdoutpistol
// sterling
// stouker_concussionrifle
// suppressorpistol
// t-21_repeater
// w-90_concussion

// ~BlasTech, on the 8th of February 2011
// ________________________________________________________

// Reflective shader used for unwrapped but not textured models

envmap_0
{
	q3map_nolightmap
    {
        map envmap
        rgbGen lightingDiffuse
        tcGen environment
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

envmap
{
	q3map_nolightmap
    {
        map envmap
        rgbGen lightingDiffuse
        tcGen environment
    }
}

// "A" 200 ACP Battle Rifle (Akimoto, Karpman, rusty skin by BlasTech)

models/weapons/a200_acp_battlerifle/acp_rifle_0
{
	q3map_nolightmap
    {
        map models/weapons/a200_acp_battlerifle/acp_rifle
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/a200_acp_battlerifle/acp_rifle_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/a200_acp_battlerifle/acp_rifle
{
	q3map_nolightmap
    {
        map models/weapons/a200_acp_battlerifle/acp_rifle
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/a200_acp_battlerifle/acp_rifle_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/weapons/a200_acp_battlerifle/acp_rifle_rusty_0
{
	q3map_nolightmap
    {
        map models/weapons/a200_acp_battlerifle/acp_rifle_rusty
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/a200_acp_battlerifle/acp_rifle_rusty_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/a200_acp_battlerifle/acp_rifle_rusty
{
	q3map_nolightmap
    {
        map models/weapons/a200_acp_battlerifle/acp_rifle_rusty
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/a200_acp_battlerifle/acp_rifle_rusty_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/weapons/a200_acp_battlerifle/holosight
{
	q3map_nolightmap
	cull	twosided
    {
        map models/weapons/a200_acp_battlerifle/holosight
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave random 0.96 0.04 0.2 1
    }
}

models/weapons/a200_acp_battlerifle/holosight_back
{
	q3map_nolightmap
    {
        map models/weapons/a200_acp_battlerifle/holosight_back
        blendFunc GL_DST_COLOR GL_ONE
        rgbGen identity
        tcMod rotate -2
    }
}

models/weapons/a200_acp_battlerifle/holosight_front
{
	q3map_nolightmap
    {
        clampMap models/weapons/a200_acp_battlerifle/holosight_front
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
        tcMod rotate 6
    }
}

// "A" 200 Accelerated Charged Particle Pistol (BlasTech)

models/weapons/a200_acp_pistol/pistol_0
{
	q3map_nolightmap
    {
        map models/weapons/a200_acp_pistol/pistol
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/a200_acp_pistol/pistol_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
    {
        map models/weapons/a200_acp_pistol/pistol_glow
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

models/weapons/a200_acp_pistol/pistol
{
	q3map_nolightmap
    {
        map models/weapons/a200_acp_pistol/pistol
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/a200_acp_pistol/pistol_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

// ACP Array Gun (Reven, BlasTech)

models/weapons/acp_arraygun/shotty_0
{
	q3map_nolightmap
    {
        map models/weapons/acp_arraygun/shotty
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/acp_arraygun/shotty_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/acp_arraygun/shotty
{
	q3map_nolightmap
    {
        map models/weapons/acp_arraygun/shotty
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/acp_arraygun/shotty_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

// ACP Heavy Anti-Materiel Rifle -57c, the Hammer of Shirke (BlasTech)

models/weapons/acp_sniperrifle/rifle_0
{
	q3map_nolightmap
    {
        map models/weapons/acp_sniperrifle/rifle
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/acp_sniperrifle/rifle_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
    {
        map models/weapons/acp_sniperrifle/rifle_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

models/weapons/acp_sniperrifle/rifle
{
	q3map_nolightmap
    {
        map models/weapons/acp_sniperrifle/rifle
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/acp_sniperrifle/rifle_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map models/weapons/acp_sniperrifle/rifle_glow
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

models/weapons/acp_sniperrifle/window_lower
{
	q3map_nolightmap
    {
        map envmap
        rgbGen lightingDiffuse
        tcGen environment
    }
    {
        map models/weapons/acp_sniperrifle/window_lower
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/acp_sniperrifle/window_lower_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
    {
        map models/weapons/acp_sniperrifle/window_lower_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

models/weapons/acp_sniperrifle/window
{
	q3map_nolightmap
    {
        map models/weapons/acp_sniperrifle/window
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen lightingDiffuse
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

// Notice that the scope mask also is the ammo indicator shader... Be careful. It is used on the model as well. 

models/weapons/acp_sniperrifle/scope_mask
{
	nomipmaps
	cull	twosided
    {
        animMap 1 models/weapons/acp_sniperrifle/scope_mask0 models/weapons/acp_sniperrifle/scope_mask1 models/weapons/acp_sniperrifle/scope_mask2 models/weapons/acp_sniperrifle/scope_mask2 models/weapons/acp_sniperrifle/scope_mask2 models/weapons/acp_sniperrifle/scope_mask2 models/weapons/acp_sniperrifle/scope_mask2 models/weapons/acp_sniperrifle/scope_mask2 models/weapons/acp_sniperrifle/scope_mask2 models/weapons/acp_sniperrifle/scope_mask2 models/weapons/acp_sniperrifle/scope_mask0 models/weapons/acp_sniperrifle/scope_mask- 
        blendFunc GL_ONE GL_ONE
        glow
    }
    {
        map models/weapons/acp_sniperrifle/scope_mask
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen identity
    }
    {
        map models/weapons/acp_sniperrifle/scope_mask_overlay
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        rgbGen identity
    }
}

// Imperial ARC Caster (BlasTech)

models/weapons/arc_casterimperial/arc_caster_0
{
	q3map_nolightmap
    {
        map models/weapons/arc_casterimperial/arc_caster
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/arc_casterimperial/arc_caster_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/arc_casterimperial/arc_caster
{
	q3map_nolightmap
    {
        map models/weapons/arc_casterimperial/arc_caster
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/arc_casterimperial/arc_caster_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

// MK1 Prototype

models/weapons/arc_castermk1/prototype_0
{
	q3map_nolightmap
    {
        map models/weapons/arc_castermk1/prototype
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/arc_castermk1/prototype_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/arc_castermk1/prototype
{
	q3map_nolightmap
    {
        map models/weapons/arc_castermk1/prototype
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/arc_castermk1/prototype_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

// Wookiee Crossbows (BlasTech)

models/weapons/bowcaster/crossbow_0
{
	q3map_nolightmap
    {
        map models/weapons/bowcaster/crossbow
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/bowcaster/crossbow_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/bowcaster/crossbow
{
	q3map_nolightmap
    {
        map models/weapons/bowcaster/crossbow
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/bowcaster/crossbow_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/weapons/bowcaster/crossbow2_0
{
	q3map_nolightmap
    {
        map models/weapons/bowcaster/crossbow2
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/bowcaster/crossbow2_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/bowcaster/crossbow2
{
	q3map_nolightmap
    {
        map models/weapons/bowcaster/crossbow2
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/bowcaster/crossbow2_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/weapons/bowcaster/eyeprotection_0
{
	q3map_nolightmap
    {
        map models/weapons/bowcaster/eyeprotection
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/bowcaster/eyeprotection
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/weapons/bowcaster/eyeprotection
{
	q3map_nolightmap
    {
        map models/weapons/bowcaster/eyeprotection
        rgbGen lightingDiffuse
    }
}

models/weapons/bowcaster/lens
{
	q3map_nolightmap
    {
        map models/weapons/bowcaster/lens
        rgbGen lightingDiffuse
    }
    {
        map envmap
        blendFunc GL_DST_COLOR GL_ONE
        detail
        tcGen environment
        tcMod scale 4 4
    }
    {
        map models/weapons/scopelens_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
    }
}

models/weapons/bowcaster/lens_night
{
	q3map_nolightmap
    {
        map models/weapons/bowcaster/lens
        rgbGen lightingDiffuse
    }
    {
        map envmap
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        detail
        tcGen environment
        tcMod scale 4 4
    }
    {
        map models/weapons/scopelens_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
    }
    {
        map models/weapons/bowcaster/lens_glow
        blendFunc GL_ONE GL_ONE
        glow
        detail
    }
}

models/weapons/bowcaster/lensMask
{
	nomipmaps
	cull	twosided
    {
        map models/weapons/bowcaster/lensMask
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen identity
    }
}

models/weapons/bowcaster/lensMask_zoom
{
	nomipmaps
	cull	twosided
    {
        map models/weapons/bowcaster/lensMask_zoom
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        detail
        rgbGen vertex
    }
}

// Classic Assault Bowcaster

models/weapons/bowcaster_classic/crossbow_0
{
	q3map_nolightmap
    {
        map models/weapons/bowcaster_classic/crossbow
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/bowcaster_classic/crossbow_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/bowcaster_classic/crossbow
{
	q3map_nolightmap
    {
        map models/weapons/bowcaster_classic/crossbow
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/bowcaster_classic/crossbow_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/weapons/bowcaster_classic/crossbow2
{
	q3map_nolightmap
    {
        map models/weapons/bowcaster_classic/crossbow2
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/bowcaster_classic/crossbow2_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/bowcaster_classic/crossbow2_0
{
	q3map_nolightmap
    {
        map models/weapons/bowcaster_classic/crossbow2
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/bowcaster_classic/crossbow2_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/weapons/bowcaster_classic/lens
{
	q3map_nolightmap
    {
        map models/weapons/bowcaster/lens
        rgbGen lightingDiffuse
    }
    {
        map envmap
        blendFunc GL_DST_COLOR GL_ONE
        detail
        tcGen environment
        tcMod scale 4 4
    }
    {
        map models/weapons/scopelens_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
    }
    {
        map models/weapons/bowcaster/lens_glow
        blendFunc GL_ONE GL_ONE
        glow
        detail
    }
}

// Heavy Wookiee Crossbow (BlasTech)

models/weapons/bowcaster_heavy/bowcaster_0
{
	q3map_nolightmap
    {
        map models/weapons/bowcaster_heavy/bowcaster
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/bowcaster_heavy/bowcaster_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/bowcaster_heavy/bowcaster
{
	q3map_nolightmap
    {
        map models/weapons/bowcaster_heavy/bowcaster
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/bowcaster_heavy/bowcaster_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/weapons/bowcaster_heavy/lens
{
	q3map_nolightmap
    {
        map models/weapons/bowcaster/lens
        rgbGen lightingDiffuse
    }
    {
        map envmap
        blendFunc GL_DST_COLOR GL_ONE
        detail
        tcGen environment
        tcMod scale 4 4
    }
    {
        map models/weapons/scopelens_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
    }
}

models/weapons/bowcaster_heavy/lens_night
{
	q3map_nolightmap
    {
        map models/weapons/bowcaster/lens
        rgbGen lightingDiffuse
    }
    {
        map envmap
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        detail
        tcGen environment
        tcMod scale 4 4
    }
    {
        map models/weapons/scopelens_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
    }
    {
        map models/weapons/bowcaster_heavy/lens_glow
        blendFunc GL_ONE GL_ONE
        glow
        detail
    }
}

models/weapons/bowcaster_heavy/scope_mask
{
	nomipmaps
	cull	twosided
    {
        map models/weapons/bowcaster_heavy/scope_mask_overlay
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        rgbGen identity
    }
    {
        map models/weapons/bowcaster_heavy/scope_mask
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen identity
    }
}

// Bryar Rifle (BlasTech)

models/weapons/bryar_rifle/bryar_0
{
	q3map_nolightmap
    {
        map models/weapons/bryar_rifle/bryar
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/bryar_rifle/bryar_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
        tcMod scale 2 1
    }
}

models/weapons/bryar_rifle/bryar
{
	q3map_nolightmap
    {
        map models/weapons/bryar_rifle/bryar
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/bryar_rifle/bryar_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/weapons/bryar_rifle/scope_mask
{
	nomipmaps
	cull	twosided
    {
        map models/weapons/dlt-19_heavyblaster/scope_mask_overlay
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        rgbGen identity
    }
    {
        map models/weapons/bryar_rifle/scope_mask
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen identity
    }
}

// Carbonite Rifle (Reven, BlasTech)

models/weapons/carboniterifle/rifle_0
{
	q3map_nolightmap
    {
        map models/weapons/carboniterifle/rifle
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/carboniterifle/rifle_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap
        blendFunc GL_DST_COLOR GL_ONE
        detail
        tcGen environment
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
    {
        map models/weapons/carboniterifle/rifle_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

models/weapons/carboniterifle/rifle
{
	q3map_nolightmap
    {
        map models/weapons/carboniterifle/rifle
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/carboniterifle/rifle_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map models/weapons/carboniterifle/rifle_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

// Remote Detonator (Reven) 

models/weapons/charge_detonator/detonator
{
	q3map_nolightmap
    {
        map models/weapons/charge_detonator/detonator
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/charge_detonator/detonator_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

// Sequencer Charge (Reven, BlasTech)

models/weapons/charge_sequencer/mine_0
{
	q3map_nolightmap
    {
        map models/weapons/charge_sequencer/mine
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/charge_sequencer/mine_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
        tcMod scale 2 1
    }
    {
        map models/weapons/charge_sequencer/mine_glow_green
        blendFunc GL_ONE GL_ONE
        glow
        detail
        rgbGen identity
    }
}

models/weapons/charge_sequencer/mine
{
	q3map_nolightmap
    {
        map models/weapons/charge_sequencer/mine
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/charge_sequencer/mine_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map models/weapons/charge_sequencer/mine_glow_green
        blendFunc GL_ONE GL_ONE
        detail
        rgbGen identity
    }
}

models/weapons/charge_sequencer/mine_armed
{
	q3map_nolightmap
    {
        map models/weapons/charge_sequencer/mine
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/charge_sequencer/mine_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map models/weapons/charge_sequencer/mine_glow_red
        blendFunc GL_ONE GL_ONE
        detail
        rgbGen identity
    }
    {
        map models/weapons/charge_sequencer/mine_glow_red2
        blendFunc GL_ONE GL_ONE
        detail
        rgbGen wave sin 0 1 0 1
    }
}

// CR-24 Flame Carbine (Reven, BlasTech)

models/weapons/cr-24_flamecarbine/carbine_0
{
	q3map_nolightmap
    {
        map models/weapons/cr-24_flamecarbine/carbine
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/cr-24_flamecarbine/carbine_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/cr-24_flamecarbine/carbine
{
	q3map_nolightmap
    {
        map models/weapons/cr-24_flamecarbine/carbine
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/cr-24_flamecarbine/carbine_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

// DC-15A Blaster Rifle, w/ scope & launcher attachments (BlasTech)

models/weapons/dc-15a_rifle/rifle_0
{
	q3map_nolightmap
    {
        map models/weapons/dc-15a_rifle/rifle
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/dc-15a_rifle/rifle_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/dc-15a_rifle/rifle
{
	q3map_nolightmap
    {
        map models/weapons/dc-15a_rifle/rifle
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/dc-15a_rifle/rifle_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/weapons/dc-15a_rifle/lens
{
	q3map_nolightmap
    {
        map models/weapons/dc-15a_rifle/lens
        rgbGen lightingDiffuse
    }
    {
        map envmap
        blendFunc GL_DST_COLOR GL_ONE
        detail
        tcGen environment
        tcMod scale 4 4
    }
    {
        map models/weapons/scopelens_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
    }
}

models/weapons/dc-15a_rifle/scope_mask
{
	nomipmaps
	cull	twosided
    {
        map models/weapons/dc-15a_rifle/scope_mask_overlay
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        rgbGen identity
    }
    {
        map models/weapons/dc-15a_rifle/scope_mask
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen identity
    }
}

models/weapons/dc-15a_rifle/scope_mask_aaa
{
	nomipmaps
	cull	twosided
    {
        map models/weapons/dc-15a_rifle/scope_mask_overlay
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        rgbGen identity
    }
    {
        map models/weapons/dc-15a_rifle/scope_mask_aaa
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen identity
    }
}

models/weapons/dc-15a_rifle/antiarmor_0
{
	q3map_nolightmap
    {
        map models/weapons/dc-15a_rifle/antiarmor
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/dc-15a_rifle/antiarmor_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/dc-15a_rifle/antiarmor
{
	q3map_nolightmap
    {
        map models/weapons/dc-15a_rifle/antiarmor
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/dc-15a_rifle/antiarmor_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

// DE-10 Pistol (Silverfang, BlasTech)

models/weapons/de-10_pistol/pistol_0
{
	q3map_nolightmap
    {
        map models/weapons/de-10_pistol/pistol
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/de-10_pistol/pistol_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/de-10_pistol/pistol
{
	q3map_nolightmap
    {
        map models/weapons/de-10_pistol/pistol
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/de-10_pistol/pistol_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/weapons/de-10_pistol/lens
{
	q3map_nolightmap
    {
        map models/weapons/de-10_pistol/lens
        rgbGen lightingDiffuse
    }
    {
        map envmap
        blendFunc GL_DST_COLOR GL_ONE
        detail
        tcGen environment
        tcMod scale 4 4
    }
    {
        map models/weapons/scopelens_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
    }
}

models/weapons/de-10_pistol/scope_mask
{
	nomipmaps
	cull	twosided
    {
        map models/weapons/de-10_pistol/scope_mask_overlay
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        rgbGen identity
    }
    {
        map models/weapons/de-10_pistol/scope_mask
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen identity
    }
}

// DH-17 Pistol (Silverfang, BlasTech)

models/weapons/dh-17_pistol/pistol_0
{
	q3map_nolightmap
    {
        map models/weapons/dh-17_pistol/pistol
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/dh-17_pistol/pistol_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/dh-17_pistol/pistol
{
	q3map_nolightmap
    {
        map models/weapons/dh-17_pistol/pistol
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/dh-17_pistol/pistol_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/weapons/dh-17_pistol/scope
{
	q3map_nolightmap
    {
        map models/weapons/dh-17_pistol/scope
        rgbGen lightingDiffuse
    }
    {
        map envmap
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        detail
        tcGen environment
        tcMod scale 2 2
    }
    {
        map models/weapons/scopelens_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
    }
}

models/weapons/dh-17_pistol/scope_mask
{
	nomipmaps
	cull	twosided
    {
        map models/weapons/dh-17_pistol/scope_mask_overlay
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        rgbGen identity
    }
    {
        map models/weapons/dh-17_pistol/scope_mask
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen identity
    }
}

// DL-18 Blaster Pistol (Reven, BlasTech)

models/weapons/dl-18_pistol/pistol_0
{
	q3map_nolightmap
    {
        map models/weapons/dl-18_pistol/pistol
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/dl-18_pistol/pistol_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/dl-18_pistol/pistol
{
	q3map_nolightmap
    {
        map models/weapons/dl-18_pistol/pistol
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/dl-18_pistol/pistol_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

// DLT-19 Heavy Blaster Rifle

models/weapons/dlt-19_heavyblaster/rifle_0
{
	q3map_nolightmap
    {
        map models/weapons/dlt-19_heavyblaster/rifle
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/dlt-19_heavyblaster/rifle_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/dlt-19_heavyblaster/rifle
{
	q3map_nolightmap
    {
        map models/weapons/dlt-19_heavyblaster/rifle
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/dlt-19_heavyblaster/rifle_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/weapons/dlt-19_heavyblaster/lens
{
	q3map_nolightmap
    {
        map models/weapons/dlt-19_heavyblaster/lens
        rgbGen lightingDiffuse
    }
    {
        map envmap
        blendFunc GL_DST_COLOR GL_ONE
        detail
        tcGen environment
        tcMod scale 4 4
    }
    {
        map models/weapons/scopelens_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
    }
}

models/weapons/dlt-19_heavyblaster/scope_mask
{
	nomipmaps
	cull	twosided
    {
        map models/weapons/dlt-19_heavyblaster/scope_mask_overlay
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        rgbGen identity
    }
    {
        map models/weapons/dlt-19_heavyblaster/scope_mask
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen identity
    }
}

// AA-35c Pulse Array Gun (Dusk, BlasTech)

models/weapons/doublebarrel_arraygun/rifle_0
{
	q3map_nolightmap
    {
        map models/weapons/doublebarrel_arraygun/rifle
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/doublebarrel_arraygun/rifle_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
    {
        map models/weapons/doublebarrel_arraygun/rifle_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave random 0.96 0.04 0.0 1
    }
}

models/weapons/doublebarrel_arraygun/rifle
{
	q3map_nolightmap
    {
        map models/weapons/doublebarrel_arraygun/rifle
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/doublebarrel_arraygun/rifle_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map models/weapons/doublebarrel_arraygun/rifle_glow
        blendFunc GL_ONE GL_ONE
        rgbGen wave random 0.96 0.04 0.0 1
    }
}

// Dressellian Slugthrower Rifle (Silverfang, BlasTech)

models/weapons/drsln_slugrifle/rifle_0
{
	q3map_nolightmap
    {
        map models/weapons/drsln_slugrifle/rifle
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/drsln_slugrifle/rifle_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/drsln_slugrifle/rifle
{
	q3map_nolightmap
    {
        map models/weapons/drsln_slugrifle/rifle
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/drsln_slugrifle/rifle_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/weapons/drsln_slugrifle/lens
{
	q3map_nolightmap
    {
        map models/weapons/drsln_slugrifle/lens
        rgbGen lightingDiffuse
    }
    {
        map envmap
        blendFunc GL_DST_COLOR GL_ONE
        detail
        tcGen environment
        tcMod scale 4 4
    }
    {
        map models/weapons/scopelens_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
    }
}

models/weapons/drsln_slugrifle/scope_mask
{
	nomipmaps
	cull	twosided
    {
        map models/weapons/drsln_slugrifle/scope_mask_overlay
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        rgbGen identity
    }
    {
        map models/weapons/drsln_slugrifle/scope_mask
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen identity
    }
}

// DXR-6 Disruptor Carbine

models/weapons/dxr6_heavycarbine/disruptor_0
{
	q3map_nolightmap
    {
        map models/weapons/dxr6_heavycarbine/disruptor
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/dxr6_heavycarbine/disruptor_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/dxr6_heavycarbine/disruptor
{
	q3map_nolightmap
    {
        map models/weapons/dxr6_heavycarbine/disruptor
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/dxr6_heavycarbine/disruptor_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/weapons/dxr6_heavycarbine/disruptor_glow_0
{
	q3map_nolightmap
    {
        map models/weapons/dxr6_heavycarbine/disruptor
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/dxr6_heavycarbine/disruptor_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
    {
        map models/weapons/dxr6_heavycarbine/disruptor_glow
        blendFunc GL_ONE GL_ONE
        rgbGen wave random 0.4 0.1 0 1
    }
}

models/weapons/dxr6_heavycarbine/disruptor_glow
{
	q3map_nolightmap
    {
        map models/weapons/dxr6_heavycarbine/disruptor
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/dxr6_heavycarbine/disruptor_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map models/weapons/dxr6_heavycarbine/disruptor_glow
        blendFunc GL_ONE GL_ONE
        rgbGen wave random 0.4 0.1 0 1
    }
}

models/weapons/dxr6_heavycarbine/lens
{
	q3map_nolightmap
    {
        map models/weapons/dxr6_heavycarbine/lens
        rgbGen lightingDiffuse
    }
    {
        map envmap
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        detail
        tcGen environment
        tcMod scale 2 2
    }
    {
        map models/weapons/scopelens_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
    }
}

// E-11 Blaster Rifle (BlasTech)

models/weapons/e-11_rifle/blaster_0
{
	q3map_nolightmap
    {
        map models/weapons/e-11_rifle/blaster
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/e-11_rifle/blaster_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/e-11_rifle/blaster
{
	q3map_nolightmap
    {
        map models/weapons/e-11_rifle/blaster
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/e-11_rifle/blaster_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/weapons/e-11_rifle/scope
{
	q3map_nolightmap
    {
        map models/weapons/e-11_rifle/scope_trans
        blendFunc GL_ZERO GL_ONE_MINUS_SRC_COLOR
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/e-11_rifle/scope
        blendFunc GL_ONE GL_ONE
        rgbGen lightingDiffuse
    }
    {
        map envmap
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        detail
        tcGen environment
        tcMod scale 2 2
    }
    {
        map models/weapons/scopelens_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
    }
}

models/weapons/e-11_rifle/electroscope
{
	q3map_nolightmap
    {
        map models/weapons/e-11_rifle/scope_trans
        blendFunc GL_ZERO GL_ONE_MINUS_SRC_COLOR
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/e-11_rifle/electroscope
        blendFunc GL_ONE GL_ONE
        rgbGen lightingDiffuse
    }
    {
        map envmap
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        detail
        tcGen environment
        tcMod scale 2 2
    }
    {
        map models/weapons/scopelens_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
    }
    {
        map models/weapons/e-11_rifle/electroscope_glow
        blendFunc GL_ONE GL_ONE
        glow
        detail
    }
}

models/weapons/e-11_rifle/scope_mask
{
	nomipmaps
	cull	twosided
    {
        map models/weapons/e-11_rifle/scope_mask_overlay
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        rgbGen identity
    }
    {
        map models/weapons/e-11_rifle/scope_mask
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen identity
    }
}

models/weapons/e-11_rifle/electroscope_Mask
{
	nomipmaps
	cull	twosided
    {
        map models/weapons/e-11_rifle/electroscope_mask_overlay
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        rgbGen identity
    }
    {
        map models/weapons/e-11_rifle/scope_mask
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen identity
    }
}

models/weapons/e-11a_rifle/twisted_coil
{
	q3map_nolightmap
	cull	twosided
    {
        map models/weapons/e-11a_rifle/twisted_coil
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/e-11a_rifle/twisted_coil
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
    }
}

models/weapons/e-11a_rifle/scope_Mask
{
	nomipmaps
	cull	twosided
    {
        map models/weapons/e-11_rifle/scope_mask_overlay
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        rgbGen identity
    }
    {
        map models/weapons/e-11a_rifle/scope_mask
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen identity
    }
}

models/weapons/e-11a_rifle/electroscope_Mask
{
	nomipmaps
	cull	twosided
    {
        map models/weapons/e-11_rifle/electroscope_mask_overlay
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        rgbGen identity
    }
    {
        map models/weapons/e-11a_rifle/scope_mask
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen identity
    }
}

models/weapons/e-11b_rifle/blaster_0
{
	q3map_nolightmap
    {
        map models/weapons/e-11b_rifle/blaster
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/e-11b_rifle/blaster_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/e-11b_rifle/blaster
{
	q3map_nolightmap
    {
        map models/weapons/e-11b_rifle/blaster
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/e-11b_rifle/blaster_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/weapons/e-11b_rifle/scope_Mask
{
	nomipmaps
	cull	twosided
    {
        map models/weapons/e-11_rifle/scope_mask_overlay
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        rgbGen identity
    }
    {
        map models/weapons/e-11b_rifle/scope_mask
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen identity
    }
}

models/weapons/e-11b_rifle/electroscope_Mask
{
	nomipmaps
	cull	twosided
    {
        map models/weapons/e-11_rifle/electroscope_mask_overlay
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        rgbGen identity
    }
    {
        map models/weapons/e-11b_rifle/scope_mask
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen identity
    }
}

// E-60R Rocket Launcher (Silverfang)

models/weapons/e-60r_rocketlauncher/launcher_0
{
	q3map_nolightmap
    {
        map models/weapons/e-60r_rocketlauncher/launcher
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/e-60r_rocketlauncher/launcher_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/e-60r_rocketlauncher/launcher
{
	q3map_nolightmap
    {
        map models/weapons/e-60r_rocketlauncher/launcher
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/e-60r_rocketlauncher/launcher_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

// EE-3 Blaster (Jonthe, Pande)

models/weapons/ee-3_rifle/blaster_0
{
	q3map_nolightmap
    {
        map models/weapons/ee-3_rifle/blaster
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/ee-3_rifle/blaster_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
	{
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/ee-3_rifle/blaster
{
	q3map_nolightmap
    {
        map models/weapons/ee-3_rifle/blaster
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/ee-3_rifle/blaster_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/weapons/ee-3_rifle/scope_mask
{
	nomipmaps
	cull	twosided
    {
        map models/weapons/ee-3_rifle/scope_mask_overlay
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        rgbGen identity
    }
    {
        map models/weapons/ee-3_rifle/scope_mask
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen identity
    }
}

// ELG-3A "Diplomat's Blaster" Pistol (Silverfang/BlasTech) ((Previously Reven, Suibuku))

models/weapons/elg-3a_pistol/pistol_0
{
	q3map_nolightmap
    {
        map models/weapons/elg-3a_pistol/pistol
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/elg-3a_pistol/pistol_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap
        blendFunc GL_DST_COLOR GL_ONE
        detail
        tcGen environment
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/elg-3a_pistol/pistol
{
	q3map_nolightmap
    {
        map models/weapons/elg-3a_pistol/pistol
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/elg-3a_pistol/pistol_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

// FWG-5 Flechette Pistol (IG-64, Suibuku)

models/weapons/fwg-5_flechettepistol/fwg5_0
{
	q3map_nolightmap
    {
        map models/weapons/fwg-5_flechettepistol/fwg5
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/fwg-5_flechettepistol/fwg5_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/fwg-5_flechettepistol/fwg5
{
	q3map_nolightmap
    {
        map models/weapons/fwg-5_flechettepistol/fwg5
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/fwg-5_flechettepistol/fwg5_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

// Bacta Grenade (Xel, Silverfang)

models/weapons/grenade_bacta/grenade_0
{
    q3map_nolightmap
    {
        map models/weapons/grenade_bacta/grenade
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/grenade_bacta/grenade_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/weapons/grenade_bacta/bacta_0
{
    q3map_nolightmap
    {
        map models/weapons/grenade_bacta/bacta
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/grenade_bacta/bacta_glow
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

// C-14A Stun Grenades (Reven, BlasTech)

models/weapons/grenade_c-14a/grenade_0
{
	q3map_nolightmap
    {
        map models/weapons/grenade_c-14a/grenade
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/grenade_c-14a/grenade_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/grenade_c-14a/grenade
{
	q3map_nolightmap
    {
        map models/weapons/grenade_c-14a/grenade
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/grenade_c-14a/grenade_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

// C-22 frag grenades (Karpman)

models/weapons/grenade_c22frag/c22frag_0
{
	q3map_nolightmap
    {
        map models/weapons/grenade_c22frag/c22frag
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/grenade_c22frag/c22frag_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/grenade_c22frag/c22frag
{
	q3map_nolightmap
    {
        map models/weapons/grenade_c22frag/c22frag
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/grenade_c22frag/c22frag_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

// Code-key Thermal Detonators (Reven, MaceCrusherMadusunus)

models/weapons/grenade_codekey/grenade_0
{
	q3map_nolightmap
    {
        map models/weapons/grenade_codekey/grenade
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/grenade_codekey/grenade_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/grenade_codekey/grenade
{
	q3map_nolightmap
    {
        map models/weapons/grenade_codekey/grenade
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/grenade_codekey/grenade_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

// Concussion Grenades (Dusk/Silverfang, BlasTech)

models/weapons/grenade_concussion/grenade_0
{
	q3map_nolightmap
    {
        map models/weapons/grenade_concussion/grenade
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/grenade_concussion/grenade_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/grenade_concussion/grenade
{
	q3map_nolightmap
    {
        map models/weapons/grenade_concussion/grenade
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/grenade_concussion/grenade_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

// CryoBan grenades (IG-64, Karpman)

models/weapons/grenade_cryoban/cryoban_0
{
	q3map_nolightmap
    {
        map models/weapons/grenade_cryoban/cryoban
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/grenade_cryoban/cryoban_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/grenade_cryoban/cryoban
{
	q3map_nolightmap
    {
        map models/weapons/grenade_cryoban/cryoban
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/grenade_cryoban/cryoban_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

// Flash-Bang Grenades (Reven, BlasTech)

models/weapons/grenade_flashbang/grenade_0
{
	q3map_nolightmap
    {
        map models/weapons/grenade_flashbang/grenade
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/grenade_flashbang/grenade_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/grenade_flashbang/grenade
{
	q3map_nolightmap
    {
        map models/weapons/grenade_flashbang/grenade
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/grenade_flashbang/grenade_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

// Plasma Grenade (Dusk, BlasTech)

models/weapons/grenade_plasma/grenade_0
{
	q3map_nolightmap
    {
        map models/weapons/grenade_plasma/grenade
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/grenade_plasma/grenade_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/grenade_plasma/grenade
{
	q3map_nolightmap
    {
        map models/weapons/grenade_plasma/grenade
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/grenade_plasma/grenade_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

// Smoke Grenades (Dusk-Silverfang, Suibuku)

models/weapons/grenade_smoke/grenade_0
{
	q3map_nolightmap
    {
        map models/weapons/grenade_smoke/grenade
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/grenade_smoke/grenade_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/grenade_smoke/grenade
{
	q3map_nolightmap
    {
        map models/weapons/grenade_smoke/grenade
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/grenade_smoke/grenade_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

// Stun Grenade (Reven)

models/weapons/grenade_stun/grenade_0
{
	q3map_nolightmap
    {
        map models/weapons/grenade_stun/grenade
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/grenade_stun/grenade_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/grenade_stun/grenade
{
	q3map_nolightmap
    {
        map models/weapons/grenade_stun/grenade
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/grenade_stun/grenade_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

// EASTER EGG: Cave Johnson's Lemons! (BlasTech)

models/weapons/grenade_stun/lemon
{
	q3map_nolightmap
    {
        map models/weapons/grenade_stun/lemon
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/grenade_stun/lemon_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

// Thermal detonators (Karpman)

models/weapons/grenade_thermal/thermal_det_black_0
{
	q3map_nolightmap
    {
        map models/weapons/grenade_thermal/thermal_det_black
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/grenade_thermal/thermal_det_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/grenade_thermal/thermal_det_black
{
	q3map_nolightmap
    {
        map models/weapons/grenade_thermal/thermal_det_black
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/grenade_thermal/thermal_det_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/weapons/grenade_thermal/thermal_det_silver_0
{
	q3map_nolightmap
    {
        map models/weapons/grenade_thermal/thermal_det_silver
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/grenade_thermal/thermal_det_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/grenade_thermal/thermal_det_silver
{
	q3map_nolightmap
    {
        map models/weapons/grenade_thermal/thermal_det_silver
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/grenade_thermal/thermal_det_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/weapons/grenade_thermal/thermal_det_gold_0
{
	q3map_nolightmap
    {
        map models/weapons/grenade_thermal/thermal_det_gold
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/grenade_thermal/thermal_det_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/grenade_thermal/thermal_det_gold
{
	q3map_nolightmap
    {
        map models/weapons/grenade_thermal/thermal_det_gold
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/grenade_thermal/thermal_det_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

// BlasTech reskins

models/weapons/grenade_thermal/thermal_det_camo
{
	q3map_nolightmap
    {
        map models/weapons/grenade_thermal/thermal_det_camo
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/grenade_thermal/thermal_det_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/grenade_thermal/thermal_det_white
{
	q3map_nolightmap
    {
        map models/weapons/grenade_thermal/thermal_det_white
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/grenade_thermal/thermal_det_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/grenade_thermal/thermal_det_bronze
{
	q3map_nolightmap
    {
        map models/weapons/grenade_thermal/thermal_det_bronze
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/grenade_thermal/thermal_det_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

// KYD-21 Blaster Pistol (Silverfang, BlasTech)

models/weapons/kyd-21_pistol/pistol_0
{
	q3map_nolightmap
    {
        map models/weapons/kyd-21_pistol/pistol
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/kyd-21_pistol/pistol_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap
        blendFunc GL_DST_COLOR GL_ONE
        detail
        tcGen environment
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/kyd-21_pistol/pistol
{
	q3map_nolightmap
    {
        map models/weapons/kyd-21_pistol/pistol
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/kyd-21_pistol/pistol_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/weapons/kyd-21_pistol/pistol_clean_0
{
	q3map_nolightmap
    {
        map models/weapons/kyd-21_pistol/pistol_clean
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/kyd-21_pistol/pistol_clean_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap
        blendFunc GL_DST_COLOR GL_ONE
        detail
        tcGen environment
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/kyd-21_pistol/pistol_clean
{
	q3map_nolightmap
    {
        map models/weapons/kyd-21_pistol/pistol_clean
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/kyd-21_pistol/pistol_clean_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/weapons/kyd-21_pistol/window
{
	q3map_nolightmap
    {
        map models/weapons/kyd-21_pistol/window
        blendFunc GL_SRC_ALPHA GL_ONE
        rgbGen lightingDiffuse
        alphaGen const 0.1
        tcMod scroll 70 45.5
        tcmod scale 3.0 2.0
        glow
    }
}

// Class VI Automated Laser Projector (BlasTech)

models/weapons/laserprojector/projector_0
{
	q3map_nolightmap
    {
        map models/weapons/laserprojector/projector
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/laserprojector/projector_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        detail
        tcGen environment
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/laserprojector/projector
{
	q3map_nolightmap
    {
        map models/weapons/laserprojector/projector
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/laserprojector/projector_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        detail
        tcGen environment
    }
}

// L8 "Striker", also used for L7 "Liquid Silver" (Dusk, BlasTech, additional art by Bigglesinspain)

models/weapons/l8striker_pistol/liquidsilver
{
	q3map_nolightmap
    {
        map models/weapons/l8striker_pistol/liquidsilver
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/l8striker_pistol/liquidsilver_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
        tcMod scale 2 1
    }
}

models/weapons/l8striker_pistol/liquidgold
{
	q3map_nolightmap
    {
        map models/weapons/l8striker_pistol/liquidgold
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/l8striker_pistol/liquidsilver_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
        tcMod scale 2 1
    }
}

models/weapons/l8striker_pistol/liquidrust_0
{
	q3map_nolightmap
    {
        map models/weapons/l8striker_pistol/liquidrust
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/l8striker_pistol/liquidrust_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
        tcMod scale 2 1
    }
}

models/weapons/l8striker_pistol/liquidrust
{
	q3map_nolightmap
    {
        map models/weapons/l8striker_pistol/liquidrust
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/l8striker_pistol/liquidrust_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/weapons/l8_scope/scopeMask
{
	nomipmaps
	cull	twosided
    {
        map models/weapons/l8_scope/scopeMask
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen identity
    }
}

models/weapons/l8_scope/scope
{
	q3map_nolightmap
    {
        map models/weapons/l8_scope/scope
        rgbGen lightingDiffuse
    }
    {
        map envmap
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        detail
        tcGen environment
        tcMod scale 4 4
    }
    {
        map models/weapons/l8_scope/scope_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
    }
}

// LJ-70 Concussion Rifle (Reven, BlasTech)

models/weapons/lj-70_concrifle/rifle_0
{
	q3map_nolightmap
    {
        map models/weapons/lj-70_concrifle/rifle
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/lj-70_concrifle/rifle_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/lj-70_concrifle/rifle
{
	q3map_nolightmap
    {
        map models/weapons/lj-70_concrifle/rifle
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/lj-70_concrifle/rifle_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

// LS-180 Heavy ACP Repeater (Dusk, BlasTech)

models/weapons/ls-180_acp_heavyrepeater/repeater_0
{
	q3map_nolightmap
    {
        map models/weapons/ls-180_acp_heavyrepeater/repeater
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/ls-180_acp_heavyrepeater/repeater_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
    }
    {
        map models/weapons/ls-180_acp_heavyrepeater/repeater_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/weapons/ls-180_acp_heavyrepeater/repeater
{
	q3map_nolightmap
    {
        map models/weapons/ls-180_acp_heavyrepeater/repeater
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/ls-180_acp_heavyrepeater/repeater_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

// LT-60 Slugthrower Carbine (Dusk, Suibuku)

models/weapons/lt-60_slugcarbine/carbine_0
{
	q3map_nolightmap
    {
        map models/weapons/lt-60_slugcarbine/carbine
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/lt-60_slugcarbine/carbine_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/lt-60_slugcarbine/carbine
{
	q3map_nolightmap
    {
        map models/weapons/lt-60_slugcarbine/carbine
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/lt-60_slugcarbine/carbine_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/weapons/lt-60_slugcarbine/reddot
{
	q3map_nolightmap
    {
        map envmap
        blendFunc GL_DST_COLOR GL_ONE
        detail
        tcGen environment
        tcMod scale 2 2
    }
    {
        map models/weapons/lt-60_slugcarbine/reddot
        blendFunc GL_ZERO GL_ONE_MINUS_SRC_COLOR
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/lt-60_slugcarbine/reddot
        blendFunc GL_ONE GL_ONE
        rgbGen lightingDiffuse
    }
}

models/weapons/lt-60_slugcarbine_comm/lens
{
	q3map_nolightmap
    {
        map models/weapons/lt-60_slugcarbine_comm/lens
        rgbGen lightingDiffuse
    }
    {
        map envmap
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        detail
        tcGen environment
        tcMod scale 2 2
    }
    {
        map models/weapons/scopelens_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
    }
}

models/weapons/lt-60_slugcarbine_comm/lens_Mask
{
	nomipmaps
	cull	twosided
    {
        map models/weapons/lt-60_slugcarbine_comm/lens_Mask_overlay
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        rgbGen identity
    }
    {
        map models/weapons/lt-60_slugcarbine_comm/lens_Mask
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen identity
    }
}

// Laser Trip Mine (BlasTech)

models/weapons/mine_lasertrip/mk1_0
{
	q3map_nolightmap
    {
        map models/weapons/mine_lasertrip/mk1
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/mine_lasertrip/mk1_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
        tcMod scale 2 1
    }
}

models/weapons/mine_lasertrip/mk1
{
	q3map_nolightmap
    {
        map models/weapons/mine_lasertrip/mk1
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/mine_lasertrip/mk1_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/weapons/mine_lasertrip/mk1_glow
{
	q3map_nolightmap
    {
        map models/weapons/mine_lasertrip/mk1
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/mine_lasertrip/mk1_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        animMap 20 models/weapons/mine_lasertrip/glow1 models/weapons/mine_lasertrip/glow2 models/weapons/mine_lasertrip/glow3 models/weapons/mine_lasertrip/glow4 models/weapons/mine_lasertrip/glow5 models/weapons/mine_lasertrip/glow6 models/weapons/mine_lasertrip/glow7 models/weapons/mine_lasertrip/glow8
        blendFunc GL_ONE GL_ONE
        glow
    }
}

models/weapons/mine_lasertrip/mk2_0
{
	q3map_nolightmap
    {
        map models/weapons/mine_lasertrip/mk2
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/mine_lasertrip/mk2_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
        tcMod scale 2 1
    }
}

models/weapons/mine_lasertrip/mk2
{
	q3map_nolightmap
    {
        map models/weapons/mine_lasertrip/mk2
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/mine_lasertrip/mk2_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/weapons/mine_lasertrip/mk2_glow
{
	q3map_nolightmap
    {
        map models/weapons/mine_lasertrip/mk2
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/mine_lasertrip/mk2_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        animMap 20 models/weapons/mine_lasertrip/glow1 models/weapons/mine_lasertrip/glow2 models/weapons/mine_lasertrip/glow3 models/weapons/mine_lasertrip/glow4 models/weapons/mine_lasertrip/glow5 models/weapons/mine_lasertrip/glow6 models/weapons/mine_lasertrip/glow7 models/weapons/mine_lasertrip/glow8
        blendFunc GL_ONE GL_ONE
        glow
    }
}

// Packered Mortar Gun (BlasTech)

models/weapons/packered_mortargun/gun_0
{
	q3map_nolightmap
    {
        map models/weapons/packered_mortargun/gun
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/packered_mortargun/gun_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap
        blendFunc GL_DST_COLOR GL_ONE
        detail
        tcGen environment
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/packered_mortargun/gun
{
	q3map_nolightmap
    {
        map models/weapons/packered_mortargun/gun
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/packered_mortargun/gun_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/weapons/packered_mortargun/grill_0
{
	q3map_nolightmap
	cull	twosided
    {
        map models/weapons/packered_mortargun/grill
        alphaFunc GE128
        depthWrite
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/packered_mortargun/grill_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
    }
}

models/weapons/packered_mortargun/grill
{
	q3map_nolightmap
	cull	twosided
    {
        map models/weapons/packered_mortargun/grill
        alphaFunc GE128
        depthWrite
        rgbGen lightingDiffuse
    }
}

// Proton Carbine (Silverfang, Suibuku)

models/weapons/protoncarbine/carbine_0
{
	q3map_nolightmap
    {
        map models/weapons/protoncarbine/carbine
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/protoncarbine/carbine_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/protoncarbine/carbine
{
	q3map_nolightmap
    {
        map models/weapons/protoncarbine/carbine
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/protoncarbine/carbine_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

// Pulse Cannon (Silverfang, BlasTech)

models/weapons/pulsecannon/rifle_0
{
	q3map_nolightmap
    {
        map models/weapons/pulsecannon/rifle
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/pulsecannon/rifle_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
    {
        map models/weapons/pulsecannon/rifle_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave random 0.96 0.04 0.1 1
    }
}

models/weapons/pulsecannon/rifle
{
	q3map_nolightmap
    {
        map models/weapons/pulsecannon/rifle
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/pulsecannon/rifle_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map models/weapons/pulsecannon/rifle_glow
        blendFunc GL_ONE GL_ONE
        rgbGen wave random 0.96 0.04 0.1 1
    }
}

// Q2 Hold-Out Blaster Pistol (Silverfang, BlasTech)

models/weapons/q2_holdoutpistol/pistol_0
{
	q3map_nolightmap
    {
        map models/weapons/q2_holdoutpistol/pistol
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/q2_holdoutpistol/pistol_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap
        blendFunc GL_DST_COLOR GL_ONE
        detail
        tcGen environment
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/q2_holdoutpistol/pistol
{
	q3map_nolightmap
    {
        map models/weapons/q2_holdoutpistol/pistol
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/q2_holdoutpistol/pistol_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

// Sterling SMG (BlasTech)

models/weapons/sterling/sterling_0
{
	q3map_nolightmap
    {
        map models/weapons/sterling/sterling
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/sterling/sterling_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/sterling/sterling
{
	q3map_nolightmap
    {
        map models/weapons/sterling/sterling
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/sterling/sterling_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

// Stouker Concussion Rifle (the real Dark Forces one)

models/weapons/stouker_concussionrifle/rifle_0
{
	q3map_nolightmap
    {
        map models/weapons/stouker_concussionrifle/rifle
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/stouker_concussionrifle/rifle_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/stouker_concussionrifle/rifle
{
	q3map_nolightmap
    {
        map models/weapons/stouker_concussionrifle/rifle
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/stouker_concussionrifle/rifle_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/weapons/stouker_concussionrifle/rifle_rusty_0
{
	q3map_nolightmap
    {
        map models/weapons/stouker_concussionrifle/rifle_rusty
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/stouker_concussionrifle/rifle_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/stouker_concussionrifle/rifle_rusty
{
	q3map_nolightmap
    {
        map models/weapons/stouker_concussionrifle/rifle_rusty
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/stouker_concussionrifle/rifle_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

// Trandoshan Suppressor Pistol (Dusk, BlasTech)

models/weapons/SuppressorPistol/pistol_0
{
	q3map_nolightmap
    {
        map models/weapons/SuppressorPistol/pistol
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/SuppressorPistol/pistol_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
        tcMod scale 2 1
    }
}

models/weapons/SuppressorPistol/pistol
{
	q3map_nolightmap
    {
        map models/weapons/SuppressorPistol/pistol
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/SuppressorPistol/pistol_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

// T-21 Light Repeating Blaster (Pande)

models/weapons/t-21_repeater/t21_0
{
	q3map_nolightmap
    {
        map models/weapons/t-21_repeater/t21
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/t-21_repeater/t21_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/t-21_repeater/t21
{
	q3map_nolightmap
    {
        map models/weapons/t-21_repeater/t21
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/t-21_repeater/t21_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/weapons/t-21_repeater/bipod
{
	q3map_nolightmap
    {
        map envmap
        rgbGen lightingDiffuse
        tcGen environment
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

// W-90 Concussion Rifle (Reven, Suibuku)

models/weapons/w-90_concussion/w-90_0
{
	q3map_nolightmap
    {
        map models/weapons/w-90_concussion/w-90
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/w-90_concussion/w-90_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map textures/common/env_chrome
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        detail
        tcGen environment
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons/w-90_concussion/w-90
{
	q3map_nolightmap
    {
        map models/weapons/w-90_concussion/w-90
        rgbGen lightingDiffuse
    }
    {
        map models/weapons/w-90_concussion/w-90_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map textures/common/env_chrome
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        detail
        tcGen environment
    }
}

models/weapons/w-90_scope/scope
{
	q3map_nolightmap
    {
        map models/weapons/e-11_rifle/scope
        rgbGen lightingDiffuse
    }
    {
        map envmap
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        detail
        tcGen environment
        tcMod scale 2 2
    }
    {
        map models/weapons/scopelens_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
    }
}
