#ifndef __DFS_HPC_CC__
#define __DFS_HPC_CC__

#include <iostream>
#include <vector>
#include <stack>
#include <omp.h>

#include "graph.hh"

namespace simple {
    class DFS_HPC {
        public:
            DFS_HPC();
            DFS_HPC(Graph *G);
    };
}

#endif
