// JKG_ShaderPatches: BlasTech - replaces the glowing baseJKA body blastholes with JKG burnmarks. 

// Bullet wounds

gfx/damage/bodyburnmark1
{
	surfaceparm	nomarks
	surfaceparm	trans
	polygonOffset
	q3map_nolightmap
    {
        clampmap gfx/burnmarks/projmark3
        blendFunc GL_DST_COLOR GL_ONE_MINUS_SRC_ALPHA
        alphaGen vertex
    }
}

// Explosive wounds

gfx/damage/bodybigburnmark1
{
	qer_editorimage	gfx/damage/bodyburnmark_glow
	surfaceparm	nomarks
	surfaceparm	trans
	polygonOffset
	q3map_nolightmap
    {
        clampmap gfx/burnmarks/projmark2
        blendFunc GL_DST_COLOR GL_ONE_MINUS_SRC_ALPHA
        alphaGen vertex
    }
}

// Last modified by: BlasTech on 2012-02-05