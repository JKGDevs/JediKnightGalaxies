// q3map_sun <red> <green> <blue> <intensity> <degrees> <elevation>

// degree of 0 = from the east

// degree of 90 = north

// altitude of 0 = sunrise/set, 90 = noon

// Pure Black

textures/mp/s_black
{
	qer_editorimage	textures/mp/black.tga
	surfaceparm	noimpact
	surfaceparm	nomarks
	q3map_nolightmap
    {
        map textures/mp/black
        rgbGen identity
    }
}

textures/mp/s_black_fog
{
	qer_editorimage	textures/fogs/fog
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0 0 0 ) 512.0
}

textures/mp/s_duel3_fog
{
	qer_editorimage	textures/fogs/fog
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0 0 0.25 ) 512.0
}

// PURE  LIGHT SURF

textures/mp/s_flatred_bright
{
	q3map_lightimage	textures/mp/red.tga
	qer_editorimage	textures/mp/red.tga
	q3map_surfacelight	20000
	q3map_lightsubdivide	256
	q3map_nolightmap
    {
        map textures/mp/red
    }
}

textures/mp/s_spacectf1
{
// q3map_surfacelight	40

// sun 0.887778 0.868540 1.000000 60 0 75

	qer_editorimage	textures/skies/space_bk
	q3map_lightsubdivide	256
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks

	notc
	q3map_nolightmap
	skyParms	textures/skies/space 512 -
}

// PURE RED SKY SURF

textures/mp/s_redsky
{
// q3map_sun <red> <green> <blue> <intensity> <degrees> <elevation>

	q3map_lightimage	textures/mp/red.tga
	qer_editorimage	textures/mp/red.tga
	q3map_surfacelight	130
	q3map_lightsubdivide	512
	sun 1 0.1 0.1 140 140 90
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	textures/mp/red 256 -
}

textures/mp/s_space
{
	qer_editorimage	textures/skies/space_bk
	q3map_surfacelight	70
	q3map_lightsubdivide	256
	sun 0.538903 0.6962 1 60 0 35
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	textures/skies/space 512 -
}

// ---------------------------------------------

// Floor Shaders

// ---------------------------------------------

// shiny floor metal

textures/mp/s_metalc
{
// q3map_nonplanar

	q3map_shadeangle	90
	qer_editorimage	textures/mp/s_metalc.tga
    {
        map textures/mp/s_metalc_env
        rgbGen identity
        tcGen environment
        tcMod scale 0.25 0.25
    }
    {
        map textures/mp/s_metalc
        blendFunc GL_ONE_MINUS_SRC_ALPHA GL_SRC_ALPHA
        rgbGen identity
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

// shiny floor metal dark

textures/mp/s_metalc_dark
{
	q3map_shadeangle	90
	qer_editorimage	textures/mp/s_metalc_dark.tga
    {
        map textures/mp/s_metalc_env
        rgbGen identity
        tcGen environment
        tcMod scale 0.25 0.25
    }
    {
        map textures/mp/s_metalc_dark
        blendFunc GL_ONE_MINUS_SRC_ALPHA GL_SRC_ALPHA
        rgbGen identity
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

// ---------------------------------------------

// Light Shaders

// ---------------------------------------------

// solid white

textures/mp/s_flatwhite
{
// final value

	qer_editorimage	textures/kejim/flatwhite.tga
	q3map_surfacelight	3000
	q3map_lightsubdivide	64
	q3map_nolightmap
    {
        map textures/kejim/flatwhite
    }
}

textures/mp/s_flatblue
{
// final value

// q3map_lightsubdivide 64

	q3map_lightimage	textures/mp/s_flatblue_blend.tga
	qer_editorimage	textures/mp/s_flatblue.tga
	q3map_surfacelight	3000
    {
        map $lightmap
        rgbGen identity
    }
    {
        map textures/mp/s_flatblue
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/mp/s_flatblue_blend
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.8 0.05 0 10
    }
}

textures/mp/s_flatred
{
// final value

// q3map_lightsubdivide 64

	qer_editorimage	textures/mp/s_flatred.tga
	q3map_surfacelight	3000
    {
        map $lightmap
        rgbGen identity
    }
    {
        map textures/mp/s_flatred
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/mp/s_flatred
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.8 0.05 0 10
    }
}

// solid yellow /w lightmap and blend

textures/mp/s_flatyellow
{
// final value

// q3map_lightsubdivide 64

	qer_editorimage	textures/mp/s_flatyellow.tga
	q3map_surfacelight	3000
    {
        map $lightmap
        rgbGen identity
    }
    {
        map textures/mp/s_flatyellow
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/mp/s_flatyellow_blend
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.8 0.05 0 10
    }
}

// solid yellow /w lightmap and blend - nolight

textures/mp/s_flatyellow_nolight
{
	qer_editorimage	textures/mp/s_flatyellow.tga
    {
        map $lightmap
        rgbGen identity
    }
    {
        map textures/mp/s_flatyellow
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/mp/s_flatyellow_blend
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.8 0.05 0 10
    }
}

// ---------------------------------------------

// Track Lights (used in consession)

// ---------------------------------------------

// Small Single Track - Vertical

textures/mp/s_tracklight
{
// final

	qer_editorimage	textures/mp/s_tracklight.tga

    {
        map $lightmap
    }
    {
        map textures/mp/s_tracklight
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/mp/s_tracklight_blend
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.8 0.1 0 0.5
    }
}

textures/mp/s_tracklight_d
{
// final

	qer_editorimage	textures/mp/s_tracklight.tga
	q3map_surfacelight	1000
	q3map_lightsubdivide	64

    {
        map $lightmap
    }
    {
        map textures/mp/s_tracklight
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/mp/s_tracklight_blend
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.8 0.1 0 0.5
    }
}

// Large Single Track - Horizontal

textures/mp/s_tracklight2
{
// final

	qer_editorimage	textures/cairn/tracklight.tga
	q3map_surfacelight	500
    {
        map $lightmap
    }
    {
        map textures/cairn/tracklight
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/cairn/tracklight_glow
        blendFunc GL_ONE GL_ONE
    }
}

// Multiple Out-Of-Order Slits

textures/mp/s_tracklight3
{
// subject to change

	qer_editorimage	textures/cairn/tracklight2.tga
	q3map_surfacelight	1500
	q3map_lightsubdivide	64
    {
        map $lightmap
    }
    {
        map textures/cairn/tracklight2
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/cairn/tracklight2_glow
        blendFunc GL_ONE GL_ONE
    }
}

textures/mp/s_tracklight3_bright
{
// subject to change

	qer_editorimage	textures/cairn/tracklight2.tga
	q3map_surfacelight	3500
	q3map_lightsubdivide	64
    {
        map $lightmap
    }
    {
        map textures/cairn/tracklight2
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/cairn/tracklight2_glow
        blendFunc GL_ONE GL_ONE
    }
}

// Rect 3 Slit Trimmer

textures/mp/s_tracklight4
{
// final

	qer_editorimage	textures/kejim/kej_lights.tga
	q3map_surfacelight	750
	q3map_lightsubdivide	64
    {
        map $lightmap
    }
    {
        map textures/kejim/kej_lights
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/kejim/kej_lights_glow
        blendFunc GL_ONE GL_ONE
    }
}

textures/mp/s_tracklight5
{
// final

	qer_editorimage	textures/doomgiver/doortrim01d
	q3map_surfacelight	1500
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
        rgbGen wave sin 1 0.15 0 8
    }
}

textures/mp/s_tracklight6
{
// final

	qer_editorimage	textures/kejim/metal1
	q3map_surfacelight	1500
    {
        map $lightmap
    }
    {
        map textures/kejim/metal1
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/kejim/metal1glow
        blendFunc GL_ONE GL_ONE
    }
}

textures/mp/s_tracklight7
{
// final

	qer_editorimage	textures/kejim/strutlight
	q3map_surfacelight	3000
    {
        map $lightmap
    }
    {
        map textures/kejim/strutlight
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/kejim/strutlightglow
        blendFunc GL_ONE GL_ONE
    }
}

// ---------------------------------------------

// Micro Pattern Lights

// ---------------------------------------------

// Single White Strip

textures/mp/s_lightpanel1
{
	qer_editorimage	textures/kejim/lightpanel02
	q3map_surfacelight	1500
	q3map_lightsubdivide	120
    {
        map $lightmap
    }
    {
        map textures/kejim/lightpanel02
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/kejim/lightpanel02glow
        blendFunc GL_ONE GL_ONE
    }
}

// Single White Strip (BRIGHT)

textures/mp/s_lightpanel1_bright
{
	qer_editorimage	textures/kejim/lightpanel02
	q3map_surfacelight	1500
	q3map_lightsubdivide	120
    {
        map $lightmap
    }
    {
        map textures/kejim/lightpanel02
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/kejim/lightpanel02glow
        blendFunc GL_ONE GL_ONE
    }
}

// Double Wide White Strip

textures/mp/s_lightpanel2
{
	qer_editorimage	textures/kejim/lightpanel
	q3map_surfacelight	1500
	q3map_lightsubdivide	120
    {
        map $lightmap
    }
    {
        map textures/kejim/lightpanel
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/kejim/lightpanelglow
        blendFunc GL_ONE GL_ONE
    }
}

// Double Wide White Strip (BRIGHT)

textures/mp/s_lightpanel2_bright
{
	qer_editorimage	textures/kejim/lightpanel
	q3map_surfacelight	1500
	q3map_lightsubdivide	120
    {
        map $lightmap
    }
    {
        map textures/kejim/lightpanel
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/kejim/lightpanelglow
        blendFunc GL_ONE GL_ONE
    }
}

// ---------------------------------------------

// Lights

// ---------------------------------------------

textures/mp/s_squarelight
{
	qer_editorimage	textures/impdetention/light_white
	q3map_surfacelight	3000
	q3map_nolightmap
    {
        map textures/impdetention/light_white
        rgbGen identity
    }
}

// Simple Round Light

textures/mp/s_roundy
{
	qer_editorimage	textures/cairn/tracklight3
	q3map_surfacelight	6000
	q3map_lightsubdivide	64
    {
        map $lightmap
    }
    {
        map textures/cairn/tracklight3
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/cairn/tracklight3_glow
        blendFunc GL_ONE GL_ONE
    }
}

// Square Elevator Keys

textures/mp/s_deathconlight
{
	qer_editorimage	textures/doomgiver/deathconlight.tga
	q3map_surfacelight	3000
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
        rgbGen wave sin 0.75 0.005 0 10
    }
}

// Squares (blue'ish tint)

textures/mp/s_squareslight
{
	qer_editorimage	textures/mp/s_squareslight.tga
	q3map_surfacelight	3000
    {
        map $lightmap
    }
    {
        map textures/mp/s_squareslight
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/mp/s_squareslight_blend
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.75 0.005 0 10
    }
}

// Squares (yellowish'ish tint)

textures/mp/s_squareslight_y
{
	qer_editorimage	textures/mp/s_squareslight_y.tga
	q3map_surfacelight	3000
    {
        map $lightmap
    }
    {
        map textures/mp/s_squareslight_y
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
// rgbGen wave sin 0.75 0.005 0 10

        map textures/mp/s_squareslight_y_blend
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.5 0.05 0 10
    }
}

// Metal Track Light

textures/mp/s_metal_hlight
{
	qer_editorimage	textures/mp/s_metal_hlight.tga
	q3map_surfacelight	3000
	q3map_lightsubdivide	60
    {
        map $lightmap
    }
    {
        map textures/mp/s_metal_hlight
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/mp/s_metal_hlight_blend
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.75 0.005 0 10
    }
}

// Metal Track Light Vertical

textures/mp/s_metal_vlight
{
	qer_editorimage	textures/mp/s_metal_vlight.tga
	q3map_surfacelight	3000
	q3map_lightsubdivide	60
    {
        map $lightmap
    }
    {
        map textures/mp/s_metal_vlight
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/mp/s_metal_vlight_blend
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.75 0.005 0 10
    }
}

// Dual Red Strips

textures/mp/s_redstrip
{
	q3map_lightimage	textures/mp/s_redstrip_blend.tga
	qer_editorimage	textures/mp/s_redstrip.tga
	q3map_surfacelight	3000
	q3map_lightsubdivide	60
    {
        map $lightmap
    }
    {
        map textures/mp/s_redstrip
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/mp/s_redstrip_blend
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.85 0.005 0 10
    }
}

// Dual Red Strips

textures/mp/s_redstrip2
{
	q3map_lightimage	textures/mp/s_redstrip_blend.tga
	qer_editorimage	textures/mp/s_redstrip.tga
	q3map_surfacelight	5500
	q3map_lightsubdivide	60
    {
        map $lightmap
    }
    {
        map textures/mp/s_redstrip
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/mp/s_redstrip_blend
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.85 0.005 0 10
    }
}

textures/mp/s_redstrip3_nolight
{
	q3map_lightimage	textures/mp/s_redstrip_blend.tga
	qer_editorimage	textures/mp/s_redstrip.tga
    {
        map $lightmap
    }
    {
        map textures/mp/s_redstrip
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/mp/s_redstrip_blend
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.85 0.005 0 10
    }
}

// Dual Blue Strips

textures/mp/s_bluestrip
{
	q3map_lightimage	textures/mp/s_bluestrip_blend.tga
	qer_editorimage	textures/mp/s_bluestrip.tga
	q3map_surfacelight	3000
	q3map_lightsubdivide	60
    {
        map $lightmap
    }
    {
        map textures/mp/s_bluestrip
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/mp/s_bluestrip_blend
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.85 0.005 0 10
    }
}

textures/mp/s_taspir
{
	q3map_lightimage	textures/taspir/light.tga
	qer_editorimage	textures/taspir/light.tga
	q3map_surfacelight	5000
	q3map_lightsubdivide	64
    {
        map $lightmap
    }
    {
        map textures/taspir/light
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/taspir/light
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

textures/mp/s_steptrim1
{
	q3map_lightimage	textures/mp/s_steptrim1_blend.tga
	qer_editorimage	textures/mp/s_steptrim1.tga
	q3map_surfacelight	3000
	q3map_lightsubdivide	64
    {
        map $lightmap
    }
    {
        map textures/mp/s_steptrim1
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/mp/s_steptrim1_blend
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

textures/mp/s_steptrim1_b
{
	q3map_lightimage	textures/mp/s_steptrim1_blendb.tga
	qer_editorimage	textures/mp/s_steptrim1.tga
	q3map_surfacelight	3000
	q3map_lightsubdivide	64
    {
        map $lightmap
    }
    {
        map textures/mp/s_steptrim1
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/mp/s_steptrim1_blendb
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

textures/mp/s_steptrim1_r
{
	q3map_lightimage	textures/mp/s_steptrim1_blendr.tga
	qer_editorimage	textures/mp/s_steptrim1.tga
	q3map_surfacelight	3000
	q3map_lightsubdivide	64
    {
        map $lightmap
    }
    {
        map textures/mp/s_steptrim1
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/mp/s_steptrim1_blendr
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

textures/mp/s_trim4b
{
// q3map_surfacelight 750

// {

// map textures/doomgiver/trim4dglow

// blendFunc GL_ONE GL_ONE

// rgbGen const ( 0.000000 0.000000 1.000000 )

// }

	q3map_lightimage	textures/mp/s_trim4d_blend
	qer_editorimage	textures/doomgiver/trim4d
    {
        map $lightmap
    }
    {
        map textures/doomgiver/trim4d
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/mp/s_trim4_blend
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.25 0.1 0 10
    }
}

// Blue Flare

textures/mp/s_flare_blue
{
	qer_editorimage	textures/doomgiver/flare_blue
	surfaceparm	nonsolid
	surfaceparm	trans
	q3map_nolightmap
	deformvertexes	autoSprite	
    {
        map textures/doomgiver/flare_blue
        blendFunc GL_ONE GL_ONE
        rgbGen wave noise 1 0.2 0 25
    }
}

// ---------------------------------------------

// Wall Shaders

// ---------------------------------------------

// gray circle pattern

textures/mp/s_graywall
{
	qer_editorimage	textures/mp/s_graywall.tga
    {
        map $lightmap
        rgbGen identity
    }
    {
        map textures/mp/s_graywall
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

// gray circle pattern (phong)

textures/mp/s_graywall_hr
{
	q3map_shadeangle	90
	qer_editorimage	textures/mp/s_graywall_phg.tga
    {
        map $lightmap
        rgbGen identity
    }
    {
        map textures/mp/s_graywall
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

// dark gray circle pattern

textures/mp/s_graywall_dk
{
	qer_editorimage	textures/mp/s_graywall_dk.tga
    {
        map $lightmap
        rgbGen identity
    }
    {
        map textures/mp/s_graywall_dk
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

// dark gray circle pattern (phong)

textures/mp/s_graywall_dk_hr
{
	q3map_shadeangle	90
	qer_editorimage	textures/mp/s_graywall_dk_phg.tga
    {
        map $lightmap
        rgbGen identity
    }
    {
        map textures/mp/s_graywall_dk
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

// ---------------------------------------------

// Base Metal Walls

// ---------------------------------------------

textures/mp/s_metal_lg
{
// surfaceparm nolightmap

// q3map_vertexscale 1.0

// q3map_lightmapsamplesize 8

// q3map_surfacelight 10

// q3map_backsplash 100 5

// surfaceparm pointlight

// Q3map_novertexshadows - This is what keeps those caulk vis blockers from causing ugly shadows to form on your terrain.

	qer_editorimage	textures/mp/s_metal_lg.tga
    {
        map $lightmap
        rgbGen identity
    }
    {
        map textures/mp/s_metal_lg
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/mp/s_metal_lg_dk
{
	qer_editorimage	textures/mp/s_metal_lg_dk.tga
    {
        map $lightmap
        rgbGen identity
    }
    {
        map textures/mp/s_metal_lg_dk
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/mp/s_metal_lg_d2
{
// surfaceparm nolightmap

// q3map_vertexscale 1.0

// q3map_lightmapsamplesize 8

// q3map_surfacelight 10

// q3map_backsplash 100 5

// surfaceparm pointlight

// Q3map_novertexshadows - This is what keeps those caulk vis blockers from causing ugly shadows to form on your terrain.

	q3map_shadeangle	90
	qer_editorimage	textures/mp/s_metal_lg.tga
    {
        map $lightmap
        rgbGen identity
    }
    {
        map textures/mp/s_metal_lg
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/mp/s_metal_lg_dk_d2
{
	q3map_shadeangle	90
	qer_editorimage	textures/mp/s_metal_lg_dk.tga
    {
        map $lightmap
        rgbGen identity
    }
    {
        map textures/mp/s_metal_lg_dk
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/mp/s_fatalsky
{
// sun 0.630913 0.804796 1.000000 150 115 65

	q3map_lightimage	textures/skies/desert_up.tga
	qer_editorimage	textures/skies/fatal_dn.tga
	q3map_surfacelight	145
	q3map_lightsubdivide	1024
	sun 0.874291 1 0.819403 170 25 80
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	textures/skies/fatal 512 -
}

textures/mp/s_ground
{
	qer_editorimage	textures/yavin/ground
    {
        map $lightmap
    }
    {
        map textures/yavin/ground
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/mp/s_blue_flare
{
	qer_editorimage	textures/flares/flare_blue.tga
	surfaceparm	nomarks
	surfaceparm	trans
	q3map_nolightmap
	cull	twosided
	deformvertexes	autoSprite	
    {
        map textures/flares/flare_blue
        blendFunc GL_ONE GL_ONE
    }
}

textures/mp/s_red_flare
{
	qer_editorimage	textures/flares/flare_red.tga
	surfaceparm	nomarks
	surfaceparm	trans
	q3map_nolightmap
	cull	twosided
	deformvertexes	autoSprite	
    {
        map textures/flares/flare_red
        blendFunc GL_ONE GL_ONE
    }
}

textures/mp/s_flare
{
	qer_editorimage	textures/flares/standard_flare.tga
	surfaceparm	nomarks
	surfaceparm	trans
	q3map_nolightmap
	cull	twosided
	deformvertexes	autoSprite	
    {
        map textures/flares/standard_flare
        blendFunc GL_ONE GL_ONE
    }
}

textures/mp/s_rockwall
{
// q3map_forcemeta

	q3map_shadeangle	120
	qer_editorimage	textures/desert/t_rockwall1
	q3map_nonplanar
	q3map_clipmodel
	q3map_splotchfix
	q3map_onlyvertexlighting
    {
        map textures/desert/t_rockwall1
        rgbGen vertex
    }
}

textures/mp/s_rockwall_nophong
{
// q3map_forcemeta

// q3map_shadeangle 120

// q3map_onlyvertexlighting

	qer_editorimage	textures/desert/t_rockwall1
	q3map_nonplanar
	q3map_clipmodel
	q3map_splotchfix
    {
// rgbGen vertex

        map textures/desert/t_rockwall1
    }
}

// ---------------------------------------------

// Korriban

// ---------------------------------------------

// fogparms ( Red, Green, Blue=Color of opacity layer) Number.0=depth of transparency/opacity border.

textures/mp/s_kfog
{
// Ford_bluefog.tga

// fogparms	( 0.41 0.42 0.58 ) 6500.0

// cull	twosided

	qer_editorimage	textures
	qer_nocarve
	qer_trans	0.5
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0.45 0.35 0.23 ) 12800.0
}

textures/mp/s_kor_sky
{
// {

// map textures/skies/cloudlayer2

// blendFunc GL_DST_ALPHA GL_SRC_COLOR

// tcMod scroll 0.005 0

// }

// {

// map textures/skies/cloudlayer3

// blendFunc GL_SRC_ALPHA GL_SRC_COLOR

// tcMod scroll 0.01 0

// }

	q3map_lightimage	textures/skies/b_up.tga
	qer_editorimage	textures/fogs/fog.tga
	q3map_surfacelight	115
	q3map_lightsubdivide	512
	sun 0.766119 0.79123 1 175 75 70
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	textures/skies/b 512 -
}

textures/mp/s_ylight
{
	q3map_lightimage	textures/mp/ylight_blend.tga
	qer_editorimage	textures/mp/ylight.tga
	q3map_surfacelight	3000
    {
        map $lightmap
    }
    {
        map textures/mp/ylight
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/mp/ylight_blend
        blendFunc GL_DST_COLOR GL_ONE
    }
}

textures/mp/s_ylight_blue
{
	q3map_lightimage	textures/mp/ylight_blue_blend.tga
	qer_editorimage	textures/mp/ylight_blue.tga
	q3map_surfacelight	3000
    {
        map $lightmap
    }
    {
        map textures/mp/ylight_blue
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/mp/ylight_blue_blend
        blendFunc GL_DST_COLOR GL_ONE
    }
}

textures/mp/s_ylight_red
{
	q3map_lightimage	textures/mp/ylight_blue_red.tga
	qer_editorimage	textures/mp/ylight_red.tga
	q3map_surfacelight	3000
    {
        map $lightmap
    }
    {
        map textures/mp/ylight_red
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/mp/ylight_red_blend
        blendFunc GL_DST_COLOR GL_ONE
    }
}

textures/mp/s_kor_fog
{
// fogparms	( 0.984314 0.576471 0.4 ) 2048.0

// fogparms	( 1.000000 0.728909 0.624151 ) 2048.0

// fogparms	( 0.98 0.98 0.98 ) 1024.0

// fogparms	( 1.000000 0.801246 0.552090 ) 512.0

	qer_editorimage	textures/fogs/fog
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0.25 0.201246 0.11509 ) 512.0
}

textures/mp/s_kor_fog2
{
// fogparms	( 0.984314 0.576471 0.4 ) 2048.0

// fogparms	( 1.000000 0.728909 0.624151 ) 2048.0

// fogparms	( 0.98 0.98 0.98 ) 1024.0

// fogparms	( 1.000000 0.801246 0.552090 ) 512.0

	qer_editorimage	textures/fogs/fog
	q3map_surfacelight	250
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0.73 0.52 0.43 ) 256.0
}

textures/mp/kor_outside
{
// q3map_shadeangle	90

	qer_editorimage	textures/quicktrip/rock_sandstone.tga
    {
        map $lightmap
        rgbGen identity
    }
    {
        map textures/quicktrip/rock_sandstone
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

// ---------------------------------------------

// Bespin (in progress)

// ---------------------------------------------

textures/mp/s_bespin
{
	q3map_lightimage	textures/mp/li_bespin.tga
	qer_editorimage	textures/mp/li_bespin.tga
	q3map_surfacelight	50
	q3map_lightsubdivide	512
	sun 1 0.728909 0.624151 60 50 60
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	textures/skies/bespin 512 -
}

textures/mp/s_bespin_fog
{
// fogparms	( 0.984314 0.576471 0.4 ) 2048.0

// fogparms	( 1.000000 0.728909 0.624151 ) 2048.0

// fogparms	( 0.98 0.98 0.98 ) 1024.0

// fogparms	( 1.000000 0.801246 0.552090 ) 1024.0

	qer_editorimage	textures/fogs/fog
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0.25 0.201246 0.11509 ) 1024.0
}

textures/mp/s_bespin_white
{
	qer_editorimage	textures/mp/white.tga
	surfaceparm	noimpact
	surfaceparm	nomarks
	q3map_nolightmap
    {
        map textures/mp/white
        rgbGen identity
    }
}

textures/mp/s_bespin_brown
{
	qer_editorimage	textures/mp/brown.tga
	surfaceparm	noimpact
	surfaceparm	nomarks
	q3map_nolightmap
    {
        map textures/mp/black
        rgbGen identity
    }
}

// ---------------------------------------------

// VJUN/BYSS THEME

// ---------------------------------------------

textures/mp/s_vjunsky
{
// R G B intensity degrees elevation

// q3map_lightimage textures/skies/dos_up.tga

// q3map_surfacelight 	145

// sun 1 0.85 0.5 150 300 80

// skyParms	textures/skies/dos 512 -

// sun 1 0.85 0.5 120 90 90

// skyParms	textures/skies/byss 512 -

	qer_editorimage	textures/skies/sky.tga
	q3map_lightsubdivide	512
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	textures/skies/cs1 512 -
}

textures/mp/s_vjun_fog
{
	qer_editorimage	textures/fogs/fog
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0.25 0.201246 0.11509 ) 512.0
}

textures/mp/s_vjun_light_strip
{
	q3map_lightimage	textures/vjun/light_strip_glw.tga
	qer_editorimage	textures/vjun/light_strip.tga
	q3map_surfacelight	3000
    {
        map $lightmap
    }
    {
        map textures/vjun/light_strip
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
// rgbGen identity

        map textures/vjun/light_strip_glw
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.6 0.05 0 1
    }
}

textures/mp/s_vjun_light_strip2
{
	q3map_lightimage	textures/mp/s_light_strip2_glw.tga
	qer_editorimage	textures/mp/s_light_strip2.tga
	q3map_surfacelight	1500
    {
        map $lightmap
    }
    {
        map textures/mp/s_light_strip2
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
// rgbGen identity

        map textures/mp/s_light_strip2_glw
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.6 0.05 0 1
    }
}

textures/mp/s_vjun_light_vjun
{
	q3map_lightimage	textures/vjun/light_vjun.tga
	qer_editorimage	textures/vjun/light_vjun.tga
	q3map_surfacelight	5000
    {
        map $lightmap
    }
    {
        map textures/vjun/light_vjun
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/mp/light_vjun_blend
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

textures/mp/s_vjun_light_vjuny
{
	q3map_lightimage	textures/mp/s_light_vjuny.tga
	qer_editorimage	textures/mp/s_light_vjuny.tga
	q3map_surfacelight	5000
    {
        map $lightmap
    }
    {
        map textures/mp/s_light_vjuny
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/mp/s_light_vjuny_blend
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

textures/mp/s_vjun_u_light02
{
	q3map_lightimage	textures/bespin/u_light02glow.tga
	qer_editorimage	textures/bespin/u_light02.tga
	q3map_surfacelight	2500
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

textures/mp/s_vjun_grate_light
{
	q3map_lightimage	textures/rocky_ruins/grate_light_glw.tga
	qer_editorimage	textures/rocky_ruins/grate_light.tga
	q3map_surfacelight	2500
    {
        map $lightmap
    }
    {
        map textures/rocky_ruins/grate_light
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/rocky_ruins/grate_light_glw
        blendFunc GL_ONE GL_ONE
    }
}

textures/mp/s_vjun_lights
{
	q3map_lightimage	textures/mp/s_lights_blend.tga
	qer_editorimage	textures/vjun/lights.tga
	q3map_surfacelight	2500
    {
        map $lightmap
    }
    {
        map textures/vjun/lights
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/mp/s_lights_blend
        blendFunc GL_ONE GL_ONE
    }
}

textures/mp/s_wall17
{
    {
// blendFunc GL_SRC_ALPHA GL_ONE

// tcmod scale .25 .25

        map textures/byss/env_large_floor2
        rgbGen identity
        tcGen environment
    }
    {
// map textures/mp/s_metalc.tga

        map textures/mp/s_wall17
        blendFunc GL_ONE_MINUS_SRC_ALPHA GL_SRC_ALPHA
        rgbGen identity
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

// ---------------------------------------------

// Flares

// ---------------------------------------------

textures/mp/s_flare_big
{
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
	cull	twosided
    {
        map textures/flares/standard_flare_bigger
        blendFunc GL_ONE GL_ONE
        rgbGen const ( 0.752941 0.752941 0.752941 )
        alphaGen portal 70
    }
}

textures/mp/s_flare_yellow
{
	qer_editorimage	textures/flares/standard_flare
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
	cull	twosided
    {
        map textures/flares/standard_flare
        blendFunc GL_ONE GL_ONE
        depthFunc disable
        rgbGen const ( 1.000000 1.000000 0.368627 )
    }
}

// ---------------------------------------------

// RIFT THEME

// ---------------------------------------------

textures/mp/s_rift_fog
{
	qer_editorimage	textures/fogs/fog.tga
	qer_nocarve
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0 0.0901961 0.184314 ) 6277.0
}

textures/mp/s_blacksky
{
	qer_editorimage	textures/mp/black.tga
	q3map_surfacelight	25
	q3map_lightsubdivide	512
	sun 1 1 1 32 90 90
	surfaceparm	noimpact
	q3map_nolightmap
    {
        map textures/mp/black
    }
}

textures/mp/s_rift_newcolumn
{
	q3map_lightimage	textures/rift/newcolumn_glow
	qer_editorimage	textures/rift/newcolumn.tga
	q3map_surfacelight	50
    {
        map $lightmap
    }
    {
        map textures/rift/newcolumn
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/rift/newcolumn_glow
        blendFunc GL_ONE GL_ONE
    }
}

// ---------------------------------------------

// Bounty Arena

// ---------------------------------------------

textures/mp/s_bountysky
{
// q3map_sun <red> <green> <blue> <intensity> <degrees> <elevation>

// skyParms textures/skies/nar 512 -

	q3map_lightimage	textures/skies/nar_up.tga
	qer_editorimage	textures/fogs/fog.tga
	q3map_surfacelight	115
	q3map_lightsubdivide	512
	sun 0.912072 1 0.646675 140 270 60
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	textures/skies/korb 512 -
}

// ---------------------------------------------

// Hoth Arena

// ---------------------------------------------

textures/mp/s_hothsky
{
// q3map_sun <red> <green> <blue> <intensity> <degrees> <elevation>

// skyParms textures/skies/hoth 512 -

// skyParms textures/skies/yavin 512 -

	q3map_lightimage	textures/skies/hoth_up.tga
	qer_editorimage	textures/fogs/fog.tga
	q3map_surfacelight	145
	q3map_lightsubdivide	512
	sun 0.8 0.8 0.95 160 140 80
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	textures/skies/hevil 512 -
}

textures/mp/s_hothpit_fog
{
	qer_editorimage	textures/fogs/fog
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0.35 0.35 0.35 ) 1024.0
}

textures/mp/s_hoth_fog
{
	qer_editorimage	textures/fogs/fog.tga
	qer_nocarve
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0.5 0.5 0.54 ) 8096.0
}

// q3map_lightmapsamplesize

textures/mp/s_cliffwall1
{
	q3map_shadeangle	90
	qer_editorimage	textures/yavin/rock3.tga
	q3map_nonplanar
	q3map_splotchfix
	q3map_forcemeta
    {
        map $lightmap
        rgbGen identity
    }
    {
        map textures/yavin/rock3
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/mp/s_cliffwall1_nophong
{
	qer_editorimage	textures/yavin/rock3.tga
	q3map_nonplanar
	q3map_splotchfix
	q3map_forcemeta
    {
        map $lightmap
        rgbGen identity
    }
    {
        map textures/yavin/rock3
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/mp/s_cliffwall1top
{
	q3map_shadeangle	160
	qer_editorimage	textures/mp/h_clifftrim.tga
	q3map_nonplanar
	q3map_splotchfix
	q3map_forcemeta
    {
        map $lightmap
        rgbGen identity
    }
    {
        map textures/mp/h_clifftrim
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/mp/s_cliffwall1top_nophong
{
	qer_editorimage	textures/mp/h_clifftrim.tga
	q3map_nonplanar
	q3map_splotchfix
	q3map_forcemeta
    {
        map $lightmap
        rgbGen identity
    }
    {
        map textures/mp/h_clifftrim
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/mp/s_outground_phong
{
	q3map_bounce	2
	q3map_shadeangle	90
	qer_editorimage	textures/hoth/h_outground
	q3map_nonplanar
	q3map_splotchfix
    {
        map $lightmap
        rgbGen identity
    }
    {
        map textures/hoth/h_outground
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/mp/s_outground_nophong
{
	q3map_bounce	2
	qer_editorimage	textures/hoth/h_outground
	q3map_nonplanar
	q3map_splotchfix
    {
        map $lightmap
        rgbGen identity
    }
    {
        map textures/hoth/h_outground
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/mp/s_outground_3_phong
{
// q3map_nolightmap

// {

// map textures/hoth/h_outground_3

// rgbGen vertex

// }

	q3map_shadeangle	90
	qer_editorimage	textures/hoth/h_outground_3
	q3map_nonplanar
	q3map_splotchfix
    {
        map $lightmap
        rgbGen identity
    }
    {
        map textures/hoth/h_outground_3
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/mp/s_outground_3_nophong
{
	qer_editorimage	textures/hoth/h_outground_3
	q3map_nonplanar
	q3map_splotchfix
    {
        map $lightmap
        rgbGen identity
    }
    {
        map textures/hoth/h_outground_3
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

// ---------------------------------------------

// H_Evil + Factory Arena

// ---------------------------------------------

textures/mp/s_evilsky
{
// q3map_sun <red> <green> <blue> <intensity> <degrees> <elevation>

	q3map_lightimage	textures/skies/hoth_up.tga
	qer_editorimage	textures/fogs/fog.tga
	q3map_surfacelight	145
	q3map_lightsubdivide	256
	sun 0.9 0.8 0.8 160 35 80
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	textures/skies/hevil 512 -
}

textures/mp/s_wall2_nophong
{
// q3map_nonplanar

	qer_editorimage	textures/quicktrip/qt_wall2
    {
        map $lightmap
        rgbGen identity
    }
    {
        map textures/quicktrip/qt_wall2
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/mp/s_duel2sky
{
// q3map_sun <red> <green> <blue> <intensity> <degrees> <elevation>

// skyParms textures/skies/hoth 512 -

// skyParms textures/skies/yavin 512 -

	q3map_lightimage	textures/skies/hoth_up.tga
	qer_editorimage	textures/fogs/fog.tga
	q3map_surfacelight	130
	q3map_lightsubdivide	256
	sun 0.8 0.8 0.95 160 170 70
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	textures/skies/hevil 512 -
}

textures/mp/desert_03
{
	q3map_lightimage	textures/skies/sky.tga
	qer_editorimage	textures/skies/sky.tga
	q3map_surfacelight	140
	q3map_lightsubdivide	512
	sun 0.905458 0.97021 1 125 345 85
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	q3map_nolightmap
	skyParms	textures/skies/desert 512 -
}

// ---------------------------------------------

// Rooftop

// ---------------------------------------------

textures/mp/s_thinlight
{
// q3map_lightsubdivide

	q3map_lightimage	textures/rail/thinlight
	qer_editorimage	textures/rail/thinlight
	q3map_surfacelight	350
    {
        map $lightmap
    }
    {
        map textures/rail/thinlight
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/rail/thinlight_glow1
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.5 0.05 0 0.3
    }
    {
        map textures/rail/thinlight_glow2
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.8 0.05 0 0.3
    }
}

textures/mp/s_thinlight_r
{
// q3map_lightsubdivide

	q3map_lightimage	textures/rail/thinlight
	qer_editorimage	textures/rail/thinlight
	q3map_surfacelight	350
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/rail/thinlight
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/rail/thinlight_glow1
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.5 0.05 0 0.3
    }
    {
        map textures/rail/thinlight_glow2
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.8 0.05 0 0.3
    }
}

textures/mp/roofog
{
	qer_editorimage	textures/fogs/fog.tga
	qer_nocarve
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0 0.141176 0.184314 ) 5000.0
}

textures/mp/s_building_ext2test_r
{
	qer_editorimage	textures/rooftop/building_ext2test
    {
        map $lightmap
    }
    {
        map textures/rooftop/building_ext2test
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/mp/s_building_ext2test_glwr
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
    {
        map textures/mp/s_building_ext2test_glw2r
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

// ---------------------------------------------

// Track Lights (used in consession)

// ---------------------------------------------

// Small Single Track - Vertical

textures/mp/s_tracklight_nolight
{
// final

	qer_editorimage	textures/mp/s_tracklight.tga
	q3map_surfacelight	1000
	q3map_lightsubdivide	64

    {
        map $lightmap
    }
    {
        map textures/mp/s_tracklight
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/mp/s_tracklight_blend
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.8 0.1 0 0.5
    }
}

textures/mp/s_ylight_nolight
{
	q3map_lightimage	textures/mp/ylight_blue_red.tga
	qer_editorimage	textures/mp/ylight_red.tga
    {
        map $lightmap
    }
    {
        map textures/mp/ylight_red
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/mp/ylight_red_blend
        blendFunc GL_DST_COLOR GL_ONE
        glow
    }
}

