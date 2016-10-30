// JKG_ShaderPatches: BlasTech - replaces BaseJKA saber blades with the various lightsaber blade styles of JKG, moves saber flare and "throw Force glow 'round hand" here

gfx/effects/sabers/red_glow
{
	qer_editorimage	gfx/lightsabers/glow_static
	cull	twosided
    {
        map gfx/lightsabers/glow_static
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen const ( 0.650980 0.070588 0.023529 )
//        rgbGen const ( 0.611765 0.101961 0.070588 )
    }
}

gfx/effects/sabers/orange_glow
{
	qer_editorimage	gfx/lightsabers/glow_static
	cull	twosided
    {
        map gfx/lightsabers/glow_static
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen const ( 0.843137 0.415686 0.035294 )
//        rgbGen const ( 0.725490 0.427451 0.156863 )
    }
}

gfx/effects/sabers/yellow_glow
{
	qer_editorimage	gfx/lightsabers/glow
	cull	twosided
    {
        map gfx/lightsabers/glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen const ( 0.760784 0.541176 0.027451 )
//        rgbGen const ( 0.690196 0.513726 0.101961 )
    }
}

gfx/effects/sabers/green_glow
{
	qer_editorimage	gfx/lightsabers/glow
	cull	twosided
    {
        map gfx/lightsabers/glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen const ( 0.274510 0.650980 0.023529 )
//        rgbGen const ( 0.294118 0.580392 0.105882 )
    }
}

gfx/effects/sabers/blue_glow
{
	qer_editorimage	gfx/lightsabers/glow
	cull	twosided
    {
        map gfx/lightsabers/glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen const ( 0.023529 0.458824 0.666667 )
//        rgbGen const ( 0.113725 0.435294 0.588235 )
    }
}

gfx/effects/sabers/purple_glow
{
	qer_editorimage	gfx/lightsabers/glow
	cull	twosided
    {
        map gfx/lightsabers/glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen const ( 0.470588 0.027451 0.760784 )
//        rgbGen const ( 0.454902 0.101961 0.690196 )
    }
}

gfx/effects/sabers/red_line
{
	qer_editorimage	gfx/lightsabers/blade_unstable
	cull	twosided
    {
        animMap 5.2 gfx/lightsabers/blade_unstable gfx/lightsabers/blade_unstable2 gfx/lightsabers/blade_unstable3 gfx/lightsabers/blade_unstable4 
        blendFunc GL_ONE GL_ONE
        rgbGen wave inversesawtooth 0 1 0 5.2
    }
    {
        animMap 5.2 gfx/lightsabers/blade_unstable4 gfx/lightsabers/blade_unstable gfx/lightsabers/blade_unstable2 gfx/lightsabers/blade_unstable3 
        blendFunc GL_ONE GL_ONE
        rgbGen wave sawtooth 0 1 0 5.2
    }
}

gfx/effects/sabers/orange_line
{
	cull	twosided
    {
        map gfx/lightsabers/blade_cheap_static
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/effects/sabers/yellow_line
{
	qer_editorimage	gfx/lightsabers/blade_compressed
	cull	twosided
    {
        map gfx/lightsabers/blade_cheap
        blendFunc GL_ZERO GL_ONE_MINUS_SRC_COLOR
        rgbGen identity
    }
    {
        animMap 5.2 gfx/lightsabers/blade_compressed gfx/lightsabers/blade_compressed2 gfx/lightsabers/blade_compressed3 gfx/lightsabers/blade_compressed4
        blendFunc GL_ONE GL_ONE
        rgbGen wave inversesawtooth 0 1 0 5.2
    }
    {
        animMap 5.2 gfx/lightsabers/blade_compressed4 gfx/lightsabers/blade_compressed gfx/lightsabers/blade_compressed2 gfx/lightsabers/blade_compressed3
        blendFunc GL_ONE GL_ONE
        rgbGen wave sawtooth 0 1 0 5.2
    }
}

gfx/effects/sabers/green_line
{
	cull	twosided
    {
        map gfx/lightsabers/blade_cheap
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/effects/sabers/blue_line
{
	cull	twosided
    {
        map gfx/lightsabers/blade_dull
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/effects/sabers/purple_line
{
	cull	twosided
    {
        map gfx/lightsabers/blade_sharp
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/effects/sabers/saberBlur
{
	cull	twosided
    {
        clampmap gfx/lightsabers/trail_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen vertex
    }
    {
        clampmap gfx/lightsabers/trail_core
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

gfx/effects/sabers/swordTrail
{
	qer_editorimage	gfx/lightsabers/swordtrail
	cull	twosided
    {
        clampmap gfx/lightsabers/unstable_trail_glow
        blendFunc GL_ZERO GL_ONE_MINUS_SRC_COLOR
        rgbGen identity
    }
    {
        clampmap gfx/lightsabers/unstable_trail_core
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen vertex
    }
}

// Last modified by: BlasTech on 2011-02-18

