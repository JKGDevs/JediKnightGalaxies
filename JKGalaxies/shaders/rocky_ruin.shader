textures/rocky_ruins/rock_phong
{
	q3map_shadeangle	120
	qer_editorimage	textures/factory/outside_rock1b.tga
	q3map_nonplanar
	q3map_splotchfix
	q3map_forcemeta
	q3map_onlyvertexlighting
    {
// tcMod scale 0.25 0.25

        map textures/factory/outside_rock1b
        rgbGen vertex
    }
}

textures/rocky_ruins/grate2
{
	q3map_material	HollowMetal
	q3map_alphashadow
	cull	twosided
    {
        map textures/rocky_ruins/grate2
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
        map textures/rocky_ruins/grate2
        blendFunc GL_DST_COLOR GL_ZERO
        depthFunc equal
        glow
    }
}

textures/rocky_ruins/grate_light
{
	q3map_material	HollowMetal
	q3map_nolightmap
    {
        map textures/rocky_ruins/grate_light
        rgbGen vertex
    }
    {
        map textures/rocky_ruins/grate_light_glw
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/rocky_ruins/r_floor2b
{
    {
        map $lightmap
    }
    {
        map textures/rocky_ruins/r_floor2b
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/common/env_chrome
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        detail
        alphaGen const 0.25
        tcGen environment
    }
    {
        map textures/rocky_ruins/r_floor2b
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        detail
        rgbGen identity
    }
}

textures/rocky_ruins/arrow1
{
	polygonOffset
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        clampmap textures/rocky_ruins/arrow1
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

textures/rocky_ruins/arrow4_b
{
	polygonOffset
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map textures/rocky_ruins/arrow4_b
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        depthWrite
    }
}

textures/rocky_ruins/arrow1_b
{
	polygonOffset
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map textures/rocky_ruins/arrow1_b
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        depthWrite
    }
}

textures/rocky_ruins/arrow2
{
	polygonOffset
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map textures/rocky_ruins/arrow2
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        depthWrite
    }
}

textures/rocky_ruins/arrow2_b
{
	polygonOffset
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map textures/rocky_ruins/arrow2_b
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        depthWrite
    }
}

textures/rocky_ruins/arrow3
{
	polygonOffset
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map textures/rocky_ruins/arrow3
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        depthWrite
    }
}

textures/rocky_ruins/arrow3_b
{
	polygonOffset
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map textures/rocky_ruins/arrow3_b
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        depthWrite
    }
}

textures/rocky_ruins/arrow4
{
	polygonOffset
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map textures/rocky_ruins/arrow4
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        depthWrite
    }
}

textures/rocky_ruins/barsign_paint
{
	q3map_nolightmap
    {
        map textures/rocky_ruins/barsign_paint
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

textures/rocky_ruins/screen2
{
	q3map_nolightmap
    {
        map textures/rocky_ruins/screen2
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
        tcMod scroll 0.25 0
    }
    {
        map textures/rocky_ruins/screen2_glw
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
        tcMod scroll 0.25 0
    }
}

textures/rocky_ruins/screen5
{
	q3map_nolightmap
    {
        map textures/rocky_ruins/screen5
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        depthWrite
    }
    {
        map textures/rocky_ruins/screen5_glw
        blendFunc GL_ONE GL_ONE
        depthWrite
        glow
        rgbGen identity
    }
    {
        map textures/rocky_ruins/screen5_anim
        blendFunc GL_ONE GL_ONE
        depthWrite
        glow
        rgbGen wave sin 0 0.5 0 1
    }
}

textures/rocky_ruins/screen4
{
	q3map_nolightmap
    {
        map textures/rocky_ruins/screen4
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/rocky_ruins/screen4_glw
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
    {
        clampmap textures/rocky_ruins/screen4_anim
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
        tcMod scale 2 1
        tcMod rotate 20
    }
}

textures/rocky_ruins/screen3
{
	q3map_nolightmap
    {
        map textures/rocky_ruins/screen3
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/rocky_ruins/screen3_glw
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
    {
        clampmap textures/rocky_ruins/screen3_anim
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
        tcMod scale 2 1
        tcMod rotate -10
    }
}

textures/rocky_ruins/halfwall_top_effect
{
	qer_editorimage	textures/rocky_ruins/halfwall_top
	q3map_nolightmap
    {
        map textures/rocky_ruins/ciclox04
        blendFunc GL_ONE GL_SRC_COLOR
        depthWrite
        rgbGen identity
        tcMod scroll 0 2
    }
    {
        map textures/rocky_ruins/grate3_effect3
        blendFunc GL_DST_COLOR GL_ONE
        tcMod scroll 0 0.5
    }
    {
        map textures/rocky_ruins/halfwall_top
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen lightingDiffuse
    }
}

textures/rocky_ruins/grate3_effect
{
	qer_editorimage	textures/rocky_ruins/grate3
	q3map_material	HollowMetal
    {
        map $lightmap
    }
    {
        map textures/rocky_ruins/ciclox04
        blendFunc GL_DST_COLOR GL_ZERO
        tcMod scroll 2 0
    }
    {
        map textures/rocky_ruins/grate3_effect2
        blendFunc GL_ONE GL_ONE
        tcMod scroll 0.5 0
    }
    {
        map textures/rocky_ruins/grate3
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

textures/rocky_ruins/bluelight1
{
    {
        map textures/rocky_ruins/bluelight1
        blendFunc GL_ONE GL_ZERO
    }
    {
        map textures/rocky_ruins/bluelight2
        blendFunc GL_ONE GL_ONE
        rgbGen wave inversesawtooth 0 1 0 0.5
    }
    {
        map textures/rocky_ruins/bluelight3
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0 1 0 0.6
    }
}

textures/rocky_ruins/ticket_stubs
{
	qer_editorimage	textures/rocky_ruins/grouptickets
	surfaceparm	nonsolid
	polygonOffset
    {
// blendFunc GL_ONE GL_ZERO

        map textures/rocky_ruins/grouptickets
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

textures/rocky_ruins/poster
{
	qer_editorimage	textures/rocky_ruins/poster
	surfaceparm	nonsolid
	polygonOffset
    {
// blendFunc GL_ONE GL_ZERO

        map textures/rocky_ruins/poster
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

textures/rocky_ruins/palor_door_switch
{
	qer_editorimage	textures/rocky_ruins/palor_door_closed
	q3map_nolightmap
    {
        map textures/colors/black
    }
    {
        animMap 1 textures/rocky_ruins/palor_door_closed textures/rocky_ruins/palor_door_open 
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
    {
        map textures/rocky_ruins/palor_frame
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

textures/rocky_ruins/parlor_switch1
{
	qer_editorimage	textures/rocky_ruins/palor_map_base
	q3map_nolightmap
    {
        map textures/colors/black
        rgbGen identity
    }
    {
        map textures/rocky_ruins/palor_map_base
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
    {
        animMap 2 textures/rocky_ruins/parlor_1switch_1 textures/rocky_ruins/parlor_1switch_2 textures/rocky_ruins/parlor_1switch_3 textures/rocky_ruins/parlor_1switch_4 
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
    {
        map textures/rocky_ruins/palor_map_frame
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

textures/rocky_ruins/parlor_switch2
{
	qer_editorimage	textures/rocky_ruins/palor_map_base
	q3map_nolightmap
    {
        map textures/colors/black
        rgbGen identity
    }
    {
        map textures/rocky_ruins/palor_map_base
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
    {
        animMap 2 textures/rocky_ruins/parlor_2switch_1 textures/rocky_ruins/parlor_2switch_2 textures/rocky_ruins/parlor_2switch_3 textures/rocky_ruins/parlor_2switch_4 
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
    {
        map textures/rocky_ruins/palor_map_frame
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

textures/rocky_ruins/parlor_switch3
{
	qer_editorimage	textures/rocky_ruins/palor_map_base
	q3map_nolightmap
    {
        map textures/colors/black
        rgbGen identity
    }
    {
        map textures/rocky_ruins/palor_map_base
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
    {
        animMap 2 textures/rocky_ruins/parlor_3switch_1 textures/rocky_ruins/parlor_3switch_2 textures/rocky_ruins/parlor_3switch_3 textures/rocky_ruins/parlor_3switch_4 
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
    {
        map textures/rocky_ruins/palor_map_frame
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

textures/rocky_ruins/parlor_switch4
{
	qer_editorimage	textures/rocky_ruins/palor_map_base
	q3map_nolightmap
    {
        map textures/colors/black
        rgbGen identity
    }
    {
        map textures/rocky_ruins/palor_map_base
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
    {
        animMap 2 textures/rocky_ruins/parlor_4switch_1 textures/rocky_ruins/parlor_4switch_2 textures/rocky_ruins/parlor_4switch_3 textures/rocky_ruins/parlor_4switch_4 
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
    {
        map textures/rocky_ruins/palor_map_frame
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

textures/rocky_ruins/newgrate
{
	qer_editorimage	textures/rocky_ruins/newgrate
	q3map_material	HollowMetal
	q3map_alphashadow
	cull	twosided
    {
        map textures/rocky_ruins/newgrate
        alphaFunc GE128
        blendFunc GL_ONE GL_ZERO
        depthWrite
    }
    {
        map $lightmap
        blendFunc GL_ONE GL_ZERO
        depthFunc equal
    }
    {
        map textures/rocky_ruins/newgrate
        blendFunc GL_DST_COLOR GL_ZERO
        depthFunc equal
    }
}

textures/rocky_ruins/exit_arrows
{
	qer_editorimage	textures/rocky_ruins/exit_arrows
	polygonOffset
	q3map_nolightmap
    {
        map textures/rocky_ruins/exit_arrows
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

textures/rocky_ruins/rocky_light
{
    {
        map $lightmap
    }
    {
        map textures/rocky_ruins/rocky_light
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/rocky_ruins/rocky_light_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/rocky_ruins/grate
{
	q3map_material	HollowMetal
    {
        map $lightmap
    }
    {
        map textures/rocky_ruins/grate
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/rocky_ruins/grating_new
{
	q3map_material	HollowMetal
    {
        map $lightmap
    }
    {
        map textures/rocky_ruins/grating_new
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/rocky_ruins/metal2_top
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/rocky_ruins/metal2_top
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/rocky_ruins/metal1_bottom
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/rocky_ruins/metal1_bottom
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/rocky_ruins/metal1_top
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/rocky_ruins/metal1_top
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/rocky_ruins/metal2
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/rocky_ruins/metal2
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/rocky_ruins/metal2_bottom
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/rocky_ruins/metal2_bottom
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/rocky_ruins/metal1
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/rocky_ruins/metal1
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/rocky_ruins/pillar_side
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/rocky_ruins/pillar_side
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/rocky_ruins/r_floor
{
    {
        map $lightmap
    }
    {
        map textures/rocky_ruins/r_floor
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/rocky_ruins/r_floor2
{
    {
        map $lightmap
    }
    {
        map textures/rocky_ruins/r_floor2
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/rocky_ruins/r_diwall
{
	q3map_material	HollowMetal
    {
        map $lightmap
    }
    {
        map textures/rocky_ruins/r_diwall
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/rocky_ruins/rancor_floor
{
	q3map_material	HollowMetal
    {
        map $lightmap
    }
    {
        map textures/rocky_ruins/rancor_floor
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/rocky_ruins/pilar_side_b
{
	qer_editorimage	textures/rocky_ruins/pillar_side
	q3map_material	SolidMetal
	q3map_nolightmap
    {
        map textures/rocky_ruins/pillar_side
        rgbGen lightingDiffuse
    }
}

models/map_objects/rocky_ruins/light_rancor_cage
{
	q3map_nolightmap
	q3map_onlyvertexlighting
	cull	twosided
    {
        map models/map_objects/rocky_ruins/light_rancor_cage
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

models/map_objects/rocky_ruins/light_rancor_bulb
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/rocky_ruins/light_rancor_bulb
        blendFunc GL_ONE GL_ONE
    }
}

models/map_objects/rocky_ruins/light_rancor
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/rocky_ruins/light_rancor
        rgbGen lightingDiffuse
    }
}

models/map_objects/rocky_ruins/monitor_buttons
{
    {
        map $lightmap
    }
    {
        map models/map_objects/rocky_ruins/monitor_buttons
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map models/map_objects/rocky_ruins/monitor_buttons_glw
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

models/map_objects/rocky_ruins/monitor_screen_03
{
    {
        map models/map_objects/rocky_ruins/monitor_screen_03
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map models/map_objects/rocky_ruins/monitor_screen_03_glw
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
    {
        map models/map_objects/rocky_ruins/monitor_screen_03_wipe
        blendFunc GL_ONE GL_ONE
        rgbGen identity
        tcMod scroll 0 -0.125
    }
}

models/map_objects/rocky_ruins/monitor_screen_02
{
    {
        map models/map_objects/rocky_ruins/monitor_screen_02
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map models/map_objects/rocky_ruins/monitor_screen_02_glw
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

models/map_objects/rocky_ruins/monitor_screen_01
{
    {
        map models/map_objects/rocky_ruins/monitor_screen_01
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map models/map_objects/rocky_ruins/monitor_screen_01_glw
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

models/map_objects/rocky_ruins/floorlight
{
    {
        map models/map_objects/rocky_ruins/floorlight
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/rocky_ruins/floorlight_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

models/map_objects/rocky_ruins/bone_pile_flesh
{
	q3map_nolightmap
	cull	twosided
    {
        map models/map_objects/rocky_ruins/bone_pile_flesh
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen lightingDiffuse
    }
}

models/map_objects/rocky_ruins/switch
{
    {
        map $lightmap
    }
    {
        map models/map_objects/rocky_ruins/switch
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        oneshotanimMap 1 models/map_objects/rocky_ruins/switch_r models/map_objects/rocky_ruins/switch_g 
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/decals/barsign
{
	polygonOffset
    {
        map textures/rocky_ruins/barsign
        blendFunc GL_ONE GL_ONE
    }
    {
        map textures/rocky_ruins/barsign_glow
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

textures/decals/screen1
{
	polygonOffset
    {
        map textures/rocky_ruins/screen1
        blendFunc GL_ONE GL_ONE
    }
    {
        map textures/rocky_ruins/screen1_glw
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

