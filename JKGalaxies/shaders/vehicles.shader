gfx/misc/shields_green
{
	deformvertexes	bulge	0 0.75 0
    {
        map gfx/effects/p_shield
        blendFunc GL_DST_COLOR GL_ONE
        rgbGen entity
        tcGen environment
        tcMod rotate 200
        tcMod turb 0.6 0.3 0 0.6
    }
    {
        map gfx/effects/p_shield
        blendFunc GL_DST_COLOR GL_ONE
        rgbGen entity
        tcMod rotate -600
        tcMod scale 2 3
    }
}

gfx/misc/shields_blue
{
	deformvertexes	bulge	0 0.75 0
    {
        map gfx/effects/p_shield
        blendFunc GL_DST_COLOR GL_ONE
        rgbGen entity
        tcGen environment
        tcMod rotate 200
        tcMod turb 0.6 0.3 0 0.6
    }
    {
        map gfx/effects/p_shield
        blendFunc GL_DST_COLOR GL_SRC_ALPHA
        rgbGen entity
        alphaGen entity
        tcMod rotate -600
        tcMod scale 2 3
    }
}

