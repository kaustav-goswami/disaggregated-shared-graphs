cmake_minimum_required(VERSION 3.9.1)
project(SimpleGraphForDisaggregatedMemory)


set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -fopenmp -O3")
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/)

# Option to enable USE_MIDDLEWARE for space-control
option(USE_MIDDLEWARE "Enable middleware and metadata management" OFF)

# Make sure that files are not repeated
set(SOURCES src/main.cc
                src/graph.cc
                src/sssp.cc
                src/bfs.cc
                src/bc.cc
                src/tc.cc
                src/pr.cc
                src/cc.cc
                src/dfs.cc
                src/dfs_hpc.cc
                src/cc_hpc.cc
                src/lookup.cc
                ext/cpp-arg-parse/src/argparse.cc
)

# Set source files based on USE_MIDDLEWARE
if(USE_MIDDLEWARE)
    add_definitions(-DUSE_MIDDLEWARE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

    add_subdirectory(ext/dmalloc/lib/)

    set(SOURCES ${SOURCES}
                ext/dmalloc/lib/src/s_permissions.cc
                ext/dmalloc/lib/src/s_dmalloc.cc
                ext/dmalloc/lib/src/utility.cc
                ext/dmalloc/lib/src/simple_graphs_middleware.cc
    )
else()
    set(SOURCES ${SOURCES}
                src/dmalloc.cc
    )
endif()

add_executable(simple_graph ${SOURCES})

if(USE_MIDDLEWARE)
    target_link_libraries(simple_graph PRIVATE SpaceControl)
    target_compile_definitions(simple_graph PRIVATE USE_MIDDLEWARE)
endif()