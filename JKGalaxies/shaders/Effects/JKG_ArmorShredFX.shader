// JKGalaxies Armor/Durability EFX Shaders by Futuza
// Makes dust/rust/metal flake puffs, used by armor shred debuff
// might also use it for durability as well

textures/common/metal_debris
{
	qer_editorimage	map textures/common/metalflake_rust.jpg
	surfaceparm	trans
	q3map_onlyvertexlighting
	cull	twosided
    {
        map textures/common/metalflake_rust
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
        tcMod scroll 0.5 0.3
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

// Last modified by: Futuza on 2026-05-25