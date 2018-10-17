#include "cinder/text/gl/TextureRenderer.h"

#include "cinder/gl/gl.h"
#include "cinder/gl/scoped.h"
#include "cinder/gl/ConstantConversions.h"
#include "cinder/GeomIo.h"
#include "cinder/ip/Fill.h"
#include "cinder/ip/Flip.h"
#include "cinder/Log.h"

#include "cinder/text/FontManager.h"

using namespace ci;
using namespace ci::app;
using namespace std;

namespace cinder { namespace text { namespace gl {

// Shared font cache
std::unordered_map<Font, TextureRenderer::FontCache> TextureRenderer::fontCache;

bool TextureRenderer::mSharedCacheEnabled = false;
TextureRenderer::TexArrayCache TextureRenderer::mSharedTexArrayCache;


// Shader
const char* vertShader = R"V0G0N(
#version 150

uniform mat4	ciModelViewProjection;

in vec4		ciPosition;
in vec2		ciTexCoord0;
in vec4		ciColor;

out highp vec2 texCoord;
out vec4 globalColor;

void main( void )
{
	texCoord = ciTexCoord0;
	globalColor = ciColor;
	gl_Position	= ciModelViewProjection * ciPosition;
}
)V0G0N";

const char* fragShader = R"V0G0N(
#version 150

uniform sampler2DArray uTexArray;
uniform uint uLayer;
uniform vec2 uSubTexSize;
uniform vec2 uSubTexOffset;

in vec2 texCoord;
in vec4 globalColor;

uniform vec4 runColor;

out vec4 color;

void main( void )
{ 
	vec3 coord = vec3((texCoord.x * uSubTexSize.x) + uSubTexOffset.x, (texCoord.y * uSubTexSize.y) + uSubTexOffset.y, uLayer);
	vec4 texColor = texture( uTexArray, coord );

	color = vec4(1.0, 1.0, 1.0, texColor.r);
	color = color * globalColor;
	//color = vec4(1.0,0.0,0.0,1.0);
}
)V0G0N";


GLint getMaxTextureSize()
{
	static GLint maxSize = -1;
	if( maxSize == -1 ) {
		glGetIntegerv( GL_MAX_TEXTURE_SIZE, &maxSize );
	}
	return maxSize;
}

TextureRenderer::TextureRenderer()
	: mOffset( ci::vec2() )
{
	ci::gl::GlslProgRef shader = ci::gl::GlslProg::create( vertShader, fragShader );
	shader->uniform( "uTexArray", 0 );

	if( mBatch == nullptr ) {
		mBatch = ci::gl::Batch::create( ci::geom::Rect( ci::Rectf( 0.f, 0.f, 1.f, 1.f ) ), shader );
	}
}

void TextureRenderer::setLayout( const Layout& layout )
{
	mLayout = layout;
	allocateFbo( std::max( mLayout.measure().x, mLayout.measure().y ) );
	renderToFbo();
}

ci::gl::TextureRef TextureRenderer::getTexture()
{
	return mFbo->getColorTexture();
}

void TextureRenderer::allocateFbo( int size )
{
	

	if( mFbo == nullptr || mFbo->getWidth() < size || mFbo->getHeight() < size ) {
		// Go up by pow2 until we get the new size
		int fboSize = 1;

		while( fboSize < size ) {
			fboSize *= 2;
		}

		// Allocate
		ci::gl::Fbo::Format fboFormat;
		ci::gl::Texture::Format texFormat;
		texFormat.setMagFilter( GL_NEAREST );
		texFormat.setMinFilter( GL_LINEAR );
		//fboFormat.setColorTextureFormat( ci::gl::Texture2d::Format().internalFormat( GL_RGBA32F ) );
		fboFormat.setColorTextureFormat( texFormat );

		GLint maxRenderBufferSize;
		glGetIntegerv( GL_MAX_RENDERBUFFER_SIZE_EXT, &maxRenderBufferSize );
		if( fboSize < maxRenderBufferSize ) {
			mFbo = ci::gl::Fbo::create( fboSize, fboSize, fboFormat );
		}
		else {
			CI_LOG_E( "Cannot allocate FBO, requested dimensions are bigger than maximum render buffer size." );
		}
	}
}

void TextureRenderer::renderToFbo()
{
	if( mFbo ) {
		// Set viewport
		ci::gl::ScopedViewport viewportScope( 0, 0, mFbo->getWidth(), mFbo->getHeight() );
		ci::gl::ScopedMatrices matricesScope;
		ci::gl::setMatricesWindow( mFbo->getSize(), true );

		// Draw text into FBO
		ci::gl::ScopedFramebuffer fboScoped( mFbo );
		ci::gl::clear( ci::ColorA( 0.0, 0.0, 0.0, 0.0 ) );

		ci::gl::ScopedBlendAlpha alpha;

		for( auto& line : mLayout.getLines() ) {
			for( auto& run : line.runs ) {
				ci::gl::color( ci::ColorA( run.color, run.opacity ) );

				for( auto& glyph : run.glyphs ) {
					// Make sure we have the glyph
					if( TextureRenderer::getCacheForFont( run.font ).glyphs.count( glyph.index ) != 0 ) {
						ci::gl::ScopedMatrices matrices;

						ci::gl::translate( ci::vec2( glyph.bbox.getUpperLeft() ) + mOffset );
						ci::gl::scale( glyph.bbox.getSize().x, glyph.bbox.getSize().y );

						auto fontCache = getCacheForFont( run.font );
						auto glyphCache = fontCache.glyphs[glyph.index];

						//auto tex = getCacheForFont( run.font ).glyphs[glyph.index].texArray->getTexture();
						auto tex = fontCache.texArrayCache.texArray->getTexture();

						//ci::gl::ScopedBlendAlpha alphaBlend;
						mBatch->getGlslProg()->uniform( "uLayer", (uint32_t)glyphCache.layer );

						//ci::vec2 subTexSize = glyph.bbox.getSize() / ci::vec2( tex->getWidth(), tex->getHeight() );
						mBatch->getGlslProg()->uniform( "uSubTexSize", glyphCache.subTexSize );
						mBatch->getGlslProg()->uniform( "uSubTexOffset", glyphCache.subTexOffset );

						ci::gl::ScopedTextureBind texBind( tex->getTarget(), tex->getId() );
						mBatch->draw();
					}
					else {
						//ci::app::console() << "Could not find glyph for index: " << glyph.index << std::endl;
					}
				}
			}
		}
	}
}


void TextureRenderer::draw()
{
	if( mFbo ) {
		ci::gl::ScopedBlendPremult blend;
		ci::gl::draw( mFbo->getColorTexture() );
	}
}

void TextureRenderer::loadFont( const Font& font )
{
	if( TextureRenderer::fontCache.count( font ) == 0 ) {
		TextureRenderer::cacheFont( font );
	}
}

void TextureRenderer::unloadFont( const Font& font )
{
	TextureRenderer::uncacheFont( font );
}

TextureRenderer::FontCache& TextureRenderer::getCacheForFont( const Font& font )
{
	if( !TextureRenderer::fontCache.count( font ) ) {
		TextureRenderer::cacheFont( font );
	}

	return TextureRenderer::fontCache[font];
}

// Cache glyphs to gl texture array(s)
void TextureRenderer::cacheFont( const Font& font, const std::string chars )
{
	// partial font
	cacheGlyphs( font, chars );

	// entire font
	//std::vector<uint32_t> glyphIndices = cinder::text::FontManager::get()->getGlyphIndices( font );
	//cacheGlyphs( font, glyphIndices );
}

void TextureRenderer::uncacheFont( const Font& font )
{
	if( !TextureRenderer::fontCache.count( font ) ) {
		std::unordered_map<Font, FontCache>::iterator it = TextureRenderer::fontCache.find( font );
		TextureRenderer::fontCache.erase( it );
		
		// TODO: 
		// - find region in rect pack and remove rect ref
		// - find texture layer and location and clear
	}
}

TextureArrayRef TextureRenderer::makeTextureArray()
{
	int layerCount = 16;

	//ivec3 atlasSize = ivec3( ivec2( glm::max( MIN_ATLAS_SIZE, getMaxTextureSize() / 4 ) ), 1 );
	ivec2 atlasSize = ivec2( 1024 );

	auto texArray = TextureArray::create( ivec3( atlasSize.x, atlasSize.y, layerCount ) );
	return texArray;
}

void TextureRenderer::cacheGlyphs( const Font& font, const std::string string )
{
	std::vector<uint32_t> glyphIndices = cinder::text::FontManager::get()->getGlyphIndices( font, string );
	cacheGlyphs( font, glyphIndices );
}

void TextureRenderer::cacheGlyphs( const Font& font, const std::vector<std::pair<uint32_t, uint32_t>> &unicodeRange )
{
	std::vector<uint32_t> glyphIndices;
	for( auto range : unicodeRange ){
		auto indices = cinder::text::FontManager::get()->getGlyphIndices( font, range );
		glyphIndices.insert( glyphIndices.end(), indices.begin(), indices.end() );
	}
	cacheGlyphs( font, glyphIndices );
}

void TextureRenderer::cacheGlyphs( const Font& font, const std::vector<uint32_t> &glyphIndices )
{
	bool dirty = false;
	
	// get Texture ARray cache based on whether we are using chared coche or not
	TexArrayCache *texArrayCache;
	if( TextureRenderer::mSharedCacheEnabled ) {
		texArrayCache = &TextureRenderer::mSharedTexArrayCache;
	}
	else {
		texArrayCache = &TextureRenderer::fontCache[font].texArrayCache;
	}

	// get or create the TextureArray
	if( !texArrayCache->texArray ){
		texArrayCache->texArray = makeTextureArray();
	}
	auto textureArray = texArrayCache->texArray;

	// get or create the Glyph Channel
	if( !texArrayCache->layerChannel ){
		texArrayCache->layerChannel = ci::Channel::create( textureArray->getWidth(), textureArray->getHeight() );
		ci::ip::fill( texArrayCache->layerChannel.get(), ( uint8_t )0 );
		texArrayCache->currentLayerIdx = 0;
	}

	auto layerChannel = texArrayCache->layerChannel;
	int layerIndex = texArrayCache->currentLayerIdx;

    for( auto glyphIndex : glyphIndices ) {

		auto glyphCache = TextureRenderer::fontCache[font].glyphs[glyphIndex];
		if( glyphCache.layer > -1 ) {
			continue;
		}
		else {
			CI_LOG_V( "cache this font" );
		}

        dirty = true;

		FT_BitmapGlyph glyph = cinder::text::FontManager::get()->getGlyphBitmap( font, glyphIndex );
		ci::ivec2 glyphSize( glyph->bitmap.width, glyph->bitmap.rows );
		ci::ivec2 padding = ci::ivec2( 4 ) - ( glyphSize % ci::ivec2( 4 ) );
		
		auto region = textureArray->request( glyphSize, layerIndex, ivec2( 2.0f ) );
	
		// if current layer is full, upload channel to texture, increment layer id, reset channel
		if( region.layer < 0 ) {
			
			uploadChannelToTexture( *texArrayCache );

			// clear channel
			ci::ip::fill( layerChannel.get(), ( uint8_t )0 );
			layerIndex++;
			texArrayCache->currentLayerIdx = layerIndex;

			// request a new region
			region = textureArray->request( glyphSize, layerIndex, ivec2( 2.0f ) );
		}

		ivec2 offset = region.rect.getUpperLeft();
		auto layer = region.layer;

		// fill channel
		ci::ChannelRef channel = ci::Channel::create( glyphSize.x, glyphSize.y, glyphSize.x * sizeof( unsigned char ), sizeof( unsigned char ), glyph->bitmap.buffer );
		if( channel->getData() ) {
			ci::ChannelRef flippedChannel = ci::Channel::create( glyphSize.x, glyphSize.y );
			ci::ip::fill( flippedChannel.get(), ( uint8_t )0 );
			ci::ip::flipVertical( *channel, flippedChannel.get() );
			ci::Channel8uRef expandedChannel = ci::Channel8u::create( glyphSize.x + padding.x, glyphSize.y + padding.y );
			ci::ip::fill( expandedChannel.get(), ( uint8_t )0 );
			expandedChannel->copyFrom( *flippedChannel, ci::Area( 0, 0, glyphSize.x, glyphSize.y ) );

			ci::Surface8u surface( *expandedChannel );
			int mipLevel = 0;
			GLint dataFormat;
			GLenum dataType;
			ci::gl::TextureBase::SurfaceChannelOrderToDataFormatAndType<uint8_t>( surface.getChannelOrder(), &dataFormat, &dataType );

			int w = surface.getWidth();
			int h = surface.getHeight();

			// update channel
			layerChannel->copyFrom( *expandedChannel, Area( 0, 0, w, h ), offset );
		}
		
		//TextureRenderer::fontCache[font].glyphs[glyphIndex].texArray = textureArray;
		TextureRenderer::fontCache[font].texArrayCache = *texArrayCache;
		TextureRenderer::fontCache[font].glyphs[glyphIndex].layer = layer;
		TextureRenderer::fontCache[font].glyphs[glyphIndex].size = ci::vec2( glyphSize );
		TextureRenderer::fontCache[font].glyphs[glyphIndex].size = ci::vec2( offset );
		TextureRenderer::fontCache[font].glyphs[glyphIndex].subTexOffset = ci::vec2( offset ) / ci::vec2( textureArray->getSize() );
		TextureRenderer::fontCache[font].glyphs[glyphIndex].subTexSize = ci::vec2( glyphSize ) / ci::vec2( textureArray->getSize() );
		
		CI_LOG_V( glyphIndex << " " << ci::vec2( offset ) / ci::vec2( textureArray->getSize() ) << " " << ci::vec2( glyphSize ) / ci::vec2( textureArray->getSize() ) );
    }

    // we need to reflect any characters we haven't uploaded
    if( dirty )
        uploadChannelToTexture( *texArrayCache );
}


void TextureRenderer::uploadChannelToTexture( TexArrayCache &texArrayCache )
{
	// upload channel to texture
	auto channel = texArrayCache.layerChannel;
	ci::Surface8u surface( *channel );
	int mipLevel = 0;
	GLint dataFormat;
	GLenum dataType;
	ci::gl::TextureBase::SurfaceChannelOrderToDataFormatAndType<uint8_t>( surface.getChannelOrder(), &dataFormat, &dataType );
	auto tex = texArrayCache.texArray->getTexture();
	tex->update(  (void*)surface.getData(), dataFormat, dataType, mipLevel, surface.getWidth(), surface.getHeight(), 1, 0, 0, texArrayCache.currentLayerIdx );
}

/*
TextureRenderer::TexArrayCache TextureRenderer::getTextureCache()
{
	if( mSharedCacheEnabled ) {
		return mSharedTexArrayCache;
	}
	else {
		CI_LOG_W( "Shared Cache for fonts is not enabled. Try passing in a Font reference." );
		return TexArrayCache();
	}
}

TextureRenderer::TexArrayCache TextureRenderer::getTextureCache( const Font& font )
{
	auto fontCache = TextureRenderer::fontCache[font].texArrayCache;
	return fontCache;
}*/

/////
TexturePack::TexturePack()
	: mRectangleId( 0 )
{}

TexturePack::TexturePack( int width, int height )
	: mRectangleId( 0 )
{
	init( width, height );
}

TexturePack::TexturePack( const ci::ivec2 &size )
	: TexturePack( size.x, size.y )
{}

void TexturePack::init( int width, int height )
{
	mUsedRectangles.clear();
	mFreeRectangles.clear();
	mFreeRectangles.insert( { mRectangleId++, ci::Rectf( vec2( 0.0f, 0.0f ), vec2( width, height ) ) } );
}

std::pair<uint32_t, ci::Rectf> TexturePack::insert( const ci::ivec2 &size, bool merge )
{
	return insert( size.x, size.y, merge );
}

std::pair<uint32_t, ci::Rectf> TexturePack::insert( int width, int height, bool merge )
{
	// Find where to put the new rectangle.
	uint32_t freeNodeId = 0;
	Rectf newRect = Rectf::zero();
	int bestScore = std::numeric_limits<int>::max();

	// Try each free rectangle to find the best one for placement.
	for( const auto &rect : mFreeRectangles ) {
		// If this is a perfect fit upright, choose it immediately.
		if( width == rect.second.getWidth() && height == rect.second.getHeight() ) {
			newRect = Rectf( rect.second.getUpperLeft(), rect.second.getUpperLeft() + vec2( width, height ) );
			bestScore = std::numeric_limits<int>::min();
			freeNodeId = rect.first;
			break;
		}
		// Does the rectangle fit upright?
		else if( width <= rect.second.getWidth() && height <= rect.second.getHeight() ) {
			int score = static_cast<int>( rect.second.getWidth() * rect.second.getHeight() ) - width * height;
			if( score < bestScore ) {
				newRect = Rectf( rect.second.getUpperLeft(), rect.second.getUpperLeft() + vec2( width, height ) );
				bestScore = score;
				freeNodeId = rect.first;
			}
		}
	}

	// Abort if we didn't have enough space in the bin.
	if( newRect.getWidth() == 0 || newRect.getHeight() == 0 ) {
		throw TexturePackOutOfBoundExc();
	}

	// Remove the space that was just consumed by the new rectangle.

	// Compute the lengths of the leftover area.
	const int w = static_cast<int>( mFreeRectangles[freeNodeId].getWidth() - newRect.getWidth() );
	const int h = static_cast<int>( mFreeRectangles[freeNodeId].getHeight() - newRect.getHeight() );

	// Placing newRect into mFreeRectangles[freeNodeId] results in an L-shaped free area, which must be split into
	// two disjoint rectangles. This can be achieved with by splitting the L-shape using a single line.
	// We have two choices: horizontal or vertical.	

	// Maximize the larger area == minimize the smaller area.
	// Tries to make the single bigger rectangle.
	bool splitHorizontal = ( newRect.getWidth() * h > w * newRect.getHeight() );

	// Perform the actual split.
	// Form the two new rectangles.
	Rectf bottom = Rectf::zero();
	Rectf right = Rectf::zero();
	if( splitHorizontal ) {
		bottom = Rectf( 0, 0, mFreeRectangles[freeNodeId].getWidth(), mFreeRectangles[freeNodeId].getHeight() - newRect.getHeight() ).getMoveULTo( vec2( mFreeRectangles[freeNodeId].getX1(), mFreeRectangles[freeNodeId].getY1() + newRect.getHeight() ) );
		right = Rectf( 0, 0, mFreeRectangles[freeNodeId].getWidth() - newRect.getWidth(), newRect.getHeight() ).getMoveULTo( vec2( mFreeRectangles[freeNodeId].getX1() + newRect.getWidth(), mFreeRectangles[freeNodeId].getY1() ) );
	}
	else { // Split vertically
		bottom = Rectf( 0, 0, newRect.getWidth(), mFreeRectangles[freeNodeId].getHeight() - newRect.getHeight() ).getMoveULTo( vec2( mFreeRectangles[freeNodeId].getX1(), mFreeRectangles[freeNodeId].getY1() + newRect.getHeight() ) );
		right = Rectf( 0, 0, mFreeRectangles[freeNodeId].getWidth() - newRect.getWidth(), mFreeRectangles[freeNodeId].getHeight() ).getMoveULTo( vec2( mFreeRectangles[freeNodeId].getX1() + newRect.getWidth(), mFreeRectangles[freeNodeId].getY1() ) );
	}

	// Add the new rectangles into the free rectangle pool if they weren't degenerate.
	if( bottom.getWidth() > 0 && bottom.getHeight() > 0 )
		mFreeRectangles.insert( { mRectangleId++, bottom } );
	if( right.getWidth() > 0 && right.getHeight() > 0 )
		mFreeRectangles.insert( { mRectangleId++, right } );

	// Remove old rectangle
	mFreeRectangles.erase( mFreeRectangles.find( freeNodeId ) );

	// Perform a Rectangle Merge step if desired.
	if( merge ) {
		mergeFreeList();
	}

	// Remember the new used rectangle.
	pair<uint32_t, Rectf> idRectPair = { mRectangleId++, newRect };
	mUsedRectangles.insert( idRectPair );

	return idRectPair;
}

void TexturePack::erase( uint32_t id, bool merge )
{
	if( mUsedRectangles.count( id ) ) {
		mFreeRectangles.insert( { mRectangleId++, mUsedRectangles[id] } );
		mUsedRectangles.erase( mUsedRectangles.find( id ) );

		if( merge ) {
			mergeFreeList();
		}
	}
}

void TexturePack::mergeFreeList( size_t maxIterations )
{
	// Do a Theta(n^2) loop to see if any pair of free rectangles could me merged into one.
	// Note that we miss any opportunities to merge three rectangles into one. (should call this function again to detect that)
	for( size_t n = 0; n < maxIterations; ++n ) {
		bool merged = false;

		for( auto i = mFreeRectangles.begin(); i != mFreeRectangles.end(); ++i ) {
			//for( auto j = mFreeRectangles.begin(); j != mFreeRectangles.end(); ++j ) {
			//if( i != j ) {
			for( auto j = std::next( i ); j != mFreeRectangles.end(); ++j ) {
				if( i->second.getWidth() == j->second.getWidth() && i->second.getX1() == j->second.getX1() ) {
					if( i->second.getY1() == j->second.getY1() + j->second.getHeight() ) {
						i->second.y1 -= j->second.getHeight();
						i->second.y2 -= j->second.getHeight();
						i->second.y2 += j->second.getHeight();
						j = std::prev( mFreeRectangles.erase( j ) );
						merged = true;
					}
					else if( i->second.getY1() + i->second.getHeight() == j->second.getY1() ) {
						i->second.y2 += j->second.getHeight();
						j = std::prev( mFreeRectangles.erase( j ) );
						merged = true;
					}
				}
				else if( i->second.getHeight() == j->second.getHeight() && i->second.getY1() == j->second.getY1() ) {
					if( i->second.getX1() == j->second.getX1() + j->second.getWidth() ) {
						i->second.x1 -= j->second.getWidth();
						i->second.x2 -= j->second.getWidth();
						i->second.x2 += j->second.getWidth();
						j = std::prev( mFreeRectangles.erase( j ) );
						merged = true;
					}
					else if( i->second.getX1() + i->second.getWidth() == j->second.getX1() ) {
						i->second.x2 += j->second.getWidth();
						j = std::prev( mFreeRectangles.erase( j ) );
						merged = true;
					}
				}
			}
			//}
		}

		if( ! merged ) {
			break;
		}
	}
}

void TexturePack::debugDraw() const
{
	ci::gl::ScopedColor freeColor( ColorA::gray( 0.4f ) );
	for( const auto &rect : mFreeRectangles ) {
		ci::gl::drawStrokedRect( rect.second );
	}
	ci::gl::ScopedColor usedColor( ColorA::gray( 0.8f ) );
	for( const auto &rect : mUsedRectangles ) {
		ci::gl::drawStrokedRect( rect.second );
	}
}

//////////////

TextureArray::TextureArray( const ci::ivec3 &size )
	: mSize( size )
{
	mTexturePacks.resize( mSize.z, TexturePack( mSize.x, mSize.y ) );

	ci::gl::Texture3d::Format format;
	format.setTarget( GL_TEXTURE_2D_ARRAY );
	format.setInternalFormat( GL_RED );
	mTexture = ci::gl::Texture3d::create( size.x, size.y, size.z, format );
}

TextureArray::Region TextureArray::request( const ci::ivec2 &size, const ci::ivec2 &padding )
{
	int i = 0;
	bool filledPages = false;
	while( !filledPages ) {
		if( i >= mTexturePacks.size() - 1 )
			filledPages = true;

		try {
			pair<uint32_t, Rectf> rect = mTexturePacks[i].insert( size + padding * 2, false );
			return Region( rect.second, i );
		}
		catch( const TexturePackOutOfBoundExc &exc ) {
			i++;
		}
	}
	return Region();
}

TextureArray::Region TextureArray::request( const ci::ivec2 &size, int layerIndex, const ci::ivec2 &padding )
{
	if( size.x == 0 || size.y == 0 )
		return Region( Rectf::zero(), layerIndex );

	try {
		pair<uint32_t, Rectf> rect = mTexturePacks[layerIndex].insert( size + padding * 2, false );
		return Region( rect.second, layerIndex );
	}
	catch( const TexturePackOutOfBoundExc &exc ) {
		return Region();
	}
}

} } } // namespace cinder::text::gl
