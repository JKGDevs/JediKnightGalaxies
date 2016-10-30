textures/hoth/mp_rock2
{
	q3map_shadeangle	120
	qer_editorimage	textures/hoth/mp_rock2
	q3map_material	Ice
	q3map_nonplanar
	q3map_splotchfix
	q3map_novertexshadows
    {
        map textures/hoth/mp_rock2
        rgbGen identity
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/hoth/ice_phong
{
	q3map_shadeangle	120
	qer_editorimage	textures/hoth/h_basicwall.tga
	q3map_material	Ice
	q3map_nonplanar
	q3map_splotchfix
	q3map_novertexshadows
    {
        map textures/hoth/h_basicwall
        rgbGen identity
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/hoth/cut_ice_phong
{
	q3map_shadeangle	120
	qer_editorimage	textures/hoth/h_basicwall_2.tga
	q3map_material	Ice
	q3map_nonplanar
	q3map_novertexshadows
    {
        map textures/hoth/h_basicwall_2
        rgbGen identity
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/hoth/chiseled_phong
{
	q3map_shadeangle	120
	qer_editorimage	textures/hoth/h_wallchunk
	q3map_material	Snow
	q3map_nonplanar
	q3map_novertexshadows
    {
        map textures/hoth/h_wallchunk
        rgbGen identity
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/hoth/ice_rock_phong
{
	q3map_shadeangle	120
	qer_editorimage	textures/hoth/rock_huge_snow.tga
	q3map_material	Ice
	q3map_nonplanar
	q3map_novertexshadows
    {
        map textures/hoth/rock_huge_snow
        rgbGen identity
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/hoth/grey_snow_phong
{
	q3map_shadeangle	120
	qer_editorimage	textures/hoth/h_floor.tga
	q3map_material	Snow
	q3map_nonplanar
	q3map_splotchfix
	q3map_novertexshadows
    {
        map textures/hoth/h_floor
        rgbGen identity
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/hoth/rock_huge_snow
{
	q3map_material	Rock
	q3map_nolightmap
    {
        map textures/hoth/rock_huge_snow
        rgbGen exactVertex
        tcMod scale 0.5 0.5
    }
}

textures/hoth/rock_huge_snow_model
{
// q3map_forcesunlight

	q3map_material	Rock
	q3map_nolightmap
	q3map_nonplanar
    {
        map textures/hoth/rock_huge_snow
        rgbGen exactVertex
        tcMod scale 0.5 0.5
    }
}

textures/hoth/ice_model
{
	q3map_material	Ice
	q3map_nolightmap
	q3map_nonplanar
    {
        map textures/hoth/ice
        rgbGen exactVertex
    }
}

textures/hoth/instance_basicltgray
{
	qer_editorimage	textures/cairn/basicltgray
	q3map_material	SolidMetal
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map textures/cairn/basicltgray
        rgbGen lightingDiffuse
    }
}

textures/hoth/instance_basicltgray_dam
{
	qer_editorimage	textures/cairn/basicltgray_dam
	q3map_material	SolidMetal
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map textures/cairn/basicltgray_dam
        rgbGen lightingDiffuse
    }
}

textures/hoth/snow_01
{
	q3map_shadeangle	50
	qer_editorimage	textures/hoth/snow_01
	q3map_material	Snow
	q3map_nolightmap
	q3map_nonplanar
	q3map_forcemeta
	q3map_onlyvertexlighting
    {
        map textures/hoth/snow_01
        rgbGen vertex
    }
}

textures/hoth/snow_02
{
	q3map_material	Snow
	q3map_nolightmap
    {
        map textures/hoth/snow_02
        rgbGen exactVertex
        tcMod scale 0.5 0.5
    }
}

textures/hoth/ice
{
// q3map_forcemeta

	q3map_shadeangle	225
	qer_editorimage	textures/hoth/ice
	q3map_material	Ice
	q3map_nolightmap
	q3map_nonplanar
	q3map_onlyvertexlighting
    {
        map textures/hoth/ice
        rgbGen vertex
    }
}



textures/hoth/h_outground_phong
{
// q3map_forcemeta

	q3map_shadeangle	120
	qer_editorimage	textures/hoth/h_outground
	q3map_material	Snow
	q3map_nolightmap
	q3map_nonplanar
	q3map_onlyvertexlighting
    {
        map textures/hoth/h_outground
        rgbGen vertex
    }
}

textures/hoth/h_outground_2_phong
{
// q3map_forcemeta

	q3map_shadeangle	120
	qer_editorimage	textures/hoth/h_outground_2
	q3map_material	Snow
	q3map_nolightmap
	q3map_nonplanar
	q3map_onlyvertexlighting
    {
        map textures/hoth/h_outground_2
        rgbGen vertex
    }
}

textures/hoth/h_outground_3_phong
{
// q3map_forcemeta

	q3map_shadeangle	120
	qer_editorimage	textures/hoth/h_outground_3
	q3map_material	Rock
	q3map_nolightmap
	q3map_nonplanar
	q3map_onlyvertexlighting
    {
        map textures/hoth/h_outground_3
        rgbGen vertex
    }
}

// TERRAIN BLEND SHADERS

textures/hoth/terrain_0
{
// q3map_lightmapaxis z

	q3map_shadeangle	150
	q3map_texturesize 1024 1024
	q3map_material	Snow
	q3map_nolightmap
	q3map_nonplanar
	q3map_forcemeta
	q3map_onlyvertexlighting
	q3map_tcGen	ivector ( 1024 0 0 ) ( 0 1024 0 )
    {
        map textures/hoth/h_outground
        rgbGen vertex
    }
}

textures/hoth/terrain_1
{
// q3map_lightmapaxis z

	q3map_shadeangle	150
	q3map_texturesize 1024 1024
	q3map_material	Snow
	q3map_nolightmap
	q3map_nonplanar
	q3map_forcemeta
	q3map_onlyvertexlighting
	q3map_tcGen	ivector ( 1024 0 0 ) ( 0 1024 0 )
    {
        map textures/hoth/h_outground_2
        rgbGen vertex
    }
}

textures/hoth/terrain_0to1
{
// q3map_lightmapaxis z

	q3map_shadeangle	150
	q3map_texturesize 1024 1024
	q3map_material	Snow
	q3map_nolightmap
	q3map_nonplanar
	q3map_forcemeta
	q3map_onlyvertexlighting
	q3map_tcGen	ivector ( 1024 0 0 ) ( 0 1024 0 )
    {
        map textures/hoth/h_outground
        alphaGen vertex
    }
    {
        map textures/hoth/h_outground_2
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
        alphaGen vertex
    }
}

textures/hoth/terrain_vertex
{
	q3map_material	Snow
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map textures/hoth/h_outground
        rgbGen vertex
    }
}



textures/hoth/basicltgray_shiny
{
	qer_editorimage	textures/cairn/basicltgray
    {
        map textures/imperial/etest3
        tcGen environment
    }
    {
        map textures/cairn/basicltgray
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

// MOUNTAIN---CAVE MODEL SHADERS

textures/hoth/h_outground_2_clip
{
// q3map_clipmodel

	q3map_shadeangle	120
	qer_editorimage	textures/hoth/h_outground_2
	q3map_material	Snow
	q3map_nolightmap
	q3map_nonplanar
	q3map_forcemeta
	q3map_onlyvertexlighting
    {
        map textures/hoth/h_outground_2
        rgbGen vertex
    }
}

textures/hoth/h_outground_3_clip
{
// q3map_clipmodel

	q3map_shadeangle	50
	qer_editorimage	textures/hoth/h_outground_3
	q3map_material	Rock
	q3map_nolightmap
	q3map_nonplanar
	q3map_forcemeta
	q3map_onlyvertexlighting
    {
        map textures/hoth/h_outground_3
        rgbGen vertex
    }
}

textures/hoth/snow_01_clip
{
// q3map_clipmodel

	q3map_shadeangle	50
	qer_editorimage	textures/hoth/snow_01
	q3map_material	Snow
	q3map_nolightmap
	q3map_nonplanar
	q3map_forcemeta
	q3map_onlyvertexlighting
    {
        map textures/hoth/snow_01
        rgbGen vertex
    }
}

textures/hoth/instance_outerhull
{
	qer_editorimage	textures/cairn/outerhull
	q3map_material	HollowMetal
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map textures/cairn/outerhull
        rgbGen lightingDiffuse
    }
}

textures/hoth/instance_pipe2
{
	qer_editorimage	textures/cairn/pipe2
	q3map_material	HollowMetal
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map textures/cairn/pipe2
        rgbGen lightingDiffuse
    }
}

textures/hoth/instance_stack
{
	qer_editorimage	textures/cairn/stack
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map textures/cairn/stack
        rgbGen lightingDiffuse
    }
}

textures/hoth/h_wall_04b
{
	qer_editorimage	textures/hoth/h_wall_04b
	q3map_material	SolidMetal
    {
        map textures/imperial/etest3
        tcGen environment
    }
    {
        map textures/hoth/h_wall_04b
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/hoth/icicles
{
	qer_editorimage	textures/hoth/h_icicles.tga
	q3map_material	Ice
	cull	twosided
    {
        map textures/hoth/h_icicles
        alphaFunc GE128
        blendFunc GL_SRC_ALPHA GL_ZERO
        depthWrite
    }
    {
        map $lightmap
        blendFunc GL_ONE GL_ZERO
        depthFunc equal
    }
    {
        map textures/hoth/h_icicles
        blendFunc GL_DST_COLOR GL_ZERO
        depthFunc equal
    }
}

textures/hoth/at_at_leg_b
{
	q3map_nolightmap
    {
        map textures/hoth/at_at_leg
        rgbGen identity
        tcMod scale 4 4
    }
    {
        map textures/hoth/at_at_frost
        blendFunc GL_ONE GL_ONE_MINUS_SRC_COLOR
        tcMod scale 2.2 2.2
    }
}

textures/hoth/h_door2
{
	qer_editorimage	textures/hoth/h_door2
    {
        map textures/imperial/etest3
        tcGen environment
    }
    {
        map textures/hoth/h_door2
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/hoth/h_door
{
	qer_editorimage	textures/hoth/h_door
    {
        map textures/imperial/etest3
        tcGen environment
    }
    {
        map textures/hoth/h_door
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/hoth/h_floor_03
{
	qer_editorimage	textures/hoth/h_floor_03
	q3map_material	SolidMetal
    {
        map textures/common/environ1
        tcGen environment
    }
    {
        map textures/hoth/h_floor_03
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/hoth/snow_nolightmap
{
	q3map_shadeangle	225
	qer_editorimage	textures/hoth/lsnow1a
	q3map_material	Snow
	q3map_nolightmap
	q3map_nonplanar
	q3map_onlyvertexlighting
    {
        map textures/hoth/lsnow1a
        rgbGen vertex
    }
}

textures/hoth/h_icicles
{
	q3map_material	Ice
	q3map_nolightmap
    {
        map textures/hoth/h_icicles
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

textures/hoth/ion_panel_01
{
    {
        map $lightmap
    }
    {
        map textures/hoth/ion_panel_01
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/hoth/ion_panel_01_glw
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

textures/hoth/ion_panel_02
{
    {
        map $lightmap
    }
    {
        map textures/hoth/ion_panel_02
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/hoth/ion_panel_02_glw
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

textures/hoth/ion_panel_03
{
    {
        map $lightmap
    }
    {
        map textures/hoth/ion_panel_03
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/hoth/ion_panel_03_glw
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

textures/hoth/h_control_01
{
    {
        map $lightmap
    }
    {
        map textures/hoth/h_control_01
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/hoth/h_control_01_glw
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

textures/hoth/h_control_02
{
    {
        map $lightmap
    }
    {
        map textures/hoth/h_control_02
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/hoth/h_control_02_glw
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

textures/hoth/h_control_03
{
    {
        map $lightmap
    }
    {
        map textures/hoth/h_control_03
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/hoth/h_control_03_glw
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

textures/hoth/h_control_04
{
    {
        map $lightmap
    }
    {
        map textures/hoth/h_control_04
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/hoth/h_control_04_glw
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

textures/hoth/h_door_onoff
{
    {
        map textures/common/environ1
        tcGen environment
    }
    {
        map textures/hoth/h_door_onoff
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        oneshotanimMap 1 textures/hoth/h_door_onoffr textures/hoth/h_door_onoffg 
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
        alphaGen identity
    }
}

textures/hoth/h_door2_onoff
{
    {
        map textures/common/environ1
        tcGen environment
    }
    {
        map textures/hoth/h_door2_onoff
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        oneshotanimMap 1 textures/hoth/h_door2_onoffr textures/hoth/h_door2_onoffg 
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
        alphaGen identity
    }
}

textures/hoth/door_04_a_onoff
{
	q3map_material	HollowMetal
    {
        map $lightmap
    }
    {
        map textures/hoth/door_04_a_onoff
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        oneshotanimMap 1 textures/hoth/door_04_a_onoffr textures/hoth/door_04_a_onoffg 
        blendFunc GL_ONE GL_ONE
        glow
        alphaGen identity
    }
}

textures/hoth/door_02_onoff
{
	q3map_material	HollowMetal
    {
        map textures/common/environ1
        tcGen environment
    }
    {
        map textures/hoth/door_02_onoff
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        oneshotanimMap 1 textures/hoth/door_02_onoffr textures/hoth/door_02_onoffg 
        blendFunc GL_ONE GL_ONE
        glow
        alphaGen identity
    }
}

textures/hoth/newwall
{
	qer_editorimage	textures/hoth/newwall_mask
    {
        map textures/common/environ1
        tcGen environment
    }
    {
        map textures/hoth/newwall_mask
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/hoth/fuelgradient
{
    {
        map $lightmap
    }
    {
        map textures/hoth/fuelgradient
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/hoth/snow_01_b
{
	q3map_shadeangle	50
	qer_editorimage	textures/hoth/snow_01
	q3map_material	Snow
	q3map_nolightmap
	q3map_nonplanar
	q3map_forcemeta
	q3map_onlyvertexlighting
    {
        map textures/hoth/snow_01
        rgbGen const ( 0.435294 0.435294 0.435294 )
    }
}

textures/hoth/h_outground_3_b
{
	q3map_shadeangle	50
	qer_editorimage	textures/hoth/snow_01
	q3map_material	Rock
	q3map_nolightmap
	q3map_nonplanar
	q3map_forcemeta
	q3map_onlyvertexlighting
    {
        map textures/hoth/h_outground_3
        rgbGen lightingDiffuse
    }
}

textures/hoth/h_outground_2_b
{
	q3map_shadeangle	50
	qer_editorimage	textures/hoth/snow_01
	q3map_material	Snow
	q3map_nolightmap
	q3map_nonplanar
	q3map_forcemeta
	q3map_onlyvertexlighting
    {
        map textures/hoth/h_outground_2
        rgbGen lightingDiffuse
    }
}

textures/hoth/walker_b
{
	qer_editorimage	textures/hoth/at_at_leg
	q3map_nolightmap
    {
        map textures/hoth/walker
        rgbGen identity
        tcMod scale 4 4
    }
    {
        map textures/hoth/at_at_frost
        blendFunc GL_ONE GL_ONE_MINUS_SRC_COLOR
        tcMod scale 2.2 2.2
    }
}

textures/hoth/door_02
{
	q3map_material	HollowMetal
    {
        map textures/common/environ1
        tcGen environment
    }
    {
        map textures/hoth/door_02
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/hoth/trim_01
{
    {
        map textures/common/environ1
        tcGen environment
    }
    {
        map textures/hoth/trim_01
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/hoth/h_floor_02
{
	q3map_material	HollowMetal
    {
        map textures/common/environ1
        tcGen environment
    }
    {
        map textures/hoth/h_floor_02
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/hoth/h_wall_03
{
	q3map_material	SolidMetal
    {
        map textures/common/environ1
        tcGen environment
    }
    {
        map textures/hoth/h_wall_03
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/hoth/h_wall_01
{
	q3map_material	SolidMetal
    {
        map textures/common/environ1
        tcGen environment
    }
    {
        map textures/hoth/h_wall_01
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/hoth/h_wall_02
{
	q3map_material	SolidMetal
    {
        map textures/common/environ1
        tcGen environment
    }
    {
        map textures/hoth/h_wall_02
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/hoth/floor_grate_new
{
	q3map_material	HollowMetal
	q3map_nolightmap
	cull	twosided
    {
        map textures/hoth/floor_grate_new
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map textures/hoth/floor_grate_new
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/hoth/ion_feedtube
{
	q3map_nolightmap
    {
        map gfx/misc/bolt1_rotated
        rgbGen identity
        tcMod scroll 0 1
    }
    {
        map textures/hoth/ion_feedtube
        blendFunc GL_ONE_MINUS_SRC_ALPHA GL_SRC_ALPHA
        rgbGen vertex
        tcMod scale 1 0.6
    }
    {
        map textures/hoth/ion_feedtube_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
        tcMod scale 1 0.6
    }
}

textures/hoth/ion_feedtube_copper
{
	q3map_material	HollowMetal
    {
        map $lightmap
    }
    {
        map textures/hoth/ion_feedtube_copper
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/hoth/ion_feedtube_copper_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

textures/hoth/h_wall_04b_shelf
{
	qer_editorimage	textures/hoth/h_wall_04b_shelf
	q3map_material	SolidMetal
    {
        map textures/imperial/etest3
        tcGen environment
    }
    {
        map textures/hoth/h_wall_04b_shelf
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/hoth/light_01b
{
    {
        map $lightmap
    }
    {
        map textures/hoth/light_01b
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/hoth/light_01b_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/hoth/lights_tube
{
    {
        map $lightmap
    }
    {
        map textures/hoth/lights_tube
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/hoth/lights_tube_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/hoth/lights_tube_off
{
	qer_editorimage	textures/hoth/lights_tube
    {
        map $lightmap
    }
    {
        map textures/hoth/lights_tube
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/hoth/exit_beam
{
	qer_editorimage	textures/hoth/exit_beam
	q3map_material	SolidMetal
    {
        map textures/imperial/etest3
        tcGen environment
    }
    {
        map textures/hoth/exit_beam
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/hoth/exit_beam_side
{
	qer_editorimage	textures/hoth/exit_beam_side
	q3map_material	SolidMetal
    {
        map textures/imperial/etest3
        tcGen environment
    }
    {
        map textures/hoth/exit_beam_side
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/hoth/exit_center_piece
{
	qer_editorimage	textures/hoth/exit_center_piece
    {
        map textures/imperial/etest3
        tcGen environment
    }
    {
        map textures/hoth/exit_center_piece
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/hoth/exit_plate
{
	qer_editorimage	textures/hoth/exit_plate
	q3map_material	SolidMetal
    {
        map textures/imperial/etest3
        tcGen environment
    }
    {
        map textures/hoth/exit_plate
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/hoth/exit_trim
{
	qer_editorimage	textures/hoth/exit_trim
	q3map_material	SolidMetal
    {
        map textures/imperial/etest3
        tcGen environment
    }
    {
        map textures/hoth/exit_trim
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/hoth/light_ceiling
{
    {
        map $lightmap
    }
    {
        map textures/hoth/light_ceiling
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/hoth/light_ceiling_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/hoth/blast_panel
{
	qer_editorimage	textures/hoth/blast_panel
	q3map_material	SolidMetal
    {
        map textures/imperial/etest3
        tcGen environment
    }
    {
        map textures/hoth/blast_panel
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/hoth/blast_panel_snow
{
	qer_editorimage	textures/hoth/blast_panel_snow
	q3map_material	SolidMetal
    {
        map textures/imperial/etest3
        tcGen environment
    }
    {
        map textures/hoth/blast_panel_snow
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/hoth/blast_panel2
{
	qer_editorimage	textures/hoth/blast_panel2
	q3map_material	SolidMetal
    {
        map textures/imperial/etest3
        tcGen environment
    }
    {
        map textures/hoth/blast_panel2
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/hoth/blast_panel2_snow
{
	qer_editorimage	textures/hoth/blast_panel2_snow
	q3map_material	SolidMetal
    {
        map textures/imperial/etest3
        tcGen environment
    }
    {
        map textures/hoth/blast_panel2_snow
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/hoth/blast_panel3
{
	qer_editorimage	textures/hoth/blast_panel3
	q3map_material	SolidMetal
    {
        map textures/imperial/etest3
        tcGen environment
    }
    {
        map textures/hoth/blast_panel3
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/hoth/blast_panel3_snow
{
	qer_editorimage	textures/hoth/blast_panel3_snow
	q3map_material	SolidMetal
    {
        map textures/imperial/etest3
        tcGen environment
    }
    {
        map textures/hoth/blast_panel3_snow
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/hoth/blast_panel4
{
	qer_editorimage	textures/hoth/blast_panel4
	q3map_material	SolidMetal
    {
        map textures/imperial/etest3
        tcGen environment
    }
    {
        map textures/hoth/blast_panel4
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/hoth/blast_panel4_snow
{
	qer_editorimage	textures/hoth/blast_panel4_snow
	q3map_material	SolidMetal
    {
        map textures/imperial/etest3
        tcGen environment
    }
    {
        map textures/hoth/blast_panel4_snow
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/hoth/ion_screen_01
{
	q3map_nolightmap
    {
        map textures/hoth/ion_screen_01
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
    {
        clampmap textures/hoth/ion_screen_01_ring
        blendFunc GL_ONE GL_ONE
        rgbGen identity
        tcMod rotate 8
    }
    {
        map textures/hoth/ion_screen_01_glw
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
    {
        map textures/hoth/ion_screen_01_glw2
        blendFunc GL_ONE GL_ONE
        detail
        rgbGen wave sin 0.5 0.6 0 1.25
    }
    {
        map textures/hoth/ion_screen_01_glw3
        blendFunc GL_ONE GL_ONE
        detail
        rgbGen wave square 0 1 0 1
    }
}

textures/hoth/ion_screen_03
{
	q3map_nolightmap
    {
        map textures/hoth/ion_screen_03
        rgbGen vertex
    }
    {
        map textures/hoth/ion_screen_03_wipe
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0 1 0 0.5
        tcMod scroll 0 -0.5
    }
    {
        map textures/hoth/ion_screen_03_glw
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
    {
        map textures/hoth/ion_screen_03
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

textures/hoth/ion_screen_02
{
	q3map_nolightmap
    {
        map textures/hoth/ion_screen_02
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
    {
        map textures/hoth/ion_screen_02_glw
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
    {
        map textures/hoth/ion_screen_02_pls
        blendFunc GL_ONE GL_ONE
        rgbGen wave square 0.5 0.5 0 1
    }
    {
        map textures/hoth/ion_screen_02_ret
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0 0.4 0 0.5
    }
    {
        map textures/hoth/ion_screen_02_pls2
        blendFunc GL_ONE GL_ONE
        detail
        rgbGen wave sin 0.5 0.6 0 1.25
    }
}

textures/hoth/switch_lift_green
{
    {
        map $lightmap
    }
    {
        map textures/hoth/switch_lift
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/hoth/switch_lift_green
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/hoth/switch_lift_red
{
	qer_editorimage	textures/hoth/switch_lift
    {
        map $lightmap
    }
    {
        map textures/hoth/switch_lift
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/hoth/switch_lift_red
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/hoth/copper_pipe_nospec
{
	qer_editorimage	textures/hoth/ion_feedtube_copper
	q3map_material	HollowMetal
    {
        map $lightmap
    }
    {
        map textures/hoth/ion_feedtube_copper
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/hoth/floorpanel_02
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/hoth/floorpanel_02
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/hoth/floor_grate
{
	q3map_material	HollowMetal
    {
        map $lightmap
    }
    {
        map textures/hoth/floor_grate
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/hoth/floorpanel_01
{
	q3map_material	Snow
    {
        map $lightmap
    }
    {
        map textures/hoth/floorpanel_01
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/hoth/grid_02a
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/hoth/grid_02a
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/hoth/grid_02
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/hoth/grid_02
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/hoth/grid_01
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/hoth/grid_01
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/hoth/h_basicwall
{
	q3map_material	Snow
    {
        map $lightmap
    }
    {
        map textures/hoth/h_basicwall
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/hoth/h_floor
{
	q3map_material	Snow
    {
        map $lightmap
    }
    {
        map textures/hoth/h_floor
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/hoth/h_floor_01
{
    {
        map $lightmap
    }
    {
        map textures/hoth/h_floor_01
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/hoth/ice2_specular
{
	q3map_material	Ice
    {
        map $lightmap
    }
    {
        map textures/hoth/ice2_specular
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/hoth/ice2
{
	q3map_material	Ice
	q3map_nolightmap
    {
        map textures/hoth/ice2
        rgbGen lightingDiffuse
    }
}

textures/hoth/lsnow1a
{
	q3map_material	Snow
    {
        map $lightmap
    }
    {
        map textures/hoth/lsnow1a
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/hoth/lsnow1
{
	q3map_material	Snow
    {
        map $lightmap
    }
    {
        map textures/hoth/lsnow1
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/hoth/lsnow
{
	q3map_material	Snow
    {
        map $lightmap
    }
    {
        map textures/hoth/lsnow
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/hoth/snow_03
{
	q3map_shadeangle	50
	qer_editorimage	textures/hoth/snow_01
	q3map_material	Snow
	q3map_nolightmap
	q3map_nonplanar
	q3map_forcemeta
	q3map_onlyvertexlighting
    {
        map textures/hoth/snow_03
        rgbGen vertex
    }
}

textures/hoth/snowbiga
{
	q3map_shadeangle	50
	qer_editorimage	textures/hoth/snow_01
	q3map_material	Snow
	q3map_nolightmap
	q3map_nonplanar
	q3map_forcemeta
	q3map_onlyvertexlighting
    {
        map textures/hoth/snowbiga
        rgbGen vertex
    }
}

textures/factory/outside_floor1_phong
{
// q3map_forcemeta

	q3map_shadeangle	120
	qer_editorimage	textures/factory/outside_floor1
	q3map_material	Rock
	q3map_nolightmap
	q3map_nonplanar
	q3map_onlyvertexlighting
    {
        map textures/factory/outside_floor1
        rgbGen vertex
    }
}

models/map_objects/hoth/generator_shell
{
	q3map_nolightmap
    {
        map models/map_objects/hoth/generator_shell
        rgbGen vertex
        tcMod scale 10 10
    }
    {
        map models/map_objects/hoth/generator_shell_lmap
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

models/map_objects/hoth/generator_body
{
	q3map_nolightmap
    {
        map models/map_objects/hoth/generator_body
        rgbGen vertex
        tcMod scale 20 20
    }
    {
        map models/map_objects/hoth/generator_body_lmap
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

models/map_objects/hoth/hothhall_clip
{
// q3map_clipmodel

	qer_editorimage	models/map_objects/hoth/hothhall
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/hoth/hothhall
        rgbGen lightingDiffuse
    }
}

models/map_objects/hoth/hall_3ds
{
// q3map_clipmodel

	qer_editorimage	models/map_objects/hoth/hothhall
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/hoth/hothhall
        rgbGen lightingDiffuse
    }
}

models/map_objects/hoth/hall
{
// q3map_clipmodel

	qer_editorimage	models/map_objects/hoth/hothhall
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/hoth/hothhall
        rgbGen lightingDiffuse
    }
}

models/map_objects/hoth/bacta_tank_shell
{
	q3map_nolightmap
    {
        map models/map_objects/hoth/bacta_tank_shell
        rgbGen lightingDiffuse
    }
}

models/map_objects/hoth/bacta_tank_light
{
	q3map_nolightmap
    {
        map models/map_objects/hoth/bacta_tank_light
        rgbGen vertex
    }
    {
        map models/map_objects/hoth/bacta_tank_light_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

models/map_objects/hoth/bacta_tank_liquid
{
	surfaceparm	nomarks
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
	cull	twosided
    {
        map models/map_objects/hoth/bacta_tank_liquid
        blendFunc GL_ONE GL_SRC_ALPHA
        tcMod stretch sin 1 0.2 0 0.03
        tcMod scroll 0 0.05
    }
    {
        map models/map_objects/hoth/bac_small
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        tcMod scroll 0 0.45
    }
    {
        map models/map_objects/hoth/bac_medium
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        tcMod turb 1 0.05 0 0.08
        tcMod scroll 0 0.2
    }
    {
        map models/map_objects/hoth/bac_large
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        tcMod turb 1 0.05 0 0.1
        tcMod scroll 0 0.1
    }
}

models/map_objects/hoth/bacta_tank_glass
{
    {
        map models/map_objects/hoth/bacta_tank_glass
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
    {
        map textures/common/env_chrome
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        detail
        alphaGen const 0.5
        tcGen environment
    }
}

models/map_objects/hoth/light_wall_bulb
{
    {
        map $lightmap
    }
    {
        map models/map_objects/hoth/light_wall_bulb
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map models/map_objects/hoth/light_wall_bulb_glow
        blendFunc GL_ONE GL_ONE
        glow
    }
}

models/map_objects/hoth/light_floor_bulbs
{
	qer_editorimage	models/map_objects/hoth/light_floor_bulbs
    {
        map $lightmap
    }
    {
        map models/map_objects/hoth/light_floor_bulbs
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map models/map_objects/hoth/light_floor_bulbs_glow
        blendFunc GL_ONE GL_ONE
        glow
    }
}

models/map_objects/hoth/ioncannon
{
	q3map_nolightmap
    {
        map models/map_objects/hoth/ioncan
        rgbGen vertex
    }
}

models/map_objects/hoth/generator_grate_b
{
	q3map_nolightmap
    {
        map models/map_objects/hoth/generator_grate
        rgbGen lightingDiffuse
        tcMod scale 10 10
    }
}

models/map_objects/hoth/control_panel_broken_glass
{
	q3map_nolightmap
	cull	twosided
    {
        map models/map_objects/hoth/control_panel_broken_glass
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

models/map_objects/hoth/ion_nut
{
    {
        map models/map_objects/hoth/ion_nut
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/hoth/ion_nut
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map gfx/effects/stunpass_rotated
        blendFunc GL_ONE GL_ONE
        detail
        rgbGen wave sin 0.6 0.5 0 2
        tcMod scale 0.25 1
        tcMod scroll 0.25 0.5
    }
    {
        map gfx/effects/stunpass_rotated2
        blendFunc GL_ONE GL_ONE
        detail
        tcMod scale 1.5 6
        tcMod scroll 0.1 1
    }
}

models/map_objects/hoth/ion_bigfin
{
    {
        map models/map_objects/hoth/ion_bigfin
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/hoth/ion_bigfin_glow
        blendFunc GL_ONE GL_ONE
        glow
    }
    {
        map models/map_objects/hoth/ion_bigfin_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/map_objects/hoth/console_desktop_on
{
    {
        map $lightmap
    }
    {
        map models/map_objects/hoth/console_desktop
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map models/map_objects/hoth/console_desktop_on
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

models/map_objects/hoth/console_monitor_on
{
    {
        map $lightmap
    }
    {
        map models/map_objects/hoth/console_monitor_on
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map models/map_objects/hoth/console_monitor_on
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 1 0.5 5 11
    }
}

models/map_objects/hoth/light_wallbeam
{
    {
        map models/map_objects/hoth/light_wallbeam
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/hoth/light_wallbeam_glw
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

models/map_objects/hoth/heater_coil
{
	q3map_nolightmap
    {
        map models/map_objects/hoth/heater_coil
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/hoth/heater_coil_glow
        blendFunc GL_ONE GL_ONE
        rgbGen wave sawtooth 0 0.5 0 30
    }
    {
        map models/map_objects/hoth/heater_coil_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sawtooth 0 0.5 0.5 30
    }
}

models/map_objects/hoth/light_wallbeam_lens
{
	q3map_nolightmap
    {
        map models/map_objects/hoth/light_wallbeam_lens
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/hoth/light_wallbeam_lens_glw
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sawtooth 0 0.3 0 40
    }
    {
        map models/map_objects/hoth/light_wallbeam_lens_glw
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sawtooth 0 0.3 0.5 40
    }
}

models/map_objects/hoth/hothhall
{
// q3map_clipmodel

	qer_editorimage	models/map_objects/hoth/hothhall
	surfaceparm	nodlight
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/hoth/hothhall
        rgbGen lightingDiffuse
    }
}

models/map_objects/hoth/bed_lamp_glass
{
    {
        map $lightmap
    }
    {
        map models/map_objects/hoth/bed_lamp_glass
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map models/map_objects/hoth/bed_lamp_glass_glw
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

