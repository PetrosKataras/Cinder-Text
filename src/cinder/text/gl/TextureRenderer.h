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

	bool isAreaAvailable( const ci::Rectf &rect ) const; // TODO: (keep max size in freeRect instead of going through the whole list everytime)

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

class TextureArray : public ci::gl::TextureBase {
public:
	class Format {
	public:
		Format() : mInternalFormat( GL_RGBA8 ), mNumSamples( 0 ), mImmutable( false ) {}

		Format&		samples( size_t samples ) { mNumSamples = samples; return *this; }
		Format&		internalFormat( GLint format ) { mInternalFormat = format; return *this; }
		Format&		immutable( bool immutable = true ) { mImmutable = immutable; return *this; }

		size_t		getNumSamples() const { return mNumSamples; }
		GLint		getInternalFormat() const { return mInternalFormat; }
		bool		isImmutable() const { return mImmutable; }
	protected:
		size_t		mNumSamples;
		GLint		mInternalFormat;
		bool		mImmutable;
	};

	//static TextureArrayPtr create( const ci::ivec3 &size, const Format &format = Format() );
	static TextureArrayRef create( const ci::ivec3 &size, const Format &format = Format() );
	TextureArray( const ci::ivec3 &size, const Format &format = Format() );

	class Region {
	public:
		Region( TextureArray* atlas, const ci::Rectf &rect, uint32_t id, uint16_t page, const ci::ivec2 &padding );
		~Region();

		ci::vec2 getUpperLeftTexcoord() const;
		ci::vec2 getLowerRightTexcoord() const;

		ci::Rectf getInnerBounds() const;
		ci::Rectf getOuterBounds() const;

		uint16_t getPage() const;
		uint32_t getId() const;
		ci::vec2 getPadding() const;

		TextureArray* getTextureArray() const;

	protected:
		uint16_t mPage;
		uint32_t mId;
		ci::Rectf mRect;
		ci::ivec2 mPadding;
		TextureArray* mParent;
	};

	using RegionRef = std::shared_ptr<Region>;
	using RegionPtr = std::unique_ptr<Region>;

	RegionPtr request( const ci::ivec2 &size, const ci::ivec2 &padding = ci::ivec2( 10 ) );

	//! Returns the width of the texture in pixels, ignoring clean bounds.
	GLint	getWidth() const override;
	//! Returns the height of the texture in pixels, ignoring clean bounds.
	GLint	getHeight() const override;
	//! Returns the depth of the texture in pixels, ignoring clean bounds.
	GLint	getDepth() const override;

	ci::ivec3 getSize() const;

	GLuint getId() const;
	GLenum getTarget() const;
	GLenum getInternalFormat() const;

	bool isImmutable() const;
	uint8_t getSamples() const;

	const std::vector<TexturePack>& getTexturePacks() const { return mTexturePacks; }

protected:
	void printDims( std::ostream &os ) const override;
	void release( const Region* region );
	friend class Region;

	ci::ivec3 mSize;
	std::vector<TexturePack> mTexturePacks;
	bool mImmutable;
	uint8_t mSamples;
};

//////////////////////////////////////////////////////////////////

class TextureRenderer : public cinder::text::Renderer {
  public:
	TextureRenderer();

	//void draw( const std::string& text, const ci::vec2& size = ci::vec2( 0 ) ) override;
	//void draw( const std::string& text, const Font& font, const ci::vec2 size = ci::vec2( 0 ) ) override;
	void draw() override;
	void setLayout( const cinder::text::Layout& layout ) override;
	void setOffset( ci::vec2 offset ) { mOffset = offset; }

	static void loadFont( const Font& font );
	static void unloadFont( const Font& font );

	ci::gl::TextureRef getTexture();
	//const std::vector<TexturePack>& getTexturePacks() const { return mTexturePacks; }


  protected:
	// Font + Glyph Caching (shared between all instances)
	typedef struct {
		TextureArrayRef texArray;
		unsigned int layer;
		ci::vec2 subTexSize;
		ci::vec2 subTexOffset;
	} GlyphCache;

	typedef struct {
		std::map<uint32_t, GlyphCache > glyphs;
	} FontCache;

  private:
	// Texture (FBO) caching
	ci::vec2 mOffset; // amount to offset texture in FBO
	void renderToFbo();
	void allocateFbo( int size );
	
	ci::gl::FboRef		mFbo;
	ci::gl::BatchRef	mBatch;

	FontCache& getCacheForFont( const Font& font );
	static void cacheFont( const Font& font );
	static void uncacheFont( const Font& font );

	static std::unordered_map<Font, FontCache> fontCache;
	//static std::map<ci::gl::Texture3dRef, TexturePack> mTexturePacks;
	//static RegionPtr requestRegion( const ci::ivec2 &size, const ci::ivec2 &padding = ci::ivec2( 10 ) );
};

class TexturePackOutOfBoundExc : public ci::Exception {
public:
	TexturePackOutOfBoundExc() : ci::Exception() {}
};

} } } // namespace cinder::text::gl
