#pragma once

#include <unordered_map>

#include "cinder/gl/Texture.h"
#include "cinder/gl/Batch.h"
#include "cinder/gl/Fbo.h"

#include "cinder/text/TextLayout.h"
#include "cinder/text/TextRenderer.h"

namespace cinder { namespace text { namespace gl {

class TextureRenderer {
  public:
	TextureRenderer();

	static void loadFont( const Font& font );
	static void unloadFont( const Font& font );

	void render( const std::vector<cinder::text::Layout::Line>& lines );
	void render( const cinder::text::Layout& layout );

  protected:
	// Font + Glyph Caching (shared between all instances)
	typedef struct {
		ci::gl::Texture3dRef texArray;
		unsigned int layer;
		ci::vec2 subTexSize;
	} GlyphCache;

	typedef struct {
		std::map<uint32_t, GlyphCache > glyphs;
	} FontCache;

	bool		mMipmapping { false };
	FontCache& getCacheForFont( const Font& font );

  private:
	ci::gl::BatchRef mBatch;

	
	static void cacheFont( const Font& font );
	static void uncacheFont( const Font& font );

	static std::unordered_map<Font, FontCache> fontCache;
};

} } } // namespace cinder::text::gl
