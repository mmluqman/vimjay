cmake_minimum_required(VERSION 2.4.6)
cmake_policy(SET CMP0003 OLD)

set(PROJECT_NAME camthing)

#set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build )
#set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build )

include_directories( . )
#include_directories( $ENV{HG_CLONE_DIR}/external/include )
include_directories( $ENV{HOME}/other/install/include )
link_directories($ENV{HOME}/other/install/lib)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
#include_directories(${CMAKE_BINARY_DIR}/other)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x")

###############################################################################

find_package(OpenCV)
###############################################################################

add_executable(${PROJECT_NAME}
# TBD these are just for shaders
#other/DSOnoises/DSOcellularnoise.c
#other/DSOnoises/DSOimprovednoise.c
#other/DSOnoises/DSOsimplexnoise.c
#other/DSOnoises/DSOsrdnoise.c
other/DSOnoises/cellular.c
other/DSOnoises/noise1234.c
other/DSOnoises/sdnoise1234.c
other/DSOnoises/simplexnoise1234.c
other/DSOnoises/srdnoise23.c

config.cpp
camthing.cpp
nodes.cpp
misc_nodes.cpp
modify.cpp
signals.cpp
filter.cpp
cluster.cpp
generate.cpp
structure.cpp
screencap.cpp
utility.cpp
output.cpp
input.cpp
opengl.cpp
other/simplexnoise.cpp
other/simplextextures.cpp

)

target_link_libraries(${PROJECT_NAME} 
#${SOME_LIBS} 
${OpenCV_LIBS}
glog
gflags
boost_thread
boost_filesystem
boost_system
glut
GLEW
)


if(false)
add_executable(input_test
input.cpp
utility.cpp
)

target_link_libraries(input_test 
${OpenCV_LIBS}
glog
gflags
boost_thread
boost_filesystem
boost_system
Xi
)
endif(false)
