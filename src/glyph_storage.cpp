#include "glyph_storage.hpp"

#include "thr/generic.hpp"

extern bool g_verbose;

/** \brief Compare two contained glyphs.
 *
 * \param lhs Left-hand-side operand.
 * \param rhs Right-hand-side operand.
 * \return True if lhs < rhs.
 */
static bool ft_glyph_sptr_less(const FtGlyphSptr &lhs, const FtGlyphSptr &rhs)
{
  // Existing glyph alvays goes to the left of nonexisting glyph.
  if(NULL == lhs.get())
  {
    return false;
  }
  else if(NULL == rhs.get())
  {
    return true;
  }

  unsigned lw = lhs->getCrunchedHeight(),
           rw = rhs->getCrunchedHeight();

  // Notice that even though this is 'less', we're actually sorting biggest-first.
  if(lw > rw)
  {
    return true;
  }
  else if(lw == rw)
  {
    if(lhs->getCrunchedWidth() > rhs->getCrunchedWidth())
    {
      return true;
    }
  }
  return false;
}

GlyphStorage::GlyphStorage() :
  m_glyphs_in_flight(0),
  m_failure_pending(false)
{
  // At least one glyph waiting for every thread at practically all times.
  m_concurrency = thr::hardware_concurrency() * 2;
}

void GlyphStorage::add(FtGlyph *op)
{
  boost::mutex::scoped_lock scope(m_mutex);

  std::map<unsigned, bool>::iterator iter = m_glyph_guard.find(op->getUnicode());
  if(m_glyph_guard.end() == iter)
  {
    std::ostringstream sstr;
    sstr << "trying to add glyph " << op->getUnicode() << " that has not been marked for rendering";
    BOOST_THROW_EXCEPTION(std::runtime_error(sstr.str()));
  }

  m_glyphs.push_back(boost::shared_ptr<FtGlyph>(op));

  this->concurrencyDecrement();

  if(g_verbose)
  {
    if(m_failure_pending)
    {
      std::cerr << std::endl;
      std::cerr.flush();

      m_failure_pending = false;
    }

    std::cout << *op;
    std::cout.flush();
  }
}

void GlyphStorage::concurrencyDecrement()
{
  --m_glyphs_in_flight;
  m_cond.notify_one();
}

void GlyphStorage::concurrencyIcrement()
{
  boost::mutex::scoped_lock scope(m_mutex);

  while(m_glyphs_in_flight >= m_concurrency)
  {
    m_cond.wait(scope);
  }

  ++m_glyphs_in_flight;
}

bool GlyphStorage::markGlyph(unsigned op)
{
  boost::mutex::scoped_lock scope(m_mutex);

  if(m_glyph_guard.end() == m_glyph_guard.find(op))
  {
    m_glyph_guard[op] = true;
    return true;
  }
  return false;
}

void GlyphStorage::missing(unsigned op)
{
  if(g_verbose)
  {
    boost::mutex::scoped_lock scope(m_mutex);

    if(!m_failure_pending)
    {
      std::cerr << "Failed:";

      m_failure_pending = true;
    }

    std::cerr << ' ' << op;
    std::cerr.flush();
  }
}

void GlyphStorage::sort()
{
  std::sort(m_glyphs.begin(), m_glyphs.end(), ft_glyph_sptr_less);
}

void GlyphStorage::trim()
{
  this->sort();

  unsigned glyphs_remaining = 0;

  BOOST_FOREACH(const FtGlyphSptr &vv, m_glyphs)
  {
    if(NULL == vv.get())
    {
      break;
    }
    ++glyphs_remaining;
  }

  m_glyphs.resize(glyphs_remaining);
}

