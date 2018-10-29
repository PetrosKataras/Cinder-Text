#pragma once

#include "cinder/text/FontManager.h"
#include "cinder/gl/Texture.h"

namespace cinder { namespace text { namespace gl {

class SdfRenderer
{
public:
	SdfRenderer();
	~SdfRenderer();

	//static void setFontCacheFile( const fs::path& filePath );
	static void loadFont( const Font& font, bool loadEntireFont = false );

private:
	static void cacheFont( const Font& font, bool cacheEntireFont = false );
};

} } } // namespace cinder::text::gl
