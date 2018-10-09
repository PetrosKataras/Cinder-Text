#include "cinder/text/TextureFont.h"
#include "cinder/gl/Texture.h"
#include "cinder/Log.h"
#include "cinder/text/TextLayout.h"

using namespace ci;
using namespace std;
using namespace ci::app;

namespace cinder { namespace text {

void TextureFont::setLayout( const Layout& layout )
{
	mLayout = layout;
	mLayout.setUseLigatures( true );
	allocateFbo();
}

void TextureFont::allocateFbo()
{
	// Allocate
	ci::gl::Fbo::Format fboFormat;
	ci::gl::Texture::Format texFormat;
	texFormat.setMagFilter( GL_NEAREST );
	texFormat.setMinFilter( GL_LINEAR );
	texFormat.enableMipmapping( mFormat.hasMipmapping() );
	fboFormat.setColorTextureFormat( texFormat );

	mFbo = ci::gl::Fbo::create( mFormat.getTextureWidth(), mFormat.getTextureHeight(), fboFormat );
}

void TextureFont::renderLayout( const cinder::text::Layout& layout )
{
	ci::gl::ScopedViewport viewportScope( 0, 0, mFbo->getWidth(), mFbo->getHeight() );
	ci::gl::ScopedMatrices matricesScope;
	ci::gl::setMatricesWindow( mFbo->getSize(), true );

	// Draw text into FBO
	ci::gl::ScopedFramebuffer fboScoped( mFbo );
	ci::gl::clear( ci::ColorA( 0.0, 0.0, 0.0, 0.0 ) );

	mRenderer.render( layout );
}

void TextureFont::drawGlyphs()
{
	mTextures.clear();
	std::string chars = std::string( mSupportedChars );
	
	/*
	// calculate layout
	mLayout.calculateLayout( chars );
	mRenderer.setLayout( mLayout );

	auto lines = mLayout.getLines();
	float lineHeight = mLayout.getFont().getLineHeight();
	float highestPoint = 0;
	int glyphIndex = 0;
	for (auto iter = lines.begin(); iter != lines.end(); ++iter) 
	{
		auto line = *iter;
		auto runs = line.runs;
		auto glyphCount = 0;
		for( auto runIter = runs.begin(); runIter != runs.end(); ++runIter )
		{
			auto run = *runIter;
			auto glyphs = run.glyphs;

			for( auto glyphIter = glyphs.begin(); glyphIter != glyphs.end(); ++glyphIter )
			{
				highestPoint = glm::max( (*glyphIter).bbox.y2, highestPoint );
				glyphCount++;
				
			}
		}
		if( highestPoint < mFormat.getTextureHeight() ) {
			glyphIndex += glyphCount;
		}
	}

	if( mLayout.getMaxLinesReached() ) {
		CI_LOG_I( "time for a new texture: " << glyphIndex );
		auto newText = chars.substr(glyphIndex, chars.size() );
		CI_LOG_I( "new text: " << newText );
	}

	if (highestPoint > mFormat.getTextureHeight()) {
		
	}
	mTextures.push_back( mRenderer.getTexture() );
	*/
	

	

	bool finished = false;
	while (!finished) {
		
		text::gl::TextureRenderer renderer;

		// calculate layout
		mLayout.calculateLayout( chars );
		renderLayout( mLayout );
		auto lines = mLayout.getLines();

		int curTextureIndex = mTextures.size();
		mTextures.push_back( mFbo->getColorTexture() );

		float lineHeight = mLayout.getFont().getLineHeight();
		float highestPoint = 0;
		int glyphIndex = 0;
		int stopGlyphIndex = 0;
		string stopGlyph;

		auto glyphCount = mLayout.getGlyphMap().size();
		/*
		auto glyphs = mLayout.getGlyphMap();
		for( auto glyphIter = glyphs.begin(); glyphIter != glyphs.end(); ++glyphIter )
		{
			auto glyph = (*glyphIter);
			highestPoint = glm::max( glyph.bbox.y2, highestPoint );

			if( highestPoint > mFormat.getTextureHeight() && stopGlyphIndex == 0 ) {
				stopGlyphIndex = glyphIndex - 1;
				CI_LOG_V( glyph.value );
				stopGlyph = glyph.value;
			}

			GlyphInfo glyphInfo;
			glyphInfo.glyph = glyph;
			glyphInfo.index = glyph.index;
			glyphInfo.textureIndex = curTextureIndex;
			mGlyphMap[glyph.index] = glyphInfo; 

			glyphIndex++;
		}
		*/
		
		for (auto iter = lines.begin(); iter != lines.end(); ++iter) 
		{
			auto line = *iter;
			auto runs = line.runs;
			auto glyphCount = 0;
			for( auto runIter = runs.begin(); runIter != runs.end(); ++runIter )
			{
				auto run = *runIter;
				auto glyphs = run.glyphs;

				for( auto glyphIter = glyphs.begin(); glyphIter != glyphs.end(); ++glyphIter )
				{
					/*highestPoint = glm::max( (*glyphIter).bbox.y2, highestPoint );
					glyphCount++;*/


					auto glyph = (*glyphIter);
					highestPoint = glm::max( glyph.bbox.y2, highestPoint );

					if( highestPoint > mFormat.getTextureHeight() && stopGlyphIndex == 0 && glyph.index != 3 ) {
						stopGlyphIndex = glyphIndex - 1;
						CI_LOG_V( glyph.value );
						stopGlyph = glyph.value;
					}

					GlyphInfo glyphInfo;
					glyphInfo.glyph = glyph;
					glyphInfo.index = glyph.index;
					glyphInfo.textureIndex = curTextureIndex;
					mGlyphMap[glyph.index] = glyphInfo; 

					glyphIndex++;
				
				}
			}
			/*if( highestPoint < mFormat.getTextureHeight() ) {
				glyphIndex += glyphCount;
			}*/
		}


		
		CI_LOG_V( "char size: " << glyphCount );
		if( stopGlyphIndex <= 0 && glyphIndex >= glyphCount ) {
			finished = true;
		}
		else {
			size_t index = chars.find_first_of( stopGlyph );
			chars.erase( 0, index );
		}
		
		CI_LOG_V( "new text: " << chars );
	}


	//mLayout.getLines();

	// if layout is too tall, create new layout starting at last character

	// 

	//calculateLayout( chars );

	//mLines.size();

	/*
	int index = 0;
	//float fontSize = mFontSize;
	//float leading = mLeading;
	//bool formatted = mIsFormatted;
	//std::string text = mText;

	gl::Texture::Format textureFormat = gl::Texture::Format();
	textureFormat.enableMipmapping( mFormat.hasMipmapping() );


	bool finished = false;
	while (!finished) {
		
		CI_LOG_V( "set text: " << chars );
		render( chars, false );

		int curTextureIndex = mTextures.size();
		mTextures.push_back( ci::gl::Texture::create( *getSurface(), textureFormat ) );
		std::string renderedText = pango_layout_get_text( pPangoLayout );
		
		// get index character at bottom left of surface
		int blIndex; int brIndex; int trailing;
		pango_layout_xy_to_index( pPangoLayout, PANGO_SCALE, TO_PANGO_UNITS( mCanvasSize.y ), &blIndex, &trailing );
		pango_layout_xy_to_index( pPangoLayout, TO_PANGO_UNITS( mCanvasSize.x ), TO_PANGO_UNITS( mCanvasSize.y ), &brIndex, &trailing );

		PangoRectangle rect;
		pango_layout_index_to_pos( pPangoLayout, brIndex, &rect );
		
	
		PangoLanguage *lang = pango_language_from_string( string("english").c_str() );
		PangoFontMetrics *fontMetrics = pango_font_get_metrics( pFont, lang );

		int descent = pango_font_metrics_get_descent( fontMetrics );
		{
			PangoLayoutIter* iter = pango_layout_get_iter( pPangoLayout );
	
			bool iterHasNext = true;
			while (iterHasNext) {
				PangoLayoutRun* run = (PangoLayoutRun*)pango_layout_iter_get_run_readonly( iter );
				int index = pango_layout_iter_get_index( iter );
				int baseline = PANGO_PIXELS( pango_layout_iter_get_baseline( iter ) );// / PANGO_SCALE;

				if( run )
				{
					PangoGlyphString* str = (PangoGlyphString*)run->glyphs;
					for (int i = 0; i < str->num_glyphs; i++)
					{
						PangoGlyphInfo info = str->glyphs[i];
						PangoRectangle inkRect, logRect;
						pango_font_get_glyph_extents( pFont, info.glyph, &inkRect, &logRect );

						PangoRectangle glyphRect;
						pango_layout_index_to_pos( pPangoLayout, index+i, &glyphRect );
						glyphRect.x += info.geometry.x_offset;		// x offset comes from letterspacing attribute

						pango_extents_to_pixels( NULL, &inkRect );
						pango_extents_to_pixels( NULL, &logRect );
						pango_extents_to_pixels( NULL, &glyphRect );

						vec2 textureSize =  vec2( mFormat.getTextureWidth(), mFormat.getTextureHeight() );

						vec2 inkOffset = vec2( float( glyphRect.x + inkRect.x ), float( baseline + inkRect.y ) ) / textureSize;
						vec2 inkSize = vec2( float( inkRect.width ), float( inkRect.height ) ) / textureSize;
						Rectf inkArea = Rectf( inkOffset, inkOffset + inkSize );

						vec2 logOffset = vec2( float( glyphRect.x + logRect.x ),  float( baseline + logRect.y ) ) / textureSize;
						vec2 logSize = vec2( float( logRect.width),  float( logRect.height ) ) / textureSize;
						Rectf logArea = Rectf( logOffset, logOffset + logSize );

						float x = glyphRect.x + logRect.x;
						float y = baseline + logRect.y;
						float w = logRect.width;
						float h = logRect.height;
						
						//CI_LOG_I( "run glyph:     " << index + i << " " << info.glyph );
						//CI_LOG_I( "glyph extents: " << x << " " << y << " " << w << " " << h );
						//CI_LOG_I( chars[index + i] );

						vec2 curOffset = vec2( x, y );
						GlyphInfo newInfo;
						newInfo.mTextureIndex = curTextureIndex;
						newInfo.mExtents = Rectf( logArea.scaled( textureSize ) );
						newInfo.mInkExtents = Rectf( inkArea.scaled( textureSize ) );
						newInfo.mTexCoords = Rectf( logArea.x1, 1.0f - logArea.y1, logArea.x2, 1.0f - logArea.y2 );
						newInfo.mInkTexCoords = Rectf( inkArea.x1, 1.0f - inkArea.y1, inkArea.x2, 1.0f - inkArea.y2 );
						
						float glyphBaseline = 1.0f - (baseline / textureSize.y) - newInfo.mTexCoords.y1;
						float ascent = (PANGO_ASCENT( inkRect ) / textureSize.y);
						float descent =  (PANGO_DESCENT( inkRect ) / textureSize.y);
						
						newInfo.mBaseline = glyphBaseline;	// baseline normalized relative to the glyph size (log tex coords)
						newInfo.mAscent = glyphBaseline + ascent;
						newInfo.mDescent = glyphBaseline - descent;
						newInfo.mBearingLeft = inkArea.x1 - logArea.x1;
						newInfo.mScaler = textureSize;
						mGlyphMap[info.glyph] = newInfo;
					}	
				}

				iterHasNext = pango_layout_iter_next_run( iter );
					if( !iterHasNext )
						break;
			}

			pango_layout_iter_free( iter );
		}
		
		bool glyphBottomInRange = ( (rect.y / PANGO_SCALE) + (rect.height / PANGO_SCALE) ) <  mCanvasSize.y;

		if( glyphBottomInRange ) {
			finished = true;
		}
		else {
			chars.erase( 0, blIndex );
		}
		
		CI_LOG_V( "new text: " << chars );
	}*/
}

} } // namespace cinder::text