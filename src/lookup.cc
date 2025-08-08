#include "lookup.hh"

namespace simple {

Lookup::Lookup(Graph *G, const char* distribution) {
    // Lookup(G, 0, "uniform");
    // _distribution = distribution;
    // assert(strcmp(_distribution, "uniform"));
    runLookup(G, (int) G->getV());
    // this->_G = G; /
}

std::vector<int> Lookup::getNeighs(Graph *G, int vertex) {
    std::vector<int> return_data;
    for (int i = G->row_pointer[vertex] ; i < G->row_pointer[vertex + 1]; i++)
        // this copies the data to the local memory before displaying
        return_data.push_back(G->column_index[i]);
    return return_data;
}

void Lookup::getNeighs(Graph *G, int vertex, bool display) {
    // this simply reads the neighbors and prints it on IO
    std::vector<int> return_data;
    for (int i = G->row_pointer[vertex] ; i < G->row_pointer[vertex + 1]; i++)
        if (G->doDisplay())
            std::cout << G->column_index[i] << " ";
    std::cout << std::endl;
}

void Lookup::runLookup(Graph *G, int lookup_count) {
    // standard benchmark. For lookup_count, the program looks up the graph.
    // make sure to initialize the random number gen!
    std::default_random_engine generator;
    std::uniform_int_distribution<int> distribution(0, G->getV());

    bool display_results = G->doDisplay();

    // to make sure that the results are not optimized by the compiler,
    // TODO:
    // the results are stored in an array in the local memory and then freed!

    this->_temporary_results = (int *) malloc (sizeof(int) * G->getV()); 

    #pragma omp parallel for
    for (int i = 0 ; i < lookup_count ; i++) {
        // TODO
        // random vertex!
        int node_to_lookup = distribution(generator);
        std::vector<int> results = getNeighs(G, node_to_lookup);

        // print results
        if (display_results) {
            for (size_t j = 0 ; j < results.size() ; j++)
                std::cout << results[j] << " ";
        
            std::cout << std::endl;
        }
        if (results.size() > 0)
            this->_temporary_results[node_to_lookup] = results[0];
    }
}

void Lookup::displayRandomResults() {
    std::cout << "Results of a random node: " <<
                        this->_temporary_results[0] << std::endl;
}
}

