cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(trackballs)

# Required libraries
list (APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
find_package(Guile REQUIRED)
find_package(SDL2_and_friends REQUIRED)
find_package(OpenGL REQUIRED)
find_package(ZLIB REQUIRED)
# libgc is a dependency of Guile that still needs to be provided
find_package(GC REQUIRED)

# Optionally set data dir and locale dir
set(TRACKBALLS_LOCALE_PATH "" CACHE FILEPATH "Folder to install locale files to. Empty means default.")
set(TRACKBALLS_SHARE_PATH "" CACHE FILEPATH "Folder to install data files to. Empty means default.")
set(TRACKBALLS_HIGHSCORES_PATH "" CACHE FILEPATH "Folder to install highscores file to. Empty means to SHARE_PATH.")
if ("${TRACKBALLS_LOCALE_PATH}" STREQUAL "")
  set(TRACKBALLS_LOCALE_DIR ${CMAKE_INSTALL_PREFIX}/share/locale)
else()
  set(TRACKBALLS_LOCALE_DIR ${TRACKBALLS_LOCALE_PATH})
endif()
if ("${TRACKBALLS_SHARE_PATH}" STREQUAL "")
  set(TRACKBALLS_SHARE_DIR ${CMAKE_INSTALL_PREFIX}/share/trackballs)
else()
  set(TRACKBALLS_SHARE_DIR ${TRACKBALLS_SHARE_PATH})
endif()

add_definitions(-DPACKAGE="trackballs")
add_definitions(-DVERSION="1.2.2")
add_definitions(-DLOCALEDIR="${TRACKBALLS_LOCALE_DIR}")
add_definitions(-DHAVE_CONFIG_H)
add_definitions(-DSHARE_DIR="${TRACKBALLS_SHARE_DIR}")
if (NOT "${TRACKBALLS_HIGHSCORES_PATH}" STREQUAL "")
  add_definitions(-DALT_HIGHSCORES="${TRACKBALLS_HIGHSCORES_PATH}")
endif()
add_definitions(-DDEFAULT_RESOLUTION=-1)
add_definitions(${GUILE_DEFINITIONS} ${SDL2_DEFINITIONS} ${GC_DEFINITIONS})

# Guile interface
get_directory_property( definitions DIRECTORY ${CMAKE_SOURCE_DIR} COMPILE_DEFINITIONS )
set(defstr "")
foreach( d ${definitions} )
    set(defstr "${defstr}" "-D${d}")
endforeach()

include_directories(${PROJECT_SOURCE_DIR}/src ${GUILE_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR} ${SDL2_INCLUDE_DIR} ${SDL2_IMAGE_INCLUDE_DIR} ${SDL2_MIXER_INCLUDE_DIR} ${SDL2_TTF_INCLUDE_DIR} ${ZLIB_INCLUDE_DIRS} ${GC_INCLUDE_DIRS})
get_directory_property(includes DIRECTORY  ${CMAKE_SOURCE_DIR} INCLUDE_DIRECTORIES)
set(incstr "")
foreach( d ${includes} )
    set(incstr "${incstr}" "-I${d}")
endforeach()

# cpp is the c++ preprocessor
add_custom_command(OUTPUT guile.cc.x
                  COMMAND CPP=cpp; ${GUILE_SNARF} -o guile.cc.x ${PROJECT_SOURCE_DIR}/src/guile.cc ${defstr} ${incstr} -D_GNU_SOURCE=1 -D_REENTRANT
                  DEPENDS src/guile.cc
                  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Construct executable file
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
add_executable(trackballs ${sources} guile.cc.x)
target_link_libraries(trackballs ${GUILE_LIBRARIES} ${OPENGL_LIBRARIES} ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARY} ${SDL2_MIXER_LIBRARY} ${SDL2_TTF_LIBRARY} ${ZLIB_LIBRARIES} ${GC_LIBRARIES} m)
install(TARGETS trackballs DESTINATION bin)

# Data files
install(DIRECTORY ${PROJECT_SOURCE_DIR}/share/fonts DESTINATION ${TRACKBALLS_SHARE_DIR})
install(DIRECTORY ${PROJECT_SOURCE_DIR}/share/images DESTINATION ${TRACKBALLS_SHARE_DIR})
install(DIRECTORY ${PROJECT_SOURCE_DIR}/share/levels DESTINATION ${TRACKBALLS_SHARE_DIR})
install(DIRECTORY ${PROJECT_SOURCE_DIR}/share/music DESTINATION ${TRACKBALLS_SHARE_DIR})
install(DIRECTORY ${PROJECT_SOURCE_DIR}/share/shaders DESTINATION ${TRACKBALLS_SHARE_DIR})
install(DIRECTORY ${PROJECT_SOURCE_DIR}/share/sfx DESTINATION ${TRACKBALLS_SHARE_DIR})
#todo: set executable setgid to games RX, and highscores to games RW
add_custom_command(OUTPUT highScores
                   COMMAND echo "0" | gzip -c > ${CMAKE_CURRENT_BINARY_DIR}/highScores)
add_custom_target(highscores ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/highScores)
if ("${TRACKBALLS_HIGHSCORES_PATH}" STREQUAL "")
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/highScores DESTINATION ${TRACKBALLS_SHARE_DIR})
else()
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/highScores DESTINATION ${TRACKBALLS_HIGHSCORES_PATH})
endif()

# Documentation
install(FILES ${PROJECT_SOURCE_DIR}/share/trackballs.6 DESTINATION man/man6)
file(GLOB htmldocs ${PROJECT_SOURCE_DIR}/docs/*.html ${PROJECT_SOURCE_DIR}/docs/*.css)
install(FILES ${htmldocs} DESTINATION ${TRACKBALLS_SHARE_DIR}/docs)

# Icons need special handling
file(GLOB icons ${PROJECT_SOURCE_DIR}/share/icons/*.png)
install(FILES ${icons} DESTINATION ${TRACKBALLS_SHARE_DIR}/icons/)

add_custom_command(OUTPUT execstring
                  COMMAND echo "Exec=${CMAKE_INSTALL_PREFIX}/bin/trackballs" > execstring)
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
