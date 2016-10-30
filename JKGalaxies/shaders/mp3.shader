// ---------------------------------------------
// Casey's Threewave JEDI Apprentice Shaders
// ---------------------------------------------
//
// q3map_sun <red> <green> <blue> <intensity> <degrees> <elevation>
// degree of 0 = from the east
// degree of 90 = north
// altitude of 0 = sunrise/set, 90 = noon

textures/mp/hoth_ctf_sky_top
{
// R G B intensity degrees elevation

	qer_editorimage	textures/skies/sky.tga
	q3map_lightimage textures/colors/white.tga
	q3map_lightsubdivide 1024
	q3map_surfacelight 1500
	sun 1.0 1.0 1.0 100 345 85
	surfaceparm sky
	surfaceparm noimpact
	surfaceparm nomarks
	q3map_nolightmap
	skyParms - 512 -
}

textures/mp/hoth_ctf_sky_sides
{
// R G B intensity degrees elevation

	qer_editorimage	textures/skies/sky.tga
//	q3map_lightimage textures/colors/white.tga
	q3map_lightsubdivide 1024
	q3map_surfacelight 3000
//	sun 255 255 255 355 345 85
	sun 255 255 255 0 345 85
	surfaceparm sky
	surfaceparm noimpact
	surfaceparm nomarks
	q3map_nolightmap
	skyParms - 512 -
}

textures/mp/hoth_ctf_fog
{
// fogparms ( Red, Green, Blue=Color of opacity layer) Number.0=depth of transparency/opacity border.

	qer_editorimage	textures/fogs/Hoth2_bluefog
	qer_nocarve
	qer_trans 0.5
	surfaceparm nonsolid
	surfaceparm nonopaque
	surfaceparm fog
	surfaceparm trans
	q3map_nolightmap
	fogparms ( 0.752941 0.752941 0.752941 ) 3800.0
	cull twosided
	deformvertexes	move	0 0 0 sin 0 1 0 1
}

textures/mp/taspir_sky1
{
// sun 0.827451 0.92549 0.996078 205 345 75
// R G B intensity degrees elevation
// sun 1 0.85 0.5 400 300 70

	qer_editorimage	textures/skies/sky.tga
	surfaceparm sky
	surfaceparm noimpact
	surfaceparm nomarks
	q3map_lightimage textures/colors/white.tga
	q3map_nolightmap
	q3map_surfacelight 350
	q3map_lightsubdivide 4096
	skyParms - 512 -
}

textures/mp/taspir_sky1b
{
// sun 0.827451 0.92549 0.996078 205 345 75
// R G B intensity degrees elevation
// sun 1 0.85 0.5 400 300 70

	qer_editorimage	textures/skies/sky.tga
	surfaceparm sky
	surfaceparm noimpact
	surfaceparm nomarks
	q3map_lightimage textures/colors/white.tga
	q3map_nolightmap
	q3map_surfacelight 350
	q3map_lightsubdivide 4096
	skyParms - 512 -
}

textures/mp/taspir_fog1
{
// (  Red          Green        Blue     ) Distance

	qer_editorimage	textures/colors/white.tga
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0.8 0.7 0.4 ) 5000.0
}

textures/mp/desert_03
{
	qer_editorimage textures/skies/sky.tga
	q3map_lightimage textures/skies/sky.tga
	q3map_lightsubdivide 1024
	q3map_surfacelight 800
	q3map_nolightmap
	surfaceparm sky
	surfaceparm noimpact
	surfaceparm nomarks
	sun 255 255 255 80 345 85
	skyParms textures/skies/desert 512 -
}

textures/mp/light_01_b
{
	qer_editorimage textures/mp/light_01b
    {
	map $lightmap
    }
    {
        map textures/mp/light_01b
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/mp/light_01b_glw
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

textures/mp/u_light02_1000
{
	qer_editorimage textures/bespin/u_light02
	q3map_surfacelight 1000
	q3map_lightimage textures/bespin/u_light02glow
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

textures/mp/caulk_hull
{
	qer_editorimage	textures/mp/caulk_hull
	surfaceparm	nomarks
	surfaceparm	nodraw
	q3map_nolightmap
}

textures/mp/fast_black
{
	qer_editorimage textures/colors/black
	q3map_nolightmap
	surfaceparm noimpact
	surfaceparm nomarks

	{
		map textures/colors/black
	}
}

textures/mp/fast_white
{
	qer_editorimage textures/colors/white
	q3map_nolightmap
	surfaceparm noimpact
	surfaceparm nomarks

	{
		map textures/colors/black
	}
}

textures/mp/h_basicwall_vert
{
	qer_editorimage textures/hoth/h_basicwall_2
	q3map_nolightmap
	surfaceparm noimpact
	surfaceparm nomarks

	{
		map textures/hoth/h_basicwall_2
	}
}




