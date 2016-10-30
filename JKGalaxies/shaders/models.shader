// JKG_ShaderPatches: BlasTech - deformvertexes parm to the flags, bloom to Portable Sentry Gun, lightmap to swoop & Z-95 for Mace's JKG_CargoHeist2. 
// _______________________________________________________________________________________________________________________________

models/map_objects/mp/flag
{
	q3map_nolightmap
	q3map_onlyvertexlighting
	cull	twosided
	deformvertexes	normal 0.4 -0.8			// <type-frequency-amplitude>
	deformvertexes	wave 50 sin 0 0.8 0 0.4	// <type-spread-func-base-amplitude-phase-frequency>
    {
        map models/map_objects/mp/flag
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
}

models/map_objects/mp/flag2
{
	q3map_nolightmap
	q3map_onlyvertexlighting
	cull	twosided
	deformvertexes	normal 0.4 -0.8			// <type-frequency-amplitude>
	deformvertexes	wave 50 sin 0 0.8 0 0.4
    {
        map models/map_objects/mp/flag2
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
}

models/map_objects/imp_mine/psgun
{
	qer_editorimage	models/items/psgun
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/imp_mine/psgunlight
        blendFunc GL_ONE GL_ZERO
        rgbGen wave sin 0 1 0 2
        tcMod scroll 0 1
    }
    {
        map models/items/psgun
        alphaFunc GE128
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

models/players/swoop/swoop
{
    {
        map $lightmap
    }
    {
        map models/players/swoop/swoop
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map models/players/swoop/swoop_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
    {
        map models/players/swoop/swoop_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/players/swoop/swoop_black
{
    {
        map $lightmap
    }
    {
        map models/players/swoop/swoop_black
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map models/players/swoop/swoop_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
    {
        map models/players/swoop/swoop_black_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/players/swoop/swoop_blue
{
    {
        map $lightmap
    }
    {
        map models/players/swoop/swoop_blue
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map models/players/swoop/swoop_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
    {
        map models/players/swoop/swoop_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/players/swoop/swoop_gold
{
    {
        map $lightmap
    }
    {
        map models/players/swoop/swoop_gold
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map models/players/swoop/swoop_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
    {
        map models/players/swoop/swoop_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/players/swoop/swoop_green
{
    {
        map $lightmap
    }
    {
        map models/players/swoop/swoop_green
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map models/players/swoop/swoop_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
    {
        map models/players/swoop/swoop_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/players/swoop/swoop_purple
{
    {
        map $lightmap
    }
    {
        map models/players/swoop/swoop_purple
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map models/players/swoop/swoop_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
    {
        map models/players/swoop/swoop_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/players/swoop/swoop_silver
{
    {
        map $lightmap
    }
    {
        map models/players/swoop/swoop_silver
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map models/players/swoop/swoop_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
    {
        map models/players/swoop/swoop_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/players/swoop/swoop2
{
    {
        map $lightmap
    }
    {
        map models/players/swoop/swoop2
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map models/players/swoop/swoop2_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        glow
        detail
        alphaGen lightingSpecular
    }
}

models/map_objects/ships/z95_wing
{
    {
        map models/map_objects/ships/z95_wing
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map models/map_objects/ships/z95_wing_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map textures/common/detail4
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        detail
        tcMod scale 4 4
    }
}

models/map_objects/ships/z95_cockpit
{
    {
        map envmap
        tcGen environment
        tcMod scale 2 2
    }
    {
        map models/map_objects/ships/z95_cockpit_mask
        blendFunc GL_ZERO GL_ONE_MINUS_SRC_COLOR
    }
    {
        map models/map_objects/ships/z95_cockpit
        blendFunc GL_ONE GL_ONE
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map models/map_objects/ships/z_95_canopy_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        alphaGen lightingSpecular
    }
}

models/map_objects/ships/z95_body
{
    {
        map models/map_objects/ships/z95_body
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map models/map_objects/ships/z_95_body_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map textures/common/detail4
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        detail
        tcMod scale 4 4
    }
}

// _____________________________________________________________________
// BaseJKA -->

models/map_objects/imp_mine/psd
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/items/psd
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/items/psdglow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 1 1 -1 5
    }
}

models/map_objects/imp_mine/power_convertor_panel
{
	qer_editorimage	models/map_objects/imp_mine/power_converter_panel
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/imp_mine/power_converter_panel
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/imp_mine/power_converter_panel2
        blendFunc GL_ONE GL_ONE
        rgbGen wave square 1 1 0.5 1
    }
    {
        map models/map_objects/imp_mine/power_converter_panel1
        blendFunc GL_ONE GL_ONE
        rgbGen wave square 1 1 -1 0.5
    }
}

models/map_objects/imp_mine/consoleface_04
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/imp_mine/consoleface_04
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/imp_mine/consoleface_04glow
        blendFunc GL_ONE GL_ONE
        glow
        detail
        rgbGen wave sin 1 0.2 0 45
    }
}

models/map_objects/imp_mine/consoleface_02
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/imp_mine/consoleface_02
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/imp_mine/consoleface_02glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 1 0.1 0 50
    }
}

models/map_objects/imp_mine/consoleface_03
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/imp_mine/consoleface_03
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/imp_mine/consoleface_03glow
        blendFunc GL_ONE GL_ONE
        glow
    }
    {
        map models/map_objects/imp_mine/consoleface_03glow2
        blendFunc GL_ONE GL_ONE
        glow
        detail
        rgbGen wave inversesawtooth 0 0.5 0 1
    }
}

models/map_objects/imp_mine/consoleface_01
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map gfx/2d/cropcircle2
        blendFunc GL_ONE GL_ZERO
        rgbGen wave inversesawtooth 0 1 0 0.5
        tcMod scale 3.85 4.5
    }
    {
        map models/map_objects/imp_mine/consoleface_01
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/imp_mine/consoleface_01glow
        blendFunc GL_ONE GL_ONE
        glow
    }
    {
        map models/map_objects/imp_mine/consoleface_01glow2
        blendFunc GL_ONE GL_ONE
        glow
        detail
        rgbGen wave sin 0 1 0 1
    }
    {
        map models/map_objects/imp_mine/consoleface_01glow3
        blendFunc GL_ONE GL_ONE
        glow
        detail
        rgbGen wave square 0 1 0 2
    }
}

models/map_objects/imp_mine/minelamp
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/imp_mine/minelamp
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/imp_mine/minelampglow
        blendFunc GL_ONE GL_ONE
        glow
    }
}

models/map_objects/imp_mine/control_panel_shd
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/imp_mine/control_panel_shd
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/imp_mine/control_panel_shd_on
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 1 0.25 0 2
    }
}

models/map_objects/imp_mine/mbay_monitor_screen
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/imp_mine/mbay_monitor_screen
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        animMap 4 models/map_objects/imp_mine/mbay_monitor_screen4 models/map_objects/imp_mine/mbay_monitor_screen5 models/map_objects/imp_mine/mbay_monitor_screen6 models/map_objects/imp_mine/mbay_monitor_screen7 models/map_objects/imp_mine/mbay_monitor_screen models/map_objects/imp_mine/mbay_monitor_screen1 models/map_objects/imp_mine/mbay_monitor_screen2 models/map_objects/imp_mine/mbay_monitor_screen3 
        blendFunc GL_ONE GL_ONE
        glow
    }
    {
        animMap 8 models/map_objects/imp_mine/mbay_monitor_screen models/map_objects/imp_mine/mbay_monitor_screen1 models/map_objects/imp_mine/mbay_monitor_screen2 models/map_objects/imp_mine/mbay_monitor_screen3 models/map_objects/imp_mine/mbay_monitor_screen4 models/map_objects/imp_mine/mbay_monitor_screen5 models/map_objects/imp_mine/mbay_monitor_screen6 models/map_objects/imp_mine/mbay_monitor_screen7 
        blendFunc GL_ONE GL_ONE
        glow
        detail
    }
}

models/map_objects/imp_mine/mbay_control_panel
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/imp_mine/mbay_control_panel
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/imp_mine/mbay_control_panelglow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 1 0.2 0.5 45
    }
    {
        map models/map_objects/imp_mine/mbay_control_panelblink
        blendFunc GL_ONE GL_ONE
        glow
        detail
        rgbGen wave sin 0 1 0 1
    }
}

models/map_objects/imp_mine/tubes
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/imp_mine/tubes
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/imp_mine/tubesglow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 0.5 1 1 0.5
    }
}

models/map_objects/imp_mine/tank
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/imp_mine/tank
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/imp_mine/tanklight
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sawtooth 0.75 1 0 0.25
    }
}

models/map_objects/imp_mine/shield_sm
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/items/shield_sm
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/items/shield_smglow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 1 0.5 0 2
    }
}

models/map_objects/imp_mine/shieldpack
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/items/shieldpack
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/items/shieldpackglow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 1 0.5 0 0.5
    }
}

models/map_objects/imp_mine/minelamp_on
{
	q3map_nolightmap
	q3map_onlyvertexlighting
	cull	twosided
    {
        map models/map_objects/imp_mine/minelamp_on
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 0.5 0.05 0 50
    }
}

models/map_objects/imp_mine/turret_chair_on
{
	qer_editorimage	gfx/effects/redline
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map gfx/effects/redline
        blendFunc GL_ONE GL_ZERO
        tcMod scroll 2 0
    }
    {
        map models/map_objects/imp_mine/turret_chair
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen lightingDiffuse
    }
}

models/map_objects/imp_mine/desk_shade
{
	qer_editorimage	models/map_objects/imp_mine/desk_shade
	q3map_nolightmap
	q3map_onlyvertexlighting
	cull	twosided
    {
        map models/map_objects/imp_mine/desk_shade
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 0.5 0.05 0 50
    }
}

models/map_objects/imp_mine/crystal_slag
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/imp_mine/crystal_slag
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/imp_mine/crystal_slagglow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 1 0.1 0 5
    }
}

models/map_objects/imp_mine/mine_drill_body
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/imp_mine/control5a
        blendFunc GL_ONE GL_ZERO
        glow
        rgbGen wave sin 0 1 0 1
        tcMod scale 11.5 16
    }
    {
        map models/map_objects/imp_mine/control5a
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 0 1 0 1
        tcMod scale 11.5 16
    }
    {
        map models/map_objects/imp_mine/control5b
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 0 1 -0.5 1
        tcMod scale 11.5 16
    }
    {
        map models/map_objects/imp_mine/mine_drill_body
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen lightingDiffuse
    }
}

models/map_objects/imp_mine/turret_cannon_base
{
	q3map_nolightmap
    {
        map models/map_objects/imp_mine/turret_cannon_base
        rgbGen lightingDiffuse
    }
    {
        animMap 2 models/map_objects/imp_mine/turret_glow gfx/colors/black 
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 0.3 1 0 1.5
    }
}

models/map_objects/imp_mine/probedamage
{
	q3map_nolightmap
    {
        map models/map_objects/imp_mine/probedamage
        blendFunc GL_ONE GL_ZERO
        rgbGen exactVertex
    }
    {
        map models/map_objects/imp_mine/probedamage_glw
        blendFunc GL_ONE GL_ONE
        glow
        detail
        rgbGen wave noise 0 1 0 2
    }
}

models/map_objects/imp_mine/ore_cart/cart_fx
{
	qer_editorimage	models/map_objects/imp_mine/ore_cart/cart_fx
	q3map_nolightmap
	q3map_onlyvertexlighting
	cull	twosided
    {
        map models/map_objects/imp_mine/ore_cart/cart_fx
        blendFunc GL_ONE GL_ONE
        rgbGen wave sawtooth 0 1 0 5
    }
    {
        map models/map_objects/imp_mine/ore_cart/cart_fx
        blendFunc GL_ONE GL_ONE
        rgbGen wave noise 1 0.5 0 5
    }
}

models/map_objects/kejim/crystal_panel
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/kejim/crystal_panel
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/kejim/crystal_panelglow1
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave square 1 1 0 2
    }
    {
        map models/map_objects/kejim/crystal_panelglow2
        blendFunc GL_ONE GL_ONE
        glow
        detail
    }
    {
        map models/map_objects/kejim/crystal_panelglow3
        blendFunc GL_ONE GL_ONE
        glow
        detail
        rgbGen wave square 0 1 0 0.5
    }
}

models/map_objects/kejim/crystal_lights01
{
	cull	twosided
    {
        map $lightmap
    }
    {
        map models/map_objects/kejim/crystal_lights01a
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map models/map_objects/kejim/crystal_lights01
        blendFunc GL_ONE GL_ONE
        glow
    }
}

models/map_objects/kejim/crystal_lights02
{
	cull	twosided
    {
        map $lightmap
    }
    {
        map models/map_objects/kejim/crystal_lights01a
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map models/map_objects/kejim/crystal_lights02
        blendFunc GL_ONE GL_ONE
        glow
    }
}

models/map_objects/kejim/tram_panel01
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/kejim/tram_panel01
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/kejim/tram_panel01glow
        blendFunc GL_ONE GL_ONE
        glow
    }
    {
        map models/map_objects/kejim/tram_panel01blink
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 0 1 0 1
    }
}

models/map_objects/kejim/tram_panel02
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/kejim/tram_panel02
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/kejim/tram_panel02glow
        blendFunc GL_ONE GL_ONE
        glow
    }
    {
        map models/map_objects/kejim/tram_panel02blink
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 0 2 0 2
    }
}

models/map_objects/kejim/tube_glass
{
	q3map_nolightmap
    {
        map models/map_objects/kejim/tube_glass
        blendFunc GL_ONE GL_ONE
        glow
    }
}

models/map_objects/kejim/tram_supports
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/kejim/tram_supports
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/kejim/tram_supportslight
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave noise 1 0.25 0 3
    }
    {
        map models/map_objects/kejim/tram_supportslight
        blendFunc GL_ONE GL_ONE
        glow
    }
}

models/map_objects/kejim/tube
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/kejim/tube
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/kejim/tubelights
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave triangle 1 1 0 2
    }
}

models/map_objects/kejim/bombcrystal
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/kejim/bombcrystal
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/kejim/bombcrystal
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave noise 0 1 0 5
    }
    {
        map models/map_objects/kejim/bombcrystal
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 0.5 0.25 0 0.5
    }
    {
        map models/map_objects/kejim/crystalsg
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave noise 0 0.1 0 2
        tcMod scroll 1 1
        tcMod scale 1 2
    }
}

models/map_objects/kejim/stasispanel
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/kejim/stasispanel
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/kejim/stasispanelglow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 1 0.5 0 2
    }
}

models/map_objects/kejim/stasistube
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/kejim/stasistube
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/kejim/stasistubeglow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 1 1 0 0.25
    }
}

models/map_objects/kejim/stasisglass
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/kejim/stasisglass
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

models/map_objects/kejim/crystals
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/kejim/crystals
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen identity
        alphaGen wave sin 0.3 0 0 0
    }
    {
        map models/map_objects/kejim/crystalsg
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave noise 0.2 0.3 0 2
    }
}

models/map_objects/kejim/crystals2
{
	cull	twosided
    {
        map gfx/effects/reflection
        blendFunc GL_ONE GL_ONE
        rgbGen identity
        tcGen environment
        tcMod scale 3 3
    }
    {
        map models/map_objects/kejim/crystals2
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        rgbGen identity
    }
    {
        map models/map_objects/kejim/crystals
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen identity
        alphaGen wave sin 0.2 0 0 0
    }
}

models/map_objects/kejim/contain
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/kejim/contain
        blendFunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/kejim/contain2
        blendFunc GL_ONE GL_ONE
        glow
    }
}

models/map_objects/kejim/contop
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/kejim/contop
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/kejim/contopglow
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.5 0.2 0 10
    }
}

models/map_objects/kejim/exampanels
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/kejim/exampanels
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/kejim/exampanelsglow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 1 0.25 0 1
    }
    {
        map models/map_objects/kejim/exampanels1
        blendFunc GL_ONE GL_ONE
        glow
        detail
        rgbGen wave sin 0 1 0 1
    }
    {
        map models/map_objects/kejim/exampanels2
        blendFunc GL_ONE GL_ONE
        glow
        detail
        rgbGen wave sin 0 1 -0.333 1
    }
    {
        map models/map_objects/kejim/exampanels3
        blendFunc GL_ONE GL_ONE
        glow
        detail
        rgbGen wave sin 0 1 0.333 1
    }
}

models/map_objects/kejim/gen_body
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/kejim/gen_body
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/kejim/gen_bodyglow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 1 0.1 0 25
    }
}

models/map_objects/kejim/gen_arches
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map textures/kejim/coolant
        blendFunc GL_ONE GL_ZERO
        tcMod scroll 1 0
        tcMod stretch sin 1 0.1 0 1
    }
    {
        map models/map_objects/kejim/gen_arches
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen lightingDiffuse
    }
}

models/map_objects/kejim/ravenbody
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/kejim/ravenbody
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/kejim/ravenbody_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map models/map_objects/kejim/ravenbodyglow
        blendFunc GL_ONE GL_ONE
        glow
        detail
        rgbGen wave sin 1 0.5 0 25
    }
}

models/map_objects/kejim/dish
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/imp_mine/psgunlight
        blendFunc GL_ONE GL_ZERO
        rgbGen wave sin 0 0.25 0 1
    }
    {
        map models/map_objects/imp_mine/psgunlight
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 0 1 0 1
        tcMod scroll 5 5
        tcMod turb 0 1 0 1
        tcMod scale 25 1
    }
    {
        map models/map_objects/kejim/dish
        alphaFunc GE128
        rgbGen lightingDiffuse
    }
}

models/map_objects/kejim/cargo_small_02
{
	q3map_nolightmap
    {
        map models/map_objects/kejim/cargo_small_02
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/kejim/cargo_small_02_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 1 0.5 0.25 0.75
    }
}

models/map_objects/kejim/doorpad
{
    {
        animMap 4 models/map_objects/kejim/doorpad models/map_objects/kejim/doorpad1 
        blendFunc GL_ONE GL_ZERO
    }
}

models/map_objects/kejim/doorpad_on
{
	qer_editorimage	models/map_objects/kejim/doorpad
    {
        animMap 4 models/map_objects/kejim/doorpad1 models/map_objects/kejim/doorpad2 
        blendFunc GL_ONE GL_ZERO
    }
}

models/map_objects/kejim/gen_body_d
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/kejim/gen_body_d
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/kejim/gen_body_dglow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave noise 1 0.5 -0.5 7
    }
    {
        map models/map_objects/imp_detention/tie_body_damageglow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave noise 1 0.5 0 7
    }
}

models/map_objects/kejim/cutsupport_d
{
	qer_editorimage	models/map_objects/kejim/cutsupport_d
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/kejim/cutsupport_d
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/kejim/cutsupport_dglow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave noise 1 0.5 -0.5 7
    }
    {
        map models/map_objects/imp_detention/tie_body_damageglow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave noise 1 0.5 0 7
    }
}

models/map_objects/kejim/cutbody_d
{
	qer_editorimage	models/map_objects/kejim/cutsupport_d
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/kejim/cutbody_d
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/kejim/cutbody_dglow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave noise 1 0.5 -0.5 7
    }
}

models/map_objects/kejim/cutstrut_d
{
	qer_editorimage	models/map_objects/kejim/cutstrut_d
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/kejim/cutstrut_d
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
}

models/map_objects/kejim/cutstrut
{
	qer_editorimage	models/map_objects/kejim/cutstrut
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/kejim/cutstrut
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/kejim/cutstrutglow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 1 0.5 0 0.25
    }
}

models/map_objects/kejim/cutbody
{
	qer_editorimage	models/map_objects/kejim/cutbody
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/kejim/cutbody
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/kejim/cutbodyglow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 1 1 0 10
    }
}

models/map_objects/kejim/generator_original
{
	qer_editorimage	models/map_objects/kejim/generator
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map gfx/damage_overlays/electricity
        blendFunc GL_ONE GL_ZERO
        tcMod rotate 50
        tcMod scroll 45 75
        tcMod scale 5 5
        tcMod stretch sin 1 0.1 5 5
    }
    {
        map gfx/damage_overlays/electricity
        blendFunc GL_ONE GL_ONE
        glow
        tcMod rotate -50
        tcMod scroll -75 -50
        tcMod scale 5 5
        tcMod stretch sin 1 0.1 5 5
    }
    {
        map models/map_objects/kejim/generator
        alphaFunc GE128
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
}

models/map_objects/kejim/generator
{
	qer_editorimage	models/map_objects/kejim/generator
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        animMap 2 gfx/damage_overlays/electricity gfx/colors/black 
        blendFunc GL_ONE GL_ZERO
        tcMod rotate 50
        tcMod scroll 45 75
        tcMod scale 5 5
        tcMod stretch sin 1 0.1 5 5
    }
    {
        animMap 2 gfx/damage_overlays/electricity gfx/colors/black 
        blendFunc GL_ONE GL_ONE
        glow
        tcMod rotate -50
        tcMod scroll -75 -50
        tcMod scale 5 5
        tcMod stretch sin 1 0.1 5 5
    }
    {
        animMap 2 gfx/misc/lightningflash gfx/colors/black 
        blendFunc GL_ONE GL_ONE
        glow
        tcMod turb 0 1 0 1
        tcMod scroll 1 0.5
        tcMod scale 0.5 7
        tcMod stretch sin 1 0.1 5 5
    }
    {
        map models/map_objects/kejim/generator
        alphaFunc GE128
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
}

models/map_objects/kejim/ravengine
{
	q3map_nolightmap
    {
        map models/map_objects/kejim/ravengine
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/kejim/ravenengine_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/map_objects/kejim/raven_gear
{
	q3map_nolightmap
    {
        map models/map_objects/kejim/raven_gear
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/kejim/raven_gear_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/map_objects/kejim/raven_cap
{
	q3map_nolightmap
    {
        map models/map_objects/kejim/raven_cap
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/kejim/engine_cap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map models/map_objects/kejim/raven_cap_glow
        blendFunc GL_ONE GL_ONE
        glow
        detail
        rgbGen identity
    }
}

models/map_objects/kejim/raven_cap_off
{
	qer_editorimage	models/map_objects/kejim/raven_cap
	q3map_nolightmap
    {
        map models/map_objects/kejim/raven_cap
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/kejim/engine_cap_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/map_objects/yavin/fern
{
	q3map_nolightmap
	q3map_alphashadow
	q3map_onlyvertexlighting
	cull	twosided
    {
        map models/map_objects/yavin/fern
        alphaFunc GE128
        rgbGen vertex
    }
}

models/map_objects/yavin/plant
{
	q3map_nolightmap
	q3map_alphashadow
	q3map_onlyvertexlighting
	cull	twosided
    {
        map models/map_objects/yavin/plant
        alphaFunc GE128
        rgbGen vertex
    }
}

models/map_objects/yavin/fern3
{
	q3map_nolightmap
	q3map_alphashadow
	q3map_onlyvertexlighting
	cull	twosided
    {
        map models/map_objects/yavin/fern3
        alphaFunc GE128
        rgbGen vertex
    }
    {
        map models/map_objects/yavin/fern3_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/map_objects/yavin/grass
{
	q3map_nolightmap
	q3map_alphashadow
	q3map_onlyvertexlighting
	cull	twosided
    {
        map models/map_objects/yavin/grass
        alphaFunc GE128
        rgbGen vertex
    }
}

models/map_objects/yavin/grass_tall
{
	q3map_nolightmap
	q3map_alphashadow
	q3map_onlyvertexlighting
	cull	twosided
    {
        map models/map_objects/yavin/grass_tall
        alphaFunc GE128
        rgbGen vertex
    }
}

models/map_objects/yavin/fern2
{
	q3map_nolightmap
	q3map_alphashadow
	q3map_onlyvertexlighting
	cull	twosided
    {
        map models/map_objects/yavin/fern2
        alphaFunc GE128
        rgbGen vertex
    }
}

models/map_objects/yavin/tree06b
{
	surfaceparm	trans
	q3map_nolightmap
	q3map_alphashadow
	q3map_onlyvertexlighting
	cull	twosided
    {
        map models/map_objects/yavin/tree06b
        alphaFunc GE128
        rgbGen vertex
    }
}

models/map_objects/yavin/tree01btga
{
	q3map_nolightmap
	q3map_alphashadow
	q3map_onlyvertexlighting
	cull	twosided
    {
        map models/map_objects/yavin/tree01btga
        alphaFunc GE128
        rgbGen vertex
    }
}

models/map_objects/yavin/tree05_vines
{
	q3map_nolightmap
	q3map_alphashadow
	q3map_onlyvertexlighting
	cull	twosided
    {
        map models/map_objects/yavin/tree05_vines
        alphaFunc GE128
        rgbGen vertex
    }
}

models/map_objects/yavin/tree09b
{
	q3map_nolightmap
	q3map_alphashadow
	q3map_onlyvertexlighting
	cull	twosided
    {
        map models/map_objects/yavin/tree09b
        alphaFunc GE128
        rgbGen vertex
    }
}

models/map_objects/yavin/tree08b
{
	q3map_nolightmap
	q3map_alphashadow
	q3map_onlyvertexlighting
	cull	twosided
    {
        map models/map_objects/yavin/tree08b
        alphaFunc GE128
        rgbGen vertex
    }
}

models/map_objects/yavin/tree09_vines
{
	q3map_nolightmap
	q3map_alphashadow
	q3map_onlyvertexlighting
	cull	twosided
    {
        map models/map_objects/yavin/tree09_vines
        alphaFunc GE128
        rgbGen vertex
    }
}

models/map_objects/yavin/tree09a
{
	q3map_nolightmap
	q3map_alphashadow
	q3map_onlyvertexlighting
	cull	twosided
    {
        map models/map_objects/yavin/tree09a
        alphaFunc GE128
        rgbGen vertex
    }
}

models/map_objects/yavin/tree09d
{
	q3map_nolightmap
	q3map_alphashadow
	q3map_onlyvertexlighting
	cull	twosided
    {
        map models/map_objects/yavin/tree09d
        alphaFunc GE128
        rgbGen vertex
    }
}

models/map_objects/yavin/tree2
{
	q3map_nolightmap
	q3map_alphashadow
	q3map_onlyvertexlighting
	cull	twosided
    {
        map models/map_objects/yavin/tree2
        alphaFunc GE128
        rgbGen vertex
    }
}

models/map_objects/yavin/vines
{
	q3map_nolightmap
	q3map_alphashadow
	q3map_onlyvertexlighting
	cull	twosided
    {
        map models/map_objects/yavin/vines
        alphaFunc GE128
        rgbGen vertex
    }
}

models/map_objects/yavin/light
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/yavin/light
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/yavin/light_glow
        blendFunc GL_ONE GL_ONE
        glow
        tcMod rotate 5
        tcMod scroll 0.5 0
    }
    {
        map models/map_objects/yavin/light_glow2
        blendFunc GL_ONE GL_ONE
        glow
        tcMod scale 3 3
        tcMod scroll 0 0.25
    }
}

models/map_objects/yavin/tree2_b
{
	qer_editorimage	models/map_objects/yavin/tree2
	q3map_nolightmap
	q3map_alphashadow
	q3map_onlyvertexlighting
	cull	twosided
    {
        map models/map_objects/yavin/tree2
        alphaFunc GE128
        rgbGen lightingDiffuse
    }
}

models/map_objects/yavin/tree09d_b
{
	qer_editorimage	models/map_objects/yavin/tree09d
	q3map_alphashadow
	cull	twosided
    {
        map models/map_objects/yavin/tree09d
        alphaFunc GE128
        rgbGen lightingDiffuse
    }
}

models/map_objects/yavin/tree09a_b
{
	qer_editorimage	models/map_objects/yavin/tree09a
	q3map_nolightmap
	q3map_alphashadow
	q3map_onlyvertexlighting
	cull	twosided
    {
        map models/map_objects/yavin/tree09a
        alphaFunc GE128
        rgbGen lightingDiffuse
    }
}

models/map_objects/yavin/tree08b_b
{
	qer_editorimage	models/map_objects/yavin/tree08b
	q3map_nolightmap
	q3map_alphashadow
	q3map_onlyvertexlighting
	cull	twosided
    {
        map models/map_objects/yavin/tree08b
        alphaFunc GE128
        rgbGen lightingDiffuse
    }
}

models/map_objects/yavin/tree09b_b
{
	qer_editorimage	models/map_objects/yavin/tree09b
	q3map_nolightmap
	q3map_alphashadow
	q3map_onlyvertexlighting
	cull	twosided
    {
        map models/map_objects/yavin/tree09b
        alphaFunc GE128
        rgbGen lightingDiffuse
    }
}

models/map_objects/yavin/tree06b_b
{
	qer_editorimage	models/map_objects/yavin/tree06b
	surfaceparm	trans
	q3map_nolightmap
	q3map_onlyvertexlighting
	cull	twosided
    {
        map models/map_objects/yavin/tree06b
        alphaFunc GE128
        rgbGen lightingDiffuse
    }
}

models/map_objects/yavin/grass_tall_b
{
	qer_editorimage	models/map_objects/yavin/grass_tall
	q3map_nolightmap
	q3map_alphashadow
	q3map_onlyvertexlighting
	cull	twosided
    {
        map models/map_objects/yavin/grass_tall
        alphaFunc GE128
        rgbGen lightingDiffuse
    }
}

models/map_objects/yavin/grass_b
{
	qer_editorimage	models/map_objects/yavin/grass
	q3map_nolightmap
	q3map_alphashadow
	q3map_onlyvertexlighting
	cull	twosided
    {
        map models/map_objects/yavin/grass
        alphaFunc GE128
        rgbGen lightingDiffuse
    }
}

models/map_objects/yavin/fern3_b
{
	qer_editorimage	models/map_objects/yavin/fern3
	q3map_nolightmap
	q3map_alphashadow
	q3map_onlyvertexlighting
	cull	twosided
    {
        map models/map_objects/yavin/fern3
        alphaFunc GE128
        rgbGen lightingDiffuse
    }
}

models/map_objects/yavin/plant_b
{
	qer_editorimage	models/map_objects/yavin/plant
	q3map_nolightmap
	q3map_alphashadow
	q3map_onlyvertexlighting
	cull	twosided
    {
        map models/map_objects/yavin/plant
        alphaFunc GE128
        rgbGen lightingDiffuse
    }
}

models/map_objects/yavin/tree09_vines_b
{
	qer_editorimage	models/map_objects/yavin/tree09_vines
	q3map_nolightmap
	q3map_alphashadow
	q3map_onlyvertexlighting
	cull	twosided
    {
        map models/map_objects/yavin/tree09_vines
        alphaFunc GE128
        rgbGen lightingDiffuse
    }
}

models/map_objects/yavin/vines_b
{
	qer_editorimage	models/map_objects/yavin/vines
	q3map_nolightmap
	q3map_alphashadow
	q3map_onlyvertexlighting
	cull	twosided
    {
        map models/map_objects/yavin/vines
        alphaFunc GE128
        rgbGen lightingDiffuse
    }
}

models/map_objects/nar_shaddar/skiffaces
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map gfx/effects/bryar_blob
        blendFunc GL_ONE GL_ZERO
        rgbGen wave sin 0.5 0.25 0 1
        tcMod scroll 0.5 0
        tcMod scale 6 1
    }
    {
        map gfx/effects/bryar_blob
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 0.5 0.25 0 1
        tcMod scroll -0.5 0
        tcMod scale 6 1
    }
    {
        map models/map_objects/nar_shaddar/skiffaces
        alphaFunc GE128
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
}

models/map_objects/nar_shaddar/podcar
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map gfx/effects/shield
        blendFunc GL_ONE GL_ZERO
        rgbGen identity
        tcMod stretch sin 1.65 -1 0 3
    }
    {
        map models/map_objects/nar_shaddar/podcar
        blendFunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
        rgbGen lightingDiffuse
    }
}

models/map_objects/nar_shaddar/podfin
{
	qer_editorimage	models/map_objects/nar_shaddar/podfin
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map gfx/misc/redring
        blendFunc GL_ONE GL_ZERO
        rgbGen identity
        tcMod scroll 0 -2
        tcMod rotate -25
    }
    {
        map gfx/misc/redring
        blendFunc GL_ONE GL_ONE
        rgbGen identity
        tcMod scroll 0 2
        tcMod rotate 25
    }
    {
        map models/map_objects/nar_shaddar/podfin
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        depthWrite
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/nar_shaddar/podfinglow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave square 0 1 0 0.25
    }
}

models/map_objects/nar_shaddar/podpanels
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/nar_shaddar/podpanels
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/nar_shaddar/podpanelsglow
        blendFunc GL_ONE GL_ONE
        glow
    }
}

models/map_objects/nar_shaddar/light_wall
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/nar_shaddar/light_wall
        blendFunc GL_ONE GL_ZERO
        rgbGen exactVertex
    }
    {
        map models/map_objects/nar_shaddar/light_wallglow
        blendFunc GL_ONE GL_ONE
        glow
    }
}

models/map_objects/nar_shaddar/fishglass
{
	q3map_material	Glass
    {
        map models/map_objects/nar_shaddar/fishglass
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_SRC_COLOR
    }
}

models/map_objects/nar_shaddar/fishplants
{
	q3map_nolightmap
	cull	twosided
    {
        map models/map_objects/nar_shaddar/fishplants
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

models/map_objects/nar_shaddar/plant
{
	q3map_nolightmap
	q3map_alphashadow
	q3map_onlyvertexlighting
	cull	twosided
    {
        map models/map_objects/nar_shaddar/plant
        alphaFunc GE128
        rgbGen lightingDiffuse
    }
}

models/map_objects/nar_shaddar/jabacam
{
	q3map_nolightmap
    {
        map models/map_objects/nar_shaddar/gliter
        blendFunc GL_ONE GL_ZERO
        tcMod rotate 44
        tcMod scale 5 5
    }
    {
        map models/map_objects/nar_shaddar/jabacam
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen lightingDiffuse
    }
}

models/map_objects/cairn/bcpanel
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/cairn/bcpanel
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/cairn/bcpanelglow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 1 0.1 0 40
    }
}

models/map_objects/cairn/lightfaces
{
    {
        map models/map_objects/cairn/lightfaces
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/cairn/lightfacesglow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 0.5 0.005 0 50
    }
}

models/map_objects/cairn/controlpn_face
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/cairn/controlpn_face
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/cairn/controlpn_faceglow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 1 0.1 0 25
    }
}

models/map_objects/cairn/utilitylight_face
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/cairn/utilitylight_face
        blendFunc GL_ONE GL_ONE
    }
    {
        map models/map_objects/cairn/utilitylight_face
        blendFunc GL_ONE GL_ZERO
    }
}

models/map_objects/cairn/welder
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/cairn/welder
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/cairn/welder_glow
        blendFunc GL_ONE GL_ONE
    }
}

models/map_objects/cairn/abcrystal
{
	cull	twosided
    {
        map models/map_objects/cairn/abcrystal
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 0.75 0.05 0 2
    }
}

models/map_objects/bespin/statue_crystal
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/bespin/statue_crystal
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 1 0.1 0 25
    }
}

models/map_objects/bespin/sconce_light
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/bespin/sconce_light
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/bespin/sconce_lightglow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 1 0.1 0 0.25
    }
}

models/map_objects/bespin/scow_engine
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/bespin/scow_engine
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/bespin/scow_engine
        blendFunc GL_ONE GL_ZERO
        rgbGen wave sin 0.25 0.225 1 10
    }
    {
        map models/map_objects/bespin/scow_engine
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 1 0.225 -0.5 10
    }
}

models/map_objects/bespin/street_light
{
	q3map_nolightmap
    {
        map models/map_objects/bespin/street_light
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/bespin/street_light_glw
        blendFunc GL_ONE GL_ONE
        glow
    }
}

models/map_objects/bespin/twinpodccglow
{
	qer_editorimage	models/map_objects/bespin/twinpodcc
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/bespin/twinpodcc
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/bespin/twinpodccglow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 1 0.25 0 25
    }
    {
        map models/map_objects/bespin/twinpodccglow
        blendFunc GL_ONE GL_ONE
        glow
    }
}

models/map_objects/imp_detention/door_lock
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/imp_detention/door_lock
        blendFunc GL_ONE GL_ZERO
    }
    {
        map models/map_objects/imp_detention/door_lockglow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 0 1 0 1
    }
}

models/map_objects/imp_detention/tie_body_damage
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/imp_detention/tie_body_damage
        blendFunc GL_ONE GL_ZERO
    }
    {
        map models/map_objects/imp_detention/tie_body_damageglow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave noise 1 0.5 0 10
    }
    {
        map models/map_objects/imp_detention/tie_body_damageglow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave noise 0.5 0.5 -0.5 2
    }
}

models/weapons2/thermal/thermal_w_2
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/weapons2/thermal/thermal_w_1
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/weapons2/thermal/thermal_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 0.6 0.4 0 3
    }
}

models/map_objects/cinematics/rave_screen_pilot
{
	q3map_nolightmap
    {
        map models/map_objects/cinematics/rave_screen_pilot
        blendFunc GL_ONE GL_ZERO
    }
    {
        map models/map_objects/cinematics/rave_screen_pilot1
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave square 0 1 0 0.25
    }
    {
        map models/map_objects/cinematics/rave_screen_pilot2
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 0 1 -0.5 0.5
    }
    {
        map models/map_objects/cinematics/rave_screen_pilot3
        blendFunc GL_ONE GL_ONE
        glow
        detail
        rgbGen wave sin 1 0.5 -1 0.5
    }
    {
        map models/map_objects/cinematics/rave_screen_pilot4
        blendFunc GL_ONE GL_ONE
        glow
        detail
        rgbGen wave sawtooth 0 1 0 0.9
        tcMod scroll 1 0
    }
    {
        map models/map_objects/cinematics/rave_screen_pilot4
        blendFunc GL_ONE GL_ONE
        glow
        detail
        rgbGen wave sawtooth 0 1 0 1
        tcMod scroll 0 1
    }
}

models/map_objects/cinematics/rave_screen_copilot
{
    {
        map models/map_objects/cinematics/rave_screen_copilot2
        blendFunc GL_ONE GL_ZERO
        rgbGen wave sawtooth 0.5 1.3 1 1
    }
    {
        map models/map_objects/cinematics/rave_screen_pilot4
        blendFunc GL_DST_COLOR GL_ONE
        tcMod scroll -1 0
        tcMod scale 2 3
    }
}

models/map_objects/cinematics/imp_wall
{
	qer_editorimage	models/map_objects/cinematics/imp_wall
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map textures/impdetention/light_fx
        blendFunc GL_ONE GL_ZERO
        tcMod scroll -0.3 0
        tcMod scale 3 1
    }
    {
        map models/map_objects/cinematics/imp_wall
        blendFunc GL_ONE GL_SRC_ALPHA
    }
    {
        map models/map_objects/cinematics/imp_wallglow
        blendFunc GL_ONE GL_ONE
        glow
    }
}

models/map_objects/mp/force_boon
{
	q3map_nolightmap
	q3map_onlyvertexlighting
	cull	twosided
    {
        map models/map_objects/mp/force_boon
        alphaFunc GE128
        blendFunc GL_SRC_ALPHA GL_ZERO
        depthWrite
    }
    {
        map models/map_objects/mp/force_boon
        blendFunc GL_ONE GL_ZERO
        depthFunc equal
        rgbGen lightingDiffuse
    }
}

models/map_objects/mp/boon
{
	q3map_nolightmap
	q3map_onlyvertexlighting
	cull	twosided
    {
        map models/map_objects/mp/boon
        blendFunc GL_ONE GL_ONE
        glow
    }
    {
        clampmap models/map_objects/mp/boon_pulse1
        blendFunc GL_ONE GL_ONE
        glow
        tcMod rotate 50
    }
    {
        clampmap models/map_objects/mp/boon_pulse2
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sin 1 0.6 0.2 3
        tcMod rotate -25
    }
}

models/map_objects/mp/dark_forceother
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/mp/dark_forceother
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/mp/dark_forceother_glow
        blendFunc GL_ONE GL_ONE
        glow
    }
}

models/map_objects/mp/saber_throw
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/mp/saber_throw
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/mp/saber_throw_glow
        blendFunc GL_ONE GL_ONE
        glow
    }
}

models/map_objects/mp/light_telepathy
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/mp/light_telepathy
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/mp/light_telepathy_glow
        blendFunc GL_ONE GL_ONE
        glow
    }
}

models/map_objects/mp/light_heal
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/mp/light_heal
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/mp/light_heal_glow
        blendFunc GL_ONE GL_ONE
        glow
    }
}

models/map_objects/mp/dark_rage
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/mp/dark_rage
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/mp/dark_rage_glow
        blendFunc GL_ONE GL_ONE
        glow
    }
}

models/map_objects/mp/dark_enlight
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/mp/dark_enlight
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/mp/dark_enlight_glow
        blendFunc GL_ONE GL_ONE
        glow
    }
}

models/map_objects/mp/dark_grip
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/mp/dark_grip
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/mp/dark_grip_glow
        blendFunc GL_ONE GL_ONE
        glow
    }
}

models/map_objects/mp/dark_lightning
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/mp/dark_lightning
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/mp/dark_lightning_glow
        blendFunc GL_ONE GL_ONE
        glow
    }
}

models/map_objects/mp/dark_drain
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/mp/dark_drain
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/mp/dark_drain_glow
        blendFunc GL_ONE GL_ONE
        glow
    }
}

models/map_objects/mp/saber_attack
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/mp/saber_attack
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/mp/saber_attack_glow
        blendFunc GL_ONE GL_ONE
        glow
    }
}

models/map_objects/mp/saber_defend
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/mp/saber_defend
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/mp/saber_defend_glow
        blendFunc GL_ONE GL_ONE
        glow
    }
}

models/map_objects/mp/light_absorb
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/mp/light_absorb
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/mp/light_absorb_glow
        blendFunc GL_ONE GL_ONE
        glow
    }
}

models/map_objects/mp/light_enlight
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/mp/light_enlight
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/mp/light_enlight_glow
        blendFunc GL_ONE GL_ONE
        glow
    }
}

models/map_objects/mp/light_healother
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/mp/light_healother
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/mp/light_healother_glow
        blendFunc GL_ONE GL_ONE
        glow
    }
}

models/map_objects/mp/light_protect
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/mp/light_protect
        blendFunc GL_ONE GL_ZERO
    }
    {
        map models/map_objects/mp/light_protect_glow
        blendFunc GL_ONE GL_ONE
        glow
    }
}

models/map_objects/mp/force_telepathy
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/mp/force_telepathy
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/mp/light_telepathy_glow
        blendFunc GL_ONE GL_ONE
    }
}

models/map_objects/mp/force_pull
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/mp/force_pull
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/mp/force_pull_glow
        blendFunc GL_ONE GL_ONE
        glow
    }
}

models/map_objects/mp/force_push
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/mp/force_push
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/mp/force_push_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave triangle 0 1 0 1
        tcMod stretch sawtooth 0.9 0.8 0 1
    }
}

models/map_objects/mp/force_sight
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/mp/force_sight
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/mp/force_sight_glow
        blendFunc GL_ONE GL_ONE
        glow
    }
}

models/map_objects/mp/force_speed
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/mp/force_speed
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/mp/force_speed_glow
        blendFunc GL_ONE GL_ONE
        glow
    }
}

models/map_objects/mp/force_lightning
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/mp/force_lightning
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/mp/force_lightning_glow
        blendFunc GL_ONE GL_ONE
    }
}

models/map_objects/mp/force_jump
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/mp/force_jump
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/mp/force_jump_glow
        blendFunc GL_ONE GL_ONE
        glow
    }
}

models/map_objects/mp/force_heal
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/mp/force_heal
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/mp/light_heal_glow
        blendFunc GL_ONE GL_ONE
    }
}

models/map_objects/mp/force_grip
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/mp/force_grip
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/mp/force_grip_glow
        blendFunc GL_ONE GL_ONE
    }
}

models/map_objects/mp/shield
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/mp/shield_mp
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/mp/shield_glow
        blendFunc GL_ONE GL_ONE
        glow
    }
    {
        map models/map_objects/mp/shield_glow2
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave sawtooth 0 1 3 1
    }
}

models/map_objects/mp/medpac_mp
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/mp/medpac_mp
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/mp/health
        blendFunc GL_ONE_MINUS_DST_COLOR GL_ONE
        rgbGen wave sin 0 1 0 0.75
        tcMod scroll 0 3
    }
    {
        map models/map_objects/mp/health_field2
        blendFunc GL_DST_COLOR GL_ONE
        tcGen environment
        tcMod scroll 0.5 0.5
    }
}

models/map_objects/mp/shield_sm_mp
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/mp/shield_sm_mp
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/mp/shield_field
        blendFunc GL_DST_COLOR GL_ONE
        rgbGen wave triangle 0.6 1 0 1
        tcMod scroll 0 3
    }
    {
        map models/map_objects/mp/shield_field2
        blendFunc GL_DST_COLOR GL_ONE
        tcGen environment
        tcMod scroll 0.5 0.5
    }
}

models/map_objects/mp/bacta_mp
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/mp/bacta_mp
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/mp/health
        blendFunc GL_ONE_MINUS_DST_COLOR GL_ONE
        rgbGen wave sin 0 1 0 0.75
        tcMod scroll 0 3
    }
    {
        map models/map_objects/mp/health_field2
        blendFunc GL_DST_COLOR GL_ONE
        tcGen environment
        tcMod scroll 0.5 0.5
    }
}

models/map_objects/mp/psd_mp
{
	qer_editorimage	models/map_objects/mp/psd_mp
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/mp/psd_mp
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/mp/shield_field
        blendFunc GL_DST_COLOR GL_ONE
        rgbGen wave triangle 0.6 1 0 1
        tcMod scroll 0 3
    }
    {
        map models/map_objects/mp/shield_field2
        blendFunc GL_DST_COLOR GL_ONE
        tcGen environment
        tcMod entityTranslate
        tcMod scroll 0.5 0.5
    }
}

models/map_objects/doom/antenna
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map models/map_objects/doom/antenna
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/doom/antenna_glow
        alphaFunc GE192
        blendFunc GL_ONE GL_ONE
        tcMod scroll 0 2
    }
    {
        map models/map_objects/doom/antenna_glow2
        blendFunc GL_ONE GL_ONE
    }
}

models/chunks/metal/metal
{
	q3map_nolightmap
    {
        map models/chunks/metal/metal
        rgbGen lightingDiffuse
        tcMod scale 2 2
    }
}

models/chunks/metal/metal2
{
	q3map_nolightmap
    {
        map models/chunks/metal/metal2
        rgbGen lightingDiffuse
    }
}

models/chunks/rock/rock
{
    {
        map models/chunks/rock/rock
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
        tcMod scale 2 2
    }
}

models/chunks/rock/rock2
{
    {
        map models/chunks/rock/rock2
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
        tcMod scale 2 2
    }
}

models/chunks/rock/rock3
{
    {
        map models/chunks/rock/rock3
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
        tcMod scale 2 2
    }
}

models/chunks/crate/crate1
{
	q3map_nolightmap
    {
        map models/chunks/crate/crate1
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
        tcMod scale 2 2
    }
}

models/chunks/crate/crate2
{
	q3map_nolightmap
    {
        map models/chunks/crate/crate2
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
        tcMod scale 2 2
    }
    {
        map models/chunks/metal/metal_spec
        blendFunc GL_ONE_MINUS_DST_COLOR GL_ONE
    }
}

models/weapons2/stun_baton/blades
{
	q3map_nolightmap
    {
        map models/weapons2/stun_baton/blades
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map textures/cairn/lightning3
        blendFunc GL_ONE GL_ONE
        glow
        tcMod turb 0 1 0 1
        tcMod scroll 0 5
    }
    {
        map gfx/effects/protectionfield2
        blendFunc GL_ONE GL_ONE
        glow
        tcMod scroll -2 0
    }
}

textures/bespin/bench_test
{
	qer_editorimage	textures/bespin/n_wall03
	q3map_nolightmap
    {
        map textures/bespin/n_wall03
        rgbGen lightingDiffuse
        tcMod scale 1.4 4
        tcMod transform 0 1 1 0 0 0
    }
    {
        map models/map_objects/test/benchlight
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/impdetention/fc_pillar
{
	qer_editorimage	textures/impdetention/metal
	q3map_nolightmap
    {
        map textures/impdetention/metal2
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
}

textures/impdetention/fc_pillar_lite
{
	qer_editorimage	textures/impdetention/metal
	q3map_nolightmap
    {
        map models/map_objects/test/metal_test2
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/test/fc_pillar2_lite
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        rgbGen identity
    }
    {
        map models/map_objects/test/rust_test
        blendFunc GL_DST_COLOR GL_ONE
        tcMod scale 4 4
    }
}

models/map_objects/test/metal
{
	q3map_nolightmap
    {
        map models/map_objects/test/metal
        rgbGen lightingDiffuse
        tcMod scale 6 6
    }
    {
        map models/map_objects/test/fc_pillar2_lite
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

models/map_objects/test/fc_pillar2
{
	q3map_nolightmap
    {
        map models/map_objects/test/fc_pillar2
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/test/rust_test
        blendFunc GL_DST_COLOR GL_ONE
        rgbGen identity
        tcMod scale 4 4
    }
}

models/map_objects/test/beam_chris
{
	qer_editorimage	models/map_objects/test/beam
	q3map_nolightmap
    {
        map models/map_objects/test/beam
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
}

models/map_objects/ships/tie_fighter_body
{
    {
        map models/map_objects/ships/tie_fighter_body
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/ships/tie_fighter_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map models/map_objects/ships/tie_fighter_glow
        blendFunc GL_ONE GL_ONE
        glow
        detail
        rgbGen identity
    }
}

models/map_objects/ships/tie_fighter_wing
{
    {
        map models/map_objects/ships/tie_fighter_wing
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/ships/tie_fighter_wing_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/map_objects/ships/tie_bomber_wing2
{
    {
        map models/map_objects/ships/tie_bomber_wing2
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/ships/tie_bomber_wing_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/map_objects/ships/tie_bomber_window
{
	q3map_nolightmap
    {
        map models/map_objects/ships/tie_bomber_window
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/ships/tie_bomber_glass_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/map_objects/ships/tie_bomber
{
	q3map_nolightmap
    {
        map models/map_objects/ships/tie_bomber
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/ships/tie_bomber_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
    {
        map models/map_objects/ships/tie_bomber_glow
        blendFunc GL_ONE GL_ONE
        glow
        detail
    }
}

models/map_objects/ships/tie_bomber_new
{
	qer_editorimage	models/map_objects/ships/tie_bomber
	q3map_nolightmap
    {
        map models/map_objects/ships/tie_bomber
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/ships/tie_bomber_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/map_objects/ships/academy_pods
{
    {
        map $lightmap
        rgbGen identity
    }
    {
        map models/map_objects/ships/academy_pods
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

models/map_objects/ships/academy_body
{
    {
        map $lightmap
        rgbGen identity
    }
    {
        map models/map_objects/ships/academy_body
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map models/map_objects/ships/academy_body_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

models/map_objects/ships/academy_cockpit
{
    {
        map $lightmap
        rgbGen identity
    }
    {
        map models/map_objects/ships/academy_cockpit
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map models/map_objects/ships/academy_cockpit_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

models/map_objects/ships/academy_door
{
    {
        map $lightmap
        rgbGen identity
    }
    {
        map models/map_objects/ships/academy_door
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map models/map_objects/ships/academy_door_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

models/map_objects/ships/academy_engine
{
    {
        map $lightmap
        rgbGen identity
    }
    {
        map models/map_objects/ships/academy_engine
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map models/map_objects/ships/academy_engine_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

models/map_objects/ships/academy_frame
{
    {
        map $lightmap
        rgbGen identity
    }
    {
        map models/map_objects/ships/academy_frame
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map models/map_objects/ships/academy_frame_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

models/map_objects/ships/academy_beam
{
    {
        map $lightmap
        rgbGen identity
    }
    {
        map models/map_objects/ships/academy_beam
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

models/map_objects/hoth/infirmary_table_2_glasstube
{
	q3map_nolightmap
	cull	twosided
    {
        map models/map_objects/hoth/infirmary_table_2_glasstube
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map textures/common/env_chrome
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        alphaGen const 0.5
        tcGen environment
    }
}

models/map_objects/hoth/e_web_cannon
{
	q3map_nolightmap
    {
        map models/map_objects/hoth/e_web_cannon
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/hoth/e_webcannon_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/map_objects/hoth/e_web_bmerge
{
	surfaceparm	trans
	q3map_nolightmap
	cull	twosided
    {
        map models/map_objects/hoth/e_web_bmerge
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/hoth/eweb_bmerge_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/map_objects/hoth/turret_bottom
{
    {
        map models/map_objects/hoth/turret_bottom
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/hoth/turret_bottom_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/map_objects/hoth/turret_top
{
    {
        map models/map_objects/hoth/turret_top
    }
    {
        map models/map_objects/hoth/turret_top_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/map_objects/roof_top/podcar
{
	q3map_nolightmap
    {
        map models/map_objects/roof_top/podcar
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/roof_top/podcar_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/map_objects/roof_top/beam
{
	q3map_nolightmap
	cull	twosided
    {
        map models/map_objects/roof_top/beam
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

models/map_objects/imperial/projector
{
	q3map_nolightmap
    {
        map models/map_objects/imperial/projector
        rgbGen lightingDiffuse
    }
    {
        map models/map_objects/imperial/projector_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

