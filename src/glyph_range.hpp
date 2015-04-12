#ifndef GLYPH_RANGE_HPP
#define GLYPH_RANGE_HPP

#include "ft_face.hpp"

#include <list>

// Forward declaration.
class GlyphStorage;

/** \brief Class representing glyph range.
 */
class GlyphRange
{
  private:
    /** Range. */
    std::vector<unsigned> m_range;

    /** Allowed to render? */
    bool m_enabled;

  public:
    /** \brief Empty constructor.
     *
     * Empty constructor deliberately creates an invalid range.
     */
    GlyphRange() :
      m_enabled(false) { }

    /** \brief Constructor.
     *
     * \param ps Start.
     * \param pe End.
     */
    GlyphRange(unsigned ps, unsigned pe);

  private:
    /** \brief Sort the contents of range vector.
     */
    void sort();

  public:
    /** \brief Add a range.
     *
     * \param ps First character to add.
     * \param pe Last character to add.
     */
    void add(unsigned ps, unsigned pe);

    /** \brief Remove a single character.
     *
     * \param op Character to remove.
     */
    void remove(unsigned op);

    /** \brief Remove a range.
     *
     * \param ps First character to remove.
     * \param pe Last character to remove.
     */
    void remove(unsigned ps, unsigned pe);

    /** \brief Render this range.
     *
     * \param dst Target glyph list.
     * \param src Font list.
     * \param target_size Target render size.
     * \return Number of glyphs queued.
     */
    unsigned queue(GlyphStorage &storage, std::list<FtFaceSptr> &src, unsigned target_size) const;

  public:
    /** \brief Add a single character.
     *
     * \param op Character to add.
     */
    inline void add(unsigned op)
    {
      m_range.push_back(op);

      this->sort();
    }

    /** \brief Allow this set.
     */
    inline void enable()
    {
      m_enabled = true;
    }

    /** \brief Deny this set.
     */
    inline void disable()
    {
      m_enabled = false;
    }

    /** \brief Tell if this range has been enabled.
     *
     * \return True if yes, false if no.
     */
    inline bool isEnabled() const
    {
      return m_enabled;
    }
};

#endif

