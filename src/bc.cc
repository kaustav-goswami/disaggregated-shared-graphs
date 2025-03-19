#include "bc.hh"

namespace simple {
BC::BC(Graph *G) {
    // All the variables used to navigate the graph and generate BC data will
    // be local to the host.
    double* centrality = (double*)calloc(G->getV(), sizeof(double));
    for (int s = 0 ; s < G->getV() ; s++) {
        int *stack = (int *) malloc (G->getV() * sizeof(int));
        int *queue = (int *) malloc (G->getV() * sizeof(int));

        int *pred_count = (int *) calloc (G->getV(), sizeof(int));
        int** preds = (int**)malloc(G->getV() * sizeof(int*));

        double* sigma = (double*)calloc(G->getV(), sizeof(double));
        double* dist = (double*)calloc(G->getV(), sizeof(double));
        double* delta = (double*)calloc(G->getV(), sizeof(double));
        
        for (int i = 0; i < G->getV(); i++) {
            preds[i] = (int *) malloc (G->getV() * sizeof(int));
        }
        
        sigma[s] = 1;
        dist[s] = 0;
        int queue_start = 0, queue_end = 0;
        queue[queue_end++] = s;

        while (queue_start < queue_end) {
            int v = queue[queue_start++];
            stack[queue_start - 1] = v;

            int row_start = G->row_pointer[v];
            int row_end = G->row_pointer[v + 1];

            for (int i = row_start; i < row_end; i++) {
                int w = G->column_index[i];

                if (dist[w] == 0 && w != s) {
                    queue[queue_end++] = w;
                    dist[w] = dist[v] + 1;
                }
                if (dist[w] == dist[v] + 1) {
                    sigma[w] += sigma[v];
                    preds[w][pred_count[w]++] = v;
                }
            }
        }

        while (queue_start > 0) {
            int w = stack[--queue_start];
            for (int i = 0; i < pred_count[w]; i++) {
                int v = preds[w][i];
                delta[v] += (sigma[v] / sigma[w]) * (1 + delta[w]);
            }
            if (w != s) {
                centrality[w] += delta[w];
            }
        }

        free(stack);
        free(queue);
        free(pred_count);
        for (int i = 0; i < G->getV(); i++) {
            free(preds[i]);
        }
        free(preds);
        free(sigma);
        free(dist);
        free(delta);
    }
    if (G->doDisplay()) {
        for (int i = 0 ; i < G->getV() ; i++)
            std::cout << "  Vertex: " << i << " : " << centrality[i] <<
                                                                    std::endl;
        std::cout << std::endl;
    }
}
}