#ifndef GLYPH_STORAGE_HPP
#define GLYPH_STORAGE_HPP

#include "ft_glyph.hpp"

#include <boost/thread/condition_variable.hpp>

#include <vector>

/** \brief Storage for glyphs.
 */
class GlyphStorage
{
  private:
    /** Convenience typedef. */
    typedef std::vector<FtGlyphSptr> container_type;

  public:
    /** Iterator type. */
    typedef container_type::iterator iterator;

    /** Iterator type. */
    typedef container_type::const_iterator const_iterator;

  private:
    /** Glyph container. */
    container_type m_glyphs;

    /** Glyph rendering guard. */
    std::map<unsigned, bool> m_glyph_guard;

    /** Cond. */
    boost::condition_variable m_cond;

    /** Guard. */
    boost::mutex m_mutex;

    /** Number of glyphs 'in flight'. */
    unsigned m_glyphs_in_flight;

    /** Maximum number of glyphs 'in flight'. */
    unsigned m_concurrency;

    /** Reported failures pending? */
    bool m_failure_pending;

  public:
    /** \brief Iterator to begin.
     *
     * \return Iterator to begin.
     */
    inline iterator begin()
    {
      return m_glyphs.begin();
    }

    /** \brief Iterator to begin.
     *
     * \return Iterator to begin.
     */
    inline const_iterator begin() const
    {
      return m_glyphs.begin();
    }

    /** \brief Clear the storage.
     */
    inline void clear()
    {
      m_glyphs.clear();
    }

    /** \brief Iterator to end.
     *
     * \return Iterator to end.
     */
    inline iterator end()
    {
      return m_glyphs.end();
    }

    /** \brief Iterator to end.
     *
     * \return Iterator to end.
     */
    inline const_iterator end() const
    {
      return m_glyphs.end();
    }

    /** \brief Erase from an iterator.
     *
     * \param op Iterator to erase from.
     * \return Next iterator position.
     */
    inline iterator erase(const iterator &op)
    {
      return m_glyphs.erase(op);
    }

  public:
    /** Constructor. */
    GlyphStorage();

    /** Destructor. */
    ~GlyphStorage() { }

  private:
    /** \brief Decrement number of glyphs 'in flight'.
     *
     * Must be called from a locked context.
     */
    void concurrencyDecrement();

  public:
    /** \brief Add a glyph to the storage.
     *
     * \param op Glyph to add.
     */
    void add(FtGlyph *op);

    /** \brief Increment number of glyphs 'in flight'.
     */
    void concurrencyIcrement();

    /** \brief Mark a glyph for rendering.
     *
     * Glyphs may be only be marked for rendering one time, the point is to prevent rendering the same glyph
     * multiple times.
     *
     * \param op Unicode number of glyph.
     * \return True if glyph may be rendered, false otherwise.
     */
    bool markGlyph(unsigned op);

    /** \brief Report a glyph is missing.
     *
     * \param op Unicode id of the glyph.
     */
    void missing(unsigned op);

    /** \brief Sort the storage.
     */
    void sort();

    /** \brief Trim the storage.
     *
     * Removes everything that is no longer in use.
     */
    void trim();

  public:
    /** \brief Tell if storage is empty.
     *
     * \return True if yes, false if no.
     */
    inline bool empty() const
    {
      return m_glyphs.empty();
    }

    /** \brief Accessor.
     *
     * \param op Index to access.
     * \return Stored glyph pointer.
     */
    inline FtGlyph* get(unsigned idx)
    {
      BOOST_ASSERT(m_glyphs.size() > idx);

      return m_glyphs[idx].get();
    }

    /** \brief Const accessor.
     *
     * \param op Index to access.
     * \return Stored glyph pointer.
     */
    inline const FtGlyph* get(unsigned idx) const
    {
      BOOST_ASSERT(m_glyphs.size() > idx);

      return m_glyphs[idx].get();
    }

    /** \brief Get the container inside.
     *
     * \return Reference to container.
     */
    inline container_type& getContainer()
    {
      return m_glyphs;
    }

    /** \brief Get the container inside.
     *
     * \return Const reference to container.
     */
    inline const container_type& getContainer() const
    {
      return m_glyphs;
    }

    /** \brief Get the size of the glyph vector.
     *
     * \return Number of glyphs stored.
     */
    inline unsigned size() const
    {
      return static_cast<unsigned>(m_glyphs.size());
    }
};

#endif

