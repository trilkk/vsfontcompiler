#include "ft_library.hpp"

#include "defaults.hpp"

#include <sstream>

#include "ftmodapi.h"

FtLibrary FtLibrary::ftlib;

FtLibrary::FtLibrary()
{
  FT_Error err = FT_Init_FreeType(&m_library_reference);

  if(0 != err)
  {
    std::ostringstream sstr;
    sstr << "could not init FreeType: " << err;
    BOOST_THROW_EXCEPTION(std::runtime_error(sstr.str()));
  }
}

FtLibrary::~FtLibrary()
{
  FT_Error err = FT_Done_Library(m_library_reference);

  if(0 != err)
  {
    std::ostringstream sstr;
    sstr << "could not close FreeType: " << err;
    BOOST_THROW_EXCEPTION(std::runtime_error(sstr.str()));
  }
}
