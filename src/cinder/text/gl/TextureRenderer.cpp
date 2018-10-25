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

bool							TextureRenderer::mSharedCacheEnabled = false;
TextureRenderer::TexArrayCache	TextureRenderer::mSharedTexArrayCache;
TextureArray::Format			TextureRenderer::mTextureArrayFormat;


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
	vec3 coord = vec3((texCoord.x * uSubTexSize.x) + uSubTexOffset.x, ((1.0 - texCoord.y) * uSubTexSize.y) + uSubTexOffset.y, uLayer);
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
{
	ci::gl::GlslProgRef shader = ci::gl::GlslProg::create( vertShader, fragShader );
	shader->uniform( "uTexArray", 0 );

	if( mBatch == nullptr ) {
		mBatch = ci::gl::Batch::create( ci::geom::Rect( ci::Rectf( 0.f, 0.f, 1.f, 1.f ) ), shader );
	}
}

void TextureRenderer::render( const cinder::text::Layout& layout )
{
	render( layout.getLines() );
}

void TextureRenderer::render( const std::vector<cinder::text::Layout::Line>& lines )
{
	for( auto& line : lines ) 
	{
		for( auto& run : line.runs ) {
			ci::gl::ScopedGlslProg scopedShader( ci::gl::getStockShader( ci::gl::ShaderDef().color() ) );
			ci::gl::ScopedColor( ci::ColorA( run.color, run.opacity ) );

			for( auto& glyph : run.glyphs ) 
			{	
				// Make sure we have the glyph
				if( TextureRenderer::getCacheForFont( run.font ).glyphs.count( glyph.index ) != 0 ) {
					ci::gl::ScopedMatrices matrices;
					ci::gl::translate( ci::vec2( glyph.bbox.getUpperLeft() ) );
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

void TextureRenderer::render( const TextureRenderer::RenderLineCache &line )
{
	ci::gl::ScopedGlslProg scopedShader( ci::gl::getStockShader( ci::gl::ShaderDef().color() ) );
		
	for( auto& glyph : line.glyphCache) 
	{
		ci::gl::ScopedMatrices matrices;
		ci::gl::translate( ci::vec2( glyph.rect.getUpperLeft() ) );
		ci::gl::scale( glyph.rect.getSize() );
		auto tex = glyph.texture;
		mBatch->getGlslProg()->uniform( "uLayer", glyph.layer );
		mBatch->getGlslProg()->uniform( "uSubTexSize", glyph.texSize );
		mBatch->getGlslProg()->uniform( "uSubTexOffset", glyph.texOffset );
		ci::gl::ScopedTextureBind texBind( tex->getTarget(), tex->getId() );
		mBatch->draw();
	}
}

TextureRenderer::RenderLineCache TextureRenderer::cacheLine( const cinder::text::Layout::Line &line )
{
	std::vector<RenderGlyphCacheStuff> glyphCaches;
	vec2 br = vec2();
	vec2 ul = vec2( FLT_MAX );
	for( auto& run : line.runs ) 
	{
		for( auto& glyph : run.glyphs ) 
		{	
			// Make sure we have the glyph
			if( TextureRenderer::getCacheForFont( run.font ).glyphs.count( glyph.index ) != 0 ) 
			{
				vec2 pos =  ci::vec2( glyph.position + glyph.offset);
				vec2 size =  glyph.size;
				glm::mat3 mtrx = glm::translate( mat3(), ci::vec2( glyph.position + glyph.offset) );
				mtrx = glm::scale( mtrx, glyph.size );
			
				auto fontCache = getCacheForFont( run.font );
				auto glyphCache = fontCache.glyphs[glyph.index];
				auto tex = fontCache.texArrayCache.texArray->getTexture();

				RenderGlyphCacheStuff gc;
				gc.texture = tex;
				gc.layer = (uint32_t)glyphCache.layer;
				gc.texSize = glyphCache.subTexSize;
				gc.texOffset = glyphCache.subTexOffset;
				gc.mtrx = mtrx;
				gc.rect = Rectf( pos, pos + size );
				gc.color = ci::ColorA( run.color, run.opacity );
				glyphCaches.push_back( gc );
				
				br.x = glm::max( br.x, glyph.bbox.x2 );
				br.y = glm::max( br.y, glyph.bbox.y2 );
				ul.x = glm::min( ul.x, glyph.bbox.x1 );
				ul.y = glm::min( ul.y, glyph.bbox.y1 );
			}
			else {
				//ci::app::console() << "Could not find glyph for index: " << glyph.index << std::endl;
			}
		}
	}
	RenderLineCache lineCache;
	lineCache.glyphCache = glyphCaches;
	lineCache.bounds = Rectf( ul, br );
	return lineCache;
}

std::vector<std::pair<uint32_t, ci::ivec2>> TextureRenderer::getGlyphMapForLayout( const cinder::text::Layout& layout )
{
	auto lines = layout.getLines();
	std::vector<std::pair<uint32_t, ivec2>> map;
	for( auto& line : lines ) 
	{
		for( auto& run : line.runs ) 
		{
			for( auto& glyph : run.glyphs ) 
			{	
				// Make sure we have the glyph
				if( TextureRenderer::getCacheForFont( run.font ).glyphs.count( glyph.index ) != 0 ) {
					auto fontCache = getCacheForFont( run.font );
					
					int glyphIndex = cinder::text::FontManager::get()->getGlyphIndex( run.font, glyph.index, 0 );
					auto glyphCache = fontCache.glyphs[glyph.index];
					map.push_back( {glyph.index, glyph.position } );
				}
			}
		}
	}
	
	return map;
}

void TextureRenderer::loadFont( const Font& font, bool loadEntireFont )
{
	if( TextureRenderer::fontCache.count( font ) == 0 ) {
		TextureRenderer::cacheFont( font, loadEntireFont );
		CI_LOG_V( "Font loaded: \n" << font );
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

std::map<uint16_t, TextureRenderer::GlyphCache> TextureRenderer::getGylphMapForFont( const Font& font )
{
	return getCacheForFont( font ).glyphs;
}

ci::gl::Texture3dRef TextureRenderer::getTextureForFont( const Font& font )
{
	return getCacheForFont( font ).texArrayCache.texArray->getTexture();
}

// Cache glyphs to gl texture array(s)
void TextureRenderer::cacheFont( const Font& font,  bool cacheEntireFont )
{
	if( cacheEntireFont ) {
		// entire font
		std::vector<uint32_t> glyphIndices = cinder::text::FontManager::get()->getGlyphIndices( font );
		cacheGlyphs( font, glyphIndices );
	}
	else {
		// partial font
		cacheGlyphs( font, defaultUnicodeRange() );
	}
}

void TextureRenderer::uncacheFont( const Font& font )
{
	if( mSharedCacheEnabled ) {
		CI_LOG_W( "Cannot un-cache fonts loaded into shared caches" );
		return;
	}

	auto count = TextureRenderer::fontCache.count( font );
	if( count ) {
		std::unordered_map<Font, FontCache>::iterator it = TextureRenderer::fontCache.find( font );
		TextureRenderer::fontCache.erase( it );
	}
}

TextureArrayRef TextureRenderer::makeTextureArray()
{
	auto texArray = TextureArray::create( mTextureArrayFormat );
	return texArray;
}

void TextureRenderer::cacheGlyphs( const Font& font, const std::string string, const std::string language, Script script, Direction dir )
{
	// Shape the substring
	Shaper shaper( font );
	
	std::vector<uint32_t> glyphIndices;
	std::vector<Shaper::Glyph> shapedGlyphs = shaper.getShapedText( Shaper::Text( { string, language, script, dir } ) );
	for( auto glyph : shapedGlyphs ) {
		glyphIndices.push_back( glyph.index );
	}

	//std::vector<uint32_t> glyphIndices = cinder::text::FontManager::get()->getGlyphIndices( font, string );
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
	
	if( texArrayCache->filled ) {
		CI_LOG_E( "Texture Array is filled. Cannot cache any more gyphs. Increase the size or depth or the Texture Array." );
		return;
	}

    for( auto glyphIndex : glyphIndices ) 
	{
		auto glyphCache = TextureRenderer::fontCache[font].glyphs[glyphIndex];
		if( glyphCache.layer > -1 || texArrayCache->filled ) {
			continue;
		}

        dirty = true;

		FT_BitmapGlyph bitmapGlyph = cinder::text::FontManager::get()->getGlyphBitmap( font, glyphIndex );
		FT_Glyph glyph = cinder::text::FontManager::get()->getGlyph( font, glyphIndex );
		FT_BBox bbox;
		FT_Glyph_Get_CBox( glyph, FT_GLYPH_BBOX_PIXELS, &bbox );

		ci::ivec2 glyphSize( bitmapGlyph->bitmap.width, bitmapGlyph->bitmap.rows );
		if( glyphSize.x < 0 || glyphSize.y < 0 )
		{
			CI_LOG_W( "Problem with size of retreived glyph bitmap for glyph " << glyphIndex );
			continue;
		}
		ci::ivec2 padding = ci::ivec2( 4 ) - ( glyphSize % ci::ivec2( 4 ) );
		auto region = textureArray->request( glyphSize, layerIndex, ivec2( 2.0f ) );
	
		// if current layer is full, upload channel to texture, increment layer id, reset channel
		if( region.layer < 0 ) {
			
			uploadChannelToTexture( *texArrayCache );

			// clear channel
			ci::ip::fill( layerChannel.get(), ( uint8_t )0 );
			dirty = false;

			if( layerIndex + 1 >= textureArray->getDepth() ) {
				texArrayCache->filled = true;
				CI_LOG_E( "Texture Array is filled. Cannot cache any more gyphs. Increase the size or depth or the Texture Array." );
			} else {
				layerIndex++;
				texArrayCache->currentLayerIdx = layerIndex;

				// request a new region
				region = textureArray->request( glyphSize, layerIndex, ivec2( 2.0f ) );
			}
			
		}

		if( region.layer > -1 )
		{ 
			ivec2 offset = region.rect.getUpperLeft();
			auto layer = region.layer;

			// fill channel
			ci::ChannelRef channel = ci::Channel::create( glyphSize.x, glyphSize.y, glyphSize.x * sizeof( unsigned char ), sizeof( unsigned char ), bitmapGlyph->bitmap.buffer );
			if( channel->getData() ) {
				//ci::ChannelRef flippedChannel = ci::Channel::create( glyphSize.x, glyphSize.y );
				//ci::ip::fill( flippedChannel.get(), ( uint8_t )0 );
				//ci::ip::flipVertical( *channel, flippedChannel.get() );
				ci::Channel8uRef expandedChannel = ci::Channel8u::create( glyphSize.x + padding.x, glyphSize.y + padding.y );
				ci::ip::fill( expandedChannel.get(), ( uint8_t )0 );
				expandedChannel->copyFrom( *channel, ci::Area( 0, 0, glyphSize.x, glyphSize.y ) );

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
			TextureRenderer::fontCache[font].glyphs[glyphIndex].offset = ci::vec2( bbox.xMin, bbox.yMax );
			TextureRenderer::fontCache[font].glyphs[glyphIndex].subTexOffset = ci::vec2( offset ) / ci::vec2( textureArray->getSize() );
			TextureRenderer::fontCache[font].glyphs[glyphIndex].subTexSize = ci::vec2( glyphSize ) / ci::vec2( textureArray->getSize() );
		} 
		else 
		{
			CI_LOG_W( "No valid region can be found in the atlas." );
		}
    }

    // we need to reflect any characters we haven't uploaded
    if( dirty )
        uploadChannelToTexture( *texArrayCache );
}


void TextureRenderer::uploadChannelToTexture( TexArrayCache &texArrayCache )
{
	// upload channel to texture
	auto texArray = texArrayCache.texArray;
	auto channel = texArrayCache.layerChannel;
	int layerIdx = texArrayCache.currentLayerIdx;
	texArray->update( channel, layerIdx );
}


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

TextureArray::TextureArray( const TextureArray::Format fmt )
	: mSize( fmt.mSize ), mFormat( fmt )
{
	mTexturePacks.resize( mSize.z, TexturePack( mSize.x, mSize.y ) );

	bool mipmap = fmt.mMipmapping;
	auto format = ci::gl::Texture3d::Format()
		.target( GL_TEXTURE_2D_ARRAY )
		.internalFormat( fmt.mInternalFormat )
		.maxAnisotropy( fmt.mMaxAnisotropy )
		.mipmap( mipmap ).magFilter( mipmap ? GL_LINEAR : GL_NEAREST ).minFilter( mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST );

	mTexture = ci::gl::Texture3d::create( mSize.x, mSize.y, mSize.z, format );
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
	else if(  size.x > mSize.x || size.y > mSize.y )
		return Region();

	try {
		pair<uint32_t, Rectf> rect = mTexturePacks[layerIndex].insert( size + padding * 2, false );
		return Region( rect.second, layerIndex );
	}
	catch( const TexturePackOutOfBoundExc &exc ) {
		return Region();
	}
}

void TextureArray::update( ci::ChannelRef channel, int layerIdx )
{
	// upload channel to texture
	ci::Surface8u surface( *channel );
	int mipLevel = 0;
	GLint dataFormat;
	GLenum dataType;
	ci::gl::TextureBase::SurfaceChannelOrderToDataFormatAndType<uint8_t>( surface.getChannelOrder(), &dataFormat, &dataType );
	mTexture->update(  (void*)surface.getData(), dataFormat, dataType, mipLevel, surface.getWidth(), surface.getHeight(), 1, 0, 0, layerIdx );

	if( mFormat.mMipmapping )
	{
		ci::gl::ScopedTextureBind scopedTexBind( mTexture );
		glGenerateMipmap( mTexture->getTarget() );		
	}
}


} } } // namespace cinder::text::gl
