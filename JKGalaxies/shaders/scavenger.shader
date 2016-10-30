gfx/misc/red_flare
{
	cull	disable
    {
        map gfx/misc/rfl1d2.tga
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/misc/red_ring
{
	cull	disable
    {
        map gfx/misc/redring.tga
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/misc/red_ring2
{
	cull	disable
    {
        map gfx/misc/redring2.tga
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
        tcMod scroll 2 0
    }
}

gfx/misc/scavaltfire
{
	cull	disable
    {
        map gfx/misc/afl8b.tga
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/misc/trail2
{
	cull	disable
    {
        map gfx/misc/trail1_3.tga
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/misc/tet1
{
	cull	disable
    {
        map gfx/misc/tet1_3.tga
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

models/weapons2/scavenger/scav
{
    {
        map models/weapons2/scavenger/scav.tga
        rgbGen lightingDiffuse
    }
    {
        map models/weapons2/scavenger/scav_spec.tga
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        rgbGen identity
        alphaGen lightingSpecular
    }
}

scavExplosion
{
	sort	additive
	cull	disable
    {
        animMap 4 gfx/misc/exp04_1.tga gfx/misc/exp04_2.tga gfx/misc/exp04_3.tga 
        blendFunc GL_ONE GL_ONE
        rgbGen wave inversesawtooth 0 1 0 4
    }
    {
        animMap 4 gfx/misc/exp04_2.tga gfx/misc/exp04_3.tga gfx/colors/black.tga 
        blendFunc GL_ONE GL_ONE
        rgbGen wave sawtooth 0 1 0 4
    }
}

scavExplosionSlow
{
	sort	additive
	cull	disable
    {
        animMap 4.4 gfx/misc/exp04_1.tga gfx/misc/exp04_2.tga gfx/misc/exp04_3.tga 
        blendFunc GL_ONE GL_ONE
        rgbGen wave inversesawtooth 0 1 0 4.4
    }
    {
        animMap 4.4 gfx/misc/exp04_2.tga gfx/misc/exp04_3.tga gfx/colors/black.tga 
        blendFunc GL_ONE GL_ONE
        rgbGen wave sawtooth 0 1 0 4.4
    }
}

scavExplosionFast
{
	sort	additive
	cull	disable
    {
        animMap 7.5 gfx/misc/exp04_1.tga gfx/misc/exp04_2.tga gfx/misc/exp04_3.tga 
        blendFunc GL_ONE GL_ONE
        rgbGen wave inversesawtooth 0 1 0 7.5
    }
    {
        animMap 7.5 gfx/misc/exp04_2.tga gfx/misc/exp04_3.tga gfx/colors/black.tga 
        blendFunc GL_ONE GL_ONE
        rgbGen wave sawtooth 0 1 0 7.5
    }
}

