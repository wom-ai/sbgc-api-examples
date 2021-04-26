cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(LinuxSerialControl)

set(CMAKE_CXX_STANDARD 11) 
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -Wall" )

set(COMMON_LIBS rt dl pthread)
message(STATUS "COMMON_LIBS=${COMMON_LIBS}")

#--------------------------------------------------------------------------------------

include_directories( ../libraries/SBGC_lib)
include_directories( ../libraries/SBGC_Linux)
add_executable( LinuxSerialControl
  LinuxSerialControl.cpp
  ../libraries/SBGC_Linux/SBGC_Linux.cpp
)
#target_include_directories( uart
#  PUBLIC ${SPINNAKER_INCLUDE_DIR}
#)
#target_link_libraries( uart
#  PRIVATE ${OpenCV_LIBS}
#  ${SPINNAKER_LIBS}
#)

