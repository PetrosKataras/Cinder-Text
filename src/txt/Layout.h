#pragma once

#include <memory>
#include <vector>

#include <cinder/Vector.h>
#include <cinder/Rect.h>

#include "txt/Font.h"
#include "txt/FontManager.h"
#include "txt/Parser.h"

namespace txt
{
	class Layout
	{
		public:
			typedef struct {
				uint32_t index;
				ci::Rectf bbox;
			} Glyph;

			struct Run {
				Run( const Font& font, const ci::Color& color )
					: font( font )
					, color( color )
				{};

				const Font font;
				const ci::Color color;

				std::vector<Glyph> glyphs;
			};

			struct Line {
				std::vector<Run> runs;
			};

			typedef enum Alignment { LEFT, CENTER, RIGHT } Alignment;
			enum { GROW = 0 };

			Layout();

			void calculateLayout( const Font& font, std::string text );
			void calculateLayout( std::string text );

			std::vector<Line>& getLines() { return mLines; };

			void setSize( ci::vec2 size ) { mSize = size; }
			void setLeading( float leading ) { mLeading = leading; };
			void setTracking( float tracking ) { mTracking = tracking;};
			void setAlignment( Alignment alignment ) { mAlignment = alignment; };

		private:
			float mLeading;
			float mTracking;
			Alignment mAlignment;

			void reset();
			ci::vec2 mPen;
			Line mCurLine;
			float mLineWidth = 0;
			float mLineHeight = 0;
			float mAscender = 0;

			ci::vec2 mSize;

			void addSubstringToCurLine( Parser::Substring& substring );
			void addRunToCurLine( Run& run );
			void addCurLine( );
			std::vector<Line> mLines;

			bool mMaxLinesReached = false;
	};
}