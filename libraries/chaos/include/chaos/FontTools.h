#pragma once

#include <chaos/StandardHeaders.h>

namespace chaos
{
  /**
   * FontTools : provide some static methods to manipulate tools
   */

  class FontTools
  {
  public:


    /** Metrics of a character */
    class CharacterMetrics
    {
    public:

      /** default constructor */
      CharacterMetrics():
        advance({0, 0}),
        width(0),
        height(0),
        bitmap_left(0),
        bitmap_top(0){}

      /** constructor with initialization */
      CharacterMetrics(FT_BitmapGlyph in_bitmap_glyph):
        advance({in_bitmap_glyph->root.advance.x >> 16, in_bitmap_glyph->root.advance.y >> 16}),
        width(in_bitmap_glyph->bitmap.width),
        height(in_bitmap_glyph->bitmap.rows),
        bitmap_left(in_bitmap_glyph->left),
        bitmap_top(in_bitmap_glyph->top){}

      FT_Vector advance;      
      int       width;
      int       height;
      int       bitmap_left;
      int       bitmap_top;        
    };

    /** Bitmap + Metrics of a character */
    class CharacterBitmapGlyph : public CharacterMetrics
    {
    public:

      /** default constructor */
      CharacterBitmapGlyph(): 
        bitmap_glyph(nullptr){}

      /** constructor with initialization */
      CharacterBitmapGlyph(FT_BitmapGlyph in_bitmap_glyph):
        CharacterMetrics(in_bitmap_glyph),
        bitmap_glyph(in_bitmap_glyph){}

      /** the glyph */
      FT_BitmapGlyph bitmap_glyph;
    };

    /** generate a bitmap from a glyph slot */
    static FIBITMAP * GenerateImage(FT_GlyphSlot glyph, int bpp = 8);
    /** generate a bitmap from a bitmap */
    static FIBITMAP * GenerateImage(FT_Bitmap & bitmap, int bpp = 8);
    /** generate a bitmap from a font an a string */
    static FIBITMAP * GenerateImage(FT_Face face, char const * str, int bpp = 8);
    /** generate a bitmap from a font an a character */
    static FIBITMAP * GenerateImage(FT_Face face, char c, int bpp = 8);
    /** generate a bitmap glyph from a character */
    static FT_BitmapGlyph GetBitmapGlyph(FT_Face face, char c, bool accept_notfound_glyph); 
    /** generate a cache with all glyph required for a string */
    static std::map<char, CharacterBitmapGlyph> GetGlyphCacheForString(FT_Face face, char const * str);
  };
};
