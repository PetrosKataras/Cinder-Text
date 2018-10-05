#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "txt/TextBox.h"
#include "txt/FontTexture.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class TextureFontApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;

	txt::Layout mLayout;
	txt::gl::TextureRenderer mRenderer;
	ci::Rectf mTextBox = ci::Rectf( vec2(), vec2( 1024.f ) );
	txt::AttributedString mAttrStr;

	vector<gl::TextureRef>	mFontTextures;
	gl::BatchRef			mRectBatch;
	//std::unordered_map<ci::Font::Glyph, GlyphInfo>	mGlyphMap;
};

void TextureFontApp::setup()
{
	//std::string	chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnooppqrstuvwxyz1234567890().?!,:;'\"&*=+-/\\@#_[]<>%^ftllflfiphrids\xC3\xA0\303\221\xEF\xAC\x81\xEF\xAC\x82";
	//std::string	chars = "A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h i j k l m n o p p q r s t u v w x y z 1 2 3 4 5 6 7 8 9 0";
	//mAttrStr << chars << txt::AttributeFontFamily( "Arial" ) << txt::AttributeFontSize( 36.f );

	//mLayout.setFont( txt::Font( "Arial", 36 ) );
	//mLayout.setSize( mTextBox.getSize() );
	//mLayout.calculateLayout( mAttrStr );
	//mRenderer.setLayout( mLayout );


	float mTracking = 0.f;
	float mLineHeight = 1.2;
	ivec2 layoutSize = ivec2( 1024 );
	
	auto font = std::make_shared<txt::Font>( "Arial", 15.0f * 8.0f );
	txt::gl::TextureRenderer::loadFont( *font );

	mLayout.setFont( *font );
	mLayout.setSize( layoutSize );
	mLayout.setTracking( mTracking );
	//mLayout.setLanguage( mLanguage );
	//mLayout.setScript( mScript );
	//mLayout.setDirection( mDirection );
	//mLayout.setLineHeight( mLineHeight );
	mLayout.setLineHeight( txt::Unit( mLineHeight, txt::EM ) );
	//mLayout.calculateLayout( mTestText );

	//mRenderer.setLayout( mLayout );
	



	bool mipmap = true;
	auto fontTexture = txt::FontTexture::create( txt::FontTexture::Format().enableMipmapping( mipmap ).textureWidth( layoutSize.x ).textureHeight( layoutSize.y ) );
	//fontTexture->setFont( *font );
	//fontTexture->setTracking( mTracking );
	//fontTexture->setLineHeight( txt::Unit( mLineHeight, txt::EM ) );
	fontTexture->setLayout( mLayout );
	fontTexture->drawGlyphs();
	
	mFontTextures = fontTexture->getTextures();


	mRectBatch = gl::Batch::create( geom::Rect( Rectf( vec2(), vec2( layoutSize ) ) ).texCoords( vec2( 0, 0 ) , vec2( 1, 0), vec2( 1, 1), vec2( 0, 1) ) , gl::getStockShader( gl::ShaderDef().texture() ) );

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
		
		// flip to draw
		gl::translate( vec2( 0, tex->getHeight() ) );
		gl::rotate( M_PI, vec3( 1, 0, 0 ) );

		gl::ScopedTextureBind scpTex( mFontTextures[i] );
		//gl::ScopedGlslProg scpGlsl( gl::getStockShader( gl::ShaderDef().texture() ) );
		//gl::drawSolidRect( drawingArea );
		
		mRectBatch->draw();
	}
}

CINDER_APP( TextureFontApp, RendererGl )
