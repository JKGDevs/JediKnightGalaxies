models/map_objects/taspir/lamp_bulb
{
	q3map_nolightmap
    {
        map models/map_objects/taspir/lamp_bulb
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/taspir/lamp_bulb_glw
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

models/map_objects/taspir/control_station
{
	q3map_nolightmap
    {
        map models/map_objects/taspir/control_station
        rgbGen lightingDiffuse
    }
}

models/map_objects/taspir/control_station_screen
{
    {
        map textures/colors/black
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/taspir/control_station_screen2_glow
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 1 -0.5 1 0.24
        tcMod stretch sin 2.5 0.5 0 0.32
        tcMod rotate 20
    }
    {
        map models/map_objects/taspir/control_station_screen2_glow2
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 1 0.5 1 0.24
        tcMod rotate -10
    }
    {
        map models/map_objects/taspir/control_station_screen
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen const ( 0.000000 0.000000 0.000000 )
    }
}

models/map_objects/taspir/siding2_2_2
{
	q3map_material	HollowMetal
	q3map_nolightmap
    {
        map textures/taspir/siding2
        rgbGen vertex
        tcMod scale 2 2
    }
}

models/map_objects/taspir/newdoor1_b_1_1
{
	q3map_nolightmap
    {
        map textures/taspir/newdoor1_b
        rgbGen vertex
    }
}

models/map_objects/taspir/floor_joist_cap
{
	q3map_nolightmap
    {
        map models/map_objects/taspir/floor_joist_cap
        rgbGen lightingDiffuse
    }
}

models/map_objects/taspir/lava_tube_claw
{
	q3map_nolightmap
    {
        map models/map_objects/taspir/lava_tube_claw
        rgbGen lightingDiffuse
    }
}

models/map_objects/taspir/lava_tube_3_1
{
	q3map_nolightmap
    {
        map models/map_objects/taspir/lava_tube
        rgbGen lightingDiffuse
        tcMod scale 3 1
    }
}

models/map_objects/taspir/lava_tube_tile2_4_25
{
	q3map_nolightmap
    {
        map models/map_objects/taspir/lava_tube_tile02
        rgbGen lightingDiffuse
        tcMod scale 8 1
    }
}

models/map_objects/taspir/lava_tube_tile1_1_1
{
	q3map_nolightmap
    {
        map models/map_objects/taspir/lava_tube_tile01
        rgbGen lightingDiffuse
    }
}

models/map_objects/taspir/lava_tube_tile1_4_25
{
	q3map_nolightmap
    {
        map models/map_objects/taspir/lava_tube_tile01
        rgbGen lightingDiffuse
        tcMod scale 6 1
    }
}

models/map_objects/taspir/pipe_small_5_5
{
	q3map_nolightmap
    {
        map models/map_objects/taspir/pipe_small_5_5
        rgbGen lightingDiffuse
        tcMod scale 5 5
    }
}

models/map_objects/taspir/steam_jet_parts
{
	q3map_nolightmap
    {
        map models/map_objects/taspir/steam_jet_parts
        rgbGen lightingDiffuse
    }
    {
        animMap 10 models/map_objects/taspir/jet_frame1 models/map_objects/taspir/jet_frame1 models/map_objects/taspir/jet_frame2 models/map_objects/taspir/jet_frame2 models/map_objects/taspir/jet_frame3 models/map_objects/taspir/jet_frame3 models/map_objects/taspir/jet_frame4 models/map_objects/taspir/jet_frame4 models/map_objects/taspir/jet_frame5 models/map_objects/taspir/jet_frame5 
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

models/map_objects/taspir/siding2_1_1
{
	q3map_material	HollowMetal
	q3map_nolightmap
    {
        map textures/taspir/siding2
        rgbGen lightingDiffuse
    }
}

models/map_objects/taspir/wallnew2_1_1
{
	q3map_material	HollowMetal
	q3map_nolightmap
    {
        map textures/taspir/wallnew2
        rgbGen lightingDiffuse
    }
}

models/map_objects/taspir/seam_30_1
{
	q3map_nolightmap
	cull	twosided
    {
        map models/map_objects/taspir/seam_30_1
        rgbGen lightingDiffuse
        tcMod scale 60 1
    }
}

models/map_objects/taspir/tank_top_top_7_7
{
	q3map_material	HollowMetal
	q3map_nolightmap
	cull	twosided
    {
        map models/map_objects/taspir/tank_top_top
        rgbGen lightingDiffuse
    }
}

models/map_objects/taspir/tank_top_5_154
{
	q3map_nolightmap
    {
        map models/map_objects/taspir/tank_top
        rgbGen lightingDiffuse
        tcMod scale 7 1
    }
}

models/map_objects/taspir/tank_top_top_1_1
{
	q3map_material	HollowMetal
	q3map_nolightmap
	cull	twosided
    {
        clampmap models/map_objects/taspir/tank_top_top
        rgbGen lightingDiffuse
    }
}

models/map_objects/taspir/basefloor_1_1
{
	q3map_material	HollowMetal
	q3map_nolightmap
    {
        map textures/taspir/basefloor
        rgbGen lightingDiffuse
    }
}

models/map_objects/taspir/control_station_screen2
{
	qer_editorimage	textures/colors/black
	q3map_nolightmap
    {
        map models/map_objects/taspir/control_station_screen2_glow
        blendFunc GL_ONE GL_SRC_COLOR
        rgbGen wave sin 1 -0.5 1 0.24
        tcMod stretch sin 2.5 0.5 0 0.32
        tcMod rotate 20
    }
    {
        map models/map_objects/taspir/control_station_screen2_glow2
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 1 0.5 1 0.24
        tcMod rotate -10
    }
    {
        map models/map_objects/taspir/control_station_screen2
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen lightingDiffuse
    }
}

models/map_objects/taspir/lamp2_bulb
{
	qer_editorimage	textures/taspir/lamp2_bulb
	q3map_nolightmap
    {
        map models/map_objects/taspir/lamp2_bulb
        blendFunc GL_ONE GL_ZERO
    }
}

models/map_objects/taspir/lava_ring
{
	q3map_nolightmap
    {
        map models/map_objects/taspir/lava_ring
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/taspir/lava_ring_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

models/map_objects/taspir/strut
{
	q3map_nolightmap
    {
        map models/map_objects/taspir/strut
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/taspir/strut_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
    {
        map models/map_objects/taspir/strut_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave noise 0 1 0 5
    }
}

models/map_objects/taspir/steam_jets
{
	q3map_nolightmap
    {
        map models/map_objects/taspir/steam_jets
        rgbGen lightingDiffuse
    }
    {
        animMap 4 models/map_objects/taspir/jetside_frame1 models/map_objects/taspir/jetside_frame1 models/map_objects/taspir/jetside_frame2 models/map_objects/taspir/jetside_frame2 models/map_objects/taspir/jetside_frame3 models/map_objects/taspir/jetside_frame3 models/map_objects/taspir/jetside_frame4 models/map_objects/taspir/jetside_frame4 
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
    {
        map models/map_objects/taspir/steam_jet_lightmap
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

models/map_objects/taspir/airpure
{
    {
        map models/map_objects/taspir/airpure
        rgbGen lightingDiffuse
    }
    {
        animMap 4 models/map_objects/taspir/airpure_frame4 models/map_objects/taspir/airpure_frame4 models/map_objects/taspir/airpure_frame3 models/map_objects/taspir/airpure_frame3 models/map_objects/taspir/airpure_frame2 models/map_objects/taspir/airpure_frame2 models/map_objects/taspir/airpure_frame1 models/map_objects/taspir/airpure_frame1 
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

models/map_objects/taspir/tank_top_top
{
	qer_editorimage	models/map_objects/taspir/tank_top_top
	q3map_material	HollowMetal
	q3map_nolightmap
	cull	twosided
    {
        clampmap models/map_objects/taspir/tank_top_top
        rgbGen lightingDiffuse
    }
}

textures/taspir/ore_lightemitting
{
	q3map_lightimage	textures/taspir/lava00
	qer_editorimage	textures/taspir/lava00
	q3map_surfacelight	1600
	q3map_lightsubdivide	64
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	lava
	surfaceparm	trans
	q3map_nolightmap
	q3map_onlyvertexlighting
	q3map_novertexshadows
	cull	twosided
    {
        map textures/imp_mine/lava2
        glow
        rgbGen wave sin 1.2 0.4 0 0.3
        tcMod scroll 0.04 0.04
    }
}

textures/taspir/ore_lightemitting_lavapool
{
	q3map_lightimage	textures/taspir/lava00
	qer_editorimage	textures/taspir/lava00
	q3map_surfacelight	1200
	q3map_lightsubdivide	512
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	lava
	surfaceparm	trans
	q3map_nolightmap
	q3map_onlyvertexlighting
	q3map_novertexshadows
	cull	twosided
    {
        map textures/imp_mine/lava2
        glow
        rgbGen wave sin 1.2 0.4 0 0.3
        tcMod scroll 0.04 0.04
    }
}

textures/taspir/ore_lightemitting_lavapool2
{
	q3map_lightimage	textures/taspir/lava00
	qer_editorimage	textures/taspir/lava00
	q3map_surfacelight	1200
	q3map_lightsubdivide	512
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	lava
	surfaceparm	trans
	q3map_nolightmap
	q3map_onlyvertexlighting
	q3map_novertexshadows
	cull	twosided
    {
        map textures/imp_mine/lava2
        glow
        rgbGen wave sin 1.2 0.4 0 0.3
        tcMod scroll 0.04 0.04
    }
}

textures/taspir/taspir_phongrock
{
	q3map_shadeangle	150
	qer_editorimage	textures/taspir/taspir_rock2.tga
	q3map_lightmapsamplesize	64
	q3map_nonplanar
    {
        map textures/taspir/taspir_rock2
        rgbGen identity
        tcMod scale 0.5 0.5
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/taspir/grating_building2
{
	qer_editorimage	textures/imp_mine/grating
	q3map_material	HollowMetal
	q3map_alphashadow
	cull	twosided
    {
        map textures/imp_mine/grating
        alphaFunc GE128
        depthWrite
        rgbGen identity
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        depthFunc equal
    }
}

textures/taspir/taspir1_mapbase
{
	nopicmip
	q3map_nolightmap
    {
        map textures/taspir/taspir1_mapbase
    }
    {
        map textures/taspir/taspir1_mapglow2
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0 1 0 0.5
    }
}

textures/taspir/taspir1_mapbase2
{
	qer_editorimage	textures/taspir/taspir1_mapbase
	nopicmip
	q3map_nolightmap
    {
        map textures/taspir/taspir1_mapbase
    }
    {
        map textures/taspir/taspir1_mapglow3
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0 1 0 0.5
    }
}

textures/taspir/taspir1_mapbase3
{
	qer_editorimage	textures/taspir/taspir1_mapbase
	nopicmip
	q3map_nolightmap
    {
        map textures/taspir/taspir1_mapbase
    }
    {
        map textures/taspir/taspir1_mapglow4
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0 1 0 0.5
    }
}

textures/taspir/taspir1_mapbase4
{
	qer_editorimage	textures/taspir/taspir1_mapbase
	nopicmip
	q3map_nolightmap
    {
        map textures/taspir/taspir1_mapbase
    }
    {
        map textures/taspir/taspir1_mapglow5
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0 1 0 0.5
    }
}

textures/taspir/control1
{
    {
        map $lightmap
    }
    {
        map textures/taspir/control1
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/taspir/control1_glw
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

textures/taspir/control2
{
	q3map_nolightmap
    {
        map textures/taspir/control2
        rgbGen vertex
    }
    {
        map textures/taspir/control2_glw
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
    {
        map textures/taspir/control2_glw2
        blendFunc GL_ONE GL_ONE
        rgbGen wave square 0 1 0 1
    }
    {
        map textures/taspir/control2_glw3
        blendFunc GL_ONE GL_ONE
        rgbGen wave triangle 0 1 0 1.5
    }
}

textures/taspir/control3
{
    {
        map $lightmap
    }
    {
        map textures/taspir/control3
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/taspir/control3_pls
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.5 0.35 0 0.6
    }
    {
        map textures/taspir/control3_glw
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

textures/taspir/control4
{
    {
        map $lightmap
    }
    {
        map textures/taspir/control4
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/taspir/control4_glw
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

textures/taspir/control6
{
    {
        map $lightmap
    }
    {
        map textures/taspir/control6
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/taspir/control6_glw
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

textures/taspir/landing_pad
{
	q3map_nolightmap
    {
        map textures/taspir/landing_pad
        rgbGen vertex
    }
    {
        map textures/taspir/landingplat_glw
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
    {
        map textures/taspir/landing_wipeh
        blendFunc GL_ONE GL_ONE
        rgbGen identity
        tcMod scroll 0.25 0
    }
    {
        map textures/taspir/landing_wipev
        blendFunc GL_ONE GL_ONE
        rgbGen identity
        tcMod scroll 0 -0.25
    }
    {
        map textures/taspir/landing_pad
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

textures/taspir/taspir1_mapbase5
{
	qer_editorimage	textures/taspir/taspir1_mapbase
	nopicmip
	q3map_nolightmap
    {
        map textures/taspir/taspir1_mapbase
    }
    {
        map textures/taspir/taspir1_mapglow6
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0 1 0 0.5
    }
}

textures/taspir/taspir1_mapbase_all
{
	qer_editorimage	textures/taspir/taspir1_mapbase
	nopicmip
	q3map_nolightmap
    {
        map textures/taspir/taspir1_mapbase
    }
    {
        map textures/taspir/taspir1_mapglow_all
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 0 1 0 0.5
    }
}

textures/taspir/rust_glow_y
{
	qer_editorimage	textures/taspir/512_512rust
    {
        map $lightmap
    }
    {
        map textures/taspir/512_512rust
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/taspir/512_512rust_glow_y
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/taspir/rust_glow_r
{
	qer_editorimage	textures/taspir/512_512rust
    {
        map $lightmap
    }
    {
        map textures/taspir/512_512rust
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/taspir/512_512rust_glow_r
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/taspir/rust_glow_g
{
	qer_editorimage	textures/taspir/512_512rust
    {
        map $lightmap
    }
    {
        map textures/taspir/512_512rust
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/taspir/512_512rust_glow_g
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/taspir/rust_glow_b
{
	qer_editorimage	textures/taspir/512_512rust
    {
        map $lightmap
    }
    {
        map textures/taspir/512_512rust
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/taspir/512_512rust_glow_b
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/taspir/light_ceiling
{
	qer_editorimage	textures/taspir/light
    {
        map $lightmap
    }
    {
        map textures/taspir/light
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/taspir/light_glow
        blendFunc GL_ONE GL_ONE
        glow
    }
}

textures/taspir/t_wall5
{
    {
        map $lightmap
    }
    {
        map textures/taspir/t_wall5
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/taspir/t_wall5_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/taspir/lamp2_bulb
{
	qer_editorimage	textures/taspir/lamp2_bulb
    {
        map $lightmap
    }
    {
        map textures/taspir/lamp2_bulb
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/taspir/lamp2_bulb_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/taspir/display_controls
{
    {
        map $lightmap
    }
    {
        map textures/taspir/display_controls
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/taspir/display_controls_glw
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
    {
        map textures/taspir/display_controls_wp
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0 1 0.7 0.75
        tcMod scroll 0 -0.75
    }
    {
        map textures/taspir/display_controls_up
        blendFunc GL_ONE GL_ONE
        rgbGen wave triangle 0 1 0 1
    }
}

textures/taspir/display_controls2
{
    {
        map $lightmap
    }
    {
        map textures/taspir/display_controls2
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/taspir/display_controls2_glw
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
    {
        map textures/taspir/display_controls2_glwa
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.75 1 0 0.75
    }
    {
        map textures/taspir/display_controls2_glwb
        blendFunc GL_ONE GL_ONE
        rgbGen wave square 0 1 0 0.75
    }
    {
        map textures/taspir/display_controls2_glwc
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.75 1 0 0.85
    }
}

textures/taspir/display_controls3
{
    {
        map $lightmap
    }
    {
        map textures/taspir/display_controls3
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/taspir/display_controls3_glw
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
    {
        map textures/taspir/display_controls3_glw2
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.5 1 0 0.5
    }
}

textures/taspir/lift_switch
{
    {
        map $lightmap
    }
    {
        map textures/taspir/lift_switch
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/taspir/lift_switch_glw
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

textures/taspir/lift_switch_off
{
    {
        map $lightmap
    }
    {
        map textures/taspir/lift_switch_off
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/taspir/lift_switch_glw
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
    {
        map textures/taspir/lift_switch_off_wp
        blendFunc GL_ONE GL_ONE
        rgbGen identity
        tcMod scroll 0 0.5
    }
    {
        map textures/taspir/lift_switch_off
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map textures/taspir/lift_switch_off_blb
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.5 0.35 0 0.6
    }
}

textures/taspir/display_controls4
{
	q3map_nolightmap
    {
        map textures/taspir/display_controls4
    }
    {
        map textures/taspir/display_controls4_glow1
        blendFunc GL_ONE GL_ONE
        rgbGen wave inversesawtooth 0 1 0 0.5
    }
    {
        map textures/taspir/display_controls4_glow2
        blendFunc GL_ONE GL_ONE
        rgbGen wave square 0 1 0 0.55
    }
    {
        map textures/taspir/display_controls4_glow3
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0 1 0 0.85
    }
}

textures/taspir/power_tube
{
	qer_editorimage	textures/taspir/power_tube
	q3map_nolightmap
    {
        map textures/taspir/ggoo_blue
        blendFunc GL_ONE GL_SRC_COLOR
        tcMod scroll 0 0.5
    }
    {
        map textures/taspir/ggoo_blue
        blendFunc GL_DST_COLOR GL_ZERO
        tcMod scroll 0 0.1
    }
    {
        map textures/taspir/power_tube
        blendFunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

textures/taspir/256_128rustv
{
	qer_editorimage	textures/taspir/256_128rustv
    {
        map $lightmap
    }
    {
        map textures/taspir/256_128rustv
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/taspir/256_128rustv_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/taspir/panellong
{
	qer_editorimage	textures/taspir/panellong
    {
        map $lightmap
    }
    {
        map textures/taspir/panellong
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/taspir/panellong_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/taspir/smalltrim
{
	qer_editorimage	textures/taspir/smalltrim
    {
        map $lightmap
    }
    {
        map textures/taspir/smalltrim
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/taspir/smalltrim_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/taspir/t_trim2
{
	qer_editorimage	textures/taspir/t_trim2
    {
        map $lightmap
    }
    {
        map textures/taspir/t_trim2
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/taspir/t_trim2_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/taspir/door
{
	qer_editorimage	textures/taspir/door
    {
        map $lightmap
    }
    {
        map textures/taspir/door
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/taspir/door_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/taspir/door_off
{
	qer_editorimage	textures/taspir/door
    {
        map $lightmap
    }
    {
        map textures/taspir/door
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/taspir/floor
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/taspir/floor
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/skies/taspir1
{
// sun 0.827451 0.92549 0.996078 205 345 75

// R G B intensity degrees elevation

// sun 1 0.85 0.5 400 300 70

	qer_editorimage	textures/skies/sky.tga
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	q3map_nolightmap
	skyParms	- 512 -
}

textures/fogs/taspir1_fog
{
// (  Red          Green        Blue     ) Distance

	qer_editorimage	textures/colors/white.tga
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0.8 0.7 0.4 ) 4096.0
}

