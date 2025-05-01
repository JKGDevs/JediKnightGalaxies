//female tusken shaders by Noodle

models/players/tuskenf/fabric_alpha
{
	cull	twosided
    {
        map models/players/tuskenf/fabric_alpha
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen lightingDiffuse
    }
}

models/players/tuskenf/helmet
{
    {
        map models/players/tuskenf/helmet
        rgbGen lightingDiffuse
    }
    {
        map models/players/tuskenf/helmet_s
        blendFunc GL_SRC_ALPHA GL_ONE
        alphaGen lightingSpecular
	detail
    }
}

models/players/tuskenf/decorations
{
    {
        map models/players/tuskenf/decorations
        rgbGen lightingDiffuse
    }
    {
        map models/players/tuskenf/decorations_s
        blendFunc GL_SRC_ALPHA GL_ONE
        alphaGen lightingSpecular
	detail
    }
}

models/players/tuskenf/gloves
{
    {
        map models/players/tuskenf/gloves
        rgbGen lightingDiffuse
    }
    {
        map models/players/tuskenf/gloves_s
        blendFunc GL_SRC_ALPHA GL_ONE
        alphaGen lightingSpecular
	detail
    }
}