#ifndef SKY_LINE_HPP
#define SKY_LINE_HPP

#include "sky_line_location.hpp"

#include <boost/filesystem.hpp>

// Forward declaration.
class GlyphStorage;
class FtGlyph;

/** Skyline algorithm fitting class.
 */
class SkyLine
{
  public:
    /** Size step used - some graphics hardware can only take textures on 4 pixel granularity. */
    static const unsigned SIZE_STEP = 4;

  private:
    /** Bitmap data. */
    uint8_t *m_bitmap;

    /** Skyline data. */
    unsigned *m_line;

    /** Width. */
    unsigned m_width;

    /** Maximum height. */
    unsigned m_max_height;

    /** Number of wasted pixels. */
    unsigned m_wasted;

  public:
    /** \brief Constructor.
     *
     * \param pw Width.
     * \param pmaxh Maximum height.
     */
    SkyLine(unsigned pw, unsigned pmaxh);

    /** Destructor. */
    ~SkyLine();

  private:
    /** \brief Allocate a location.
     *
     * \param op Location to allocate.
     */
    void allocate(const SkyLineLocation &op);

    /** \brief Report how much space would be wasted by given location.
     *
     * \param op Location.
     * \return Wasted space in pixels.
     */
    unsigned getWastedSpace(const SkyLineLocation &op) const;

  public:
    /** \brief Fit a glyph.
     *
     * \param op Glyph to fit.
     * \return Location to fit into. May be invalid if did not fit.
     */
    SkyLineLocation fit(const FtGlyph &op);

    /** \brief Perform fitting of all glyphs in a storage.
     *
     * \param glyphs Glyph storage to use.
     * \param xmlfile C file structure to write to, if set.
     * \param pidx Page index to use when writing, if set.
     * \param glst Use OpenGL coordinates when writing, if set.
     * \return Glyphs fit.
     */
    unsigned fitAll(GlyphStorage &glyph, FILE *xmlfile = NULL, unsigned pidx = 0, bool glst = true);

    /** \brief Report largest used height.
     *
     * \return Used height.
     */
    unsigned getUsedHeight() const;

    /** \brief Report current usage.
     *
     * \return Usage value.
     */
    float getUsage() const;

    /** \brief \brief Insert a glyph.
     *
     * Location must be valid and should have been returned from a previous call to fit().
     * Appropriate texture coordinates will be written into the glyph.
     *
     * \param loc Location.
     * \param gly Glyph to insert into given location.
     */
    void insert(const SkyLineLocation &loc, FtGlyph &gly);

    /** \brief Write a generated bitmap into a file.
     *
     * \param op Filename to write to.
     */
    void save(const boost::filesystem::path &op);
};

#endif

