#include "sky_line_fitter.hpp"

#include "glyph_storage.hpp"
#include "sky_line.hpp"

#include "thr/dispatch.hpp"

extern bool g_verbose;

SkyLineFitter::SkyLineFitter(unsigned pmax) :
  m_max_size(pmax),
  m_best_count(0),
  m_best_usage(0.0f),
  m_best_width(0),
  m_best_height(0),
  m_last_print_width(0) { }

void SkyLineFitter::queue(GlyphStorage &glyphs)
{
  // round down to the next step
  m_max_size -= m_max_size % SkyLine::SIZE_STEP;

  for(unsigned ii = m_max_size; (0 < ii); ii -= SkyLine::SIZE_STEP)
  {
    thr::dispatch(attempt_thread, boost::ref(*this), boost::ref(glyphs), ii, m_max_size);
  }
}

void SkyLineFitter::storeAttempt(unsigned pcount, float pusage, unsigned pw, unsigned ph)
{
  boost::mutex::scoped_lock scope(m_mutex);

  if((pcount >= m_best_count) && (pusage > m_best_usage))
  {
    m_best_count = pcount;
    m_best_usage = pusage;
    m_best_width = pw;
    m_best_height = ph;

    if(g_verbose)
    {
      std::ostringstream sstr;
      sstr << "\rBest: " << m_best_count << " / " << m_best_usage << " (" <<
        m_best_width << 'x' << m_best_height << ')';
      unsigned current_width = static_cast<unsigned>(sstr.str().length());
      for(unsigned ii = current_width; (ii < m_last_print_width); ++ii)
      {
        sstr << ' ';
      }
      m_last_print_width = current_width;
      std::cout << sstr.str() << std::flush;
    }
  }
}

void SkyLineFitter::attempt_thread(SkyLineFitter &slf, GlyphStorage &glyphs, unsigned pw, unsigned pmaxh)
{
  SkyLine sl(pw, pmaxh);
  unsigned count = sl.fitAll(glyphs);

  slf.storeAttempt(count, sl.getUsage(), pw, sl.getUsedHeight());
 }

