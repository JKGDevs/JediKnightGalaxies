textures/shadow/fan
{
	cull	disable
	polygonOffset
    {
        map textures/test/fan_shadow.jpg
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        tcMod rotate -4
    }
}

textures/shadow/grate
{
	cull	disable
	polygonOffset
    {
        map textures/test/grate_shadow.jpg
        blendFunc GL_DST_COLOR GL_SRC_COLOR
    }
}

textures/shadow/haze
{
	surfaceparm	nonsolid

	cull back
    {
        map textures/test/fan_haze.jpg
        blendFunc GL_ONE GL_ONE
	tcMod scale 4 1
	tcMod scroll -1.0 0
	rgbGen const ( 0.1 0.1 0.1 )
    }

}
