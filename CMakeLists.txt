cmake_minimum_required(VERSION 2.8.6)

# The Project Name
project(fcitx-kkc)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
option(ENABLE_QT "Enable Qt for GUI configuration" On)

include(FindPkgConfig)
pkg_check_modules(LIBKKC "kkc-1.0>=0.2.3" REQUIRED)
find_package(Fcitx 4.2.8 REQUIRED)

if (ENABLE_QT)
find_package(Qt 4.8 REQUIRED)
pkg_check_modules(FCITX_QT "fcitx-qt>=4.2.8" REQUIRED)
endif ()

if (NOT SKK_DEFAULT_PATH)
    set(SKK_DEFAULT_PATH "/usr/share/skk/SKK-JISYO.L")
endif ()

# This will add a uninstall target. Comment out this line if other cmake packages
# you are using already provides such target (e.g. KDE).
_fcitx_add_uninstall_target()
include_directories(${FCITX4_FCITX_INCLUDE_DIRS})
include_directories(${FCITX4_FCITX_CONFIG_INCLUDE_DIRS})
include_directories(${FCITX4_FCITX_UTILS_INCLUDE_DIRS})
include_directories("${PROJECT_BINARY_DIR}")
include_directories(${LIBKKC_INCLUDE_DIRS})

link_directories(${LIBKKC_LIBRARY_DIRS})

set(CMAKE_C_FLAGS "-Wall -Wextra -Wno-sign-compare -Wno-unused-parameter -fvisibility=hidden ${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-sign-compare -Wno-unused-parameter -fvisibility=hidden ${CMAKE_CXX_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--as-needed ${CMAKE_SHARED_LINKER_FLAGS}")
set(CMAKE_MODULE_LINKER_FLAGS "-Wl,--as-needed ${CMAKE_MODULE_LINKER_FLAGS}")
add_definitions("-DLOCALEDIR=\"${CMAKE_INSTALL_PREFIX}/share/locale\"")

configure_file(config.h.in config.h
  IMMEDIATE @ONLY)

# The po dir (and targets, if any) should be added before any other translate
# related sources are processed.
add_subdirectory(po)
add_subdirectory(src)
add_subdirectory(gui)
add_subdirectory(data)
