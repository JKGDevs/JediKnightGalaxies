// *************************************************

// *   s Metashader

// *************************************************

textures/metashader/s_0
{
	qer_editorimage	textures/yavin/s_mud1
	q3map_nolightmap
	cull	twosided
    {
        map textures/yavin/s_mud1
        tcMod scale 0.1 0.1
    }
    {
        map gfx/sprites/ss_hangvine2
            surfaceSprites vertical 10 16 30 300
            ssVariance 1 2
            ssWind 1
        alphaFunc GE192
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        depthWrite
        detail
        rgbGen exactVertex
    }
    {
        clampmap gfx/sprites/ss_bubbles
            surfaceSprites effect 2.5 1 36 400
            ssFademax 700
            ssVariance 0.75 1
            ssFXDuration 2500
            ssFXGrow 2.5 30
            ssFXAlphaRange 0.5 0
            ssFXWeather
        blendFunc GL_ONE GL_ONE
        detail
    }
}

textures/metashader/s_1
{
	qer_editorimage	textures/yavin/groundjungle
	q3map_material	LongGrass
	q3map_nolightmap
	cull	twosided
    {
        map textures/yavin/groundjungle
    }
    {
        map gfx/sprites/ss_grass_grasspatchy
            surfaceSprites vertical 20 24 48 500
            ssFademax 1500
            ssFadescale 1
            ssVariance 1 2
            ssWind 0.5
        alphaFunc GE192
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        depthWrite
        rgbGen exactVertex
    }
    {
        map gfx/sprites/ss_cattail
            surfaceSprites vertical 20 32 50 500
            ssVariance 1 2.5
            ssWind 0.8
        alphaFunc GE192
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        depthWrite
        rgbGen exactVertex
    }
}

textures/metashader/s_2
{
	qer_editorimage	textures/yavin/groundjungle
	q3map_material	LongGrass
	q3map_nolightmap
	cull	twosided
    {
        map textures/yavin/groundjungle
    }
    {
        map gfx/sprites/ss_grass_grasspatchy
            surfaceSprites vertical 32 36 42 500
            ssFademax 1500
            ssFadescale 1
            ssVariance 1 2
            ssWind 0.5
        alphaFunc GE192
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        depthWrite
        rgbGen exactVertex
    }
}

textures/metashader/s_0to1
{
	qer_editorimage	textures/yavin/s_mud1
	q3map_material	LongGrass
	q3map_nolightmap
	cull	twosided
    {
        map textures/yavin/s_mud1
        alphaGen vertex
        tcMod scale 0.1 0.1
    }
    {
        map gfx/sprites/ss_hangvine2
            surfaceSprites vertical 10 16 30 300
            ssVariance 1 2
            ssWind 1
        alphaFunc GE192
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        depthWrite
        detail
        rgbGen exactVertex
    }
    {
        clampmap gfx/sprites/ss_bubbles
            surfaceSprites effect 2.5 1 36 400
            ssFademax 700
            ssVariance 0.75 1
            ssFXDuration 2500
            ssFXGrow 2.5 30
            ssFXAlphaRange 0.5 0
            ssFXWeather
        blendFunc GL_ONE GL_ONE
        detail
    }
    {
        map textures/yavin/groundjungle
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        alphaGen vertex
    }
    {
        map gfx/sprites/ss_grass_grasspatchy
            surfaceSprites vertical 20 24 48 500
            ssFademax 1500
            ssFadescale 1
            ssVariance 1 2
            ssWind 0.5
        alphaFunc GE192
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        depthWrite
        rgbGen exactVertex
    }
    {
        map gfx/sprites/ss_cattail
            surfaceSprites vertical 20 32 50 500
            ssVariance 1 2.5
            ssWind 0.8
        alphaFunc GE192
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        depthWrite
        rgbGen exactVertex
    }
}

textures/metashader/s_0to2
{
	qer_editorimage	textures/yavin/s_mud1
	q3map_material	LongGrass
	q3map_nolightmap
	cull	twosided
    {
        map textures/yavin/s_mud1
        alphaGen vertex
        tcMod scale 0.1 0.1
    }
    {
        map gfx/sprites/ss_hangvine2
            surfaceSprites vertical 10 16 30 300
            ssVariance 1 2
            ssWind 1
        alphaFunc GE192
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        depthWrite
        detail
        rgbGen exactVertex
    }
    {
        clampmap gfx/sprites/ss_bubbles
            surfaceSprites effect 2.5 1 36 400
            ssFademax 700
            ssVariance 0.75 1
            ssFXDuration 2500
            ssFXGrow 2.5 30
            ssFXAlphaRange 0.5 0
            ssFXWeather
        blendFunc GL_ONE GL_ONE
        detail
    }
    {
        map textures/yavin/groundjungle
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        alphaGen vertex
    }
    {
        map gfx/sprites/ss_grass_grasspatchy
            surfaceSprites vertical 32 36 42 500
            ssFademax 1500
            ssFadescale 1
            ssVariance 1 2
            ssWind 0.5
        alphaFunc GE192
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        depthWrite
        rgbGen exactVertex
    }
}

textures/metashader/s_1to2
{
	qer_editorimage	textures/yavin/groundjungle
	q3map_material	LongGrass
	q3map_nolightmap
	cull	twosided
    {
        map textures/yavin/groundjungle
        alphaGen vertex
    }
    {
        map gfx/sprites/ss_grass_grasspatchy
            surfaceSprites vertical 20 24 48 500
            ssFademax 1500
            ssFadescale 1
            ssVariance 1 2
            ssWind 0.5
        alphaFunc GE192
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        depthWrite
        rgbGen exactVertex
    }
    {
        map gfx/sprites/ss_cattail
            surfaceSprites vertical 20 32 50 500
            ssVariance 1 2.5
            ssWind 0.8
        alphaFunc GE192
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        depthWrite
        rgbGen exactVertex
    }
    {
        map textures/yavin/groundjungle
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        alphaGen vertex
    }
    {
        map gfx/sprites/ss_grass_grasspatchy
            surfaceSprites vertical 32 36 42 500
            ssFademax 1500
            ssFadescale 1
            ssVariance 1 2
            ssWind 0.5
        alphaFunc GE192
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        depthWrite
        rgbGen exactVertex
    }
}

textures/metashader/canyon_0
{
	q3map_nolightmap
	q3map_onlyvertexlighting
	q3map_novertexshadows
    {
        map textures/yavin/grasspatchy_underwater
        rgbGen vertex
        tcMod scale 0.25 0.25
    }
}

textures/metashader/canyon_1
{
	q3map_nolightmap
	q3map_onlyvertexlighting
	q3map_novertexshadows
    {
        map textures/yavin/grasspatchy_swampsprite
        rgbGen vertex
        tcMod scale 0.1 0.1
    }
}

textures/metashader/canyon_2
{
	q3map_nolightmap
	q3map_onlyvertexlighting
	q3map_novertexshadows
    {
        map textures/yavin/grasspatchy_reeds
        rgbGen vertex
        tcMod scale 0.1 0.1
    }
}

textures/metashader/canyon_0to1
{
	q3map_nolightmap
	q3map_onlyvertexlighting
	q3map_novertexshadows
    {
        map textures/yavin/grasspatchy_underwater
        rgbGen vertex
        alphaGen vertex
        tcMod scale 0.25 0.25
    }
    {
        map textures/yavin/grasspatchy_swampsprite
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
        alphaGen vertex
        tcMod scale 0.1 0.1
    }
}

textures/metashader/canyon_0to2
{
	q3map_nolightmap
	q3map_onlyvertexlighting
	q3map_novertexshadows
    {
        map textures/yavin/grasspatchy_swampsprite
        rgbGen vertex
        alphaGen vertex
        tcMod scale 0.25 0.25
    }
    {
        map textures/yavin/grasspatchy_reeds
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
        alphaGen vertex
        tcMod scale 0.1 0.1
    }
}

textures/metashader/canyon_1to2
{
	q3map_nolightmap
	q3map_onlyvertexlighting
	q3map_novertexshadows
    {
        map textures/yavin/grasspatchy_swampsprite
        rgbGen vertex
        alphaGen vertex
        tcMod scale 0.1 0.1
    }
    {
        map textures/yavin/grasspatchy_reeds
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
        alphaGen vertex
        tcMod scale 0.1 0.1
    }
}

// /////////////////////////////

// /////////////////////////////

// /////////////////////////////

// /////////////////////////////

// ///////

// SANDCRAWLER Metashader

// //////

textures/metashader/smswoop_0
{
	q3map_onlyvertexlighting
	q3map_novertexshadows
    {
        map textures/yavin/ceiling
        rgbGen vertex
        tcMod scale 0.1 0.1
    }
}

textures/metashader/smswoop_1
{
	q3map_onlyvertexlighting
	q3map_novertexshadows
    {
        map textures/yavin/s_rock3
        rgbGen vertex
        tcMod scale 0.1 0.1
    }
}

textures/metashader/smswoop_2
{
	q3map_onlyvertexlighting
	q3map_novertexshadows
    {
        map textures/yavin/rock2
        rgbGen vertex
        tcMod scale 0.1 0.1
    }
}

textures/metashader/smswoop_0to1
{
	q3map_onlyvertexlighting
	q3map_novertexshadows
    {
        map textures/yavin/ceiling
        rgbGen vertex
        alphaGen vertex
        tcMod scale 0.1 0.1
    }
    {
        map textures/yavin/s_rock3
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
        alphaGen vertex
        tcMod scale 0.1 0.1
    }
}

textures/metashader/smswoop_0to2
{
	q3map_onlyvertexlighting
	q3map_novertexshadows
    {
        map textures/yavin/ceiling
        rgbGen vertex
        alphaGen vertex
        tcMod scale 0.1 0.1
    }
    {
        map textures/yavin/rock2
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
        alphaGen vertex
        tcMod scale 0.1 0.1
    }
}

textures/metashader/smswoop_1to2
{
	q3map_onlyvertexlighting
	q3map_novertexshadows
    {
        map textures/yavin/s_rock3
        rgbGen vertex
        alphaGen vertex
        tcMod scale 0.1 0.1
    }
    {
        map textures/yavin/rock2
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
        alphaGen vertex
        tcMod scale 0.1 0.1
    }
}

