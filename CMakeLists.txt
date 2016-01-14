cmake_minimum_required (VERSION 2.6)
project (OXYGINE_FLOW)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(OXYGINE_FLOW_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src)
add_library(oxygine-flow STATIC 
	src/Scene.cpp 
	src/Scene.h
	src/flow.cpp 
	src/flow.h
	src/Transition.cpp 
	src/Transition.h
)

set(OXYGINE_FLOW_INCLUDE_DIRS ${OXYGINE_FLOW_INCLUDE_DIRS} PARENT_SCOPE)