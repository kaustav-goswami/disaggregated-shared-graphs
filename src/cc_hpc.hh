#ifndef __CC_HPC_HH__
#define __CC_HPC_HH__

#include <iostream>
#include <vector>
#include <atomic>
#include <omp.h>

#include "graph.hh"

namespace simple {
class CC_HPC {
    private:
        std::vector<int> component;
    public:
        CC_HPC();
        CC_HPC(Graph *G);
        void printCC(Graph *G);
};
}

#endif
