# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "C:/Users/excalibur/Desktop/Projeler/C++ Real Project/Astralis-Engine/build/_deps/fastnoiselite-src")
  file(MAKE_DIRECTORY "C:/Users/excalibur/Desktop/Projeler/C++ Real Project/Astralis-Engine/build/_deps/fastnoiselite-src")
endif()
file(MAKE_DIRECTORY
  "C:/Users/excalibur/Desktop/Projeler/C++ Real Project/Astralis-Engine/build/_deps/fastnoiselite-build"
  "C:/Users/excalibur/Desktop/Projeler/C++ Real Project/Astralis-Engine/build/_deps/fastnoiselite-subbuild/fastnoiselite-populate-prefix"
  "C:/Users/excalibur/Desktop/Projeler/C++ Real Project/Astralis-Engine/build/_deps/fastnoiselite-subbuild/fastnoiselite-populate-prefix/tmp"
  "C:/Users/excalibur/Desktop/Projeler/C++ Real Project/Astralis-Engine/build/_deps/fastnoiselite-subbuild/fastnoiselite-populate-prefix/src/fastnoiselite-populate-stamp"
  "C:/Users/excalibur/Desktop/Projeler/C++ Real Project/Astralis-Engine/build/_deps/fastnoiselite-subbuild/fastnoiselite-populate-prefix/src"
  "C:/Users/excalibur/Desktop/Projeler/C++ Real Project/Astralis-Engine/build/_deps/fastnoiselite-subbuild/fastnoiselite-populate-prefix/src/fastnoiselite-populate-stamp"
)

set(configSubDirs Debug)
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/excalibur/Desktop/Projeler/C++ Real Project/Astralis-Engine/build/_deps/fastnoiselite-subbuild/fastnoiselite-populate-prefix/src/fastnoiselite-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Users/excalibur/Desktop/Projeler/C++ Real Project/Astralis-Engine/build/_deps/fastnoiselite-subbuild/fastnoiselite-populate-prefix/src/fastnoiselite-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
