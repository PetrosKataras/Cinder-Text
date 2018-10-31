#pragma once

#include <unordered_map>

#include "cinder/gl/Texture.h"
#include "cinder/gl/Batch.h"
#include "cinder/gl/Fbo.h"
#include "cinder/Log.h"

#include "cinder/text/TextLayout.h"
#include "cinder/text/TextRenderer.h"
#include "cinder/text/Types.h"

#if defined( CINDER_GL_ES_2 )
#define CINDER_TEXTURE_RENDERER_USE_TEXTURE2D
#endif // ! defined( CINDER_GL_ES_2 )

namespace cinder { namespace text { namespace gl {

using TextureArrayRef= std::shared_ptr<class TextureArray>;

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
	std::map<uint32_t, ci::Rectf>&			getFreeRectangles() { return mFreeRectangles; }
	const std::map<uint32_t, ci::Rectf>&	getUsedRectangles() const { return mUsedRectangles; }
	std::map<uint32_t, ci::Rectf>&			getUsedRectangles() { return mUsedRectangles; }

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
	struct Format {
		Format(){}

		Format& size( ci::ivec3 size = vec3( 1024, 1024, 8 ) )	{ mSize = size; return *this; }
		Format& mipmap( bool enableMipmapping = true )			{ mMipmapping = enableMipmapping; return *this; }
		Format& maxAnisotropy( float maxAnisotropy )			{ mMaxAnisotropy = maxAnisotropy; return *this; }
		Format& internalFormat( GLint internalFormat )			{ mInternalFormat = internalFormat; return *this; }
		
	  protected:
		bool		mMipmapping { false };
		float		mMaxAnisotropy { -1 };
		GLint		mInternalFormat { GL_RED };
		ci::ivec3	mSize { ci::ivec3( 1024, 1024, 8 ) };

		friend class TextureArray;
		friend class TextureRenderer;
	};


  public:
	
	static TextureArrayRef create( const TextureArray::Format fmt = TextureArray::Format() ) { return std::make_shared<TextureArray>( fmt ); }

	TextureArray() {};
	TextureArray( const TextureArray::Format fmt);

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
	GLint		getWidth() const { return mSize.x; };
	//! Returns the height of the texture in pixels, ignoring clean bounds.
	GLint		getHeight() const { return mSize.y;};
	//! Returns the depth of the texture in pixels, ignoring clean bounds.
	GLint		getDepth() const { return mSize.z; };
	//! Returns texture array dimensions - z is used for TextureArrays
	ci::ivec3	getSize() const {	return mSize; }
	//! Returns the number of blocks use 
	GLint		getBlockCount() const { return mTextureIndices.size(); };

	const std::vector<TexturePack>& getTexturePacks() const { return mTexturePacks; }
	void update( ci::ChannelRef channel, int layerIdx );
	//! Expand the amount of texture blocks that we can write glyphs to 
	void expand();

	int									getTextureBlockIndex( int block ) const { return mTextureIndices[block]; }
	std::vector<int>					getTextureIndices() const { return mTextureIndices; }

protected:
	ci::ivec3				 mSize;
	std::vector<TexturePack> mTexturePacks;
	TextureArray::Format	 mFormat;
	std::vector<int>		 mTextureIndices;
};


//////////////////////////////////////////////////////////////////
class TextureRenderer {
  public:
	// Font + Glyph Caching (shared between all instances)
	typedef struct {
		int block = -1;		// index of TextureArray::mTextures vector - either Texture2d or Texture3d Array
		int layer = -1;		// layer within the block - always one when sing Texture2d. Otherwise an index up to the depth within TextureArray
		ci::vec2 size;
		ci::vec2 offset;
		ci::vec2 subTexSize;
		ci::vec2 subTexOffset;
	} GlyphCache;

	typedef struct {
		TextureArrayRef texArray;
		ci::ChannelRef	layerChannel;
		int				currentLayerIdx;
		bool			filled = false;
	} TexArrayCache;

	typedef struct {
		std::map<uint16_t, GlyphCache > glyphs;
		TexArrayCache					texArrayCache;
	} FontCache;

	// cache struct for rendering
	
	typedef struct {
		ci::gl::BatchRef batch;
		int textureIndex;
		int count;
	} GlyphBatch;

	typedef struct {
		ci::Rectf bounds;
		std::vector< GlyphBatch > batches;
	} LayoutCache;

	typedef struct {
		std::vector<vec4> posScales;
		std::vector<vec3> texCoords;
		std::vector<vec2> texCoordSizes;
		std::vector<vec4> colors;
		int textureIndex;
		int glyphCount;
	} BatchCacheData;



  public:
	TextureRenderer();

	static void enableSharedCaches( bool enable = true ) { mSharedCacheEnabled = enable; };
	static void setTextureFormat( TextureArray::Format fmt ) { mTextureArrayFormat = fmt; };

	static void loadFont( const Font& font, bool loadEntireFont = false );
	static void unloadFont( const Font& font );

	static void cacheGlyphs( const Font& font, const std::string string, const std::string language = "en", Script script = Script::LATIN, Direction dir = Direction::LTR );
	static void cacheGlyphs( const Font& font, const std::vector<uint32_t> &glyphIndices );
	static void cacheGlyphs( const Font& font, const std::vector<std::pair<uint32_t, uint32_t>> &unicodeRange );

	LayoutCache cacheLine( const cinder::text::Layout::Line &line );
	LayoutCache cacheLayout( const cinder::text::Layout &layout );

	static std::string defaultChars() { return "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890().?!,:;'\"&*=+-/\\@#_[]<>%^llflfiphrids\303\251\303\241\303\250\303\240"; }
	// https://en.wikipedia.org/wiki/Latin_script_in_Unicode
	static std::vector<std::pair<uint32_t, uint32_t>> defaultUnicodeRange() { return { { 0x0000, 0x007F }, { 0x0080, 0x00FF }, { 0xFB00, 0xFB06 } }; }

	FontCache& getCacheForFont( const Font& font );
	std::map<uint16_t, GlyphCache> getGylphMapForFont( const Font &font );
#ifdef CINDER_TEXTURE_RENDERER_USE_TEXTURE2D
	std::vector<ci::gl::Texture2dRef> getTexturesForFont(const Font& font)
	{
		std::vector<ci::gl::Texture2dRef> textures;
		for( auto texIndex : getCacheForFont(font).texArrayCache.texArray->getTextureIndices() ) {
			textures.push_back( mTextures[texIndex] );
		}
		return textures;
	}
#else
	std::vector<ci::gl::Texture3dRef> getTexturesForFont( const Font& font )
	{
		std::vector<ci::gl::Texture3dRef> textures;
		for( auto texIndex : getCacheForFont(font).texArrayCache.texArray->getTextureIndices() ) {
			textures.push_back( mTextures[texIndex] );
		}
		return textures;
	}
#endif

	void render( const std::vector<cinder::text::Layout::Line>& lines );
	void render( const cinder::text::Layout& layout );
	void render( const cinder::text::gl::TextureRenderer::LayoutCache &line );

	std::vector<std::pair<uint32_t, ci::ivec2>> getGlyphMapForLayout( const cinder::text::Layout& layout );
	static void printCachedFonts() {
		for( auto font : fontCache ) {
			CI_LOG_V( font.first );
		}
	}

  private:
	ci::gl::BatchRef	mBatch;

	void TextureRenderer::cacheRun( std::set<int> &textureSet, std::unordered_map<int, BatchCacheData> &batchCaches, const Layout::Run& run, ci::Rectf& bounds );
	std::vector< GlyphBatch > generateBatches(const std::unordered_map<int, BatchCacheData> &batchCaches );

	static void cacheFont( const Font& font, bool cacheEntireFont = false );
	
	static void uncacheFont( const Font& font );

	static std::unordered_map<Font, FontCache>	fontCache;
	
	//! The TextureArray cache, when mSharedCacheEnabled is set to true
	static TexArrayCache						mSharedTexArrayCache;
	//! Whether the texture cache is shared for all fonts, otherwise per-font
	static bool									mSharedCacheEnabled;
	//! Options for the texture array
	static TextureArray::Format					mTextureArrayFormat;
	//! Stores all textures use for font cache
#ifdef CINDER_TEXTURE_RENDERER_USE_TEXTURE2D
	static std::vector<ci::gl::Texture2dRef>	 mTextures;
#else
	static std::vector<ci::gl::Texture3dRef>	 mTextures;
#endif

	static void uploadChannelToTexture( TexArrayCache &texArrayCache );

	static TextureArrayRef makeTextureArray() { return TextureArray::create( mTextureArrayFormat ); };

	friend class TextureArray;
};

class TexturePackOutOfBoundExc : public ci::Exception {
public:
	TexturePackOutOfBoundExc() : ci::Exception() {}
};

} } } // namespace cinder::text::gl
