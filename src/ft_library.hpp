#ifndef FT_LIBRARY_HPP
#define FT_LIBRARY_HPP

#include "ft2build.h"
#include FT_FREETYPE_H

/** FreeType library abstraction.
 */
class FtLibrary
{
  private:
    /** Class instance. */
    static FtLibrary ftlib;

  private:
    /** Freetype library handle. */
    FT_Library m_library_reference;

  public:
    /** Constructor. */
    FtLibrary();

    /** Destructor. */
    ~FtLibrary();

  public:
    /** \brief Accessor.
     *
     * \return FreeType library reference.
     */
    static FT_Library& get()
    {
      return ftlib.m_library_reference;
    }
};

#endif

