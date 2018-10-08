#pragma once

#include "cinder/Buffer.h"
#include "cinder/text/Font.h"

#include <memory>
#include <map>

namespace text {

typedef std::shared_ptr<class SystemFonts> SystemFontsRef;

class SystemFonts
{
  public:
	static SystemFontsRef get()
	{
		static SystemFontsRef ref( new SystemFonts() );
		return ref;
	};

	ci::BufferRef getFontBuffer( std::string family, std::string style );

	std::string getDefaultFamily() { return mDefaultFamily; };
	std::string getDefaultStyle() { return mDefaultStyle; };
	int getDefaultSize() { return mDefaultSize; };

  private:
	SystemFonts();

	void listFaces();
	std::map<std::string,std::vector<std::string>> 	mFaces;
	std::map<std::string,ci::fs::path>				mSystemNameToPath;

	std::string	mDefaultFamily;
	std::string	mDefaultStyle;
	int			mDefaultSize;
};

} // namespace text
