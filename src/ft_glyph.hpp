#ifndef FT_GLYPH_HPP
#define FT_GLYPH_HPP

#include "defaults.hpp"

#include "ft2build.h"
#include FT_FREETYPE_H
#include FT_BITMAP_H

/** \brief Represents one rendered glyph.
 */
class FtGlyph
{
  private:
    /** Unicode number. */
    unsigned m_unicode;

    /** FreeType bitmap. */
    FT_Bitmap m_bitmap;

    /** Bitmap data. */
    uint8_t *m_crunched;

    /** Bitmap size. */
    unsigned m_size;

    /** Target size. */
    unsigned m_target_size;

    /** Dropdown distance as percentage of full glyph size. */
    float m_dropdown;

    /** Freetype glyph data. */
    float m_width;

    /** Freetype glyph data. */
    float m_height;

    /** Freetype glyph data. */
    float m_left;

    /** Freetype glyph data. */
    float m_top;

    /** Freetype glyph data. */
    float m_advance_x;

    /** Freetype glyph data. */
    float m_advance_y;

    /** Bitmap width. */
    unsigned m_bitmap_w;

    /** Bitmap height. */
    unsigned m_bitmap_h;

    /** OpenGL data. */
    float m_x1;

    /** OpenGL data. */
    float m_y1;

    /** OpenGL data. */
    float m_x2;

    /** OpenGL data. */
    float m_y2;

    /** OpenGL data. */
    float m_s1;

    /** OpenGL data. */
    float m_t1;

    /** OpenGL data. */
    float m_s2;

    /** OpenGL data. */
    float m_t2;

    /** Font page. */
    unsigned m_page;

  public:
    /** \brief Constructor.
     *
     * \param pcode Unicode number.
     * \param bitmap Bitmap to copy.
     * \param psize Bitmap render size.
     * \param pw Width.
     * \param ph Height.
     * \param pleft Left.
     * \param ptop Top.
     * \param pax Advance x.
     * \param pay Advance y.
     * \param bw Bitmap width.
     * \param bh Bitmap height.
     * \param bdata Bitmap data.
     */
    FtGlyph(unsigned pcode, FT_Bitmap *bitmap, unsigned psize, unsigned ptarget, float pdropdown, float pleft,
        float ptop, float pax, float pay);

    /** \brief Destructor.
     */
    ~FtGlyph();

  private:
    /** \brief Contract the crunched from down.
     *
     * \return Pixels contracted.
     */
    unsigned contractDown();

    /** \brief Contract the crunched from left.
     *
     * \return Pixels contracted.
     */
    unsigned contractLeft();

    /** \brief Contract the crunched from right.
     *
     * \return Pixels contracted.
     */
    unsigned contractRight();

    /** \brief Contract the crunched from up.
     *
     * \return Pixels contracted.
     */
    unsigned contractUp();

    /** \brief Tell if a column is empty.
     *
     * \param op Column index.
     * \return True if yes.
     */
    bool isEmptyColumn(unsigned op);

    /** \brief Tell if a row is empty.
     *
     * \param op Row index.
     * \return True if yes.
     */
    bool isEmptyRow(unsigned op);

    /** \brief Regenerate the crunched area as a subset of what it was.
     *
     * \param px Sub-area X offset.
     * \param py Sub-area Y offset.
     * \param pw Sub-area width.
     * \param ph Sub-area height.
     */
    void subCrunched(unsigned px, unsigned py, unsigned pw, unsigned ph);

  public:
    /** \brief Copy this into a larger bitmap.
     *
     * \param tgt Target bitmap.
     * \param tw Target bitmap width.
     * \param th Target bitmap height.
     * \param idx Index to copy to.
     */
    void copy(uint8_t *tgt, unsigned tw, unsigned th, unsigned idx);

    /** \brief Crunch this bitmap.
     */
    void crunch();

    /** \brief Write the current glyph info into a file.
     *
     * \param fptr FILE pointer.
     * \param glst Use OpenGL texture cooredinates (as opposed to DirectX).
     */
    void write(FILE *fptr, bool glst = true);

  public:
    /** \brief Get crunched data.
     *
     * Scanlines in data are in normal image order.
     *
     * \return Pointer to crunched data.
     */
    inline uint8_t* getCrunched() const
    {
      return m_crunched;
    }

    /** \brief Get crunched bitmap width.
     *
     * \return Bitmap width.
     */
    inline unsigned getCrunchedWidth() const
    {
      return m_bitmap_w;
    }

    /** \brief Get crunched bitmap height.
     *
     * \return Bitmap height.
     */
    inline unsigned getCrunchedHeight() const
    {
      return m_bitmap_h;
    }

    /** \brief Get unicode number of the glyph.
     *
     * \return Glyph unicode id.
     */
    inline unsigned getUnicode() const
    {
      return m_unicode;
    }

    /** \brief Set the page number.
     *
     * \param op Page number.
     */
    inline void setPage(unsigned op)
    {
      m_page = op;
    }

    /** \brief Set the OpenGL texture coordinate data.
     *
     * \param s1 First S coordinate.
     * \param t1 First T coordinate.
     * \param s2 Second S coordinate.
     * \param t2 Second T coordinate.
     * \param page Font page.
     */
    inline void setST(float s1, float t1, float s2, float t2)
    {
      m_s1 = s1;
      m_t1 = t1;
      m_s2 = s2;
      m_t2 = t2;
    }

  public:
    /** \cond */
    friend std::ostream& operator<<(std::ostream &lhs, const FtGlyph &rhs);
    /** \endcond */
};

/** Convenience typedef. */
typedef boost::shared_ptr<FtGlyph> FtGlyphSptr;

/** \brief Output shader to a stream.
  *
  * \param lhs Left-hand-side operand.
  * \param rhs Right-hand-side operand.
  * \return Stream after input.
  */
std::ostream& operator<<(std::ostream &lhs, const FtGlyph &rhs);

#endif
