// JKG_ShaderPatches: BlasTech - Directs fire and electricity shaders

gfx/misc/forceprotect
{
	polygonOffset
	q3map_nolightmap
	sort	decal
    {
        map gfx/PlayerOverlays/carbonite
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen identity
    }
    {
        map envmap_spec
        blendFunc GL_DST_COLOR GL_ONE
        detail
        tcGen environment
    }
}

gfx/effects/fire2
{
	cull	twosided
    {
        map gfx/flames/particle_fire1
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen vertex
    }
}

gfx/effects/fire3
{
	cull	twosided
    {
        map gfx/flames/particle_fire2
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen vertex
    }
}

gfx/effects/fire4
{
	cull	twosided
    {
        map gfx/flames/particle_fire3
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen vertex
    }
}

gfx/effects/white_fire1
{
	cull	twosided
    {
        map gfx/flames/particle_white4
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/effects/white_fire2
{
	cull	twosided
    {
        map gfx/flames/particle_white2
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/effects/white_fire1mult
{
	cull	twosided
    {
        map gfx/flames/particle_white5
        blendFunc GL_DST_COLOR GL_ONE
        rgbGen vertex
    }
}

gfx/effects/white_fire2mult
{
	cull	twosided
    {
        map gfx/flames/particle_white6
        blendFunc GL_DST_COLOR GL_ONE
        rgbGen vertex
    }
}

gfx/effects/forcePush
{
	cull	twosided
    {
        clampMap gfx/effects/force_push
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen vertex
    }
}

gfx/effects/saberFlare
{
	nomipmaps
    {
        map gfx/effects/saberflare
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen vertex
    }
}

gfx/effects/yellow_glow
{
	cull	twosided
    {
        map gfx/lightsabers/glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen const ( 0.988200 0.976500 0.486300 )
    }
    {
        clampMap gfx/effects/force_push
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen vertex
        tcMod rotate -44
        tcMod stretch noise 0.6 0.1 0 4
    }
}

gfx/effects/fire_radial
{
	q3map_nolightmap
	cull	twosided
    {
        map gfx/flames/particle_fire8
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

gfx/misc/blueLine
{
	cull	twosided
    {
        map gfx/electricity/electricity
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/misc/electric
{
	qer_editorimage	gfx/misc/lightning3
	cull	twosided
	deformvertexes	bulge	0 2 0
    {
        map gfx/PlayerOverlays/electricity2
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
        tcMod scroll 0.5 1
        tcMod scale 3 4.5
    }
}

gfx/misc/fullbodyelectric2
{
	cull	twosided
	deformvertexes	bulge	0 0.25 0
    {
        map gfx/PlayerOverlays/electricity
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
        tcMod scroll 0.5 1
        tcMod scale -2 -3.5
    }
}


gfx/misc/colflare
{
	cull	twosided
	deformvertexes	bulge	0 0.25 0
    {
        map gfx/misc/colflare
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

// ___________________________________________________

gfx/effects/shock_ripple
{
	cull	twosided
    {
        map gfx/effects/shock_ripple
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        rgbGen identity
        tcMod scale 3 1
        tcMod scroll 0.9 0
    }
    {
        map gfx/effects/shock_ripple
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        rgbGen identity
        tcMod scale 2 1
        tcMod scroll -0.4 0
    }
}

gfx/effects/sho
{
	cull	twosided
    {
        map gfx/effects/sho
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        rgbGen identity
        tcMod scale 1 0.5
        tcMod scroll 0 2.9
    }
    {
        map gfx/effects/sho
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        rgbGen identity
        tcMod scroll 0 -1.8
    }
}

gfx/effects/shock_ball
{
	cull	twosided
    {
        map gfx/effects/shock_ball
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        rgbGen identity
    }
}

gfx/effects/saberDamageGlow
{
	polygonOffset
	cull	twosided
    {
        map gfx/effects/saberdamageglow
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/effects/cloakedShader
{
    {
        map gfx/effects/chrome2
        blendFunc GL_DST_COLOR GL_ONE
        rgbGen entity
        tcGen environment
        tcMod scroll 0.3 0.2
        tcMod turb 0.6 0.3 0 0.2
    }
}

gfx/effects/wookie1
{
	q3map_nolightmap
	cull	twosided
    {
        map gfx/effects/wookie1
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

gfx/effects/wookie2
{
	polygonOffset
	cull	twosided
    {
        map gfx/effects/wookie2
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen vertex
    }
}

gfx/effects/whiteGlow
{
    {
        map gfx/effects/whiteglow
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/effects/blaster_blob
{
	cull	twosided
    {
        map gfx/effects/blaster_blob
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        alphaGen vertex
    }
}

gfx/effects/bryar_blob
{
	cull	twosided
    {
        map gfx/effects/bryar_blob
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        alphaGen vertex
    }
}

gfx/effects/blasterSideFlash
{
	cull	twosided
    {
        map gfx/effects/blastersideflash
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/effects/blasterFrontFlash
{
	cull	twosided
    {
        map gfx/effects/blasterfrontflash
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/effects/bryarSideFlash
{
	cull	twosided
    {
        map gfx/effects/bryarsideflash
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/effects/bryarFrontFlash
{
	cull	twosided
    {
        map gfx/effects/bryarfrontflash
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/effects/greenSideFlash
{
	cull	twosided
    {
        map gfx/effects/greensideflash
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/effects/greenFrontFlash
{
	cull	twosided
    {
        map gfx/effects/greenfrontflash
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/effects/redLine
{
	cull	twosided
    {
        map gfx/effects/redline
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/effects/blueLine
{
	cull	twosided
    {
        map gfx/effects/blueline
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/effects/redLine2
{
	cull	twosided
    {
        map gfx/mp/redline
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/effects/greenShot
{
	cull	twosided
    {
        map gfx/effects/green_shot
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

gfx/effects/rorangeShot
{
	cull	twosided
    {
        map gfx/effects/rorange_shot
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

gfx/effects/plume1
{
	cull	twosided
    {
        map gfx/effects/plume1
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/effects/plume2
{
	cull	twosided
    {
        map gfx/effects/plume2
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/effects/plume3
{
	cull	twosided
    {
        map gfx/effects/plume3
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/effects/light_cone
{
	cull	twosided
    {
        map gfx/effects/light_cone
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
        tcMod scale 3 1
        tcMod scroll 0.3 0
    }
    {
        map gfx/effects/light_cone
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
        tcMod scale 4 1
        tcMod scroll -0.1 0
    }
}

gfx/effects/lava
{
	cull	twosided
    {
        map gfx/effects/lava
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
        alphaGen vertex
    }
}

gfx/effects/lava2
{
	cull	twosided
    {
        map gfx/effects/lava2
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
        alphaGen vertex
    }
}

gfx/effects/rocket_muz
{
	cull	twosided
    {
        map gfx/effects/rocket_muz
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/effects/rocket_muz2
{
	cull	twosided
    {
        map gfx/effects/rocket_muz2
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/effects/irid_shield
{
	cull	twosided
    {
        map gfx/effects/mp_weapon_holo2
        blendFunc GL_ONE GL_ONE
        rgbGen const ( 0.200000 0.200000 0.200000 )
        tcMod scroll 0 -0.2
        tcMod scale 2 6
    }
    {
        map gfx/effects/mp_weapon_holo2
        blendFunc GL_SRC_ALPHA GL_ONE
        rgbGen identity
        alphaGen lightingSpecular
        tcMod scroll 0 -0.3
        tcMod scale 2 10
    }
}

gfx/effects/slime1
{
	cull	twosided
    {
        map gfx/effects/slime1
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
        alphaGen vertex
    }
}

gfx/effects/slime2
{
	cull	twosided
    {
        map gfx/effects/slime2
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
        alphaGen vertex
    }
}

gfx/effects/shard
{
	cull	twosided
    {
        map gfx/effects/shard
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
        alphaGen vertex
    }
}

gfx/effects/drained
{
	cull	twosided
    {
        map gfx/effects/drained
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

gfx/effects/drainedadd
{
	cull	twosided
    {
        map gfx/effects/drainedadd
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen vertex
    }
}

gfx/effects/ripple
{
	cull	twosided
    {
        map gfx/effects/ripple
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/effects/protectionfield
{
	cull	twosided
    {
        map gfx/effects/protectionfield
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/effects/water_splash
{
	cull	twosided
    {
        map gfx/effects/water_splash
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/effects/gcircle
{
	q3map_nolightmap
	cull	twosided
    {
        map gfx/effects/gcircle
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/effects/burst
{
	q3map_nolightmap
	cull	twosided
    {
        map gfx/effects/burst
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/effects/snowpuff2
{
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	cull	twosided
    {
        map gfx/effects/snowpuff2
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/effects/ftail
{
	surfaceparm	trans
	q3map_nolightmap
	cull	twosided
    {
        map gfx/effects/ftail
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/effects/ftail1
{
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
	cull	twosided
    {
        map gfx/effects/ftail1
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/effects/mring1
{
	q3map_nolightmap
	cull	twosided
    {
        map gfx/effects/mring1
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/effects/cmuzzle
{
	surfaceparm	nonopaque
	surfaceparm	trans
	cull	twosided
    {
        map gfx/effects/cmuzzle
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen vertex
    }
}

gfx/effects/caustic1
{
	surfaceparm	trans
	q3map_nolightmap
    {
        map gfx/effects/caustic2
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/effects/tractor
{
    {
        map gfx/effects/tractor
        blendFunc GL_ONE GL_SRC_COLOR
        tcMod scroll -0.5 0
    }
}

gfx/effects/hyper01
{
	q3map_nolightmap
    {
        map gfx/effects/hyper01
        blendFunc GL_ONE GL_ZERO
        tcMod scroll 1 0
        tcMod turb 0 0.25 0 0.25
    }
    {
        map gfx/effects/hyper02
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        tcMod scroll 1.5 0
        tcMod turb 0 0.1 0 0.1
    }
}

gfx/effects/engine
{
	cull	twosided
    {
        map gfx/effects/engine
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen vertex
    }
}

gfx/effects/puffdark
{
	qer_editorimage	gfx/effects/snowpuff2
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	cull	twosided
    {
        map gfx/effects/darkpuff
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

gfx/effects/Fpuff
{
	qer_editorimage	gfx/effects/snowpuff2
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	cull	twosided
    {
        map gfx/effects/fpuff
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/effects/blaster_blob_gs
{
	cull	twosided
    {
        map gfx/effects/blaster_blob_gs
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        alphaGen vertex
    }
}

gfx/effects/Wcloud
{
	qer_editorimage	gfx/effects/snowpuff2
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	cull	twosided
    {
        map gfx/effects/wcloud
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/effects/hspit
{
	qer_editorimage	gfx/effects/snowpuff2
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	cull	twosided
    {
        map gfx/effects/hspit
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen vertex
    }
}

gfx/effects/gb_dot
{
	q3map_nolightmap
	cull	twosided
    {
        map gfx/effects/gb_dot
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

gfx/effects/jk_mist
{
	q3map_nolightmap
    {
        map gfx/effects/jk_mist
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/effects/FL1
{
	qer_editorimage	gfx/effects/fire4
	cull	twosided
    {
        map gfx/effects/fl1
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen vertex
    }
}

gfx/effects/FL2
{
	qer_editorimage	gfx/effects/fire4
	cull	twosided
    {
        map gfx/effects/fl2
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen vertex
    }
}

gfx/effects/FL3
{
	qer_editorimage	gfx/effects/fire4
	cull	twosided
    {
        map gfx/effects/fl3
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen vertex
    }
}

gfx/effects/FL4
{
	qer_editorimage	gfx/effects/fire4
	cull	twosided
    {
        map gfx/effects/fl4
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen vertex
    }
}

gfx/effects/FL5
{
	qer_editorimage	gfx/effects/fire4
	cull	twosided
    {
        map gfx/effects/flc1
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen vertex
    }
}

gfx/effects/FL6
{
	qer_editorimage	gfx/effects/fire4
	cull	twosided
    {
        map gfx/effects/flc2
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen vertex
    }
}

gfx/effects/jk_mist_alpha
{
	qer_editorimage	gfx/effects/jk_mist
	q3map_nolightmap
    {
        map gfx/effects/jk_mist_alpha
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen lightingDiffuse
    }
}

gfx/effects/WcloudA
{
	qer_editorimage	gfx/effects/wclouda
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	cull	twosided
    {
        map gfx/effects/wclouda
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

gfx/effects/eplosion_wave
{
	cull	twosided
    {
        map gfx/effects/eplosion_wave
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/effects/hyper_stars
{
	qer_editorimage	textures/common/stars
	q3map_nolightmap
	cull	twosided
    {
        map textures/common/stars
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
        tcMod scale 0.25 0.25
    }
}

gfx/effects/metal_shard
{
	q3map_nolightmap
    {
        map gfx/effects/metal_shard
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map gfx/effects/metal_shard_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

gfx/effects/scorch
{
	polygonOffset
	q3map_nolightmap
    {
        map gfx/effects/scorch
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        clampmap gfx/effects/embers
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen vertex
    }
    {
        clampmap gfx/effects/embers
        blendFunc GL_ONE GL_ONE
        glow
        detail
        rgbGen wave sin 0.5 -0.5 0 0.2
    }
}

gfx/effects/scorch_mp
{
	polygonOffset
	q3map_nolightmap
	sort	decal
	cull	twosided
    {
        clampmap gfx/effects/scorch_mp
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        alphaGen vertex
    }
}

gfx/effects/embers
{
	polygonOffset
	q3map_nolightmap
    {
        clampmap gfx/effects/embers
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen vertex
        alphaGen vertex
    }
}

gfx/effects/greenline
{
	qer_editorimage	gfx/effects/redline
	cull	twosided
    {
        map gfx/effects/greenline
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/effects/sd_glow
{
	q3map_nolightmap
	cull	twosided
    {
        map gfx/effects/sd_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

gfx/effects/possessedShell
{
	qer_editorimage	gfx/effects/white_fire1
	deformvertexes	bulge	1 1 0
    {
        map gfx/effects/hyper01
        blendFunc GL_DST_COLOR GL_ONE
        glow
        rgbGen wave noise 1 0.2 0 207
        tcMod rotate 91
        tcMod scroll -15 11
    }
}

gfx/effects/jk_dirt_grey
{
	cull	twosided
    {
        map gfx/effects/jk_dirt_grey
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
        alphaGen vertex
    }
}

gfx/effects/jk_smoke5
{
	cull	twosided
    {
        map gfx/effects/jk_smoke5
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
        alphaGen vertex
    }
}

gfx/effects/jk_terra_chunk2
{
	cull	twosided
    {
        map gfx/effects/jk_terra_chunk2
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
        alphaGen vertex
    }
}

gfx/effects/bp_blob
{
	cull	twosided
    {
        map gfx/effects/bp_blob
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
        alphaGen vertex
    }
}

gfx/effects/specs
{
	cull	twosided
    {
        map gfx/effects/specs
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/effects/jk_smoke
{
	cull	twosided
    {
        map gfx/effects/jk_smoke
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/effects/bp_smoke01
{
	cull	twosided
    {
        map gfx/effects/bp_smoke01
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
        alphaGen vertex
    }
}

gfx/effects/bp_smoke02
{
	cull	twosided
    {
        map gfx/effects/bp_smoke02
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
        alphaGen vertex
    }
}

gfx/misc/black_smoke
{
	cull	twosided
    {
        map gfx/effects/black_smoke
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

gfx/misc/black_smoke2
{
	cull	twosided
    {
        map gfx/effects/black_smoke2
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

gfx/misc/dots
{
	cull	twosided
    {
        map gfx/misc/dots
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/misc/smoke2
{
	cull	twosided
    {
        map gfx/effects/smoke2
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

gfx/misc/personalshield
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

gfx/misc/ion_shield
{
	deformvertexes	bulge	0 12 0
    {
        map gfx/effects/p_shield
        blendFunc GL_DST_COLOR GL_ONE
        rgbGen entity
        tcGen environment
        tcMod rotate 200
        tcMod turb 0.6 0.3 0 0.6
        tcMod scale 5 8
    }
    {
        map gfx/effects/p_shield
        blendFunc GL_DST_COLOR GL_ONE
        rgbGen entity
        tcMod rotate -600
        tcMod scale 4 6
    }
}

gfx/misc/spark_group
{
	cull	twosided
    {
        map gfx/misc/spark_group
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen vertex
    }
}

gfx/misc/smallGlassChunk1
{
	cull	twosided
    {
        map gfx/misc/small_glass1
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/misc/smallGlassChunk2
{
	cull	twosided
    {
        map gfx/misc/small_glass2
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/misc/electric
{
	cull	disable
	deformvertexes	bulge	0 2 0
    {
        map gfx/misc/lightning3
        blendFunc GL_ONE GL_ONE
        rgbGen identity
        tcMod scroll 0.5 1
        tcMod scale 3 4.5
    }
}

gfx/misc/fullbodyelectric2
{
	cull	disable
	deformvertexes	bulge	0 0.25 0
    {
        map gfx/misc/lightning3
        blendFunc GL_ONE GL_ONE
        rgbGen identity
        tcMod scroll 0.5 1
        tcMod scale -2 -3.5
    }
}

gfx/misc/dust
{
	cull	twosided
    {
        map gfx/misc/dust
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/misc/wring1
{
	surfaceparm	nonopaque
	surfaceparm	trans
	cull	twosided
    {
        animMap 8 gfx/misc/wring1 gfx/misc/wring2 gfx/misc/wring3 
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/misc/cring
{
	surfaceparm	trans
	cull	twosided
    {
        map gfx/misc/csparks
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen vertex
    }
}

gfx/misc/csparks
{
	qer_editorimage	gfx/misc/cring
	surfaceparm	trans
	cull	twosided
    {
        map textures/flares/flare_ring2
        blendFunc GL_SRC_ALPHA GL_ONE
        rgbGen vertex
    }
}

gfx/misc/spikeflare
{
	qer_editorimage	gfx/misc/cring
	surfaceparm	trans
	cull	twosided
    {
        map gfx/misc/cring
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen vertex
    }
}

gfx/misc/bluebolt
{
	qer_editorimage	gfx/misc/blueline
	cull	twosided
    {
        map gfx/misc/blueline1
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
        tcMod scroll 0 -3
    }
}

gfx/misc/bball
{
	qer_editorimage	gfx/misc/cring
	surfaceparm	trans
	cull	twosided
    {
        map gfx/effects/bball
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen vertex
    }
}

gfx/misc/spikeb
{
	qer_editorimage	gfx/misc/cring
	surfaceparm	trans
	cull	twosided
    {
        map gfx/effects/spikeb
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen vertex
    }
}

gfx/misc/spblast
{
	qer_editorimage	gfx/misc/cring
	surfaceparm	trans
	cull	twosided
    {
        map gfx/effects/spikeb1
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen vertex
    }
}

gfx/misc/rline
{
	qer_editorimage	gfx/misc/blueline
	cull	twosided
    {
        map gfx/misc/rline
        blendFunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

gfx/misc/bline
{
	qer_editorimage	gfx/misc/blueline
	cull	twosided
    {
        map gfx/misc/bline
        blendFunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

gfx/misc/spark4
{
	qer_editorimage	gfx/misc/spark3
	cull	twosided
    {
        map gfx/misc/spark4
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen vertex
    }
}

gfx/misc/mine
{
	nopicmip
	notc
	q3map_nolightmap
	cull	twosided
    {
        clampmap gfx/misc/mine
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

gfx/misc/bline_w
{
	qer_editorimage	gfx/misc/blueline
	cull	twosided
    {
        map gfx/misc/bline-w
        blendFunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
        tcMod scroll 0 2.5
    }
}

gfx/misc/redbolt
{
	qer_editorimage	gfx/misc/blueline
	cull	twosided
    {
        map gfx/misc/redline
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen vertex
        tcMod scroll 0 -3
    }
}

effects/fire
{
	cull	twosided
    {
        map gfx/effects/fire
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen vertex
    }
}

effects/refraction
{
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	cull	twosided
    {
        map $whiteimage
        blendFunc GL_SRC_ALPHA GL_SRC_COLOR
        rgbGen entity
        alphaGen entity
        tcMod rotate 15
        tcMod turb 0 0.03 0 2
    }
}

effects/refract_2
{
	qer_editorimage	effects/refraction
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	cull	twosided
    {
        map $whiteimage
        blendFunc GL_SRC_ALPHA GL_SRC_COLOR
        rgbGen entity
        alphaGen entity
        tcMod stretch triangle 2 1 0 0.4
    }
}

gfx/chunks/chain1
{
	qer_editorimage	gfx/misc/blueline
	cull	twosided
    {
        map gfx/chunks/chain1
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
        alphaGen vertex
    }
}

gfx/chunks/chain2
{
	qer_editorimage	gfx/misc/blueline
	cull	twosided
    {
        map gfx/chunks/chain2
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
        alphaGen vertex
    }
}

gfx/2d/wedge
{
	nomipmaps
	cull	twosided
    {
        map gfx/2d/wedge
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/2d/lock
{
	cull	twosided
    {
        map gfx/2d/lock
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen vertex
    }
}

gfx/2d/insertTick
{
	nomipmaps
	cull	twosided
    {
        map gfx/2d/tick
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen vertex
    }
}

models/map_objects/imp_mine/turret_chair_dmg
{
    {
        map models/map_objects/imp_mine/turret_chair_dmg
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
}

