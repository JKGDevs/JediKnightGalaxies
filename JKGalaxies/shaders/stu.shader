textures/stu/kor
{
	qer_editorimage	textures/fogs/fog.tga
	qer_nocarve
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	q3map_onlyvertexlighting
	fogparms	( 1 1 0.882353 ) 6400.0
	cull	disable
}

textures/stu/rift
{
	qer_editorimage	textures/fogs/fog.tga
	qer_nocarve
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0 0.0901961 0.184314 ) 6977.0
}

textures/stu/stutest_fog
{
	qer_editorimage	textures/fogs/fog.tga
	qer_nocarve
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0.95 0.97 0.77 ) 7000.0
	cull	disable
}

textures/stu/stutest_fog2
{
	qer_editorimage	textures/fogs/fog.tga
	qer_nocarve
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 1.0 0.87 0.87 ) 5577.0
	cull	disable
}

textures/stu/nodrop_trigger
{
	qer_editorimage	textures/system/trigger
	qer_nocarve
	qer_trans	0.5
	surfaceparm	nomarks
	surfaceparm	nodraw
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trigger
	surfaceparm nodrop
	surfaceparm	trans
	q3map_nolightmap
}

textures/stu/black_noimpact
{
	qer_editorimage	textures/colors/black.tga
	surfaceparm	noimpact
	surfaceparm	nomarks
	q3map_nolightmap
    {
        map $whiteimage
        rgbGen const ( 0.000000 0.000000 0.000000 )
    }
}

textures/stu/kor_sun
{
	qer_editorimage	textures/skies/sky.tga
	q3map_surfacelight	0
	q3map_lightsubdivide	512
	//sun 0.827451 0.92549 0.996078 205 345 75
	//R G B intensity degrees elevation
	sun 255 255 255 355 345 85
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	q3map_nolightmap
	skyParms	- 512 -
}

