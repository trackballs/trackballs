cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

# Required libraries
list (APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
find_package(Guile)

add_definitions(-DPACKAGE="trackballs")
add_definitions(-DVERSION="1.1.5")
add_definitions(-DLOCALEDIR="${CMAKE_INSTALL_PREFIX}/share/locale")
add_definitions(-DHAVE_CONFIG_H)
add_definitions(-DSHARE_DIR="${CMAKE_INSTALL_PREFIX}/share/trackballs")
add_definitions(-DDEFAULT_RESOLUTION=1)

# Construct defn list 
get_directory_property( definitions DIRECTORY ${CMAKE_SOURCE_DIR} COMPILE_DEFINITIONS )
set(defstr "")
foreach( d ${definitions} )
    set(defstr "${defstr} -D${d}")
endforeach()

# TODO: make portable (how? test via raspi compilation)
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

# TODO:
# Other files: copy all except in icons, where we run installIcons
