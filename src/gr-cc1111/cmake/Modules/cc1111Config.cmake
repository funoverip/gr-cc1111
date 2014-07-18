INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_CC1111 cc1111)

FIND_PATH(
    CC1111_INCLUDE_DIRS
    NAMES cc1111/api.h
    HINTS $ENV{CC1111_DIR}/include
        ${PC_CC1111_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREEFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    CC1111_LIBRARIES
    NAMES gnuradio-cc1111
    HINTS $ENV{CC1111_DIR}/lib
        ${PC_CC1111_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(CC1111 DEFAULT_MSG CC1111_LIBRARIES CC1111_INCLUDE_DIRS)
MARK_AS_ADVANCED(CC1111_LIBRARIES CC1111_INCLUDE_DIRS)

