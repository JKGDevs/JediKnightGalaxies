textures/doomgiver/grating
{
    {
        map textures/imp_mine/grating
        alphaFunc GE128
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        depthWrite
        rgbGen identity
    }
    {
        map $lightmap
        alphaFunc GE128
        blendFunc GL_DST_COLOR GL_SRC_COLOR
    }
}

textures/doomgiver/forcefield
{
	qer_editorimage	textures/doomgiver/energything.tga
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	playerclip
	surfaceparm	shotclip
	surfaceparm	forcefield
	surfaceparm	trans
	q3map_nolightmap
    {
        map textures/doomgiver/energything2
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.65 0.35 0 0.2
        tcMod scroll -1 2
    }
    {
        map textures/doomgiver/energything2
        blendFunc GL_ONE GL_ONE
        tcMod scroll 2 1
    }
}

textures/doomgiver/oz_test
{
	qer_editorimage	textures/doomgiver/energything.tga
	surfaceparm	nomarks
	surfaceparm	forcefield
	q3map_nolightmap
    {
        animMap 1 textures/doomgiver/1 textures/doomgiver/2 textures/doomgiver/3 textures/doomgiver/4 textures/doomgiver/5 textures/doomgiver/6 textures/doomgiver/7 textures/doomgiver/8 
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.5 0.1 0.2 0.4
        tcMod scroll 10 15
    }
    {
        map gfx/effects/mp_b_forcefield_d
        blendFunc GL_DST_COLOR GL_ONE
        rgbGen wave sin 0.7 0.3 1 1
        tcMod scroll -20 -50
    }
}

textures/doomgiver/comp_panel
{
    {
        map $lightmap
    }
    {
        map textures/doomgiver/comp_panel
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        oneshotanimMap 1 textures/doomgiver/comp_panel1 textures/doomgiver/comp_panel1 
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        rgbGen const ( 1.000000 0.000000 0.000000 )
    }
}

textures/doomgiver/cfield
{
	qer_editorimage	textures/cairn/lightning3
	surfaceparm	nomarks
	surfaceparm	forcefield
	q3map_nolightmap
    {
        map textures/cairn/lightning3
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 0 0.5 0 1
        tcMod stretch sin 0 0.1 0 1
        tcMod scroll 0 5
        tcMod scale 0.1 0.1
    }
    {
        map textures/cairn/lightning3
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 0 0.5 0.3 1
        tcMod stretch sin 0 0.1 0.7 1
        tcMod scroll 0 -5
        tcMod scale 0.3 0.3
    }
    {
        map textures/cairn/lightning3
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 0 0.5 0.7 1
        tcMod scroll 5 0
        tcMod stretch sin 0 1 0 1
        tcMod scale 0.3 0.3
    }
}

textures/doomgiver/conduit_field
{
	qer_editorimage	textures/cairn/lightning3
	surfaceparm	nomarks
	surfaceparm	forcefield
	q3map_nolightmap
    {
        map textures/doomgiver/enrgy1
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.1 0.1 0 5
        tcMod scroll 0 -3
        tcMod scale 4 1
    }
    {
        map textures/doomgiver/enrgy1
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.1 0.1 0.3 4
        tcMod scroll 0 0.5
        tcMod scale 2 1
    }
    {
        map textures/cairn/lightning3
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.1 0.5 0.7 5
        tcMod scroll 5 0
        tcMod stretch sin 0 0.5 0 5
        tcMod scale 2 2
    }
    {
        map textures/doomgiver/enrgy1
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.25 0.1 0 10
        tcMod scroll 0 3
        tcMod scale 4 1
    }
    {
        map textures/doomgiver/enrgy1
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.1 0.1 -0.3 4
        tcMod scroll 0 -0.5
        tcMod scale 2 1
    }
}

textures/doomgiver/oz_test2
{
	qer_editorimage	textures/doomgiver/energything.tga
	surfaceparm	nomarks
	surfaceparm	forcefield
	q3map_nolightmap
    {
        animMap 1 gfx/test/one gfx/test/two gfx/test/three gfx/test/four gfx/test/five gfx/test/six gfx/test/seven gfx/test/eight 
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.5 0.1 0.2 0.4
        tcMod scroll 10 15
    }
    {
        map gfx/effects/mp_b_forcefield_d
        blendFunc GL_DST_COLOR GL_ONE
        rgbGen wave sin 0.7 0.3 1 1
        tcMod scroll -20 -50
    }
}

textures/doomgiver/trimabc2
{
    {
        map $lightmap
    }
    {
        map textures/doomgiver/trimabc2
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/doomgiver/trimabc2glow
        blendFunc GL_ONE GL_ONE
    }
}

textures/doomgiver/trimabc
{
    {
        map $lightmap
    }
    {
        map textures/doomgiver/trimabc
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/doomgiver/trimabcglow
        blendFunc GL_ONE GL_ONE
        glow
    }
}

textures/doomgiver/control_panel_03
{
	qer_editorimage	textures/doomgiver/control_panel_03
    {
        map textures/doomgiver/doom_display1
        blendFunc GL_ONE GL_ZERO
        tcMod scroll 0 0.05
        tcMod scale 1 0.5
    }
    {
        map textures/tests/energything2
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 1 0.25 0 4
        tcMod scroll 5 0.1
        tcMod scale 4 4
    }
    {
        animMap 1 textures/doomgiver/doom_display1 textures/doomgiver/screen09 textures/doomgiver/bluescreen textures/doomgiver/doom_screen1 textures/doomgiver/screen02 
        blendFunc GL_ONE GL_ONE
        tcMod scale 3.75 3
    }
    {
        map textures/doomgiver/control_panel_03
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_SRC_COLOR
    }
}

textures/doomgiver/comp_panel_alpha
{
	qer_editorimage	textures/doomgiver/comp_panel_alpha
    {
        map textures/doomgiver/screen06
        blendFunc GL_ONE GL_ZERO
        tcMod scroll 0.2 0
        tcMod scale 1 2.5
    }
    {
        map textures/doomgiver/screen06
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 1 0.5 0 1
        tcMod scroll 0.2 0
        tcMod scale 1 2.5
    }
    {
        map textures/tests/energything2
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 1 0.35 0 4
        tcMod scroll 0 0.25
        tcMod scale 3 3
    }
    {
        map textures/doomgiver/comp_panel_alpha
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_SRC_COLOR
    }
}

textures/doomgiver/comp_panel_new_2
{
	qer_editorimage	textures/doomgiver/comp_panel_new_2
    {
        map textures/doomgiver/doom_display1
        blendFunc GL_ONE GL_ZERO
        tcMod scale 3 1
    }
    {
        map textures/doomgiver/doom_display1
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.75 0.15 0 8
        tcMod scale 3 1
    }
    {
        map textures/tests/energything2
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 1 0.25 0 8
        tcMod scroll 0 0.1
        tcMod scale 10 10
    }
    {
        map textures/doomgiver/comp_panel_new_2
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_SRC_COLOR
    }
}

textures/doomgiver/comm_room
{
	qer_editorimage	textures/doomgiver/comm_room
    {
        map textures/doomgiver/energything
        blendFunc GL_ONE GL_ZERO
        rgbGen wave sin 1 0.5 0 5
        tcMod scroll 5 0.25
        tcMod scale 5 5
    }
    {
        map textures/doomgiver/comm_room
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map textures/doomgiver/comm_roomglow
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 1 0.05 0 8
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_SRC_COLOR
    }
}

textures/doomgiver/control_panel_02
{
	qer_editorimage	textures/doomgiver/control_panel_02
    {
        map textures/doomgiver/screen09
        blendFunc GL_ONE GL_ZERO
        rgbGen wave square 0.5 0.5 -0.5 0.5
        tcMod scroll 0 0.25
        tcMod scale 5 8
    }
    {
        map textures/tests/energything2
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 1 0.25 0 8
        tcMod scroll 5 0.1
        tcMod scale 8 8
    }
    {
        animMap 4 textures/doomgiver/screen01 textures/doomgiver/screen03 textures/doomgiver/screen05 
        blendFunc GL_ONE GL_ONE
        rgbGen wave square 0 1 0 0.5
        tcMod scale 3 3
    }
    {
        map textures/doomgiver/control_panel_02
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_SRC_COLOR
    }
}

textures/doomgiver/deathcon1a
{
	qer_editorimage	textures/doomgiver/deathcon1a
    {
        map textures/doomgiver/screen03
        blendFunc GL_ONE GL_ZERO
        tcMod scale 5.5 5.5
    }
    {
        map textures/doomgiver/screen03
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.5 0.25 0 6
        tcMod scale 5.5 5.5
    }
    {
        map textures/tests/energything2
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 1 0.15 0 8
        tcMod scroll 0 0.05
        tcMod scale 10 10
    }
    {
        map textures/doomgiver/deathcon1a
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_SRC_COLOR
    }
}

textures/doomgiver/redlightsd
{
    {
        map $lightmap
    }
    {
        map textures/doomgiver/redlightsd
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/doomgiver/redlightsdglow
        blendFunc GL_ONE GL_ONE
    }
}

textures/doomgiver/grate01d
{
    {
        map $lightmap
    }
    {
        map textures/doomgiver/grate01d
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/doomgiver/grate01dglow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 1 0.05 0 8
    }
}

textures/doomgiver/light1d
{
    {
        map $lightmap
    }
    {
        map textures/doomgiver/light1d
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/doomgiver/light1dglow
        blendFunc GL_ONE GL_ONE
        glow
    }
}

textures/doomgiver/doortrim01d
{
    {
        map $lightmap
    }
    {
        map textures/doomgiver/doortrim01d
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/doomgiver/doortrim01dglow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 1 0.15 0 8
    }
}

textures/doomgiver/512_512d
{
    {
        map $lightmap
    }
    {
        map textures/doomgiver/512_512d
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/doomgiver/512_512dglow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 1 0.15 0 0.5
    }
}

textures/doomgiver/shieldpaneloff
{
	qer_editorimage	textures/doomgiver/comp_panel_alpha
    {
        map textures/doomgiver/shieldon1
        blendFunc GL_ONE GL_ZERO
        tcMod scroll 0.2 0
        tcMod scale 2 2
    }
    {
        map textures/doomgiver/shieldon1
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 1 0.75 0 2
        tcMod scroll 0.2 0
        tcMod scale 3 3
    }
    {
        map textures/tests/energything2
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 1 0.35 0 4
        tcMod scroll 0 0.25
        tcMod scale 3 3
    }
    {
        map textures/doomgiver/comp_panel_alpha
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        animMap 0 textures/doomgiver/comp_panel_alphaglow textures/doomgiver/comp_panel_alphaglow3 
        blendFunc GL_ONE GL_ONE
        rgbGen wave square 1 0.75 0 1
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_SRC_COLOR
    }
}

textures/doomgiver/shieldpanel
{
	qer_editorimage	textures/doomgiver/comp_panel_alpha
    {
        map textures/tests/energything2
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 1 0.35 0 4
        tcMod scroll 1 0.25
        tcMod scale 3 3
    }
    {
        map textures/doomgiver/comp_panel_alpha
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        animMap 1 textures/doomgiver/comp_panel_alphaglow textures/doomgiver/comp_panel_alphaglow3 
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 1 0.5 0 0.5
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_SRC_COLOR
    }
}

textures/doomgiver/mapd
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map textures/doomgiver/mapd
        blendFunc GL_ONE GL_ONE
    }
}

textures/doomgiver/mapd2
{
	qer_editorimage	textures/doomgiver/mapd2
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map textures/doomgiver/mapd2
        blendFunc GL_ONE GL_ONE
    }
}

textures/doomgiver/control5
{
    {
        map $lightmap
    }
    {
        map textures/doomgiver/control5
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/doomgiver/control5_glow
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0 1 0 1
    }
}

textures/doomgiver/mapd1
{
	qer_editorimage	textures/doomgiver/mapd1
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map textures/doomgiver/mapd1
        blendFunc GL_ONE GL_ONE
    }
}

textures/doomgiver/doorhangar2
{
    {
        map $lightmap
    }
    {
        map textures/doomgiver/doorhangar2
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/doomgiver/doorhangar2glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave square 0 1 0 1
    }
}

textures/doomgiver/doom_screen1
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map textures/doomgiver/doom_screen1
        blendFunc GL_ONE GL_ONE
    }
}

textures/doomgiver/fuel1
{
	qer_editorimage	textures/doomgiver/comp_panel7
    {
        map $lightmap
    }
    {
        map textures/doomgiver/fuel1
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen wave sin 1 0.1 0.1 1
        tcMod scroll 0 0.5
        tcMod turb 1 -0.05 -1 0.25
    }
}

textures/doomgiver/pipe1
{
    {
        map textures/doomgiver/energy
        blendFunc GL_ONE GL_ZERO
        tcMod scroll 0 1
    }
    {
        map textures/doomgiver/energy
        blendFunc GL_ONE GL_ONE
        rgbGen wave noise 0.5 0.5 0 4
        tcMod scroll 0 1
    }
    {
        map textures/doomgiver/pipe1
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_SRC_COLOR
    }
}

textures/doomgiver/tramtrack
{
    {
        map textures/doomgiver/tramtrack
        blendFunc GL_ONE GL_ZERO
    }
    {
        map textures/doomgiver/enrgy1
        blendFunc GL_ONE GL_ONE
        rgbGen wave noise 0.25 0.25 0 5
        tcMod scroll 0 4
        tcMod scale 2 1
        tcMod turb 1 5 -0.5 5
    }
    {
        map textures/doomgiver/tramtrack
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map textures/doomgiver/tramtrackglow
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.5 1 0 1
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_SRC_COLOR
    }
}

textures/doomgiver/hyperspace
{
	qer_editorimage	textures/doomgiver/arrow
	q3map_nolightmap
    {
        map textures/doomgiver/hyper02
        blendFunc GL_SRC_ALPHA GL_ONE
        alphaGen entity
        tcMod scroll 1.5 0
        tcMod turb 0 0.1 0 0.1
    }
    {
        map textures/doomgiver/hyper01
        blendFunc GL_SRC_ALPHA GL_ONE
        alphaGen entity
        tcMod scroll 1 0
        tcMod turb 0 0.25 0 0.25
    }
}

//delete

textures/doomgiver/arrow
{
	qer_editorimage	textures/doomgiver/arrow
	q3map_nolightmap
    {
        map textures/doomgiver/hyper02
        blendFunc GL_SRC_ALPHA GL_ONE
        alphaGen entity
        tcMod scroll 1.5 0
        tcMod turb 0 0.1 0 0.1
    }
    {
        map textures/doomgiver/hyper01
        blendFunc GL_SRC_ALPHA GL_ONE
        alphaGen entity
        tcMod scroll 1 0
        tcMod turb 0 0.25 0 0.25
    }
}

textures/doomgiver/deathconlight
{
    {
        map $lightmap
    }
    {
        map textures/doomgiver/deathconlight
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/doomgiver/deathconlight
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 0.75 0.005 0 10
    }
}

textures/doomgiver/bigpipe1d
{
	qer_editorimage	textures/doomgiver/bpipe2d
    {
        map $lightmap
    }
    {
        map textures/doomgiver/bpipe2d
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/doomgiver/energything
        blendFunc GL_ONE GL_ONE
        rgbGen wave noise 0.25 0.25 0 5
        tcMod scroll 5 4
        tcMod scale 100 2
    }
    {
        map textures/doomgiver/energything2
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.1 0.25 -0.5 10
        tcMod scroll -3 -4
        tcMod scale 100 1
    }
}

textures/doomgiver/flare_blue
{
// deformvertexes	autoSprite

	surfaceparm	nonsolid
	surfaceparm	trans
	q3map_nolightmap
    {
        map textures/doomgiver/flare_blue
        blendFunc GL_SRC_ALPHA GL_ONE
        rgbGen wave noise 1 0.2 0 25
        alphaGen entity
    }
}

textures/doomgiver/hyper03_b
{
	qer_editorimage	textures/doomgiver/arrow2
	q3map_nolightmap
	sort	opaque
    {
        map textures/doomgiver/hyper03
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        tcMod scroll 1.5 0
    }
}

textures/doomgiver/light_whited
{
    {
        map $lightmap
    }
    {
        map textures/doomgiver/light_whited
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/doomgiver/light_whited_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 0.75 0.05 0 5
    }
}

textures/doomgiver/door_red
{
	qer_editorimage	textures/doomgiver/door1d
    {
        map $lightmap
    }
    {
        map textures/doomgiver/door1d
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/doomgiver/door1d_red
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 1 0.05 0 5
    }
}

textures/doomgiver/door1_locked
{
	qer_editorimage	textures/doomgiver/door1
    {
        map $lightmap
    }
    {
        map textures/doomgiver/door1
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/doomgiver/door1_red
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 1 0.05 0 5
    }
}

textures/doomgiver/doomscreen
{
	q3map_nolightmap
    {
        map textures/doomgiver/doomscreen
        blendFunc GL_ONE GL_ONE
    }
}

textures/doomgiver/door_opener
{
	qer_editorimage	textures/doomgiver/door1d
    {
        map $lightmap
    }
    {
        map textures/doomgiver/door1d
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        oneshotanimMap 1 textures/doomgiver/door1d_red textures/doomgiver/door1d_green 
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 1 0.05 0 5
    }
}

textures/doomgiver/pipe2
{

    {
        map $lightmap
    }
    {
        map textures/doomgiver/pipe2
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/doomgiver/largepipe
{

    {
        map $lightmap
    }
    {
        map textures/doomgiver/largepipe
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/doomgiver/basic2
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/doomgiver/basic2
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/doomgiver/basic
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/doomgiver/basic
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/doomgiver/floorgrate1
{
	q3map_material	HollowMetal
    {
        map $lightmap
    }
    {
        map textures/doomgiver/floorgrate1
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/doomgiver/floord
{
	q3map_material	HollowMetal
    {
        map $lightmap
    }
    {
        map textures/doomgiver/floord
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/doomgiver/metalsec1d
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/doomgiver/metalsec1d
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/doomgiver/new5d
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/doomgiver/new5d
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/doomgiver/new3d
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/doomgiver/new3d
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/doomgiver/new2d
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/doomgiver/new2d
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

