cmake_minimum_required(VERSION 3.1)
cmake_policy(SET CMP0079 NEW)

option(LATREN_BUILD_STANDALONE OFF)
option(LATREN_BUNDLE_RESOURCES OFF)
set(LATREN_RESOURCE_DIR ${RUNTIME_OUTPUT_DIRECTORY}/../res)
set(LATREN_INTERNAL_RESOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/res/.latren)

set(VERSION_LATREN 0.1)
project(latren VERSION ${VERSION_LATREN})

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
if(CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_definitions(DEBUG_BUILD)
endif()

set(THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty)
set(SUBMODULE_ASSIMP_DIR ${THIRD_PARTY_DIR}/assimp)
set(SUBMODULE_BULLET_DIR ${THIRD_PARTY_DIR}/bullet)
set(SUBMODULE_FMT_DIR ${THIRD_PARTY_DIR}/fmt)
set(SUBMODULE_FREETYPE_DIR ${THIRD_PARTY_DIR}/freetype)
set(SUBMODULE_GLEW_DIR ${THIRD_PARTY_DIR}/glew)
set(SUBMODULE_GLFW_DIR ${THIRD_PARTY_DIR}/glfw)
set(SUBMODULE_GLM_DIR ${THIRD_PARTY_DIR}/glm)
set(SUBMODULE_SPDLOG_DIR ${THIRD_PARTY_DIR}/spdlog)
set(SUBMODULE_OPENAL_DIR ${THIRD_PARTY_DIR}/openal-soft)
set(BUILD_SHARED_LIBS OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
# set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB")

add_compile_definitions(LATREN_EXPORT_DLL)

# include paths
set(LATREN_THIRDPARTY_INCLUDES
    ${SUBMODULE_ASSIMP_DIR}/include
    ${SUBMODULE_BULLET_DIR}/src
    ${SUBMODULE_FMT_DIR}/include
    ${SUBMODULE_GLEW_DIR}/include
    ${SUBMODULE_GLM_DIR}
    ${SUBMODULE_SPDLOG_DIR}/include
    ${SUBMODULE_FREETYPE_DIR}/include
    ${SUBMODULE_GLFW_DIR}/include
    ${SUBMODULE_OPENAL_DIR}/include)
# sources
file(GLOB_RECURSE GLOB_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

# output binary
add_library(latren SHARED ${GLOB_SOURCES})

string(REPLACE ";" "\n\t- " SourcesFormatted "\n\t- ${Sources}")

set(STATIC_LIB_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/lib CACHE BOOL "" FORCE)
if(LATREN_BUILD_STANDALONE)
    set(SHARED_LIB_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/bin CACHE BOOL "" FORCE)
else()
    set(SHARED_LIB_OUTPUT_DIR ${EXECUTABLE_OUTPUT_PATH} CACHE BOOL "" FORCE)
endif()

message("static library output: ${STATIC_LIB_OUTPUT_DIR}")
message("shared library output: ${SHARED_LIB_OUTPUT_DIR}")

target_include_directories(latren PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include ${LATREN_THIRDPARTY_INCLUDES})
target_link_directories(latren PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib ${CMAKE_CURRENT_BINARY_DIR}/lib ${CMAKE_BINARY_DIR})
if(NOT LATREN_BUILD_STANDALONE)
    target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include ${LATREN_THIRDPARTY_INCLUDES})
    target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib ${CMAKE_CURRENT_BINARY_DIR}/lib ${CMAKE_BINARY_DIR})
endif()

set(ARCHIVE_OUTPUT_DIRECTORY ${STATIC_LIB_OUTPUT_DIR} CACHE BOOL "" FORCE)
set(LIBRARY_OUTPUT_PATH ${STATIC_LIB_OUTPUT_DIR} CACHE BOOL "" FORCE)
set(RUNTIME_OUTPUT_DIRECTORY ${SHARED_LIB_OUTPUT_DIR} CACHE BOOL "" FORCE)

macro(target_link_latren_depen lib)
    target_link_libraries(latren ${lib})
    if(NOT LATREN_BUILD_STANDALONE)
        target_link_libraries(${CMAKE_PROJECT_NAME} ${lib})
    endif()
    set_target_properties(${lib} PROPERTIES
        LIBRARY_OUTPUT_PATH ${STATIC_LIB_OUTPUT_DIR}
        ARCHIVE_OUTPUT_DIRECTORY ${STATIC_LIB_OUTPUT_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${SHARED_LIB_OUTPUT_DIR})
endmacro()

### ALL LIBRARIES ###
target_link_libraries(latren assimp)
target_link_libraries(${CMAKE_PROJECT_NAME} opengl32)

message("[ Assimp ]")
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(USE_STATIC_CRT OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)
add_subdirectory(${SUBMODULE_ASSIMP_DIR})
target_link_latren_depen(assimp)

message("[ Bullet ]")
set(USE_MSVC_RUNTIME_LIBRARY_DLL ON CACHE BOOL "" FORCE)
set(BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
set(BUILD_CPU_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_OPENGL3_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_BULLET2_DEMOS OFF CACHE BOOL "" FORCE)
add_subdirectory(${SUBMODULE_BULLET_DIR})
target_link_latren_depen(BulletDynamics)
target_link_latren_depen(BulletCollision)
target_link_latren_depen(LinearMath)

message("[ fmt ]")
add_subdirectory(${SUBMODULE_FMT_DIR})
target_link_latren_depen(fmt)

message("[ GLM ]")
add_subdirectory(${SUBMODULE_GLM_DIR})
target_link_latren_depen(glm)

message("[ spdlog ]")
add_subdirectory(${SUBMODULE_SPDLOG_DIR})
target_link_latren_depen(spdlog)

message("[ openal ]")
set(ALSOFT_INSTALL OFF CACHE BOOL "" FORCE)
set(ALSOFT_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ALSOFT_UTILS OFF CACHE BOOL "" FORCE)
add_subdirectory(${SUBMODULE_OPENAL_DIR})
target_link_latren_depen(OpenAL)

# i couldn't get these to work statically
set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)

message("[ FreeType ]")
add_subdirectory(${SUBMODULE_FREETYPE_DIR})
target_link_latren_depen(freetype)

message("[ GLEW ]")
set(glew-cmake_BUILD_STATIC OFF CACHE BOOL "" FORCE)
add_subdirectory(${SUBMODULE_GLEW_DIR})
target_link_latren_depen(libglew_shared)

message("[ GLFW ]")
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(${SUBMODULE_GLFW_DIR})
target_link_latren_depen(glfw)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
# project version macros
target_compile_definitions(latren PRIVATE LATREN_VERSION_MAJ=${PROJECT_VERSION_MAJOR} LATREN_VERSION_MIN=${PROJECT_VERSION_MINOR})
set_target_properties(latren PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${SHARED_LIB_OUTPUT_DIR})

# resources
if(LATREN_BUNDLE_RESOURCES)
    message("Copying resources to ${LATREN_RESOURCE_DIR}")
    file(COPY ${LATREN_INTERNAL_RESOURCE_DIR} DESTINATION ${LATREN_RESOURCE_DIR})
endif()