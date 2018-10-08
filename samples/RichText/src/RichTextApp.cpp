#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/FileWatcher.h"
#include "cinder/Utilities.h"

#include "text/FontManager.h"
#include "text/TextLayout.h"
#include "text/gl/TextureRenderer.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class RichTextApp : public App
{
	public:
		void setup() override;
		void mouseDown( MouseEvent event ) override;
		void update() override;
		void draw() override;

		//void updateLayout();
		void textFileUpdated( const ci::WatchEvent& event );

		//std::string mAttrText = "<span font-family=\"HelveticaRounded LT Std Blk\" font-style=\"Black\">This is system \ntext</span><span font-family=\"Source Serif Pro\" font-style=\"Regular\" font-size=\"20\" color=\"#ff0000\"><span font-size=\"20\">Ligatures like \"fi tf\"</span> This is a test of mixing<br/>font attributes like <i>italics</i>, <span color=\"#0000FF\">color</span> and <b>Bold!</b> </span><span font-family=\"Source Serif Pro\"> Here is some white serif text at <span font-size=\"30\">different</span><span font-size=\"10\"> sizes</span></span>";

		text::Layout mLayout;
		text::gl::TextureRenderer mRenderer;

		ci::Rectf mTextBox;

		// Base font, need to remove this, shouldn't be required
		std::shared_ptr<text::Font> mBaseFont;

		std::string testTextFilename = "text/richText.txt";

};

void RichTextApp::setup()
{
	setWindowSize( 1920.f, 1080.f );

	mTextBox = ci::Rectf( 50, 50, 1024.f, 1024.f );

	mBaseFont = std::make_shared<text::Font>( loadAsset( ( "fonts/SourceSansPro/SourceSansPro-Regular.otf" ) ), 12 );

	// Load font faces to use with rich text
	text::FontManager::get()->loadFace( getAssetPath( "fonts/SourceSansPro/SourceSansPro-Regular.otf" ) );
	text::FontManager::get()->loadFace( getAssetPath( "fonts/SourceSansPro/SourceSansPro-It.otf" ) );
	text::FontManager::get()->loadFace( getAssetPath( "fonts/SourceSansPro/SourceSansPro-Bold.otf" ) );

	ci::FileWatcher::instance().watch( ci::app::getAssetPath( testTextFilename ), std::bind( &RichTextApp::textFileUpdated, this, std::placeholders::_1 ) );
}

void RichTextApp::mouseDown( MouseEvent event )
{
}

void RichTextApp::update()
{
}

void RichTextApp::textFileUpdated( const ci::WatchEvent& watchEvent )
{
	// Layout text
	text::RichText richText( ci::loadString( ci::loadFile( watchEvent.getFile() ) ) );
	text::AttributedString attr( richText );

	//attr << text::AttributeFontFamily( "test" )
	//     << text::AttributeFontStyle( "italic" )
	//     << text::AttributeFontSize( 28 )
	//     << text::RichText( "HELLO!" );

	mLayout.setSize( mTextBox.getSize() );
	mLayout.calculateLayout( attr );
	mRenderer.setLayout( mLayout );
}

void RichTextApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
	ci::gl::enableAlphaBlending();
	ci::gl::ScopedMatrices matrices;
	ci::gl::translate( mTextBox.getUpperLeft() );

	ci::gl::color( 0.25, 0.25, 0.25 );
	ci::gl::drawStrokedRect( ci::Rectf( ci::vec2( 0.f ), mTextBox.getSize() ) );

	ci::gl::color( 1, 1, 1 );
	mRenderer.draw();
}

CINDER_APP( RichTextApp, RendererGl )
