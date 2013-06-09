set(KKC_CONFIG_SRCS
  main.cpp
  configwidget.cpp
  )
set(KKC_CONFIG_HDRS
  main.h
  configwidget.h
  )
fcitx_translate_add_sources(
  ${KKC_CONFIG_HDRS}
  ${KKC_CONFIG_SRCS})

if(NOT ENABLE_QT)
  return()
endif()

include_directories(
  ${FCITX_QT_INCLUDE_DIRS}
  ${QT_QTCORE_INCLUDE_DIR}
  ${QT_QTGUI_INCLUDE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  )

link_directories(
  ${FCITX_QT_LIBRARY_DIRS}
  )
qt4_wrap_ui(KKC_CONFIG_SRCS configwidget.ui)
qt4_wrap_cpp(KKC_CONFIG_MOCS ${KKC_CONFIG_HDRS})
add_library(fcitx-kkc-config
  MODULE ${KKC_CONFIG_SRCS} ${KKC_CONFIG_MOCS})

set_target_properties(fcitx-kkc-config PROPERTIES
                      LINK_FLAGS "-Wl,--no-undefined"
)
target_link_libraries(fcitx-kkc-config
  debug ${QT_QTCORE_LIBRARY_DEBUG}
  debug ${QT_QTDBUS_LIBRARY_DEBUG}
  debug ${QT_QTGUI_LIBRARY_DEBUG}
  optimized ${QT_QTCORE_LIBRARY_RELEASE}
  optimized ${QT_QTDBUS_LIBRARY_RELEASE}
  optimized ${QT_QTGUI_LIBRARY_RELEASE}
  ${FCITX_QT_LIBRARIES}
  )

install(TARGETS fcitx-kkc-config DESTINATION ${FCITX4_ADDON_INSTALL_DIR}/fcitx/qt)
