source_files = Split('fontcompiler.cpp ft_face.cpp ft_glyph.cpp')

import os

env = Environment(
    ENV = os.environ,
    LIBS = 
    ['boost_program_options-mt', 'jpeg', 
      'png', 'boost_system-mt', 
      'boost_filesystem-mt', 'freetype', 'z'],
    CPPPATH='/opt/local/include/freetype2',
    )

env.Program('fontcompiler', source_files)

