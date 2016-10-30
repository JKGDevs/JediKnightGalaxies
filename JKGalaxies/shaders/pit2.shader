textures/impdetention/light1_2
{
	qer_editorimage	textures/impdetention/lightnewglow
	q3map_surfacelight	4000
    {
        map $lightmap
    }
    {
        map textures/impdetention/light1
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave triangle 1 0.025 1 25
    }
}

textures/impdetention/lightnewglow2
{
	qer_editorimage	textures/impdetention/lightnewglow
	q3map_surfacelight	4000
	q3map_nolightmap
    {
        map textures/impdetention/lightnewglow
        rgbGen wave triangle 1 0.025 1 25
    }
}

textures/impdetention/light_white_flare2
{
	qer_editorimage	textures/impdetention/light_white
	q3map_surfacelight	4000
	q3map_nolightmap
    {
        map textures/impdetention/light_white
        rgbGen identity
    }
}

