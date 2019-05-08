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


class TextureArray {

 public:
	 //! Defines a limited set of options for Textures in TextureArray. Valid for both Texture2d and Texture3d instances.
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

	//! Defines a rectangular area and layer index for a region of a texture where a glyph is located in a texture.
	struct Region {
		Region( const ci::Rectf &rect = Rectf::zero(), int layer = -1 ):
			rect( rect ), layer( layer )
		{};

		ci::Rectf rect;
		int		  layer;
	};

  public:
	
	static TextureArrayRef create( const TextureArray::Format fmt = TextureArray::Format() ) { return std::make_shared<TextureArray>( fmt ); }

	TextureArray() {};
	TextureArray( const TextureArray::Format fmt);

	//! Request the next valid area of a texture
	Region request( const ci::ivec2 &size, const ci::ivec2 &padding = ci::ivec2( 10 ) );
	//! Request the a valid area of a texture on a specific layer
	Region request( const ci::ivec2 &size, int layerIndex, const ci::ivec2 &padding = ci::ivec2( 10 ) );

	//! Returns the width of the texture in pixels, ignoring clean bounds.
	GLint		getWidth() const { return mSize.x; };
	//! Returns the height of the texture in pixels, ignoring clean bounds.
	GLint		getHeight() const { return mSize.y;};
	//! Returns the depth of the texture in pixels, ignoring clean bounds.
	GLint		getDepth() const { return mSize.z; };
	//! Returns texture array dimensions - z is used for TextureArrays
	ci::ivec3	getSize() const {	return mSize; }
	//! Returns the number of blocks/textures used 
	GLint		getBlockCount() const { return mTextureIndices.size(); };

	//! Returns a vector of all texture packs
	const std::vector<TexturePack>& getTexturePacks() const { return mTexturePacks; }
	//! Updates the active texture on the specified layer index to the content of the specified channel
	void update( ci::ChannelRef channel, int layerIdx );
	//! Expand the amount of texture blocks that we can write glyphs to 
	void expand();

	//! Get the texture index defined for the specified block index
	int					getTextureBlockIndex( int block ) const { return mTextureIndices[block]; }
	//! Get all all of the texture block indices referred to
	std::vector<int>	getTextureBlockIndices() const { return mTextureIndices; }

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

	// cache structs for rendering
	
	typedef struct {
//		ci::gl::BatchRef batch;
		ci::gl::VaoRef vao;
		ci::gl::VboMeshRef vboMesh;
		ci::gl::GlslProgRef shader;
		std::vector<int> texIndices;
		std::vector<std::pair<int, int>> ranges;
		int count;
	} GlyphBatch;

	typedef struct {
		std::vector<vec3> vertPositions;		// vertex position (normalized)
		std::vector<vec2> vertTexCoords;		// vertex texture coordination (normalized)
		std::vector<vec4> colors;				// glyph color (from run)

		std::vector<vec4> posSize;
		std::vector<vec3> texCoords;
		std::vector<vec2> texCoordSizes;

		std::vector<vec4> posOffsets;		// glyph index (within layout) in w slot for now
		std::vector<vec2> scaleOffsets;
		std::vector<vec4> colorOffsets;
		int textureIndex;
		int glyphCount;
	} BatchCacheData;

	typedef struct {
		ci::Rectf bounds;
		GlyphBatch batch;
		std::vector< ci::vec3 > positionOffsets;

		bool dynamicOffset;
		bool dynamicScale;
		bool dynamicColor;

		ci::gl::VboMesh::MappedAttrib<vec4> mapDynamicOffset() {
			return batch.vboMesh->mapAttrib4f( geom::CUSTOM_4 );
		}
		ci::gl::VboMesh::MappedAttrib<vec2> mapDynamicScale() {
			return batch.vboMesh->mapAttrib2f( geom::CUSTOM_5 );
		}
		ci::gl::VboMesh::MappedAttrib<vec4> mapDynamicColor() {
			return batch.vboMesh->mapAttrib4f( geom::CUSTOM_6 );
		}

		void applyOffsetAttr( ci::gl::VboMesh::MappedAttrib<vec4> &attrib, const ci::vec4 pos )
		{
			for( int i = 0; i < 6; ++i ){ *attrib++ = pos; }
		}
		void applyScaleAttr( ci::gl::VboMesh::MappedAttrib<vec2> &attrib, const ci::vec2 scale )
		{
			for( int i = 0; i < 6; ++i ){ *attrib++ = scale; }
		}
		void applyColorAttr( ci::gl::VboMesh::MappedAttrib<vec4> &attrib, const ci::ColorA color )
		{
			for( int i = 0; i < 6; ++i ){ *attrib++ = color; }
		}

		void unmapDynamicOffset( ci::gl::VboMesh::MappedAttrib<vec4> &attrib ) { attrib.unmap(); }
		void unmapDynamicScale( ci::gl::VboMesh::MappedAttrib<vec2> &attrib ) { attrib.unmap(); }
		void unmapDynamicColor( ci::gl::VboMesh::MappedAttrib<vec4> &attrib ) { attrib.unmap(); }

	} LayoutCache;

  public:
	TextureRenderer();

	//! Default characters to cache in string form
	static std::string defaultChars() { return "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890().?!,:;'\"&*=+-/\\@#_[]<>%^llflfiphrids\303\251\303\241\303\250\303\240"; }
	//! Default characters to cache as a unicode range, which is more inclusive than defaultChars() https://en.wikipedia.org/wiki/Latin_script_in_Unicode
	static std::vector<std::pair<uint32_t, uint32_t>> defaultUnicodeRange() { return { { 0x0000, 0x007F }, { 0x0080, 0x00FF }, { 0xFB00, 0xFB06 } }; }

	//! Enable all loaded fonts to use the same set of texture arrays
	static void enableSharedCaches( bool enable = true ) { mSharedCacheEnabled = enable; };
	//! Sets the texture format that cached font textures will use, including texture cache dimensions
	static void setTextureFormat( TextureArray::Format fmt ) { mTextureArrayFormat = fmt; };

	//! Staticly load/cache the specified font, optionally loading every glyph of the font
	static void loadFont( const Font& font, bool loadEntireFont = false );
	//! Renove the cached font from the cache
	static void unloadFont( const Font& font );
	//! Print all cached fonts to the console
	static void printCachedFonts() {
		for( auto font : fontCache ) {
			CI_LOG_V( font.first );
		}
	}

	//! Cache the specified glyphs for the specified font, providing the glyphs in string form
	static void cacheGlyphs( const Font& font, const std::string string, const std::string language = "en", Script script = Script::LATIN, Direction dir = Direction::LTR );
	//! Cache the specified glyphs for the specified font as a vector of glyph indices
	static void cacheGlyphs( const Font& font, const std::vector<uint32_t> &glyphIndices );
	//! Cache the specified glyphs for the specified font as a vector of unicode ranges
	static void cacheGlyphs( const Font& font, const std::vector<std::pair<uint32_t, uint32_t>> &unicodeRange );


	//! Cache the specified layout line. Returns a LayoutCache object, which can then be efficientll rendered later.
	LayoutCache cacheLine( const cinder::text::Layout::Line &line );
	//! Cache the specified layout. Returns a LayoutCache object, which can then be efficientll rendered later.
	LayoutCache cacheLayout( const cinder::text::Layout &layout, bool enableDynamicOffset = false, bool enableDynamicScale = false, bool enableDynamicColor = false );
	void updateCache( LayoutCache &layout );

	//! Returns the FontCache object, which can be used to render outside of the TextureRenderer object.
	FontCache& getCacheForFont( const Font& font );
	//! Returns a map of Glyph information for the specified font
	std::map<uint16_t, GlyphCache> getGylphMapForFont( const Font &font );
	//! Returns the textures used for the specified font
#ifdef CINDER_TEXTURE_RENDERER_USE_TEXTURE2D
	std::vector<ci::gl::Texture2dRef> getTexturesForFont(const Font& font)
	{
		std::vector<ci::gl::Texture2dRef> textures;
		for( auto texIndex : getCacheForFont(font).texArrayCache.texArray->getTextureBlockIndices() ) {
			textures.push_back( mTextures[texIndex] );
		}
		return textures;
	}
#else
	std::vector<ci::gl::Texture3dRef> getTexturesForFont( const Font& font )
	{
		std::vector<ci::gl::Texture3dRef> textures;
		for( auto texIndex : getCacheForFont(font).texArrayCache.texArray->getTextureBlockIndices() ) {
			textures.push_back( mTextures[texIndex] );
		}
		return textures;
	}
#endif

	//! Renders a vector of lines from a layout
	void render( const std::vector<cinder::text::Layout::Line>& lines );
	//! Renders a text Layout as a whole
	void render( const cinder::text::Layout& layout );
	//! Efficiently renders a LayoutCache object, which is preferred for repeated renders.
	void render( const cinder::text::gl::TextureRenderer::LayoutCache &line );

	//! Get the glyph map vector for a previously configured text Layout
	std::vector<std::pair<uint32_t, ci::ivec2>> getGlyphMapForLayout( const cinder::text::Layout& layout );

  private:
	ci::gl::BatchRef	mBatch;

	void cacheRun( std::unordered_map<int, BatchCacheData> &batchCaches, const Layout::Run& run, ci::Rectf& bounds );
	//std::vector< GlyphBatch > generateBatches(const std::unordered_map<int, BatchCacheData> &batchCaches );
	GlyphBatch generateBatch(const std::unordered_map<int, BatchCacheData> &batchCaches, bool enableDynamicOffset = false, bool enableDynamicScale = false, bool enableDynamicColor = false );

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
