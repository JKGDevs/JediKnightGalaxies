textures/bounty/flag
{
	q3map_nolightmap
	q3map_alphashadow
	cull	twosided
    {
        map textures/bounty/flag
        alphaFunc GE192
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        depthWrite
        rgbGen vertex
    }
}

textures/bounty/flag2
{
	qer_editorimage	textures/bounty/flag2
	q3map_nolightmap
	q3map_alphashadow
	cull	twosided
    {
        map textures/bounty/flag2
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        depthWrite
        rgbGen vertex
    }
}

textures/bounty/flag2_b
{
	qer_editorimage	textures/bounty/flag2
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	trans
	q3map_nolightmap
	q3map_alphashadow
	cull	twosided
    {
        map textures/bounty/flag2
        alphaFunc GE128
        rgbGen vertex
    }
}

textures/bounty/flag3
{
	qer_editorimage	textures/bounty/flag3
	q3map_nolightmap
	q3map_alphashadow
	cull	twosided
    {
        map textures/bounty/flag3
        alphaFunc GE192
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        depthWrite
        rgbGen vertex
    }
}

textures/bounty/door_onoff
{
    {
        map $lightmap
    }
    {
        map textures/bounty/door_onoff
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        oneshotanimMap 1 textures/bounty/door_onoffr textures/bounty/door_onoffg 
        blendFunc GL_ONE GL_ONE
        alphaGen identity
    }
}

textures/bounty/new_wall1_light
{
    {
        map $lightmap
    }
    {
        map textures/bounty/new_wall1_light
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/bounty/new_wall1_light_glow
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

textures/bounty/new_wall1_light_off
{
	qer_editorimage	textures/bounty/new_wall1_light
    {
        map $lightmap
    }
    {
        map textures/bounty/new_wall1_light
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

models/map_objects/bounty/light
{
	q3map_nolightmap
    {
        map models/map_objects/bounty/light
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/bounty/light_glw
        blendFunc GL_ONE GL_ONE
    }
}

models/map_objects/bounty/cache_panel
{
    {
        map $lightmap
    }
    {
        map models/map_objects/bounty/cache_panel
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map models/map_objects/bounty/cache_panel_glw
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
    {
        map models/map_objects/bounty/cache_panel_anim
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0 0.5 0 1
    }
}

models/map_objects/bounty/light_ledge_inner
{
    {
        map $lightmap
    }
    {
        map models/map_objects/bounty/light_ledge_inner
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map models/map_objects/bounty/light_ledge_inner_glw
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

models/map_objects/bounty/sconce_light
{
    {
        map $lightmap
    }
    {
        map models/map_objects/bounty/sconce_light
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map models/map_objects/bounty/sconce_light_glw
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

