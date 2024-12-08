cmake_minimum_required(VERSION 3.15)
project(makefsdata_portable LANGUAGES C)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()


option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(BUILD_SHARED_LIBS "Build shared libs" OFF)
option(BUILD_TESTS "Build tests" ON)


if(ENABLE_ASAN)
    message(STATUS "Building with AddressSanitizer enabled")
    set(SANITIZER_FLAGS "-fsanitize=address -fno-omit-frame-pointer")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SANITIZER_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SANITIZER_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${SANITIZER_FLAGS}")
endif()

# Set defaults for C compilation
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Include directories for public headers
include_directories(${CMAKE_SOURCE_DIR}/include)

# Library target
add_library(makefsdata_portable STATIC src/makefsdata_portable.c src/platform.c)
target_include_directories(makefsdata_portable PUBLIC ${CMAKES_SOURCE_DIR}/include)
set_target_properties(makefsdata_portable PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

# CLI front-end
add_executable(makefsdata_portable_cli src/main.c)
target_link_libraries(makefsdata_portable_cli makefsdata_portable)
set_target_properties(makefsdata_portable_cli PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin
)

set(INSTALL_BIN_DIR ${CMAKE_INSTALL_PREFIX}/bin)
set(INSTALL_LIB_DIR ${CMAKE_INSTALL_PREFIX}/lib)
set(INSTALL_INCLUDE_DIR ${CMAKE_INSTALL_PREFIX}/include)

# Install executables
install(TARGETS makefsdata_portable_cli RUNTIME DESTINATION ${INSTALL_BIN_DIR})

# Install libraries
install(TARGETS makefsdata_portable ARCHIVE DESTINATION ${INSTALL_LIB_DIR})

# Install headers
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/ DESTINATION ${INSTALL_INCLUDE_DIR})

# Tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# CPack for installers
include(CPack)
# Configure CPack variables
set(CPACK_PACKAGE_NAME "makefsdata_portable")
set(CPACK_PACKAGE_VERSION "0.1.0")
set(CPACK_PACKAGE_CONTACT "kthompson395@dallasmakerspace.org")
