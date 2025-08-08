#ifndef GRAPH_HH_
#define GRAPH_HH_

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cinttypes>
#include <map>
#include <algorithm>
#include <sstream>
#include <vector>
#include <chrono>

#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <assert.h>
#include <time.h>

#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstdint>

// The program needs to understand the middleware! This conplicates the entire
// implementation. If the compiler flag is set to use the middleware, the
// program automatically understands that a graph_t structure is allocated
// instead of a int* to the memory mapped region.

#ifdef USE_MIDDLEWARE
#include "../ext/dmalloc/lib/src/simple_graphs_middleware.hh"
#else
// For the vanilla version, it still requires the base version of int *.
#include "dmalloc.hh"
#endif

// This is the updated version that supports metadata about the nodes of the 
// graph cuts. In this version, we implement graph lookups with permission.
// The allocator will assign a group id for a set of nodes.

// We do not support weights in this version.

// To implement a graph, we need to hardcode the metadata of the graph. In this
// simple graph processing framework, we'll support simple directed and
// weighted graphs. The length of the metadata includes the number of vertices,
// edges, size of the row pointer index array, size of the column index array,
// size of the weights, and a synchroinization variable.
#define METADATA 6
// To print the statistics of the program, we need chrono.
using namespace std::chrono;

namespace simple {
// Instead of hardcoding offset indices as numbers, I am using a enum for
// better understandability.
// TODO: Update the .cc file!
#pragma once
enum OffsetList {
    VERTEX,
    EDGE,
    ROWP_SIZE,
    COLI_SIZE,
    WEIGHT_SIZE,
    SYNC,
    // There needs to be a variable defining the total number of groups
    GROUPS,
    // After the sync variable, there will be an array with the pointers to the
    // actual row pointers. In the physical memory, checks will be placed to
    // determine if a given node and access rights have permission to access
    // the memory
    // PERMISSION_ARRAY
};


#pragma once
// For better readability, there are a bunch of typedefs
typedef struct cut_info cut_t;

// TODO: In the future, we'll also implement graph cuts and graph algorighms
// with cut data structures.
struct cut_info {
    int starting_node;                  // Vertex where the cut starts
    int ending_node;                    // Vertex where the cut ends
    int num_nodes;
    int num_edges;
    int *ptr_to_start_in_indices;       // pointer to the index array
    int index_end;                      // until when the pointers are valid
    int *ptr_to_start_in_indptr;        // pointer to the indptr array
    int indptr_end;                     // until when the indptr are valid
};

// Synchronization states should also be encoded into enum
#pragma once
enum SyncState {
    ALLOCATING,
    READY
};

#pragma once
class Graph {
    private:
        // Graph class stores a pointer to the mmaped region of the actual
        // graph. For any graph related operation, the *_graph pointer is used
        // to find the head of the graph.
        // std::map<std::string, void*> addr_map;
        // void* _mmap;
        int* _graph;
        // The middleware needs to be included for me to be able to compile
        // this file. Circular dependency!
        // graph_t graph_info;
        // The CSRs metadata is stored as independent variables in the local
        // memory. The metadata includes the number of edges, vertices, size of
        // the row_pointer array, size of the column_idx array and a
        // synchronization variable aligned by int. The metadata is also
        // stored on the mmap.
        // _____________________________________________________________ .. ___
        // | V | E | rowp | coli | grp  | sync | grp* | row* | col* | weights |
        // |   |   | size | size | size |      |      |      |      |         |
        // |___|___|______|______|______|______|______|______|______|___ .. __|
        //
        // The graph constructor will set these variables.
        //
        int _V;
        int _E;
        int _size_row_pointer;
        int _size_col_idx;
        int _size_weights;
        int _local_sync_copy;
        // There's also the need to store the access level of this host using
        // a private variable in this initialization.
#ifdef USE_MIDDLEWARE
        int _my_access_group;
        int _total_access_groups;
#endif
        // We need a collective structure to maintain all the metadata
        // privately. This will be an array of the first 6 elements.
        int *_metadata;

        // need to set a boolean to determine if there are weights
        bool _has_weight;
        // Assign a verbose variable and another for test
        bool _test;
        // Need avariable for randomization
        bool _randomize;
        bool _display;
        bool _verbose;
        bool _load;
#ifdef USE_MIDDLEWARE
        graph_alloc_t *_secure_graph;
#endif

        // So, this graph structure needs to have a pointer to the middleware
        // struct.

        // The master node will write the graph from the given file into the
        // mmaped space!
#ifdef USE_MIDDLEWARE
        // a secure graph writer is needed to make sure that the group informa-
        // tion is in the input file itself.
        void secureGraphWriter(std::string path);
#else
        void graphWriter(std::string path);
#endif
        // More private methods
#ifdef USE_MIDDLEWARE
        void secureDefineMemory(int host_id);
#else
        void defineMemory(size_t size, int host_id);
#endif

        void initializeMemory();
#ifdef USE_MIDDLEWARE
        void initializeSecureMemory();
#endif
    protected:
        // Need to add a couple of set methods to make my life easier to set
        // the metadata and debug more easily
        // TODO
        void setV(int value);
        void setE(int value);
#ifdef USE_MIDDLEWARE
        void setPermissionPointerSize(int value);
#endif
        void setRowPointerSize(int value);
        void setColIndexSize(int value);
        void setWeightsSize(int value);
        // The twitter graph is in el format. Need to convert to csr before we
        // are able to start processing the graph.
        void convertELtoCSR(std::string path, int host_id);
    public:
        // The graph object should be allocated however, this object does not
        // store the graph. Instead it is a wrapper around a mmap
        Graph(std::string path, size_t size, int host_id, bool test,
                bool randomize, bool display, bool verbose);
        Graph(std::string path, size_t size, int host_id, bool test,
                                                            bool randomize,
                                                            bool display,
                                                            bool verbose,
                                                            bool load);
        // To implement permissions, we need a group identifier.
#ifdef USE_MIDDLEWARE
        Graph(std::string path, size_t size, int host_id, int group_id, 
                                                            bool test,
                                                            bool randomize,
                                                            bool display,
                                                            bool verbose,
                                                            bool load);
#endif
        // To remove offset value retrival, we'll use three pointers for
        // getting the rowpointer, colindex and the weights directly from the
        // object; the read/write permissions will be set by the mmap call
        // depending upon the host_id.
        int *row_pointer;
        int *column_index;
#ifdef USE_MIDDLEWARE
        // If the user wants to assgin groups, then the start and end of the
        // vertex will be managed by the program. It'll simply keep pointers
        int *access_group_pointer;
#endif
        int *weights;
        // Format of the graph should include int a normal range of values.
        int getOffset(int index);
        // We finally need some public methods that makes life easier to
        // program the grraph algorithm. The values can be read out the
        // metadata array direcly but we want to prevent that!
        int getV();
        int getE();
        int getRowPointerSize();
        int getColIndexSize();
        int getWeightsSize();

#ifdef USE_MIDDLEWARE
        inline void setPermissionPointerSize(int value) { 
                                            _total_access_groups = value; } ;
        // getter methods for the metadata
        inline int getMyGroupId() { return _my_access_group; };
        inline int getPermissionPointerSize() { return _total_access_groups; };
#endif
        void printGraph();

        // Need a getter method for the starting node.
        int getStartingNode();
        inline bool doDisplay() { return _display; };
};
}
// extern class Graph *G;
#endif
