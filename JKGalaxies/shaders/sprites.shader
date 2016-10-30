// surfaceSprites <type> <width> <height> <density> <fadedist>

// Type is vertical, oriented, effect

// ssFademax <fademax>

// ssFadescale <fadescale>

// ssVariance <varwidth> <varheight>

// ssHangdown

// ssAnyangle

// ssFaceup

// ssWind <wind>

// ssWindIdle <windidle>

// ssVertSkew <skew>

// ssFXDuration <duration>

// ssFXGrow <growwidth> <growheight>

// ssFXAlphaRange <alphastart> <startend>

// ssFXWeather

// //////////////////////////////////////////////////////

// ////// test cases

// //////////////////////////////////////////////////////

textures/sprites/ss_raintest
{
	qer_editorimage	textures/yavin/brock1
	q3map_material	Concrete
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map textures/yavin/brock1
        blendFunc GL_ONE GL_ZERO
        rgbGen vertex
    }
    {
        map gfx/sprites/rainhit
            surfaceSprites effect 2.5 2.5 16 400
            ssVariance 1 0.75
            ssFXDuration 135
            ssFXGrow 6 6
            ssFXWeather
        blendFunc GL_ONE GL_ONE
    }
    {
        map gfx/sprites/rainring
            surfaceSprites effect 2 2 28 350
            ssVariance 2 1
            ssFaceup
            ssFXDuration 220
            ssFXGrow 2.5 2.5
            ssFXWeather
        blendFunc GL_ONE GL_ONE
    }
}

textures/sprites/ss_test1
{
	qer_editorimage	textures/yavin/grass
	q3map_material	LongGrass
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map textures/yavin/grass
        blendFunc GL_ONE GL_ZERO
        rgbGen vertex
        tcMod scale 0.25 0.25
    }
}

textures/sprites/ss_test2
{
	qer_editorimage	textures/yavin/grass
	q3map_material	LongGrass
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map textures/yavin/grass
        blendFunc GL_ONE GL_ZERO
        rgbGen vertex
        tcMod scale 0.25 0.25
    }
    {
        map gfx/sprites/rock_sm
            surfaceSprites oriented 2 2 40 256
            ssVariance 1 1
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        depthWrite
    }
    {
        map gfx/sprites/rock_md
            surfaceSprites oriented 4 4 80 320
            ssVariance 1 1
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        depthWrite
    }
    {
        map gfx/sprites/rock_lg
            surfaceSprites oriented 6 6 100 400
            ssVariance 1 1
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        depthWrite
    }
}

textures/sprites/ss_test3
{
	qer_editorimage	textures/yavin/grass
	q3map_material	LongGrass
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map textures/yavin/grass
        blendFunc GL_ONE GL_ZERO
        rgbGen vertex
        tcMod scale 0.25 0.25
    }
    {
        map gfx/sprites/ss_hangvine
            surfaceSprites vertical 32 20 40 400
            ssFademax 1500
            ssFadescale 2
            ssVariance 1 2
            ssWind 0.8
        alphaFunc GE192
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        depthWrite
        rgbGen vertex
    }
}

// MCG

textures/sprites/ss_testall
{
	qer_editorimage	textures/yavin/grass
	q3map_material	LongGrass
	q3map_nolightmap
	q3map_onlyvertexlighting
	cull	disable
    {
        map textures/yavin/grass
        blendFunc GL_ONE GL_ZERO
        rgbGen vertex
        tcMod scale 0.25 0.25
    }
    {
        map gfx/sprites/ss_grass_reeds
            surfaceSprites vertical 16 16 32 500
            ssFademax 2000
            ssFadescale 3
            ssVariance 1 1
            ssWind 1
        alphaFunc GE192
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        depthWrite
    }
}

