#pragma once

#include <memory>
#include <vector>

#include <cinder/Vector.h>
#include <cinder/Rect.h>

#include "cinder/text/Font.h"
#include "cinder/text/FontManager.h"
#include "cinder/text/AttributedString.h"
#include "cinder/text/TextUnits.h"
#include "cinder/text/Shaper.h"

namespace cinder { namespace text {

typedef enum Alignment { LEFT, CENTER, RIGHT, JUSTIFIED } Alignment;
enum { GROW = 0 };

class Layout {
  public:
	// A single character
	typedef struct {
		uint32_t index;
		ci::Rectf bbox;			// combined position, offset and size
		ci::vec2 position;		// upper left position of glyph
		ci::vec2 size;			// size of glyph
		ci::vec2 offset;		// position offset of glyph
		std::string value;
	} Glyph;

	// A group of characters with the same attributes
	struct Run {
		Run( const Font& font, const ci::Color& color, const float& opacity )
			: font( font )
			, color( color )
			, opacity( opacity )
		{};

		Font font;
		ci::Color color;
		float opacity;

		std::vector<Glyph> glyphs;
	};

	// A line of runs fit within the layout
	struct Line {
		std::vector<Run> runs;
		int width;
		int getTotalGlyphs()
		{
			int totalGlyphs = 0;

			for( auto& run : runs ) {
				totalGlyphs += run.glyphs.size();
			}

			return totalGlyphs;
		}

	};

	Layout();

	// Layout Calculation
	void calculateLayout( std::string text );
	void calculateLayout( const AttributedString& attrString );

	// Lines
	const std::vector<Line>& getLines() const { return mLines; };
	std::vector<Line>& getLines() { return mLines; };

	// Layout Attributes
	const Font& getFont() const { return mFont; }
	Layout& setFont( const Font& font ) { mFont = font; return *this; }

	const ci::Color& getColor() const { return mColor; }
	Layout& setColor( const ci::Color& color ) { mColor = color; return *this; }

	const ci::vec2& getSize() const { return mSize; }
	Layout& setSize( ci::vec2 size ) { mSize = size; return *this; }

	const ci::vec2 measure();

	const std::vector<ci::Rectf>& getGlyphBoxes() const { return mGlyphBoxes; }

	float getLineHeight() const { return mLineHeight.getValue( getFont().getLineHeight() ); }
	Layout& setLineHeight( const float& lineHeight ) { mLineHeight = cinder::text::Unit( lineHeight ); return *this; };
	Layout& setLineHeight( const Unit& lineHeight ) { mLineHeight = lineHeight; return *this; };

	float getTracking() const { return mTracking.getValue( getFont().getSize() ); }
	Layout& setTracking( float tracking ) { mTracking = cinder::text::Unit( tracking ); return *this; };
	Layout& setTracking( const Unit& tracking ) { mTracking = tracking; return *this; };

	Layout& setUseLigatures( const bool useLigatures ) { mUseLigatures = useLigatures; return *this; };
	Layout& setUseKerning( const bool useKerning ) { mUseKerning = useKerning; return *this; };
	Layout& setUseClig( const bool useClig ) { mUseClig = useClig; return *this; };
	Layout& setUseCalt( const bool useCalt ) { mUseCalt = useCalt; return *this; };

	std::string getLanguage() const { return mLanguage; }
	Layout& setLanguage( std::string language ) { mLanguage = language; return *this; }

	Script getScript() const { return mScript; }
	Layout& setScript( Script script ) { mScript = script; return *this; }

	Direction getDirection() const { return mDirection; }
	Layout& setDirection( Direction direction )
	{
		mDirection = direction;

		// If alignment isn't manually set, default to LEFT for LTR and RIGHT for RTL
		if( mUseDefaultAlignment ) {
			mAlignment = mDirection == Direction::LTR ? Alignment::LEFT : Alignment::RIGHT;
		}

		return *this;
	}

	const Alignment& getAlignment() const { return mAlignment; }
	Layout& setAlignment( Alignment alignment )
	{
		mAlignment = alignment;
		mUseDefaultAlignment = false;
		return *this;
	};

private:
	// Attributes
	Font mFont;
	ci::Color mColor;
	Alignment mAlignment;
	bool mUseDefaultAlignment;
	cinder::text::Unit mLineHeight;
	cinder::text::Unit mTracking;

	bool mUseLigatures;
	bool mUseKerning;
	bool mUseClig;
	bool mUseCalt;

	std::string mLanguage;
	Script mScript;
	Direction mDirection;

	ci::vec2 mSize;
	ci::vec2 mLayoutSize;

	// Layout calculation
	void resetLayout();

	// Line breaking
	struct BreakIndices {
		int textBreakIndex = -1;
		int glyphBreakIndex = -1;
		bool found = false;
	};
	BreakIndices getClosestBreakForShapedText( int startIndex, const std::vector<Shaper::Glyph>& shapedGlyphs, const std::vector<uint8_t> lineBreaks, Direction direction );

	ci::vec2 mCurDirection;
	float mCharPos, mLinePos;
	Line mCurLine;
	float mCurLineWidth = 0;
	float mCurLineHeight = 0;
	std::vector<ci::Rectf> mGlyphBoxes;

	float getLineHeightForSubstring( const AttributedString::Substring& substring, const Font& runFont );

	void addSubstringToCurLine( AttributedString::Substring& substring );
	void addRunToCurLine( Run& run );
	void addCurLine();
	void applyAlignment();
	std::vector<Line> mLines;

	bool mMaxLinesReached = false;
};

} } // namespace cinder::text
