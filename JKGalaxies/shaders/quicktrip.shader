// ///////////////////////////

// ***Indexmap***//

// ///////////////////////////

textures/quicktrip/desert_0
{
	q3map_shadeangle	150
	qer_editorimage	textures/quicktrip/sand
	q3map_material	Gravel
	q3map_nolightmap
	q3map_nonplanar
	q3map_onlyvertexlighting
	q3map_tcGen	ivector ( 1024 0 0 ) ( 0 1024 0 )
    {
// tcmod scale .25 .25

        map textures/quicktrip/sand
        rgbGen vertex
    }
    {
        clampmap textures/quicktrip/shrubbery2
            surfaceSprites flattened 20 32 200 1200
            ssVariance 0.4 1.5
            ssWind 0.75
        alphaFunc GE192
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        depthWrite
        rgbGen vertex
    }
    {
        clampmap textures/quicktrip/shrubbery1
            surfaceSprites flattened 18 32 80 2000
            ssVariance 0.2 0.5
            ssWind 0.25
        alphaFunc GE192
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        depthWrite
        rgbGen vertex
    }
    {
        clampmap textures/quicktrip/shrubbery3
            surfaceSprites flattened 30 28 140 1600
            ssVariance 0.2 0.3
            ssWind 0.5
        alphaFunc GE192
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        depthWrite
        rgbGen vertex
    }
}

textures/quicktrip/desert_1
{
	q3map_shadeangle	150
	qer_editorimage	textures/quicktrip/dirt_sand
	q3map_material	Gravel
	q3map_nolightmap
	q3map_nonplanar
	q3map_onlyvertexlighting
	q3map_tcGen	ivector ( 1024 0 0 ) ( 0 1024 0 )
    {
// tcmod scale .25 .25

        map textures/quicktrip/dirt_sand
        rgbGen vertex
    }
}

textures/quicktrip/desert_2
{
	q3map_shadeangle	150
	qer_editorimage	textures/quicktrip/rock_sandstone
	q3map_nolightmap
	q3map_nonplanar
	q3map_onlyvertexlighting
	q3map_tcGen	ivector ( 1024 0 0 ) ( 0 1024 0 )
    {
// tcmod scale .1 .1

        map textures/quicktrip/rock_sandstone
        rgbGen vertex
    }
}

textures/quicktrip/desert_0to1
{
	q3map_shadeangle	150
	qer_editorimage	textures/quicktrip/sand
	q3map_material	Gravel
	q3map_nolightmap
	q3map_nonplanar
	q3map_onlyvertexlighting
	q3map_tcGen	ivector ( 1024 0 0 ) ( 0 1024 0 )
    {
// tcmod scale .25 .25

        map textures/quicktrip/sand
        rgbGen vertex
        alphaGen vertex
    }
    {
// tcmod scale .25 .25

        map textures/quicktrip/dirt_sand
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
        alphaGen vertex
    }
}

textures/quicktrip/desert_0to2
{
	q3map_shadeangle	150
	qer_editorimage	textures/quicktrip/sand
	q3map_nolightmap
	q3map_nonplanar
	q3map_onlyvertexlighting
	q3map_tcGen	ivector ( 1024 0 0 ) ( 0 1024 0 )
    {
// tcmod scale .25 .25

        map textures/quicktrip/sand
        rgbGen vertex
        alphaGen vertex
    }
    {
// tcmod scale .25 .25

        map textures/quicktrip/rock_sandstone
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
        alphaGen vertex
    }
}

textures/quicktrip/desert_1to2
{
	q3map_shadeangle	150
	qer_editorimage	textures/quicktrip/dirt_sand
	q3map_nolightmap
	q3map_nonplanar
	q3map_onlyvertexlighting
	q3map_tcGen	ivector ( 1024 0 0 ) ( 0 1024 0 )
    {
// tcmod scale .25 .25

        map textures/quicktrip/dirt_sand
        rgbGen vertex
        alphaGen vertex
    }
    {
// tcmod scale .25 .25

        map textures/quicktrip/rock_sandstone
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
        alphaGen vertex
    }
}

// VERTEX LIT TEXTURES

textures/quicktrip/qt_basestone_phong
{
	q3map_shadeangle	190
	qer_editorimage	textures/quicktrip/qt_basestone
	q3map_nolightmap
	q3map_nonplanar
	q3map_splotchfix
	q3map_onlyvertexlighting
    {
        map textures/quicktrip/qt_basestone
        rgbGen vertex
    }
}

textures/quicktrip/qt_stonefloor_phong
{
	q3map_shadeangle	190
	qer_editorimage	textures/quicktrip/qt_stonefloor
	q3map_nolightmap
	q3map_nonplanar
	q3map_splotchfix
	q3map_onlyvertexlighting
    {
        map textures/quicktrip/qt_stonefloor
        rgbGen vertex
    }
}

textures/quicktrip/qt_uncut_phong
{
	q3map_shadeangle	190
	qer_editorimage	textures/quicktrip/qt_uncut
	q3map_nolightmap
	q3map_nonplanar
	q3map_splotchfix
	q3map_onlyvertexlighting
    {
        map textures/quicktrip/qt_uncut
        rgbGen vertex
    }
}

textures/quicktrip/qt_wall2_phong
{
	q3map_shadeangle	190
	qer_editorimage	textures/quicktrip/qt_wall2
	q3map_nolightmap
	q3map_nonplanar
	q3map_splotchfix
	q3map_onlyvertexlighting
    {
        map textures/quicktrip/qt_wall2
        rgbGen vertex
    }
}

textures/quicktrip/decal_grime_top
{
	polygonOffset
	q3map_nolightmap
    {
        map textures/quicktrip/decal_grime_top
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

textures/quicktrip/decal_grime_bttm
{
	polygonOffset
	q3map_nolightmap
    {
        map textures/quicktrip/decal_grime_bttm
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

// VERTEX LIT TEXTURES

textures/quicktrip/sandstone_phong
{
	q3map_shadeangle	190
	qer_editorimage	textures/quicktrip/rock_sandstone
	q3map_nolightmap
	q3map_nonplanar
	q3map_splotchfix
	q3map_onlyvertexlighting
    {
        map textures/quicktrip/rock_sandstone
        rgbGen vertex
    }
}

