cmake_minimum_required(VERSION 3.0.0)
project(render VERSION 0.1.0)

include(CTest)
enable_testing()


SET(SOURCES ${PROJECT_SOURCE_DIR}/main.cpp)

if(WIN32)
SET(SOURCES ${SOURCES} ${PROJECT_SOURCE_DIR}/platforms/windows.cpp)
endif()


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(render ${SOURCES})

include_directories(C:/VulkanSDK/1.2.176.1/Include)

target_link_libraries(render PRIVATE C:/VulkanSDK/1.2.176.1/Lib/vulkan-1.lib)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
