//
// LightSpacePerspectiveShadowTechnique.cpp
//
// Implement an OSG shadow technique for vtosg.
//
// Copyright (c) 2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"

#if (OPENSCENEGRAPH_MAJOR_VERSION==2 && OPENSCENEGRAPH_MINOR_VERSION>6) || OPENSCENEGRAPH_MAJOR_VERSION>2

#include "LightSpacePerspectiveShadowTechnique.h"
#include <osg/Texture3D>
#include <sstream>

CLightSpacePerspectiveShadowTechnique::CLightSpacePerspectiveShadowTechnique()
{
	// Override shaders here
	_mainVertexShader = NULL;
	_shadowVertexShader = NULL;
	_shadowFragmentShader = NULL;
	_mainFragmentShader = NULL;

	int MaxFragmentShaderTextureUnits;
	glGetIntegerv(0x8872, &MaxFragmentShaderTextureUnits);
	m_JitterTextureUnit = MaxFragmentShaderTextureUnits - 1;
}

void CLightSpacePerspectiveShadowTechnique::ViewData::init(ThisClass * st, osgUtil::CullVisitor * cv)
{
	// Reset the main shader
	st->_mainFragmentShader = new osg::Shader( osg::Shader::FRAGMENT, st->GenerateFragmentShaderSource());
	BaseClass::ViewData::init( st, cv );
	// The base class init has set up a fake texture for the base texture unit
	// so add this texture to any of the other active units
	osg::Texture* pFakeTex = dynamic_cast<osg::Texture*>(_stateset->getTextureAttribute(st->_baseTextureUnit, osg::StateAttribute::TEXTURE));
	osg::ref_ptr<osg::IntArray> UnitArray = new osg::IntArray;
	for (std::map<uint, uint>::iterator iTr = st->m_AdditionalTerrainTextureUnits.begin();
			iTr != st->m_AdditionalTerrainTextureUnits.end(); iTr++)
	{
		uint Unit = (*iTr).first;
		UnitArray->push_back(Unit);
		_stateset->setTextureAttribute(Unit, pFakeTex, osg::StateAttribute::ON);
		_stateset->setTextureMode(Unit,GL_TEXTURE_1D, osg::StateAttribute::OFF);
		_stateset->setTextureMode(Unit,GL_TEXTURE_2D, osg::StateAttribute::ON);
		_stateset->setTextureMode(Unit,GL_TEXTURE_3D, osg::StateAttribute::OFF);
	}
	// Add additional uniforms here
	_stateset->addUniform(new osg::Uniform("renderingVTPBaseTexture", false));
	st->InitJittering(_stateset.get());
	_stateset->addUniform(new osg::Uniform("VTPJitterTextureUnit", int(dynamic_cast<CLightSpacePerspectiveShadowTechnique*>(st)->m_JitterTextureUnit)));
	_texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::NEAREST); // Kill PCF filtering
	_texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::NEAREST);

	osg::Uniform *pUniform = new osg::Uniform(osg::Uniform::INT, "LSPST_AdditionalTerrainTextures", st->m_AdditionalTerrainTextureUnits.size());
	pUniform->setArray(UnitArray.get());
	_stateset->addUniform(pUniform);
}

void CLightSpacePerspectiveShadowTechnique::InitJittering(osg::StateSet *pStateSet)
{
    // create a 3D texture with hw mipmapping
    osg::Texture3D* pJitterTexture = new osg::Texture3D;
    pJitterTexture->setFilter(osg::Texture3D::MIN_FILTER,osg::Texture3D::NEAREST);
    pJitterTexture->setFilter(osg::Texture3D::MAG_FILTER,osg::Texture3D::NEAREST);
    pJitterTexture->setWrap(osg::Texture3D::WRAP_S,osg::Texture3D::REPEAT);
    pJitterTexture->setWrap(osg::Texture3D::WRAP_T,osg::Texture3D::REPEAT);
    pJitterTexture->setWrap(osg::Texture3D::WRAP_R,osg::Texture3D::REPEAT);
    pJitterTexture->setUseHardwareMipMapGeneration(true);

    const uint size = 16;
    const uint gridW =  8;
    const uint gridH =  8;
    uint R = (gridW * gridH / 2);
    pJitterTexture->setTextureSize(size, size, R);

    // then create the 3d image to fill with jittering data
    osg::Image* pJitterImage = new osg::Image;
    uchar *pJitterData = new uchar[size * size * R * 4];

    for ( uint s = 0; s < size; ++s )
    {
        for ( uint t = 0; t < size; ++t )
        {
            float v[4], d[4];

            for ( uint r = 0; r < R; ++r )
            {
                const int x = r % ( gridW / 2 );
                const int y = ( gridH - 1 ) - ( r / (gridW / 2) );

                // Generate points on a  regular gridW x gridH rectangular
                // grid.   We  multiply  x   by  2  because,  we  treat  2
                // consecutive x  each loop iteration.  Add 0.5f  to be in
                // the center of the pixel. x, y belongs to [ 0.0, 1.0 ].
                v[0] = float( x * 2     + 0.5f ) / gridW;
                v[1] = float( y         + 0.5f ) / gridH;
                v[2] = float( x * 2 + 1 + 0.5f ) / gridW;
                v[3] = v[1];

                // Jitter positions. ( 0.5f / w ) == ( 1.0f / 2*w )
                v[0] += ((float)rand() * 2.f / RAND_MAX - 1.f) * ( 0.5f / gridW );
                v[1] += ((float)rand() * 2.f / RAND_MAX - 1.f) * ( 0.5f / gridH );
                v[2] += ((float)rand() * 2.f / RAND_MAX - 1.f) * ( 0.5f / gridW );
                v[3] += ((float)rand() * 2.f / RAND_MAX - 1.f) * ( 0.5f / gridH );

                // Warp to disk; values in [-1,1]
                d[0] = sqrtf( v[1] ) * cosf( 2.f * 3.1415926f * v[0] );
                d[1] = sqrtf( v[1] ) * sinf( 2.f * 3.1415926f * v[0] );
                d[2] = sqrtf( v[3] ) * cosf( 2.f * 3.1415926f * v[2] );
                d[3] = sqrtf( v[3] ) * sinf( 2.f * 3.1415926f * v[2] );

                // store d into unsigned values [0,255]
                const uint tmp = ( (r * size * size) + (t * size) + s ) * 4;
                pJitterData[ tmp + 0 ] = (uchar)( ( 1.f + d[0] ) * 127  );
                pJitterData[ tmp + 1 ] = (uchar)( ( 1.f + d[1] ) * 127  );
                pJitterData[ tmp + 2 ] = (uchar)( ( 1.f + d[2] ) * 127  );
                pJitterData[ tmp + 3 ] = (uchar)( ( 1.f + d[3] ) * 127  );
            }
        }
    }
    pJitterImage->setImage(size, size, R, GL_RGBA4, GL_RGBA, GL_UNSIGNED_BYTE, pJitterData, osg::Image::USE_NEW_DELETE);
    pJitterTexture->setImage(pJitterImage);

    pStateSet->setTextureAttributeAndModes(m_JitterTextureUnit, pJitterTexture, osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE);
}


std::string CLightSpacePerspectiveShadowTechnique::GenerateFragmentShaderSource()
{
	std::stringstream ShaderSource;
	ShaderSource
		// Number of pixels over which to tile the jitter map
		<< "#define JITTER_MAP_TILING 32" << std::endl
		// Size of penumbra as a fraction of this fragments shadow map eye space z
		<< "#define PENUMBRA_SIZE_FACTOR 0.005" << std::endl

		<< "uniform bool renderingVTPBaseTexture;" << std::endl;
	if (m_AdditionalTerrainTextureUnits.size())
		ShaderSource << "uniform sampler2D LSPST_AdditionalTerrainTextures[" << m_AdditionalTerrainTextureUnits.size() << "];" << std::endl;
	ShaderSource
		<< "uniform sampler2D baseTexture;" << std::endl
        << "uniform sampler2DShadow shadowTexture;" << std::endl
        << "uniform sampler3D VTPJitterTextureUnit;" << std::endl

		<< "void main(void)" << std::endl
		<< "{" << std::endl
		// The following code assumes that the return value from shadow map lookup will
		// be zero or one. This appears not to be the case when the mag filter is set
		// to LINEAR when it looks like some PCF filtering is done. This may only apply on
		// certain GPUs.
		<< "  float shadow = 0.0;" << std::endl
		<< "  float shadow2 = 0.0;" << std::endl
		<< "  int totalSamples = 64;" << std::endl
		<< "  int testSamples = 8;" << std::endl
		<< "  vec4 shadowMapCoord = gl_TexCoord[" << _shadowTextureUnit << "];" << std::endl
		<< "  vec4 smCoord = shadowMapCoord;" << std::endl
		// shadowMapCoord.w holds the distance this fragment is from the shadow
		// casting camera (-shadoweyespace.z) normally always positive.
		// I am not sure how well this works in LISPSM as the draw bounds camera
		// seems to be always nearer than the view bounds camera giving a smaller penumbra.
		<< "  float penumbraWidth = PENUMBRA_SIZE_FACTOR * shadowMapCoord.w;" << std::endl
		<< "  vec3 jitterCoord = vec3( gl_FragCoord.xy/JITTER_MAP_TILING, 0.0 );" << std::endl
		<< "  gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);" << std::endl
		<< "  for(int i = 0; i < testSamples/2; i++)" << std::endl
		<< "  {" << std::endl
		<< "    vec4 offset = (2.0 * texture3D(VTPJitterTextureUnit, jitterCoord)) - 1.0;" << std::endl
		<< "    jitterCoord.z += 1.0/32.0;" << std::endl
		<< "    smCoord.xy = offset.xy * penumbraWidth + shadowMapCoord.xy;" << std::endl
		<< "    shadow += shadow2DProj(shadowTexture, smCoord).r;" << std::endl
		<< "    smCoord.xy = offset.zw * penumbraWidth + shadowMapCoord.xy;" << std::endl
		<< "    shadow += shadow2DProj(shadowTexture, smCoord).r;" << std::endl
		<< "  }" << std::endl
		<< "  shadow /= testSamples;" << std::endl
		<< "  if((shadow - 1) * shadow != 0)" << std::endl
		<< "  {" << std::endl
		<< "    for(int i = 0; i < (totalSamples - testSamples)/2; i++)" << std::endl
		<< "    {" << std::endl
		<< "      vec4 offset = (2.0 * texture3D(VTPJitterTextureUnit, jitterCoord)) - 1.0;" << std::endl
		<< "      jitterCoord.z += 1.0/32.0;" << std::endl
		<< "      smCoord.xy = offset.xy * penumbraWidth + shadowMapCoord.xy;" << std::endl
		<< "      shadow2 += shadow2DProj(shadowTexture, smCoord);" << std::endl
		<< "      smCoord.xy = offset.zw * penumbraWidth + shadowMapCoord.xy;" << std::endl
		<< "      shadow2 += shadow2DProj(shadowTexture, smCoord);" << std::endl
		<< "    }" << std::endl
		<< "    shadow = (shadow * testSamples + shadow2) / totalSamples;" << std::endl
		<< "  }" << std::endl

		<< "  vec4 colorAmbientEmissive;" << std::endl
		<< "  if (renderingVTPBaseTexture)" << std::endl
		// Lighting is off so make up a sensible value
		// I could pass this in as a parameter
		<< "    colorAmbientEmissive = vec4(0.4, 0.4, 0.4, 1.0);" << std::endl
		<< "  else" << std::endl
		<< "    colorAmbientEmissive =  gl_FrontLightModelProduct.sceneColor" << std::endl
		<< "                            +  gl_FrontLightProduct[0].ambient;" << std::endl

		// A dummy white texture has been applied by default to all geometry
		// so the call to texture2d will always return a valid value
		<< "  vec4 Colour = texture2D( baseTexture, gl_TexCoord[" << _baseTextureUnit << "].xy );" << std::endl;

	// Apply any additional terrain textures before the shadow is applied
	if (m_AdditionalTerrainTextureUnits.size())
	{
		ShaderSource
			<< "  if (renderingVTPBaseTexture)" << std::endl
			<< "  {" << std::endl
			<< "    vec3 TempColour;" << std::endl
			<< "    vec4 TexColour;" << std::endl;
		int Index = 0;
		for (std::map<uint, uint>::iterator iTr = m_AdditionalTerrainTextureUnits.begin();
							iTr != m_AdditionalTerrainTextureUnits.end(); iTr++)
		{
			uint Unit = (*iTr).first;
			uint Mode = (*iTr).second;
			switch (Mode)
			{
			case GL_ADD:
				ShaderSource
					<< "    TexColour = texture2D(LSPST_AdditionalTerrainTextures["
					<< Index << "], gl_TexCoord["
					<< Unit << "].xy);" << std::endl
					<< "    Colour.rgb += TexColour.rgb;" << std::endl
					<< "    Colour.a *= TexColour.a;" << std::endl
					<< "    Colour = clamp(Colour, 0.0. 1.0);" << std::endl;
				break;
			case GL_BLEND:
				ShaderSource
					<< "    TexColour = texture2D(LSPST_AdditionalTerrainTextures["
					<< Index << "], gl_TexCoord["
					<< Unit << "].xy);" << std::endl
					<< "    TempColour = mix(Colour.rgb, gl_TextureEnvColor[" << Index << "].rgb, TexColour.rgb);" << std::endl
					<< "    Colour = vec4(TempColour, Colour.a * TexColour.a);" << std::endl;
				break;
			case GL_REPLACE:
				ShaderSource
					<< "    Colour = texture2D(LSPST_AdditionalTerrainTextures["
					<< Index << "], gl_TexCoord["
					<< Unit << "].xy);" << std::endl;
				break;
			case GL_MODULATE:
				ShaderSource
					<< "    Colour *= texture2D(LSPST_AdditionalTerrainTextures["
					<< Index << "], gl_TexCoord["
					<< Unit << "].xy);" << std::endl;
				break;
			case GL_DECAL:
				ShaderSource
					<< "    TexColour = texture2D(LSPST_AdditionalTerrainTextures["
					<< Index << "], gl_TexCoord["
					<< Unit << "].xy);" << std::endl
					<< "    TempColour = mix(Colour.rgb, TexColour.rgb, TexColour.a);" << std::endl
					<< "    Colour = vec4(TempColour, Colour.a);" << std::endl;
				break;
			}
			Index++;
		}
		ShaderSource
			<< "  }" << std::endl;
	}
	ShaderSource
		<< "  Colour *= mix( colorAmbientEmissive, gl_Color, shadow );" << std::endl
		<< "  gl_FragColor = Colour;" << std::endl
		<< "}" << std::endl;
	return ShaderSource.str();
}

void CLightSpacePerspectiveShadowTechnique::AddAdditionalTerrainTextureUnit(const uint Unit, const uint Mode)
{
	m_AdditionalTerrainTextureUnits[Unit] = Mode;
	dirty();
}

void CLightSpacePerspectiveShadowTechnique::RemoveAdditionalTerrainTextureUnit(const uint Unit)
{
	m_AdditionalTerrainTextureUnits.erase(Unit);
	dirty();
}

void CLightSpacePerspectiveShadowTechnique::RemoveAllAdditionalTerrainTextureUnits()
{
	m_AdditionalTerrainTextureUnits.clear();
	dirty();
}

#endif
