cmake_minimum_required(VERSION 3.28)
project(RedBall LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 选项：是否构建为共享库，共享库设为 ON；静态库设为 OFF
option(BUILD_SHARED_LIBS "Build libraries as shared" OFF)

# 查找本地安装的 SFML 开发包
# set(SFML_STATIC_LIBRARIES TRUE) 
# find_package(SFML COMPONENTS Graphics Window System REQUIRED)

# 获取SFML源码并构建
include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.1
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)

# Box2D
include(FetchContent)
FetchContent_Declare(
    box2d
    GIT_REPOSITORY https://github.com/erincatto/box2d.git
    GIT_TAG v3.1.0
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM
)
FetchContent_MakeAvailable(box2d)

add_library(GameLib
    src/RedBall.cpp
    src/DebugDrawUtils.cpp
)
target_include_directories(GameLib PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(GameLib
    PUBLIC
        SFML::Graphics
        SFML::Window
        SFML::System
        box2d
)
add_executable(main src/main.cpp)
target_compile_features(main PRIVATE cxx_std_17)

target_link_libraries(main
    PUBLIC
        GameLib
)

# 在Windows系统中（使用 MinGW-w64）构建时
# 静态链接需要将libstdc++、libgcc和libwinpthread-1.dll编译进可执行文件中，避免运行时依赖外部 DLL
if(WIN32 AND (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")))
  target_link_libraries(main PRIVATE
      -static-libgcc
      -static-libstdc++
    )  
  target_link_options(main PRIVATE
      "-static"
      "-Wl,-Bstatic,--whole-archive" -lwinpthread "-Wl,--no-whole-archive"
    )
endif()

add_custom_command(TARGET main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/data
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/data  
    COMMENT "Copying data directory to output directory"
)

# 测试 & 打包
enable_testing()
add_test(NAME SmokeTest
         COMMAND main
         WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

install(TARGETS main RUNTIME DESTINATION .)
install(DIRECTORY "data/"
        DESTINATION "data"
        FILES_MATCHING PATTERN "*"
)

set(CPACK_PACKAGE_NAME "RedBall")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_VENDOR "HDU")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "CMake-RedBall")
set(CPACK_PACKAGE_CONTACT "contact@example.com")
include(CPack)
