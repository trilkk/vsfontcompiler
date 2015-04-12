#ifndef FT_FACE_HPP
#define FT_FACE_HPP

#include "defaults.hpp"

#include <boost/thread.hpp>

#include "ft2build.h"
#include FT_FREETYPE_H

class FtGlyph;

/** \brief Class representing one freetype font.
 */
class FtFace
{
  private:
    /** Font face associated with this. */
    FT_Face m_face;

    /** Size associated with this face. */
    unsigned m_size;

    /** Dropdown distance as percentage of full glyph size. */
    float m_dropdown;

  public:
    /** \brief Default constructor.
     *
     * Throws an exception on error.
     *
     * \param filename Font file to open.
     * \param psize Precalc render size.
     * \param pdropdown Precalc dissipation scale.
     */
    FtFace(const std::string &filename, unsigned psize, float pdropdown);

    /** \brief Destructor.
     */
    ~FtFace();

  public:
    /** \brief Tell if this has a glyph.
     *
     * \param unicode Unicode glyph number.
     * \return True if yes, false if no.
     */
    bool hasGlyph(unsigned unicode);

    /** \brief Loads a glyph.
     *
     * \param unicode Unicode glyph number.
     * \param targetsize
     * \return Glyph object if successful, false on error.
     */
    FtGlyph* renderGlyph(unsigned unicode, unsigned targetsize);
};

/** Convenience typedef. */
typedef boost::shared_ptr<FtFace> FtFaceSptr;

#endif

