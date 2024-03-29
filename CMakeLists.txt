cmake_minimum_required(VERSION 3.16)
project(media-player-test C CXX)
set(CMAKE_CXX_STANDARD 14)

add_subdirectory(lib/glfw)
add_subdirectory(lib/ffmpeg)

add_definitions(-DGL_SILENCE_DEPRECATION)

if(APPLE)
	list(APPEND EXTRA_LIBS "-framework OpenGL")
elseif(WIN32)
	list(APPEND EXTRA_LIBS "-lglu32 -lopenGL32")
	set (CMAKE_EXE_LINKER_FLAGS "-std=gnu99 -static -static-libgcc -static-libstdc++ -mwindows")
else()
	list(APPEND EXTRA_LIBS "-lGL -lGLU -lX11")
endif()

set(GCC_COVERAGE_COMPILE_FLAGS "-g")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" "${GCC_COVERAGE_COMPILE_FLAGS}")
add_definitions("${GCC_COVERAGE_COMPILE_FLAGS}")

add_executable(media-player-test src/main.cpp src/loadFrame.cpp)
target_link_libraries(media-player-test FFmpeg glfw ${EXTRA_LIBS})
