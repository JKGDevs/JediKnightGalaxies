// JKG_ShaderPatches: BlasTech - Bloom to Portable Sentry Gun and Detpack lights

models/items/psgun
{
	qer_editorimage	models/items/psgun
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/imp_mine/psgunlight
        blendFunc GL_ONE GL_ZERO
        tcMod scroll 0 1
    }
    {
        map models/map_objects/imp_mine/psgunlight
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0 1 0 2
        tcMod scroll 0 1
    }
    {
        map models/items/psgun
        alphaFunc GE128
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen lightingDiffuse
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

models/weapons2/detpack/lights
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/weapons2/detpack/lights
        rgbGen lightingDiffuse
    }
    {
        map models/weapons2/detpack/detpak_light_spec
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
        map models/weapons2/detpack/lightsglow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave square 0 1 0 2
    }
}

models/weapons2/detpack/nolights
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/weapons2/detpack/lights
        rgbGen lightingDiffuse
    }
    {
        map models/weapons2/detpack/detpak_light_spec
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

// ______________________________________________

models/items/medpac
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/items/medpac
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/items/medpacglow
        blendFunc GL_ONE GL_ONE
        rgbGen wave square 1 1 0 0.75
    }
}

models/items/power_converter_panelon
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/items/power_converter_panelon
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/items/power_converter_panel1
        blendFunc GL_ONE GL_ONE
        rgbGen wave square 0 1 0.5 1
    }
    {
        map models/items/power_converter_panel2
        blendFunc GL_ONE GL_ONE
        rgbGen wave square 0 1 1 0.5
    }
}

models/items/shieldpackon
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/items/shieldpackon
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/items/shieldpackglow
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 1 0.5 0 2
    }
}

models/items/battery
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/items/battery
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/items/battery_glow
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.4 0.4 0 0.5
    }
}

models/items/bacta
{
	q3map_nolightmap
	q3map_onlyvertexlighting
	sort	seeThrough
    {
        map textures/kejim/coolant
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 1 0.25 0 0.25
        tcMod scroll 0.15 0
    }
    {
        map models/items/bacta
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen lightingDiffuse
    }
}

models/items/power_converter_panel
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/items/power_converter_panel
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/items/power_converter_panel1
        blendFunc GL_ONE GL_ONE
        rgbGen wave square 1 1 0.5 1
    }
    {
        map models/items/power_converter_panel2
        blendFunc GL_ONE GL_ONE
        rgbGen wave square 1 1 -1 0.5
    }
}

models/items/a_shield
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/items/a_shield
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/items/a_shield_glow
        blendFunc GL_ONE GL_ONE
    }
    {
        animMap 2 models/items/a_shield_glow2 gfx/colors/black 
        blendFunc GL_ONE GL_ONE
        rgbGen wave inversesawtooth 0 2 0.5 1.5
    }
}

models/items/a_pwr_converter
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/items/a_pwr_converter
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/items/a_pwr_converter_glow
        blendFunc GL_ONE GL_ONE
    }
    {
        animMap 2 models/items/a_pwr_converter_glow2 gfx/colors/black 
        blendFunc GL_ONE GL_ONE
        rgbGen wave sawtooth 0.25 2 0 2
    }
}

models/items/forcegem
{
	q3map_nolightmap
    {
        map models/items/forcegem
        blendFunc GL_ONE GL_ZERO
    }
    {
        map models/items/forcegem
        blendFunc GL_SRC_ALPHA GL_ONE
        alphaGen lightingSpecular
    }
    {
        map models/items/forcegem
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.4 0.28 0.5 0.45
        tcMod rotate 16
    }
}

models/items/antenna
{

	q3map_nolightmap
    {
        map models/items/antenna
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/items/antenna_glow2
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 1.4 1 0.7 2
    }
}

