textures/nar_streets/wall01
{
    {
        map $lightmap
    }
    {
        map textures/nar_streets/wall01
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/nar_streets/winlite
        blendFunc GL_ONE GL_ONE_MINUS_SRC_COLOR
    }
}

textures/nar_streets/baselight1
{
    {
        map $lightmap
    }
    {
        map textures/nar_streets/baselight1
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/nar_streets/baselight1_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/nar_streets/baselight2
{
    {
        map $lightmap
    }
    {
        map textures/nar_streets/baselight2
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/nar_streets/baselight2_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/nar_streets/trimlights1
{
    {
        map $lightmap
    }
    {
        map textures/nar_streets/trimlights1
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/nar_streets/trimlights1_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/nar_streets/tracklight
{
    {
        map $lightmap
    }
    {
        map textures/nar_streets/tracklight
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/nar_streets/tracklight_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/nar_streets/wall08
{
    {
        map $lightmap
    }
    {
        map textures/nar_streets/wall08
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/nar_streets/wall08_glow
        blendFunc GL_ONE GL_ONE_MINUS_SRC_COLOR
    }
}

textures/nar_streets/tracklight_streets
{
    {
        map $lightmap
    }
    {
        map textures/nar_streets/tracklight_streets
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/nar_streets/tracklight_streets_glow
        blendFunc GL_ONE GL_ONE
    }
}

textures/nar_streets/barswitch
{
    {
        map $lightmap
    }
    {
        map textures/nar_streets/barswitch
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/nar_streets/barswitchglowr
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 1 1 0 1
    }
}

textures/nar_streets/barswitchon
{
	qer_editorimage	textures/nar_streets/barswitch
    {
        map $lightmap
    }
    {
        map textures/nar_streets/barswitch2
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/nar_streets/barswitchglow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 1 0.05 0 5
    }
}

textures/nar_streets/forceswitchred
{
	qer_editorimage	textures/nar_streets/forceswitch
    {
        map $lightmap
    }
    {
        map textures/nar_streets/forceswitch
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/nar_streets/forceswitcha
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 0 1 0 1
    }
    {
        map textures/nar_streets/forceswitcha2
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 1 0.05 0 5
    }
}

textures/nar_streets/forceswitchblue
{
	qer_editorimage	textures/nar_streets/forceswitch
    {
        map $lightmap
    }
    {
        map textures/nar_streets/forceswitch
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/nar_streets/forceswitchb
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 1 0.05 0 5
    }
}

textures/nar_streets/eleswitch
{
    {
        map $lightmap
    }
    {
        map textures/nar_streets/eleswitch
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/nar_streets/eleswitcha
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 0 1 0 1
    }
    {
        map textures/nar_streets/eleswitchb
        blendFunc GL_ONE GL_ONE
        glow
    }
}

textures/nar_streets/eleswitchon
{
	qer_editorimage	textures/nar_streets/eleswitch
    {
        map $lightmap
    }
    {
        map textures/nar_streets/eleswitch
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/nar_streets/eleswitcha
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0 1 -0.5 1
    }
    {
        map textures/nar_streets/eleswitchb
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 0 1 0 1
    }
}

textures/nar_streets/ns_screen_green
{
    {
        map $lightmap
    }
    {
        map textures/nar_streets/ns_screen_green
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/nar_streets/ns_screen_green_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave triangle 0 1 0 2
    }
}

textures/nar_streets/flashred_glow
{
	qer_editorimage	textures/nar_streets/flashred
    {
        map $lightmap
    }
    {
        map textures/nar_streets/flashred
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/nar_streets/flashred_glow
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0 1 0 1
    }
}

textures/nar_hideout/lightshaft
{
    {
        map $lightmap
    }
    {
        map textures/nar_hideout/lightshaft
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/nar_hideout/lightshaft_glow
        blendFunc GL_ONE GL_ONE
        glow
    }
}

textures/nar_hideout/pipe1_ref
{
	qer_editorimage	textures/nar_hideout/pipe1
	surfaceparm	forcefield
    {
        map $lightmap
    }
    {
        map textures/nar_hideout/pipe1
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/nar_hideout/nscameraswitch
{
	qer_editorimage	textures/nar_hideout/nscameraswitch
    {
        map gfx/effects/decoystatic
        blendFunc GL_ONE GL_ZERO
        rgbGen wave noise 1 0.75 0 5
        tcMod scroll 5 2
        tcMod scale 7 9
    }
    {
        map gfx/effects/decoystatic
        blendFunc GL_ONE GL_ONE
        rgbGen wave noise 1 0.5 -0.5 1
        tcMod scroll -2 -2
        tcMod scale 1 9
    }
    {
        map textures/nar_hideout/nscameraswitch
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_SRC_COLOR
    }
    {
        map textures/nar_hideout/nscameraswitchglow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave square 0 1 0 1
    }
}

textures/narshaddaa/tunnellights
{
    {
        map $lightmap
    }
    {
        map textures/narshaddaa/tunnellights
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/narshaddaa/tunnellights_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/narshaddaa/ll_tracklight
{
    {
        map $lightmap
    }
    {
        map textures/narshaddaa/ll_tracklight
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/narshaddaa/ll_tracklight_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/narshaddaa/symbols
{
	qer_editorimage	textures/narshaddaa/symbol1
	cull	twosided
    {
        map $lightmap
    }
    {
        animMap 4 textures/narshaddaa/fuelpump1 textures/narshaddaa/fuelpump2 textures/narshaddaa/fuelpump3 textures/narshaddaa/fuelpump4 
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen vertex
    }
    {
        map textures/narshaddaa/fuelpump_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave square 0 1 0 1
    }
}

textures/narshaddaa/ll_smalllights
{
    {
        map $lightmap
    }
    {
        map textures/narshaddaa/ll_smalllights
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/narshaddaa/ll_smalllights_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/narshaddaa/light1
{
    {
        map $lightmap
    }
    {
        map textures/narshaddaa/light1
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/narshaddaa/light1_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/narshaddaa/fuelpump1
{
    {
        map $lightmap
    }
    {
        map textures/narshaddaa/fuelpump1
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/narshaddaa/fuelpump_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 0 7 0 1
    }
}

textures/narshaddaa/fuelpump2
{
    {
        map $lightmap
    }
    {
        map textures/narshaddaa/fuelpump2
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/narshaddaa/fuelpump_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 0 7 0 1
    }
}

textures/narshaddaa/fuelpump3
{
    {
        map $lightmap
    }
    {
        map textures/narshaddaa/fuelpump3
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/narshaddaa/fuelpump_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 0 7 0 1
    }
}

textures/narshaddaa/fuelpump4
{
    {
        map $lightmap
    }
    {
        map textures/narshaddaa/fuelpump4
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/narshaddaa/fuelpump_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 0 7 0 1
    }
}

textures/narshaddaa/piller2_noclip
{
	qer_editorimage	textures/narshaddaa/piller2
	surfaceparm	nonsolid
    {
        map $lightmap
    }
    {
        map textures/narshaddaa/piller2
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

