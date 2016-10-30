textures/system/nodrop
{
	qer_nocarve
	qer_trans	0.5
	surfaceparm	nomarks
	surfaceparm	nodraw
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm nodrop
	surfaceparm	trans
	q3map_nolightmap
}

textures/system/blockplayer
{
	qer_trans	0.15
	surfaceparm	noimpact
	surfaceparm	nomarks
	surfaceparm	nodraw
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	playerclip
	surfaceparm	trans
}

textures/system/blocknpc
{
	qer_trans	0.5
	surfaceparm	noimpact
	surfaceparm	nomarks
	surfaceparm	nodraw
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	monsterclip
	surfaceparm	trans
}

textures/system/clip
{
	qer_trans	0.5
	surfaceparm	noimpact
	surfaceparm	nomarks
	surfaceparm	nodraw
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	playerclip
	surfaceparm	monsterclip
	surfaceparm	trans
	q3map_nolightmap
}

textures/system/cushion
{
	qer_nocarve
	qer_trans	0.3
	surfaceparm	nodamage
	surfaceparm	nomarks
	surfaceparm	nodraw
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
}

textures/system/hint
{
	qer_nocarve
	qer_trans	0.3
	surfaceparm	noimpact
	surfaceparm	nodraw
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_hint
	q3map_nolightmap
	q3map_structural
}

textures/system/origin
{
	qer_nocarve
	surfaceparm	nodraw
	surfaceparm	nonsolid
	q3map_nolightmap
	q3map_origin
}

textures/system/noimpact
{
	surfaceparm	noimpact
	surfaceparm	nodraw
	surfaceparm	nonopaque
	surfaceparm	trans
}

textures/system/areaportal
{
	surfaceparm	nomarks
	surfaceparm	nodraw
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
	q3map_areaportal
	q3map_structural
}

textures/system/trigger
{
	qer_nocarve
	qer_trans	0.5
	surfaceparm	nodraw
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trigger
	surfaceparm	trans
	q3map_nolightmap
}

textures/system/caulk
{
	surfaceparm	nomarks
	surfaceparm	nodraw
	q3map_nolightmap
}

textures/system/nodraw
{
	surfaceparm	nomarks
	surfaceparm	nodraw
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
}

textures/system/weatherzone
{
	qer_trans	0.5
	surfaceparm	nomarks
	surfaceparm	nodraw
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
}

textures/system/nodraw_solid
{
	qer_editorimage	textures/system/nodraw.tga
	surfaceparm	nomarks
	surfaceparm	nodraw
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
}

textures/system/nodraw_slime
{
	qer_editorimage	textures/system/nodraw.tga
	surfaceparm	nomarks
	surfaceparm	nodraw
	surfaceparm	nonopaque
	q3map_nolightmap
}

textures/system/mirror1
{
	qer_editorimage	textures/system/qer_mirror.tga
	portal
	q3map_nolightmap
}

textures/system/portal
{
	qer_editorimage	textures/system/qer_portal.tga
	portal
	q3map_nolightmap
    {
        map gfx/colors/black
        blendFunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
        alphaGen portal 256
    }
}

textures/system/portal_512
{
	qer_editorimage	textures/system/qer_portal.tga
	portal
	q3map_nolightmap
    {
        map gfx/colors/black
        blendFunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
        alphaGen portal 512
    }
}

textures/system/shotonlyclip
{
	surfaceparm	nomarks
	surfaceparm	nodraw
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	shotclip
	surfaceparm	trans
}

textures/system/cluster_portal
{
	qer_trans	0.5
	surfaceparm	nomarks
	surfaceparm	nodraw
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm detail
	surfaceparm	trans
	q3map_clusterportal
}

textures/system/do_not_enter
{
	qer_trans	0.5
	surfaceparm	nomarks
	surfaceparm	nodraw
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	botclip
	surfaceparm detail
	surfaceparm	trans
}

textures/system/fog_black
{
	qer_editorimage	textures/system/fog.tga
	qer_nocarve
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm nodrop
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0 0 0 ) 2048.0
	cull	back
}

textures/system/outside
{
	qer_trans	0.5
	surfaceparm	nomarks
	surfaceparm	nodraw
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	outside
	surfaceparm	trans
	cull	twosided
}

textures/system/inside
{
	qer_trans	0.5
	surfaceparm	nomarks
	surfaceparm	nodraw
	surfaceparm	inside
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	cull	twosided
}

textures/system/physics_clip
{
	qer_editorimage	textures/system/physics_clip
	qer_trans	0.3
	surfaceparm	nodraw
	surfaceparm	trans
	q3map_nolightmap
}

textures/system/terrain
{
// qer_trans	0.9

	qer_editorimage	textures/system/terrain
	qer_nocarve
	surfaceparm	nodraw
	q3map_terrain
	q3map_nolightmap
}

textures/system/ladder
{
	qer_trans	0.4
	surfaceparm	nomarks
	surfaceparm	nodraw
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	ladder
	surfaceparm	trans
	q3map_nolightmap
}

textures/system/mirror_yavin
{
	qer_editorimage	textures/system/qer_portal.tga
	surfaceparm	nomarks
	portal
	q3map_nolightmap
	sort	portal
    {
        map gfx/colors/black
        blendFunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
        alphaGen portal 512
    }
}

textures/system/slick
{
	qer_editorimage	textures/system/slick
	qer_trans	0.15
	surfaceparm	slick
	surfaceparm	noimpact
	surfaceparm	nomarks
	surfaceparm	nodraw
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	playerclip
	surfaceparm	trans
}

textures/system/caulk_nonsolid
{
	qer_editorimage	textures/system/caulk
	surfaceparm	nomarks
	surfaceparm	nodraw
	surfaceparm	nonsolid
	q3map_nolightmap
}

textures/system/antiportal
{
	qer_nocarve
	qer_trans	0.3
	surfaceparm	noimpact
	surfaceparm	nodraw
	surfaceparm	antiportal
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_structural
}

textures/system/skip
{
	qer_nocarve
	qer_trans	0.3
	surfaceparm	noimpact
	surfaceparm	nodraw
	surfaceparm	skip
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_structural
}

textures/system/clipmodel_nodraw
{
	qer_editorimage	textures/system/physics_clip
	surfaceparm	nodraw
	q3map_nolightmap
	q3map_clipmodel
    {
        map textures/system/physics_clip
        rgbGen exactVertex
    }
}

textures/system/caulk_water
{
	qer_editorimage	textures/system/caulk
	surfaceparm	nomarks
	surfaceparm	nodraw
	surfaceparm	water
	q3map_nolightmap
}

textures/system/caulk_lava
{
	qer_editorimage	textures/system/caulk
	surfaceparm	nomarks
	surfaceparm	nodraw
	surfaceparm	lava
	q3map_nolightmap
}

textures/system/caulk_acid
{
	qer_editorimage	textures/system/caulk
	surfaceparm	nomarks
	surfaceparm	nodraw
	surfaceparm	slime
	q3map_nolightmap
}

textures/system/lightgrid
{
	qer_trans	0.5
	surfaceparm	nomarks
	surfaceparm	nodraw
	surfaceparm	lightgrid
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm detail
	surfaceparm	trans
	q3map_nolightmap
}

textures/system/viewscreen1
{
	nopicmip
	nomipmaps
	q3map_nolightmap
    {
        map textures/system/viewscreen1
        blendFunc GL_ONE GL_ZERO
        rgbGen vertex
    }
}

textures/common/caulk
{
	qer_editorimage	textures/system/caulk
	surfaceparm	nomarks
	surfaceparm	nodraw
	q3map_nolightmap
}

textures/region
{
	q3map_nolightmap
    {
        map textures/region
    }
}

textures/colors/black
{
	q3map_nolightmap
    {
        map $whiteimage
        rgbGen const ( 0.000000 0.000000 0.000000 )
    }
}

textures/colors/black_nomarks
{
	qer_editorimage	textures/colors/black.tga
	surfaceparm	nomarks
	q3map_nolightmap
    {
        map $whiteimage
        rgbGen const ( 0.000000 0.000000 0.000000 )
    }
}

textures/colors/yellow
{
	q3map_nolightmap
    {
        map $whiteimage
        rgbGen const ( 1.000000 1.000000 0.000000 )
    }
}

textures/colors/white
{
	q3map_nolightmap
    {
        map $whiteimage
        rgbGen const ( 1.000000 1.000000 1.000000 )
    }
}

textures/colors/white2
{
	qer_editorimage	textures/colors/white
    {
        map $lightmap
    }
}

