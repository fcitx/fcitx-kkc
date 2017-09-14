# Add language code here each language code should correspond to a
# [language].po file in this directory.
file(GLOB PO_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" *.po)

foreach(po_file ${PO_FILES})
  string(REPLACE ".po" "" po_lang "${po_file}")
  fcitx_translate_add_po_file("${po_lang}" "${po_file}")
endforeach()

# This will register a pot target to update your po and pot files according to
# your source files, targets to compile and install po files into mo
# files as well as merging necessary translation into certain files.
# You can use `make pot` to extract strings to be translated from your source
# files.
fcitx_translate_set_pot_target(pot fcitx-kkc fcitx-kkc.pot
  # uncomment the following line and set to your email address.
  # BUGADDR your-email@address.com
  )
