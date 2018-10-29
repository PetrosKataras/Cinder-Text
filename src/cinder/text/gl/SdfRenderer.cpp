#include "SdfRenderer.h"

#include "cinder/app/App.h"
#include "cinder/gl/gl.h"
#include "msdfgen/msdfgen.h"
#include "msdfgen/util.h"0

using namespace ci;
using namespace ci::app;
using namespace std;

namespace cinder { namespace text { namespace gl {

SdfRenderer::SdfRenderer()
{
}


SdfRenderer::~SdfRenderer()
{
}

/*void SdfRenderer::setFontCacheFile( const fs::path& filePath )
{
	// set the font cache fle location
}*/

void SdfRenderer::loadFont( const Font& font, bool loadEntireFont )
{
	// look for font in font cache filre
	// if font is found, load it from file
	// else

}

// Cache glyphs to gl texture array(s)
void SdfRenderer::cacheFont( const Font& font,  bool cacheEntireFont )
{
	if( cacheEntireFont ) {
		// entire font
		std::vector<uint32_t> glyphIndices = cinder::text::FontManager::get()->getGlyphIndices( font );
		//cacheGlyphs( font, glyphIndices );
	}
	else {
		// partial font
		//cacheGlyphs( font, defaultUnicodeRange() );
	}
}
/*
void SdfRenderer::cacheGlyphs(  const Font& font, const std::vector<uint32_t> &glyphIndices )
{
	
	
}*/

} } } // namespace cinder::text::gl