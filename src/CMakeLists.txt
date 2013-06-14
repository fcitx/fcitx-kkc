# Source files
set(fcitx_kkc_sources
  kkc-config.c
  kkc.c
  )

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

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dictionary_list.in ${CMAKE_CURRENT_BINARY_DIR}/dictionary_list @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/dictionary_list rule DESTINATION "${FCITX4_PREFIX}/share/fcitx/kkc")
