cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(trackballs)

# Compile options
add_compile_options(-Os)
add_compile_options(-ggdb3)

# ASAN for debugging
set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fsanitize=address" )
set( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address" )

# Required libraries
list (APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
find_package(Guile)

add_definitions(-DPACKAGE="trackballs")
add_definitions(-DVERSION="1.1.5")
add_definitions(-DLOCALEDIR="${CMAKE_INSTALL_PREFIX}/share/locale")
add_definitions(-DHAVE_CONFIG_H)
add_definitions(-DSHARE_DIR="${CMAKE_INSTALL_PREFIX}/share/trackballs")
add_definitions(-DDEFAULT_RESOLUTION=-1)

# Guile interface
get_directory_property( definitions DIRECTORY ${CMAKE_SOURCE_DIR} COMPILE_DEFINITIONS )
set(defstr "")
foreach( d ${definitions} )
    set(defstr "${defstr} -D${d}")
endforeach()

add_custom_command(OUTPUT guile.cc.x
                  COMMAND CPP=cpp; guile-snarf -o guile.cc.x ${PROJECT_SOURCE_DIR}/src/guile.cc ${defstr}  -pthread -I${GUILE_INCLUDE_DIRS} -g -O2 -I/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT -I./ -I../ 
                  DEPENDS src/guile.cc
                  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Construct executable file
include_directories(${PROJECT_SOURCE_DIR}/src ${GUILE_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
add_executable(trackballs ${sources} guile.cc.x)
target_link_libraries(trackballs ${GUILE_LIBRARIES} GL GLU SDL SDL_mixer SDL_image SDL_ttf z pthread m)
install(TARGETS trackballs DESTINATION bin)

# Data files
install(DIRECTORY ${PROJECT_SOURCE_DIR}/share/fonts DESTINATION share/trackballs/)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/share/images DESTINATION share/trackballs/)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/share/levels DESTINATION share/trackballs/)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/share/music DESTINATION share/trackballs/)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/share/sfx DESTINATION share/trackballs/)
install(FILES ${PROJECT_SOURCE_DIR}/share/highScores DESTINATION share/trackballs/)

# Documentation
install(FILES ${PROJECT_SOURCE_DIR}/share/trackballs.6 DESTINATION man/man6)
# < insert other files!

# Icons need special handling
file(GLOB icons ${PROJECT_SOURCE_DIR}/share/icons/*.png)
install(FILES ${icons} DESTINATION share/trackballs/icons/)

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
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${lang}.gmo DESTINATION share/locale/${lang}/LC_MESSAGES/ RENAME trackballs.mo)
endforeach()
