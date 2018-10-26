#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/text/FontManager.h"
#include "cinder/text/gl/TextureRenderer.h"
#include "cinder/text/TextLayout.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class SdfBasicApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;

	text::gl::TextureRenderer mRenderer;
	text::Layout mLayout;
};

void SdfBasicApp::setup()
{
	text::Font font1 = text::Font( ci::app::loadAsset( "../../assets/fonts/SourceSansPro/SourceSansPro-Regular.otf" ), 24.f );
	text::Font font2 = text::Font( ci::app::loadAsset( "../../assets/fonts/SourceSerifPro/SourceSerifPro-Bold.otf" ), 24.f );
	text::gl::TextureRenderer::loadFont( font1 );
	text::gl::TextureRenderer::loadFont( font2 );

	mLayout.setFont( font1 );
	mLayout.setSize( vec2(getWindowSize()) * vec2(0.9f) );
	text::AttributedString attrStr;

	mLayout.calculateLayout( "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Praesent lobortis risus sed aliquam commodo. Praesent porttitor rhoncus tempus. Vivamus aliquet ullamcorper neque ac blandit. Proin nec mi vitae ligula blandit ornare sit amet at nibh. Donec vel lacus vitae tellus vehicula laoreet in at turpis. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia Curae; Nam pulvinar, purus non tempus tincidunt, arcu ex fringilla velit, quis pharetra nulla sapien eget justo. Praesent urna augue, fringilla vitae malesuada a, aliquam sed tellus." );
}

void SdfBasicApp::mouseDown( MouseEvent event )
{
}

void SdfBasicApp::update()
{
}

void SdfBasicApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );

	{
		gl::ScopedMatrices scpMtrx;
		gl::translate( vec2( getWindowSize() ) * vec2( 0.05f ) );
		mRenderer.render( mLayout );
	}
}

CINDER_APP( SdfBasicApp, RendererGl )
