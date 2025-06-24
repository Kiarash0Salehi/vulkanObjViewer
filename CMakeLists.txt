# CMakeList.txt : CMake project for CF vulkan, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

cmake_minimum_required(VERSION 3.8)
project("Vulkan")

set(SRC 
    main.cpp 
    vke.h 
    vke.cpp 
    VulkanContext.h 
    stb_image.h
    header/mat4.h 
    header/euler_angle.h 
    header/matrix.h
    header/angle.h
    header/LoadObj.h 
    header/vec.h 
    LoadObj.c 
    header/model.h
    logger.c
)

# Ensure Vulkan SDK environment variable is set
if(NOT DEFINED ENV{VULKAN_SDK})
    message(FATAL_ERROR "VULKAN_SDK environment variable is not set. Please install Vulkan SDK and set the environment variable.")
endif()

# Set glslang paths based on Vulkan SDK
#set(GLSLANG_INCLUDE_DIR "$ENV{VULKAN_SDK}/Include/glslang")
#set(GLSLANG_LIBRARY "$ENV{VULKAN_SDK}/Lib/glslang.lib")

# Check if glslang include directory exists
#if(EXISTS ${GLSLANG_INCLUDE_DIR})
#    message(STATUS "GLSLANG_INCLUDE_DIR: ${GLSLANG_INCLUDE_DIR}")
#    message(STATUS "GLSLANG_LIBRARY: ${GLSLANG_LIBRARY}")
#else()
#    message(FATAL_ERROR "GLSLANG_INCLUDE_DIR not found at ${GLSLANG_INCLUDE_DIR}")
#endif()

#set(SPIRV-Tools-opt_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/glslang/)
#set(SPIRV-Tools_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/glslang/)
#set(SPIRV-Tools-optTargets_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/SPIRV-Tools/source/opt)
find_package(Vulkan REQUIRED)
#set(CMAKE_PREFIX_PATH include/SPIRV-Tools/)
#set(SPIRV-Tools_DIR include/SPIRV-Tools/)
#set(SPIRV-Tools-opt_DIR include/SPIRV-Tools/)
#find_package(SPIRV-Tools CONFIG REQUIRED)
#set(CMAKE_PREFIX_PATH "$ENV{VULKAN_SDK}/Lib/cmake/glslang")
#find_package(glslang CONFIG REQUIRED)

add_subdirectory(include/glm EXCLUDE_FROM_ALL)

include_directories(Vulkan::Headers;)
link_libraries(${Vulkan_LIBRARY};)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -02 -pipe")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -02 -pipe")

# Copy the shaders directory to the build directory
file(COPY ${CMAKE_SOURCE_DIR}/shaders DESTINATION ${CMAKE_BINARY_DIR})

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 99)

if(DIST STREQUAL "ON")
    add_compile_definitions(DIST=${BUILD_TYPE})
    add_executable(${PROJECT_NAME} WIN32 ${SRC})
else()
    add_executable(${PROJECT_NAME} ${SRC})
endif()

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)
set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD 99)

#set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/include/glm")
#find_package(glm CONFIG REQUIRED PATHS "${CMAKE_CURRENT_SOURCE_DIR}/include/glm")
target_link_libraries(${PROJECT_NAME} PRIVATE 
    Vulkan::Vulkan 
    glm::glm-header-only 
    ${CMAKE_CURRENT_SOURCE_DIR}/CF.lib
)

target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/header/"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/"
)

