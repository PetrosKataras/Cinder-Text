#pragma once
#include "cinder/app/App.h"
#include "cinder/gl/gl.h"
#include "cinder/text/TextLayout.h"
#include "cinder/text/gl/TextureRenderer.h"

namespace cinder { namespace text {

typedef std::shared_ptr<class TextureFont>	TextureFontRef;

class TextureFont
{
  public:

	struct GlyphInfo {
		int16_t index;
		Layout::Glyph glyph;
		int textureIndex;
	};

	class Format {
	  public:
		Format() : mTextureWidth( 1024 ), mTextureHeight( 1024 ), mPremultiply( false ), mMipmapping( false )
		{}
		
		//! Sets the width of the textures created internally for glyphs. Default \c 1024
		Format&		textureWidth( int32_t textureWidth ) { mTextureWidth = textureWidth; return *this; }
		//! Returns the width of the textures created internally for glyphs. Default \c 1024
		int32_t		getTextureWidth() const { return mTextureWidth; }
		//! Sets the height of the textures created internally for glyphs. Default \c 1024
		Format&		textureHeight( int32_t textureHeight ) { mTextureHeight = textureHeight; return *this; }
		//! Sets the height of the textures created internally for glyphs. Default \c 1024
		int32_t		getTextureHeight() const { return mTextureHeight; }
		
		//! Sets whether the TextureFont render premultiplied output. Default \c false
		Format&		premultiply( bool premult = true ) { mPremultiply = premult; return *this; }
		//! Returns whether the TextureFont renders premultiplied output. Default \c false
		bool		getPremultiply() const { return mPremultiply; }
		
		//! Enables or disables mipmapping. Default is disabled.
		Format&		enableMipmapping( bool enable = true ) { mMipmapping = enable; return *this; }
		//! Returns whether the TextureFont texture has mipmapping enabled
		bool		hasMipmapping() const { return mMipmapping; }
		
	  protected:
		int32_t		mTextureWidth, mTextureHeight;
		bool		mPremultiply;
		bool		mMipmapping;
	};

	    //! Creates a new PangoTextureFontRef with font \a font, ensuring that glyphs necessary to render \a supportedChars are renderable, and format \a format
	static TextureFontRef create( const Format &format = Format(), const std::string &supportedChars = TextureFont::defaultChars() )
	{ return TextureFontRef( new TextureFont( format, supportedChars ) ); }

	TextureFont( const Format &format, const std::string &supportedChars ) : 
		mFormat( format ), 
		mSupportedChars( supportedChars )
	{}
	~TextureFont() {};

	void setLayout( const text::Layout& layout );
	void drawGlyphs();
	//! Returns the current set of characters along with its location into the set of textures
	const std::unordered_map<int16_t, text::TextureFont::GlyphInfo>& getGlyphMap() const { return mGlyphMap; }
	//! Returns the vector of gl::TextureRef corresponding to each page of the atlas
	const std::vector<ci::gl::TextureRef>& getTextures() const { return mTextures; }


  private:
	//static std::string		defaultChars() { return "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnooppqrstuvwxyz1234567890().?!,:;'\"&*=+-/\\@#_[]<>%^ftllflfiphrids\xC3\xA0\303\221\xEF\xAC\x81\xEF\xAC\x82"; }
	static std::string		defaultChars() { return "A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h i j k l m n o p q r s t u v w x y z 1 2 3 4 5 6 7 8 9 0 ( ) . ? ! , : ' \" & * = + - / \\ @ # _ [ ] < > % ^ ft ll fl fi ph ri ds \xC3\xA0 \303\221 ;"; }
	
	Layout mLayout;
	std::vector<ci::gl::Texture2dRef>				mTextures;
	std::unordered_map<int16_t, text::TextureFont::GlyphInfo>		mGlyphMap;
	//std::unordered_map<ci::Font::Glyph, GlyphInfo>	mGlyphMap;
	TextureFont::Format								mFormat;
	std::string										mSupportedChars;
	text::gl::TextureRenderer						mRenderer;
	ci::gl::FboRef									mFbo;

	void renderLayout( const cinder::text::Layout& layout );
	void allocateFbo();

	/*
	public:

	class Format {
	  public:
		Format() : mTextureWidth( 1024 ), mTextureHeight( 1024 ), mPremultiply( false ), mMipmapping( false )
		{}
		
		//! Sets the width of the textures created internally for glyphs. Default \c 1024
		Format&		textureWidth( int32_t textureWidth ) { mTextureWidth = textureWidth; return *this; }
		//! Returns the width of the textures created internally for glyphs. Default \c 1024
		int32_t		getTextureWidth() const { return mTextureWidth; }
		//! Sets the height of the textures created internally for glyphs. Default \c 1024
		Format&		textureHeight( int32_t textureHeight ) { mTextureHeight = textureHeight; return *this; }
		//! Sets the height of the textures created internally for glyphs. Default \c 1024
		int32_t		getTextureHeight() const { return mTextureHeight; }
		
		//! Sets whether the TextureFont render premultiplied output. Default \c false
		Format&		premultiply( bool premult = true ) { mPremultiply = premult; return *this; }
		//! Returns whether the TextureFont renders premultiplied output. Default \c false
		bool		getPremultiply() const { return mPremultiply; }
		
		//! Enables or disables mipmapping. Default is disabled.
		Format&		enableMipmapping( bool enable = true ) { mMipmapping = enable; return *this; }
		//! Returns whether the TextureFont texture has mipmapping enabled
		bool		hasMipmapping() const { return mMipmapping; }
		
	  protected:
		int32_t		mTextureWidth, mTextureHeight;
		bool		mPremultiply;
		bool		mMipmapping;
	};
	
	  //! Creates a new PangoTextureFontRef with font \a font, ensuring that glyphs necessary to render \a supportedChars are renderable, and format \a format
	static PangoTextureFontRef		create( const ci::ivec2 &size, bool mLoadExternalfonts, const Format &format = Format(), const std::string &supportedChars = PangoTextureFont::defaultChars() )
	{ return PangoTextureFontRef( new PangoTextureFont( size, mLoadExternalfonts, format, supportedChars ) ); }

	void drawGlyphs();

	//! Returns the current set of characters along with its location into the set of textures
	const std::unordered_map<ci::Font::Glyph, GlyphInfo>& getGlyphMap() const { return mGlyphMap; }
	//! Returns the vector of gl::TextureRef corresponding to each page of the atlas
	const std::vector<ci::gl::TextureRef>& getTextures() const { return mTextures; }

  protected:
	PangoTextureFont( const ci::ivec2 &size, bool mLoadExternalfonts, const Format &format, const std::string &supportedChars );
  
  private:
	//static std::string		defaultChars() { return "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890().?!,:;'\"&*=+-/\\@#_[]<>%^llflfiphrids\303\251\303\241\303\250\303\240"; }
	static std::string		defaultChars() { return "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnooppqrstuvwxyz1234567890().?!,:;'\"&*=+-/\\@#_[]<>%^ftllflfiphrids\xC3\xA0\303\221\xEF\xAC\x81\xEF\xAC\x82"; }

	std::vector<ci::gl::Texture2dRef>				mTextures;
	std::unordered_map<ci::Font::Glyph, GlyphInfo>	mGlyphMap;
	PangoTextureFont::Format						mFormat;
	std::string										mSupportedChars;*/
};

} } // namespace cinder::text
