textures/byss/byss_basic1_piller_light
{
    {
        map $lightmap
    }
    {
        map textures/byss/byss_basic1_piller_light
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/byss/byss_piller_glow
        blendFunc GL_ONE GL_ONE
        glow
    }
}

textures/byss/emiter
{
	qer_editorimage	textures/byss/emitter
    {
        map $lightmap
    }
    {
        map textures/byss/emitter
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/byss/emitter_glow
        blendFunc GL_ONE GL_ONE
        glow
    }
}

textures/byss/static_field
{
	surfaceparm	nomarks
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
    {
        map textures/byss/static_field
        tcMod scroll 0 -5
    }
    {
        map textures/byss/static_field2
        blendFunc GL_ONE GL_ONE
        tcMod scroll 0 2
    }
    {
        map textures/byss/static_field3
        blendFunc GL_ONE GL_ONE
        rgbGen wave sawtooth 0 4 0 2
        tcMod scroll 0 2
    }
}

textures/byss/large_floor
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/byss/large_floor
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/byss/env_large_floor
        blendFunc GL_SRC_ALPHA GL_ONE
        alphaGen const 0.25
        tcGen environment
    }
}

textures/byss/pathway
{
	qer_editorimage	textures/byss/pathway
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/byss/pathway
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/byss/env_large_floor
        blendFunc GL_SRC_ALPHA GL_ONE
        alphaGen const 0.25
        tcGen environment
    }
}

textures/byss/lights
{
    {
        map $lightmap
    }
    {
        map textures/byss/lights
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/byss/lights_glw
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/byss/byss_light
{
    {
        map $lightmap
    }
    {
        map textures/byss/byss_light
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/byss/byss_light_glw
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/byss/tractor
{
	q3map_nolightmap
    {
        map textures/byss/tractor
        blendFunc GL_ONE GL_SRC_COLOR
        tcMod scroll -0.5 0
    }
}

textures/byss/isd_basepanel1
{
	qer_editorimage	textures/byss/isd_basepanel1
	q3map_nonplanar
	q3map_splotchfix
	q3map_forcemeta
    {
        map $lightmap
    }
    {
        map textures/byss/isd_basepanel1
        blendFunc GL_DST_COLOR GL_ZERO
        tcMod scale 0.8 0.8
    }
    {
        map textures/byss/isd_basepanel2
        blendFunc GL_DST_COLOR GL_ZERO
        tcMod scale 0.5 0.5
    }
}

textures/byss/isd_midsection
{
	qer_editorimage	textures/byss/isd_midsection
    {
        map $lightmap
    }
    {
        map textures/byss/isd_midsection
        blendFunc GL_DST_COLOR GL_ZERO
        tcMod scale 0.8 0.8
    }
    {
        map textures/byss/isd_midsection_glow
        blendFunc GL_ONE GL_ONE
        glow
        tcMod scale 0.8 0.8
    }
}

textures/byss/static_field_2sided
{
	qer_editorimage	textures/byss/static_field
	surfaceparm	nomarks
	surfaceparm	nonopaque
	surfaceparm	forcefield
	surfaceparm	trans
	q3map_nolightmap
	cull	twosided
    {
        map textures/byss/static_field
        blendFunc GL_ONE GL_ONE
        tcMod scroll 0 -5
    }
    {
        map textures/byss/static_field2
        blendFunc GL_ONE GL_ONE
        tcMod scroll 0 2
    }
    {
        map textures/byss/static_field3
        blendFunc GL_ONE GL_ONE
        rgbGen wave sawtooth 0 4 0 2
        tcMod scroll 0 2
    }
}

textures/byss/static_field_absorb
{
// surfaceparm	forcefield

	qer_editorimage	textures/byss/static_field
	surfaceparm	nomarks
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
	cull	twosided
    {
        map textures/byss/static_field
        blendFunc GL_ONE GL_ONE
        tcMod scroll 0 -5
    }
    {
        map textures/byss/static_field2
        blendFunc GL_ONE GL_ONE
        tcMod scroll 0 2
    }
    {
        map textures/byss/static_field3
        blendFunc GL_ONE GL_ONE
        rgbGen wave sawtooth 0 4 0 2
        tcMod scroll 0 2
    }
}

textures/byss/isd_bridge
{
    {
        map $lightmap
    }
    {
        map textures/byss/isd_bridge
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/byss/isd_bridge_glw
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/byss/isd_midsection2
{
    {
        map $lightmap
    }
    {
        map textures/byss/isd_midsection2
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/byss/isd_midsection2_glw
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/byss/isd_decal1
{
	polygonOffset
    {
        map textures/byss/isd_decal1
        alphaFunc GE128
        blendFunc GL_ONE GL_ZERO
        rgbGen identity
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        depthFunc equal
        rgbGen identity
    }
}

textures/byss/isd_decal2
{
	polygonOffset
    {
        map textures/byss/isd_decal2
        alphaFunc GE128
        blendFunc GL_ONE GL_ZERO
        rgbGen identity
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        depthFunc equal
        rgbGen identity
    }
}

textures/byss/isd_decal3
{
	polygonOffset
	q3map_nolightmap
    {
        map textures/byss/isd_decal3
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

textures/byss/isd_decal4
{
	polygonOffset
	q3map_nolightmap
    {
        map textures/byss/isd_decal4
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

textures/byss/byss_gate_onoff
{
    {
        map $lightmap
    }
    {
        map textures/byss/byss_gate_onoff
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        oneshotanimMap 1 textures/byss/byss_gate_onoffr textures/byss/byss_gate_onoffg 
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/byss/byss_gatedoor_onoff
{
    {
        map $lightmap
    }
    {
        map textures/byss/byss_gatedoor_onoff
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        oneshotanimMap 1 textures/byss/byss_gatedoor_onoffr textures/byss/byss_gatedoor_onoffg 
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/byss/door1
{
    {
        map $lightmap
    }
    {
        map textures/byss/door1
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        oneshotanimMap 1 textures/byss/door1_red textures/byss/door1_grn 
        blendFunc GL_ONE GL_ONE
        glow
    }
}

textures/byss/byss_switch
{
    {
        map $lightmap
    }
    {
        map textures/byss/byss_switch
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/byss/byss_switch_glow
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
    {
        oneshotanimMap 1 textures/byss/byss_switch_locked textures/byss/byss_switch_open 
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

textures/byss/byss_crate_side
{
    {
        map $lightmap
    }
    {
        map textures/byss/byss_crate_side
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/byss/byss_crate_side_glw
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/byss/byss_crate_top
{
    {
        map $lightmap
    }
    {
        map textures/byss/byss_crate_top
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/byss/byss_crate_top_glw
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

textures/byss/static_field2
{
	qer_editorimage	textures/byss/static_field
	surfaceparm	nomarks
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
	cull	twosided
    {
        map textures/byss/power2
        blendFunc GL_ONE GL_ONE_MINUS_SRC_COLOR
        tcMod scroll 0 -2
    }
    {
        map textures/byss/power3
        blendFunc GL_ONE GL_ONE_MINUS_SRC_COLOR
        tcMod scroll -2 0
    }
    {
        map textures/byss/power3
        blendFunc GL_ONE GL_ONE_MINUS_SRC_COLOR
        tcMod scroll 2 0
    }
    {
        map textures/byss/power2
        blendFunc GL_ONE GL_ONE_MINUS_SRC_COLOR
        tcMod scroll 0 2
    }
}

textures/byss/static_field3
{
	qer_editorimage	textures/byss/static_field
	surfaceparm	nomarks
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
	cull	twosided
    {
        map textures/byss/power22
        blendFunc GL_ONE GL_ONE_MINUS_SRC_COLOR
        tcMod scroll 0 -2
    }
    {
        map textures/byss/power33
        blendFunc GL_ONE GL_ONE_MINUS_SRC_COLOR
        tcMod scroll -2 0
    }
    {
        map textures/byss/power33
        blendFunc GL_ONE GL_ONE_MINUS_SRC_COLOR
        tcMod scroll 2 0
    }
    {
        map textures/byss/power22
        blendFunc GL_ONE GL_ONE_MINUS_SRC_COLOR
        tcMod scroll 0 2
    }
}

textures/byss/static_field4
{
	qer_editorimage	textures/byss/static_field
	surfaceparm	nomarks
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
	cull	twosided
    {
        map textures/byss/power222
        blendFunc GL_ONE GL_ONE_MINUS_SRC_COLOR
        tcMod turb 0 1 0 1
    }
    {
        map textures/byss/power222
        blendFunc GL_ONE GL_ONE_MINUS_SRC_COLOR
        tcMod rotate -400
    }
    {
        map textures/byss/power222
        blendFunc GL_ONE GL_ONE_MINUS_SRC_COLOR
        tcMod rotate 400
    }
}

textures/byss/static_field5
{
	qer_editorimage	textures/byss/static_field
	surfaceparm	nomarks
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
	cull	twosided
    {
        map textures/byss/power22
        blendFunc GL_ONE GL_ONE_MINUS_SRC_COLOR
        tcMod turb 0 1 0 1
    }
    {
        map textures/byss/power33
        blendFunc GL_ONE GL_ONE_MINUS_SRC_COLOR
        tcMod rotate -400
    }
    {
        map textures/byss/power33
        blendFunc GL_ONE GL_ONE_MINUS_SRC_COLOR
        tcMod rotate 400
    }
}

textures/byss/byss_basicwall
{
	qer_editorimage	textures/byss/byss_basicwall
    {
        map $lightmap
    }
    {
        map textures/byss/byss_basicwall
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/byss/byss_basicwall_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/byss/floor_byss
{
	qer_editorimage	textures/byss/large_floor
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/byss/floor_byss
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/byss/env_large_floor
        blendFunc GL_SRC_ALPHA GL_ONE
        alphaGen const 0.25
        tcGen environment
    }
}

textures/byss/floor_byss2
{
	qer_editorimage	textures/byss/large_floor
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/byss/floor_byss2
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/byss/env_large_floor
        blendFunc GL_SRC_ALPHA GL_ONE
        alphaGen const 0.25
        tcGen environment
    }
}

textures/byss/basicred
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/byss/basicred
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/byss/byss_basic1
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/byss/byss_basic1
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/byss/byss_basic2
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/byss/byss_basic2
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/byss/biglight
{
	qer_editorimage	textures/byss/biglight
    {
        map $lightmap
    }
    {
        map textures/byss/biglight
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/byss/biglight_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/byss/biglight_2
{
	qer_editorimage	textures/byss/biglight_2
    {
        map $lightmap
    }
    {
        map textures/byss/biglight_2
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/byss/biglight_2_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/byss/byss_light2
{
	qer_editorimage	textures/byss/byss_light2
    {
        map $lightmap
    }
    {
        map textures/byss/byss_light2
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/byss/byss_light2_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/byss/trim6
{
    {
        map $lightmap
    }
    {
        map textures/byss/trim6
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/byss/trim6_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/byss/stairs_front
{
    {
        map $lightmap
    }
    {
        map textures/byss/stairs_front
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/byss/stairs_front_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/byss/byss_ghall_trim
{
	qer_editorimage	textures/byss/byss_ghall_trim
    {
        map $lightmap
    }
    {
        map textures/byss/byss_ghall_trim
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/byss/byss_ghall_trim_glow
        blendFunc GL_ONE GL_ONE
        glow
    }
}

textures/byss/piper5
{
    {
        map $lightmap
    }
    {
        map textures/byss/piper5
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/byss/piper5_glow
        blendFunc GL_ONE GL_ONE
    }
}

models/map_objects/byss/tractor_base
{
	q3map_nolightmap
    {
        map models/map_objects/byss/tractor_base
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/byss/tractor_base_glw
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

models/map_objects/byss/tractor_base_panel
{
	q3map_nolightmap
    {
        map models/map_objects/byss/tractor_base_panel
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/byss/tractor_base_panel_glw
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

models/map_objects/byss/tractor_top
{
	q3map_nolightmap
    {
        map models/map_objects/byss/tractor_top
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/byss/tractor_top_glw
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

models/map_objects/byss/tractor_top_panel
{
	q3map_nolightmap
    {
        map models/map_objects/byss/tractor_top_panel
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/byss/tractor_top_panel_glw
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

models/map_objects/byss/brace
{
	q3map_nolightmap
    {
        map models/map_objects/byss/brace
        rgbGen vertex
    }
    {
        map models/map_objects/byss/brace_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

models/map_objects/byss/brace_light
{
	qer_editorimage	models/map_objects/byss/brace
	q3map_nolightmap
    {
        map models/map_objects/byss/brace_light
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/byss/brace_light_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

models/map_objects/byss/firing_station_consoles
{
	q3map_nolightmap
    {
        map models/map_objects/byss/firing_station_consoles
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/byss/firing_station_consoles_glow
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

models/map_objects/byss/control_panel_lights
{
	q3map_nolightmap
    {
        map models/map_objects/byss/control_panel_lights
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/byss/control_panel_lights_glow
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

models/map_objects/byss/firing_station
{
	qer_editorimage	models/map_objects/byss/firing_station
    {
        map models/map_objects/byss/firing_station
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/byss/firing_station_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

models/map_objects/byss/control_panel
{
	qer_editorimage	models/map_objects/byss/control_panel
    {
        map models/map_objects/byss/control_panel
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/byss/control_panel_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

