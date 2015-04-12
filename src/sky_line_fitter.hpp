#ifndef SKY_LINE_FITTER_HPP
#define SKY_LINE_FITTER_HPP

#include <boost/thread/mutex.hpp>

// Forward declaration.
class GlyphStorage;

/** \brief Fitting class for skylines.
 */
class SkyLineFitter
{
  private:
    /** Maximum size to fit. */
    unsigned m_max_size;

    /** Best fit count. */
    unsigned m_best_count;

    /** Best usage. */
    float m_best_usage;

    /** Best width (for given best fit and usage). */
    unsigned m_best_width;

    /** Best height (for given best fit and usage). */
    unsigned m_best_height;

    /** Width of last line output. */
    unsigned m_last_print_width;

    /** Concurrency guard. */
    boost::mutex m_mutex;

  public:
    /** \brief Constructor.
     */
    SkyLineFitter(unsigned pmax);

    /** \brief Destructor. */
    ~SkyLineFitter() { }

  private:
    /** \brief Store an attempt.
     *
     * \param pcount Count.
     * \param pusage Usage.
     * \param pw Width.
     * \param ph Height.
     */
    void storeAttempt(unsigned pcount, float pusage, unsigned pw, unsigned ph);

  public:
    /** \brief Queue all attempts.
     *
     * \param glyphs Glyph storage to use.
     */
    void queue(GlyphStorage &glyphs);

  private:
    /** Attempt thread.
     *
     * \param slf Sky line fitter.
     * \param pw Width to use.
     * \param pmaxh Maximum height.
     */
    static void attempt_thread(SkyLineFitter &slf, GlyphStorage &glyphs, unsigned pw, unsigned pmaxh);

  public:
    /** \brief Get best width.
     *
     * \return Best width.
     */
    inline unsigned getBestWidth() const
    {
      return m_best_width;
    }

    /** \brief Get best height.
     *
     * \return Best height.
     */
    inline unsigned getBestHeight() const
    {
      return m_best_height;
    }
};

#endif

