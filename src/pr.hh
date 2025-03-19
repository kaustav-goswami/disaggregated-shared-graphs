#ifndef PR_HH_
#define PR_HH_

#include <iostream>
#include <math.h>

#include "graph.hh"

namespace simple {
class PR {
    private:
        double _damping_factor;
        double _epsilon;
        int _max_iterations;

    protected:
        void calculateOutDegrees(Graph *G, int *out_degrees);
        void setMetaParams(
                double damping_factor, double epsilon, int max_iterations);
        void pageRank(Graph *G);
        void printRank(double *rank, int size);
    
    public:
        PR(Graph *G);
        PR(Graph *G, double damping_factor, double epsilon,
                                                        int max_iterations);
};
}
#endif