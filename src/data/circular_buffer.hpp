#ifndef DATA_CIRCULAR_BUFFER_HPP
#define DATA_CIRCULAR_BUFFER_HPP

#include "math/generic.hpp"

#include <boost/scoped_array.hpp>

namespace data
{
  /** \brief Circular buffer template class.
   *
   * Circular buffer is a class with amortized O(1) addition and O(1) removal of elements. It is intended to be
   * used for temporary storage that is rapidly filled and depleted by a small number of elements.
   *
   * After a slight initial cost, operations in circular buffer cause no memory allocation or deallocation.
   */
  template <typename Type> class CircularBuffer
  {
    private:
      /** Actual array for data. */
      boost::scoped_array<Type> m_array;

      /** Size of the array. */
      unsigned m_size;

      /** Next index to extract. */
      unsigned m_index_current;

      /** Next index to insert to. */
      unsigned m_index_insert;

    public:
      /** Constructor.
       *
       * Initializes the buffer with minimum usable size.
       *
       * \param psize Optional initial size of the array (default: 2).
       */
      CircularBuffer(unsigned psize = 2) :
        m_array(new Type[psize]),
        m_size(psize),
        m_index_current(0),
        m_index_insert(0)
      {
        BOOST_ASSERT(1 < psize); // impossible to know if array is empty or if size <= 1
      }

    public:
      /** \brief Get the capacity of the buffer.
       *
       * \return Current capacity.
       */
      unsigned capacity() const
      {
        return m_size;
      }

      /** \brief Tell if the buffer is empty.
       *
       * \return True if yes, false if no.
       */
      bool empty() const
      {
        return (m_index_current == m_index_insert);
      }

      /** \brief Get next index of a given index.
       *
       * \param op Index given.
       * \return Next index.
       */
      unsigned nextIndex(unsigned op) const
      {
        if(++op >= m_size)
        {
          return 0;
        }
        return op;
      }

      /** \brief Get number of elements in the buffer.
       *
       * \return Number of current elements.
       */
      unsigned size() const
      {
        if(m_index_insert >= m_index_current)
        {
          return m_index_insert - m_index_current;
        }
        return m_index_insert + (m_size - m_index_current);
      }

    private:
      /** \brief Grow the buffer.
       *
       * Only grow the array if the next insertion would make it full.
       *
       * \return Next insertion slot.
       */
      unsigned grow()
      {
        unsigned new_size = m_size + math::max(m_size >> 1, static_cast<unsigned>(1)),
                 ii = 0,
                 jj = m_index_current,
                 count = m_size - 1;
        Type *new_array = new Type[new_size];

        BOOST_ASSERT(this->size() == count);

        do {
          new_array[ii] = m_array[jj];

          ++ii;
          jj = this->nextIndex(jj);
        } while(ii < count);

        m_array.reset(new_array);
        m_size = new_size;
        m_index_current = 0;
        m_index_insert = ii;

        return ii + 1;
      }

    public:
      /** \brief Clear the buffer.
       */
      void clear()
      {
        m_index_current = 0;
        m_index_insert = 0;
      }

      /** \brief Get an item from the array.
       *
       * Throws an error if empty.
       *
       * \return Extracted item.
       */
      Type& get()
      {
        if(this->empty())
        {
          std::ostringstream sstr;
          sstr << "curcular buffer of size " << m_size << " is empty";
          BOOST_THROW_EXCEPTION(std::runtime_error(sstr.str()));
        }

        unsigned ret = m_index_current;
        m_index_current = this->nextIndex(ret);
        return m_array[ret];
      }

      /** \brief Put an item into the array.
       *
       * Increases array size if no room.
       *
       * \param op Entry to insert.
       */
      void put(const Type& op)
      {
        unsigned next_insert = this->nextIndex(m_index_insert);

        // grow in size if would be full after insertion
        if(m_index_current == next_insert)
        {
          next_insert = this->grow();
        }

        m_array[m_index_insert] = op;
        m_index_insert = next_insert;
      }
  };
}

#endif

