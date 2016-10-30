textures/flares/standard_flare
{
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
	deformvertexes	autoSprite	
    {
        map textures/flares/flare_white
        blendFunc GL_ONE GL_ONE
        depthFunc disable
        rgbGen exactVertex
        alphaGen portal 50
    }
}

textures/flares/flare_red
{
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
	deformvertexes	autoSprite	
    {
        map textures/flares/standard_flare
        blendFunc GL_ONE GL_ONE
        depthFunc disable
        rgbGen const ( 1.000000 0.384314 0.384314 )
        alphaGen portal 50
    }
}

textures/flares/flare_blue
{
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
    {
        map textures/flares/flare_blue
        blendFunc GL_ONE GL_ONE
        rgbGen const ( 0.752941 0.752941 0.752941 )
        alphaGen portal 50
    }
}

textures/flares/flare_white
{
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
	cull	twosided
    {
        map textures/flares/standard_flare
        blendFunc GL_ONE GL_ONE
        depthFunc disable
        rgbGen const ( 0.752941 0.752941 0.752941 )
        alphaGen portal 50
    }
}

textures/flares/light_white
{
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
    {
        map textures/flares/light_white
        blendFunc GL_ONE GL_ONE
        rgbGen exactVertex
    }
}

textures/flares/flare_lt_blue
{
	qer_editorimage	textures/flares/flare_blue
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
    {
        map textures/flares/flare_lt_blue
        blendFunc GL_ONE GL_ONE
        depthFunc disable
        rgbGen const ( 0.501961 0.501961 0.501961 )
    }
}

textures/flares/clip_flare_white
{
	qer_editorimage	textures/system/clip
	q3map_flare	textures/flares/flare_lt_blue
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
    {
        map textures/system/clip
        blendFunc GL_ONE GL_ONE
        rgbGen exactVertex
    }
}

textures/flares/solid_blue
{
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
    {
        map textures/flares/solid_blue
        blendFunc GL_ONE GL_ONE
        rgbGen exactVertex
    }
}

textures/flares/flare_ring2
{
	qer_editorimage	textures/flares/standard_flare
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
	cull	twosided
    {
        map textures/flares/flare_ring2
        blendFunc GL_ONE GL_ONE
        rgbGen exactVertex
    }
}

textures/flares/flare_white_pulse
{
	qer_editorimage	textures/flares/standard_flare
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
	cull	twosided
    {
        map textures/flares/standard_flare
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.5 1 0.2 0.5
    }
}

textures/flares/standard_flare_bigger
{
	surfaceparm	nomarks
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

textures/flares/flare_crystal
{
	qer_editorimage	textures/flares/flare_blue
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
    {
        map textures/flares/flare_blue
        blendFunc GL_ONE GL_ONE
        rgbGen const ( 0.501961 0.501961 0.501961 )
        alphaGen portal 45
    }
}

textures/flares/flare_yellow
{
	qer_editorimage	textures/flares/standard_flare
	surfaceparm	nomarks
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
        alphaGen portal 45
    }
}

textures/flares/flare_red_pulse
{
	qer_editorimage	textures/flares/flare_red
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
	deformvertexes	autoSprite	
    {
        map textures/flares/flare_red
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.5 1 0.2 0.5
    }
}

textures/flares/flare_blue_pulse
{
	qer_editorimage	textures/flares/flare_blue
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
    {
        map textures/flares/flare_blue
        blendFunc GL_ONE GL_ONE_MINUS_SRC_COLOR
        rgbGen wave sin 0.5 1 0.2 0.5
        alphaGen portal 50
    }
}

textures/flares/flare_beam
{
	qer_editorimage	gfx/misc/hbeam1
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
	cull	twosided
    {
        map gfx/misc/hbeam1
        blendFunc GL_ONE GL_ONE
        rgbGen wave triangle 0.3 0.1 0.1 6
        tcMod scroll 0 -7
        tcMod stretch sin 1 0.15 0 1
    }
    {
        map gfx/misc/hbeam1a
        blendFunc GL_ONE GL_ONE
        rgbGen const ( 0.184314 0.015686 0.858824 )
        tcMod scroll 0 -4
        tcMod scale 1 2
    }
    {
        map gfx/misc/flaretail
        blendFunc GL_ONE GL_ONE
        tcMod stretch sin 1 0.03 0 1
        tcMod scroll 0 -6
    }
    {
        map gfx/misc/smflare
        blendFunc GL_ONE GL_ONE
        tcMod stretch sin 1 0.3 0 5
        tcMod scroll 0 -1
    }
}

textures/flares/flare_orange
{
	qer_editorimage	textures/flares/standard_flare
	qer_nocarve
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
	cull	twosided
	deformvertexes	autoSprite	
    {
        map textures/flares/standard_flare
        blendFunc GL_ONE GL_ONE
        depthFunc disable
        rgbGen const ( 0.921569 0.607843 0.329412 )
        alphaGen portal 50
    }
}

textures/flares/flare_bluehue
{
	qer_editorimage	textures/flares/standard_flare
	qer_nocarve
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
	cull	twosided
	deformvertexes	autoSprite	
    {
        map textures/flares/standard_flare
        blendFunc GL_ONE GL_ONE
        depthFunc disable
        rgbGen const ( 0.784314 0.843137 0.917647 )
        alphaGen portal 50
    }
}

textures/flares/solid_orange
{
	qer_editorimage	textures/flares/solid_orange
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
    {
        map textures/flares/solid_orange
        blendFunc GL_ONE GL_ONE
        rgbGen exactVertex
    }
}

textures/flares/flare_blue2
{
	qer_editorimage	textures/flares/standard_flare
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
	cull	twosided
	deformvertexes	autoSprite	
    {
        map textures/flares/standard_flare
        blendFunc GL_ONE GL_ONE
        depthFunc disable
        rgbGen const ( 0.000000 0.250980 0.501961 )
        alphaGen portal 90
    }
}

textures/flares/solid_red
{
	qer_editorimage	textures/flares/solid_red
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
    {
        map textures/flares/solid_red
        blendFunc GL_ONE GL_ONE
        rgbGen exactVertex
    }
}

textures/flares/Wedge_Blue_flare
{
	qer_editorimage	textures/flares/flare_blue
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
    {
        map textures/flares/flare_blue
        blendFunc GL_ONE GL_ONE
        rgbGen const ( 0.752941 0.752941 0.752941 )
        alphaGen portal 70
    }
}

textures/flares/Vjun2_Green_flare
{
	qer_editorimage	textures/flares/standard_flare
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
    {
        map textures/flares/standard_flare
        blendFunc GL_ONE GL_ONE
        rgbGen const ( 0.796078 0.984314 0.517647 )
        alphaGen portal 300
    }
}

textures/flares/flare_wedge
{
	qer_editorimage	textures/flares/standard_flare_bigger
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
	cull	twosided
    {
        map textures/flares/flare_wedge
        blendFunc GL_ONE GL_ONE
        rgbGen const ( 0.752941 0.752941 0.752941 )
        alphaGen portal 70
    }
}

textures/flares/flare_stampede
{
	qer_editorimage	textures/flares/flare_blue
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
    {
        map textures/flares/flare_blue
        blendFunc GL_ONE GL_ONE
        rgbGen const ( 0.752941 0.752941 0.752941 )
        alphaGen portal 140
    }
}

gfx/misc/smflare
{
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
	cull	twosided
    {
        map gfx/misc/smflare
        blendFunc GL_ONE GL_ONE
    }
}

