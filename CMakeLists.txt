cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(trackballs)

# Compile options (ought to set mode dependent)
#add_compile_options(-Os)
#add_compile_options(-ggdb3)
add_compile_options(-Wall)

# Required libraries
list (APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
find_package(Guile)

# Optionally set data dir and locale dir
set(TRACKBALLS_LOCALE_OPT "" CACHE FILEPATH "Folder to install locale files to. Empty means default.")
set(TRACKBALLS_SHARE_OPT "" CACHE FILEPATH "Folder to install data files to. Empty means default.")
if ("${TRACKBALLS_LOCALE_OPT}" STREQUAL "")
  set(TRACKBALLS_LOCALE_DIR ${CMAKE_INSTALL_PREFIX}/share/locale)
else()
  set(TRACKBALLS_LOCALE_DIR ${TRACKBALLS_LOCALE_OPT})
endif()
if ("${TRACKBALLS_SHARE_OPT}" STREQUAL "")
  set(TRACKBALLS_SHARE_DIR ${CMAKE_INSTALL_PREFIX}/share/trackballs)
else()
  set(TRACKBALLS_SHARE_DIR ${TRACKBALLS_SHARE_OPT})
endif()


add_definitions(-DPACKAGE="trackballs")
add_definitions(-DVERSION="1.1.5")
add_definitions(-DLOCALEDIR="${TRACKBALLS_LOCALE_DIR}")
add_definitions(-DHAVE_CONFIG_H)
add_definitions(-DSHARE_DIR="${TRACKBALLS_SHARE_DIR}")
add_definitions(-DDEFAULT_RESOLUTION=-1)

# Guile interface
get_directory_property( definitions DIRECTORY ${CMAKE_SOURCE_DIR} COMPILE_DEFINITIONS )
set(defstr "")
foreach( d ${definitions} )
    set(defstr "${defstr} -D${d}")
endforeach()

add_custom_command(OUTPUT guile.cc.x
# -pthread
                  COMMAND CPP=cpp; guile-snarf -o guile.cc.x ${PROJECT_SOURCE_DIR}/src/guile.cc ${defstr}   -I${GUILE_INCLUDE_DIRS} -g -O2 -I/usr/include/SDL2 -D_GNU_SOURCE=1 -D_REENTRANT -I./ -I../ 
                  DEPENDS src/guile.cc
                  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Construct executable file
include_directories(${PROJECT_SOURCE_DIR}/src ${GUILE_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
add_executable(trackballs ${sources} guile.cc.x)
target_link_libraries(trackballs ${GUILE_LIBRARIES} GL GLU SDL2 SDL2_mixer SDL2_image SDL2_ttf z m)
install(TARGETS trackballs DESTINATION bin)

# Data files
install(DIRECTORY ${PROJECT_SOURCE_DIR}/share/fonts DESTINATION ${TRACKBALLS_SHARE_DIR})
install(DIRECTORY ${PROJECT_SOURCE_DIR}/share/images DESTINATION ${TRACKBALLS_SHARE_DIR})
install(DIRECTORY ${PROJECT_SOURCE_DIR}/share/levels DESTINATION ${TRACKBALLS_SHARE_DIR})
install(DIRECTORY ${PROJECT_SOURCE_DIR}/share/music DESTINATION ${TRACKBALLS_SHARE_DIR})
install(DIRECTORY ${PROJECT_SOURCE_DIR}/share/shaders DESTINATION ${TRACKBALLS_SHARE_DIR})
install(DIRECTORY ${PROJECT_SOURCE_DIR}/share/sfx DESTINATION ${TRACKBALLS_SHARE_DIR})
#todo: set executable setgid to games RX, and highscores to games RW
install(FILES ${PROJECT_SOURCE_DIR}/share/highScores DESTINATION ${TRACKBALLS_SHARE_DIR})

# Documentation
install(FILES ${PROJECT_SOURCE_DIR}/share/trackballs.6 DESTINATION man/man6)
file(GLOB htmldocs ${PROJECT_SOURCE_DIR}/docs/*.html ${PROJECT_SOURCE_DIR}/docs/*.css)
install(FILES ${htmldocs} DESTINATION ${TRACKBALLS_SHARE_DIR}/docs)

# Icons need special handling
file(GLOB icons ${PROJECT_SOURCE_DIR}/share/icons/*.png)
install(FILES ${icons} DESTINATION ${TRACKBALLS_SHARE_DIR}/icons/)

add_custom_command(OUTPUT execstring
                  COMMAND echo "Exec=${CMAKE_INSTALL_PREFIX}/bin/trackballs" >> execstring)
add_custom_command(OUTPUT trackballs.desktop
                  COMMAND cat ${PROJECT_SOURCE_DIR}/share/icons/trackballs.desktop execstring > ${CMAKE_CURRENT_BINARY_DIR}/trackballs.desktop
                  DEPENDS ${PROJECT_SOURCE_DIR}/share/icons/trackballs.desktop execstring)
add_custom_target(desktop ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/trackballs.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/trackballs.desktop DESTINATION share/applications)
# todo: spam copy the ${icons} to wherever KDE, GNOME, etc. would pick up on them

# Translations!
find_package(Gettext)
file(GLOB po_files po/*.po)
foreach(po_file ${po_files})
  get_filename_component(lang ${po_file} NAME_WE)
  GETTEXT_PROCESS_PO_FILES( ${lang} ALL PO_FILES ${po_file} )
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${lang}.gmo DESTINATION ${TRACKBALLS_LOCALE_DIR}/${lang}/LC_MESSAGES/ RENAME trackballs.mo)
endforeach()
