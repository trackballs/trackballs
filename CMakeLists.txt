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
set(TRACKBALLS_LOCALE_DIR "" CACHE FILEPATH "Folder to install locale files to. Empty means default.")
set(TRACKBALLS_SHARE_DIR "" CACHE FILEPATH "Folder to install data files to. Empty means default.")
set(TRACKBALLS_HIGHSCORES_DIR "" CACHE STRING "Folder to install highscores file to. Empty means to TRACKBALLS_SHARE_DIR. ~ will install per user")

if ("${TRACKBALLS_LOCALE_DIR}" STREQUAL "")
  set(TRACKBALLS_LOCALE ${CMAKE_INSTALL_PREFIX}/share/locale)
else()
  set(TRACKBALLS_LOCALE ${TRACKBALLS_LOCALE_DIR})
endif()
if ("${TRACKBALLS_SHARE_DIR}" STREQUAL "")
  set(TRACKBALLS_SHARE ${CMAKE_INSTALL_PREFIX}/share/trackballs)
else()
  set(TRACKBALLS_SHARE ${TRACKBALLS_SHARE_DIR})
endif()

add_definitions(-DPACKAGE="trackballs")
add_definitions(-DVERSION="1.2.3")
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
install(DIRECTORY ${PROJECT_SOURCE_DIR}/share/fonts DESTINATION ${TRACKBALLS_SHARE})
install(DIRECTORY ${PROJECT_SOURCE_DIR}/share/images DESTINATION ${TRACKBALLS_SHARE})
install(DIRECTORY ${PROJECT_SOURCE_DIR}/share/levels DESTINATION ${TRACKBALLS_SHARE})
install(DIRECTORY ${PROJECT_SOURCE_DIR}/share/music DESTINATION ${TRACKBALLS_SHARE})
install(DIRECTORY ${PROJECT_SOURCE_DIR}/share/shaders DESTINATION ${TRACKBALLS_SHARE})
install(DIRECTORY ${PROJECT_SOURCE_DIR}/share/sfx DESTINATION ${TRACKBALLS_SHARE})
#todo: set executable setgid to games RX, and highscores to games RW
add_custom_command(OUTPUT highScores
                   COMMAND echo "0" | gzip -c > ${CMAKE_CURRENT_BINARY_DIR}/highScores)
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
install(FILES ${PROJECT_SOURCE_DIR}/share/trackballs.6 DESTINATION man/man6)
file(GLOB htmldocs ${PROJECT_SOURCE_DIR}/docs/*.html ${PROJECT_SOURCE_DIR}/docs/*.css)
install(FILES ${htmldocs} DESTINATION ${TRACKBALLS_SHARE}/docs)

# Icons are split up into separate folders
file(GLOB icons ${PROJECT_SOURCE_DIR}/share/icons/*.png)
foreach(file ${icons})
  string(REGEX REPLACE ".*trackballs-" "" filea ${file})
  string(REPLACE ".png" "" sz ${filea})
  install(FILES ${file}
          DESTINATION ${TRACKBALLS_SHARE}/../icons/hicolor/${sz}/apps/
          RENAME trackballs.png)
endforeach()
install(FILES ${PROJECT_SOURCE_DIR}/share/icons/trackballs.svg
        DESTINATION ${TRACKBALLS_SHARE}/../icons/hicolor/scalable/apps/)

add_custom_command(OUTPUT execstring
                  COMMAND echo "Exec=${CMAKE_INSTALL_PREFIX}/bin/trackballs" > execstring)
add_custom_command(OUTPUT trackballs.desktop
                  COMMAND cat ${PROJECT_SOURCE_DIR}/share/icons/trackballs.desktop execstring > ${CMAKE_CURRENT_BINARY_DIR}/trackballs.desktop
                  DEPENDS ${PROJECT_SOURCE_DIR}/share/icons/trackballs.desktop execstring)
add_custom_target(desktop ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/trackballs.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/trackballs.desktop DESTINATION ${TRACKBALLS_SHARE}/../applications)

# Translations!
find_package(Gettext)
file(GLOB po_files po/*.po)
foreach(po_file ${po_files})
  get_filename_component(lang ${po_file} NAME_WE)
  GETTEXT_PROCESS_PO_FILES( ${lang} ALL PO_FILES ${po_file} )
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${lang}.gmo DESTINATION ${TRACKBALLS_LOCALE}/${lang}/LC_MESSAGES/ RENAME trackballs.mo)
endforeach()
