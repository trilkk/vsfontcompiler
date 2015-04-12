#include "sky_line.hpp"

#include "gfx/image_png.hpp"
#include "glyph_storage.hpp"

#include "math/generic.hpp"

SkyLine::SkyLine(unsigned pw, unsigned pmaxh) :
  m_bitmap(NULL),
  m_width(pw),
  m_max_height(pmaxh),
  m_wasted(0)
{
  m_line = new unsigned[m_width];

  memset(m_line, 0, sizeof(unsigned) * m_width);
}

SkyLine::~SkyLine()
{
  delete[] m_bitmap;
  delete[] m_line;
}

void SkyLine::allocate(const SkyLineLocation &op)
{
  unsigned end_height = op.getY() + op.getHeight();

  for(unsigned ii = op.getX(), ee = op.getX() + op.getWidth(); (ii < ee); ++ii)
  {
    BOOST_ASSERT(op.getY() >= m_line[ii]);

    m_line[ii] = end_height;
  }

  m_wasted += op.getWasted();
}

SkyLineLocation SkyLine::fit(const FtGlyph &op)
{
  unsigned bitmap_w = op.getCrunchedWidth(),
           bitmap_h = op.getCrunchedHeight();

  if((0 >= bitmap_w) || (0 >= bitmap_h))
  {
    return SkyLineLocation(0, 0, 0, 0);
  }

  unsigned minh = UINT_MAX,
           maxh = 0;

  // Find out minimum and maximum heights.
  for(unsigned ii = 0; (ii < m_width); ++ii)
  {
    unsigned current_height = m_line[ii];

    minh = std::min(minh, current_height);
    maxh = std::max(maxh, current_height);
  }

  // No need to try to insert beyond limits.
  maxh = std::min(maxh, m_max_height - bitmap_h);

  // Fit starting from minimum height.
  for(unsigned ii = minh; (ii <= maxh); ++ii)
  {
    for(unsigned jj = 0; (jj < m_width); ++jj)
    {
      unsigned current_height = m_line[jj];

      if(current_height == ii)
      {
        int width_i = static_cast<int>(bitmap_w);
        int iter_i = static_cast<int>(jj);
        int kk = std::max(iter_i - width_i + 1, 0);
        int ee = std::min(iter_i, static_cast<int>(m_width) - width_i);

        while(kk <= ee)
        {
          unsigned fitting_pixels = 0;

          while(m_line[kk + static_cast<int>(fitting_pixels)] <= current_height)
          {
            ++fitting_pixels;

            // location found
            if(fitting_pixels >= bitmap_w)
            {
              SkyLineLocation ret(static_cast<unsigned>(kk), current_height, bitmap_w, bitmap_h);

              ret.setWasted(this->getWastedSpace(ret));

              return ret;
            }
          }

          ++kk;
        }
      }
    }
  }

  return SkyLineLocation();
}

unsigned SkyLine::fitAll(GlyphStorage &glyphs, FILE *xmlfile, unsigned pidx, bool glst)
{
  GlyphStorage::iterator ii, ee;
  unsigned ret = 0;

  for(ii = glyphs.begin(), ee = glyphs.end(); (ii != ee); ++ii)
  {
    FtGlyph *gly = ii->get();
    SkyLineLocation loc = this->fit(*gly);

    if(!loc.isValid())
    {
      break;
    }

    this->allocate(loc);

    if(NULL != xmlfile)
    {
      this->insert(loc, *gly);

      gly->setPage(pidx);
      gly->write(xmlfile, glst);

      *ii = FtGlyphSptr();
    }

    ++ret;
  }

  return ret;
}

float SkyLine::getUsage() const
{
  unsigned used_height = this->getUsedHeight();
  unsigned wasted = m_wasted;

  for(unsigned ii = 0; (ii < m_width); ++ii)
  {
    wasted += used_height - m_line[ii];
  }

  if(0 >= used_height)
  {
    return 0.0f;
  }
  
  return 1.0f - static_cast<float>(wasted) / static_cast<float>(m_width * used_height);
}

unsigned SkyLine::getUsedHeight() const
{
  unsigned ret = 0;

  for(unsigned ii = 0; (ii < m_width); ++ii)
  {
    ret = math::max(m_line[ii], ret);
  }

  unsigned remainder = ret % SIZE_STEP;

  return (0 < remainder) ? (ret - remainder + SIZE_STEP) : ret;
}

unsigned SkyLine::getWastedSpace(const SkyLineLocation &op) const
{
  unsigned ret = 0;

  for(unsigned ii = op.getX(), ee = op.getX() + op.getWidth(); (ii < ee); ++ii)
  {
    unsigned current_height = m_line[ii];

    BOOST_ASSERT(op.getY() >= current_height);

    ret += op.getY() - current_height;
  }

  return ret;
}

void SkyLine::insert(const SkyLineLocation &loc, FtGlyph &op)
{
  // whitespace character
  if((0 == loc.getWidth()) || (0 == loc.getHeight()))
  {
    BOOST_ASSERT(0 >= op.getCrunchedWidth());
    BOOST_ASSERT(0 >= op.getCrunchedHeight());
    return;
  }

  if(NULL == m_bitmap)
  {
    unsigned bitmap_size = m_width * m_max_height;

    m_bitmap = new uint8_t[bitmap_size];

    memset(m_bitmap, 0, bitmap_size);
  }

  // The final image is arranged scanlines from bottom to top, since it written to disk. On the other hand,
  // crunched images are arranged like their FreeType -rendered counterparts, from top to bottom.
  unsigned scanline_width = loc.getWidth();
  uint8_t *dst = m_bitmap + (loc.getY() * m_width) + loc.getX();
  uint8_t *src = op.getCrunched() + ((loc.getHeight() - 1) * scanline_width);

  BOOST_ASSERT((op.getCrunchedWidth() == scanline_width) &&
      (op.getCrunchedHeight() == loc.getHeight()));

  for(unsigned ii = 0; (ii < loc.getHeight()); ++ii)
  {
    memcpy(dst, src, scanline_width);

    dst += m_width;
    src -= scanline_width;
  }

  float fw = static_cast<float>(m_width),
        fh = static_cast<float>(m_max_height),
        s1 = static_cast<float>(loc.getX()) / fw,
        t1 = static_cast<float>(loc.getY()) / fh,
        s2 = s1 + (static_cast<float>(loc.getWidth()) / fw),
        t2 = t1 + (static_cast<float>(loc.getHeight()) / fh);

  op.setST(s1, t1, s2, t2);
}

void SkyLine::save(const boost::filesystem::path &op)
{
  gfx::image_png_save(op.generic_string(), m_width, m_max_height, 8, m_bitmap);
}

