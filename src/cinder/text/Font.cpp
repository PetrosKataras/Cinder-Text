#include "cinder/text/Font.h"
#include "cinder/text/FontManager.h"
#include "cinder/text/SystemFonts.h"

namespace cinder { namespace text {

// Font
Font::Font( ci::DataSourceRef source, int size )
	: mSize( size )
	, mFaceId( FontManager::get()->getFaceId( source->getFilePath().string() ) )
{
}

Font::Font( uint32_t faceId, int size )
	: mSize( size )
	, mFaceId( faceId )
{
}

Font::Font( std::string family, int size )
	: Font( family, "Regular", size )
{

}
Font::Font( std::string family, std::string style, int size )
	: mSize( size )
	, mFaceId( FontManager::get()->getFaceId( family, style ) )
{
}

std::string Font::getFamily() const
{
	return FontManager::get()->getFontFamily( *this );
}

std::string Font::getStyle() const
{
	return FontManager::get()->getFontStyle( *this );
}

float Font::getLineHeight() const
{
	return FontManager::get()->getLineHeight( *this );
}

DefaultFont::DefaultFont()
	: Font( SystemFonts::get()->getDefaultFamily(), SystemFonts::get()->getDefaultStyle(), SystemFonts::get()->getDefaultSize() )
{}

} } // namespace cinder::text
