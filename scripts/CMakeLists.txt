cmake_minimum_required(VERSION 3.0.0)
project(index VERSION 0.0.1)

include(CTest)
enable_testing()


set(CMAKE_BUILD_TYPE Release)
# set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "../../exe/")

# ############################################################
set(INDEX-BUILD "build")
set(INDEX-FIND "find")
set(KERNELIZE "kernelize")
set(MINIMIZER_DIGEST "minimize")

set(KERNELIZE_EXEC "kernelize.cpp")
set(MINIMIZER_DIGEST_EXEC "minimize.cpp")
set(INDEX-BUILD_EXEC "build.cpp")
set(INDEX-FIND_EXEC "find.cpp")

# GLOBAL OPTIONS
############################################################
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic -std=c++17 -O3")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic -std=c++17 -g")

# files for index
############################################################
file(GLOB index_C "index/*.cpp")
file(GLOB index_H "index/*.h")
file(GLOB index ${index_C} ${index_H})

# SDSL
############################################################
include_directories(~/include) #SDSL headers are here
LINK_DIRECTORIES(~/lib) #SDSL lib are here

# EXECUTABLES
############################################################

add_executable(${KERNELIZE} ${KERNELIZE_EXEC})
target_link_libraries(${KERNELIZE} "-lsdsl")
target_link_libraries(${KERNELIZE} "-ldivsufsort")
target_link_libraries(${KERNELIZE} "-ldivsufsort64")
target_link_libraries(${KERNELIZE} "-lboost_program_options")

add_executable(${MINIMIZER_DIGEST} ${MINIMIZER_DIGEST_EXEC})
target_link_libraries(${MINIMIZER_DIGEST} "-lsdsl")
target_link_libraries(${MINIMIZER_DIGEST} "-ldivsufsort")
target_link_libraries(${MINIMIZER_DIGEST} "-ldivsufsort64")
target_link_libraries(${MINIMIZER_DIGEST} "-lboost_program_options")

# index
##############################
add_executable(${INDEX-BUILD} ${INDEX-BUILD_EXEC} ${UTILS} ${index})
target_link_libraries(${INDEX-BUILD} "-lsdsl")
target_link_libraries(${INDEX-BUILD} "-ldivsufsort")
target_link_libraries(${INDEX-BUILD} "-ldivsufsort64")
target_link_libraries(${INDEX-BUILD} "-lboost_program_options")

add_executable(${INDEX-FIND} ${INDEX-FIND_EXEC} ${UTILS} ${index})
target_link_libraries(${INDEX-FIND} "-lsdsl")
target_link_libraries(${INDEX-FIND} "-ldivsufsort")
target_link_libraries(${INDEX-FIND} "-ldivsufsort64")
target_link_libraries(${INDEX-FIND} "-lboost_program_options")