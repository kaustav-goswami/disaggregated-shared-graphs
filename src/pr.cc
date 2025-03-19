#include "pr.hh"

namespace simple {
PR::PR(Graph *G) {
    // set the metaparams with the default values
    setMetaParams(0.85, 1e-6, 100);

    // business as usual
    pageRank(G);
    std::cout << std::endl;
    
}
PR::PR(Graph *G, double damping_factor, double epsilon,
                                                    int max_iterations) {
    // set the metaparams with the the user defined values
    setMetaParams(damping_factor, epsilon, max_iterations);

    // business as usual
    pageRank(G);
    std::cout << std::endl;

}
void PR::pageRank(Graph *G) {
    double *rank = (double *) malloc (G->getV() * sizeof(double));

    double* prev_rank = (double *) malloc (G->getV() * sizeof(double));
    int* out_degrees = (int*) calloc (G->getV(), sizeof(int));

    calculateOutDegrees(G, out_degrees);

    for (int i = 0; i < G->getV(); i++) {
        rank[i] = 1.0 / (double) G->getV();
    }

    for (int iter = 0; iter < this->_max_iterations; iter++) {
        double diff = 0.0;
        for (int i = 0; i < G->getV(); i++) {
            prev_rank[i] = rank[i];
            rank[i] = (1.0 - this->_damping_factor) / G->getV();
        }

        for (int i = 0; i < G->getV(); i++) {
            int row_start = G->row_pointer[i];
            int row_end = G->row_pointer[i + 1];

            for (int j = row_start; j < row_end; j++) {
                int neighbor = G->column_index[j];
                rank[neighbor] += this->_damping_factor * 
                        prev_rank[i] / out_degrees[i];
            }
        }

        for (int i = 0; i < G->getV(); i++) {
            diff += fabs(rank[i] - prev_rank[i]);
        }

        if (diff < this->_epsilon) {
            break;
        }
    }

    free(prev_rank);
    free(out_degrees);

    if (G->doDisplay())
        printRank(rank, G->getV());
    free(rank);
}

void PR::calculateOutDegrees(Graph *G, int *out_degrees) {
    for (int i = 0 ; i < G->getV() ; i++)
        out_degrees[i] = G->row_pointer[i + 1] - G->row_pointer[i]; 
}

void PR::setMetaParams(double damping_factor, double epsilon,
                                                        int max_iterations) {
    _damping_factor = damping_factor;
    _epsilon = epsilon;
    _max_iterations = max_iterations;
}

void PR::printRank(double *rank, int size) {
        for (int i = 0 ; i < size ; i++)
            std::cout << "  Vertex: " << i << " : " << rank[i] << std::endl;
}
}