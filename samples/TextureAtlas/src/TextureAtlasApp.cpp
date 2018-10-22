#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Log.h"

#include "cinder/text/FontManager.h"
#include "cinder/text/TextLayout.h"
#include "cinder/text/gl/TextureRenderer.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class TextureAtlasApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;

	cinder::text::Layout mLayout;
	cinder::text::gl::TextureRenderer mRenderer;
	ci::gl::Texture3dRef mGlyphTexture;
	ci::gl::BatchRef mBatch;

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

in vec2 texCoord;
in vec4 globalColor;

out vec4 color;

void main( void )
{ 
	vec3 coord = vec3(texCoord.x, texCoord.y, uLayer);
	vec4 texColor = texture( uTexArray, coord );

	color = vec4(1.0, 1.0, 1.0, texColor.r);
	color *= globalColor;
	//color = vec4(1.0,0.0,0.0,1.0);
}
)V0G0N";
};

void TextureAtlasApp::setup()
{
	auto font1 = text::Font( "Arial", 24 * 8 );
	auto font2 = text::Font( loadAsset( "../../assets/fonts/SourceSansPro/SourceSansPro-Regular.otf" ), 72 );
	auto font3 = text::Font( loadAsset( "../../assets/fonts/SourceSerifPro/SourceSerifPro-Black.otf" ), 200 );
	auto font4 = text::Font( loadAsset( "../../assets/fonts/SourceSerifPro/SourceSerifPro-Light.otf" ), 100 );

	//std::string text = "Aa Bb Cc Dd Ee Ff Gg Hh Ii Jj Kk Ll Mm Nn Oo Pp Qq Rr Ss Tt Uu Vv Ww Xx Yy Zz";
	//std::string text = "THIS IS A TEST";
	std::string text = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890().?!,:;'\"&*=+-/\\@#_[]<>%^llflfiphrids\303\251\303\241\303\250\303\240fiftff";

	// Enable glyphs to be cached in a shared texture array
	cinder::text::gl::TextureRenderer::enableSharedCaches( true );
	cinder::text::gl::TextureArray::Format fmt = cinder::text::gl::TextureArray::Format()
		.size( vec3( 1024, 1024, 4 ) )
		.internalFormat( GL_RED )
		.maxAnisotropy( gl::Texture2d::getMaxAnisotropyMax() )
		.mipmap( true );
	cinder::text::gl::TextureRenderer::setTextureFormat( fmt );

	// Glyph caching
	// caches specified string
	//mRenderer.cacheGlyphs( font1, text );
	// caches specified unicode range
	//mRenderer.cacheGlyphs( font1, cinder::text::gl::TextureRenderer::defaultUnicodeRange() );
	// caches default unicode range by default
	mRenderer.loadFont( font1 );
	mRenderer.loadFont( font2 );
	mRenderer.loadFont( font3 );
	mRenderer.loadFont( font4 );

	

	// Atlas retreival
	//auto fontCache = mRenderer.getCacheForFont( font1 );
	//mGlyphTexture = fontCache.texArrayCache.texArray->getTexture();
	//auto glyphMap = fontCache.glyphs;

	auto glyphMap = mRenderer.getGylphMapForFont( font1 );
	mGlyphTexture = mRenderer.getTextureForFont( font1 );

	mRenderer.unloadFont( font1 );
	mRenderer.unloadFont( font2 );
	mRenderer.unloadFont( font3 );
	mRenderer.unloadFont( font4 );

	// Rendering glyphs for text
	ci::gl::GlslProgRef shader = ci::gl::GlslProg::create( vertShader, fragShader );
	shader->uniform( "uTexArray", 0 );
	mBatch = ci::gl::Batch::create( ci::geom::Rect( Rectf( vec2(0.0), vec2( 1.0f ))), shader );
}

void TextureAtlasApp::mouseDown( MouseEvent event )
{
}

void TextureAtlasApp::update()
{
}

void TextureAtlasApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );

	if( !mGlyphTexture )
		return;

	vec2 size = mGlyphTexture->getBounds().getSize();
	int depth = mGlyphTexture->getDepth();
	int cols = ceil( sqrt( depth ) );
	int rows = cols;

	// draw all textures in the texture array
	for( int i = 0; i < depth; i++ ) {
	
		float w = getWindowWidth() / cols;
		float h = w * (size.y / size.x);
		float x = (i % cols) * w;
		float y = (floor( float(i) / float(cols) )) * h;
		
		ci::Area drawingArea = Area( vec2( x, y ), vec2( x, y ) + vec2( w, h ) );

		{
			gl::ScopedMatrices scpMtrx;
			gl::translate( drawingArea.getUL() );
			//float s =  float(drawingArea.getWidth()) / size.y;
			gl::scale( vec2( drawingArea.getSize() ) );

			mBatch->getGlslProg()->uniform( "uLayer", (uint32_t) i );
			ci::gl::ScopedTextureBind texBind( mGlyphTexture->getTarget(), mGlyphTexture->getId() );
			mBatch->draw();
		}

		gl::drawStrokedRect( drawingArea );
	}
	
}

CINDER_APP( TextureAtlasApp, RendererGl )
