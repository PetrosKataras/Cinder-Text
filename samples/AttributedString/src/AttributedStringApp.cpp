#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/text/TextLayout.h"
#include "cinder/text/AttributedString.h"
#include "cinder/text/gl/TextureRenderer.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class AttributedStringApp : public App
{
	public:
		void setup() override;
		void mouseDown( MouseEvent event ) override;
		void update() override;
		void draw() override;

		text::Layout mLayout;
		text::gl::TextureRenderer mRenderer;

		ci::Rectf mTextBox = ci::Rectf( 200.f, 200.f, 1000.f, 1000.f );
		text::AttributedString mAttrStr;

		std::string testText = "This is a test of a basic string to be drawn in a textbox, testing 12345. This is a test of a basic string to be drawn in a textbox. This is a test of a basic string to be drawn in a textbox. This is a test of a basic string to be drawn in a textbox.";
};

void AttributedStringApp::setup()
{
	setWindowSize( 1024.f, 768.f );

	mAttrStr << "It's" << text::AttributeColor( ci::ColorA( 1.0f, 0.f, 1.f ) ) << text::AttributeFontFamily( "Helvetica" );
	mAttrStr << " Wednesday" << text::AttributeColor( ci::ColorA( 1.f, 1.f, 1.f ) )  << text::AttributeFontStyle( "Italic" );
	mAttrStr << " my" << text::AttributeFontStyle( "Regular" ) << text::AttributeLineBreak();
	mAttrStr << "Dudes!" << text::AttributeFontSize( 50.f ) << text::AttributeFontStyle( "Bold" );
	mAttrStr << text::RichText( "<br/><span font-fammily=\"Helvetica\" font-style=\"Italic\" color=\"#ff0000\">Rich Text</span>" );

	mLayout.setSize( mTextBox.getSize() );
	mLayout.calculateLayout( mAttrStr );
	//mRenderer.setLayout( mLayout );
}

void AttributedStringApp::mouseDown( MouseEvent event )
{
	mTextBox.set( mTextBox.x1, mTextBox.y1, event.getX(), event.getY() );
	mLayout.setSize( mTextBox.getSize() );
	mLayout.calculateLayout( mAttrStr );
	//mRenderer.setLayout( mLayout );
}

void AttributedStringApp::update()
{
}

void AttributedStringApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
	ci::gl::enableAlphaBlending();
	ci::gl::ScopedMatrices matrices;
	ci::gl::translate( mTextBox.getUpperLeft() );

	ci::gl::color( 0.25, 0.25, 0.25 );
	ci::gl::drawStrokedRect( ci::Rectf( ci::vec2( 0.f ), mTextBox.getSize() ) );

	ci::gl::color( 1, 1, 1 );
	mRenderer.render( mLayout );
}

CINDER_APP( AttributedStringApp, RendererGl, [&]( App::Settings* settings )
{
	//settings->setHighDensityDisplayEnabled( true );
} )
