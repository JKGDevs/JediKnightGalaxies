textures/vjun/grate
{
	qer_editorimage	textures/vjun/grate
	surfaceparm	nonopaque
	surfaceparm	trans
	cull	twosided
    {
        map textures/vjun/grate
        alphaFunc GE192
        blendFunc GL_ONE GL_ZERO
        depthWrite
        rgbGen identity
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        depthFunc equal
    }
}

textures/vjun/grate2
{
	qer_editorimage	textures/vjun/grate2
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_material	HollowMetal
	cull	twosided
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        depthFunc equal
    }
    {
        map textures/vjun/grate2
        alphaFunc LT128
        blendFunc GL_ONE GL_ZERO
        depthWrite
        rgbGen identity
    }
}

textures/vjun/rocky_phong
{
	q3map_shadeangle	120
	qer_editorimage	textures/vjun/o_ground2
	q3map_nolightmap
	q3map_nonplanar
	q3map_onlyvertexlighting
    {
// tcMod scale 0.25 0.25

        map textures/vjun/o_ground2
        rgbGen vertex
        tcGen base
    }
}

textures/vjun/wall14
{
    {
        map $lightmap
    }
    {
        map textures/vjun/wall14
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/vjun/wall14_glw
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/vjun/vj4vertexphong
{
// q3map_forcemeta

	q3map_shadeangle	120
	qer_editorimage	models/map_objects/vjun/vj4
	q3map_nolightmap
	q3map_nonplanar
	q3map_onlyvertexlighting
    {
        map models/map_objects/vjun/vj4
        rgbGen vertex
    }
}

textures/vjun/hangardoor
{
    {
        map $lightmap
    }
    {
        map textures/vjun/hangardoor
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/vjun/hangardoor_glw
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 0 1 0.125 0.5
    }
}

textures/vjun/hangar_console
{
    {
        map $lightmap
    }
    {
        map textures/vjun/hangar_console
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/vjun/hangar_console_glw
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/vjun/hangar_console2
{
    {
        map $lightmap
    }
    {
        map textures/vjun/hangar_console2
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/vjun/hangar_console2_glw
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/vjun/hangar_console3
{
    {
        map $lightmap
    }
    {
        map textures/vjun/hangar_console3
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/vjun/hangar_console3_glw
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/vjun/light_strip
{
    {
        map $lightmap
    }
    {
        map textures/vjun/light_strip
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/vjun/light_strip_glw
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/vjun/light_strip2
{
    {
        map $lightmap
    }
    {
        map textures/vjun/light_strip2
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/vjun/light_strip2_glw
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/vjun/wall_saberroom
{
    {
        map $lightmap
    }
    {
        map textures/vjun/wall_saberroom
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/vjun/wall_saberroom_glw
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/vjun/door1_onoff
{
    {
        map $lightmap
    }
    {
        map textures/vjun/door1_onoff
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        oneshotanimMap 1 textures/vjun/door1_onoffr textures/vjun/door1_onoffg 
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
        alphaGen identity
    }
}

textures/vjun/door2_onoff
{
    {
        map $lightmap
    }
    {
        map textures/vjun/door2_onoff
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        oneshotanimMap 1 textures/vjun/door2_onoff_glowr textures/vjun/door2_onoff_glowg 
        blendFunc GL_ONE GL_ONE
        glow
        alphaGen identity
    }
}

textures/vjun/o_door_onoff
{
    {
        map $lightmap
    }
    {
        map textures/vjun/o_door_onoff
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        oneshotanimMap 1 textures/vjun/o_door_onoffr textures/vjun/o_door_onoffg 
        blendFunc GL_ONE GL_ONE
        glow
        alphaGen identity
    }
}

textures/vjun/lights3
{
    {
        map $lightmap
    }
    {
        map textures/vjun/lights3
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/vjun/lights3_glw
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/vjun/light_vjun
{
    {
        map $lightmap
    }
    {
        map textures/vjun/light_vjun
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/vjun/light_vjun_glw
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/vjun/vader_floor2
{
	q3map_material	SolidMetal
	q3map_nolightmap
    {
        map textures/vjun/vader_floor2
        rgbGen vertex
    }
    {
        map textures/byss/env_large_floor2
        blendFunc GL_ONE GL_ONE_MINUS_SRC_COLOR
        rgbGen identity
        tcGen environment
    }
    {
        map textures/vjun/vader_floor2_mask
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
    {
        map textures/vjun/vader_floor2_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/vjun/vader_trim3
{
	q3map_nolightmap
    {
        map textures/vjun/vader_trim3
        rgbGen vertex
    }
    {
        map textures/byss/env_large_floor2
        blendFunc GL_SRC_ALPHA GL_ONE
        rgbGen identity
        tcGen environment
    }
    {
        map textures/vjun/vader_trim3_mask
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

textures/vjun/floor3
{
    {
        map $lightmap
    }
    {
        map textures/vjun/floor3
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/common/environ1
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        tcGen environment
    }
    {
        map textures/vjun/floor3_mask
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map textures/vjun/floor3_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/vjun/wall8
{
	q3map_nolightmap
    {
        map textures/vjun/wall8
        rgbGen vertex
    }
    {
        map textures/byss/env_large_floor2
        blendFunc GL_SRC_ALPHA GL_ONE
        rgbGen identity
        tcGen environment
    }
    {
        map textures/vjun/wall8_mask
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

textures/vjun/vader_floor
{
	q3map_material	SolidMetal
	q3map_nolightmap
    {
        map textures/vjun/vader_floor
        rgbGen vertex
    }
    {
        map textures/byss/env_large_floor2
        blendFunc GL_SRC_ALPHA GL_ONE
        rgbGen identity
        tcGen environment
    }
    {
        map textures/vjun/vader_floor_mask
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

textures/vjun/walk_ceiling
{
	q3map_material	SolidMetal
	q3map_nolightmap
    {
        map textures/vjun/walk_ceiling
        rgbGen vertex
    }
    {
        map textures/byss/env_large_floor2
        blendFunc GL_SRC_ALPHA GL_ONE
        rgbGen identity
        tcGen environment
    }
    {
        map textures/vjun/walk_ceiling_mask
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

textures/vjun/vader_walkway
{
	q3map_material	SolidMetal
	q3map_nolightmap
    {
        map textures/vjun/vader_walkway
        rgbGen vertex
    }
    {
        map textures/byss/env_large_floor2
        blendFunc GL_SRC_ALPHA GL_ONE
        rgbGen identity
        alphaGen const 0.25
        tcGen environment
    }
    {
        map textures/vjun/vader_walkway_mask
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

textures/vjun/wall9
{
	q3map_nolightmap
    {
        map textures/vjun/wall9
        rgbGen vertex
    }
    {
        map textures/byss/env_large_floor2
        blendFunc GL_SRC_ALPHA GL_ONE
        rgbGen identity
        alphaGen const 0.25
        tcGen environment
    }
    {
        map textures/vjun/wall9_mask
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
    {
        map textures/vjun/wall9_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/vjun/vader_trim2
{
	q3map_nolightmap
    {
        map textures/vjun/vader_trim2
        rgbGen vertex
    }
    {
        map textures/byss/env_large_floor2
        blendFunc GL_SRC_ALPHA GL_ONE
        rgbGen identity
        tcGen environment
    }
    {
        map textures/vjun/vader_trim2_mask
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

textures/vjun/grate1
{
	qer_editorimage	textures/vjun/grate1
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_material	HollowMetal
	cull	twosided
    {
        map textures/vjun/grate1
        alphaFunc GE192
        blendFunc GL_ONE GL_ZERO
        depthWrite
        rgbGen identity
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        depthFunc equal
        rgbGen identity
    }
}

textures/vjun/light_vjun_amber
{
	qer_editorimage	textures/vjun/light_vjunamber
    {
        map $lightmap
    }
    {
        map textures/vjun/light_vjunamber
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/vjun/light_vjunamber_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/vjun/grate_light_amber
{
	qer_editorimage	textures/vjun/grateamber_light
    {
        map $lightmap
    }
    {
        map textures/vjun/grateamber_light
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/vjun/grateamber_light_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/vjun/screen1
{
	q3map_nolightmap
	q3map_novertexshadows
    {
        map textures/vjun/screen1
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

textures/vjun/frametech
{
    {
        map $lightmap
    }
    {
        map textures/vjun/frametech
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/vjun/frametech_glw
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/vjun/lightground
{
    {
        map $lightmap
    }
    {
        map textures/vjun/lightground
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/vjun/lightground_glw
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/vjun/light2
{
    {
        map $lightmap
    }
    {
        map textures/vjun/light2
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/vjun/light2_glw
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/vjun/tech
{
	q3map_nolightmap
    {
        map textures/vjun/tech
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
    {
        map textures/vjun/tech
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen const ( 0.700000 0.700000 0.700000 )
    }
}

textures/vjun/pipe_effect
{
	q3map_nolightmap
    {
        map textures/vjun/pipe_effect
        blendFunc GL_ONE GL_ZERO
        tcMod scroll 2 0
        tcMod scale 0.5 1
    }
    {
        map textures/vjun/pipe_effect2
        blendFunc GL_ONE GL_ONE
        rgbGen wave triangle 0 1 0 1
        tcMod scroll 2 0
    }
    {
        map textures/vjun/pipe_effect3
        blendFunc GL_ONE GL_ONE
        rgbGen wave inversesawtooth 0 1 0 9
        tcMod scroll 5 0
    }
}

textures/vjun/circle_light
{
    {
        map $lightmap
    }
    {
        map textures/vjun/circle_light
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/vjun/circle_light_glw
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/vjun/vader_switch
{
    {
        map $lightmap
    }
    {
        map textures/vjun/vader_switch
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/vjun/vader_switch_on
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/vjun/vader_switch_off
{
	qer_editorimage	textures/vjun/vader_switch
    {
        map $lightmap
    }
    {
        map textures/vjun/vader_switch
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/vjun/vader_switch_off
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/vjun/redlight_vjun
{
	qer_editorimage	textures/vjun/redlight_vjun
    {
        map $lightmap
    }
    {
        map textures/vjun/redlight_vjun
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/vjun/redlight_vjun_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/vjun/redlight_strip
{
	qer_editorimage	textures/vjun/redlight_strip
    {
        map $lightmap
    }
    {
        map textures/vjun/redlight_strip
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/vjun/redlight_strip_glw
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/vjun/redlights3
{
	qer_editorimage	textures/vjun/redlights3
    {
        map $lightmap
    }
    {
        map textures/vjun/redlights3
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/vjun/redlights3_glw
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/vjun/redgrate_light
{
	qer_editorimage	textures/vjun/redgrate_light
    {
        map $lightmap
    }
    {
        map textures/vjun/redgrate_light
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/vjun/redgrate_light_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/vjun/explosive_decal
{
	polygonOffset
    {
        map textures/vjun/explosive_decal
        blendFunc GL_ONE GL_ONE
    }
    {
        map textures/vjun/explosive_decal_glow
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0 1 0 0.5
    }
}

models/map_objects/vjun/egg_console
{
    {
        map $lightmap
    }
    {
        map models/map_objects/vjun/egg_console
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map models/map_objects/vjun/egg_console_glw
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

models/map_objects/vjun/lamp
{
    {
        map $lightmap
    }
    {
        map models/map_objects/vjun/lamp
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map models/map_objects/vjun/lamp_glw
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

models/map_objects/vjun/vj4_b
{
	qer_editorimage	models/map_objects/vjun/vj4
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/vjun/vj4
        rgbGen lightingDiffuse
        tcMod scale 4 4
    }
}

models/map_objects/vjun/door_frame
{
	q3map_nolightmap
    {
        map models/map_objects/vjun/door_frame
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/vjun/door_frame_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

models/map_objects/vjun/prong
{
    {
        map $lightmap
    }
    {
        map models/map_objects/vjun/prong
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map models/map_objects/vjun/prong_glw
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

models/map_objects/vjun/globe_grid
{
	q3map_nolightmap
	cull	twosided
    {
        map models/map_objects/vjun/globe_grid
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/vjun/globe_continents
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
    {
        map models/map_objects/vjun/globe_blink1
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 0 1 10 1.5
    }
    {
        map models/map_objects/vjun/globe_blink2
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 0 1 2 0.5
    }
}

models/map_objects/vjun/electric_pipe_shdr
{
    {
        map models/map_objects/vjun/electric_pipe_shdr
        blendFunc GL_ONE GL_ZERO
        tcMod scroll 0 2
    }
    {
        map models/map_objects/vjun/pipe_effect2
        blendFunc GL_ONE GL_ONE
        rgbGen wave triangle 0 1 0 1
        tcMod scroll 0 1.5
    }
    {
        map models/map_objects/vjun/pipe_effect3
        blendFunc GL_ONE GL_ONE
        rgbGen wave inversesawtooth 0 1 0 9
        tcMod scroll 0 2
        tcMod scale 0.5 1
    }
}

models/map_objects/vjun/window_frame
{
    {
        map models/map_objects/vjun/window_frame
        rgbGen lightingDiffuse
    }
}

models/map_objects/vjun/control_station_panel3
{
    {
        map models/map_objects/vjun/control_station_panel3
        rgbGen lightingDiffuse
    }
    {
        animMap 4 models/map_objects/vjun/control_station_panel3_glow1 models/map_objects/vjun/control_station_panel3_glow2 models/map_objects/vjun/control_station_panel3_glow3 models/map_objects/vjun/control_station_panel3_glow4 models/map_objects/vjun/control_station_panel3_glow2 models/map_objects/vjun/control_station_panel3_glow4 models/map_objects/vjun/control_station_panel3_glow3 
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

models/map_objects/vjun/control_station_panel4
{
    {
        map models/map_objects/vjun/control_station_panel4
        rgbGen lightingDiffuse
    }
    {
        animMap 2 models/map_objects/vjun/control_station_panel4_glow1 models/map_objects/vjun/control_station_panel4_glow2 models/map_objects/vjun/control_station_panel4_glow3 models/map_objects/vjun/control_station_panel4_glow4 models/map_objects/vjun/control_station_panel4_glow3 models/map_objects/vjun/control_station_panel4_glow4 models/map_objects/vjun/control_station_panel4_glow2 models/map_objects/vjun/control_station_panel4_glow3 models/map_objects/vjun/control_station_panel4_glow1 models/map_objects/vjun/control_station_panel4_glow4 
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

models/map_objects/vjun/control_station_panel2
{
	q3map_nolightmap
    {
        map models/map_objects/vjun/control_station_panel2
        rgbGen lightingDiffuse
    }
    {
        animMap 2 models/map_objects/vjun/control_station_panel2_glow1 models/map_objects/vjun/control_station_panel2_glow1 models/map_objects/vjun/control_station_panel2_glow2 models/map_objects/vjun/control_station_panel2_glow2 models/map_objects/vjun/control_station_panel2_glow3 models/map_objects/vjun/control_station_panel2_glow3 models/map_objects/vjun/control_station_panel2_glow4 
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

models/map_objects/vjun/rubble_glow
{
    {
        map models/map_objects/vjun/rubble_glow
        blendFunc GL_ONE GL_ONE
    }
}

models/map_objects/vjun/control_station_panel5
{
	q3map_nolightmap
    {
        map models/map_objects/vjun/control_station_panel5
        rgbGen lightingDiffuse
    }
    {
        animMap 1 models/map_objects/vjun/control_station_panel5_glow1 models/map_objects/vjun/control_station_panel5_glow1 models/map_objects/vjun/control_station_panel5_glow2 models/map_objects/vjun/control_station_panel5_glow2 models/map_objects/vjun/control_station_panel5_glow3 models/map_objects/vjun/control_station_panel5_glow3 models/map_objects/vjun/control_station_panel5_glow4 models/map_objects/vjun/control_station_panel5_glow4 
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

models/map_objects/vjun/block_light
{
    {
        map $lightmap
    }
    {
        map models/map_objects/vjun/block_light
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map models/map_objects/vjun/block_light_glw
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

models/map_objects/vjun/ceiling_light_glass
{
    {
        map $lightmap
    }
    {
        map models/map_objects/vjun/ceiling_light_glass
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map models/map_objects/vjun/ceiling_light_glass_glw
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

models/map_objects/vjun/reflector
{
    {
        map models/map_objects/vjun/reflector
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/vjun/reflector_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/imperial/door_1_onoff
{
    {
        map $lightmap
    }
    {
        map textures/imperial/door_1_onoff
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        oneshotanimMap 1 textures/imperial/door_1_onoffg textures/imperial/door_1_onoffr 
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
        alphaGen identity
    }
}

textures/imperial/door_2_onoff
{
    {
        map $lightmap
    }
    {
        map textures/imperial/door_2_onoff
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        oneshotanimMap 1 textures/imperial/door_2_onoffg textures/imperial/door_2_onoffr 
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
        alphaGen identity
    }
}

textures/common/water_vjun
{
	q3map_material	Water
	q3map_nolightmap
    {
        map textures/common/water_vjun
        rgbGen vertex
        tcMod turb 0 0.04 0 0.1
        tcMod scroll 0.03 0.01
    }
}

models/map_objects/imperial/implight2
{
    {
        map models/map_objects/imperial/implight2
    }
    {
        map models/map_objects/imperial/implight2_glow
        blendFunc GL_ONE GL_ONE
        glow
    }
}

