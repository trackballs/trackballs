cmake_minimum_required(VERSION 2.8.12...4.0 FATAL_ERROR)
project(trackballs)

# Required libraries
list (APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
find_package(Guile REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_ttf REQUIRED)
set(OpenGL_GL_PREFERENCE "LEGACY")
find_package(OpenGL REQUIRED)
find_package(ZLIB REQUIRED)
# libgc is a dependency of Guile that still needs to be provided
find_package(GC REQUIRED)
find_package(Intl REQUIRED)

# Optionally set paths to install individual components
function(setordefault varname variable default)
  if ("${variable} " STREQUAL " ")
    set(${varname} ${default} PARENT_SCOPE)
  else()
    set(${varname} ${variable} PARENT_SCOPE)
  endif()
endfunction()

set(TRACKBALLS_METAINFO_DIR "" CACHE PATH "Folder to install .appdata.xml files to. If empty, the default is CMAKE_INSTALL_PREFIX/share/metainfo")
set(TRACKBALLS_APPL_DIR "" CACHE PATH "Folder to install .desktop files to. If empty, the default is CMAKE_INSTALL_PREFIX/share/applications")
set(TRACKBALLS_DOC_DIR "" CACHE PATH "Folder to install documentation to. If empty, the default is CMAKE_INSTALL_PREFIX/share/doc")
set(TRACKBALLS_BIN_DIR "" CACHE PATH "Folder to install executable files to. If empty, the default is CMAKE_INSTALL_PREFIX/bin")
set(TRACKBALLS_ICON_DIR "" CACHE PATH "Folder to install icons to. If empty, the default is CMAKE_INSTALL_PREFIX/share/icons/hicolor")
set(TRACKBALLS_LOCALE_DIR "" CACHE PATH "Folder to install locale files to. If empty, the default is CMAKE_INSTALL_PREFIX/share/locale")
set(TRACKBALLS_MAN_DIR "" CACHE PATH "Folder to install man files to. If empty, the default is CMAKE_INSTALL_PREFIX/share/man")
set(TRACKBALLS_SHARE_DIR "" CACHE PATH "Folder to install data files to. If empty, the default is CMAKE_INSTALL_PREFIX/share/trackballs")

set(TRACKBALLS_HIGHSCORES_DIR "" CACHE STRING "Folder to install highscores file to. Empty means to TRACKBALLS_SHARE_DIR/. ~ will install per user")

setordefault(TRACKBALLS_METAINFO_DIR "${TRACKBALLS_METAINFO_DIR}" ${CMAKE_INSTALL_PREFIX}/share/metainfo)
setordefault(TRACKBALLS_APPL "${TRACKBALLS_APPL_DIR}" ${CMAKE_INSTALL_PREFIX}/share/applications)
setordefault(TRACKBALLS_BIN "${TRACKBALLS_BIN_DIR}" ${CMAKE_INSTALL_PREFIX}/bin)
setordefault(TRACKBALLS_DOC "${TRACKBALLS_DOC_DIR}" ${CMAKE_INSTALL_PREFIX}/share/doc/trackballs)
setordefault(TRACKBALLS_ICON "${TRACKBALLS_ICON_DIR}" ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor)
setordefault(TRACKBALLS_LOCALE "${TRACKBALLS_LOCALE_DIR}" ${CMAKE_INSTALL_PREFIX}/share/locale)
setordefault(TRACKBALLS_MAN "${TRACKBALLS_MAN_DIR}" ${CMAKE_INSTALL_PREFIX}/share/man)
setordefault(TRACKBALLS_SHARE "${TRACKBALLS_SHARE_DIR}" ${CMAKE_INSTALL_PREFIX}/share/trackballs)

# Set defines
add_definitions(-DPACKAGE="trackballs")
add_definitions(-DVERSION="1.3.5")
add_definitions(-DLOCALEDIR="${TRACKBALLS_LOCALE}")
add_definitions(-DHAVE_CONFIG_H)
add_definitions(-DSHARE_DIR="${TRACKBALLS_SHARE}")
if (NOT "${TRACKBALLS_HIGHSCORES_DIR}" STREQUAL "")
  add_definitions(-DALT_HIGHSCORES="${TRACKBALLS_HIGHSCORES_DIR}")
endif()
add_definitions(-DDEFAULT_RESOLUTION=-1)
add_definitions(${GUILE_DEFINITIONS} ${SDL2_DEFINITIONS} ${GC_DEFINITIONS})

# Guile interface
get_directory_property( definitions DIRECTORY ${CMAKE_SOURCE_DIR} COMPILE_DEFINITIONS )
set(defstr "")
foreach( d ${definitions} )
    set(defstr "${defstr}" "-D${d}")
endforeach()

include_directories(${PROJECT_SOURCE_DIR}/src ${GUILE_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR} ${SDL2_INCLUDE_DIR} ${SDL2_IMAGE_INCLUDE_DIR} ${SDL2_MIXER_INCLUDE_DIR} ${SDL2_TTF_INCLUDE_DIR} ${ZLIB_INCLUDE_DIRS} ${GC_INCLUDE_DIRS} ${Intl_INCLUDE_DIRS})
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
target_link_libraries(trackballs ${GUILE_LIBRARIES} ${OPENGL_LIBRARIES} ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARY} ${SDL2_MIXER_LIBRARY} ${SDL2_TTF_LIBRARY} ${ZLIB_LIBRARIES} ${GC_LIBRARIES} ${Intl_LIBRARIES} m)
install(TARGETS trackballs DESTINATION ${TRACKBALLS_BIN})

# Data files
install(DIRECTORY ${PROJECT_SOURCE_DIR}/share/fonts DESTINATION ${TRACKBALLS_SHARE})
install(DIRECTORY ${PROJECT_SOURCE_DIR}/share/images DESTINATION ${TRACKBALLS_SHARE})
install(DIRECTORY ${PROJECT_SOURCE_DIR}/share/levels DESTINATION ${TRACKBALLS_SHARE})
install(DIRECTORY ${PROJECT_SOURCE_DIR}/share/music DESTINATION ${TRACKBALLS_SHARE})
install(DIRECTORY ${PROJECT_SOURCE_DIR}/share/shaders DESTINATION ${TRACKBALLS_SHARE})
install(DIRECTORY ${PROJECT_SOURCE_DIR}/share/sfx DESTINATION ${TRACKBALLS_SHARE})
install(FILES ${PROJECT_SOURCE_DIR}/share/icons/trackballs-128x128.png DESTINATION ${TRACKBALLS_SHARE}/icons/)

#todo: set executable setgid to games RX, and highscores to games RW
add_custom_command(OUTPUT highScores
                   COMMAND echo "0" | gzip -cn > ${CMAKE_CURRENT_BINARY_DIR}/highScores)
add_custom_target(highscores ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/highScores)
if ("${TRACKBALLS_HIGHSCORES_DIR}" STREQUAL "")
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/highScores DESTINATION ${TRACKBALLS_SHARE})
else()
  string(SUBSTRING "${TRACKBALLS_HIGHSCORES_DIR}" 0 1 TRACKBALLS_HS_FCHAR)
  if (NOT "${TRACKBALLS_HS_FCHAR}" STREQUAL "~")
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/highScores DESTINATION ${TRACKBALLS_HIGHSCORES_DIR})
  endif()
endif()

# Documentation
configure_file(${PROJECT_SOURCE_DIR}/share/trackballs.6 ${CMAKE_CURRENT_BINARY_DIR}/trackballs.6)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/trackballs.6 DESTINATION ${TRACKBALLS_MAN}/man6)
file(GLOB htmldocs ${PROJECT_SOURCE_DIR}/docs/*.html ${PROJECT_SOURCE_DIR}/docs/*.css)
install(FILES ${htmldocs} DESTINATION ${TRACKBALLS_DOC})

# Icons are split up into separate folders
file(GLOB icons ${PROJECT_SOURCE_DIR}/share/icons/*.png)
foreach(file ${icons})
  string(REGEX REPLACE ".*trackballs-" "" filea ${file})
  string(REPLACE ".png" "" sz ${filea})
  install(FILES ${file}
          DESTINATION ${TRACKBALLS_ICON}/${sz}/apps/
          RENAME trackballs.png)
endforeach()
install(FILES ${PROJECT_SOURCE_DIR}/share/icons/trackballs.svg
        DESTINATION ${TRACKBALLS_ICON}/scalable/apps/)
install(FILES ${PROJECT_SOURCE_DIR}/share/freedesktop/trackballs.desktop
        DESTINATION ${TRACKBALLS_APPL})
install(FILES ${PROJECT_SOURCE_DIR}/share/freedesktop/trackballs.appdata.xml
        DESTINATION ${TRACKBALLS_METAINFO_DIR})

# Translations!
option(DISABLE_NLS "Disable native language support (NLS)" OFF)
if(NOT DISABLE_NLS)
  find_package(Gettext)
  mark_as_advanced(GETTEXT_MSGFMT_EXECUTABLE GETTEXT_MSGMERGE_EXECUTABLE)
  file(GLOB po_files po/*.po)
  foreach(po_file ${po_files})
    get_filename_component(lang ${po_file} NAME_WE)
    GETTEXT_PROCESS_PO_FILES( ${lang} ALL PO_FILES ${po_file} )
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${lang}.gmo DESTINATION ${TRACKBALLS_LOCALE}/${lang}/LC_MESSAGES/ RENAME trackballs.mo)
  endforeach()
endif()
