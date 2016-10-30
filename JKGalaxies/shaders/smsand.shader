textures/sandcrawler/smsand
{
	qer_editorimage	textures/skies/sky.tga
	q3map_surfacelight	75
	q3map_lightsubdivide	512
	sun 0.75 0.79 1 40 315 40
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	textures/skies/bespin 512 -
}

textures/sandcrawler/ore
{
// deformvertexes	wave	100 sin 0 1 0 1

	qer_editorimage	textures/imp_mine/lava2
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	water
	surfaceparm	trans
	q3map_material	Water
	q3map_nolightmap
	q3map_onlyvertexlighting
	q3map_novertexshadows
    {
        map textures/imp_mine/lava2
        tcMod scroll 0.1 0.1
        tcMod turb 1 0.3 1 0.1
    }
}

textures/sandcrawler/d_bay
{
	qer_editorimage	textures/skies/sky.tga
	q3map_lightsubdivide	512
	sun 0.75 0.79 1 40 315 40
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	textures/skies/bespin 512 -
}

