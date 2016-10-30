// GRATES

textures/factory/T2_Wedge_floorgrate
{
	qer_editorimage	textures/imperial/floorgrate
	cull	twosided
    {
// blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA

// depthWrite

        map textures/imperial/floorgrate
        alphaFunc GE128
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        depthWrite
        rgbGen identity
    }
    {
// blendFunc GL_DST_COLOR GL_ZERO

// depthFunc equal

        map $lightmap
        alphaFunc GE128
        blendFunc GL_DST_COLOR GL_SRC_COLOR
    }
}

textures/factory/wall10
{
    {
        map $lightmap
    }
    {
        map textures/factory/wall10
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/cfoster/etest1
        blendFunc GL_DST_COLOR GL_ONE
        tcGen environment
    }
    {
        map textures/factory/wall10env
        blendFunc GL_SRC_ALPHA GL_ONE
        alphaGen lightingSpecular
    }
}

textures/factory/wall
{
    {
        map gfx/effects/redlighting2
        tcMod scroll 0 0.5
        tcMod scale 1 2
    }
    {
        map gfx/effects/redlighting2
        blendFunc GL_ONE_MINUS_DST_COLOR GL_ONE
        glow
        tcMod scroll 0 1
    }
    {
        map textures/factory/wall
        blendFunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map $lightmap
        blendFunc GL_ZERO GL_SRC_COLOR
    }
}

textures/factory/env_siding
{
	qer_editorimage	textures/factory/siding1
    {
        map textures/factory/enviro
        tcGen environment
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_SRC_COLOR
    }
    {
        map textures/factory/siding1
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/factory/cat_floor
{
	q3map_material	HollowMetal
	q3map_nolightmap
	cull	twosided
    {
        map textures/factory/cat_floor
        blendFunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
        depthWrite
        rgbGen vertex
        tcMod scale 2 2
    }
}

textures/factory/basic2_tiled
{
	qer_editorimage	textures/factory/basic2
	q3map_nolightmap
    {
        map textures/factory/basic2
        rgbGen vertex
        tcMod scale 7 7
    }
}

textures/factory/wall2
{
    {
        map gfx/effects/fl_blue2orange
        tcMod scroll 0 2
        tcMod stretch sin 0.7 0.01 0 1
    }
    {
        map textures/factory/wall2
        blendFunc GL_ONE GL_SRC_ALPHA
    }
    {
        map $lightmap
        blendFunc GL_ZERO GL_SRC_COLOR
    }
}

textures/factory/cat_floor2
{
	q3map_material	HollowMetal
	q3map_nolightmap
	q3map_alphashadow
    {
        map textures/factory/cat_floor2
        blendFunc GL_ONE GL_SRC_ALPHA
        depthWrite
        rgbGen vertex
        tcMod scale 4 4
    }
}

textures/factory/basic4_railing
{
	qer_editorimage	textures/factory/basemetal1
	q3map_nolightmap
    {
        map textures/factory/basemetal1
        rgbGen vertex
    }
    {
        map textures/factory/enviro
        blendFunc GL_DST_COLOR GL_ONE
        tcGen environment
    }
}

textures/factory/tracklight
{
    {
        map $lightmap
    }
    {
        map textures/factory/tracklight
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/factory/tracklight_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 0 1 0 0.5
    }
}

textures/factory/wallliner
{
    {
        map textures/factory/wallliner_energya
        tcMod scroll 2.5 0
    }
    {
        map textures/factory/wallliner_energyb
        blendFunc GL_ONE GL_ONE
        glow
        tcMod scroll 1 0
    }
    {
        map textures/factory/wallliner
        blendFunc GL_ONE_MINUS_SRC_ALPHA GL_SRC_ALPHA
    }
    {
        map $lightmap
        blendFunc GL_ZERO GL_SRC_COLOR
    }
}

textures/factory/conveyor_belt
{
	q3map_material	SolidMetal
    {
        map $lightmap
        tcMod entityTranslate
    }
    {
        map textures/factory/conveyor_belt
        blendFunc GL_DST_COLOR GL_ZERO
        tcMod scroll 0 1.13
    }
}

textures/factory/deconfield1
{
	surfaceparm	forcefield
	q3map_nolightmap
    {
        map textures/factory/deconfield2
        blendFunc GL_ONE GL_ONE
        tcMod scroll 0.25 0.45
    }
    {
        map textures/factory/deconfield2
        blendFunc GL_ONE GL_ONE
        tcMod scroll 0.1 0.25
    }
    {
        map textures/factory/deconfield1
        blendFunc GL_ONE GL_ONE_MINUS_SRC_COLOR
    }
}

textures/factory/tickertape
{
	q3map_nolightmap
	cull	twosided
    {
        map textures/factory/tickertape
        blendFunc GL_ONE GL_ONE
    }
    {
        map textures/factory/tickertape2
        blendFunc GL_ONE GL_ONE
        tcMod scroll 0.075 0
    }
    {
        map textures/factory/tickertape3
        blendFunc GL_ONE GL_ONE
        tcMod scroll 0.05 0
    }
}

textures/factory/trimlight
{
    {
        map $lightmap
    }
    {
        map textures/factory/trimlight
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/factory/trimlight_glow
        blendFunc GL_ONE GL_ONE
        glow
    }
}

textures/factory/prison_field
{
	q3map_nolightmap
    {
        map textures/factory/prison_field
        tcMod scroll 0 1
    }
    {
        map textures/factory/prison_field2
        blendFunc GL_ONE GL_ONE
        tcMod scroll 2 0.5
    }
    {
        map textures/factory/prison_field3
        blendFunc GL_ONE GL_ONE
        tcMod scroll 0 1
    }
}

textures/factory/walllinerb
{
    {
        map textures/factory/etest3
        tcGen environment
    }
    {
        map textures/factory/walllinerb
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/factory/basewall_vent
{
    {
        map textures/factory/etest3
        tcGen environment
    }
    {
        map textures/factory/basewall_vent
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/factory/piller
{
    {
        map textures/factory/etest3
        tcGen environment
    }
    {
        map textures/factory/piller
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/factory/pipe_current
{
	qer_editorimage	textures/editorimages/pipe
    {
        map $lightmap
    }
    {
        map textures/factory/pipe_electric
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/factory/electrical2
        blendFunc GL_ONE GL_ONE
        rgbGen wave noise 0 1 0 85
        tcMod turb 0 0.1 0 0.1
        tcMod scroll 2 0
    }
    {
        map textures/factory/electrical2
        blendFunc GL_ONE GL_ONE
        rgbGen wave noise 0 1 0 85
        tcMod turb 0 0.1 0 0.1
        tcMod scroll -2 0
    }
    {
        map textures/factory/deconfield2
        blendFunc GL_ONE GL_ONE_MINUS_SRC_COLOR
        rgbGen wave noise 0 1 0 99
    }
}

textures/factory/env_offset
{
	qer_editorimage	textures/editorimages/env4
	polygonOffset
	q3map_nolightmap
    {
        map textures/factory/env4
        blendFunc GL_ONE GL_ONE
        tcGen environment
    }
}

textures/factory/other_field
{
	qer_editorimage	textures/factory/deconfield2
	surfaceparm	forcefield
	q3map_nolightmap
    {
        map textures/factory/deconfield2
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.7 1 0 0.1
        tcMod stretch noise 0.2 0.01 0 1
    }
    {
        map textures/factory/deconfield2
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 0.8 0.1 0 0.3
        tcMod stretch noise 0.2 0.05 0 0.1
    }
}

textures/factory/env_pipe
{
	qer_editorimage	textures/editorimages/basemetal2
    {
        map $lightmap
    }
    {
        map textures/factory/basemetal2
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/factory/enviro
        blendFunc GL_DST_COLOR GL_ONE
        tcGen environment
    }
}

textures/factory/env_glass
{
	qer_editorimage	textures/system/etest4
	surfaceparm	nonopaque
	surfaceparm	forcefield
	q3map_nolightmap
    {
        map textures/system/etest4
        blendFunc GL_ONE GL_ONE
        tcGen environment
    }
}

textures/factory/trim_01
{
    {
        map $lightmap
    }
    {
        map textures/factory/trim_01
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/factory/env4
        blendFunc GL_ONE GL_ONE
        rgbGen const ( 0.200000 0.200000 0.200000 )
        tcGen environment
    }
}

textures/factory/cat_floor_b
{
	qer_editorimage	textures/factory/cat_floor2
	q3map_material	HollowMetal
	q3map_nolightmap
	cull	twosided
    {
        map textures/factory/cat_floor
        blendFunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
        depthWrite
        rgbGen lightingDiffuse
    }
}

textures/factory/basic2_tiled_b
{
	qer_editorimage	textures/factory/basic2
	q3map_nolightmap
    {
        map textures/factory/basic2
        rgbGen lightingDiffuse
        tcMod scale 7 7
    }
}

textures/factory/fliquid4
{
	q3map_material	Water
	q3map_nolightmap
    {
        map textures/factory/fliquid4
        tcMod scale 1 0.6
        tcMod scroll 0 1
    }
    {
        map textures/factory/fliquid5
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        tcMod scale 1 0.6
        tcMod scroll 0 0.5
    }
}

textures/factory/coolant_test
{
	qer_editorimage	textures/factory/fliquid4
	surfaceparm	nonopaque
	q3map_material	Water
	q3map_nolightmap
    {
        map textures/factory/opipe_b
        tcMod scroll 0 2
        tcMod scale 3 3
    }
    {
        map textures/factory/opipe_p
        blendFunc GL_DST_COLOR GL_SRC_ALPHA
        tcMod scroll 0 1
    }
}

textures/factory/coolant_test_red
{
	qer_editorimage	textures/factory/fliquid4
	surfaceparm	nonopaque
	q3map_nolightmap
    {
        map textures/factory/opipe_o
        tcMod scroll 0 2
        tcMod scale 2 2
    }
    {
        map textures/factory/opipe_r
        blendFunc GL_DST_COLOR GL_SRC_ALPHA
        tcMod scroll 0 1
    }
}

textures/factory/colorb1
{
	surfaceparm	nomarks
	q3map_nolightmap
	cull	twosided
    {
        map textures/factory/colorb1
        rgbGen vertex
        tcMod scale 1 0.5
        tcMod scroll 0 1
    }
    {
        map textures/factory/colorb
        blendFunc GL_ONE_MINUS_SRC_ALPHA GL_SRC_ALPHA
        tcMod scale 1 0.5
        tcMod scroll 0 0.5
    }
}

textures/factory/ggoo1
{
	q3map_nolightmap
    {
        map textures/factory/ggoo1
        tcMod scroll 0.3 0
    }
    {
        map textures/factory/ggoo3
        blendFunc GL_DST_COLOR GL_SRC_ALPHA
        tcMod scroll 0.1 0
    }
}

textures/factory/door_onoff
{
    {
        map $lightmap
    }
    {
        map textures/factory/door_onoff
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        oneshotanimMap 1 textures/factory/door_onoffr textures/factory/door_onoffg 
        blendFunc GL_ONE GL_ONE
        glow
        alphaGen identity
    }
}

textures/factory/cul_door_onoff
{
    {
        map $lightmap
    }
    {
        map textures/factory/cul_door_onoff
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        oneshotanimMap 1 textures/factory/cul_door_onoffr textures/factory/cul_door_onoffg 
        blendFunc GL_ONE GL_ONE
        alphaGen identity
    }
}

textures/factory/newcrate1
{
    {
        map $lightmap
    }
    {
        map textures/factory/newcrate1
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/factory/newcrate1_glow
        blendFunc GL_ONE GL_ONE
        rgbGen wave square 0 1 0 1
    }
}

textures/factory/piller_base
{
    {
        map $lightmap
    }
    {
        map textures/factory/piller_base
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/factory/piller_base_glow
        blendFunc GL_ONE GL_ONE
        glow
    }
}

textures/factory/newcrate3_trim
{
    {
        map $lightmap
    }
    {
        map textures/factory/newcrate3_trim
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/factory/newcrate3_trim_glow1
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0 1 0 0.45
    }
    {
        map textures/factory/newcrate3_trim_glow2
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0 1 0 0.4
    }
}

textures/factory/newcrate4_side
{
    {
        map $lightmap
    }
    {
        map textures/factory/newcrate4_side
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/factory/newcrate4_side_glow1
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave inversesawtooth 0 1 0 0.75
    }
    {
        map textures/factory/newcrate4_side_glow2
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sawtooth 0 1 0 0.8
    }
}

textures/factory/bomb_new_glow
{
	qer_editorimage	textures/factory/ggoo1
	q3map_nolightmap
    {
        map models/map_objects/factory/bomb_new_glow
        glow
        rgbGen identity
        tcMod scroll 0.3 0
    }
    {
        map textures/factory/bomb_new_glw2
        blendFunc GL_ONE_MINUS_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen identity
        tcMod scroll 0.1 0
    }
}

textures/factory/gooblue
{
	qer_editorimage	textures/factory/ggoo1
	q3map_nolightmap
    {
        map textures/factory/ggoo1
        tcMod scroll 0.3 0
    }
    {
        map textures/factory/ggoo3
        blendFunc GL_ONE_MINUS_DST_COLOR GL_ZERO
        tcMod scroll 0.1 0
    }
}

textures/factory/bomb_new_glow_deact
{
	qer_editorimage	textures/factory/ggoo1
	q3map_nolightmap
    {
        map models/map_objects/factory/bomb_new_glow_off
        glow
        rgbGen identity
        tcMod scroll 0.3 0
    }
    {
        map textures/factory/bomb_new_glw2_off
        blendFunc GL_ONE GL_ONE
        rgbGen identity
        tcMod scroll 0.1 0
    }
}

textures/factory/basemetal5
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/factory/basemetal5
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/factory/basefloor1
{
	q3map_material	HollowMetal
    {
        map $lightmap
    }
    {
        map textures/factory/basefloor1
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/factory/basemetal1
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/factory/basemetal1
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/factory/basemetal2
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/factory/basemetal2
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/factory/basemetal4
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/factory/basemetal4
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/factory/base
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/factory/base
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/factory/basicmetal2_2
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/factory/basicmetal2_2
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/factory/basic2
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/factory/basic2
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/factory/basic3
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/factory/basic3
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/factory/basic4
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/factory/basic4
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/factory/basic5
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/factory/basic5
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/factory/basic6
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/factory/basic6
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/factory/basic1
{
	q3map_material	SolidWood
    {
        map $lightmap
    }
    {
        map textures/factory/basic1
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/factory/darkfloor
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/factory/darkfloor
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/factory/darkmetal
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/factory/darkmetal
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/factory/grating
{
	q3map_material	HollowMetal
    {
        map $lightmap
    }
    {
        map textures/factory/grating
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/factory/grate
{
	q3map_material	HollowMetal
    {
        map $lightmap
    }
    {
        map textures/factory/grate
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/factory/inside_floor
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/factory/inside_floor
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/factory/metalsec1
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/factory/metalsec1
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/factory/outside_rock1b
{
	q3map_material	Rock
    {
        map $lightmap
    }
    {
        map textures/factory/outside_rock1b
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/factory/outside_floor1
{
	q3map_material	Rock
    {
        map $lightmap
    }
    {
        map textures/factory/outside_floor1
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/factory/s_basic
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/factory/s_basic
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/factory/volcanic_rock
{
	q3map_material	Rock
    {
        map $lightmap
    }
    {
        map textures/factory/volcanic_rock
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/factory/basic3_b
{
	qer_editorimage	textures/factory/basic4
	q3map_material	SolidMetal
	q3map_nolightmap
    {
        map textures/factory/basic4
        rgbGen lightingDiffuse
    }
}

textures/factory/conveyor_belt_off
{
	qer_editorimage	textures/factory/conveyor_belt
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/factory/conveyor_belt
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/factory/conveyor_belt_slow
{
	qer_editorimage	textures/factory/conveyor_belt
	q3map_material	SolidMetal
    {
        map $lightmap
        tcMod entityTranslate
    }
    {
        map textures/factory/conveyor_belt
        blendFunc GL_DST_COLOR GL_ZERO
        tcMod scroll 0 0.92
    }
}

textures/factory/basemetal5_noclip
{
	qer_editorimage	textures/factory/basemetal5
	surfaceparm	nonsolid
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/factory/basemetal5
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/factory/darkmetal_noclip
{
	qer_editorimage	textures/factory/darkmetal
	surfaceparm	nonsolid
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/factory/darkmetal
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/factory/env_pipe_noclip
{
	qer_editorimage	textures/editorimages/basemetal2
	surfaceparm	nonsolid
    {
        map $lightmap
    }
    {
        map textures/factory/basemetal2
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/factory/enviro
        blendFunc GL_DST_COLOR GL_ONE
        tcGen environment
    }
}

textures/factory/goldtop2_noclip
{
	qer_editorimage	textures/factory/goldtop2
	surfaceparm	nonsolid
    {
        map $lightmap
    }
    {
        map textures/factory/goldtop2
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/factory/conveyor_belt_fast
{
	qer_editorimage	textures/factory/conveyor_belt
	q3map_material	SolidMetal
    {
        map $lightmap
        tcMod entityTranslate
    }
    {
        map textures/factory/conveyor_belt
        blendFunc GL_DST_COLOR GL_ZERO
        tcMod scroll 0 3
    }
}

textures/factory/conveyor_belt_fast_2
{
	qer_editorimage	textures/factory/conveyor_belt
	q3map_material	SolidMetal
    {
        map $lightmap
        tcMod entityTranslate
    }
    {
        map textures/factory/conveyor_belt
        blendFunc GL_DST_COLOR GL_ZERO
        tcMod scroll 0 1.87
    }
}

models/map_objects/factory/catw
{
	surfaceparm	trans
	q3map_nolightmap
    {
        map models/map_objects/factory/catw
        blendFunc GL_ONE GL_SRC_ALPHA
        depthWrite
        rgbGen vertex
    }
}

models/map_objects/factory/power18a
{
	surfaceparm	trans
	q3map_nolightmap
    {
        map models/map_objects/factory/power18a
        depthWrite
        rgbGen vertex
        tcMod rotate 10
        tcMod scroll 0 10
    }
    {
        map models/map_objects/factory/llinep
        blendFunc GL_ONE_MINUS_DST_COLOR GL_ONE
        tcMod rotate 300
        tcMod scroll 3 0
    }
    {
        map models/map_objects/factory/llinep
        blendFunc GL_ONE_MINUS_DST_COLOR GL_ONE
        tcMod rotate 100
        tcMod scroll 3 0
    }
    {
        map models/map_objects/factory/rmask
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        tcMod stretch sin 1 0.05 0 7
    }
}

models/map_objects/factory/ceiling_light_flare
{
	q3map_flare	textures/flares/flare_lt_blue
	q3map_nolightmap
    {
        map models/map_objects/factory/ceiling_light_flare
        blendFunc GL_ONE GL_ZERO
        rgbGen vertex
    }
}

models/map_objects/factory/light_post_sm
{
	q3map_nolightmap
    {
        map models/map_objects/factory/light_post_sm
        blendFunc GL_ONE GL_SRC_ALPHA
        depthWrite
        rgbGen vertex
    }
}

models/map_objects/factory/f_door
{
	q3map_nolightmap
    {
        map models/map_objects/factory/f_door
        blendFunc GL_ONE GL_ZERO
        rgbGen vertex
    }
    {
        map models/map_objects/factory/f_door_glow
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

models/map_objects/factory/brace2
{
	q3map_nolightmap
    {
        map models/map_objects/factory/brace2
        blendFunc GL_ONE GL_ZERO
        rgbGen vertex
    }
    {
        map models/map_objects/factory/brace2_glow
        blendFunc GL_ONE GL_ONE
        glow
    }
}

models/map_objects/factory/power_glow
{
	q3map_nolightmap
    {
        map $whiteimage
        rgbGen const ( 0.001000 0.000000 0.000000 )
    }
    {
        map models/map_objects/factory/power_glow
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 1 1 0 0.2
        tcMod scroll 0 -0.1
    }
    {
        map models/map_objects/factory/power_glow
        blendFunc GL_ONE GL_ONE
        detail
        tcMod scroll -0.04 -0.08
    }
    {
        map models/map_objects/factory/power_glow
        blendFunc GL_ONE GL_ONE
        detail
        rgbGen wave sin 0.7 1 0 0.07
        tcMod scroll -0.004 -0.04
    }
    {
        map models/map_objects/factory/power_glow
        blendFunc GL_ONE GL_ONE
        detail
        rgbGen const ( 0.007843 0.800000 0.584314 )
        tcMod scale 0.7 0.7
        tcMod scroll 0 -0.03
    }
}

models/map_objects/factory/s_light_bulbs
{
	q3map_nolightmap
    {
        map models/map_objects/factory/s_light_bulbs
        rgbGen vertex
    }
    {
        map models/map_objects/factory/s_light_bulbs_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen vertex
    }
}

models/map_objects/factory/s_light
{
	q3map_flare	textures/flares/flare_lt_blue
	q3map_nolightmap
    {
        map models/map_objects/factory/s_light
        blendFunc GL_ONE GL_ZERO
        rgbGen vertex
    }
}

models/map_objects/factory/s_crate_03_liquid
{
	surfaceparm	trans
	q3map_nolightmap
    {
        map models/map_objects/factory/s_crate_03_liquid
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen lightingDiffuse
        alphaGen const 0.5
    }
}

models/map_objects/factory/s_crate_03_glass
{
	surfaceparm	trans
	q3map_nolightmap
	cull	twosided
    {
        map models/map_objects/factory/s_crate_03_glass
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen lightingDiffuse
    }
}

models/map_objects/factory/s_tank_large
{
	q3map_nolightmap
    {
        map models/map_objects/factory/s_tank_large
        rgbGen vertex
    }
    {
        map models/map_objects/factory/s_tank_large_glow
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

models/map_objects/factory/hpart
{
	q3map_nolightmap
    {
        map models/map_objects/factory/hpart
        blendFunc GL_ONE GL_ZERO
        rgbGen vertex
    }
}

models/map_objects/factory/window_frame
{
	q3map_nolightmap
    {
        map models/map_objects/factory/window_frame
        rgbGen vertex
    }
}

models/map_objects/factory/glass
{
	q3map_nolightmap
	cull	twosided
    {
        map models/map_objects/factory/glass
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/factory/glass_reflect
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        detail
        alphaGen const 0.05
        tcGen environment
        tcMod scale 1.5 1.5
    }
}

models/map_objects/factory/brace_new_light
{
	q3map_nolightmap
    {
        map models/map_objects/factory/brace_new_light
        rgbGen vertex
    }
    {
        map models/map_objects/factory/brace_new_light_glow
        blendFunc GL_ONE GL_ONE
        glow
    }
}

models/map_objects/factory/brace_new
{
	q3map_nolightmap
    {
        map models/map_objects/factory/brace_new
        rgbGen vertex
    }
}

models/map_objects/factory/ceiling_light
{
	q3map_nolightmap
    {
        map models/map_objects/factory/ceiling_light
        rgbGen vertex
    }
    {
        map models/map_objects/factory/ceiling_light_glow
        blendFunc GL_ONE GL_ONE
        glow
    }
}

models/map_objects/factory/terrain
{
// q3map_lightsubdivide	32

	q3map_shadeangle	160
	qer_editorimage	models/map_objects/factory/fterbake
	q3map_nolightmap
	q3map_nonplanar
	q3map_forcemeta
	q3map_onlyvertexlighting
    {
        map models/map_objects/factory/terrain
        blendFunc GL_ONE GL_ZERO
        rgbGen vertex
        tcMod scale 4 4
    }
    {
        map models/map_objects/factory/terrain
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen vertex
    }
}

models/map_objects/factory/ceiling_light_new
{
	q3map_nolightmap
    {
        map models/map_objects/factory/ceiling_light_new
        rgbGen vertex
    }
    {
        map models/map_objects/factory/ceiling_light_new_glow
        blendFunc GL_ONE GL_ONE
        glow
    }
}

models/map_objects/factory/pilar2
{
	q3map_nolightmap
    {
        map textures/factory/basemetal2
        blendFunc GL_ONE GL_ZERO
        rgbGen vertex
        tcMod scale 2 2
    }
    {
        map models/map_objects/factory/pilar2
        blendFunc GL_ONE GL_ONE_MINUS_SRC_COLOR
        rgbGen vertex
    }
    {
        map textures/factory/basemetal2
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        detail
        tcMod scale 4 4
    }
}

models/map_objects/factory/pilar
{
	q3map_nolightmap
    {
        map textures/factory/basic5
        blendFunc GL_ONE GL_ZERO
        rgbGen vertex
        tcMod scale 4 4
    }
    {
        map models/map_objects/factory/pilar
        blendFunc GL_ONE GL_ONE_MINUS_SRC_COLOR
        rgbGen vertex
    }
    {
        map textures/factory/basemetal2
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        tcMod scale 10 10
    }
}

models/map_objects/factory/s_crane_pipes
{
	q3map_nolightmap
    {
        map textures/factory/crane_pipe
        rgbGen lightingDiffuse
        tcMod scale 10 10
    }
    {
        map models/map_objects/factory/s_crane_pipes
        blendFunc GL_DST_COLOR GL_ZERO
        detail
        rgbGen lightingDiffuse
    }
}

models/map_objects/factory/s_crane_cables
{
	q3map_nolightmap
    {
        map textures/factory/crane_coil
        rgbGen lightingDiffuse
        tcMod scale 3 3
    }
    {
        map models/map_objects/factory/s_crane_cables
        blendFunc GL_DST_COLOR GL_ZERO
        detail
        rgbGen lightingDiffuse
    }
}

models/map_objects/factory/s_crane_parts
{
	q3map_nolightmap
    {
        map textures/factory/basic1
        rgbGen lightingDiffuse
        tcMod scale 10 10
    }
    {
        map models/map_objects/factory/s_crane_parts
        blendFunc GL_DST_COLOR GL_ZERO
        detail
        rgbGen lightingDiffuse
    }
}

models/map_objects/factory/s_crane_body
{
	q3map_nolightmap
    {
        map textures/factory/inside_floor
        rgbGen lightingDiffuse
        tcMod scale 5 10
    }
    {
        map models/map_objects/factory/s_crane_body
        blendFunc GL_DST_COLOR GL_ZERO
        detail
        rgbGen lightingDiffuse
    }
}

models/map_objects/factory/s_crane_motor
{
	q3map_nolightmap
    {
        map textures/factory/darkfloor
        rgbGen lightingDiffuse
        tcMod scale 10 10
    }
    {
        map models/map_objects/factory/s_crane_motor
        blendFunc GL_DST_COLOR GL_ZERO
        detail
        rgbGen lightingDiffuse
    }
}

models/map_objects/factory/catbig
{
	qer_editorimage	models/map_objects/factory/catbig
	surfaceparm	trans
	q3map_nolightmap
    {
        map models/map_objects/factory/catbig
        blendFunc GL_ONE GL_SRC_ALPHA
        depthWrite
        rgbGen vertex
    }
}

models/map_objects/factory/f_door_b
{
	qer_editorimage	models/map_objects/factory/f_door
	q3map_nolightmap
    {
        map models/map_objects/factory/f_door
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/factory/f_door_glow
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

models/map_objects/factory/hpart_b
{
	qer_editorimage	models/map_objects/factory/hpart
	q3map_nolightmap
    {
        map models/map_objects/factory/hpart
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
}

models/map_objects/factory/brace_new_light_b
{
	qer_editorimage	models/map_objects/factory/brace_new_light
	q3map_nolightmap
    {
        map models/map_objects/factory/brace_new_light
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/factory/brace_new_light_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

models/map_objects/factory/brace_new_b
{
	qer_editorimage	models/map_objects/factory/brace_new
	q3map_nolightmap
    {
        map models/map_objects/factory/brace_new
        rgbGen lightingDiffuse
    }
}

models/map_objects/factory/ceiling_light_new_b
{
	qer_editorimage	models/map_objects/factory/ceiling_light_new
	q3map_nolightmap
    {
        map models/map_objects/factory/ceiling_light_new
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/factory/ceiling_light_new_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

models/map_objects/factory/pilar2_b
{
	qer_editorimage	textures/factory/basemetal2
	q3map_nolightmap
    {
        map textures/factory/basemetal2
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
        tcMod scale 2 2
    }
    {
        map models/map_objects/factory/pilar2
        blendFunc GL_ONE GL_ONE_MINUS_SRC_COLOR
        rgbGen identity
    }
    {
        map textures/factory/basemetal2
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        detail
        tcMod scale 4 4
    }
}

models/map_objects/factory/pilar_b
{
	qer_editorimage	textures/factory/basic5
	q3map_nolightmap
    {
        map textures/factory/basic5
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
        tcMod scale 4 4
    }
    {
        map models/map_objects/factory/pilar
        blendFunc GL_ONE GL_ONE_MINUS_SRC_COLOR
        rgbGen identity
    }
    {
        map textures/factory/basemetal2
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        tcMod scale 10 10
    }
}

models/map_objects/factory/s_light_b
{
	qer_editorimage	models/map_objects/factory/s_light
	q3map_flare	textures/flares/flare_lt_blue
	q3map_nolightmap
    {
        map models/map_objects/factory/s_light
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
}

models/map_objects/factory/s_light_bulbs_b
{
	qer_editorimage	models/map_objects/factory/s_light_bulbs
	q3map_nolightmap
    {
        map models/map_objects/factory/s_light_bulbs
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/factory/s_light_bulbs_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

models/map_objects/factory/f_con1_screen
{
    {
        map $lightmap
    }
    {
        map models/map_objects/factory/f_con1_screen
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map models/map_objects/factory/f_con1_screen_glow
        blendFunc GL_ONE GL_ONE
        glow
    }
    {
        map models/map_objects/factory/f_con1_screen_glowr
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 0.5 0.35 0 0.6
    }
}

models/map_objects/factory/f_con2_screenunlocked
{
    {
        map $lightmap
    }
    {
        map models/map_objects/factory/f_con2_screenunlocked
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map models/map_objects/factory/f_con2_screen_glw
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
    {
        map models/map_objects/factory/f_con2_screeng
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

models/map_objects/factory/f_con2_screenlocked
{
    {
        map $lightmap
    }
    {
        map models/map_objects/factory/f_con2_screen
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map models/map_objects/factory/f_con2_screen_glw
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
    {
        map models/map_objects/factory/f_con2_screenr
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.5 0.35 0 0.6
    }
}

models/map_objects/factory/fgen_beam
{
    {
        map gfx/effects/stunpass
        tcMod scroll 0.5 0.8
        tcMod scale 1 0.2
    }
    {
        map models/map_objects/factory/fgen_beam
        blendFunc GL_ONE GL_SRC_ALPHA
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/factory/fgen_beam_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/map_objects/factory/fgen_center
{
    {
        map models/map_objects/factory/fgen_center
        rgbGen lightingDiffuse
    }
    {
        map gfx/effects/stunpass
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0 0.3 0 3
        tcMod scale 1 3
        tcMod scroll 0.3 1
    }
    {
        map gfx/effects/blueline
        blendFunc GL_ONE GL_ONE
        rgbGen wave noise 0 0.5 0 6
    }
    {
        map models/map_objects/factory/fgen_center
        blendFunc GL_ONE GL_SRC_ALPHA
        rgbGen identity
    }
    {
        map models/map_objects/factory/fgen_center_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/map_objects/factory/fgen_bottom
{
    {
        map models/map_objects/factory/fgen_bottom
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/factory/fgen_bottom
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/map_objects/factory/fgen_top
{
    {
        map models/map_objects/factory/fgen_top
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/factory/fgen_top
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/map_objects/factory/glass_b
{
	qer_editorimage	models/map_objects/factory/glass
	q3map_nolightmap
	cull	twosided
    {
        map models/map_objects/factory/glass
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/factory/glass_reflect
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        detail
        alphaGen const 0.05
        tcGen environment
        tcMod scale 1.5 1.5
    }
}

models/map_objects/factory/bomb_new
{
	q3map_nolightmap
    {
        map models/map_objects/factory/bomb_new
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/factory/bomb_new_lights
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 0 1 0 1
    }
}

models/map_objects/factory/bomb_new_glow
{
	q3map_nolightmap
    {
        map models/map_objects/factory/bomb_new_glow
        tcMod scroll 0.3 0
    }
    {
        map textures/factory/ggoo3
        blendFunc GL_ONE_MINUS_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        tcMod scroll 0.1 0
    }
    {
        map models/map_objects/factory/bomb_new_frame
        blendFunc GL_ONE_MINUS_SRC_ALPHA GL_SRC_ALPHA
        rgbGen lightingDiffuse
    }
}

models/map_objects/factory/view_panel_screen_fact
{
	q3map_nolightmap
    {
        map models/map_objects/factory/view_panel_screen_fact
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/factory/view_panel_screen_fact_glow1
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0 1 0 0.5
    }
    {
        map models/map_objects/factory/view_panel_screen_fact_glow2
        blendFunc GL_ONE GL_ONE
        rgbGen wave inversesawtooth 0 1 0 1
    }
    {
        map models/map_objects/factory/view_panel_screen_fact_glow3
        blendFunc GL_ONE GL_ONE
        tcMod scroll 0.5 0
    }
}

models/map_objects/factory/emod_broken
{
	qer_editorimage	models/map_objects/factory/bomb_new
	q3map_nolightmap
    {
        map models/map_objects/factory/emod_broken
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/factory/emod_broken_glow
        blendFunc GL_ONE GL_ONE
        glow
    }
    {
        map models/map_objects/factory/emod_broken_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 0 0.5 0 0.3
    }
}

models/map_objects/factory/emod_broken2
{
	qer_editorimage	models/map_objects/factory/bomb_new
	q3map_nolightmap
    {
        map models/map_objects/factory/emod_broken2
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/factory/emod_broken2_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 0 0.4 0 0.3
    }
    {
        map models/map_objects/factory/emod_broken2_glow
        blendFunc GL_ONE GL_ONE
    }
}

models/map_objects/cairn/container
{
	q3map_nolightmap
    {
        map models/map_objects/cairn/container
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/cairn/container_glow1
        blendFunc GL_ONE GL_ONE
        glow
    }
}

textures/wedge/large_tank_grate
{
	q3map_nolightmap
    {
        map textures/wedge/tankeffect
        rgbGen identity
        tcMod scroll 0 8
    }
    {
        map textures/wedge/large_tank_grate
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen lightingDiffuse
    }
}

textures/wedge/large_tank_fins
{
	q3map_nolightmap
    {
        map textures/wedge/large_tank_fins
        rgbGen lightingDiffuse
    }
    {
        map textures/wedge/large_tank_fins_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/wedge/large_tank_cap
{
	q3map_nolightmap
    {
        map textures/wedge/large_tank_cap
        rgbGen lightingDiffuse
    }
    {
        map textures/wedge/large_tank_cap_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

