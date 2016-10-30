// JKG - replacement for the Force Sense overlay layer on players - notice that player models still turn yellow

powerups/sightshell
{
	qer_editorimage	gfx/effects/chrome
	polygonOffset
//	deformvertexes	bulge	0 1 0
    {
        map $whiteimage
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        rgbGen const ( 0.5 0.6 1.0 )
        alphaGen lightingSpecular
    }
}

gfx/mp/chat_icon
{
	nopicmip
	nomipmaps
    {
        clampmap gfx/jkg/icon_chat
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

// __________________________________

gfx/misc/blue_portashield
{
	surfaceparm	noimpact
	surfaceparm	trans
	nopicmip
	q3map_nolightmap
	cull	twosided
    {
        map gfx/effects/mp_b_forcefield
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.65 0.35 0 0.2
        tcMod scroll -1 2
    }
    {
        map gfx/effects/mp_b_forcefield
        blendFunc GL_ONE GL_ONE
        tcMod scroll 2 1
    }
}

gfx/misc/red_portashield
{
	surfaceparm	noimpact
	surfaceparm	trans
	nopicmip
	q3map_nolightmap
	cull	twosided
    {
        map gfx/effects/mp_r_forcefield
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.65 0.35 0 0.2
        tcMod scroll -1 2
    }
    {
        map gfx/effects/mp_r_forcefield
        blendFunc GL_ONE GL_ONE
        tcMod scroll 2 1
    }
}

gfx/misc/blue_dmgshield
{
	qer_editorimage	gfx/effects/mp_b_forcefield_d
	surfaceparm	noimpact
	surfaceparm	trans
	nopicmip
	q3map_nolightmap
	cull	twosided
    {
        map gfx/effects/mp_b_forcefield_d
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.65 0.35 0 0.2
        tcMod scroll -1 2
    }
    {
        map gfx/effects/mp_b_forcefield_d1
        blendFunc GL_ONE GL_ONE_MINUS_SRC_COLOR
        tcMod scroll 2 1
    }
}

gfx/misc/sightbubble
{
	deformvertexes	bulge	0 2 0
    {
        map gfx/effects/chrome
        blendFunc GL_DST_COLOR GL_ONE
        rgbGen entity
        tcGen environment
        tcMod rotate 200
        tcMod turb 0.6 0.3 0 0.6
    }
}

powerups/placeholder
{
	surfaceparm	trans
	q3map_nolightmap
    {
        map gfx/effects/mp_weapon_holo
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.5 0.2 0 0.7
        tcMod scroll 0.8 1.2
    }
    {
        map gfx/effects/mp_weapon_holo1
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.65 0.35 0 0.2
    }
    {
        map gfx/effects/mp_weapon_holo2
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.5 0.2 3 2
        tcMod scroll 0 1
    }
}

powerups/rezout
{
	qer_editorimage	gfx/effects/mp_weapon_holo2
	surfaceparm	trans
	q3map_nolightmap
    {
        map gfx/effects/mp_weapon_holo
        blendFunc GL_ONE GL_ONE
        rgbGen wave random 0.1 0.25 0 0.7
        tcMod scroll 4.3 4.7
    }
    {
        map gfx/effects/mp_weapon_holo1
        blendFunc GL_ONE GL_ONE
    }
    {
        map gfx/effects/mp_weapon_holo2
        blendFunc GL_ONE GL_ONE
        rgbGen entity
        tcMod scale 2 2
        tcMod scroll -0.4 0.7
    }
}

powerups/ysalimarishell
{
	q3map_material	SolidMetal
	deformvertexes	wave	100 sin 0 1 0 1
    {
        map gfx/mp/ysalshell
        blendFunc GL_ONE GL_ONE
        rgbGen const ( 0.200000 0.200000 0.000000 )
        tcMod rotate 30
    }
    {
        map gfx/mp/ysalshell
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen const ( 0.200000 0.200000 0.000000 )
        tcMod rotate -18
        tcMod turb 1 0.005 0 1
    }
}

powerups/boonshell
{
	qer_editorimage	gfx/mp/ysalshell
	q3map_material	SolidMetal
    {
        map gfx/mp/boonshell
        blendFunc GL_ONE GL_ONE
        rgbGen const ( 0.200000 0.200000 0.200000 )
        tcMod rotate 10
    }
    {
        map gfx/mp/boonshell
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.2 0.1 0 1
        tcMod rotate -7
    }
}

powerups/invulnerabilityshell
{
	qer_editorimage	gfx/mp/ysalshell
	q3map_material	SolidMetal
    {
        map gfx/mp/invulnshell
        blendFunc GL_SRC_ALPHA GL_ONE
        rgbGen const ( 0.000000 0.500000 0.000000 )
        alphaGen wave triangle 1 0.5 0 0.5
        tcMod rotate -30
        tcMod stretch sin 1.2 0.2 0 0.1
    }
}

powerups/enlightenmentshell
{
	qer_editorimage	gfx/mp/ysalshell
	q3map_material	SolidMetal
    {
        map gfx/mp/enlightshell
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen const ( 0.200000 0.200000 0.200000 )
        tcMod rotate 10
    }
    {
        map gfx/mp/enlightshell
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 0.2 0.05 0 1
        tcMod rotate -7
    }
}

powerups/endarkenmentshell
{
	qer_editorimage	gfx/mp/ysalshell
    {
        map gfx/mp/enlightshell
        blendFunc GL_ZERO GL_ONE_MINUS_SRC_COLOR
        rgbGen const ( 0.700000 0.700000 0.700000 )
        tcMod rotate 10
    }
    {
        map gfx/mp/enlightshell
        blendFunc GL_ZERO GL_ONE_MINUS_SRC_COLOR
        rgbGen wave sin 0.75 0.25 0 1
        tcMod rotate -7
    }
}

powerups/ysaliredshell
{
	qer_editorimage	gfx/mp/ysalshell
	deformvertexes	wave	100 sin 0 1 0 1
    {
        map gfx/mp/ysalshell
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen const ( 0.400000 0.000000 0.000000 )
        tcMod rotate 30
    }
    {
        map gfx/mp/ysalshell
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen const ( 0.300000 0.000000 0.000000 )
        tcMod rotate -18
        tcMod turb 1 0.005 0 1
    }
}

powerups/ysaliblueshell
{
	qer_editorimage	gfx/mp/ysalshell
	q3map_material	SolidMetal
	deformvertexes	wave	100 sin 0 1 0 1
    {
        map gfx/mp/ysalshell
        blendFunc GL_ONE GL_ONE
        rgbGen const ( 0.000000 0.000000 0.300000 )
        tcMod rotate 30
    }
    {
        map gfx/mp/ysalshell
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen const ( 0.100000 0.100000 0.400000 )
        tcMod rotate -18
        tcMod turb 1 0.005 0 1
    }
}

powerups/forceshell
{
	qer_editorimage	gfx/effects/chrome
	deformvertexes	bulge	0 2 0
    {
        map gfx/mp/forceshell
        blendFunc GL_DST_COLOR GL_ONE
        rgbGen entity
        tcGen environment
        tcMod rotate 100
        tcMod turb 0.6 0.3 0 0.6
        tcMod scale 0.3 0.3
    }
}

powerups/sightshell_old
{
	qer_editorimage	gfx/effects/chrome
	deformvertexes	bulge	0 2 0
    {
        map gfx/mp/sightshell
        blendFunc GL_ONE GL_ONE
        rgbGen entity
        tcGen environment
        tcMod scale 0.35 0.35
        tcMod scroll 0.1 -0.1
    }
    {
        map gfx/mp/sightshell
        blendFunc GL_ONE GL_ONE
        rgbGen entity
        tcGen environment
        tcMod scale 0.35 0.35
        tcMod scroll 0.1 0.1
    }
}

gfx/effects/turretflashdie
{
	surfaceparm	trans
	nomipmaps
    {
        map gfx/effects/solidwhite
        blendFunc GL_ONE GL_ONE
        rgbGen entity
    }
}

sprites/imperial_attack
{
	qer_editorimage	gfx/2d/mp_imp_symbol
	nopicmip
	nomipmaps
    {
        map gfx/2d/mp_imp_symbol
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

sprites/imperial_defend
{
	qer_editorimage	gfx/2d/mp_imp_symbol
	nopicmip
	nomipmaps
    {
        map gfx/2d/mp_imp_symbol_2
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

sprites/rebel_attack
{
	qer_editorimage	gfx/2d/mp_imp_symbol
	nopicmip
	nomipmaps
    {
        map gfx/2d/mp_rebel_symbol_2
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

sprites/rebel_defend
{
	qer_editorimage	gfx/2d/mp_imp_symbol
	nopicmip
	nomipmaps
    {
        map gfx/2d/mp_rebel_symbol
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

halfShieldShell
{
	qer_editorimage	gfx/effects/shieldshell1.tga
	nomipmaps
	polygonOffset
	cull	twosided
    {
        map gfx/effects/shieldshell1
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen entity
        tcMod stretch sin 1 0.1 0 2
        tcMod rotate 55
    }
    {
        map gfx/effects/shieldshellring
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave inversesawtooth 0 1 0 1.5
        tcMod stretch sawtooth 0.6 4 0 1.5
    }
}

halfShieldZapShell
{
	qer_editorimage	gfx/effects/shieldshell1.tga
	nomipmaps
	polygonOffset
	cull	twosided
    {
        map gfx/effects/shieldshell1
        blendFunc GL_DST_COLOR GL_ONE
        rgbGen entity
        tcMod stretch sin 1 0.1 0 2
        tcMod rotate 55
    }
    {
        map gfx/effects/shieldshellring
        blendFunc GL_ONE GL_ONE
        rgbGen wave random 0.2 0.4 0 1.5
        tcMod stretch sawtooth 0.6 4 0 1.5
    }
}

gfx/mp/f_icon_ragerec
{
    {
        map gfx/mp/f_icon_ragerec
        blendFunc GL_ONE_MINUS_DST_COLOR GL_ONE_MINUS_SRC_COLOR
        rgbGen wave triangle 0.5 0.5 0 2
    }
}

gfx/mp/dmgshader_shields
{
	cull	twosided
    {
        map gfx/mp/dmgshader_shields
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen vertex
    }
}

gfx/mp/dmgshader_shieldsandhealth
{
	cull	twosided
    {
        map gfx/mp/dmgshader_shieldsandhealth
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen vertex
    }
}

gfx/mp/vchat_icon
{
	nopicmip
	nomipmaps
    {
        clampmap gfx/mp/voice_icon
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

gfx/menus/radar/goalitem
{
	nopicmip
	notc
	q3map_nolightmap
    {
        map gfx/menus/radar/goalitem
        blendFunc GL_ONE GL_ONE
        rgbGen identity
        tcMod rotate -6
    }
    {
        map gfx/menus/radar/goalitem
        blendFunc GL_ONE GL_ONE
        rgbGen identity
        tcMod rotate 5
    }
}

gfx/menus/radar/radar
{
	nopicmip
	notc
	q3map_nolightmap
    {
        map gfx/menus/radar/radar
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map gfx/menus/radar/radar_glow
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

gfx/menus/radar/arrow_w
{
	nopicmip
	notc
	q3map_nolightmap
    {
        map gfx/menus/radar/arrow_w
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

models/map_objects/mp/flag_base
{
    {
        map $lightmap
    }
    {
        map models/map_objects/mp/flag_base
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map models/map_objects/mp/flag_base_glw
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

models/map_objects/mp/emitter_dish
{
    {
        map models/map_objects/mp/emitter_dish
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/mp/emitter_dish_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

models/map_objects/mp/emitter
{
    {
        map models/map_objects/mp/emitter
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/mp/emitter_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/mp/t_wall_halllights
{
    {
        map $lightmap
    }
    {
        map textures/mp/t_wall_halllights
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/mp/t_wall_halllights_glw
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

textures/mp/t_wall_lightpanel
{
    {
        map $lightmap
    }
    {
        map textures/mp/t_wall_lightpanel
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/mp/t_wall_lightpanel_glw
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

textures/mp/t_floor_hallway
{
	surfaceparm	forcefield
    {
        map $lightmap
    }
    {
        map textures/mp/t_floor_hallway
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/common/environ11
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        tcGen environment
        tcMod scale 3 3
    }
}

textures/mp/t_floor_corner
{
	surfaceparm	forcefield
    {
        map $lightmap
    }
    {
        map textures/mp/t_floor_corner
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/common/environ11
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        tcGen environment
        tcMod scale 3 3
    }
}

textures/mp/t_floor_single
{
	surfaceparm	forcefield
    {
        map $lightmap
    }
    {
        map textures/mp/t_floor_single
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/common/environ11
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        tcGen environment
        tcMod scale 3 3
    }
}

textures/mp/t_floor_cir
{
	surfaceparm	forcefield
    {
        map $lightmap
    }
    {
        map textures/mp/t_floor_cir
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/common/environ11
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        tcGen environment
        tcMod scale 3 3
    }
}

textures/mp/t_floor_sides
{
	surfaceparm	forcefield
    {
        map $lightmap
    }
    {
        map textures/mp/t_floor_sides
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/common/environ11
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        tcGen environment
        tcMod scale 3 3
    }
}

textures/mp/t_floor_rim
{
	surfaceparm	forcefield
    {
        map $lightmap
    }
    {
        map textures/mp/t_floor_rim
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/common/environ11
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        tcGen environment
        tcMod scale 3 3
    }
}

textures/mp/t_wall_lightpanel2
{
    {
        map $lightmap
    }
    {
        map textures/mp/t_wall_lightpanel2
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/mp/t_wall_lightpanel2_glw
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

