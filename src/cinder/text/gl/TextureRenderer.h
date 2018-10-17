#pragma once

#include <unordered_map>

#include "cinder/gl/Texture.h"
#include "cinder/gl/Batch.h"
#include "cinder/gl/Fbo.h"

#include "cinder/text/TextLayout.h"
#include "cinder/text/TextRenderer.h"

namespace cinder { namespace text { namespace gl {

using TextureArrayRef = std::shared_ptr<class TextureArray>;
using TextureArrayPtr = std::unique_ptr<class TextureArray>;

class TexturePack {
public:
	TexturePack();
	TexturePack( const ci::ivec2 &size );
	TexturePack( int width, int height );

	void init( int width, int height );

	std::pair<uint32_t, ci::Rectf> insert( const ci::ivec2 &size, bool merge = true );
	std::pair<uint32_t, ci::Rectf> insert( int width, int height, bool merge = true );

	void erase( uint32_t id, bool merge = true );

	const std::map<uint32_t, ci::Rectf>&	getFreeRectangles() const { return mFreeRectangles; }
	std::map<uint32_t, ci::Rectf>&		getFreeRectangles() { return mFreeRectangles; }
	const std::map<uint32_t, ci::Rectf>&	getUsedRectangles() const { return mUsedRectangles; }
	std::map<uint32_t, ci::Rectf>&		getUsedRectangles() { return mUsedRectangles; }

	void mergeFreeList( size_t maxIterations = 15 );

	void debugDraw() const;

protected:
	uint32_t mRectangleId;
	std::map<uint32_t, ci::Rectf> mUsedRectangles;
	std::map<uint32_t, ci::Rectf> mFreeRectangles;
};

//////////////////////////////////////////////////////////////////

class TextureArray {
public:
	
	static TextureArrayRef create( const ci::ivec3 &size ) { return std::make_shared<TextureArray>( size ); }

	TextureArray( const ci::ivec3 &size );

	struct Region {
		Region( const ci::Rectf &rect = Rectf::zero(), int layer = -1 ):
			rect( rect ), layer( layer )
		{};

		ci::Rectf rect;
		int		  layer;
	};

	Region request( const ci::ivec2 &size, const ci::ivec2 &padding = ci::ivec2( 10 ) );
	Region request( const ci::ivec2 &size, int layerIndex, const ci::ivec2 &padding = ci::ivec2( 10 ) );

	//! Returns the width of the texture in pixels, ignoring clean bounds.
	GLint	getWidth() const { return mSize.x; };
	//! Returns the height of the texture in pixels, ignoring clean bounds.
	GLint	getHeight() const { return mSize.y;};
	//! Returns the depth of the texture in pixels, ignoring clean bounds.
	GLint	getDepth() const { return mSize.z; };
	ci::ivec3 getSize() const {	return mSize; }

	ci::gl::Texture3dRef getTexture() const { return mTexture; }
	const std::vector<TexturePack>& getTexturePacks() const { return mTexturePacks; }

protected:
	ci::ivec3				 mSize;
	ci::gl::Texture3dRef	 mTexture;
	std::vector<TexturePack> mTexturePacks;
};

//////////////////////////////////////////////////////////////////

class TextureRenderer : public cinder::text::Renderer {
  public:
	// Font + Glyph Caching (shared between all instances)
	typedef struct {
		int layer = -1;
		ci::vec2 size;
		ci::vec2 offset;
		ci::vec2 subTexSize;
		ci::vec2 subTexOffset;
	} GlyphCache;

	typedef struct {
		TextureArrayRef texArray;
		ci::ChannelRef	layerChannel;
		int				currentLayerIdx;
	} TexArrayCache;

	typedef struct {
		std::map<uint16_t, GlyphCache > glyphs;
		TexArrayCache					texArrayCache;
	} FontCache;

  public:
	TextureRenderer();

	//void draw( const std::string& text, const ci::vec2& size = ci::vec2( 0 ) ) override;
	//void draw( const std::string& text, const Font& font, const ci::vec2 size = ci::vec2( 0 ) ) override;
	void draw() override;
	void setLayout( const cinder::text::Layout& layout ) override;
	void setOffset( ci::vec2 offset ) { mOffset = offset; }

	static void enableSharedCaches( bool enable = true ) { mSharedCacheEnabled = enable; };

	static void loadFont( const Font& font );
	static void unloadFont( const Font& font );

	static void cacheGlyphs( const Font& font, const std::string string, const std::string language = "en", hb_script_t script = HB_SCRIPT_LATIN, hb_direction_t dir = HB_DIRECTION_LTR );
	static void cacheGlyphs( const Font& font, const std::vector<uint32_t> &glyphIndices );
	static void cacheGlyphs( const Font& font, const std::vector<std::pair<uint32_t, uint32_t>> &unicodeRange );

	ci::gl::TextureRef getTexture();

	static std::string defaultChars() { return "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890().?!,:;'\"&*=+-/\\@#_[]<>%^llflfiphrids\303\251\303\241\303\250\303\240"; }
	//static std::pair<uint32_t, uint32_t> defaultUnicodeRange() { return { 0x0040, 0x007F }; }
	// https://en.wikipedia.org/wiki/Latin_script_in_Unicode
	static std::vector<std::pair<uint32_t, uint32_t>> defaultUnicodeRange() { return { { 0x0000, 0x007F }, { 0x0080, 0x00FF }, { 0xFB00, 0xFB06 } }; }

	FontCache& getCacheForFont( const Font& font );
	std::map<uint16_t, GlyphCache> getGylphMapForFont( const Font &font );
	ci::gl::Texture3dRef getTextureForFont( const Font& font );

  private:
	// Texture (FBO) caching
	ci::vec2 mOffset; // amount to offset texture in FBO
	void renderToFbo();
	void allocateFbo( int size );
	
	ci::gl::FboRef		mFbo;
	ci::gl::BatchRef	mBatch;

	
	static void cacheFont( const Font& font );
	
	static void uncacheFont( const Font& font );

	static std::unordered_map<Font, FontCache>	fontCache;
	
	//! The TextureArray cache, when mSharedCacheEnabled is set to true
	static TexArrayCache						mSharedTexArrayCache;
	//! Whether the texture cache is shared for all fonts, otherwise per-font
	static bool									mSharedCacheEnabled;

	static TextureArrayRef makeTextureArray();
	static void uploadChannelToTexture( TexArrayCache &texArrayCache );
};

class TexturePackOutOfBoundExc : public ci::Exception {
public:
	TexturePackOutOfBoundExc() : ci::Exception() {}
};

} } } // namespace cinder::text::gl
