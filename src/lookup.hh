#ifndef __LOOKUP_HH__
#define __LOOKUP_HH__

#include <iostream>
#include <vector>
#include <random>

#include "graph.hh"

namespace simple {
    // This class needs to inheret Graph class to override the assignment of 
    // the graph.
class Lookup {
    // A simple class that implements graph vertex lookup
    private:
        // A private variable is used to assign an access group for security
        // purposes.
        int _access_group;
        char *_distribution;
        int *_temporary_results;
        // Graph *_G;
    public:
        // Class contructor that initializes the graph
        Lookup(Graph *G, const char *distribution);
        // Lookup(Graph *G, int access_group, char *distribution);
        std::vector<int> getNeighs(Graph *G, int vertex);
        void getNeighs(Graph *G, int vertex, bool display);
        // A high-performance lookup api is needed for N number of lookups
        void runLookup(Graph *G, int lookup_count);
        void displayRandomResults();


};
}

#endif
