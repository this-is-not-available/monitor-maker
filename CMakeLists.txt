# zstd/zlib
cmake_minimum_required(VERSION 3.29.3)

project(monitor-maker VERSION 0.1.2 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON CACHE BOOL "" FORCE)

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

if(WIN32)
    set(WIN_RESOURCE_FILES assets/resources.rc)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

add_executable(monitor-maker src/main.cpp src/monitor_generate.cpp src/vpk_read.cpp
  src/gui/gui.cpp src/gui/gui_callback.cpp src/gui/gui_generated.cpp src/gui/resizable_image.h
  src/models/mdl.cpp src/models/vtx.cpp src/models/vvd.cpp src/models/phy.cpp ${WIN_RESOURCE_FILES})

target_include_directories(monitor-maker PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/models
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(monitor-maker PRIVATE -ffunction-sections -fdata-sections -static)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(monitor-maker PRIVATE /GA /GL)
endif()


FetchContent_Declare(
    fltk
    GIT_REPOSITORY https://github.com/fltk/fltk
    GIT_TAG        release-1.4.0
    GIT_SHALLOW    TRUE
)

set(FLTK_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(FLTK_MSVC_RUNTIME_DLL OFF CACHE BOOL "" FORCE)
set(FLTK_BUILD_FLUID ON CACHE BOOL "" FORCE)
set(FLTK_BUILD_FORMS OFF CACHE BOOL "" FORCE)
set(FLTK_BUILD_FLTK_OPTIONS OFF CACHE BOOL "" FORCE)
set(FLTK_USE_SYSTEM_LIBJPEG OFF CACHE BOOL "" FORCE)
set(FLTK_USE_SYSTEM_LIBPNG  OFF CACHE BOOL "" FORCE)
set(FLTK_USE_SYSTEM_ZLIB    OFF CACHE BOOL "" FORCE)

FetchContent_GetProperties(fltk)
if(NOT fltk_POPULATED)
    FetchContent_Populate(fltk)
    add_subdirectory(${fltk_SOURCE_DIR} ${fltk_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

set(OpenGL_GL_PREFERENCE "LEGACY")
find_package(OpenGL REQUIRED)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_link_options(monitor-maker PRIVATE -static-libstdc++ -static-libgcc)
endif()

set_target_properties(monitor-maker PROPERTIES
    WIN32_EXECUTABLE $<NOT:$<CONFIG:Debug>>
)

if (TARGET OpenGL::GL)
  target_link_libraries(monitor-maker PRIVATE OpenGL::GL)
elseif (OPENGL_LIBRARIES)
  target_link_libraries(monitor-maker PRIVATE ${OPENGL_LIBRARIES})
endif()

FetchContent_Declare(
    sourcepp
    GIT_REPOSITORY "https://github.com/craftablescience/sourcepp"
    GIT_TAG        "v2026.4.2"
    GIT_SHALLOW    TRUE
)


set(SOURCEPP_USE_BSPPP OFF CACHE INTERNAL "" FORCE)
set(SOURCEPP_USE_DMXPP OFF CACHE INTERNAL "" FORCE)
set(SOURCEPP_USE_GAMEPP OFF CACHE INTERNAL "" FORCE)
set(SOURCEPP_USE_MDLPP OFF CACHE INTERNAL "" FORCE)
set(SOURCEPP_USE_SNDPP OFF CACHE INTERNAL "" FORCE)
set(SOURCEPP_USE_TOOLPP OFF CACHE INTERNAL "" FORCE)
set(SOURCEPP_USE_VCRYPTPP OFF CACHE INTERNAL "" FORCE)

set(SOURCEPP_VTFPP_BUILD_WITH_COMPRESSONATOR OFF CACHE INTERNAL "" FORCE)
set(SOURCEPP_VTFPP_SUPPORT_EXR OFF CACHE INTERNAL "" FORCE)
set(SOURCEPP_VTFPP_SUPPORT_QOI OFF CACHE INTERNAL "" FORCE)

set(SOURCEPP_VPKPP_SUPPORT_VPK_V54 OFF CACHE INTERNAL "" FORCE)

set(SOURCEPP_USE_FSPP ON CACHE INTERNAL "" FORCE)

if (WIN32)
  set(SOURCEPP_LINK_STATIC_MSVC_RUNTIME ON CACHE INTERNAL "" FORCE)
endif()

set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(sourcepp)
set(CMAKE_WARN_DEPRECATED ON CACHE BOOL "" FORCE)

target_link_libraries(monitor-maker PRIVATE sourcepp::vtfpp sourcepp::steampp sourcepp::vpkpp sourcepp::fspp)

if (TARGET FLTK::FLTK)
  target_link_libraries(monitor-maker PRIVATE FLTK::FLTK FLTK::FLTK_IMAGES)
else()
  target_link_libraries(monitor-maker PRIVATE fltk fltk_images)
  target_include_directories(monitor-maker PRIVATE ${FLTK_INCLUDE_DIRS})
endif()

install(TARGETS monitor-maker RUNTIME DESTINATION bin)

include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_NAME "Portal 2 Monitor Maker")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A small tool for creating custom monitor models from user-provided images for Portal 2 modding.")
set(CPACK_PACKAGE_CONTACT "121245258+this-is-not-available@users.noreply.github.com")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_STRIP_FILES TRUE)
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

if(WIN32)
    set(CPACK_GENERATOR "ZIP")
else()
    set(CPACK_GENERATOR "TGZ;DEB")
endif()

include(CPack)