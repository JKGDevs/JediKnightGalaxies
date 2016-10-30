// JKG_ShaderPatches: BlasTech - Directing saber burnmarks on players

// G2Marks drawn on players by lightsaber blades

gfx/damage/saberglowmark
{
	polygonOffset
    {
        map gfx/burnmarks/burnline
        blendFunc GL_DST_COLOR GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map gfx/burnmarks/burnline_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave noise 1 0.6 0 0.1
    }
}

/// Rivetmark is drawn on architecture by lightsaber blades. Go easy with this one... Notice that I've directed it to a grayed out burnmarks/burnline now. 

gfx/damage/rivetmark
{
	surfaceparm	nomarks
	surfaceparm	trans
	polygonOffset
	q3map_nolightmap
    {
        clampmap gfx/burnmarks/burnline_overlay
        blendFunc GL_DST_COLOR GL_SRC_COLOR
    }
}

// __________________________________________________________________

gfx/damage/rivetmark_old
{
	surfaceparm	nomarks
	surfaceparm	trans
	polygonOffset
	q3map_nolightmap
    {
        clampmap gfx/damage/rivetmark
        blendFunc GL_DST_COLOR GL_SRC_COLOR
    }
}

// Old ugly EFX burnmarks

gfx/damage/burnmark4
{
	polygonOffset
    {
        clampmap gfx/damage/burnmark3
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
        alphaGen vertex
    }
}

gfx/damage/burnmark1
{
	polygonOffset
    {
        clampmap gfx/damage/burnmark1
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
        alphaGen vertex
    }
}

// Probably not being used - missing texture in Base

gfx/damage/saberburnmark
{
	polygonOffset
    {
        map gfx/burnmarks/burnline
        blendFunc GL_DST_COLOR GL_ONE_MINUS_SRC_ALPHA
    }
}

// Same as generic model caps shader

gfx/damage/saberglowmark2
{
	polygonOffset
    {
        map textures/common/caps
    }
    {
        map textures/common/caps_glow
        blendFunc GL_ONE GL_ONE
        rgbGen wave noise 1 0.5 0 2
    }
}

// Last modified by: BlasTech on 2010-10-02