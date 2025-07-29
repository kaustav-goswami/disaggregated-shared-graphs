#ifndef GRAPH_HH_
#define GRAPH_HH_

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cinttypes>
#include <map>

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

// This is the updated version that supports metadata about graph cuts. While
// the rest of the structure of the graphs will remain the same, the initial
// head will have more info. This version supports upto 1024 cuts.
#define MAX_CUTS 1024

// We do not support weights in this version.

// To implement a graph, we need to hardcode the metadata of the graph. In this
// simple graph processing framework, we'll support simple directed and
// weighted graphs. The length of the metadata includes the number of vertices,
// edges, size of the row pointer index array, size of the column index array,
// size of the weights, and a synchroinization variable.
#define METADATA 6

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
    SYNC
};

#pragma once
// For better readability, there are a bunch of typedefs
typedef struct cut_info cut_t;

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
        graph_t graph_info;
        // The CSRs metadata is stored as independent variables in the local
        // memory. The metadata includes the number of edges, vertices, size of
        // the row_pointer array, size of the column_idx array and a
        // synchronization variable aligned by int. The metadata is also
        // stored on the mmap.
        // ___________________________________________________________ .. _____
        // | V | E | size | size | sync | row | col | weights                 |
        // |___|___|______|______|______|_____|_____|_________________ .. ____|
        //
        // The graph constructor will set these variables.
        //
        int _V;
        int _E;
        int _size_row_pointer;
        int _size_col_idx;
        int _size_weights;
        int _local_sync_copy;
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
        // The master node will write the graph from the given file into the
        // mmaped space!
        void graphWriter(std::string path);
    protected:
        // Need to add a couple of set methods to make my life easier to set
        // the metadata and debug more easily
        // TODO
        void setV(int value);
        void setE(int value);
        void setRowPointerSize(int value);
        void setColIndexSize(int value);
        void setWeightsSize(int value);
    public:
        // The graph object should be allocated however, this object does not
        // store the graph. Instead it is a wrapper around a mmap
        Graph(std::string path, int host_id, bool test, bool randomize,
                                                bool display, bool verbose);
        // To remove offset value retrival, we'll use three pointers for
        // getting the rowpointer, colindex and the weights directly from the
        // object; the read/write permissions will be set by the mmap call
        // depending upon the host_id.
        int *row_pointer;
        int *column_index;
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
        void printGraph();

        // Need a getter method for the starting node.
        int getStartingNode();
        inline bool doDisplay() { return _display; };
};
}
// extern class Graph *G;
#endif
