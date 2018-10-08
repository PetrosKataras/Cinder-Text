#pragma once

#include "cinder/Vector.h"
#include "cinder/text/Font.h"

#include <memory>
#include <string>

#include "hb.h"
//typedef struct hb_buffer_t hb_buffer_t;
//typedef struct hb_font_t hb_font_t;

namespace text {

class Shaper
{
  public:
	enum Feature {
		LIGATURES,
		KERNING,
		CLIG,
		CALT
	};

	typedef struct {
		std::string data;
		std::string language;
		hb_script_t script;
		hb_direction_t direction;
		const char* c_data() { return data.c_str(); };
	} Text;

	typedef struct {
		uint32_t index;

		ci::vec2 offset;
		ci::vec2 advance;
		uint32_t cluster;
		std::string text;
		std::vector<int> textIndices;
	} Glyph;

	Shaper( const Font& font );
	~Shaper();

	std::vector<Shaper::Glyph> getShapedText( Text& text );
	void addFeature( Feature feature );
	void removeFeature( Feature feature );

  private:
	// Harfbuzz
	hb_font_t* 	getHarfbuzzFont( Font& font ) { return mFont; };

	hb_font_t* 					mFont;
	hb_buffer_t*				mBuffer;
	std::vector<hb_feature_t>	mFeatures;
};

} // namespace text
