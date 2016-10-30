textures/desert/t_rockwall2
{
	q3map_shadeangle	150
	qer_editorimage	textures/desert/t_rockwall2
	q3map_nonplanar
	q3map_splotchfix
    {
        map $lightmap
        rgbGen identity
    }
    {
        map textures/desert/t_rockwall2
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/desert/sandfloor2b
{
	q3map_shadeangle	150
	qer_editorimage	textures/desert/sandfloor2b
	q3map_material	Sand
	q3map_nonplanar
	q3map_splotchfix
    {
        map $lightmap
        rgbGen identity
    }
    {
        map textures/desert/sandfloor2b
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/desert/light_yellow
{
    {
        map $lightmap
    }
    {
        map textures/desert/light_yellow
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/desert/light_yellow_glw
        blendFunc GL_ONE GL_ONE
        glow
    }
}

textures/desert/stucco_grime_detail
{
	qer_editorimage	textures/desert/stucco_grime_bottom
    {
        map $lightmap
    }
    {
        map textures/desert/stucco_grime_bottom
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/common/detail4
        blendFunc GL_DST_COLOR GL_ZERO
        detail
        tcMod scale 4 8
    }
}

textures/desert/door_switch_blue_decal
{
	qer_editorimage	textures/desert/door_switch_blue
	polygonOffset
	q3map_nolightmap
    {
        map textures/desert/door_switch_blue
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

textures/desert/door_switch_red_decal
{
	qer_editorimage	textures/desert/door_switch_red
	polygonOffset
	q3map_nolightmap
    {
        map textures/desert/door_switch_red
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

textures/desert/door_switch_stucco_decal
{
	qer_editorimage	textures/desert/door_switch_stucco
	polygonOffset
	q3map_nolightmap
    {
        map textures/desert/door_switch_stucco
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

textures/desert/t_rockwall1_clip
{
// q3map_shadeangle	120

// q3map_nonplanar

	q3map_shadeangle	120
	qer_editorimage	textures/desert/t_rockwall1
	q3map_nolightmap
	q3map_nonplanar
	q3map_forcemeta
	q3map_onlyvertexlighting
    {
        map textures/desert/t_rockwall1
        rgbGen vertex
    }
    {
        map textures/common/detail9
        blendFunc GL_DST_COLOR GL_ONE
        rgbGen identity
        tcMod scale 10 10
    }
}

textures/desert/t_rockwall1_clip_sm2
{
// q3map_forcemeta

	q3map_shadeangle	120
	qer_editorimage	textures/desert/t_rockwall1
	q3map_nolightmap
	q3map_nonplanar
	q3map_clipmodel
	q3map_onlyvertexlighting
    {
        map textures/desert/t_rockwall1
        rgbGen vertex
    }
    {
        map textures/common/detail9
        blendFunc GL_DST_COLOR GL_ONE
        rgbGen identity
        tcMod scale 10 10
    }
}

textures/desert/t_rockwall1_clip_sm
{
// q3map_forcemeta

	q3map_shadeangle	120
	qer_editorimage	textures/desert/t_rockwall1
	q3map_nolightmap
	q3map_nonplanar
	q3map_clipmodel
	q3map_onlyvertexlighting
    {
        map textures/desert/t_rockwall1
        rgbGen vertex
    }
}

textures/desert/sandfloor2_Phong
{
	q3map_shadeangle	120
	qer_editorimage	textures/desert/sandfloor2
	q3map_material	Sand
	q3map_nolightmap
	q3map_nonplanar
	q3map_onlyvertexlighting
    {
        map textures/desert/sandfloor2
        rgbGen vertex
    }
}

textures/desert/s_switch_locked
{
    {
        map $lightmap
    }
    {
        map textures/desert/s_switch_locked
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/desert/s_switch_locked_glw
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0 1 0 1
        tcMod turb 0 0 0 0
    }
}

textures/desert/s_switch_unlocked
{
    {
        map $lightmap
    }
    {
        map textures/desert/s_switch_unlocked
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/desert/s_switch_unlocked_glw
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

textures/desert/dbay_hud
{
	q3map_nolightmap
    {
        map textures/desert/dbay_hud
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        oneshotanimMap 1 textures/desert/dbay_hud_ll textures/desert/dbay_hud_lr textures/desert/dbay_hud_ul textures/desert/dbay_hud_ur 
        blendFunc GL_ONE GL_ONE
        glow
    }
}

textures/desert/panel_monitor_on
{
    {
        map $lightmap
    }
    {
        map textures/desert/panel_monitor_on
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/desert/panel_monitor_on_glw
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/desert/panel_monitor
{
    {
        map $lightmap
    }
    {
        map textures/desert/panel_monitor
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/desert/panel_monitor_glw
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/desert/s_door_1_onoff
{
    {
        map $lightmap
    }
    {
        map textures/desert/s_door_1_onoff
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        oneshotanimMap 1 textures/desert/s_door_1_onoffr textures/desert/s_door_1_onoffg 
        blendFunc GL_ONE GL_ONE
        glow
        alphaGen identity
    }
}

textures/desert/s_door_2_onoff
{
    {
        map $lightmap
    }
    {
        map textures/desert/s_door_2_onoff
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        oneshotanimMap 1 textures/desert/s_door_2_onoffr textures/desert/s_door_2_onoffg 
        blendFunc GL_ONE GL_ONE
        alphaGen identity
    }
}

textures/desert/s_door_3_onoff
{
    {
        map $lightmap
    }
    {
        map textures/desert/s_door_3_onoff
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        oneshotanimMap 1 textures/desert/s_door_3_onoffr textures/desert/s_door_3_onoffg 
        blendFunc GL_ONE GL_ONE
        glow
        alphaGen identity
    }
}

textures/desert/door_lrg_onoff
{
    {
        map $lightmap
    }
    {
        map textures/desert/door_lrg_onoff
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        oneshotanimMap 1 textures/desert/door_lrg_onoffr textures/desert/door_lrg_onoffg 
        blendFunc GL_ONE GL_ONE
        glow
        alphaGen identity
    }
}

textures/desert/s_console
{
    {
        map $lightmap
    }
    {
        map textures/desert/s_console
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/desert/s_console_glw
        blendFunc GL_ONE GL_ONE
        glow
        alphaGen identity
    }
    {
        map textures/desert/s_console_anim
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 0.5 0.6 0 1.25
    }
    {
        map textures/desert/s_console_anim2
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 0 1.5 0 1
    }
}

textures/desert/s_windowfront_decal
{
	polygonOffset
	q3map_nolightmap
    {
        map textures/desert/s_windowfront_decal
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

textures/desert/s_windowside_decal
{
	polygonOffset
	q3map_nolightmap
    {
        map textures/desert/s_windowside_decal
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

textures/desert/s_light
{
    {
        map $lightmap
    }
    {
        map textures/desert/s_light
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/desert/s_light_glw
        blendFunc GL_ONE GL_ONE
        glow
        alphaGen identity
    }
}

textures/desert/stucco
{
	q3map_material	Plaster
    {
        map $lightmap
    }
    {
        map textures/desert/stucco
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/common/detail5
        blendFunc GL_DST_COLOR GL_ONE
        rgbGen identity
        tcMod scale 3 3
    }
}

textures/desert/stucco_top_grime
{
    {
        map $lightmap
    }
    {
        map textures/desert/stucco_top_grime
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/common/detail5
        blendFunc GL_DST_COLOR GL_ONE
        rgbGen identity
        tcMod scale 3 3
    }
}

textures/desert/stucco_128
{
    {
        map $lightmap
    }
    {
        map textures/desert/stucco_128
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/common/detail5
        blendFunc GL_DST_COLOR GL_ONE
        rgbGen identity
        tcMod scale 3 3
    }
}

textures/desert/stucco_grime
{
    {
        map $lightmap
    }
    {
        map textures/desert/stucco_grime
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/common/detail5
        blendFunc GL_DST_COLOR GL_ONE
        rgbGen identity
        tcMod scale 3 1
    }
}

textures/desert/stucco_grime_bottom
{
    {
        map $lightmap
    }
    {
        map textures/desert/stucco_grime_bottom
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/common/detail5
        blendFunc GL_DST_COLOR GL_ONE
        rgbGen identity
        tcMod scale 3 3
    }
}

textures/desert/stucco_grime_top_bottom
{
    {
        map $lightmap
    }
    {
        map textures/desert/stucco_grime_top_bottom
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/common/detail5
        blendFunc GL_DST_COLOR GL_ONE
        rgbGen identity
        tcMod scale 3 3
    }
}

textures/desert/stucco_rim
{
    {
        map $lightmap
    }
    {
        map textures/desert/stucco_rim
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/common/detail5
        blendFunc GL_DST_COLOR GL_ONE
        rgbGen identity
        tcMod scale 3 3
    }
}

textures/desert/stucco_rim_2
{
    {
        map $lightmap
    }
    {
        map textures/desert/stucco_rim_2
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/common/detail5
        blendFunc GL_DST_COLOR GL_ONE
        rgbGen identity
        tcMod scale 3 3
    }
}

textures/desert/decal_stucco_vent2
{
	polygonOffset
	q3map_material	HollowMetal
	q3map_nolightmap
    {
        map textures/desert/decal_stucco_vent2
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

textures/desert/decal_s_detail1
{
	polygonOffset
	q3map_nolightmap
    {
        map textures/desert/decal_s_detail1
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

textures/desert/decal_stucco_detail1
{
	polygonOffset
	q3map_nolightmap
    {
        map textures/desert/decal_stucco_detail1
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

textures/desert/turbolift_base
{
    {
        map $lightmap
    }
    {
        map textures/desert/turbolift_base
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/desert/turbolift_base_glw
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/desert/decal_stucco_detail1_lightmapped
{
	qer_editorimage	textures/desert/decal_stucco_detail1
	surfaceparm	nonsolid
	polygonOffset
    {
// blendFunc GL_ONE GL_ZERO

        map textures/desert/decal_stucco_detail1
        alphaFunc GE128
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

textures/desert/decal_stucco_vent2_lightmapped
{
	qer_editorimage	textures/desert/decal_stucco_vent2
	surfaceparm	nonsolid
	polygonOffset
	q3map_material	HollowMetal
    {
// blendFunc GL_ONE GL_ZERO

        map textures/desert/decal_stucco_vent2
        alphaFunc GE128
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

textures/desert/decal_stucco_groove_lightmapped
{
	qer_editorimage	textures/desert/decal_stucco_groove
	surfaceparm	nonsolid
	polygonOffset
    {
// blendFunc GL_ONE GL_ZERO

// alphaFunc GE128

// depthWrite

        map textures/desert/decal_stucco_groove
        rgbGen identity
    }
    {
// depthFunc equal

        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/desert/decal_s_detail1_lightmapped
{
	qer_editorimage	textures/desert/decal_s_detail1
	surfaceparm	nonsolid
	polygonOffset
    {
// blendFunc GL_ONE GL_ZERO

        map textures/desert/decal_s_detail1
        alphaFunc GE128
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

textures/desert/pipe1
{
    {
        map $lightmap
    }
    {
        map textures/desert/pipe1
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/common/environ1
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        detail
        tcGen environment
    }
}

textures/desert/pipe2
{
    {
        map $lightmap
    }
    {
        map textures/desert/pipe2
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/common/environ1
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        detail
        tcGen environment
    }
}

textures/desert/turbolift
{
    {
        map $lightmap
    }
    {
        map textures/desert/turbolift
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/desert/turbolift_glw
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/desert/decal_psky_detail1_lightmapped
{
	qer_editorimage	textures/desert/decal_psky_detail1
	surfaceparm	nonsolid
	polygonOffset
    {
// blendFunc GL_ONE GL_ZERO

        map textures/desert/decal_psky_detail1
        alphaFunc GE128
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

textures/desert/decal_psky_groove_lightmapped
{
	qer_editorimage	textures/desert/decal_psky_groove
	surfaceparm	nonsolid
	polygonOffset
    {
// blendFunc GL_ONE GL_ZERO

// alphaFunc GE128

// depthWrite

        map textures/desert/decal_psky_groove
        rgbGen identity
    }
    {
// depthFunc equal

        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/desert/decal_psky_vent2_lightmapped
{
	qer_editorimage	textures/desert/decal_psky_vent2
	surfaceparm	nonsolid
	polygonOffset
	q3map_material	HollowMetal
    {
// blendFunc GL_ONE GL_ZERO

        map textures/desert/decal_psky_vent2
        alphaFunc GE128
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

textures/desert/decal_psky_vent_lightmapped
{
	qer_editorimage	textures/desert/decal_psky_vent
	surfaceparm	nonsolid
	polygonOffset
    {
// blendFunc GL_ONE GL_ZERO

        map textures/desert/decal_psky_vent
        alphaFunc GE128
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

textures/desert/dbay_hud_ll
{
	q3map_nolightmap
    {
        map textures/desert/dbay_hud
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map textures/desert/dbay_hud_ll
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/desert/dbay_hud_lr
{
	q3map_nolightmap
    {
        map textures/desert/dbay_hud
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map textures/desert/dbay_hud_lr
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/desert/dbay_hud_ul
{
	q3map_nolightmap
    {
        map textures/desert/dbay_hud
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map textures/desert/dbay_hud_ul
        blendFunc GL_ONE GL_ONE
        glow
    }
}

textures/desert/dbay_hud_ur
{
	q3map_nolightmap
    {
        map textures/desert/dbay_hud
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map textures/desert/dbay_hud_ur
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/desert/panel_dbaycontrol
{
    {
        map $lightmap
    }
    {
        map textures/desert/panel_dbaycontrol
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/desert/panel_dbaycontrol_glow
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

textures/desert/panel_dbay
{
    {
        map $lightmap
    }
    {
        map textures/desert/panel_dbay
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/desert/panel_dbay_glow
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

textures/desert/panel_tall
{
    {
        map $lightmap
    }
    {
        map textures/desert/panel_tall
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/desert/panel_tall_glow
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

textures/desert/turbolift_int
{
    {
        map $lightmap
    }
    {
        map textures/desert/turbolift_int
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/desert/turbolift_int_glw
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

textures/desert/s_tank
{
    {
        map $lightmap
    }
    {
        map textures/desert/s_tank
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/desert/fuel1
        blendFunc GL_ONE GL_ONE
        detail
        rgbGen identity
        tcMod scroll -0.25 0
    }
    {
        map textures/desert/fuel2
        blendFunc GL_ONE GL_ONE
        detail
        rgbGen identity
        tcMod scroll -0.18 0
    }
    {
        map textures/desert/s_tank
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        detail
        rgbGen vertex
    }
}

textures/desert/blotch_rust
{
	polygonOffset
    {
        map textures/desert/blotch_rust
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

textures/desert/bloch1
{
	polygonOffset
    {
        map textures/desert/bloch1
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

textures/desert/bloch2
{
	polygonOffset
    {
        map textures/desert/bloch2
        alphaFunc GE128
        blendFunc GL_ONE GL_ZERO
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        depthFunc equal
        rgbGen identity
    }
}

textures/desert/overhead_door
{
	polygonOffset
    {
        map textures/desert/overhead_door
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

textures/desert/sandfloor2old_tread2
{
	polygonOffset
	q3map_material	SolidMetal
	q3map_nolightmap
    {
        map textures/desert/sandfloor2old_tread2
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

textures/desert/metalcrust_anim_map
{
	qer_editorimage	textures/desert/m1
	q3map_nolightmap
    {
        oneshotanimMap 5 textures/desert/m1 textures/desert/m2 textures/desert/m3 textures/desert/m4 textures/desert/m5 textures/desert/m6 textures/desert/m7 textures/desert/m8 
        blendFunc GL_ONE GL_ZERO
    }
}

textures/desert/moltenmetal
{
	qer_editorimage	textures/common/crust2_a
	polygonOffset
	q3map_nolightmap
    {
        map textures/common/flava2
        tcMod scroll 0.01 0
    }
    {
        map textures/common/f1
        blendFunc GL_SRC_ALPHA GL_ONE
        tcMod stretch sin 1 0.02 0 0.1
        tcMod scroll 0.025 0
    }
    {
        map textures/common/crust2_a
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

textures/desert/decal_cracks1
{
	polygonOffset
	q3map_nolightmap
    {
        map textures/desert/decal_cracks1
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

textures/desert/decal_cracks2
{
	polygonOffset
	q3map_nolightmap
    {
        map textures/desert/decal_cracks2
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

textures/desert/decal_rancor
{
	polygonOffset
	q3map_nolightmap
    {
        map textures/desert/decal_rancor
        rgbGen vertex
    }
}

textures/desert/mp_security_station_decal
{
	surfaceparm	nonsolid
	polygonOffset
    {
        map textures/desert/mp_security_station_decal
        alphaFunc GE128
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

textures/desert/mp_1_circle_decal
{
	surfaceparm	nonsolid
	polygonOffset
    {
        map textures/desert/mp_1_circle_decal
        alphaFunc GE128
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

textures/desert/mp_2_circle_decal
{
	qer_editorimage	textures/desert/mp_2_circle_decal
	surfaceparm	nonsolid
	polygonOffset
    {
        map textures/desert/mp_2_circle_decal
        alphaFunc GE128
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

textures/desert/mp_3_circle_decal
{
	qer_editorimage	textures/desert/mp_3_circle_decal
	surfaceparm	nonsolid
	polygonOffset
    {
        map textures/desert/mp_3_circle_decal
        alphaFunc GE128
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

textures/desert/t_rockwall1_b
{
// q3map_shadeangle	120

// q3map_nonplanar

	q3map_shadeangle	120
	qer_editorimage	textures/desert/t_rockwall1
	q3map_nolightmap
	q3map_nonplanar
	q3map_forcemeta
	q3map_onlyvertexlighting
    {
        map textures/desert/t_rockwall1
        rgbGen vertex
    }
    {
        map textures/common/detail9
        blendFunc GL_DST_COLOR GL_ONE
        rgbGen identity
        tcMod scale 10 10
    }
}

textures/desert/outerhull
{
	q3map_material	HollowMetal
    {
        map $lightmap
    }
    {
        map textures/desert/outerhull
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/desert/s_basic_floor
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/desert/s_basic_floor
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/desert/s_basic_1
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/desert/s_basic_1
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/desert/s_basic
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/desert/s_basic
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/desert/s_conbelt
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/desert/s_conbelt
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/desert/s_gratefloor
{
	q3map_material	HollowMetal
    {
        map $lightmap
    }
    {
        map textures/desert/s_gratefloor
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/desert/s_floor_lrg
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/desert/s_floor_lrg
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/desert/s_floor1
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/desert/s_floor1
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/desert/s_floor_elevator
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/desert/s_floor_elevator
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/desert/s_treds
{
	q3map_material	HollowMetal
    {
        map $lightmap
    }
    {
        map textures/desert/s_treds
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/desert/walkway_section1
{
	q3map_material	HollowMetal
    {
        map $lightmap
    }
    {
        map textures/desert/walkway_section1
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/desert/walkway_section2
{
	q3map_material	HollowMetal
    {
        map $lightmap
    }
    {
        map textures/desert/walkway_section2
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

models/map_objects/desert/switch_locked
{
	q3map_nolightmap
    {
        map models/map_objects/desert/switch
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/desert/switch_locked
        blendFunc GL_ONE GL_ONE
        glow
    }
}

models/map_objects/desert/switch_unlocked
{
	q3map_nolightmap
    {
        map models/map_objects/desert/switch
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/desert/switch_unlocked
        blendFunc GL_ONE GL_ONE
        glow
    }
}

models/map_objects/desert/switch_open
{
	q3map_nolightmap
    {
        map models/map_objects/desert/switch
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/desert/switch_open
        blendFunc GL_ONE GL_ONE
        glow
    }
}

models/map_objects/desert/vent_grime
{
	q3map_nolightmap
    {
        map models/map_objects/desert/vent_grime
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

models/map_objects/desert/landing_light_glow
{
	q3map_nolightmap
    {
        map models/map_objects/desert/landing_light_glow
        rgbGen vertex
    }
    {
        map models/map_objects/desert/landing_light_glow
        blendFunc GL_ONE GL_ONE
        glow
    }
}

models/map_objects/desert/wall_light
{
	q3map_nolightmap
    {
        map models/map_objects/desert/wall_light
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/desert/wall_light_glow
        blendFunc GL_ONE GL_ONE
        glow
    }
}

models/map_objects/desert/sandcrawler_02
{
	q3map_nolightmap
    {
        map models/map_objects/desert/sandcrawler_02
        rgbGen vertex
    }
}

models/map_objects/desert/sandcrawler_01
{
	q3map_nolightmap
    {
        map models/map_objects/desert/sandcrawler_01
        rgbGen vertex
    }
}

models/map_objects/desert/barrels_panel
{
    {
        map $lightmap
    }
    {
        map models/map_objects/desert/barrels_panel
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map models/map_objects/desert/barrel_panel_glw
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

models/map_objects/desert/cave_lamp_bulbs
{
    {
        map $lightmap
    }
    {
        map models/map_objects/desert/cave_lamp_bulbs
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map models/map_objects/desert/cave_lamp_bulbs_glw
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
    {
        map models/map_objects/desert/cave_lamp_bulbs_drt
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

models/map_objects/desert/support_light
{
    {
        map $lightmap
    }
    {
        map models/map_objects/desert/support_light
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map models/map_objects/desert/support_light_glw
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
    {
        map models/map_objects/desert/support_light_glw2
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave noise 0 1 0.6 0.7
    }
    {
        map models/map_objects/desert/support_light_glw3
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave triangle 0 1 0 1
    }
}

models/map_objects/desert/switch3
{
    {
        map $lightmap
    }
    {
        map models/map_objects/desert/switch3
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        oneshotanimMap 1 models/map_objects/desert/switch3_locked models/map_objects/desert/switch3_unlocked 
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

models/map_objects/desert/handle3
{
    {
        map $lightmap
    }
    {
        map models/map_objects/desert/handle3
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map models/map_objects/desert/handle3_glw
        blendFunc GL_ONE GL_ONE
        rgbGen wave triangle 0 1 0 1
    }
}

models/map_objects/desert/landing_light_glow_b
{
	qer_editorimage	models/map_objects/desert/landing_light_glow
	q3map_nolightmap
    {
        map models/map_objects/desert/landing_light_glow
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/desert/landing_light_glow
        blendFunc GL_ONE GL_ONE
        glow
    }
}

models/map_objects/desert/dome
{
	qer_editorimage	textures/desert/stucco_grime_top_bottom.tga
	q3map_nolightmap
    {
        map textures/desert/stucco_grime_top_bottom
        rgbGen lightingDiffuse
    }
}

models/map_objects/desert/view_panel_screen_desert
{
	q3map_nolightmap
    {
        map models/map_objects/desert/view_panel_screen_desert
    }
    {
        map models/map_objects/desert/view_panel_screen_desert_glow1
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0 1 0 1
    }
    {
        map models/map_objects/desert/view_panel_screen_desert_glow2
        blendFunc GL_ONE GL_ONE
        rgbGen wave square 0 1 0 5
    }
    {
        map models/map_objects/desert/view_panel_screen_desert_glow
        blendFunc GL_ONE GL_ONE
        rgbGen wave inversesawtooth 0 1 0 1
    }
    {
        map models/map_objects/desert/view_panel_screen_desert_glow3
        blendFunc GL_ONE GL_ONE
        tcMod scroll 0 0.35
    }
}

models/map_objects/desert/view_panel_effect_desert
{
    {
        map models/map_objects/desert/view_panel_effect_glow1
        rgbGen wave sawtooth 0 1 0 1
        tcMod scroll 4 0
    }
    {
        map models/map_objects/desert/view_panel_effect_glow2
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave inversesawtooth 0 1 0 1
        tcMod scroll 0 0.5
    }
}

models/map_objects/desert/view_panel_frame_desert
{
    {
        map models/map_objects/desert/view_panel_frame_desert
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/desert/view_panel_frame_desert_glow1
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0 1 0 1
    }
    {
        map models/map_objects/desert/view_panel_frame_desert_glow2
        blendFunc GL_ONE GL_ONE
        rgbGen wave inversesawtooth 0 1 0 0.75
    }
    {
        map models/map_objects/desert/view_panel_frame_desert_glow3
        blendFunc GL_ONE GL_ONE
        rgbGen wave sawtooth 0 1 0 1
    }
}

textures/kejim/kej_control3
{
    {
        map $lightmap
    }
    {
        map textures/kejim/kej_control3
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/kejim/kej_control3_glow
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

textures/kejim/kej_control
{
    {
        map $lightmap
    }
    {
        map textures/kejim/kej_control
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/kejim/kej_control_glow
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

textures/kejim/kej_control6
{
    {
        map $lightmap
    }
    {
        map textures/kejim/kej_control6
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/kejim/kej_control6_glow
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

textures/kejim/kej_control5
{
    {
        map $lightmap
    }
    {
        map textures/kejim/kej_control5
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/kejim/kej_control5_glow
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

textures/kejim/kej_control2
{
    {
        map $lightmap
    }
    {
        map textures/kejim/kej_control2
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/kejim/kej_control2_glow
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

models/map_objects/ships/falcon_disk
{
	q3map_nolightmap
	cull	twosided
    {
        map models/map_objects/ships/falcon_disk
        rgbGen lightingDiffuse
    }
}

