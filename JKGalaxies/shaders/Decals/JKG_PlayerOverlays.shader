// JKGalaxies Damage Type Body Overlay Shaders by BlasTech
// ___________________________________________________

gfx/PlayerOverlays/ice
{
	q3map_material	Ice
	q3map_nolightmap
	cull	twosided
	deformvertexes	bulge	0 1 0
    {
        map envmap
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        tcGen environment
    }
    {
        map gfx/playeroverlays/ice
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen identity
    }
}

gfx/PlayerOverlays/carbonite
{
	polygonOffset
	q3map_nolightmap
	sort	decal
    {
        map gfx/PlayerOverlays/carbonite
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen identity
    }
    {
        map envmap_spec
        blendFunc GL_DST_COLOR GL_ONE
        detail
        tcGen environment
    }
}

gfx/PlayerOverlays/stun
{
	qer_editorimage	gfx/effects/p_shield
	deformvertexes	bulge	0 0.5 0
    {
        map $whiteimage
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen const ( 0.024 0.023 0.25 )
    }
}

gfx/PlayerOverlays/electricity
{
	qer_editorimage	gfx/PlayerOverlays/electricity
	cull	twosided
	deformvertexes	bulge	0 0.5 0
    {
        map gfx/PlayerOverlays/electricity
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
        tcMod scroll 0.5 1
        tcMod scale 2 2
    }
}

gfx/PlayerOverlays/electricity2
{
	cull	twosided
	deformvertexes	bulge	0 0.25 0
    {
        map gfx/PlayerOverlays/electricity2
        blendFunc GL_ONE GL_ONE
        rgbGen identity
        tcMod scroll 0.7 1.2
        tcMod scale -2 -2
    }
}

// Cell Shading by Shader "Trial", lookin' good! :P
// Keep in mind that only one depthFunc may active at the time, hence the "//" commentation that disables that particular line. 

gfx/PlayerOverlays/cartoon
{
	qer_editorimage	gfx/effects/p_shield
	cull	back	// Just for the sheer hell of it, you can paint your character all black changing this line... Don't. :P
	deformvertexes	bulge	0 0.75 0	// Alter if you think the border is too thick or thin. 
    {
        map textures/colors/black
        depthFunc disable	// Time to pick techique... Disable gives the best look on the character, but effects, items and skies shine through. 
//        depthFunc lequal	// Less than Equal (default) doesn't have these issues, but the border shows up on all edges including mouth and eyes. 
        rgbGen identity
        alphaGen identity
    }
}

gfx/PlayerOverlays/shiny
{
	polygonOffset
	q3map_nolightmap
	sort	decal
    {
        map envmap
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        tcGen environment
    }
}

gfx/PlayerOverlays/fire
{
	deformvertexes	bulge	0 0.3 0
    {
        map gfx/PlayerOverlays/fire
        blendFunc GL_ONE GL_ONE
        glow
        tcMod scale 2 1
        tcMod scroll 1 0.6
    }
}

gfx/PlayerOverlays/burn
{
	polygonOffset
	q3map_nolightmap
	sort	decal
    {
        map gfx/PlayerOverlays/burn
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        alphaGen vertex
    }
}

// G2Marks drawn on players by lightsaber blades

gfx/burnmarks/burnline
{
	polygonOffset
    {
        map gfx/burnmarks/burnline
        blendFunc GL_DST_COLOR GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map gfx/burnmarks/burnline_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave noise 1 0.6 0 0.1
    }
}

gfx/burnmarks/projmark3_clamp
{
	surfaceparm	nomarks
	surfaceparm	trans
	polygonOffset
	q3map_nolightmap
    {
        clampmap gfx/burnmarks/projmark3
        blendFunc GL_DST_COLOR GL_ONE_MINUS_SRC_ALPHA
        alphaGen vertex
    }
}

gfx/burnmarks/projmark2_glow_clamp
{
	qer_editorimage	gfx/damage/bodyburnmark_glow
	surfaceparm	nomarks
	surfaceparm	trans
	polygonOffset
	q3map_nolightmap
    {
        clampmap gfx/burnmarks/projmark2
        blendFunc GL_DST_COLOR GL_ONE_MINUS_SRC_ALPHA
        alphaGen vertex
    }
    {
        clampmap gfx/burnmarks/projmark2_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave noise 1 0.6 0 0.35
    }
}

// Screen overlay static, used by the EC Detonator/BaseJKA DEMP2 alternate detonation

gfx/PlayerOverlays/EMP_screenflash
{
	qer_editorimage	gfx/effects/mp_b_forcefield_d
	surfaceparm	noimpact
	surfaceparm	trans
	nopicmip
	q3map_nolightmap
	cull	twosided
    {
        map gfx/effects/mp_b_forcefield_d
        blendFunc GL_DST_COLOR GL_ONE
        rgbGen wave sin 0.65 0.35 0 0.2
        tcMod scroll -1 -2
    }
}

gfx/PlayerOverlays/EMP_screenflash2
{
	qer_editorimage	gfx/effects/mp_b_forcefield_d
	surfaceparm	noimpact
	surfaceparm	trans
	nopicmip
	q3map_nolightmap
	cull	twosided
    {
        map gfx/effects/mp_b_forcefield_d1
        blendFunc GL_ONE GL_ONE_MINUS_SRC_COLOR
        glow
        rgbGen wave noise 0 1 0 1
        tcMod scroll 3 -1.6
        tcMod turb 0 0.7 0 0.2
    }
}

gfx/PlayerOverlays/EMP_screenflash3
{
	qer_editorimage	gfx/effects/mp_b_forcefield_d
	surfaceparm	noimpact
	surfaceparm	trans
	nopicmip
	q3map_nolightmap
	cull	twosided
    {
        map gfx/effects/mp_b_forcefield_d
        blendFunc GL_DST_COLOR GL_ONE
        glow
        rgbGen wave noise 0 1 0 1
        tcMod scroll 1 -2
        tcMod turb 0 0.5 0 0.6
    }
}

gfx/PlayerOverlays/metal
{
	polygonOffset
	q3map_nolightmap
	sort	decal
    {
        map textures/common/metalflake_rust
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen identity
    }
    {
        map envmap_spec
        blendFunc GL_DST_COLOR GL_ONE
        detail
        tcGen environment
    }
}

// Last modified by: Futuza on 2026-05-2026
// Previous update by: BlasTech on 2010-11-22