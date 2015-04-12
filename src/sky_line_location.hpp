#ifndef SKY_LINE_LOCATION_HPP
#define SKY_LINE_LOCATION_HPP

#include "defaults.hpp"

/** \brief Individual location within a skyline.
 *
 * Used for fitting purposes.
 */
class SkyLineLocation
{
  private:
    /** Previous insert X offset. */
    unsigned m_x;

    /** Previous insert Y offset. */
    unsigned m_y;

    /** Previous insert width. */
    unsigned m_width;

    /** Previous insert height. */
    unsigned m_height;

    /** Space wasted by insertion into this location. */
    unsigned m_wasted;

    /** Is this location valid? */
    bool m_valid;

  public:
    /** \brief Get X location.
     *
     * \return X offset.
     */
    unsigned getX() const
    {
      return m_x;
    }

    /** \brief Get Y location.
     *
     * \return Y offset.
     */
    unsigned getY() const
    {
      return m_y;
    }

    /** \brief Get width.
     *
     * \return Width.
     */
    unsigned getWidth() const
    {
      return m_width;
    }

    /** \brief Get height.
     *
     * \return Height.
     */
    unsigned getHeight() const
    {
      return m_height;
    }

    /** \brief Get wasted space.
     *
     * \return Wasted space consumed if using this location.
     */
    unsigned getWasted() const
    {
      return m_wasted;
    }

    /** \brief Is this location valid.
     *
     * \return True if yes, false if no.
     */
    bool isValid() const
    {
      return m_valid;
    }

    /** \brief Set wasted space for this location.
     *
     * \param op New wasted space.
     */
    void setWasted(unsigned op)
    {
      m_wasted = op;
    }

  public:
    /** \brief Empty constructor.
     */
    SkyLineLocation() :
      m_valid(false) { }

    /** \brief Constructor.
     *
     * \param px X offset.
     * \param py Y offset.
     * \param pwidth Width.
     * \param pheight Height.
     */
    SkyLineLocation(unsigned px, unsigned py, unsigned pwidth, unsigned pheight) :
      m_x(px),
      m_y(py),
      m_width(pwidth),
      m_height(pheight),
      m_valid(true) { }
};

#endif

