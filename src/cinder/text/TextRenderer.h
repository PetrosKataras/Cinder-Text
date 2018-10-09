#pragma once

#include "cinder/gl/Texture.h"
#include "cinder/Vector.h"
#include "cinder/text/TextLayout.h"

#include <string>

namespace cinder { namespace text {

class Renderer;
typedef std::shared_ptr<Renderer> RendererRef;

class Renderer {
  public:
	Renderer() {}

	//void draw( const Layout& layout )
	//{
	//	mLayout = layout;
	//	draw();
	//}

	virtual void draw() = 0;
	void setSize( ci::ivec2 size ) { mSize = size; };
	void setSize( int x, int y ) { mSize = ivec2( x, y ); };

	//virtual void setLayout( const Layout& layout ) { mLayout = layout; }
	//virtual const Layout& getLayout( const Layout& layout ) { return mLayout; }
	virtual ci::gl::TextureRef render( const ci::text::Layout& layout ) = 0;

  protected:
	//Layout mLayout;
	ci::ivec2 mSize;
};

} } // namespace cinder::text
