// JKG_ShaderPatches: BlasTech - JKG shaders to detpacks and saber hilts. 

// BaseJKA Detpack, "DM-01" (RavenSoft)

models/weapons2/detpack/pack
{
	q3map_nolightmap
    {
        map models/weapons2/detpack/pack
        rgbGen lightingDiffuse
    }
    {
        map models/weapons2/detpack/detpack_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

// _____________________________________________________

models/weapons2/blaster_pistol/blaster_pistol_inworld
{
	q3map_nolightmap
    {
        map models/weapons2/blaster_pistol/blaster_pistol_inworld
        rgbGen lightingDiffuse
    }
}

models/weapons2/blaster_pistol/blaster_pistol_inview_alpha
{
	q3map_nolightmap
    {
        map models/weapons2/blaster_pistol/blaster_pistol_inview_alpha
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        depthWrite
        rgbGen lightingDiffuse
    }
    {
        map models/weapons2/blaster_pistol/w_suppressor_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/weapons2/blaster_pistol/blaster_pistol_inview
{
	q3map_nolightmap
    {
        map models/weapons2/blaster_pistol/blaster_pistol_inview
        rgbGen lightingDiffuse
    }
    {
        map models/weapons2/blaster_pistol/w_blaster_pistol_inview_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/weapons2/concussion/rifle_energy
{
    {
        map models/weapons2/concussion/power32
        tcMod scroll 0 6
    }
    {
        map models/weapons2/concussion/power38
        blendFunc GL_DST_COLOR GL_SRC_ALPHA
        tcMod scroll 8 1
    }
}

models/weapons2/concussion/rifle_inview
{
	q3map_nolightmap
    {
        map models/weapons2/concussion/rifle_inview
        rgbGen lightingDiffuse
    }
    {
        map models/weapons2/concussion/concussion_rifle_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/weapons2/concussion/rifle_energy_old
{
	qer_editorimage	models/weapons2/concussion/rifle_energy3
    {
        map models/weapons2/concussion/rifle_energy3
        tcMod scroll 5 0
        tcMod scale 5 1
    }
    {
        map models/weapons2/concussion/rifle_energy1
        blendFunc GL_ONE GL_ONE
        tcMod scroll 0 4
        tcMod transform 0.5 0 0 1 0 0
        tcMod turb 0 1 0 1
    }
    {
        map models/weapons2/concussion/rifle_energy2
        blendFunc GL_ONE GL_ONE
        tcMod scroll 0 -5
        tcMod stretch square 0.5 1 0 6
        tcMod turb 0 4 0 1
    }
}

models/weapons2/blaster_r/blaster_r
{
	q3map_nolightmap
    {
        map models/weapons2/blaster_r/blaster_r
        rgbGen lightingDiffuse
    }
    {
        map models/weapons2/blaster_r/blaster_r_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/weapons2/blaster_r/scope
{
	q3map_nolightmap
    {
        map models/weapons2/blaster_r/scope
        rgbGen lightingDiffuse
    }
    {
        map models/weapons2/blaster_r/scope_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/weapons2/disruptor/disruptor
{
	q3map_nolightmap
    {
        map models/weapons2/disruptor/disruptor
        rgbGen lightingDiffuse
    }
    {
        map models/weapons2/disruptor/disprutor_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/weapons2/disruptor/hand
{
	q3map_nolightmap
    {
        map models/weapons2/disruptor/hand
        rgbGen lightingDiffuse
    }
    {
        map models/weapons2/disruptor/hand
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/weapons2/bowcaster/bowcaster
{
	q3map_nolightmap
    {
        map models/weapons2/bowcaster/bowcaster
        rgbGen lightingDiffuse
    }
    {
        map models/weapons2/bowcaster/bow_caster_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/weapons2/heavy_repeater/heavy_repeater
{
	q3map_nolightmap
    {
        map models/weapons2/heavy_repeater/heavy_repeater
        rgbGen lightingDiffuse
    }
    {
        map models/weapons2/heavy_repeater/heavy_repeater_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/weapons2/golan_arms/golan_arms
{
	q3map_nolightmap
    {
        map models/weapons2/golan_arms/golan_arms
        rgbGen lightingDiffuse
    }
    {
        map models/weapons2/golan_arms/golan_arms_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/weapons2/golan_arms/canister2
{
	q3map_nolightmap
    {
        map models/weapons2/golan_arms/canister2
        rgbGen lightingDiffuse
    }
    {
        map models/weapons2/golan_arms/canister2_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/weapons2/merr_sonn/merr_sonn_inview
{
	q3map_nolightmap
    {
        map models/weapons2/merr_sonn/merr_sonn_inview
        rgbGen lightingDiffuse
    }
    {
        map models/weapons2/merr_sonn/merr_sonn_inview_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/weapons2/thermal/thermal
{
	q3map_nolightmap
    {
        map models/weapons2/thermal/thermal
        rgbGen lightingDiffuse
    }
    {
        map models/weapons2/thermal/thermal_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/weapons2/thermal/hand
{
	q3map_nolightmap
    {
        map models/weapons2/thermal/hand
        rgbGen lightingDiffuse
    }
    {
        map models/weapons2/thermal/hand_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/weapons2/laser_trap/hand
{
	q3map_nolightmap
    {
        map models/weapons2/laser_trap/hand
        rgbGen lightingDiffuse
    }
    {
        map models/weapons2/laser_trap/hand_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/weapons2/demp2/demp2
{
	q3map_nolightmap
    {
        map models/weapons2/demp2/demp2
        rgbGen lightingDiffuse
    }
    {
        map models/weapons2/demp2/demp2_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/weapons2/saber/saber
{
    {
        map models/weapons2/saber/saber
        rgbGen lightingDiffuse
    }
    {
        map models/weapons2/saber/saber_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
    {
        map models/weapons2/saber/saber_glow
        blendFunc GL_ONE GL_ONE
        glow
        detail
        rgbGen identity
    }
}

models/weapons2/saber_1/saber_1
{
    {
        map models/weapons2/saber_1/saber_1
        rgbGen lightingDiffuse
    }
    {
        map models/weapons2/saber_1/saber_1_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map models/weapons2/saber_1/saber_1_glow
        blendFunc GL_ONE GL_ONE
        glow
        detail
        rgbGen identity
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons2/saber_2/saber_2
{
    {
        map models/weapons2/saber_2/saber_2
        rgbGen lightingDiffuse
    }
    {
        map models/weapons2/saber_2/saber_2_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons2/saber_3/saber_3
{
    {
        map models/weapons2/saber_3/saber_3
        rgbGen lightingDiffuse
    }
    {
        map models/weapons2/saber_3/saber_3_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons2/saber_4/saber_4
{
    {
        map models/weapons2/saber_4/saber_4
        rgbGen lightingDiffuse
    }
    {
        map models/weapons2/saber_4/saber_4_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons2/saber_5/saber_5
{
    {
        map models/weapons2/saber_5/saber_5
        rgbGen lightingDiffuse
    }
    {
        map models/weapons2/saber_5/saber_5_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map models/weapons2/saber_5/saber_5_glow
        blendFunc GL_ONE GL_ONE
        glow
        detail
        rgbGen identity
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons2/saber_6/saber_6
{
    {
        map models/weapons2/saber_6/saber_6
        rgbGen lightingDiffuse
    }
    {
        map models/weapons2/saber_6/saber_6_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons2/saber_7/saber_7
{
    {
        map models/weapons2/saber_7/saber_7
        rgbGen lightingDiffuse
    }
    {
        map models/weapons2/saber_7/saber_7_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons2/saber_8/saber_8
{
    {
        map models/weapons2/saber_8/saber_8
        rgbGen lightingDiffuse
    }
    {
        map models/weapons2/saber_8/saber_8_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map models/weapons2/saber_8/saber_8_glow
        blendFunc GL_ONE GL_ONE
        detail
        rgbGen identity
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}

models/weapons2/saber_9/saber_9
{
    {
        map models/weapons2/saber_9/saber_9
        rgbGen lightingDiffuse
    }
    {
        map models/weapons2/saber_9/saber_9_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map models/weapons2/saber_9/saber_9_glow
        blendFunc GL_ONE GL_ONE
        glow
        detail
        rgbGen identity
    }
    {
        map envmap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        rgbGen lightingDiffuse
        alphaGen lightingSpecular
        tcGen environment
    }
}
