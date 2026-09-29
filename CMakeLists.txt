cmake_minimum_required(VERSION 3.10)
set(ENV{PKG_CONFIG_PATH} "C:/dev/external/vcpkg/installed/x64-windows/lib/pkgconfig")

project(ldac_tools C)

set(CMAKE_C_STANDARD 11)

option(ASAN "Enable AddressSanitizer" OFF)

execute_process(
    COMMAND git describe --tags --abbrev=4 --dirty --always
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

add_compile_options(
    -MMD -MP
    -O3
    -g
    -march=native
    -std=gnu11
    -Wall # -Wextra
)

add_definitions(-DVERSION="${GIT_VERSION}")

include_directories(
    libldac/inc
    libldac/src
    ../getopt-win/src
)

if(ASAN)
    add_compile_options(-fsanitize=address)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=address")
endif()

set(LDAC_SRC
    libldacdec.c
    bit_allocation.c
    huffCodes.c
    bit_reader.c
    utility.c
    imdct.c
    spectrum.c
)

add_library(ldac STATIC ${LDAC_SRC})

set_target_properties(ldac PROPERTIES
    OUTPUT_NAME "ldac"
)
if(!WIN32)
    set_target_properties(ldac PROPERTIES
        SOVERSION 1
    )
endif()
target_link_libraries(ldac)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SNDFILE REQUIRED sndfile)
pkg_check_modules(SAMPLERATE REQUIRED samplerate)

link_directories(${SNDFILE_LIBRARY_DIRS} ${SAMPLERATE_LIBRARY_DIRS} ${CMAKE_BINARY_DIR})

set(LDACENC_SRC
    ldacenc.c
    libldac/src/ldaclib.c
    libldac/src/ldacBT.c
)

if(WIN32)
    list(APPEND LDACENC_SRC ../getopt-win/src/getopt.c)
endif()

add_executable(ldacenc ${LDACENC_SRC})

target_link_libraries(ldacenc
    ${SNDFILE_LIBRARIES}
    ${SAMPLERATE_LIBRARIES}
)

if(WIN32)
target_link_libraries(ldacenc shlwapi)
endif()

target_include_directories(ldacenc PRIVATE
    ${SNDFILE_INCLUDE_DIRS}
    ${SAMPLERATE_INCLUDE_DIRS}
)

add_executable(ldacdec ldacdec.c)

target_link_libraries(ldacdec
    ldac
    ${SNDFILE_LIBRARIES}
)

target_include_directories(ldacdec PRIVATE
    ${SNDFILE_INCLUDE_DIRS}
    ${SAMPLERATE_INCLUDE_DIRS}
)

#add_executable(mdct_imdct
#    #mdct_imdct.c
#    libldac/src/ldaclib.c
#    imdct.c
#)

#target_link_libraries(mdct_imdct
#    ldac
#    ${SNDFILE_LIBRARIES}
#)
#
#target_include_directories(mdct_imdct PRIVATE
#    ${SNDFILE_INCLUDE_DIRS}
#)

# --- Clean ---
set_directory_properties(PROPERTIES
    ADDITIONAL_MAKE_CLEAN_FILES
    "libldac.so;ldac.dll;ldacenc;ldacdec"
)
