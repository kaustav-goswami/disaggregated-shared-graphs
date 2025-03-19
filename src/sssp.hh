#ifndef SSSP_HH_
#define SSSP_HH_
// Simple implementation of sssp
//
#include <iostream>

#include <stdlib.h>
#include <limits.h>

#include "graph.hh"

namespace simple {
class SSSP  {
    private:
    public:
        SSSP(Graph *G, int source);
        void dijkstra(Graph *G, int source);
};
}
#endif