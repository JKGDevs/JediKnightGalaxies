// JKG_ShaderPatches: BlasTech - changes the blending of the decal burnmarks used in maps, which are currently modified JKG burnmarks. 

textures/decals/burnmark1
{
	qer_editorimage	textures/decals/burnmark1_edim
	polygonOffset
	q3map_nolightmap
    {
        map textures/decals/burnmark1
        blendFunc GL_DST_COLOR GL_ONE_MINUS_SRC_ALPHA
    }
}

textures/decals/burnmark2
{
	qer_editorimage	textures/decals/burnmark2_edim
	polygonOffset
	q3map_nolightmap
    {
        map textures/decals/burnmark2
        blendFunc GL_DST_COLOR GL_ONE_MINUS_SRC_ALPHA
    }
}

textures/decals/burnmark3
{
	qer_editorimage	textures/decals/burnmark3_edim
	polygonOffset
	q3map_nolightmap
    {
        map textures/decals/burnmark3
        blendFunc GL_DST_COLOR GL_ONE_MINUS_SRC_ALPHA
    }
}

textures/decals/burnmark4
{
	qer_editorimage	textures/decals/burnmark4_edim
	polygonOffset
	q3map_nolightmap
    {
        map textures/decals/burnmark4
        blendFunc GL_DST_COLOR GL_ONE_MINUS_SRC_ALPHA
    }
}

textures/decals/burnmark5
{
	qer_editorimage	textures/decals/burnmark5_edim
	polygonOffset
	q3map_nolightmap
    {
        map textures/decals/burnmark5
        blendFunc GL_DST_COLOR GL_ONE_MINUS_SRC_ALPHA
    }
}

// ________________________________________________________

textures/decals/mp_rebel_symbol
{
	qer_editorimage	textures/decals/mp_rebel_symbol
	polygonOffset
	q3map_nolightmap
    {
        map textures/decals/mp_rebel_symbol
        alphaFunc GE128
        blendFunc GL_DST_COLOR GL_SRC_COLOR
    }
}

textures/decals/mp_imperial_symbol
{
	qer_editorimage	textures/decals/mp_imperial_symbol
	polygonOffset
	q3map_nolightmap
    {
        map textures/decals/mp_imperial_symbol
        alphaFunc GE128
        blendFunc GL_DST_COLOR GL_SRC_COLOR
    }
}

textures/decals/mp_b_stripe
{
	polygonOffset
	q3map_nolightmap
    {
        map textures/decals/mp_b_stripe
        alphaFunc GE128
        blendFunc GL_DST_COLOR GL_SRC_COLOR
    }
}

textures/decals/mp_r_stripe
{
	polygonOffset
	q3map_nolightmap
    {
        map textures/decals/mp_r_stripe
        alphaFunc GE128
        blendFunc GL_DST_COLOR GL_SRC_COLOR
    }
}

textures/decals/mp_r_lstripcap
{
	polygonOffset
    {
        map $lightmap
    }
    {
        map textures/decals/mp_r_lstripcap
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/decals/mp_r_lstripcapglow
        blendFunc GL_ONE GL_ONE
    }
}

textures/decals/mp_b_lstripcap
{
	polygonOffset
    {
        map $lightmap
    }
    {
        map textures/decals/mp_b_lstripcap
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/decals/mp_b_lstripcapglow
        blendFunc GL_ONE GL_ONE
    }
}

textures/decals/mp_r_lstrip
{
	polygonOffset
    {
        map $lightmap
    }
    {
        map textures/decals/mp_r_lstrip
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/decals/mp_r_lstripglow
        blendFunc GL_ONE GL_ONE
    }
}

textures/decals/mp_b_lstrip
{
	polygonOffset
    {
        map $lightmap
    }
    {
        map textures/decals/mp_b_lstrip
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/decals/mp_b_lstripglow
        blendFunc GL_ONE GL_ONE
    }
}

textures/decals/dirt1
{
	polygonOffset
	q3map_nolightmap
    {
        map textures/decals/dirt1
        alphaFunc GE128
        blendFunc GL_DST_COLOR GL_SRC_COLOR
    }
}

textures/decals/scratch1
{
	surfaceparm	nonopaque
	surfaceparm	trans
	polygonOffset
	q3map_nolightmap
    {
        map textures/decals/scratch1
        alphaFunc GE128
        blendFunc GL_DST_COLOR GL_SRC_COLOR
    }
}

textures/decals/scratch2
{
	surfaceparm	nonopaque
	surfaceparm	trans
	polygonOffset
	q3map_nolightmap
    {
        map textures/decals/scratch2
        alphaFunc GE128
        blendFunc GL_DST_COLOR GL_SRC_COLOR
    }
}

textures/decals/caution_strip
{
	qer_editorimage	textures/decals/caution2.tga
	surfaceparm	nonopaque
	surfaceparm	trans
	polygonOffset
	q3map_nolightmap
    {
        map textures/decals/caution2
        alphaFunc GE128
        blendFunc GL_DST_COLOR GL_SRC_COLOR
    }
}

textures/decals/sign_decal1
{
	qer_editorimage	textures/decals/sign_decal1
	polygonOffset
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map textures/decals/sign_decal1
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

textures/decals/sign_decal2
{
	qer_editorimage	textures/decals/sign_decal2
	polygonOffset
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map textures/decals/sign_decal2
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

textures/decals/sign_decal3
{
	qer_editorimage	textures/decals/sign_decal3
	polygonOffset
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map textures/decals/sign_decal3
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

textures/decals/arrow_decal
{
	qer_editorimage	textures/decals/arrow_decal
	polygonOffset
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map textures/decals/arrow_decal
        blendFunc GL_ONE GL_ONE
    }
}

textures/decals/slashmark1
{
	qer_editorimage	textures/decals/slashmark1
	polygonOffset
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map textures/decals/slashmark1
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
        alphaGen const 0.7
    }
}

gfx/effects/waterpuddle
{
	surfaceparm	nomarks
	surfaceparm	trans
	polygonOffset
	q3map_nolightmap
    {
        map gfx/effects/waterpuddle
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
        alphaGen vertex
    }
}

footstep_r
{
	polygonOffset
	q3map_nolightmap
    {
        clampmap gfx/decals/steps/footstep_r
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
        alphaGen vertex
    }
}

footstep_l
{
	polygonOffset
	q3map_nolightmap
    {
        clampmap gfx/decals/steps/footstep_r
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
        alphaGen vertex
        tcMod transform -1 0 0 1 1 0
    }
}

footstep_heavy_r
{
	polygonOffset
	q3map_nolightmap
    {
        clampmap gfx/decals/steps/footstep_heavy_r
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
        alphaGen vertex
    }
}

footstep_heavy_l
{
	polygonOffset
	q3map_nolightmap
    {
        clampmap gfx/decals/steps/footstep_heavy_r
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
        alphaGen vertex
        tcMod transform -1 0 0 1 1 0
    }
}

// Last modified by: BlasTech on 2011-02-17