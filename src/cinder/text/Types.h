#pragma once
#include <stdint.h>

#define SCRIPT_TAG(c1,c2,c3,c4) ((uint32_t)((((uint32_t)(c1)&0xFF)<<24)|(((uint32_t)(c2)&0xFF)<<16)|(((uint32_t)(c3)&0xFF)<<8)|((uint32_t)(c4)&0xFF)))

namespace cinder { namespace text {

//! Matches 1:1 with HarfBuzz's hb_direction_t type
enum class Direction {
  INVALID = 0,
  LTR = 4,
  RTL,
  TTB,
  BTT
};

//! Matches 1:1 with HarfBuzz's hb_script_t type
enum class Script : uint32_t {
  /*1.1*/ COMMON			= SCRIPT_TAG ('Z','y','y','y'),
  /*1.1*/ INHERITED			= SCRIPT_TAG ('Z','i','n','h'),
  /*5.0*/ UNKNOWN			= SCRIPT_TAG ('Z','z','z','z'),

  /*1.1*/ ARABIC			= SCRIPT_TAG ('A','r','a','b'),
  /*1.1*/ ARMENIAN			= SCRIPT_TAG ('A','r','m','n'),
  /*1.1*/ BENGALI			= SCRIPT_TAG ('B','e','n','g'),
  /*1.1*/ CYRILLIC			= SCRIPT_TAG ('C','y','r','l'),
  /*1.1*/ DEVANAGARI			= SCRIPT_TAG ('D','e','v','a'),
  /*1.1*/ GEORGIAN			= SCRIPT_TAG ('G','e','o','r'),
  /*1.1*/ GREEK			= SCRIPT_TAG ('G','r','e','k'),
  /*1.1*/ GUJARATI			= SCRIPT_TAG ('G','u','j','r'),
  /*1.1*/ GURMUKHI			= SCRIPT_TAG ('G','u','r','u'),
  /*1.1*/ HANGUL			= SCRIPT_TAG ('H','a','n','g'),
  /*1.1*/ HAN				= SCRIPT_TAG ('H','a','n','i'),
  /*1.1*/ HEBREW			= SCRIPT_TAG ('H','e','b','r'),
  /*1.1*/ HIRAGANA			= SCRIPT_TAG ('H','i','r','a'),
/*1.1*/ KANNADA			= SCRIPT_TAG ('K','n','d','a'),
  /*1.1*/ KATAKANA			= SCRIPT_TAG ('K','a','n','a'),
  /*1.1*/ LAO				= SCRIPT_TAG ('L','a','o','o'),
  /*1.1*/ LATIN			= SCRIPT_TAG ('L','a','t','n'),
  /*1.1*/ MALAYALAM			= SCRIPT_TAG ('M','l','y','m'),
  /*1.1*/ ORIYA			= SCRIPT_TAG ('O','r','y','a'),
  /*1.1*/ TAMIL			= SCRIPT_TAG ('T','a','m','l'),
  /*1.1*/ TELUGU			= SCRIPT_TAG ('T','e','l','u'),
  /*1.1*/ THAI			= SCRIPT_TAG ('T','h','a','i'),

  /*2.0*/ TIBETAN			= SCRIPT_TAG ('T','i','b','t'),

  /*3.0*/ BOPOMOFO			= SCRIPT_TAG ('B','o','p','o'),
  /*3.0*/ BRAILLE			= SCRIPT_TAG ('B','r','a','i'),
  /*3.0*/ CANADIAN_SYLLABICS		= SCRIPT_TAG ('C','a','n','s'),
  /*3.0*/ CHEROKEE			= SCRIPT_TAG ('C','h','e','r'),
  /*3.0*/ ETHIOPIC			= SCRIPT_TAG ('E','t','h','i'),
  /*3.0*/ KHMER			= SCRIPT_TAG ('K','h','m','r'),
  /*3.0*/ MONGOLIAN			= SCRIPT_TAG ('M','o','n','g'),
  /*3.0*/ MYANMAR			= SCRIPT_TAG ('M','y','m','r'),
  /*3.0*/ OGHAM			= SCRIPT_TAG ('O','g','a','m'),
  /*3.0*/ RUNIC			= SCRIPT_TAG ('R','u','n','r'),
  /*3.0*/ SINHALA			= SCRIPT_TAG ('S','i','n','h'),
  /*3.0*/ SYRIAC			= SCRIPT_TAG ('S','y','r','c'),
  /*3.0*/ THAANA			= SCRIPT_TAG ('T','h','a','a'),
  /*3.0*/ YI				= SCRIPT_TAG ('Y','i','i','i'),

  /*3.1*/ DESERET			= SCRIPT_TAG ('D','s','r','t'),
  /*3.1*/ GOTHIC			= SCRIPT_TAG ('G','o','t','h'),
  /*3.1*/ OLD_ITALIC			= SCRIPT_TAG ('I','t','a','l'),

  /*3.2*/ BUHID			= SCRIPT_TAG ('B','u','h','d'),
  /*3.2*/ HANUNOO			= SCRIPT_TAG ('H','a','n','o'),
  /*3.2*/ TAGALOG			= SCRIPT_TAG ('T','g','l','g'),
  /*3.2*/ TAGBANWA			= SCRIPT_TAG ('T','a','g','b'),

  /*4.0*/ CYPRIOT			= SCRIPT_TAG ('C','p','r','t'),
  /*4.0*/ LIMBU			= SCRIPT_TAG ('L','i','m','b'),
  /*4.0*/ LINEAR_B			= SCRIPT_TAG ('L','i','n','b'),
  /*4.0*/ OSMANYA			= SCRIPT_TAG ('O','s','m','a'),
  /*4.0*/ SHAVIAN			= SCRIPT_TAG ('S','h','a','w'),
  /*4.0*/ TAI_LE			= SCRIPT_TAG ('T','a','l','e'),
  /*4.0*/ UGARITIC			= SCRIPT_TAG ('U','g','a','r'),

  /*4.1*/ BUGINESE			= SCRIPT_TAG ('B','u','g','i'),
  /*4.1*/ COPTIC			= SCRIPT_TAG ('C','o','p','t'),
  /*4.1*/ GLAGOLITIC			= SCRIPT_TAG ('G','l','a','g'),
  /*4.1*/ KHAROSHTHI			= SCRIPT_TAG ('K','h','a','r'),
  /*4.1*/ NEW_TAI_LUE			= SCRIPT_TAG ('T','a','l','u'),
  /*4.1*/ OLD_PERSIAN			= SCRIPT_TAG ('X','p','e','o'),
  /*4.1*/ SYLOTI_NAGRI		= SCRIPT_TAG ('S','y','l','o'),
  /*4.1*/ TIFINAGH			= SCRIPT_TAG ('T','f','n','g'),

  /*5.0*/ BALINESE			= SCRIPT_TAG ('B','a','l','i'),
  /*5.0*/ CUNEIFORM			= SCRIPT_TAG ('X','s','u','x'),
  /*5.0*/ NKO				= SCRIPT_TAG ('N','k','o','o'),
  /*5.0*/ PHAGS_PA			= SCRIPT_TAG ('P','h','a','g'),
  /*5.0*/ PHOENICIAN			= SCRIPT_TAG ('P','h','n','x'),

  /*5.1*/ CARIAN			= SCRIPT_TAG ('C','a','r','i'),
  /*5.1*/ CHAM			= SCRIPT_TAG ('C','h','a','m'),
  /*5.1*/ KAYAH_LI			= SCRIPT_TAG ('K','a','l','i'),
  /*5.1*/ LEPCHA			= SCRIPT_TAG ('L','e','p','c'),
  /*5.1*/ LYCIAN			= SCRIPT_TAG ('L','y','c','i'),
  /*5.1*/ LYDIAN			= SCRIPT_TAG ('L','y','d','i'),
  /*5.1*/ OL_CHIKI			= SCRIPT_TAG ('O','l','c','k'),
  /*5.1*/ REJANG			= SCRIPT_TAG ('R','j','n','g'),
  /*5.1*/ SAURASHTRA			= SCRIPT_TAG ('S','a','u','r'),
  /*5.1*/ SUNDANESE			= SCRIPT_TAG ('S','u','n','d'),
  /*5.1*/ VAI				= SCRIPT_TAG ('V','a','i','i'),

  /*5.2*/ AVESTAN			= SCRIPT_TAG ('A','v','s','t'),
  /*5.2*/ BAMUM			= SCRIPT_TAG ('B','a','m','u'),
  /*5.2*/ EGYPTIAN_HIEROGLYPHS	= SCRIPT_TAG ('E','g','y','p'),
  /*5.2*/ IMPERIAL_ARAMAIC		= SCRIPT_TAG ('A','r','m','i'),
  /*5.2*/ INSCRIPTIONAL_PAHLAVI	= SCRIPT_TAG ('P','h','l','i'),
  /*5.2*/ INSCRIPTIONAL_PARTHIAN	= SCRIPT_TAG ('P','r','t','i'),
  /*5.2*/ JAVANESE			= SCRIPT_TAG ('J','a','v','a'),
  /*5.2*/ KAITHI			= SCRIPT_TAG ('K','t','h','i'),
  /*5.2*/ LISU			= SCRIPT_TAG ('L','i','s','u'),
  /*5.2*/ MEETEI_MAYEK		= SCRIPT_TAG ('M','t','e','i'),
  /*5.2*/ OLD_SOUTH_ARABIAN		= SCRIPT_TAG ('S','a','r','b'),
  /*5.2*/ OLD_TURKIC			= SCRIPT_TAG ('O','r','k','h'),
  /*5.2*/ SAMARITAN			= SCRIPT_TAG ('S','a','m','r'),
  /*5.2*/ TAI_THAM			= SCRIPT_TAG ('L','a','n','a'),
  /*5.2*/ TAI_VIET			= SCRIPT_TAG ('T','a','v','t'),

  /*6.0*/ BATAK			= SCRIPT_TAG ('B','a','t','k'),
  /*6.0*/ BRAHMI			= SCRIPT_TAG ('B','r','a','h'),
  /*6.0*/ MANDAIC			= SCRIPT_TAG ('M','a','n','d'),

  /*6.1*/ CHAKMA			= SCRIPT_TAG ('C','a','k','m'),
  /*6.1*/ MEROITIC_CURSIVE		= SCRIPT_TAG ('M','e','r','c'),
  /*6.1*/ MEROITIC_HIEROGLYPHS	= SCRIPT_TAG ('M','e','r','o'),
  /*6.1*/ MIAO			= SCRIPT_TAG ('P','l','r','d'),
  /*6.1*/ SHARADA			= SCRIPT_TAG ('S','h','r','d'),
  /*6.1*/ SORA_SOMPENG		= SCRIPT_TAG ('S','o','r','a'),
  /*6.1*/ TAKRI			= SCRIPT_TAG ('T','a','k','r'),

  /*
   * Since: 0.9.30
   */
  /*7.0*/ BASSA_VAH			= SCRIPT_TAG ('B','a','s','s'),
  /*7.0*/ CAUCASIAN_ALBANIAN		= SCRIPT_TAG ('A','g','h','b'),
  /*7.0*/ DUPLOYAN			= SCRIPT_TAG ('D','u','p','l'),
  /*7.0*/ ELBASAN			= SCRIPT_TAG ('E','l','b','a'),
  /*7.0*/ GRANTHA			= SCRIPT_TAG ('G','r','a','n'),
  /*7.0*/ KHOJKI			= SCRIPT_TAG ('K','h','o','j'),
  /*7.0*/ KHUDAWADI			= SCRIPT_TAG ('S','i','n','d'),
  /*7.0*/ LINEAR_A			= SCRIPT_TAG ('L','i','n','a'),
  /*7.0*/ MAHAJANI			= SCRIPT_TAG ('M','a','h','j'),
  /*7.0*/ MANICHAEAN			= SCRIPT_TAG ('M','a','n','i'),
  /*7.0*/ MENDE_KIKAKUI		= SCRIPT_TAG ('M','e','n','d'),
  /*7.0*/ MODI			= SCRIPT_TAG ('M','o','d','i'),
  /*7.0*/ MRO				= SCRIPT_TAG ('M','r','o','o'),
  /*7.0*/ NABATAEAN			= SCRIPT_TAG ('N','b','a','t'),
  /*7.0*/ OLD_NORTH_ARABIAN		= SCRIPT_TAG ('N','a','r','b'),
  /*7.0*/ OLD_PERMIC			= SCRIPT_TAG ('P','e','r','m'),
  /*7.0*/ PAHAWH_HMONG		= SCRIPT_TAG ('H','m','n','g'),
  /*7.0*/ PALMYRENE			= SCRIPT_TAG ('P','a','l','m'),
  /*7.0*/ PAU_CIN_HAU			= SCRIPT_TAG ('P','a','u','c'),
  /*7.0*/ PSALTER_PAHLAVI		= SCRIPT_TAG ('P','h','l','p'),
  /*7.0*/ SIDDHAM			= SCRIPT_TAG ('S','i','d','d'),
  /*7.0*/ TIRHUTA			= SCRIPT_TAG ('T','i','r','h'),
  /*7.0*/ WARANG_CITI			= SCRIPT_TAG ('W','a','r','a'),

  /*8.0*/ AHOM			= SCRIPT_TAG ('A','h','o','m'),
  /*8.0*/ ANATOLIAN_HIEROGLYPHS	= SCRIPT_TAG ('H','l','u','w'),
  /*8.0*/ HATRAN			= SCRIPT_TAG ('H','a','t','r'),
  /*8.0*/ MULTANI			= SCRIPT_TAG ('M','u','l','t'),
  /*8.0*/ OLD_HUNGARIAN		= SCRIPT_TAG ('H','u','n','g'),
  /*8.0*/ SIGNWRITING			= SCRIPT_TAG ('S','g','n','w'),

  /*
   * Since 1.3.0
   */
  /*9.0*/ ADLAM			= SCRIPT_TAG ('A','d','l','m'),
  /*9.0*/ BHAIKSUKI			= SCRIPT_TAG ('B','h','k','s'),
  /*9.0*/ MARCHEN			= SCRIPT_TAG ('M','a','r','c'),
  /*9.0*/ OSAGE			= SCRIPT_TAG ('O','s','g','e'),
  /*9.0*/ TANGUT			= SCRIPT_TAG ('T','a','n','g'),
  /*9.0*/ NEWA			= SCRIPT_TAG ('N','e','w','a'),

  /*
   * Since 1.6.0
   */
  /*10.0*/MASARAM_GONDI		= SCRIPT_TAG ('G','o','n','m'),
  /*10.0*/NUSHU			= SCRIPT_TAG ('N','s','h','u'),
  /*10.0*/SOYOMBO			= SCRIPT_TAG ('S','o','y','o'),
  /*10.0*/ZANABAZAR_SQUARE		= SCRIPT_TAG ('Z','a','n','b'),

  /*
   * Since 1.8.0
   */
  /*11.0*/DOGRA			= SCRIPT_TAG ('D','o','g','r'),
  /*11.0*/GUNJALA_GONDI		= SCRIPT_TAG ('G','o','n','g'),
  /*11.0*/HANIFI_ROHINGYA		= SCRIPT_TAG ('R','o','h','g'),
  /*11.0*/MAKASAR			= SCRIPT_TAG ('M','a','k','a'),
  /*11.0*/MEDEFAIDRIN			= SCRIPT_TAG ('M','e','d','f'),
  /*11.0*/OLD_SOGDIAN			= SCRIPT_TAG ('S','o','g','o'),
  /*11.0*/SOGDIAN			= SCRIPT_TAG ('S','o','g','d'),

  /* No script set. */
  INVALID				= SCRIPT_TAG(0,0,0,0),

  /* Dummy values to ensure any SCRIPT_TAG_t value can be passed/stored as t
   * without risking undefined behavior.  We have two, for historical reasons.
   * SCRIPT_TAG_MAX used to be unsigned, but that was invalid Ansi C, so was changed
   * to _MAX_VALUE to be equal to SCRIPT_TAG_MAX_SIGNED as well.
   *
   * See this thread for technicalities:
   *
   *   https://lists.freedesktop.org/archives/harfbuzz/2014-March/004150.html
   */
  MAX_VALUE				= SCRIPT_TAG(0x7f,0xff,0xff,0xff), /*< skip >*/
  MAX_VALUE_SIGNED			= SCRIPT_TAG(0x7f,0xff,0xff,0xff) /*< skip >*/

};

} } // namespace cinder::text
