#ifndef DFS_HH_
#define DFS_HH_
// Simple implementation of dfs
//
#include <iostream>
#include "graph.hh"

namespace simple {
#pragma once
class DFS  {
    private:
    public:
        DFS();
        DFS(Graph *G, int starting_vertex);
        // An API like method for other classes to use DFS.
        void dfsAPI(Graph *G, int vertex, int *visited, int component_id,
                                                               int* component);
        void dfs(Graph *G, int vertex, int *visited);
};
}
#endif