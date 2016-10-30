// skyparms work like this:

// q3map_sun <red> <green> <blue> <intensity> <degrees> <elevation>

// color will be normalized, so it doesn't matter what range you use

// intensity falls off with angle but not distance 100 is a fairly bright sun

// degree of 0 = from the east, 90 = north, etc.  altitude of 0 = sunrise/set, 90 = noon

textures/skies/normallight
{
	qer_editorimage	textures/skies/sky.tga
	q3map_surfacelight	75
	q3map_lightsubdivide	512
	sun 0.75 0.79 1 250 0 65
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	q3map_nolightmap
	skyParms	textures/skies/nebula2 512 -
}

textures/skies/ships
{
	qer_editorimage	textures/skies/sky.tga
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	q3map_nolightmap
	skyParms	textures/skies/nebula2 512 -
}

textures/skies/bluelight
{
	lightcolor	( 0 0 1 )
	qer_editorimage	textures/skies/sky.tga
	q3map_surfacelight	35
	sun 0.75 0.79 1 130 0 40
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	q3map_nolightmap
	skyParms	textures/skies/nebula2 512 -
}

textures/skies/orangelight
{
	lightcolor	( 1 0.501961 0 )
	qer_editorimage	textures/skies/sky.tga
	q3map_surfacelight	200
	q3map_lightsubdivide	512
	sun 0.75 0.79 1 250 40 60
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	q3map_nolightmap
	skyParms	textures/skies/nebula2 512 -
}

textures/skies/test
{
	qer_editorimage	textures/skies/sky.tga
	q3map_surfacelight	75
	q3map_lightsubdivide	512
	sun 0.75 0.79 1 50 0 65
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks

	q3map_nolightmap
	skyParms	textures/skies/test 512 -
}

textures/skies/test2
{
	qer_editorimage	textures/skies/sky.tga
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	textures/skies/desert 512 -
}

textures/skies/bespin
{
	qer_editorimage	textures/skies/sky.tga
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	textures/skies/bespin 512 -
}

// skyparms work like this:

// q3map_sun <red> <green> <blue> <intensity> <degrees> <elevation>

// color will be normalized, so it doesn't matter what range you use

// intensity falls off with angle but not distance 100 is a fairly bright sun

// degree of 0 = from the east, 90 = north, etc.  altitude of 0 = sunrise/set, 90 = noon

textures/skies/test3
{
	qer_editorimage	textures/skies/sky.tga
	q3map_surfacelight	15
	q3map_lightsubdivide	512
	sun 0.75 0.79 1 65 46 40
	surfaceparm	sky
	surfaceparm	noimpact

	surfaceparm	nonsolid
	surfaceparm	nonopaque
	q3map_nolightmap
	skyParms	textures/skies/bespin 512 -
}

// skyparms work like this:

// q3map_sun <red> <green> <blue> <intensity> <degrees> <elevation>

// color will be normalized, so it doesn't matter what range you use

// intensity falls off with angle but not distance 100 is a fairly bright sun

// degree of 0 = from the east, 90 = north, etc.  altitude of 0 = sunrise/set, 90 = noon

textures/skies/right_light
{
	qer_editorimage	textures/skies/sky.tga
	q3map_surfacelight	15
	q3map_lightsubdivide	512
	sun 0.75 0.79 1 65 180 65
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	q3map_nolightmap
	skyParms	textures/skies/bespin 512 -
}

// skyparms work like this:

// q3map_sun <red> <green> <blue> <intensity> <degrees> <elevation>

// color will be normalized, so it doesn't matter what range you use

// intensity falls off with angle but not distance 100 is a fairly bright sun

// degree of 0 = from the east, 90 = north, etc.  altitude of 0 = sunrise/set, 90 = noon

textures/skies/test4
{
	qer_editorimage	textures/skies/sky.tga
	q3map_surfacelight	15
	q3map_lightsubdivide	512
	sun 1 0.627451 0.258824 100 46 40
	surfaceparm	sky
	surfaceparm	noimpact

	surfaceparm	nonsolid
	surfaceparm	nonopaque
	q3map_nolightmap
	skyParms	textures/skies/bespin 512 -
}

// skyparms work like this:

// q3map_sun <red> <green> <blue> <intensity> <degrees> <elevation>

// color will be normalized, so it doesn't matter what range you use

// intensity falls off with angle but not distance 100 is a fairly bright sun

// degree of 0 = from the east, 90 = north, etc.  altitude of 0 = sunrise/set, 90 = noon

textures/skies/test5
{
	qer_editorimage	textures/skies/sky.tga
	q3map_surfacelight	100
	q3map_lightsubdivide	512
	sun 1 0.843137 0.54902 130 0 90
	surfaceparm	sky
	surfaceparm	noimpact

	surfaceparm	nonsolid
	surfaceparm	nonopaque
	q3map_nolightmap
	skyParms	textures/skies/bespin 512 -
}

textures/skies/kejim
{
	qer_editorimage	textures/skies/sky.tga
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks

	notc
	q3map_nolightmap
	skyParms	textures/skies/desert2 512 -
}

textures/skies/kejim_light
{
	qer_editorimage	textures/skies/sky.tga
	q3map_surfacelight	10
	sun 1 1 1 200 0 70
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	textures/skies/desert2 512 -
}

textures/skies/new_test
{
	qer_editorimage	textures/skies/sky.tga
	q3map_surfacelight	80
	sun 0.75 0.79 1 130 5 40
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	q3map_novertexshadows
	skyParms	textures/skies/desert2 512 -
}

// skyparms work like this:

// q3map_sun <red> <green> <blue> <intensity> <degrees> <elevation>

// color will be normalized, so it doesn't matter what range you use

// intensity falls off with angle but not distance 100 is a fairly bright sun

// degree of 0 = from the east, 90 = north, etc.  altitude of 0 = sunrise/set, 90 = noon

textures/skies/artus_light
{
	qer_editorimage	textures/skies/sky.tga
	q3map_surfacelight	75
	q3map_lightsubdivide	512
	sun 0.75 0.79 1 250 0 65
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	q3map_nolightmap
	skyParms	textures/skies/desert 512 -
}

textures/skies/yavin
{
	qer_editorimage	textures/skies/sky.tga
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	textures/skies/yavin 512 -
}

textures/skies/space
{
	qer_editorimage	textures/skies/space_bk
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks

	notc
	q3map_nolightmap
	skyParms	textures/skies/space 512 -
}

textures/skies/ice
{
	qer_editorimage	textures/skies/sky.tga
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	textures/skies/ice 512 -
}

textures/skies/col10
{
	qer_editorimage	textures/skies/sky.tga
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks

	notc
	q3map_nolightmap
	skyParms	textures/skies/col10 512 -
}

textures/skies/dm_col1
{
	qer_editorimage	textures/skies/sky.tga
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks

	notc
	q3map_nolightmap
	skyParms	textures/skies/col10 512 -
}

textures/skies/t1_fatal
{
	qer_editorimage	textures/skies/t1_fatal
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	textures/skies/fatal 512 -
}

textures/skies/yavin_nodraw
{
	qer_editorimage	textures/skies/sky.tga
	sun 0.933333 1 0.8 0 0 0
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	q3map_nolightmap
	skyParms	- 512 -
}

textures/skies/desert
{
	qer_editorimage	textures/skies/sky.tga
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	textures/skies/desert 512 -
}

textures/skies/hevil
{
	q3map_lightimage	textures/colors/white
	qer_editorimage	textures/skies/sky.tga
	q3map_surfacelight	800
	q3map_lightsubdivide	512
	sun 1 1 1 400 300 50
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	textures/skies/hevil 128 -
    {
        map textures/skies/cloudlayer2
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        tcMod scroll 0 0.00225
        tcMod scale 4 4
    }
}

textures/skies/cloudlayer2
{
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
    {
        map textures/skies/cloudlayer2
        blendFunc GL_DST_ALPHA GL_SRC_COLOR
        tcMod scroll 0.005 0
    }
    {
        map textures/skies/cloudlayer3
        blendFunc GL_SRC_ALPHA GL_SRC_COLOR
        tcMod scroll 0.01 0
    }
}


textures/skies/hoth
{
// sun 0.827451 0.92549 0.996078 205 345 75

// R G B intensity degrees elevation

	qer_editorimage	textures/skies/sky.tga
	q3map_lightsubdivide	512
	sun 255 255 255 355 345 85
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	q3map_nolightmap
	skyParms	- 512 -
}

textures/skies/Wedge_sky
{
// sun 0.827451 0.92549 0.996078 205 345 75

// R G B intensity degrees elevation

	qer_editorimage	textures/skies/Wedge_sky
	q3map_lightsubdivide	512
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	q3map_nolightmap
	skyParms	textures/skies/wedge 512 -
}


textures/skies/siege_1
{
// sun 0.827451 0.92549 0.996078 205 345 75

// R G B intensity degrees elevation

	qer_editorimage	textures/skies/sky.tga
	q3map_lightsubdivide	512
	sun 255 255 255 355 345 85
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	q3map_nolightmap
	skyParms	- 512 -
}

textures/skies/siege_2
{
// sun 0.827451 0.92549 0.996078 205 345 75

// R G B intensity degrees elevation

// q3map_lightsubdivide	512

// sun 255 255 253 355 345 85

	qer_editorimage	textures/skies/sky.tga
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	q3map_nolightmap
	skyParms	- 512 -
}

// SKYBOXES

// q3map_sun <red> <green> <blue> <intensity> <degrees> <elevation>

// color will be normalized, so it doesn't matter what range you use

// intensity falls off with angle but not distance 100 is a fairly bright sun

// degree of 0 = from the east, 90 = north, etc.  altitude of 0 = sunrise/set, 90 = noon

textures/skies/T1_Fatal_skybox
{
	qer_editorimage	textures/skies/T1_Fatal.tga
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	q3map_nolightmap
	skyParms	env/t1_fatal/fatal 512 -
}

textures/skies/hevil_2
{
	q3map_lightimage	textures/colors/white
	qer_editorimage	textures/skies/sky.tga
	q3map_surfacelight	800
	q3map_lightsubdivide	512
	sun 1 1 1 400 300 50
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	textures/skies/hevil 128 -
}

textures/skies/cloudlayer_test
{
	qer_editorimage	textures/skies/cloudlayer2
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
    {
        map textures/skies/cloudlayer2
        blendFunc GL_ONE_MINUS_SRC_ALPHA GL_ONE
        rgbGen const ( 0.501961 0.501961 0.501961 )
        tcMod scroll 0.05 0
        tcMod turb 1 0.1 0.1 0.1
    }
    {
        map textures/skies/alphasquare
        blendFunc GL_ZERO GL_SRC_COLOR
    }
}

// Test sky

// q3map_sun <red> <green> <blue> <intensity> <degrees> <elevation>

// intensity falls off with angle but not distance 100 is a fairly bright sun

// degree of 0 = from the east, 90 = north, etc.  altitude of 0 = sunrise/set, 90 = noon

textures/skies/boba_night
{
// q3map_surfacelight	220

// sun 0.219608 0.176471 0.635294 220 75 75

	qer_editorimage	textures/skies/sky.tga
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	q3map_novertexshadows
	skyParms	textures/skies/b 512 -
}

textures/skies/boba_late_afternoon
{
// q3map_surfacelight	220

// sun 0.219608 0.176471 0.635294 220 75 75

	qer_editorimage	textures/skies/sky.tga
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	q3map_novertexshadows
	skyParms	textures/skies/bespin 512 -
}

textures/skies/boba_afternoon
{
// q3map_surfacelight	220

// sun 0.219608 0.176471 0.635294 220 75 75

	qer_editorimage	textures/skies/sky.tga
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	q3map_novertexshadows
	skyParms	textures/skies/yavin 512 -
}

textures/skies/test_korriban1
{
	q3map_lightimage	textures/colors/white
	qer_editorimage	textures/skies/sky.tga
	q3map_surfacelight	50
	q3map_lightsubdivide	512
	sun 1 1 1 450 15 60
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	textures/skies/hevil 128 -
}

textures/skies/rail_sun
{
// sun 0.827451 0.92549 0.996078 205 345 75

// R G B intensity degrees elevation

	qer_editorimage	textures/skies/sky.tga
	q3map_lightsubdivide	512
	sun 1 1 1 355 345 85
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	q3map_nolightmap
	skyParms	- 512 -
}

textures/skies/korr2
{
	qer_editorimage	textures/skies/sky.tga
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	textures/skies/desert 512 -
}

textures/skies/cloudlayer_yavin
{
	qer_editorimage	textures/skies/cloudlayer2
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
    {
        map textures/skies/cloudlayer2
        blendFunc GL_ONE_MINUS_SRC_ALPHA GL_ONE
        rgbGen const ( 0.501961 0.501961 0.501961 )
        tcMod scroll 0.01 0
        tcMod turb 1 0.01 0.1 0.1
    }
    {
        map textures/skies/alphasquare
        blendFunc GL_ZERO GL_SRC_COLOR
    }
}

textures/skies/rogue_test
{
	qer_editorimage	textures/skies/sky.tga
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	q3map_nolightmap
	skyParms	textures/skies/nebula2 512 -
}

textures/skies/t2_dpred
{
	qer_editorimage	textures/skies/sky.tga
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	textures/skies/dosuun 512 -
}

textures/skies/dune
{
	qer_editorimage	textures/skies/sky.tga
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	textures/skies/dune 512 -
}

textures/skies/NarKreeta
{
	qer_editorimage	textures/skies/sky.tga
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	textures/skies/korb 512 -
}

textures/skies/korriban
{
	qer_editorimage	textures/skies/sky.tga
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	textures/skies/dos 512 -
}

textures/skies/Dosuun
{
	qer_editorimage	textures/skies/sky.tga
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	textures/skies/dos 512 -
}

textures/skies/Coruscant
{
	qer_editorimage	textures/skies/sky.tga
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	textures/skies/cs1 512 -
}

textures/skies/byss
{
	qer_editorimage	textures/skies/sky.tga
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	textures/skies/byss 512 -
}

textures/skies/byss_mp
{
	qer_editorimage	textures/skies/sky.tga
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	textures/skies/byss 512 -
}

textures/skies/cloudlayer_surprise
{
	qer_editorimage	textures/skies/cloudlayer_t1_surprise
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
    {
        map textures/skies/cloudlayer_t1_surprise
        blendFunc GL_SRC_ALPHA GL_ONE
        rgbGen const ( 0.501961 0.501961 0.501961 )
        tcMod scroll 0.01 0
        tcMod turb 1 0.01 0.1 0.1
    }
}

textures/skies/vjun
{
	qer_editorimage	textures/skies/sky.tga
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	textures/skies/vu 512 -
}

textures/skies/ct
{
	qer_editorimage	textures/skies/sky.tga
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	textures/skies/ct 512 -
}

textures/skies/sky_nodraw
{
	qer_editorimage	textures/skies/sky.tga
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	q3map_nolightmap
	skyParms	textures/system/caulk 512 -
}

