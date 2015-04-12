#include "ft_glyph.hpp"

#include "ft_library.hpp"
#include "math/generic.hpp"

#include <sstream>

/** Use manhattan distance instead of actual distance. */
#define USE_MANHATTAN 1

/** \brief Distance between two coordinates.
 *
 * \param x1 First X coordinate.
 * \param y1 First Y coordinate.
 * \param x2 Second X coordinate.
 * \param y2 Second Y coordinate.
 * \return Distance as integer.
 */
static float fdist(int x1, int y1, int x2, int y2)
{
  int dx = x2 - x1,
      dy = y2 - y1;
#if defined(USE_MANHATTAN)
  return static_cast<float>(std::abs(dx) + std::abs(dy));
#else
  return sqrtf(static_cast<float>(dx * dx + dy * dy));
#endif
}

/** \brief Return the value at a given location in a freetype glyph.
 *
 * Will return 0 if the point is 'outside' the bitmap.
 *
 * \param glyph Glyph to examine.
 * \param px X coordinate.
 * \param py Y coordinate.
 * \param size Big size.
 * \return Value found.
 */
static bool get_ftbitmap_value(const FT_Bitmap *bitmap, int px, int py)
{
  if((px < 0) || (py < 0))
  {
    return 0;
  }

  unsigned ux = static_cast<unsigned>(px);
  unsigned uy = static_cast<unsigned>(py);

  if((ux >= bitmap->width) || (uy >= bitmap->rows))
  {
    return 0;
  }

  return (127 < bitmap->buffer[uy * bitmap->width + ux]);
}

/** \brief Return the distance field value of a coordinate.
 *
 * Ineffective. Don't care.
 *
 * \param glyph Glyph to examine.
 * \param px X coordinate.
 * \param py Y coordinate.
 * \param dist_scale Scale for distances in bitmap.
 * \return Depth field value.
 */
static uint8_t get_ftbitmap_dfield_value(const FT_Bitmap *bitmap, int px, int py, int search, float dist_scale)
{
  float closest = FLT_MAX;
  float ret;

  if(get_ftbitmap_value(bitmap, px, py))
  {
    for(int ii = px - search; (ii <= px + search); ++ii)
    {
      for(int jj = py - search; (jj <= py + search); ++jj)
      {
        float dist = fdist(ii, jj, px, py);
        if(dist < closest)
        {
          if(!get_ftbitmap_value(bitmap, ii, jj))
          {
            closest = dist;
          }
        }
      }
    }
    ret = std::min(0.5f + (closest + 0.5f) * dist_scale, 1.0f);
  }
  else
  {
    for(int ii = px - search; (ii <= px + search); ++ii)
    {
      for(int jj = py - search; (jj <= py + search); ++jj)
      {
        float dist = fdist(ii, jj, px, py);
        if(dist < closest)
        {
          if(get_ftbitmap_value(bitmap, ii, jj))
          {
            closest = dist;
          }
        }
      }
    }
    ret = std::max(0.5f - (closest + 0.5f) * dist_scale, 0.0f);
  }

  return static_cast<uint8_t>(math::lround(ret * 255.0f));
}

FtGlyph::FtGlyph(unsigned pcode, FT_Bitmap *bitmap, unsigned psize, unsigned ptarget, float pdropdown,
    float pleft, float ptop, float pax, float pay) :
  m_unicode(pcode),
  m_crunched(NULL),
  m_size(psize),
  m_target_size(ptarget),
  m_dropdown(pdropdown),
  m_width(static_cast<float>(bitmap->width)),
  m_height(static_cast<float>(bitmap->rows)),
  m_left(pleft),
  m_top(ptop),
  m_advance_x(pax),
  m_advance_y(pay),
  m_bitmap_w(ptarget * 2 + 1),
  m_bitmap_h(ptarget * 2 + 1),
  m_x1(0.0f),
  m_y1(0.0f),
  m_x2(0.0f),
  m_y2(0.0f),
  m_s1(0.0f),
  m_t1(0.0f),
  m_s2(0.0f),
  m_t2(0.0f)
{
  FT_Bitmap_New(&m_bitmap);
  
  FT_Error err = FT_Bitmap_Copy(FtLibrary::get(), bitmap, &m_bitmap);
  if(0 != err)
  {
    std::ostringstream sstr;
    sstr << "could not copy rendered FreeType bitmap for character " << m_unicode;
    BOOST_THROW_EXCEPTION(std::runtime_error(sstr.str()));
  }
}

FtGlyph::~FtGlyph()
{
  // May have to destroy the large bitmap.
  if(NULL == m_crunched)
  {
    FT_Bitmap_Done(FtLibrary::get(), &m_bitmap);
  }
  delete[] m_crunched;
}

unsigned FtGlyph::contractDown()
{
  unsigned empty_rows = 0;

  for(unsigned ii = 0; (ii < m_bitmap_h); ++ii)
  {
    if(!this->isEmptyRow(m_bitmap_h - ii - 1))
    {
      break;
    }
    ++empty_rows;
  }

  if(empty_rows >= m_bitmap_h)
  {
    delete[] m_crunched;

    this->m_bitmap_w = this->m_bitmap_h = 0;
    this->m_crunched = NULL;

    return empty_rows;
  }

  if(1 < empty_rows)
  {
    this->subCrunched(0, 0, m_bitmap_w, m_bitmap_h - empty_rows + 1);

    return empty_rows - 1;
  }
  return 0;
}

unsigned FtGlyph::contractLeft()
{
  unsigned empty_columns = 0;

  for(unsigned ii = 0; (ii < m_bitmap_w); ++ii)
  {
    if(!this->isEmptyColumn(ii))
    {
      break;
    }
    ++empty_columns;
  }

  if(empty_columns >= m_bitmap_w)
  {
    delete[] m_crunched;

    this->m_bitmap_w = this->m_bitmap_h = 0;
    this->m_crunched = NULL;

    return empty_columns;
  }

  if(1 < empty_columns)
  {
    this->subCrunched(empty_columns - 1, 0, m_bitmap_w - empty_columns + 1, m_bitmap_h);

    return empty_columns - 1;
  }
  return 0;
}

unsigned FtGlyph::contractRight()
{
  unsigned empty_columns = 0;

  for(unsigned ii = 0; (ii < m_bitmap_w); ++ii)
  {
    if(!this->isEmptyColumn(m_bitmap_w - ii - 1))
    {
      break;
    }
    ++empty_columns;
  }

  if(empty_columns >= m_bitmap_w)
  {
    delete[] m_crunched;

    this->m_bitmap_w = this->m_bitmap_h = 0;
    this->m_crunched = NULL;

    return empty_columns;
  }

  if(1 < empty_columns)
  {
    this->subCrunched(0, 0, m_bitmap_w - empty_columns + 1, m_bitmap_h);

    return empty_columns - 1;
  }
  return 0;
}

unsigned FtGlyph::contractUp()
{
  unsigned empty_rows = 0;

  for(unsigned ii = 0; (ii < m_bitmap_h); ++ii)
  {
    if(!this->isEmptyRow(ii))
    {
      break;
    }
    ++empty_rows;
  }

  if(empty_rows >= m_bitmap_h)
  {
    delete[] m_crunched;

    this->m_bitmap_w = this->m_bitmap_h = 0;
    this->m_crunched = NULL;

    return empty_rows;
  }

  if(1 < empty_rows)
  {
    this->subCrunched(0, empty_rows - 1, m_bitmap_w, m_bitmap_h - empty_rows + 1);

    return empty_rows - 1;
  }
  return 0;
}

void FtGlyph::copy(uint8_t *tgt, unsigned tw, unsigned th, unsigned idx)
{
  unsigned div = tw / m_bitmap_w,
           row = idx / div,
           col = idx % div,
           linesize = m_bitmap_w * sizeof(uint8_t);

  // Image in memory is in OpenGL row order.
  tgt += (th - 1 - row * m_bitmap_h) * tw + col * m_bitmap_w;

  for(unsigned ii = 0; (ii < m_bitmap_h); ++ii)
  {
    memcpy(tgt - ii * tw, m_crunched + ii * m_bitmap_w, linesize);
  }
}

void FtGlyph::crunch()
{
  if(NULL == m_crunched)
  {
    float fsize = static_cast<float>(m_size);
    float ftarget = static_cast<float>(m_target_size);
    float dist_scale(0.5f / (fsize * m_dropdown));
    float step = fsize / ftarget;
    float pixel_scale = 1.0f / ftarget;
    int search = static_cast<int>(math::ceil(fsize * m_dropdown));
    int ox = m_bitmap.width / 2;
    int oy = m_bitmap.rows / 2;
    unsigned bitmap_down = 0;
    unsigned bitmap_left = 0;
    unsigned bitmap_right = 0;
    unsigned bitmap_scope_horiz = 1;
    unsigned bitmap_scope_vert = 1;
    unsigned bitmap_up = 0;
    unsigned horiz_expand = static_cast<unsigned>(math::ceil(static_cast<float>(ox) / step));
    unsigned vert_expand = static_cast<unsigned>(math::ceil(static_cast<float>(oy) / step));
    float left = (m_left + static_cast<float>(ox)) / fsize - (static_cast<float>(m_bitmap_w) * 0.5f) / ftarget;
    float top = (m_top - static_cast<float>(oy)) / fsize + (static_cast<float>(m_bitmap_h) * 0.5f) / ftarget;
    bool down_done = false;
    bool left_done = false;
    bool right_done = false;
    bool up_done = false;
    bool done = false;
    
    // reserve 'enough' space for the crunched bitmap, then initialize the central point
    m_crunched = new uint8_t[m_bitmap_w * m_bitmap_h];
    memset(m_crunched, 0, m_bitmap_w * m_bitmap_h);
    {
      uint8_t dfval = get_ftbitmap_dfield_value(&m_bitmap, ox, oy, search, dist_scale);
      m_crunched[m_target_size * m_bitmap_w + m_target_size] = dfval;
    }

    // Expansion.
    for(unsigned expansion = 0; ((expansion < horiz_expand) || (expansion < vert_expand) || !done); ++expansion)
    {
      done = true;

      if(!down_done || (expansion < vert_expand))
      {
        down_done = true;

        ++bitmap_down;
        ++bitmap_scope_vert;

        uint8_t *iter = m_crunched + (m_target_size + bitmap_down) * m_bitmap_w + m_target_size - bitmap_left;

        for(unsigned ii = 0; (ii < bitmap_scope_horiz); ++ii)
        {
          uint8_t dfval = get_ftbitmap_dfield_value(&m_bitmap,
              math::lround(static_cast<float>(ii - bitmap_left) * step) + ox,
              math::lround(static_cast<float>(bitmap_down) * step) + oy,
              search, dist_scale);

          if(0 < dfval)
          {
            if(0 >= ii)
            {
              left_done = false;
            }
            else if(bitmap_scope_horiz - 1 <= ii)
            {
              right_done = false;
            }
            down_done = false;
            done = false;
          }

          *iter = dfval;
          ++iter;
        }
      }
      if(!left_done || (expansion < horiz_expand))
      {
        left_done = true;

        ++bitmap_left;
        ++bitmap_scope_horiz;

        uint8_t *iter = m_crunched + (m_target_size - bitmap_up) * m_bitmap_w + m_target_size - bitmap_left;

        for(unsigned ii = 0; (ii < bitmap_scope_vert); ++ii)
        {
          uint8_t dfval = get_ftbitmap_dfield_value(&m_bitmap,
              math::lround(static_cast<float>(-bitmap_left) * step) + ox,
              math::lround(static_cast<float>(ii - bitmap_up) * step) + oy,
              search, dist_scale);

          if(0 < dfval)
          {
            if(0 >= ii)
            {
              up_done = false;
            }
            else if(bitmap_scope_vert - 1 <= ii)
            {
              down_done = false;
            }
            left_done = false;
            done = false;
          }

          *iter = dfval;
          iter += m_bitmap_w;
        }
      }
      if(!right_done || (expansion < horiz_expand))
      {
        right_done = true;

        ++bitmap_right;
        ++bitmap_scope_horiz;

        uint8_t *iter = m_crunched + (m_target_size - bitmap_up) * m_bitmap_w + m_target_size + bitmap_right;

        for(unsigned ii = 0; (ii < bitmap_scope_vert); ++ii)
        {
          uint8_t dfval = get_ftbitmap_dfield_value(&m_bitmap,
              math::lround(static_cast<float>(bitmap_right) * step) + ox,
              math::lround(static_cast<float>(ii - bitmap_up) * step) + oy,
              search, dist_scale);

          if(0 < dfval)
          {
            if(0 >= ii)
            {
              up_done = false;
            }
            else if(bitmap_scope_vert - 1 <= ii)
            {
              down_done = false;
            }
            right_done = false;
            done = false;
          }

          *iter = dfval;
          iter += m_bitmap_w;
        }
      }
      if(!up_done || (expansion < vert_expand))
      {
        up_done = true;

        ++bitmap_up;
        ++bitmap_scope_vert;

        uint8_t *iter = m_crunched + (m_target_size - bitmap_up) * m_bitmap_w + m_target_size - bitmap_left;

        for(unsigned ii = 0; (ii < bitmap_scope_horiz); ++ii)
        {
          uint8_t dfval = get_ftbitmap_dfield_value(&m_bitmap,
              math::lround(static_cast<float>(ii - bitmap_left) * step) + ox,
              math::lround(static_cast<float>(-bitmap_up) * step) + oy,
              search, dist_scale);

          if(0 < dfval)
          {
            if(0 >= ii)
            {
              left_done = false;
            }
            else if(bitmap_scope_horiz - 1 <= ii)
            {
              right_done = false;
            }
            up_done = false;
            done = false;
          }

          *iter = dfval;
          ++iter;
        }
      }
    }

    // Represent glyph absolute metrics in units of font size.
    m_width /= fsize;
    m_height /= fsize;
    m_left /= fsize;
    m_top /= fsize;

    // The divisions by 64 are due to the advance values being expressed as 1/64ths of a pixel.
    m_advance_x /= fsize * 64.0f;
    m_advance_y /= fsize * 64.0f;

    // Actual glyph quad coordinates.
    left += static_cast<float>(this->contractLeft()) * pixel_scale;
    top -= static_cast<float>(this->contractUp()) * pixel_scale;

    this->contractRight();
    this->contractDown();

    float fwidth = static_cast<float>(m_bitmap_w) / static_cast<float>(m_target_size);
    float fheight = static_cast<float>(m_bitmap_h) / static_cast<float>(m_target_size);

    m_x1 = left;
    m_y1 = top - fheight;
    m_x2 = left + fwidth;
    m_y2 = top;

    // Large bitmap no longer needed.
    FT_Bitmap_Done(FtLibrary::get(), &m_bitmap);
  }
}

bool FtGlyph::isEmptyColumn(unsigned op)
{
  BOOST_ASSERT(op < m_bitmap_w);

  for(unsigned ii = 0; (ii < m_bitmap_h); ++ii)
  {
    if(0 < m_crunched[ii * m_bitmap_w + op])
    {
      return false;
    }
  }
  return true;
}

bool FtGlyph::isEmptyRow(unsigned op)
{
  BOOST_ASSERT(op < m_bitmap_h);

  for(unsigned ii = 0; (ii < m_bitmap_w); ++ii)
  {
    if(0 < m_crunched[op * m_bitmap_w + ii])
    {
      return false;
    }
  }
  return true;
}

void FtGlyph::subCrunched(unsigned px, unsigned py, unsigned pw, unsigned ph)
{
  BOOST_ASSERT(0 < pw);
  BOOST_ASSERT(0 < ph);
  BOOST_ASSERT(px + pw <= m_bitmap_w);
  BOOST_ASSERT(py + ph <= m_bitmap_h);

  uint8_t *new_crunched = new uint8_t[pw * ph];

  for(unsigned jj = 0; (jj < ph); ++jj)
  {
    for(unsigned ii = 0; (ii < pw); ++ii)
    {
      new_crunched[jj * pw + ii] = m_crunched[(jj + py) * m_bitmap_w + ii + px];
    }
  }

  delete[] m_crunched;

  m_bitmap_w = pw;
  m_bitmap_h = ph;
  m_crunched = new_crunched;
}

void FtGlyph::write(FILE *fptr, bool glst)
{
  std::stringstream sstream;

  sstream << "\t<glyph>\n" <<
    "\t\t<code>" << m_unicode << "</code>\n" <<
    "\t\t<width>" << m_width << "</width>\n" <<
    "\t\t<height>" << m_height << "</height>\n" <<
    "\t\t<left>" << m_left << "</left>\n" <<
    "\t\t<top>" << m_top << "</top>\n" <<
    "\t\t<advance_x>" << m_advance_x << "</advance_x>\n" <<
    "\t\t<advance_y>" << m_advance_y << "</advance_y>\n" <<
    "\t\t<x1>" << m_x1 << "</x1>\n" <<
    "\t\t<y1>" << m_y1 << "</y1>\n" <<
    "\t\t<x2>" << m_x2 << "</x2>\n" <<
    "\t\t<y2>" << m_y2 << "</y2>\n" <<
    "\t\t<s1>" << m_s1 << "</s1>\n" <<
    "\t\t<t1>" << (glst ? m_t1 : (1.0f - m_t1)) << "</t1>\n" <<
    "\t\t<s2>" << m_s2 << "</s2>\n" <<
    "\t\t<t2>" << (glst ? m_t2 : (1.0f - m_t2)) << "</t2>\n" <<
    "\t\t<page>" << m_page << "</page>\n" <<
    "\t</glyph>\n";

  fputs(sstream.str().c_str(), fptr);
}

std::ostream& operator<<(std::ostream &lhs, const FtGlyph &rhs)
{
  for(unsigned jj = 0; (jj < rhs.m_bitmap_h); ++jj)
  {
    for(unsigned ii = 0; (ii < rhs.m_bitmap_w); ++ii)
    {
      int input = static_cast<int>(rhs.m_crunched[jj * rhs.m_bitmap_w + ii]);
      char cc = ' ';

      if(0 < input)
      {
        cc = '.';
      }
      if(math::lround(127.5 - 12.25f) < input)
      {
        cc = 'X';
      }
      if(math::lround(127.5 + 12.25f) < input)
      {
        cc = '#';
      }
      lhs << cc;
    }
    lhs << std::endl;
  }
  return lhs << "Unicode: " << rhs.m_unicode << " Size: " << rhs.m_width << " x " << rhs.m_height <<
    std::endl << "Left/Top: " << rhs.m_left << " / " << rhs.m_top << " Advance: " << rhs.m_advance_x <<
    " / " << rhs.m_advance_y << std::endl;
}
