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

	//void draw( const std::string& text, const ci::vec2& size = ci::vec2( 0 ) ) override;
	//void draw( const std::string& text, const Font& font, const ci::vec2 size = ci::vec2( 0 ) ) override;
	//void draw() override;
	//void setLayout( const cinder::text::Layout& layout ) override;
	//void setOffset( ci::vec2 offset ) { mOffset = offset; }

	static void loadFont( const Font& font );
	static void unloadFont( const Font& font );

	//! Enables or disables mipmapping. Default is disabled.
 	//void enableMipmapping( bool enable = true ) { mMipmapping = enable; }
 	//bool hasMipmapping() const { return mMipmapping; }

	//ci::gl::TextureRef getTexture();

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

  private:
	// Texture (FBO) caching
	//ci::vec2 mOffset; // amount to offset texture in FBO
	//void renderToFbo( const cinder::text::Layout& layout );
	//void allocateFbo( int size );
	//ci::gl::FboRef mFbo;

	ci::gl::BatchRef mBatch;

	FontCache& getCacheForFont( const Font& font );
	static void cacheFont( const Font& font );
	static void uncacheFont( const Font& font );

	static std::unordered_map<Font, FontCache> fontCache;
};

} } } // namespace cinder::text::gl
