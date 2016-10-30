textures/bespin/sky
{
	qer_editorimage	textures/skies/sky.tga
	q3map_surfacelight	450
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	textures/skies/bespin 512 -
}

textures/bespin/sky_platform
{
	qer_editorimage	textures/skies/sky.tga
	q3map_surfacelight	250
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	textures/skies/bespin 512 -
}

textures/bespin/u_tunpipe02
{
	surfaceparm	slick
	surfaceparm	nodamage
    {
        map $lightmap
    }
    {
        map textures/bespin/u_tunpipe02
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/bespin/u_tunpipe02_glw
        blendFunc GL_ONE GL_ONE
    }
}

textures/bespin/floor_slick_nodamage
{
	qer_editorimage	textures/impgarrison/floor01.tga
	surfaceparm	slick
	surfaceparm	nodamage
    {
        map $lightmap
    }
    {
        map textures/bespin/basic2
        blendFunc GL_DST_COLOR GL_ZERO
        alphaGen lightingSpecular
    }
}

textures/bespin/cortosis
{
	q3map_material	SolidMetal
	surfaceparm	forcefield
    {
        map $lightmap
    }
    {
        map textures/bespin/cortosis
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/bespin/u_floor02_nodamage
{
	qer_editorimage	textures/bespin/u_floor02.tga
	surfaceparm	nodamage
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/bespin/u_floor02
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/bespin/u_shaftwall
{
		
    {
        map $lightmap
    }
    {
        map textures/bespin/u_shaftwall
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/bespin/u_shaft_glow
        blendFunc GL_ONE GL_ONE_MINUS_SRC_COLOR
        detail
    }
}

textures/bespin/u_grate
{
	q3map_material	HollowMetal
    {
        map $lightmap
        alphaFunc GE128
    }
    {
        map textures/bespin/u_grate
        alphaFunc GE128
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/bespin/u_beam02
{
	cull	twosided
    {
        map $lightmap
        alphaFunc GE128
    }
    {
        map textures/bespin/u_beam02
        alphaFunc GE128
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/bespin/u_light01
{
    {
        map $lightmap
    }
    {
        map textures/bespin/u_light01
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/bespin/u_lightglow01
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

textures/bespin/u_light02
{
    {
        map $lightmap
    }
    {
        map textures/bespin/u_light02
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/bespin/u_light02glow
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 1 0.1 0 0.5
    }
}

textures/bespin/u_light02a
{

    {
        map $lightmap
    }
    {
        map textures/bespin/u_light02a
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/bespin/u_light02aglow
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 1 0.1 0 0.5
    }
}

textures/bespin/u_light03
{
	qer_editorimage	textures/bespin/u_light03

    {
        map $lightmap
    }
    {
        map textures/bespin/u_light03
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/bespin/u_light03glow
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 1 0.1 0 0.5
    }
}

textures/bespin/u_light04
{
    {
        map $lightmap
    }
    {
        map textures/bespin/u_light04
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/bespin/u_light04glow
        blendFunc GL_ONE GL_ONE
    }
}

textures/bespin/u_hangdoor02
{
    {
        map $lightmap
    }
    {
        map textures/bespin/u_hangdoor02
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/bespin/u_hangdoorglw
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.5 0.3 0 0.5
    }
}

textures/bespin/botton_on
{
    {
        map $lightmap
    }
    {
        map textures/bespin/botton
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/bespin/botton_on2
        blendFunc GL_ONE GL_ONE
    }
}

textures/bespin/force
{
	
	q3map_nolightmap
    {
        map textures/bespin/force
        blendFunc GL_ONE GL_ONE_MINUS_SRC_COLOR
        tcMod rotate -4
        tcMod turb 0 0.4 0 1
    }
    {
        map textures/bespin/force
        blendFunc GL_ONE GL_ONE
        tcMod rotate 4
        tcMod turb 0 0.3 0 1
    }
}

textures/bespin/u_tube
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/bespin/u_tube
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/bespin/u_tubeglw
        blendFunc GL_ONE GL_ONE_MINUS_SRC_COLOR
        rgbGen wave sin 0.3 0.6 0.2 0.5
    }
}

textures/bespin/u_casing03
{
		q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/bespin/u_casing03
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/bespin/u_casing03glw
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.3 0.6 0.2 0.5
    }
}

textures/bespin/u_shaftwall02
{
    {
        map $lightmap
    }
    {
        map textures/bespin/u_shaftwall02
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/bespin/u_shaft_glow02
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.3 0.6 0.2 0.5
    }
}

textures/bespin/n_win01
{
// q3map_surfacelight	500

	surfaceparm	nomarks
	polygonOffset
	q3map_nolightmap
    {
        map textures/bespin/n_win01
        alphaFunc GT0
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        rgbGen identity
    }
    {
        map textures/bespin/n_win01glow
        blendFunc GL_ONE GL_ONE
    }
}

textures/bespin/n_win02
{
// q3map_surfacelight	500

	surfaceparm	nomarks
	polygonOffset
	q3map_nolightmap
    {
        map textures/bespin/n_win02
        alphaFunc GT0
        blendFunc GL_DST_COLOR GL_SRC_COLOR
    }
    {
        map textures/bespin/n_win02glow
        blendFunc GL_ONE GL_ONE
    }
}

textures/bespin/n_win03
{
// q3map_surfacelight	500

	polygonOffset
	q3map_nolightmap
    {
        map textures/bespin/n_win03
        alphaFunc GT0
        blendFunc GL_DST_COLOR GL_SRC_COLOR
    }
    {
        map textures/bespin/n_win03glow
        blendFunc GL_ONE GL_ONE
    }
}

textures/bespin/mp_r_light04
{

    {
        map $lightmap
    }
    {
        map textures/bespin/mp_r_light04
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/bespin/mp_r_light04_glow
        blendFunc GL_ONE GL_ONE
    }
}

textures/bespin/mp_b_light04
{

    {
        map $lightmap
    }
    {
        map textures/bespin/mp_b_light04
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/bespin/mp_b_light04_glow
        blendFunc GL_ONE GL_ONE
    }
}

textures/bespin/mp_r_light02a
{

    {
        map $lightmap
    }
    {
        map textures/bespin/mp_r_light02a
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/bespin/mp_r_light02a_glow
        blendFunc GL_ONE GL_ONE
    }
}

textures/bespin/mp_b_light02a
{

    {
        map $lightmap
    }
    {
        map textures/bespin/mp_b_light02a
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/bespin/mp_b_light02a_glow
        blendFunc GL_ONE GL_ONE
    }
}

textures/bespin/mp_b_wall04
{

    {
        map $lightmap
    }
    {
        map textures/bespin/mp_b_wall04
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/bespin/mp_b_casing04
{
		q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/bespin/mp_b_casing04
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/bespin/mp_b_casing04_glow
        blendFunc GL_ONE GL_ONE
    }
}

textures/bespin/mp_r_casing04
{
		q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/bespin/mp_r_casing04
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/bespin/mp_r_casing04_glow
        blendFunc GL_ONE GL_ONE
    }
}

textures/bespin/mp_r_wall04
{
	
    {
        map $lightmap
    }
    {
        map textures/bespin/mp_r_wall04
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/bespin/mp_r_wall04_glow
        blendFunc GL_ONE GL_ONE
    }
}

textures/bespin/bottondroidopen
{
	qer_editorimage	textures/bespin/bottondroid
    {
        map $lightmap
    }
    {
        map textures/bespin/bottondroid
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/bespin/bottondroidglow
        blendFunc GL_ONE GL_ONE
    }
}

textures/bespin/n_light01
{

	polygonOffset
    {
        map $lightmap
    }
    {
        map textures/bespin/n_light01
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/bespin/n_light01_glw
        blendFunc GL_ONE GL_ONE
    }
}

textures/bespin/bswitch
{
    {
        map $lightmap
    }
    {
        map textures/bespin/bswitch
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/bespin/bswitcha
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0 1 0 1
    }
    {
        map textures/bespin/bswitchx
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 1 0.1 0 5
    }
}

textures/bespin/bswitchon
{
	qer_editorimage	textures/bespin/bswitch

    {
        map $lightmap
    }
    {
        map textures/bespin/bswitch2
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/bespin/bswitchb
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 1 0.1 0 5
    }
}

textures/bespin/botton_off
{
	qer_editorimage	textures/bespin/botton
	
    {
        map $lightmap
    }
    {
        map textures/bespin/botton
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/bespin/door01locked
{
	qer_editorimage	textures/bespin/door01

    {
        map $lightmap
    }
    {
        map textures/bespin/door01
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        oneshotanimMap 1 textures/bespin/door01lock_glw textures/bespin/door01open_glw 
        blendFunc GL_ONE GL_ONE
    }
}

textures/bespin/control01
{

    {
        map $lightmap
    }
    {
        map textures/bespin/control01
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/bespin/control01_glw
        blendFunc GL_ONE GL_ONE
    }
}

textures/bespin/control02
{
	qer_editorimage	textures/bespin/control02
    {
        map $lightmap
    }
    {
        map textures/bespin/control02
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/bespin/control02_glw
        blendFunc GL_ONE GL_ONE
    }
}

textures/bespin/control03
{
	qer_editorimage	textures/bespin/control03
    {
        map $lightmap
    }
    {
        map textures/bespin/control03
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/bespin/control03_glw
        blendFunc GL_ONE GL_ONE
    }
}

textures/bespin/control04
{
	qer_editorimage	textures/bespin/control04

    {
        map $lightmap
    }
    {
        map textures/bespin/control04
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/bespin/control04_glw
        blendFunc GL_ONE GL_ONE
    }
}

textures/bespin/u_carbfloor
{
	q3map_material	HollowMetal
    {
        map $lightmap
    }
    {
        map textures/bespin/u_carbfloor
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/bespin/u_carbfloor_glw
        blendFunc GL_ONE GL_ONE
    }
}

textures/bespin/u_carbfloor02
{

	q3map_material	HollowMetal
    {
        map $lightmap
    }
    {
        map textures/bespin/u_carbfloor02
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/bespin/u_carbfloor02_glw
        blendFunc GL_ONE GL_ONE
    }
}

textures/bespin/water2
{
	qer_editorimage	textures/bespin/water1
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	water
	surfaceparm	fog
	surfaceparm	trans
	q3map_material	Water
	q3map_nolightmap
	q3map_onlyvertexlighting
	fogparms	( 0.113725 0.137255 0.380392 ) 1024.0
    {
        map textures/bespin/water1
        blendFunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
        rgbGen exactVertex
        alphaGen const 0.2
        tcMod scroll 0.005 0.01
        tcMod turb 1 0.03 0 0.3
    }
    {
        map textures/bespin/water1
        blendFunc GL_ONE GL_SRC_ALPHA
        rgbGen exactVertex
        alphaGen const 0.2
        tcMod scroll -0.005 -0.01
        tcMod turb 0 -0.03 0.5 -0.3
    }
}

textures/bespin/breakable_grate1
{
	qer_editorimage	textures/kejim/grate02_broke
	q3map_material	SolidMetal
    {
        map textures/kejim/grate02_broke
        alphaFunc LT128
        blendFunc GL_ONE_MINUS_DST_COLOR GL_SRC_ALPHA
        rgbGen identity
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_SRC_COLOR
    }
}

textures/bespin/breakable_grate2
{
	qer_editorimage	textures/kejim/grate02_long_broken
		q3map_material	HollowMetal
    {
        map textures/kejim/grate02_long_broken
        alphaFunc LT128
        blendFunc GL_ONE GL_SRC_ALPHA
        rgbGen identity
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_SRC_COLOR
    }
}

textures/bespin/sky_test
{
	qer_editorimage	textures/skies/sky.tga
	q3map_surfacelight	450
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	textures/skies/bespin 9000 -
    {
        map textures/jontest/cloud
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        tcMod scroll 0 0.04
    }
}

textures/bespin/basic8
{
	q3map_material	Concrete
    {
        map $lightmap
    }
    {
        map textures/bespin/basic8
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/bespin/basic2
{
	q3map_material	Concrete
    {
        map $lightmap
    }
    {
        map textures/bespin/basic2
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/bespin/basic6
{
	q3map_material	Concrete
    {
        map $lightmap
    }
    {
        map textures/bespin/basic6
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/bespin/basic
{
	q3map_material	Concrete
    {
        map $lightmap
    }
    {
        map textures/bespin/basic
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/bespin/ceiling01
{
	q3map_material	Concrete
    {
        map $lightmap
    }
    {
        map textures/bespin/ceiling01
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/bespin/floor2
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/bespin/floor2
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/bespin/floor3
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/bespin/floor3
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/bespin/floor4
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/bespin/floor4
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/bespin/floor
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/bespin/floor
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/bespin/grate
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/bespin/grate
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/bespin/hydraulic
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/bespin/hydraulic
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/bespin/n_floor01
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/bespin/n_floor01
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/bespin/n_floor02
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/bespin/n_floor02
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/bespin/newfloor
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/bespin/newfloor
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/bespin/plating_b
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/bespin/plating_b
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/bespin/u_floor04
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/bespin/u_floor04
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/bespin/u_floor02
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/bespin/u_floor02
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/bespin/u_floor03
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/bespin/u_floor03
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/bespin/u_floor01
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/bespin/u_floor01
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/bespin/u_metalgreen
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/bespin/u_metalgreen
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/bespin/u_metalorange02
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/bespin/u_metalorange02
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/bespin/u_metallg
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/bespin/u_metallg
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/bespin/u_metalorange
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/bespin/u_metalorange
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/bespin/u_metalgreenrib
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/bespin/u_metalgreenrib
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/bespin/wall_iron
{
	q3map_material	SolidMetal
    {
        map $lightmap
    }
    {
        map textures/bespin/wall_iron
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/bespin/sky_duel
{
	qer_editorimage	textures/skies/sky.tga
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	textures/skies/bespin 512 -
}

