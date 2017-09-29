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
  ${CMAKE_CURRENT_BINARY_DIR}
  )

link_directories(
  ${FCITX4_FCITX_CONFIG_LIBRARY_DIRS}
  )
qt5_wrap_ui(KKC_CONFIG_SRCS dictwidget.ui adddictdialog.ui shortcutwidget.ui addshortcutdialog.ui)
add_library(fcitx-kkc-config
  MODULE ${KKC_CONFIG_SRCS})

add_definitions(-DQT_NO_KEYWORDS -std=c++11)

set_target_properties(fcitx-kkc-config PROPERTIES
                      LINK_FLAGS "-Wl,--no-undefined"
                      AUTOMOC TRUE
)
target_link_libraries(fcitx-kkc-config
  Qt5::Core
  Qt5::Widgets
  FcitxQt5::WidgetsAddons
  ${FCITX4_FCITX_CONFIG_LIBRARIES}
  ${LIBKKC_LIBRARIES}
  )

install(TARGETS fcitx-kkc-config DESTINATION ${FCITX4_ADDON_INSTALL_DIR}/qt)
