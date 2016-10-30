textures/fogs/fog1
{
	qer_editorimage	textures/fogs/fog.tga
	qer_nocarve
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0 0 0 ) 256.0
	cull	twosided
}

textures/fogs/yavin
{
	qer_editorimage	textures/fogs/fog.tga
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0.541176 0.490196 0.376471 ) 4000.0
}

textures/fogs/proto
{
	qer_editorimage	textures/fogs/fog.tga
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	fogparms	( 1 1 1 ) 20000.0
}

textures/fogs/yavin2
{
	qer_editorimage	textures/fogs/fog.tga
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	fogparms	( 0.7 0.6 0.5 ) 3900.0
}

textures/fogs/test1
{
	qer_editorimage	textures/fogs/fog.tga
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	fogparms	( 0.847059 0.847059 0.847059 ) 5500.0
}

textures/fogs/bespin
{
	qer_editorimage	textures/skies/inteldimclouds
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0.984314 0.576471 0.4 ) 768.0
}

textures/fogs/narshadaa
{
	qer_editorimage	textures/fogs/fog.tga
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0.380392 0.533333 0.0901961 ) 2500.0
}

textures/fogs/narshadaa2
{
	qer_editorimage	textures/fogs/fog.tga
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0.921569 0.972549 0.827451 ) 1200.0
}

textures/fogs/kejim_cold
{
	qer_editorimage	textures/fogs/fog.tga
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 1 1 1 ) 4000.0
}

textures/fogs/cairn_red
{
	qer_editorimage	textures/fogs/fog.tga
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0.501961 0 0 ) 4000.0
}

textures/fogs/fog_black
{
	qer_editorimage	textures/fogs/fog.tga
	qer_nocarve
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0 0 0 ) 3456.0
	cull	twosided
}

textures/fogs/fog_black2
{
	qer_editorimage	textures/fogs/fog.tga
	qer_nocarve
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0 0 0 ) 2048.0
	cull	twosided
}

textures/fogs/fog_blacksiegehoth
{
	qer_editorimage	textures/fogs/fog.tga
	qer_nocarve
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0 0 0 ) 300.0
	cull	twosided
}

textures/fogs/fog_siege2
{
	qer_editorimage	textures/fogs/fog.tga
	qer_nocarve
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0.58 0.58 0.5 ) 16384.0
	cull	twosided
}

textures/fogs/fog_blackdeathstar
{
	qer_editorimage	textures/fogs/fog.tga
	qer_nocarve
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0 0 0 ) 4400.0
	cull	twosided
}

textures/fogs/yavin_white
{
	qer_editorimage	textures/fogs/fog.tga
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 1 1 1 ) 3500.0
}

textures/fogs/bespin_notex
{
	qer_editorimage	textures/fogs/fog
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0.984314 0.576471 0.4 ) 2048.0
}

textures/fogs/bespin_cloud_layer
{
	qer_editorimage	textures/skies/inteldimclouds
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
    {
        map textures/skies/inteldimclouds
        blendFunc GL_ONE GL_ONE
        rgbGen identity
        tcMod turb 0 1 0 0.1
        tcMod rotate -7
    }
    {
        map gfx/misc/smoke2
        blendFunc GL_ONE GL_ONE
        tcMod rotate 10
        tcMod turb 0 0.1 1 0.1
    }
    {
        map textures/skies/intelredclouds
        blendFunc GL_ONE GL_ONE
        rgbGen identity
        tcMod turb 0.5 0.5 0.2 0.05
        tcMod rotate 10
    }
}

textures/fogs/fog_black_reverse
{
	qer_editorimage	textures/fogs/fog.tga
	qer_nocarve
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0 0 0 ) 3072.0
	cull	twosided
}

textures/fogs/bespin_test
{
	qer_editorimage	textures/skies/inteldimclouds
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0.984314 0.576471 0.4 ) 4096.0
    {
        map textures/skies/inteldimclouds
        blendFunc GL_ONE GL_ONE
        rgbGen identity
        tcMod turb 0 1 0 0.1
        tcMod rotate -7
    }
    {
        map gfx/misc/smoke2
        blendFunc GL_ONE GL_ONE
        tcMod rotate 10
        tcMod turb 0 0.1 1 0.1
    }
    {
        map textures/skies/intelredclouds
        blendFunc GL_ONE GL_ONE
        rgbGen identity
        tcMod turb 0.5 0.5 0.2 0.05
        tcMod rotate 10
    }
}

textures/fogs/yavin_swamp
{
	qer_editorimage	textures/fogs/fog.tga
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0.54902 0.537255 0.356863 ) 2800.0
}

textures/fogs/fog_grey
{
	lightcolor	( 1 1 1 )
	qer_editorimage	textures/fogs/fog.tga
	qer_nocarve
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0.490196 0.490196 0.490196 ) 5000.0
	cull	twosided
}

textures/fogs/fog_amber
{
	lightcolor	( 1 1 1 )
	qer_editorimage	textures/fogs/fog.tga
	qer_nocarve
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 1 0.666667 0.0509804 ) 8000.0
	cull	twosided
}

textures/fogs/smtest_fog
{
	qer_editorimage	textures/fogs/fog.tga
	qer_nocarve
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0.784314 0.666667 0.556863 ) 3500.0
	cull	twosided
}

textures/fogs/areisfog
{
	qer_editorimage	textures/fogs/fog.tga
	qer_nocarve
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0.784314 0.666667 0.556863 ) 1500.0
	cull	twosided
}

textures/fogs/rift
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

// FORD FOGS

textures/fogs/Hoth2fog
{
// fogparms ( Red, Green, Blue=Color of opacity layer) Number.0=depth of transparency/opacity border.

	qer_editorimage	textures/fogs/Hoth2_bluefog
	qer_nocarve
	qer_trans	0.5
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0.7 0.7 0.7 ) 1800.0
	cull	twosided
	deformvertexes	move	0 0 0 sin 0 1 0 1
}

textures/fogs/T2_Wedgefog
{
// fogparms ( Red, Green, Blue=Color of opacity layer) Number.0=depth of transparency/opacity border.

	qer_editorimage	textures/fogs/T2_Wedgefog
	qer_nocarve
	qer_trans	0.5
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0.752941 0.752941 0.752941 ) 8500.0
	cull	twosided
	deformvertexes	move	0 0 0 sin 0 1 0 1
}

textures/fogs/vjun1
{
// fogparms ( Red, Green, Blue=Color of opacity layer) Number.0=depth of transparency/opacity border.

	qer_editorimage	textures/fogs/T2_Wedgefog
	qer_nocarve
	qer_trans	0.5
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0.737255 0.588235 0.388235 ) 5190.0
	cull	twosided
}

textures/fogs/T1_Fatal_redfog2_light
{
// fogparms ( Red, Green, Blue=Color of opacity layer) Number.0=depth of transparency/opacity border.

	q3map_lightimage	w/game/base/textures/fogs/T1_Fatal_redfog.tga
	qer_editorimage	textures/fogs/T1_Fatal_redfog
	q3map_surfacelight	180
	qer_nocarve
	qer_trans	0.5
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0.227451 0.0235294 0.00392157 ) 250.0
	cull	twosided
	deformvertexes	move	0 0 0 sin 0 1 0 1
}

textures/fogs/greyfog
{
// fogparms ( Red, Green, Blue=Color of opacity layer) Number.0=depth of transparency/opacity border.

	qer_editorimage	textures/fogs/greyfog
	qer_nocarve
	qer_trans	0.5
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0.752941 0.752941 0.752941 ) 650.0
	cull	twosided
	deformvertexes	move	0 0 0 sin 0 1 0 1
}

textures/fogs/T1_Fatal_redfog_light
{
// fogparms ( Red, Green, Blue=Color of opacity layer) Number.0=depth of transparency/opacity border.

	q3map_lightimage	w/game/base/textures/fogs/T1_Fatal_redfog
	qer_editorimage	textures/fogs/T1_Fatal_redfog
	q3map_surfacelight	180
	qer_nocarve
	qer_trans	0.5
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0.227451 0.0235294 0.00392157 ) 750.0
	cull	twosided
	deformvertexes	move	0 0 0 sin 0 1 0 1
}

// 

textures/fogs/hoth
{
// fogparms ( Red, Green, Blue=Color of opacity layer) Number.0=depth of transparency/opacity border.

	qer_editorimage	textures/fogs/fog
	qer_nocarve
	qer_trans	0.5
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0.666667 0.666667 0.666667 ) 1500.0
}

textures/fogs/korriban
{
	qer_editorimage	textures/fogs/fog.tga
	qer_nocarve
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0.95 0.97 0.77 ) 7000.0
	cull	twosided
}

textures/fogs/rail
{
	qer_editorimage	textures/fogs/fog.tga
	qer_nocarve
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0.207843 0.207843 0.278431 ) 3500.0
}

textures/fogs/vjun1acidriver
{
	qer_editorimage	textures/fogs/fog.tga
	qer_nocarve
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0.611765 0.678431 0.239216 ) 3500.0
}

textures/fogs/lava
{
	qer_editorimage	textures/fogs/fog.tga
	qer_nocarve
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0.976471 0.0627451 0.00784314 ) 1700.0
}

textures/hoth/hoth_fog
{
	qer_editorimage	textures/system/fog
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0.435294 0.486275 0.709804 ) 6800.0
}

textures/hoth/hoth_fog2
{
	qer_editorimage	textures/system/fog
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0.643137 0.643137 0.643137 ) 5000.0
}

