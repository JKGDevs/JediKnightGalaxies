// JKG_ShaderPatches: Yzmo - JKG_Jawa, BlasTech - player model caps

models/players/stormtrooper/caps
{
	q3map_nolightmap
    {
        map gfx/PlayerOverlays/burn
        rgbGen lightingDiffuse
    }
}

  ///////////////////////////////////
  //                               // 
  //   **** Clean Shaders ****     //
  //         Nov. 7th, 2003        //
  //          By Astrocreep        //
  //          -------------        //
  //    For Level Editing info:    //
  //       www.map-center.com      //
  //          -------------        //
  //    For GtkRadiant Updates:    //
  //       www.qeradiant.com       //
  //                               //
  ///////////////////////////////////
 
textures/common/glass
{
	qer_editorimage	textures/common/glass2.tga
	q3map_tesssize	48
	qer_trans	0.5
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_material	Glass
	q3map_nolightmap
    {
        map textures/common/glass2
        blendFunc GL_ZERO GL_ONE_MINUS_SRC_COLOR
    }
    {
        map textures/common/glass2
        blendFunc GL_ONE GL_ONE
        tcGen environment
    }
}

textures/common/glass_no_tess
{
	qer_editorimage	textures/common/glass2.tga
	qer_trans	0.5
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_material	Glass
	q3map_nolightmap
    {
        map textures/common/glass2
        blendFunc GL_ZERO GL_ONE_MINUS_SRC_COLOR
    }
    {
        map textures/common/glass2
        blendFunc GL_ONE GL_ONE
        tcGen environment
    }
}

textures/common/glass_reflective
{
	qer_editorimage	textures/common/glass2.tga
	q3map_tesssize	48
	qer_trans	0.5
	surfaceparm	nonopaque
	surfaceparm	forcefield
	surfaceparm	trans
	q3map_material	Glass
	q3map_nolightmap
    {
        map textures/common/glass2
        blendFunc GL_ZERO GL_ONE_MINUS_SRC_COLOR
    }
    {
// tcMod scale 3 3

        map textures/common/glass2
        blendFunc GL_ONE GL_ONE
        tcGen environment
    }
}

textures/common/glass_reflective_double-sided
{
	qer_editorimage	textures/common/glass2.tga
	q3map_tesssize	48
	qer_trans	0.5
	surfaceparm	nonopaque
	surfaceparm	forcefield
	surfaceparm	trans
	q3map_material	Glass
	q3map_nolightmap
	cull	twosided
    {
        map textures/common/glass2
        blendFunc GL_ZERO GL_ONE_MINUS_SRC_COLOR
    }
    {
        map textures/common/glass2
        blendFunc GL_ONE GL_ONE
        tcGen environment
    }
}

textures/common/crystal
{
	qer_editorimage	textures/common/glass2.tga
	q3map_tesssize	48
	qer_trans	0.5
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_material	Glass
	q3map_nolightmap
    {
        map textures/common/glass2
        blendFunc GL_ONE GL_SRC_COLOR
    }
    {
        map textures/common/glass2
        blendFunc GL_ONE GL_ONE
        tcGen environment
    }
}

// 

// 

textures/common/antiportal
{
	qer_nocarve
	qer_trans	0.3
	surfaceparm	noimpact
	surfaceparm	nodraw
	surfaceparm	antiportal
	surfaceparm	nonsolid
	surfaceparm	trans
	q3map_structural
}

// 

// 

// 

textures/common/skip
{
	qer_nocarve
	qer_trans	0.3
	surfaceparm	noimpact
	surfaceparm	nodraw
	surfaceparm	skip
	surfaceparm	nonsolid
	surfaceparm	trans
	q3map_structural
}

textures/common/env_glass
{
	qer_editorimage	textures/common/env_metallic
	qer_trans	0.6
	surfaceparm	nonopaque
	surfaceparm	forcefield
	surfaceparm	trans
	q3map_material	Glass
	q3map_nolightmap
    {
        map textures/common/env_metallic
        blendFunc GL_ONE GL_ONE
        tcGen environment
    }
}

textures/common/env_glass_breakable
{
// surfaceparm	forcefield

	qer_editorimage	textures/common/env_metallic
	qer_trans	0.6
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_material	Glass
	q3map_nolightmap
    {
        map textures/common/env_metallic
        blendFunc GL_ONE GL_ONE
        tcGen environment
    }
}

textures/common/env_glass_green
{
	qer_editorimage	textures/common/env_metallic_green
	surfaceparm	nonopaque
	surfaceparm	forcefield
	surfaceparm	trans
	q3map_material	Glass
	q3map_nolightmap
    {
        map textures/common/env_metallic_green
        blendFunc GL_ONE GL_ONE
        tcGen environment
    }
}

textures/common/env_glass_red
{
	qer_editorimage	textures/common/env_metallic_red
	surfaceparm	nonopaque
	surfaceparm	forcefield
	surfaceparm	trans
	q3map_material	Glass
	q3map_nolightmap
    {
        map textures/common/env_metallic_red
        blendFunc GL_ONE GL_ONE
        tcGen environment
    }
}

textures/common/gradient
{
	surfaceparm	noimpact
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	trans
	q3map_nolightmap
    {
        clampmap textures/common/gradient
        alphaFunc GT0
        blendFunc GL_ONE GL_ONE
        tcMod scroll 8 0
    }
}

textures/common/gradient2
{
	qer_editorimage	textures/common/gradient2
	surfaceparm	noimpact
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	trans
	q3map_nolightmap
    {
        clampmap textures/common/gradient2
        blendFunc GL_ONE GL_ONE
        rgbGen const ( 0.752941 0.752941 0.752941 )
        tcMod scroll 8 0
    }
}

textures/common/water_1
{
	qer_editorimage	textures/common/water_1
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	water
	surfaceparm	trans
	q3map_material	Water
	q3map_onlyvertexlighting
	deformvertexes	wave	100 sin 1 2 2 0.6
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/common/water_1
        blendFunc GL_DST_COLOR GL_ZERO
        tcMod turb 0.01 0.03 5 0.1
        tcMod stretch noise 1 0.01 1 1
    }
    {
        map textures/common/water_1
        blendFunc GL_ONE GL_ONE
        tcMod turb 0.02 -0.06 0 0.1
        tcMod stretch noise 4 1 0 0.01
    }
    {
        map textures/common/stars
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0 1 10 0.5
    }
}

textures/common/env_glass_opaque
{
// surfaceparm	trans

	qer_editorimage	textures/common/env_metallic
	surfaceparm	forcefield
	q3map_material	Glass
	q3map_nolightmap
    {
        map textures/common/env_metallic
        rgbGen const ( 1.000000 1.000000 1.000000 )
        tcGen environment
    }
}

textures/common/env_glass_equal
{
	qer_editorimage	textures/common/env_metallic
	surfaceparm	nonopaque
	surfaceparm	forcefield
	surfaceparm	trans
	q3map_material	Glass
	q3map_nolightmap
    {
        map textures/common/env_metallic
        blendFunc GL_ONE GL_ONE
        rgbGen const ( 0.501961 0.501961 0.501961 )
        tcGen environment
    }
}

textures/common/neon_purple
{
	qer_editorimage	textures/common/gradient3
	surfaceparm	noimpact
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	trans
	q3map_nolightmap
    {
        map textures/common/gradient3
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen const ( 0.501961 0.000000 0.501961 )
        tcMod stretch noise 1 0.1 0 1
    }
}

textures/common/water2
{
	qer_editorimage	textures/common/direction
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	water
	surfaceparm	trans
	q3map_material	Water
    {
        map textures/common/water3
        blendFunc GL_ONE GL_SRC_ALPHA
        rgbGen const ( 0.752941 0.752941 0.752941 )
        alphaGen const 0.7
        tcMod scroll 0 -1
        tcMod turb 0.5 0.03 0 0.3
    }
    {
        map textures/common/water3
        blendFunc GL_ONE GL_SRC_ALPHA
        rgbGen const ( 0.752941 0.752941 0.752941 )
        alphaGen const 0.7
        tcMod scroll 0 -0.2
        tcMod turb 0.5 0.03 0 0.3
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/common/stars
        blendFunc GL_ONE GL_ONE
        tcMod scroll 0 1
    }
}

textures/common/caustic00
{
	surfaceparm	trans
	q3map_material	Water
	q3map_nolightmap
    {
        map textures/common/water3
        blendFunc GL_ONE GL_ONE
        tcMod scroll 0 0.3
    }
    {
        animMap 8 textures/common/caustic00 textures/common/caustic01 textures/common/caustic02 textures/common/caustic03 textures/common/caustic04 textures/common/caustic05 textures/common/caustic06 textures/common/caustic07 textures/common/caustic08 textures/common/caustic09 textures/common/caustic10 textures/common/caustic11 textures/common/caustic12 textures/common/caustic13 textures/common/caustic14 textures/common/caustic15 textures/common/caustic16 textures/common/caustic17 textures/common/caustic18 textures/common/caustic19 textures/common/caustic20 textures/common/caustic21 textures/common/caustic22 textures/common/caustic23 textures/common/caustic24 textures/common/caustic25 textures/common/caustic26 textures/common/caustic27 textures/common/caustic28 textures/common/caustic29 textures/common/caustic30 textures/common/caustic31 
        blendFunc GL_ONE_MINUS_SRC_ALPHA GL_ONE
        tcMod turb 1 0.02 0 0.1
        tcMod stretch sin 1 0.02 0 0.2
        tcMod scroll 0 0.1
    }
    {
        map textures/common/stars
        blendFunc GL_ONE GL_ONE
        tcMod turb 0 1 1 1
    }
}

textures/common/neon_green
{
	qer_editorimage	textures/common/gradient3
	surfaceparm	noimpact
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	trans
	q3map_nolightmap
    {
        map textures/common/gradient3
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen const ( 0.000000 1.000000 0.000000 )
        tcMod scroll 8 0
    }
}

textures/common/neon_red
{
	qer_editorimage	textures/common/gradient3
	surfaceparm	noimpact
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	trans
	q3map_nolightmap
    {
        map textures/common/gradient3
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen const ( 0.874510 0.000000 0.000000 )
        tcMod scroll 8 0
    }
}

textures/common/neon_orange
{
	qer_editorimage	textures/common/gradient3
	surfaceparm	noimpact
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	trans
	q3map_nolightmap
    {
        map textures/common/gradient3
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen const ( 1.000000 0.501961 0.000000 )
        tcMod scroll 8 0
    }
}

textures/common/neon_blue
{
	qer_editorimage	textures/common/gradient3
	surfaceparm	noimpact
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	trans
	q3map_nolightmap
    {
        map textures/common/gradient3
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen const ( 0.000000 0.000000 1.000000 )
        tcMod scroll 8 0
    }
    {
        map textures/common/blue_dot
        blendFunc GL_ONE GL_ONE
        glow
        tcMod scroll 1 0
    }
}

textures/common/water2_waterfall
{
	qer_editorimage	textures/common/direction
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_material	Water
	q3map_nolightmap
    {
        map textures/common/water3_joe
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        tcMod scroll 0 -1
    }
    {
        map textures/common/water3
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        alphaGen const 0.7
        tcMod scroll 0 -0.5
    }
    {
        map textures/common/stars
        blendFunc GL_ONE GL_ONE
        tcMod scale 0.25 0.25
        tcMod scroll 0 -0.5
    }
}

textures/common/water2_shadow
{
	qer_editorimage	textures/common/direction
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	water
	surfaceparm	trans
	q3map_material	Water
	q3map_nolightmap
    {
        map textures/yavin/lshadow
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen vertex
        tcMod turb 2 0.05 0 0.1
    }
    {
        map textures/common/water3
        blendFunc GL_ONE GL_SRC_ALPHA
        rgbGen const ( 0.423529 0.423529 0.423529 )
        alphaGen const 0.7
        tcMod turb 0.5 0.03 0 0.3
        tcMod scroll 0 -0.25
    }
    {
        map textures/common/stars
        blendFunc GL_ONE GL_ONE
        tcMod scroll 0 -1
    }
}

textures/common/water2_water
{
	qer_editorimage	textures/common/direction
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	water
	surfaceparm	trans
	q3map_material	Water
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map textures/yavin/lshadow
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen vertex
        tcMod turb 2 0.05 0 0.1
    }
    {
        map textures/common/water3
        blendFunc GL_ONE GL_SRC_ALPHA
        rgbGen const ( 0.423529 0.423529 0.423529 )
        alphaGen const 0.7
        tcMod turb 0.5 0.03 0 0.3
        tcMod scroll 0 -0.25
    }
    {
        map textures/common/stars
        blendFunc GL_ONE GL_ONE
        tcMod scroll 0 -1
    }
}

// 

textures/common/water2_waterfall_Tutorial
{
	qer_editorimage	textures/common/direction
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_material	Water
    {
        map textures/common/water3
        blendFunc GL_ONE GL_SRC_ALPHA
        rgbGen const ( 0.752941 0.752941 0.752941 )
        alphaGen const 0.2
        tcMod scroll 0 -1
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/common/stars
        blendFunc GL_ONE GL_ONE
        tcMod scroll 0 -1
    }
}

textures/common/water2_water1
{
	qer_editorimage	textures/common/direction
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	water
	surfaceparm	trans
	q3map_material	Water
	q3map_nolightmap
	q3map_onlyvertexlighting
	sort	seeThrough
    {
        map textures/yavin/lshadow
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen vertex
        tcMod turb 2 0.05 0 0.1
    }
    {
        map textures/common/water3
        blendFunc GL_ONE GL_SRC_ALPHA
        rgbGen const ( 0.423529 0.423529 0.423529 )
        alphaGen const 0.7
        tcMod turb 0.5 0.03 0 0.3
        tcMod scroll 0 -0.25
    }
    {
        map textures/common/stars
        blendFunc GL_ONE GL_ONE
        tcMod scroll 0 -1
    }
}

textures/common/water2_still
{
	qer_editorimage	textures/common/direction
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	water
	surfaceparm	trans
	q3map_material	Water
    {
        map textures/common/water3
        blendFunc GL_ONE GL_SRC_ALPHA
        alphaGen const 0.6
        tcMod turb 0.5 0.03 0 0.3
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/common/stars
        blendFunc GL_ONE GL_ONE
        tcMod scroll 0 -1
    }
}

textures/common/dust_animated
{
	qer_editorimage	textures/common/dust_base.jpg
	q3map_tesssize	48
	qer_trans	0.5
	surfaceparm	noimpact
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
    {
        map textures/common/dust3
        blendFunc GL_ONE GL_ONE
        rgbGen const ( 0.752941 0.752941 0.752941 )
        tcMod scroll 0.5 0.3
    }
}

textures/common/env_glass_tess
{
	qer_editorimage	textures/common/env_metallic
	q3map_tesssize	32
	surfaceparm	nonopaque
	surfaceparm	forcefield
	surfaceparm	trans
	q3map_material	Glass
	q3map_nolightmap
    {
        map textures/common/env_metallic
        blendFunc GL_ONE GL_ONE
        tcGen environment
    }
}

textures/common/env_glass_t2dp1
{
	qer_editorimage	textures/common/environ10
	q3map_tesssize	64
	surfaceparm	nonopaque
	surfaceparm	forcefield
	surfaceparm	trans
	q3map_material	Glass
	q3map_nolightmap
    {
        map textures/common/environ10
        blendFunc GL_ONE GL_ONE
        tcGen environment
        tcMod transform 1 1 1 2 0 0
    }
}

textures/common/dust_gradient
{
	qer_editorimage	textures/common/gradient
	qer_trans	0.5
	surfaceparm	noimpact
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
    {
        clampmap textures/common/gradient
        blendFunc GL_ONE GL_ONE
        rgbGen const ( 0.501961 0.501961 0.501961 )
    }
}

textures/common/dark_dust
{
	qer_editorimage	textures/common/gradient
	qer_trans	0.5
	surfaceparm	noimpact
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
    {
        clampmap textures/common/gradient
        blendFunc GL_ONE GL_ONE
        rgbGen const ( 0.141176 0.141176 0.141176 )
    }
}

textures/common/blue_gradient
{
	qer_editorimage	textures/common/gradient
	qer_trans	0.5
	surfaceparm	noimpact
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
    {
        clampmap textures/common/gradient
        blendFunc GL_ONE GL_ONE
        rgbGen const ( 0.549020 0.862745 0.984314 )
    }
}

textures/common/tan_gradient
{
	qer_editorimage	textures/common/gradient
	qer_trans	0.5
	surfaceparm	noimpact
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
    {
        clampmap textures/common/gradient
        blendFunc GL_ONE GL_ONE
        rgbGen const ( 0.807843 0.717647 0.576471 )
    }
}

textures/common/env_obsidian
{
	qer_editorimage	textures/common/env_metallic
	surfaceparm	forcefield
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map textures/colors/black
        rgbGen vertex
    }
    {
        map textures/common/environ9a
        blendFunc GL_ONE GL_ONE
        rgbGen const ( 0.439216 0.439216 0.439216 )
        tcGen environment
        tcMod scale 0.25 0.25
    }
}

textures/common/flava1
{
	qer_editorimage	textures/common/flava1
	surfaceparm	lava
	q3map_nolightmap
    {
        map textures/common/flava2
        tcMod turb 1 0.05 0.5 0.02
        tcMod scroll -0.01 0.03
    }
    {
        map textures/common/flava1
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        tcMod turb 1 0.1 0 0.05
        tcMod scroll 0 0.03
    }
    {
        map textures/common/frbl
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        tcMod turb 1 0.1 0 0.04
        tcMod scroll 0 0.02
    }
}

textures/common/black_trans_blend
{
	qer_editorimage	textures/common/black_trans_blend
	polygonOffset
	q3map_nolightmap
    {
        map textures/common/black_trans_blend
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

textures/common/gradient_twosided
{
	qer_editorimage	textures/common/gradient
	qer_trans	0.5
	surfaceparm	noimpact
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0 0 0 ) 0.0
	cull	twosided
    {
        clampmap textures/common/gradient
        blendFunc GL_ONE GL_ONE
        rgbGen const ( 0.501961 0.501961 0.501961 )
    }
}

textures/common/env_obsidian2
{
	qer_editorimage	textures/common/env_metallic
	surfaceparm	forcefield
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map textures/colors/black
        rgbGen vertex
    }
    {
        map textures/common/environ9a
        blendFunc GL_ONE GL_ONE
        rgbGen const ( 0.321569 0.321569 0.321569 )
        tcGen environment
        tcMod scale 0.25 0.25
    }
}

textures/common/water_waterfall_3
{
	qer_editorimage	textures/common/particles01
	q3map_material	Water
	q3map_nolightmap
    {
        map textures/common/particles08
        blendFunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
        tcMod scroll 0 -0.3
    }
    {
        map textures/common/particles01
        blendFunc GL_ONE GL_ONE_MINUS_SRC_COLOR
        rgbGen identity
        tcMod scroll 0 -0.25
    }
    {
        map textures/common/particles02
        blendFunc GL_ONE GL_ONE_MINUS_SRC_COLOR
        rgbGen identity
        tcMod scroll 0 -0.4
    }
    {
        map textures/common/particles04
        blendFunc GL_ONE GL_ONE_MINUS_SRC_COLOR
        rgbGen identity
        tcMod scroll 0 -0.2
    }
}

textures/common/water_waterfall_2
{
	qer_editorimage	textures/common/particles01
	q3map_nolightmap
    {
        map textures/common/particles11
        blendFunc GL_ONE GL_SRC_COLOR
        rgbGen vertex
        tcMod scroll 0 -0.3
    }
    {
        map textures/common/particles01
        blendFunc GL_ONE GL_ONE_MINUS_SRC_COLOR
        rgbGen identity
        tcMod scroll 0 -0.25
    }
    {
        map textures/common/particles02
        blendFunc GL_ONE GL_ONE_MINUS_SRC_COLOR
        rgbGen identity
        tcMod scroll 0 -0.4
    }
    {
        map textures/common/particles04
        blendFunc GL_ONE GL_ONE_MINUS_SRC_COLOR
        rgbGen identity
        tcMod scroll 0 -0.2
    }
}

textures/common/nightbeam
{
	qer_editorimage	textures/common/gradient
	qer_trans	0.5
	surfaceparm	noimpact
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
    {
        clampmap textures/common/gradient
        blendFunc GL_ONE GL_ONE
        rgbGen const ( 0.145098 0.180392 0.321569 )
    }
}

textures/common/lvr
{
	qer_editorimage	textures/common/flava1
	surfaceparm	lava
	q3map_nolightmap
    {
        map textures/common/frbl
        tcMod turb 1 0.05 0.5 0.04
        tcMod stretch sin 1 0.01 0 0.1
        tcMod scroll -0.01 0.01
    }
    {
        map textures/common/l4s
        blendFunc GL_DST_COLOR GL_SRC_ALPHA
        tcMod turb 0.5 0.1 0 0.05
        tcMod scroll 0 0.03
        tcMod stretch sin 1 0.02 0 0.02
    }
}

textures/common/lvr2
{
	qer_editorimage	textures/common/flava1
	surfaceparm	lava
	q3map_nolightmap
    {
        map textures/common/l4s
        tcMod turb 0.5 0.1 0 0.05
        tcMod scroll 0 0.03
        tcMod stretch sin 1 0.02 0 0.02
    }
    {
        map textures/common/frbl
        blendFunc GL_SRC_ALPHA GL_SRC_COLOR
        tcMod turb 1 0.05 0.5 0.04
        tcMod stretch sin 1 0.01 0 0.1
        tcMod scroll -0.01 0.01
    }
}

textures/common/dark_yellow_dust
{
	qer_editorimage	textures/common/gradient
	qer_trans	0.9
	surfaceparm	noimpact
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_material	Water
	q3map_nolightmap
    {
        clampmap textures/common/gradient
        blendFunc GL_ONE GL_ONE
        rgbGen const ( 0.349020 0.345098 0.168627 )
    }
}

textures/common/env_glass_security
{
	qer_editorimage	textures/common/security_glass
	qer_trans	0.9
	surfaceparm	nonopaque
	surfaceparm	forcefield
	surfaceparm	trans
	q3map_material	Glass
	q3map_nolightmap
    {
        map textures/common/env_metallic
        blendFunc GL_ONE GL_ONE
        tcGen environment
    }
    {
        map textures/common/security_glass
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

textures/common/glass_security_hex
{
	qer_editorimage	textures/common/sglass1
	qer_trans	0.9
	surfaceparm	nonopaque
	surfaceparm	forcefield
	surfaceparm	trans
	q3map_material	Glass
	q3map_nolightmap
    {
        map textures/common/env_metallic
        blendFunc GL_ONE GL_ONE
        tcGen environment
    }
    {
        map textures/common/sglass1
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

textures/common/glass_security_chain
{
	qer_editorimage	textures/common/sglass2
	qer_trans	0.9
	surfaceparm	nonopaque
	surfaceparm	forcefield
	surfaceparm	trans
	q3map_material	Glass
	q3map_nolightmap
    {
        map textures/common/env_metallic
        blendFunc GL_ONE GL_ONE
        tcGen environment
    }
    {
        map textures/common/sglass2
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

textures/common/glass_security_tris
{
	qer_editorimage	textures/common/sglass3
	qer_trans	0.9
	surfaceparm	nonopaque
	surfaceparm	forcefield
	surfaceparm	trans
	q3map_material	Glass
	q3map_nolightmap
    {
        map textures/common/env_metallic
        blendFunc GL_ONE GL_ONE
        tcGen environment
    }
    {
        map textures/common/sglass3
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

textures/common/glass_security_thex
{
	qer_editorimage	textures/common/sglass4
	qer_trans	0.9
	surfaceparm	nonopaque
	surfaceparm	forcefield
	q3map_material	Glass
	q3map_nolightmap
    {
        map textures/common/sglass4
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map textures/common/env_metallic
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        alphaGen const 0.5
        tcGen environment
    }
}

textures/common/glass_security_square
{
	qer_editorimage	textures/common/sglass5
	qer_trans	0.9
	surfaceparm	nonopaque
	surfaceparm	forcefield
	surfaceparm	trans
	q3map_material	Glass
	q3map_nolightmap
    {
        map textures/common/env_metallic
        blendFunc GL_ONE GL_ONE
        tcGen environment
    }
    {
        map textures/common/sglass5
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

textures/common/neonblue_gradient
{
	qer_editorimage	textures/common/gradient
	qer_trans	0.5
	surfaceparm	noimpact
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
    {
        clampmap textures/common/gradient
        blendFunc GL_ONE GL_ONE
        rgbGen const ( 0.247059 0.447059 0.811765 )
    }
}

textures/common/water2_water1_vjun1
{
	qer_editorimage	textures/common/direction
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	slime
	surfaceparm	water
	surfaceparm	trans
	q3map_material	Water
	q3map_nolightmap
	q3map_onlyvertexlighting
	sort	decal
	cull	twosided
    {
        map textures/yavin/lshadow
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen vertex
        tcMod turb 2 0.05 0 0.1
    }
    {
        map textures/common/water3_green
        blendFunc GL_ONE GL_SRC_ALPHA
        rgbGen const ( 0.423529 0.423529 0.423529 )
        alphaGen const 0.7
        tcMod turb 0.5 0.03 0 0.3
        tcMod scroll 0 -0.25
    }
    {
        map textures/common/stars
        blendFunc GL_ONE GL_ONE
        tcMod scroll 0 -1
    }
}

// 

textures/common/Water_Yavin2
{
	qer_editorimage	textures/common/direction
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	water
	surfaceparm	trans
	q3map_material	Water
	cull	twosided
    {
        map textures/common/water3
        blendFunc GL_ONE GL_SRC_ALPHA
        rgbGen const ( 0.752941 0.752941 0.752941 )
        alphaGen const 0.2
        tcMod scroll 0 -0.1
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/common/stars
        blendFunc GL_ONE GL_ONE
        tcMod scroll 0 -0.15
    }
}

// 

textures/common/water_3
{
	qer_editorimage	textures/common/water_1
	qer_trans	0.9
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	water
	surfaceparm	trans
	q3map_material	Water
	deformvertexes	wave	100 sin 1 2.5 2 0.7
    {
        map textures/common/water_1
        blendFunc GL_ONE GL_SRC_ALPHA
        rgbGen const ( 0.752941 0.752941 0.752941 )
        alphaGen const 0.2
        tcMod turb 0.5 0.04 0 0.3
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/common/stars
        blendFunc GL_ONE GL_ONE
        tcMod turb 0.4 0.04 0 0.31
    }
}

textures/common/dark_orange
{
	qer_editorimage	textures/common/gradient
	qer_trans	0.5
	surfaceparm	noimpact
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
    {
        clampmap textures/common/gradient
        blendFunc GL_ONE GL_ONE
        rgbGen const ( 0.490196 0.321569 0.090196 )
    }
}

textures/common/red_gradient
{
	qer_editorimage	textures/common/gradient
	qer_trans	0.5
	surfaceparm	noimpact
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
    {
        clampmap textures/common/gradient
        blendFunc GL_ONE GL_ONE
        rgbGen const ( 0.368627 0.000000 0.000000 )
    }
}

textures/common/glass_security_opaque
{
	qer_editorimage	textures/common/sglass3
	qer_trans	0.9
	surfaceparm	forcefield
	q3map_material	Glass
	q3map_nolightmap
    {
        map textures/common/env_metallic
        tcGen environment
    }
    {
        map textures/common/sglass3
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

textures/common/water_quicktrip
{
	qer_editorimage	textures/common/direction
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	water
	surfaceparm	trans
	q3map_material	Water
	q3map_nolightmap
	q3map_onlyvertexlighting
	sort	seeThrough
    {
        map textures/common/water3
        blendFunc GL_ONE_MINUS_DST_COLOR GL_ONE
        rgbGen vertex
        tcMod turb 0.5 0.03 0 0.3
    }
}

textures/common/frosted_glass
{
	qer_editorimage	textures/rail/frosted_glass
	q3map_tesssize	48
	qer_trans	0.5
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_material	Glass
	q3map_nolightmap
    {
        map textures/rail/frosted_glass
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map textures/common/glass2
        blendFunc GL_ONE GL_ONE
        tcGen environment
    }
}

textures/common/nightbeam_byss
{
	qer_editorimage	textures/common/gradient
	qer_trans	0.5
	surfaceparm	noimpact
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
    {
        clampmap textures/common/gradient
        blendFunc GL_ONE GL_ONE
        rgbGen const ( 0.227451 0.494118 0.788235 )
        tcMod scroll 0 0.8
    }
}

textures/common/water_vjun
{
	q3map_material	Water
	q3map_nolightmap
    {
        map textures/common/water_vjun
        rgbGen vertex
        tcMod turb 0 0.04 0 0.1
        tcMod scroll 0.03 0.01
    }
}

//
// q3map debug shaders 
//

//
// q3map lightgrid bounds
//
// the min/max bounds of brushes with this shader in a map
// will define the bounds of the map's lightgrid (model lighting)
// note: make it as small as possible around player space
// to minimize bsp size and compile time
//

textures/common/lightgrid
{
	qer_trans 0.5
	surfaceparm nodraw
	surfaceparm nolightmap
	surfaceparm nonsolid
	surfaceparm detail
	surfaceparm nomarks
	surfaceparm trans
	surfaceparm lightgrid
}

// enable with -debugsurfaces switch
debugsurfaces
{
	surfaceparm trans
	surfaceparm nolightmap
	surfaceparm nonsolid
	surfaceparm noimpact
	surfaceparm nomarks
	sort opaque
	{
		map *default
		rgbGen vertex
	}
}

// enable with -debugportals switch
debugportals
{
	surfaceparm trans
	surfaceparm nolightmap
	surfaceparm nonsolid
	surfaceparm noimpact
	surfaceparm nomarks
	sort additive
	cull none
	{
		map $whiteimage
		blendFunc GL_SRC_ALPHA GL_ONE
		rgbGen vertex
	}
}

