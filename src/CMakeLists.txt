# Source files
set(fcitx_kkc_sources
  kkc-config.c
  kkc.c
  )

include_directories(${LIBKKC_INCLUDE_DIRS})
link_directories(${LIBKKC_LIBRARY_DIRS})

# See document in FcitxMacro.cmake and the kkc .fxaddon file
# doc/fcitx-kkc.fxaddon in the fcitx repo for more information.
fcitx_add_addon_full(kkc
  DESC # Use configdesc file
  SCAN # Generate api header
  SCAN_PRIV # Generate addfunction header
  IM_CONFIG kkc.conf
  # HEADER_DIR alternative_name # change dir name your header is installed
  # LIB_NAME lib_name # module file name (without extension)
  # HEADERS headers.h # extra public headers to install
  # EXTRA_PO extra_files # extra files to translate
  LINK_LIBS ${LIBKKC_LIBRARIES} ${GLIB2_LIBRARIES}
  SOURCES ${fcitx_kkc_sources})
