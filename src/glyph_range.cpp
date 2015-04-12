#include "glyph_range.hpp"

#include "ft_glyph.hpp"
#include "glyph_storage.hpp"

#include "thr/dispatch.hpp"

/** Crunch one glyph.
 *
 * \param storage Glyph storage.
 * \param gly Glyph to crunch.
 */
static void crunch_glyph(GlyphStorage &storage, FtGlyph* gly)
{
  gly->crunch();

  storage.add(gly);
}

GlyphRange::GlyphRange(unsigned ps, unsigned pe) :
  m_enabled(false)
{
  this->add(ps, pe);
}

void GlyphRange::add(unsigned ps, unsigned pe)
{
  if(ps > pe)
  {
    std::swap(ps, pe);
  }

  for(unsigned ii = ps; (ii <= pe); ++ii)
  {
    m_range.push_back(ii);
  }

  this->sort();
}

void GlyphRange::remove(unsigned op)
{
  for(std::vector<unsigned>::iterator ii = m_range.begin(), ee = m_range.end(); (ii != ee); ++ii)
  {
    if((*ii) == op)
    {
      m_range.erase(ii);
      return;
    }
  }
}

void GlyphRange::remove(unsigned ps, unsigned pe)
{
  if(ps > pe)
  {
    std::swap(ps, pe);
  }

  for(std::vector<unsigned>::iterator ii = m_range.begin(), ee = m_range.end(); (ii != ee); ++ii)
  {
    if((*ii) >= ps)
    {
      for(std::vector<unsigned>::iterator jj = ii; (jj != ee);)
      {
        std::vector<unsigned>::iterator kk = jj;
        ++kk;

        if(pe < (*kk))
        {
          m_range.erase(ii, jj);
          return;
        }
        jj = kk;
      }
      m_range.erase(ii, ee);
      return;
    }
  }    
}

unsigned GlyphRange::queue(GlyphStorage &storage, std::list<FtFaceSptr> &src, unsigned target_size) const
{
  if(!m_enabled)
  {
    return false;
  }

  unsigned ret = 0;

  BOOST_FOREACH(unsigned gidx, m_range)
  {
    FtGlyph *gly = NULL;

    if(storage.markGlyph(gidx))
    {
      BOOST_FOREACH(FtFaceSptr &ii, src)
      {
        gly = ii->renderGlyph(gidx, target_size);
        if(NULL != gly)
        {
          storage.concurrencyIcrement();
          thr::dispatch(crunch_glyph, boost::ref(storage), gly);
          ++ret;
          break;
        }
      }
    }

    if(NULL == gly)
    {
      storage.missing(gidx);
    }
  }

  return ret;
}

void GlyphRange::sort()
{
  std::sort(m_range.begin(), m_range.end());
}

