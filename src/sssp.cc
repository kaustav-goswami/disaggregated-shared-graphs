#include "sssp.hh"


namespace simple {
SSSP::SSSP(Graph *G, int source) {
    dijkstra(G, source);
    std::cout << std::endl;
}

void SSSP::dijkstra(Graph *G, int source) {
    // dist will be in the local memory
    int *dist = (int *) malloc (G->getV() * sizeof(int));
    int *visited = (int *) calloc (G->getV(), sizeof(int));

    // initialize distances
    for (int i = 0 ; i < G->getV() ; i++)
        dist[i] = INT_MAX;

    dist[source] = 0;

    for (int count = 0 ; count < G->getV() - 1 ; count++) {
        int minDist = INT_MAX;
        int minIndex = -1;

        for (int v = 0 ; v < G->getV() ; v++) {
            if (!visited[v] && dist[v] < minDist) {
                minDist = dist[v];
                minIndex = v;
            }
        }

        visited[minIndex] = 1;

        for (int i = G->row_pointer[minIndex];
                                i < G->row_pointer[minIndex + 1]; i++) {
            int neighbor = G->column_index[i];
            int weight = G->weights[i];
            if (!visited[neighbor] &&
                        dist[minIndex] != INT_MAX &&
                        dist[minIndex] + weight < dist[neighbor]) {
                dist[neighbor] = dist[minIndex] + weight;
            }
        }
    }
    if (G->doDisplay()) {
        std::cout << "Vertex Distance from Source " << source << std::endl;
        for (int i = 0 ; i < G->getV() ; i++)
            std::cout << i << " \t\t " << dist[i] << std::endl;
    }
}
}