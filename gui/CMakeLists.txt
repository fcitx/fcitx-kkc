set(KKC_CONFIG_SRCS
  main.cpp
  dictwidget.cpp
  adddictdialog.cpp
  dictmodel.cpp
  shortcutmodel.cpp
  shortcutwidget.cpp
  rulemodel.cpp
  addshortcutdialog.cpp
  )
fcitx_translate_add_sources(
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
  ${FCITX4_FCITX_CONFIG_LIBRARY_DIRS}
  )
qt4_wrap_ui(KKC_CONFIG_SRCS dictwidget.ui adddictdialog.ui shortcutwidget.ui addshortcutdialog.ui)
add_library(fcitx-kkc-config
  MODULE ${KKC_CONFIG_SRCS})

add_definitions(-DQT_NO_KEYWORDS -std=c++11)

set_target_properties(fcitx-kkc-config PROPERTIES
                      LINK_FLAGS "-Wl,--no-undefined"
                      AUTOMOC TRUE
)
target_link_libraries(fcitx-kkc-config
  debug ${QT_QTCORE_LIBRARY_DEBUG}
  debug ${QT_QTDBUS_LIBRARY_DEBUG}
  debug ${QT_QTGUI_LIBRARY_DEBUG}
  optimized ${QT_QTCORE_LIBRARY_RELEASE}
  optimized ${QT_QTDBUS_LIBRARY_RELEASE}
  optimized ${QT_QTGUI_LIBRARY_RELEASE}
  ${FCITX_QT_LIBRARIES}
  ${FCITX4_FCITX_CONFIG_LIBRARIES}
  ${LIBKKC_LIBRARIES}
  )

install(TARGETS fcitx-kkc-config DESTINATION ${FCITX4_ADDON_INSTALL_DIR}/qt)
