#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/text/TextBox.h"
#include "cinder/text/TextLayout.h"
#include "cinder/text/TextureFont.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class TextureFontApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;

	text::Layout mLayout;
	text::gl::TextureRenderer mRenderer;
	ci::Rectf mTextBox = ci::Rectf( vec2(), vec2( 1024.f ) );
	text::AttributedString mAttrStr;

	vector<gl::TextureRef>	mFontTextures;
	gl::BatchRef			mRectBatch;
	std::unordered_map<int16_t, text::TextureFont::GlyphInfo>		mGlyphMap;
	//std::unordered_map<ci::Font::Glyph, GlyphInfo>	mGlyphMap;
};

void TextureFontApp::setup()
{
	//std::string	chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnooppqrstuvwxyz1234567890().?!,:;'\"&*=+-/\\@#_[]<>%^ftllflfiphrids\xC3\xA0\303\221\xEF\xAC\x81\xEF\xAC\x82";
	//std::string	chars = "A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h i j k l m n o p p q r s t u v w x y z 1 2 3 4 5 6 7 8 9 0";
	//mAttrStr << chars << text::AttributeFontFamily( "Arial" ) << text::AttributeFontSize( 36.f );

	//mLayout.setFont( text::Font( "Arial", 36 ) );
	//mLayout.setSize( mTextBox.getSize() );
	//mLayout.calculateLayout( mAttrStr );
	//mRenderer.setLayout( mLayout );


	float mTracking = 0.f;
	float mLineHeight = 1.0;
	float mLeading = 0.0;
	ivec2 layoutSize = ivec2( 1024 );
	float superSampling = 8.0f;
	float fontSize = 24.0f * superSampling;

	//auto font = std::make_shared<text::Font>( "Arial", fontSize );
	//auto font = std::make_shared<text::Font>( ci::app::loadAsset( "fonts/SourceSansPro/SourceSansPro-Regular.otf" ), fontSize );
	auto font = std::make_shared<text::Font>( ci::app::loadAsset( "fonts/Noto_Serif/NotoSerif-Regular.ttf" ), fontSize );
	text::gl::TextureRenderer::loadFont( *font );

	mLayout.setFont( *font );
	mLayout.setSize( layoutSize );
	mLayout.setTracking( mTracking );
	//mLayout.setLanguage( mLanguage );
	//mLayout.setScript( mScript );
	//mLayout.setDirection( mDirection );
	//mLayout.setLineHeight( mLineHeight );
	//mLayout.setLineHeight( text::Unit( mLineHeight, text::EM ) );
	//mLayout.calculateLayout( mTestText );

	//mRenderer.setLayout( mLayout );
	



	bool mipmap = true;
	//std::string chars = "ABCabc";
	auto fontTexture = text::TextureFont::create( text::TextureFont::Format().enableMipmapping( mipmap ).textureWidth( layoutSize.x ).textureHeight( layoutSize.y ) );
	//fontTexture->setFont( *font );
	//fontTexture->setTracking( mTracking );
	//fontTexture->setLineHeight( text::Unit( mLineHeight, text::EM ) );
	fontTexture->setLayout( mLayout );
	fontTexture->drawGlyphs();
	
	mFontTextures = fontTexture->getTextures();
	mGlyphMap = fontTexture->getGlyphMap();

	//mRectBatch = gl::Batch::create( geom::Rect( Rectf( vec2(), vec2( layoutSize ) ) ).texCoords( vec2( 0, 0 ) , vec2( 1, 0), vec2( 1, 1), vec2( 0, 1) ) , gl::getStockShader( gl::ShaderDef().texture() ) );
	mRectBatch = gl::Batch::create( geom::Rect( Rectf( vec2(), vec2( 1024.0f ) ) ).texCoords( vec2( 0, 1 ) , vec2( 1, 1), vec2( 1, 0), vec2( 0, 0) ) , gl::getStockShader( gl::ShaderDef().texture() ) );
	//mFontTextures = fontTexture->getTextures();
	//mGlyphMap = fontTexture->getGlyphMap();
}

void TextureFontApp::mouseDown( MouseEvent event )
{
}

void TextureFontApp::update()
{
}

void TextureFontApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
	ci::gl::ScopedMatrices matrices;
	
	gl::color( 1.0, 0.0, 0.0);
	//mTextBox.draw();
	mRenderer.draw();


	for( int i = 0; i < mFontTextures.size(); i++ ) {
		auto tex = mFontTextures[i];
		float w = float( app::getWindowWidth() ) / mFontTextures.size();
		auto drawingArea = Area::proportionalFit( Area( mFontTextures[i]->getBounds() ), Area( Rectf( w * float(i), 0.0f, w*float(i) + w, float(getWindowHeight()) ) ), true );
			
		gl::ScopedMatrices scpMtrx;
		gl::translate( drawingArea.getUL() );
		gl::scale( vec2( float(drawingArea.getWidth()) / tex->getHeight() ) );
		
		

		gl::ScopedTextureBind scpTex( mFontTextures[i] );
		//gl::ScopedGlslProg scpGlsl( gl::getStockShader( gl::ShaderDef().texture() ) );
		//gl::drawSolidRect( drawingArea );
		
		mRectBatch->draw();


		// flip to draw
		//gl::translate( vec2( 0, tex->getHeight() ) );
		//gl::rotate( M_PI, vec3( 1, 0, 0 ) );

		// draw glyph areas
		for( auto iter = mGlyphMap.begin(); iter != mGlyphMap.end(); ++iter ){
			
			int textureIndex = (*iter).second.textureIndex;
			if (textureIndex == i) {
				gl::ScopedColor scpColor;
								
				auto glyphInfo = (*iter).second;

				{
					//auto rect = Rectf( glyphInfo.glyph.bbox );
					auto rect = Rectf( glyphInfo.glyph.extents );

					// full glyph boundry
					gl::color( ColorA( 1.0, 1.0, 0.0, 0.25 ) );
					gl::drawSolidRect( rect );
					gl::color( ColorA( 0.0, 0.0, 0.0, 0.25 ) );
					gl::drawStrokedRect( rect );
				}
			}

				/*// draw logical rect
				{
					auto rect = Rectf( glyphInfo.mTexCoords );
					rect.scale( glyphInfo.mScaler );
						
					// full glyph boundry
					gl::color( ColorA( 1.0, 1.0, 0.0, 0.25 ) );
					gl::drawSolidRect( rect );
					gl::color( ColorA( 0.0, 0.0, 0.0, 0.25 ) );
					gl::drawStrokedRect( rect );
					
					// baseline
					gl::color( ColorA( 0.0, 0.0, 1.0, 1.0 ) );
					float baseline = rect.y1 + (glyphInfo.mBaseline * glyphInfo.mScaler.y);
					gl::drawLine( vec2( rect.x1, baseline ), vec2( rect.x2, baseline ) );
					
					// ascent
					float ascent = rect.y1 + (glyphInfo.mAscent * glyphInfo.mScaler.y);
					gl::drawLine( vec2( rect.x1, ascent ), vec2( rect.x2, ascent ) );
					
					// descent
					float descent = rect.y1 + (glyphInfo.mDescent * glyphInfo.mScaler.y);
					gl::drawLine( vec2( rect.x1, descent ), vec2( rect.x2,descent ) );
				}

				// draw ink rect
				{
					auto rect = Rectf( (*iter).second.mInkTexCoords );
					rect.scale( glyphInfo.mScaler );
					
					gl::color( ColorA( 1.0, 0.0, 0.0, 0.25 ) );
					gl::drawSolidRect( rect );
					gl::color(ColorA( 0.0, 0.0, 0.0, 0.25) );
					gl::drawStrokedRect( rect );

					gl::color( ColorA( 0.0, 1.0, 0.0, 1.0 ) );
				}*/

				
			}
	}
}

CINDER_APP( TextureFontApp, RendererGl )
