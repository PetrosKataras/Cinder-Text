#include "cinder/text/SystemFonts.h"
#include "cinder/Unicode.h"
#include "cinder/app/App.h"

#if defined( CINDER_MSW_DESKTOP )
	#include "cinder/msw/cindermsw.h"
	#include "cinder/msw/cindermswgdiplus.h"
	#include <strsafe.h>
#elif defined( CINDER_MAC )
	#include <CoreText/CoreText.h>
#endif

#include <unordered_map>

namespace text {

// Windows
#if defined( CINDER_MSW_DESKTOP )
HDC mFontDC = nullptr;

SystemFonts::SystemFonts()
{
	mFontDC = ::CreateCompatibleDC( NULL );

	mDefaultFamily = "Arial";
	mDefaultStyle = "Regular";
	mDefaultSize = 12;
}


int CALLBACK EnumFacesExProc( ENUMLOGFONTEX* lpelfe, NEWTEXTMETRICEX* lpntme, int FontType, LPARAM lParam )
{
	std::string familyName = ci::toUtf8( ( char16_t* )lpelfe->elfLogFont.lfFaceName );
	std::string style = ci::toUtf8( ( char16_t* )lpelfe->elfStyle );

	( *reinterpret_cast<std::map<std::string, std::vector<std::string>>*>( lParam ) )[familyName].push_back( style );

	return 1;
}

struct HFontRequest {
	LOGFONT logfont;
	std::string family;
	std::string style;
	bool fontFound = false;
};

HFontRequest mHFontRequest;

int CALLBACK LoadHFontExProc( ENUMLOGFONTEX* lpelfe, NEWTEXTMETRICEX* lpntme, int FontType, LPARAM lParam )
{
	HFontRequest* request = reinterpret_cast<HFontRequest*>( lParam );

	std::string familyName = ci::toUtf8( ( char16_t* )lpelfe->elfLogFont.lfFaceName );
	std::transform( familyName.begin(), familyName.end(), familyName.begin(), ::tolower );

	std::string style = ci::toUtf8( ( char16_t* )lpelfe->elfStyle );
	std::transform( style .begin(), style.end(), style.begin(), ::tolower );

	if( familyName == request->family && style == request->style ) {
		request->logfont = lpelfe->elfLogFont;
		request->fontFound = true;
	}

	return 1;
}

void SystemFonts::listFaces()
{
	mFaces.clear();

	::LOGFONT lf;
	lf.lfCharSet = ANSI_CHARSET;
	lf.lfFaceName[0] = '\0';

	::EnumFontFamiliesEx( mFontDC, &lf, ( FONTENUMPROC )EnumFacesExProc, reinterpret_cast<LPARAM>( &mFaces ), 0 );

	for( auto& family : mFaces ) {
		ci::app::console() << family.first << std::endl;
		ci::app::console() << "---------------------" << std::endl;

		for( auto& style : family.second ) {
			ci::app::console() << style << std::endl;
		}

		ci::app::console() << std::endl;
	}
}

ci::BufferRef SystemFonts::getFontBuffer( std::string family, std::string style )
{
	::LOGFONT lf;
	lf.lfCharSet = ANSI_CHARSET;
	lf.lfFaceName[0] = '\0';

	std::u16string faceName = ci::toUtf16( family );
	::StringCchCopy( lf.lfFaceName, LF_FACESIZE, ( LPCTSTR )faceName.c_str() );

	mHFontRequest.family = family;
	mHFontRequest.style = style;

	::EnumFontFamiliesEx( mFontDC, &lf, ( FONTENUMPROC )LoadHFontExProc, reinterpret_cast<LPARAM>( &mHFontRequest ), 0 );

	HFONT hFont;

	if( mHFontRequest.fontFound ) {
		hFont = ::CreateFontIndirectW( &mHFontRequest.logfont );

		if( hFont ) {
			::SelectObject( mFontDC, hFont );

			DWORD fontSize = ::GetFontData( mFontDC, 0, 0, NULL, 0 );

			void* fontBuffer = new BYTE[fontSize];
			DWORD length = ::GetFontData( mFontDC, 0, 0, fontBuffer, fontSize );


			return ci::Buffer::create( fontBuffer, ( unsigned int )length );
		}
		else {
			return nullptr;
		}
	}
	else {
		return nullptr;
	}
}

#elif defined( CINDER_COCOA )

SystemFonts::SystemFonts()
{
	mDefaultFamily = "ArialMT";
	mDefaultStyle = "Regular";
	mDefaultSize = 12;
	
	CFArrayRef fontUrls = ::CTFontManagerCopyAvailableFontURLs();
	CFIndex count = ::CFArrayGetCount( fontUrls );
	
	const size_t anchorPrefixLength = strlen( "postscript-name=" ); 
	for( CFIndex i = 0; i < count; ++i ) {
		unsigned char bytes[4096];
		char path[4096], postscriptName[4096];
		CFRange pathRange, pathRangeIncludingSeparators, psNameRange, psNameRangeIncludingSeparators;
		
		CFURLRef fontUrl = (CFURLRef)::CFArrayGetValueAtIndex( fontUrls, i );
		pathRange = ::CFURLGetByteRangeForComponent( fontUrl, kCFURLComponentPath, &pathRangeIncludingSeparators );
		psNameRange = ::CFURLGetByteRangeForComponent( fontUrl, kCFURLComponentFragment, &psNameRangeIncludingSeparators );
		if( pathRange.location != kCFNotFound && psNameRange.location != kCFNotFound && psNameRange.length > anchorPrefixLength ) {
			::CFURLGetBytes( fontUrl, bytes, sizeof(bytes) );
			
			CFStringRef pathCopy = ::CFStringCreateWithSubstring( kCFAllocatorDefault, ::CFURLGetString( fontUrl ), pathRange );
			CFStringRef unescapedPath = ::CFURLCreateStringByReplacingPercentEscapes( kCFAllocatorDefault, pathCopy, CFSTR("") );
			::CFStringGetCString( unescapedPath, path, 4096, kCFStringEncodingUTF8 );
			::CFRelease( pathCopy );
			::CFRelease( unescapedPath );				
			
			psNameRange.length -= anchorPrefixLength;
			psNameRange.location += anchorPrefixLength;
			strncpy( postscriptName, (const char *)&bytes[psNameRange.location], psNameRange.length );
			postscriptName[psNameRange.length] = 0;
		}
		else
			continue;
		
		mSystemNameToPath[std::string(postscriptName)] = ci::fs::path( path );
	}
	::CFRelease( fontUrls );
}

void SystemFonts::listFaces()
{
}

ci::BufferRef SystemFonts::getFontBuffer( std::string family, std::string style )
{
	return std::make_shared<ci::Buffer>( ci::loadFile( mSystemNameToPath["ArialMT"] ) );
	
	if( mSystemNameToPath.find( family ) != mSystemNameToPath.end() ) 
		return std::make_shared<ci::Buffer>( ci::loadFile( mSystemNameToPath[family] ) );
	else
		return ci::BufferRef();
}
#endif // defined( CINDER_COCOA )

} // namespace text
