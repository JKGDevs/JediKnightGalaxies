// JKG_ShaderPatches: BlasTech - Directing gfx/effects/burn

gfx/effects/burn
{
	deformvertexes	bulge	0 -0.2 0
    {
        map gfx/PlayerOverlays/fire
        blendFunc GL_ONE GL_ONE
        glow
        tcMod scale 4 3
        tcMod scroll 1 0.6
    }
}

clear
{
    {
        map $whiteimage
        blendFunc GL_ZERO GL_ONE
    }
}

gfx/effects/stunPass
{
	cull	twosided
    {
        map gfx/effects/stunpass
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen vertex
        tcMod scroll -3.5 0
        tcMod scale 1.5 1
    }
}

gfx/effects/solidWhite
{
	nomipmaps
	cull	twosided
    {
        map $whiteimage
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/effects/bubble
{
    {
        map gfx/effects/bubble
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
        alphaGen vertex
    }
}

gfx/effects/solidWhite_cull
{
	nomipmaps
    {
        map $whiteimage
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/effects/whiteFlare
{
	cull	twosided
    {
        map gfx/effects/whiteflare
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/effects/whiteFlareMult
{
	cull	twosided
    {
        map gfx/effects/whiteflare
        blendFunc GL_DST_COLOR GL_ONE
        rgbGen vertex
    }
}

gfx/effects/jet3
{
	cull	twosided
    {
        map gfx/effects/jet3
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        tcMod scroll 1 0
    }
}

gfx/effects/jet4
{
	cull	twosided
    {
        map gfx/effects/jet4
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        tcMod scroll 1 0
    }
}

gfx/effects/flare1
{
	cull	twosided
    {
        map gfx/effects/flare1
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen vertex
        tcMod rotate 4
    }
}

gfx/effects/meltMark
{
	polygonOffset
	cull	twosided
    {
        map gfx/effects/meltmark
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

gfx/effects/smokeTrail
{
	cull	twosided
    {
        map gfx/effects/smoketrail
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
        tcMod scale 1 0.7
        tcMod scroll 0 -0.2
    }
    {
        map gfx/effects/smoketrail
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
        tcMod scale 1 0.3
        tcMod scroll 0 -0.3
    }
}

gfx/effects/shield
{
	cull	twosided
    {
        map gfx/effects/shield
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen vertex
    }
}

gfx/effects/alpha_smoke
{
	cull	twosided
    {
        map gfx/effects/alpha_smoke
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
        alphaGen vertex
    }
}

gfx/effects/alpha_smoke2
{
	cull	twosided
    {
        map gfx/effects/alpha_smoke2
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
        alphaGen vertex
    }
}

gfx/effects/flamejet
{
	cull	twosided
    {
        map gfx/effects/flamejet
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen vertex
    }
}

gfx/effects/whiteFlash
{
	nomipmaps
	sort	nearest
	cull	twosided
    {
        map $whiteimage
        blendFunc GL_ONE GL_ONE
        depthFunc disable
        rgbGen vertex
    }
}

gfx/effects/pool1
{
	cull	twosided
    {
        map gfx/effects/pool1
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 0.7 0.3 0.6 0
        tcMod scroll 0.03 0
        tcMod scale 5 1
    }
}

gfx/effects/fl_blue2orange
{
	cull	twosided
    {
        map gfx/effects/fl_blue2orange
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 0.7 0.3 0.6 0
        tcMod scroll 0.03 0
        tcMod scale 5 1
    }
}

gfx/effects/fl_orange2blue
{
	cull	twosided
    {
        map gfx/effects/fl_orange2blue
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 0.7 0.3 0.6 0
        tcMod scroll 0.03 0
        tcMod scale 5 1
    }
}

gfx/effects/pool2
{
	cull	twosided
    {
        map gfx/effects/pool1
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 0.7 0.3 0.7 0
        tcMod scroll 0.1 0
        tcMod scale 3 1
    }
}

gfx/effects/pool_static
{
	cull	twosided
    {
        map gfx/effects/static2
        blendFunc GL_DST_COLOR GL_ONE
        rgbGen const ( 0.700000 0.700000 0.300000 )
        tcMod scroll -0.02 0
        tcMod scale 7 3
    }
}

gfx/effects/whiteFlare2
{
	cull	twosided
    {
        map gfx/effects/whiteflare2
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        rgbGen identity
    }
}

gfx/effects/demp2shell
{
	cull	twosided
    {
        map gfx/effects/plasma
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen vertex
        tcMod scale 2 3
        tcMod scroll 1 0.5
    }
    {
        map gfx/effects/plasma
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
        tcMod scale 3 2
        tcMod scroll 0.5 2.6
    }
}

gfx/misc/debugArrow
{
	cull	twosided
    {
        map gfx/misc/debugarrow
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
        alphaGen vertex
    }
}

gfx/misc/debugAmbient
{
	cull	twosided
    {
        map gfx/misc/debugambient
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
        alphaGen vertex
    }
}

gfx/misc/grayRing
{
	cull	twosided
    {
        map gfx/misc/grayring
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen vertex
        tcMod scale 2 1
    }
}

gfx/misc/hbeam1
{
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	cull	twosided
    {
        map gfx/misc/hbeam1
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
        tcMod scale 1 3
        tcMod scroll 0 5
    }
}

gfx/misc/hbeam1a
{
	qer_editorimage	gfx/misc/hbeam1
    {
        map gfx/misc/hbeam1a
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
        tcMod scroll 0 2
    }
}

gfx/misc/iring
{
	cull	twosided
    {
        map gfx/misc/iring
        blendFunc GL_ONE GL_ONE
    }
}

gfx/misc/bolt1
{
    {
        map gfx/misc/bolt1
        blendFunc GL_ONE GL_ONE
        tcMod scroll 0 -5
    }
    {
        map gfx/misc/blueline1
        blendFunc GL_SRC_ALPHA GL_ONE
        tcMod scroll 0 -3
    }
}

gfx/misc/dr1
{
	cull	twosided
    {
        map gfx/misc/dr1
        blendFunc GL_ONE GL_ONE
        tcMod scale 1 0.5
        tcMod scroll 0 3
    }
}

gfx/misc/vbolt
{
	qer_editorimage	gfx/misc/bolt1
    {
        map gfx/misc/bolt1_green
        blendFunc GL_ONE GL_ONE
        tcMod scroll 0 -5
    }
    {
        map gfx/misc/blueline1_green
        blendFunc GL_SRC_ALPHA GL_ONE
        tcMod scroll 0 -3
    }
}

gfx/2d/droid_view
{
	nomipmaps
	cull	twosided
    {
        clampmap gfx/2d/droid_view
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        rgbGen wave random 0.994 0.006 0 1
    }
}

gfx/2d/jsense
{
	qer_editorimage	gfx/2d/droid_view
	nomipmaps
	cull	twosided
    {
        map gfx/2d/jedisns
        blendFunc GL_ONE GL_ONE
        glow
        tcMod rotate 5
        tcMod stretch sin 1.4 0.05 1 0.1
    }
    {
        map gfx/2d/jedisns1
        blendFunc GL_ONE_MINUS_DST_COLOR GL_ONE
        tcMod stretch sin 1.4 0.07 1 0.1
        tcMod rotate 2
    }
}

models/players/reborn/boss_torso
{
    {
        map models/players/reborn/boss_torso
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/players/reborn/boss_torso_s
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen identity
        alphaGen lightingSpecular
    }
}

gfx/world/haze
{
	cull	twosided
    {
        map gfx/world/bhaze
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen exactVertex
        tcMod scale 0.006 0.004
        tcMod scroll 0.1 0
    }
}

gfx/world/haze2
{
	cull	twosided
    {
        map gfx/world/bhaze
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen exactVertex
        tcMod scale 0.008 0.007
        tcMod scroll 0.08 0.09
    }
}

