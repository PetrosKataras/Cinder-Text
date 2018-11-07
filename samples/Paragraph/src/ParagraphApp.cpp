#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/FileWatcher.h"
#include "cinder/Utilities.h"

#include "cinder/text/FontManager.h"
#include "cinder/text/gl/TextureRenderer.h"
#include "cinder/text/TextLayout.h"

#include "cinder/Unicode.h"

#include <string>
#include <iostream>

using namespace ci;
using namespace ci::app;
using namespace std;

class CinderProjectApp : public App
{
	public:
		CinderProjectApp();

		void setup() override;
		void keyDown( KeyEvent event ) override;
		void update() override;
		void draw() override;

		void updateLayout();
		void textFileUpdated( const ci::WatchEvent& event );

		std::shared_ptr<text::Font> mFont;

		text::gl::TextureRenderer mRenderer;
		text::Layout mLayout;

		int mFontSize = 24.f;
		float mTracking = 0.f;

		float mLineHeight = 1.2;

		ci::vec2 mTextBoxPos = ci::vec2( 200.f, 200.f );
		ci::vec2 mTextBoxSize = ci::vec2( 600.f, 600.f );
		//ci::Rectf mTextBox = ci::Rectf( 100.f, 100.f, 800.f, 800.f );

		// English
		std::string fontName = "fonts/notoserif/notoserif-regular.ttf";
		std::string testTextFilename = "text/english.txt";
		std::string mLanguage = "en";
		ci::text::Script mScript = text::Script::LATIN;
		ci::text::Direction mDirection = text::Direction::LTR;

		// Arabic
		//std::string fontName = "fonts/NotoArabic/NotoSansArabic-Regular.ttf";
		//std::string testTextFilename = "text/arabic.txt";
		//std::string mLanguage = "ar";
		//hb_script_t mScript = HB_SCRIPT_ARABIC;
		//hb_direction_t mDirection = HB_DIRECTION_RTL;

		// Simplified Chinese
		//std::string fontName = "fonts/NotoChinese/NotoSansCJKsc-Regular.otf";
		//std::string testTextFilename = "text/simplifiedChinese.txt";
		//std::string mLanguage = "zh-Hans";
		//hb_script_t mScript = HB_SCRIPT_HAN;
		//hb_direction_t mDirection = HB_DIRECTION_LTR;

		// Cyrillic
		//std::string fontName = "fonts/SourceSerifPro/SourceSerifPro-Regular.otf";
		//std::string testTextFilename = "text/cyrillic.txt";
		//std::string mLanguage = "ru";
		//hb_script_t mScript = HB_SCRIPT_CYRILLIC;
		//hb_direction_t mDirection = HB_DIRECTION_LTR;

		std::string mTestText;
		ci::gl::FboRef mFbo;
};

CinderProjectApp::CinderProjectApp() {}

void CinderProjectApp::setup()
{
	setWindowSize( 1024.f, 768.f );

	ci::gl::Fbo::Format fboFormat;
	ci::gl::Texture::Format texFormat;
	texFormat.setMagFilter( GL_NEAREST );
	texFormat.setMinFilter( GL_LINEAR );
	texFormat.enableMipmapping( true );
	fboFormat.setSamples( 8 );
	fboFormat.setColorTextureFormat( texFormat );
 	mFbo = ci::gl::Fbo::create( mTextBoxSize.x, mTextBoxSize.y, fboFormat );

	ci::text::gl::TextureArray::Format fmt = text::gl::TextureArray::Format()
		.size( ivec3( 1024, 1024, 4) );
	ci::text::gl::TextureRenderer::setTextureFormat( fmt );
	
	mFont = std::make_shared<text::Font>( ci::app::loadAsset( fontName ), mFontSize );
	//text::gl::TextureRenderer::loadFont( *mFont, true );	// true to load the entire font (as oppose to a predefined subset - for Latin based languages)
	text::gl::TextureRenderer::loadFont( *mFont );
	
	
	//text::gl::TextureRenderer::loadFont( *mFont ); // Testing caching, should be nearly a no-op
	//mLineHeight = mFont->getLineHeight();

	ci::FileWatcher::instance().watch( ci::app::getAssetPath( testTextFilename ), std::bind( &CinderProjectApp::textFileUpdated, this, std::placeholders::_1 ) );
}

void CinderProjectApp::update()
{
}

void CinderProjectApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
	ci::gl::enableAlphaBlending();
	ci::gl::ScopedMatrices matrices;
	ci::gl::translate( mTextBoxPos );

	ci::gl::color( 0.25, 0.25, 0.25 );
	ci::gl::drawStrokedRect( ci::Rectf( ci::vec2( 0.f ), mLayout.measure() ) );

	ci::gl::color( 1.f, 1, 1 );
	
	{
		ci::gl::ScopedViewport viewportScope( 0, 0, mFbo->getWidth(), mFbo->getHeight() );
		ci::gl::ScopedMatrices matricesScope;
		ci::gl::setMatricesWindow( mFbo->getSize(), true );
 		// Draw text into FBO
		ci::gl::ScopedFramebuffer fboScoped( mFbo );
		ci::gl::clear( ci::ColorA( 0.0, 0.0, 0.0, 0.0 ) );
		mRenderer.render( mLayout );
	}

	gl::draw( mFbo->getColorTexture() );
}


void CinderProjectApp::updateLayout()
{
	mFont = std::make_shared<text::Font>( ci::app::loadAsset( fontName ), mFontSize );

	mLayout.setFont( *mFont );
	mLayout.setSize( mTextBoxSize );
	mLayout.setTracking( mTracking );
	mLayout.setLanguage( mLanguage );
	mLayout.setScript( mScript );
	mLayout.setDirection( mDirection );
	//mLayout.setLineHeight( mLineHeight );
	mLayout.setLineHeight( text::Unit( mLineHeight, text::EM ) );
	mLayout.calculateLayout( mTestText );

	//mRenderer.setLayout( mLayout );
}

std::string unescape( const std::string& s )
{
	std::string res;
	std::string::const_iterator it = s.begin();

	while( it != s.end() ) {
		char c = *it++;

		if( c == '\\' && it != s.end() ) {
			switch( *it++ ) {
				case '\\':
					c = '\\';
					break;

				case 'n':
					c = '\n';
					break;

				case 't':
					c = '\t';
					break;

				// all other escapes
				default:
					// invalid escape sequence - skip it. alternatively you can copy it as is, throw an exception...
					continue;
			}
		}

		res += c;
	}

	return res;
}

void CinderProjectApp::textFileUpdated( const ci::WatchEvent& watchEvent )
{
	mTestText = unescape( ci::loadString( ci::loadFile( watchEvent.getFile() ) ) );
	updateLayout();
}

void CinderProjectApp::keyDown( KeyEvent event )
{
	if( event.getChar() == KeyEvent::KEY_1 ) {
		mLayout.setAlignment( text::Alignment::LEFT );
	}

	else if( event.getCode() == KeyEvent::KEY_2 ) {
		mLayout.setAlignment( text::Alignment::CENTER );
	}

	else if( event.getCode() == KeyEvent::KEY_3 ) {
		mLayout.setAlignment( text::Alignment::RIGHT );
	}

	else if( event.getCode() == KeyEvent::KEY_4 ) {

		mLayout.setAlignment( text::Alignment::JUSTIFIED );
	}

	else if( event.getCode() == KeyEvent::KEY_UP ) {
		mLineHeight -= 0.1;
	}

	else if( event.getCode() == KeyEvent::KEY_DOWN ) {
		mLineHeight += 0.1;
	}

	else if( event.getCode() == KeyEvent::KEY_LEFT ) {
		mTracking -= 0.5;
	}

	else if( event.getCode() == KeyEvent::KEY_RIGHT ) {
		mTracking += 0.5;
	}

	else if( event.getChar() == '+' ) {
		mFontSize += 5;
	}

	else if( event.getChar() == '-' ) {
		if( mFontSize > 1 ) {
			mFontSize -= 5;
		}
	}

	updateLayout();
}

CINDER_APP( CinderProjectApp, RendererGl, [&]( App::Settings* settings )
{
	//settings->setHighDensityDisplayEnabled( true );
} )
