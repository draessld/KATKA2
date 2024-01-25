cmake_minimum_required(VERSION 3.0.0)
project(index VERSION 0.0.1)

include(CTest)
enable_testing()


# set(CMAKE_BUILD_TYPE Release)
set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "../../exe/")

# ############################################################
set(INDEX-BUILD "index-build")
set(INDEX-LOCATE "index-locate")
set(KERNELIZE "kernelize")
set(MINIMIZER_DIGEST "minimizer_digest")

set(KERNELIZE_EXEC "src/kernelize.cpp")
set(MINIMIZER_DIGEST_EXEC "src/minimizer_digest.cpp")
set(INDEX-BUILD_EXEC "src/build.cpp")
set(INDEX-LOCATE_EXEC "src/locate.cpp")

# GLOBAL OPTIONS
############################################################
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic -std=c++17 -O3")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic -std=c++17 -g")

# files for index
############################################################
file(GLOB index_C "src/index/*.cpp")
file(GLOB index_H "src/index/*.h")
file(GLOB index ${index_C} ${index_H})

# # INDEX UTILS
# ############################################################
file(GLOB_RECURSE UTILS "src/utils/*.cpp")

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

add_executable(${INDEX-LOCATE} ${INDEX-LOCATE_EXEC} ${UTILS} ${index})
target_link_libraries(${INDEX-LOCATE} "-lsdsl")
target_link_libraries(${INDEX-LOCATE} "-ldivsufsort")
target_link_libraries(${INDEX-LOCATE} "-ldivsufsort64")
target_link_libraries(${INDEX-LOCATE} "-lboost_program_options")

#   KATKA2 dataset generator
set(GENERATE_DATASET "generate_dataset")
set(GENERATE_DATASET_EXEC "src/utils/KATKA2_build_dataset.c")
file(GLOB mtwister_C "src/utils/mtwister.c")
file(GLOB mtwister_H "src/utils/mtwister.h")
file(GLOB mtwister ${mtwister_C} ${mtwister_H} )

add_executable(${GENERATE_DATASET} ${GENERATE_DATASET_EXEC} ${mtwister})