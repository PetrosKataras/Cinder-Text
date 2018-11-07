#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/text/FontManager.h"
#include "cinder/text/gl/TextureRenderer.h"
#include "cinder/text/TextLayout.h"
#include "cinder/Easing.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class LineAnimatorApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;

	std::shared_ptr<text::Font> mFont;
	text::gl::TextureRenderer mRenderer;
	text::Layout mLayout;
	ci::gl::FboRef mFbo;
	std::vector<ci::text::gl::TextureRenderer::LayoutCache> mLineCache;
	ci::text::gl::TextureRenderer::LayoutCache mLayoutCache;
};

void LineAnimatorApp::setup()
{
	ci::gl::Fbo::Format fboFormat;
	ci::gl::Texture::Format texFormat;
	texFormat.setMagFilter( GL_NEAREST );
	texFormat.setMinFilter( GL_LINEAR );
	texFormat.enableMipmapping( true );
	fboFormat.setSamples( 8 );
	fboFormat.setColorTextureFormat( texFormat );
 	mFbo = ci::gl::Fbo::create( getWindowWidth() * 0.9f, getWindowHeight() * 0.9f, fboFormat );

	ci::text::gl::TextureArray::Format fmt = text::gl::TextureArray::Format()
		.size( ivec3( 1024, 1024, 8) );
	ci::text::gl::TextureRenderer::setTextureFormat( fmt );
	//ci::text::gl::TextureRenderer::enableSharedCaches();

	text::Font font1 = text::Font( ci::app::loadAsset( "../../assets/fonts/SourceSansPro/SourceSansPro-Regular.otf" ), 24.f );
	text::Font font2 = text::Font( ci::app::loadAsset( "../../assets/fonts/SourceSerifPro/SourceSerifPro-Bold.otf" ), 24.f );
	text::gl::TextureRenderer::loadFont( font1 );
	text::gl::TextureRenderer::loadFont( font2 );

	//text::gl::TextureRenderer::unloadFont( font1 );
	//text::gl::TextureRenderer::unloadFont( font2 );

	//mFont = std::make_shared<text::Font>( ci::app::loadAsset( "../../assets/fonts/SourceSansPro/SourceSansPro-Regular.otf" ), 24.f );
	//text::gl::TextureRenderer::loadFont( *mFont );
	//text::gl::TextureRenderer::loadFont( text::Font( ci::app::loadAsset( "../../assets/fonts/SourceSerifPro/SourceSerifPro-Bold.otf" ), 24.f ) );

	//mLayout.setFont( *mFont );
	mLayout.setSize( mFbo->getSize() );
	mLayout.setFont( font1 );

	text::AttributedString attrStr;
	attrStr << "Lorem ipsum dolor " << text::AttributeColor( ci::ColorA( 1.0f, 0.f, 1.f ) ) << text::AttributeFont( font1 );
	attrStr << "sit amet " << text::AttributeColor( ci::ColorA( 1.f, 1.f, 1.f ) ) << text::AttributeFont( font2 );
	attrStr << "consectetur adipiscing elit. " << text::AttributeColor( ci::ColorA( 1.0f, 1.f, 1.f ) ) << text::AttributeFontFamily( "Helvetica" );
	attrStr << "Praesent lobortis risus sed aliquam commodo. " << text::AttributeLineBreak();
	attrStr << "Praesent porttitor rhoncus tempus." << text::AttributeFontSize( 50.f ) << text::AttributeFontStyle( "Bold" );

	mLayout.calculateLayout( "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Praesent lobortis risus sed aliquam commodo. Praesent porttitor rhoncus tempus." );
	//mLayout.calculateLayout( attrStr );

	for( auto line : mLayout.getLines() )
	{
		//mLineCache.push_back( mRenderer.cacheLine( line ) );
	}

	text::gl::TextureRenderer::printCachedFonts();
	mLayoutCache = mRenderer.cacheLayout( mLayout, true, true, true );
}

void LineAnimatorApp::mouseDown( MouseEvent event )
{
}

void LineAnimatorApp::update()
{
	float duration = 1.5f;
	float delta = 1.0f / mLayoutCache.batch.count;
	float progress = fmod( getElapsedSeconds(), duration + 2.0 ) / duration;

	// scale
	{
		auto ptr = mLayoutCache.mapDynamicScale();
		for( auto i = 0; i < mLayoutCache.batch.count; i++ ){
			
			float scale = (progress) - delta * float(i);
			scale = clamp( scale, 0.0f, 1.0f );
			scale = easeOutElastic( scale, 1.5, 0.4 );
			mLayoutCache.applyScaleAttr( ptr, vec2( scale ) );
		}
		mLayoutCache.unmapDynamicScale( ptr );
	}

	
	// color
	{
		auto ptr = mLayoutCache.mapDynamicColor();
		
		for( auto i = 0; i < mLayoutCache.batch.count; i++ ){
			float alpha = progress - delta * float(i);
			alpha = clamp( alpha, 0.0f, 1.0f );
			
			//ColorA color = ci::ColorA( CM_HSV, fmod( float(getElapsedSeconds() * 0.5f) + float(i) * 0.01f, 1.0f ), 1.0f, 1.0f, alpha );
			ColorA color = ci::ColorA( 1.0f, 1.0f, 1.0f, alpha );
			mLayoutCache.applyColorAttr( ptr, color );
		}
		mLayoutCache.unmapDynamicColor( ptr );
	}

	/*
	// offset
	auto ptr = mLayoutCache.mapDynamicOffset();
	for( auto i = 0; i < mLayoutCache.batch.count; i++ ){
		float x =  0.0;
		float y = sin( getElapsedSeconds() * 2.0 + float(i) * 0.1f ) * 30.0f;
		mLayoutCache.applyOffsetAttr( ptr, vec4( x, y, 0.0, 1.0 ) );
	}
	mLayoutCache.unmapDynamicOffset( ptr );*/
}

void LineAnimatorApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
	gl::enableAlphaBlending;
	
	for( int i = 0; i < mLineCache.size(); i++ )
	{
		ci::gl::ScopedMatrices scpMtrx;
		auto line = mLineCache[i];
		float progress = sin( (getElapsedSeconds() + float(i) * 0.1) * 5.0f ) * 0.5 + 0.5;
		float alpha = progress;
		gl::ScopedColor scpColor( ColorA( 1.0, 1.0, 1.0, alpha ) );
		gl::translate( vec2( 0.0,  (1.0 - progress) * 20.0 ) );
		mRenderer.render( line );
	}

	


	/*{
		//for( auto layoutBatch : layoutBatches ) {
			auto &glyphPositionBuffer = mLayoutCache.positionOffsets;
			int glyphCount = glyphPositionBuffer.size();
			//float delta = 1.0f / float( glyphCount );
			for(int i = 0; i < glyphCount; ++i ) {
				float progress = sin( (getElapsedSeconds() + float(i) * 0.1) * 5.0f ) * 0.5 + 0.5;
				vec3 &pos = glyphPositionBuffer[i];
				pos.y = progress * 20.0f;
			}
			mRenderer.updateCache( mLayoutCache );
		//}
	}*/

	{
		ci::gl::ScopedMatrices scpMtrx;
		gl::translate( vec2( getWindowSize() ) * 0.1f );
		gl::scale( getWindowSize() / mFbo->getSize() );
		float progress = sin( (getElapsedSeconds()) * 5.0f ) * 0.5 + 0.5;
		float alpha = progress;
		gl::ScopedColor scpColor( ColorA( 1.0, 1.0, 1.0, 1.0 ) );
		//gl::translate( vec2( 0.0,  (1.0 - progress) * 20.0 ) );
		mRenderer.render( mLayoutCache );
	}

	{
		ci::gl::ScopedMatrices scpMtrx;
		gl::translate( vec2( getWindowWidth() * 0.05f, getWindowHeight() * 0.5f ) );
		mRenderer.render( mLayout );
	}
	
}

CINDER_APP( LineAnimatorApp, RendererGl( RendererGl::Options().msaa( 16 ) ) )
