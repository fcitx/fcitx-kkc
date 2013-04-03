# Add language code here each language code should correspond to a
# [language].po file in this directory.
set(PO_LANGS
  # zh_CN
  )
foreach(lang ${PO_LANGS})
  fcitx_translate_add_po_file("${lang}" "${lang}.po")
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
