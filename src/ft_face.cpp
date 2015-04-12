#include "ft_face.hpp"

#include "ft_glyph.hpp"
#include "ft_library.hpp"
#include "math/generic.hpp"

#include <sstream>

FtFace::FtFace(const std::string &filename, unsigned psize, float pdropdown) :
  m_face(NULL),
  m_size(psize),
  m_dropdown(pdropdown)
{
  if(FT_New_Face(FtLibrary::get(), filename.c_str(), 0, &(m_face)))
  {
    std::stringstream err;
    err << "could not load font: " << filename;
    BOOST_THROW_EXCEPTION(std::runtime_error(err.str()));
  }
  if(FT_Set_Pixel_Sizes(m_face, 0, m_size))
  {
    std::stringstream sstream;
    sstream << "could not set font size to " << m_size;
    throw sstream.str();
  }
}

FtFace::~FtFace()
{
  if(NULL != m_face)
  {
    FT_Done_Face(m_face);
  }
}

bool FtFace::hasGlyph(unsigned unicode)
{
  return (FT_Get_Char_Index(m_face, unicode) > 0);
}

FtGlyph* FtFace::renderGlyph(unsigned unicode, unsigned targetsize)
{
  unsigned idx = FT_Get_Char_Index(m_face, unicode);

  if(0 == idx)
  {
    //std::cerr << "could not find character of index " << unicode << std::endl;
    return NULL;
  }

  if(FT_Load_Glyph(m_face, idx, FT_LOAD_DEFAULT))
  {
    //std::cerr << "could not load glyph " << unicode << std::endl;
    return NULL;
  }

  FT_GlyphSlot glyph = m_face->glyph;
  if(glyph->format != FT_GLYPH_FORMAT_BITMAP)
  {
    FT_Error err = FT_Render_Glyph(glyph, FT_RENDER_MODE_NORMAL);

    if(0 != err)
    {
      //std::cerr << "could not render glyph: " << unicode << std::endl;
      return NULL;
    }
  }

  return new FtGlyph(unicode, &glyph->bitmap, m_size, targetsize, m_dropdown,
      static_cast<float>(glyph->bitmap_left), static_cast<float>(glyph->bitmap_top),
      static_cast<float>(glyph->advance.x), static_cast<float>(glyph->advance.y));
}

