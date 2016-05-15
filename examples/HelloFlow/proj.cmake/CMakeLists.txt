cmake_minimum_required (VERSION 2.6)
project (HELLOFLOW)

add_subdirectory(../../../../oxygine-framework oxygine-framework)
add_definitions(${OXYGINE_DEFINITIONS})
include_directories(${OXYGINE_INCLUDE_DIRS})
link_directories(${OXYGINE_LIBRARY_DIRS})

add_subdirectory(../../../ oxygine-flow)
include_directories(${OXYGINE_FLOW_INCLUDE_DIRS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
add_executable(HelloFlow
	../src/main.cpp
	../src/example.h 
	../src/example.cpp)

if (WIN32) #disable console mode for VC++
	set_target_properties(HelloFlow PROPERTIES WIN32_EXECUTABLE TRUE)
endif(WIN32)

target_link_libraries(HelloFlow oxygine-flow ${OXYGINE_CORE_LIBS} )